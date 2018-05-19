// by ZBY
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <errno.h>
#include <libc/string.h>
#include <raim/fs.h>
#include <raim/device.h>
#include <raim/vmm.h>
#include <raim/file.h>
#include <linux/rbtree.h>


// Unix V6 File System

struct v6fs_superblock {
    struct superblock;
    
    struct rb_root inopool; // inode-pool

    union {
        char raw[512];
        struct {
	        int16_t s_isize;        /* size in blocks of I list */
	        int16_t s_fsize;        /* size in blocks of entire volume */
	        int16_t s_nfree;        /* number of in core free blocks (0-100) */
	        uint16_t s_free[100];    /* in core free blocks */
	        int16_t s_ninode;       /* number of in core I nodes (0-100) */
	        uint16_t s_inode[100];   /* in core free I nodes */
	        int8_t  s_flock;        /* lock during free list manipulation */
	        int8_t  s_ilock;        /* lock during I list manipulation */
	        int8_t  s_fmod;         /* super block modified flag */
	        int8_t  s_ronly;        /* mounted read-only flag */
	        int16_t s_time[2];	    /* current date of last update */
	        int16_t pad[50];
        };
    } ondisk; // raw superblock
};

struct v6fs_inode {
    struct inode;
    
    struct v6fs_inode_ondisk {
	    int16_t	i_mode;
	    int8_t	i_nlink;
	    int8_t	i_uid;
	    int8_t	i_gid;
	    uint8_t	i_size0;
	    uint16_t	i_size1;
	    uint16_t	i_addr[8];
	    int16_t	i_atime[2];
	    int16_t	i_mtime[2];
    } ondisk;
    
    struct rb_node node;
};
#define get_inodesz(ino) ((ino)->ondisk.i_size1 | ((ino)->ondisk.i_size0<<16))
#define set_inodesz(ino, size) do { (ino)->ondisk.i_size1 = (size); (ino)->ondisk.i_size0 = (size) >> 16; } while (0)


struct v6fs_diritem {
    uint16_t inode;
    char name[14];
};

/* modes */
#define	IALLOC	0100000
#define	IFMT	060000
#define		IFDIR	040000
#define		IFCHR	020000
#define		IFBLK	060000
#define	ILARG	010000
#define	ISUID	04000
#define	ISGID	02000
#define ISVTX	01000
#define	IREAD	0400
#define	IWRITE	0200
#define	IEXEC	0100



static struct inode *v6fs_sb_get_inode(struct superblock *self, uint64_t id);
static void v6fs_ino_decref(struct inode *self);
static uint64_t datablock_alloc(struct v6fs_superblock *self);
//static void datablock_free(struct v6fs_superblock *self, uint64_t id);


///// inode


// logical block-id to physical block-id, if error return 0
static uint64_t inode_bmap(struct v6fs_inode *self, uint64_t lbid)
{
    uint16_t ref[256], *pret; uint64_t pbid = 0;
    static_assert(sizeof(ref)==512, "error");
    
    uint16_t *pi_addr;
    
    uint64_t newblk;
    DECLBDEV(self->sb->dev);
    
    if ((self->ondisk.i_mode & ILARG) == 0) {
        // direct reference
        if (0 <= lbid && lbid < 8) {
            pret = &self->ondisk.i_addr[lbid];
            goto done;
        } else {
            newblk = datablock_alloc((void*)self->sb);
            if (newblk == 0) goto fail;

            self->ondisk.i_mode |= ILARG;
            memset(ref, 0, sizeof(ref));
            memcpy(ref, self->ondisk.i_addr, 8 * 2);
            bdev_oneblkio(bdev, ref, newblk, true);
            memset(self->ondisk.i_addr, 0, sizeof(self->ondisk.i_addr));
            self->ondisk.i_addr[0] = newblk;
        }
    }

    if (lbid < 256*7) {
        // first-level indirect reference
        pi_addr = &self->ondisk.i_addr[lbid / 256];
        if (*pi_addr == 0) {
            newblk = datablock_alloc((void*)self->sb);
            if (newblk == 0) goto fail;
            *pi_addr = newblk;
        }

        bdev_oneblkio(bdev, ref, (pbid = *pi_addr), false);
        pret = &ref[lbid % 256];
        
    } else {
        // second-level indirect reference
        
        lbid -= 256*7;
        
        pi_addr = &self->ondisk.i_addr[7];
        if (*pi_addr == 0) {
            newblk = datablock_alloc((void*)self->sb);
            if (newblk == 0) goto fail;
            *pi_addr = newblk;
        }
        
        bdev_oneblkio(bdev, ref, (pbid = *pi_addr), false);
        
        pi_addr = &ref[lbid / 256];
        if (*pi_addr == 0) {
            newblk = datablock_alloc((void*)self->sb);
            if (newblk == 0) goto fail;
            *pi_addr = newblk;
            
            bdev_oneblkio(bdev, ref, pbid, true);
        }
        
        bdev_oneblkio(bdev, ref, (pbid = *pi_addr), false);
        pret = &ref[lbid % 256];
    }

done:
    if (*pret == 0) {
        newblk = datablock_alloc((void*)self->sb);
        if (newblk == 0) goto fail;
        *pret = newblk;
        if (pbid) {
            bdev_oneblkio(bdev, ref, pbid, true);
        }
    }
    return *pret;
fail:
    return 0;
}


static struct inode *v6fs_ino_lookup(struct inode *bself, const char *filename, int flags)
{
    DECLSELF(struct v6fs_inode);
    assert((self->ondisk.i_mode & IFDIR));
    
    char target[14];
    strncpy(target, filename, 14);
    
    DECLBDEV(self->sb->dev);
    
    uint64_t i, j;
    for (i = 0; i < self->length; i++) {
        uint64_t blkid = inode_bmap(self, i/512);
        assert(blkid != 0);
        struct v6fs_diritem direntry[512/16];
        bdev_oneblkio(bdev, direntry, blkid, false);
        static_assert(sizeof(direntry)==512, "error");
        for (j = 0; i + j * 16 < self->length; j++) {
            if (direntry[j].inode != 0 && memcmp(direntry[j].name, target, 14) == 0) {
                return v6fs_sb_get_inode(self->sb, direntry[j].inode);
            }
        }
    }
    return NULL;
}

static struct inode_ops v6fs_ino_ops = {
    .decref = v6fs_ino_decref,
    .lookup = v6fs_ino_lookup,
};

static struct v6fs_inode *new_inode(struct superblock *sb)
{
    struct v6fs_inode *ino = memset(kmalloc(sizeof(struct v6fs_inode),0),0,sizeof(struct v6fs_inode));
    inode_ctor(ino, sb);
    ino->id = -1; // mark as invalid
    ino->ops = &v6fs_ino_ops;
    return ino;
}






///// file ops

int v6fs_at_open(struct file *filp)
{
    kprintf("open %p\n", filp);
    return 0;
}
void v6fs_at_close(struct file *filp)
{
    kprintf("close %p\n", filp);
}
ssize_t v6fs_io(struct file *filp, userptr uptr, size_t len, uint64_t pos, bool iswrite)
{
    char buf[512];
    
    struct v6fs_inode *ino = (void *)filp->inode;
    DECLBDEV(ino->sb->dev);

    ssize_t ret = 0;
    size_t curlen;    
    uint64_t lbid, pbid; // logical / physical block id
    
    if (pos + len < pos) { // avoid interger overflow
        ret = -EINVAL;
        goto done;
    }
    
    // check boundary
    if (!iswrite && pos + len > ino->length) {
        if (pos > ino->length) len = 0; else len = ino->length - pos;
    }
    
    while (len) {
        lbid = pos / 512;
        pbid = inode_bmap(ino, lbid);
        if (pbid == 0) break;
        
        curlen = 512 - pos % 512;
        if (len < curlen) curlen = len;
        
        if (iswrite) {
            if (curlen < 512) bdev_oneblkio(bdev, &buf, pbid, false);
            copy_from_user(buf + pos % 512, uptr, curlen);
            kprintf("write!\n");
            bdev_oneblkio(bdev, buf, pbid, true);
        } else {
            bdev_oneblkio(bdev, buf, pbid, false);
            copy_to_user(uptr, buf + pos % 512, curlen);
        }
        
        len -= curlen;
        uptr += curlen;
        pos += curlen;
        ret += curlen;
    }
    
    // update inode length if necessary
    if (pos > ino->length) {
        ino->length = pos;
    }
done:
    return ret;
}
static struct file_ops v6fs_file_ops = {
    .at_open = v6fs_at_open,
    .at_close = v6fs_at_close,
    .io = v6fs_io,
};


///// superblock

// alloc a data block, will zero that block, return physical block id, if no-space, return 0
static uint64_t datablock_alloc(struct v6fs_superblock *self)
{
    uint64_t ret;
    DECLBDEV(self->dev);
    char buf[512];
    
    if (self->ondisk.s_nfree <= 0) return 0;
    ret = self->ondisk.s_free[--self->ondisk.s_nfree];
    //kprintf("%d\n", (int)ret);
    if (!(self->ondisk.s_isize + 2 <= ret && ret < self->ondisk.s_fsize)) return 0;
    
    if (self->ondisk.s_nfree == 0) {
        // load next queue
        
        bdev_oneblkio(bdev, buf, ret, false);
        memcpy(&self->ondisk.s_nfree, buf, 2);
        memcpy(self->ondisk.s_free, buf + 2, 2 * 100);
    }
    
    memset(buf, 0, sizeof(buf));
    bdev_oneblkio(bdev, buf, ret, true);
    
    return ret;
}

/*static void datablock_free(struct v6fs_superblock *self, uint64_t id) // free a datablock
{
    DECLBDEV(self->dev);
    
    if (self->ondisk.s_nfree >= 100) {
        char buf[512];
        memset(buf, 0, sizeof(buf));
        memcpy(buf, &self->ondisk.s_nfree, 2);
        memcpy(buf + 2, self->ondisk.s_free, 2 * 100);
        bdev_oneblkio(bdev, buf, id, true);
        self->ondisk.s_nfree = 1;
        self->ondisk.s_free[0] = id;
    } else {
        self->ondisk.s_free[self->ondisk.s_nfree++] = id;
    }
}*/

static struct v6fs_inode *inopool_find(struct v6fs_superblock *self, uint64_t id)
{
    struct rb_node *node = self->inopool.rb_node;

  	while (node) {
  		struct v6fs_inode *ino = container_of(node, struct v6fs_inode, node);
		if (id < ino->id)
  			node = node->rb_left;
		else if (id > ino->id)
  			node = node->rb_right;
		else
  			return ino;
	}
	
	return NULL;
}

static void inopool_insert(struct v6fs_superblock *self, struct v6fs_inode *ino)
{
  	struct rb_node **new = &(self->inopool.rb_node), *parent = NULL;

  	/* Figure out where to put new node */
  	while (*new) {
  		struct v6fs_inode *this = container_of(*new, struct v6fs_inode, node);
		parent = *new;
  		if (ino->id < this->id)
  			new = &((*new)->rb_left);
  		else if (ino->id > this->id)
  			new = &((*new)->rb_right);
  		else
  			BUG();
  	}

  	/* Add new node and rebalance tree. */
  	rb_link_node(&ino->node, parent, new);
  	rb_insert_color(&ino->node, &self->inopool);
}
static void inopool_remove(struct v6fs_superblock *self, struct v6fs_inode *ino)
{
    rb_erase(&ino->node, &self->inopool);
}

static void v6fs_ino_decref(struct inode *bself)
{
    DECLSELF(struct v6fs_inode);
    DECLBDEV(self->sb->dev);
    
    if (inode_decref(self) == 0) {
    
        assert(self->length < (1<<24));
        set_inodesz(self, self->length);
        
        // write inode to disk
        struct v6fs_inode_ondisk buf[16]; // 16 inode per sector

        bdev_oneblkio(bdev, buf, 2 + ((self->id-1)/16), false);
        buf[(self->id-1)%16] = self->ondisk;
        bdev_oneblkio(bdev, buf, 2 + ((self->id-1)/16), true);
        
        
        kprintf("inode delete %p\n", bself);
        inopool_remove((void*)self->sb, self);
        kfree(self);
    }
}

static struct inode *v6fs_sb_get_inode(struct superblock *bself, uint64_t id)
{
    DECLSELF(struct v6fs_superblock);
    struct v6fs_inode *ino;
    
    ino = inopool_find(self, id);
    if (ino) return inode_addref(ino);
    
    ino = new_inode(self);
    kprintf("new inode %p\n", ino);
    
    ino->id = id;
    DECLBDEV(self->dev);
    
    struct v6fs_inode_ondisk buf[16]; // 16 inode per sector
    static_assert(sizeof(buf)==512, "error");
    
    bdev_oneblkio(bdev, &buf, 2 + ((id-1)/16), false);
    
    ino->ondisk = buf[(id-1)%16];
    ino->length = get_inodesz(ino);
    
    inopool_insert(self, ino);
        
    //kprintf("len=%d\n",ino->length);
    //dump(&ino->ondisk, sizeof(ino->ondisk));
    
    return ino;
}



static void v6fs_sb_sync(struct superblock *bself)
{
    DECLSELF(struct v6fs_superblock);
    DECLBDEV(self->dev);
    
    // FIXME: sync inode
    
    // sync superblock
    bdev_oneblkio(bdev, self->ondisk.raw, 1, true);
}



static struct superblock_ops v6fs_sb_ops = { // virtual function table
    .get_inode = v6fs_sb_get_inode,
    .sync = v6fs_sb_sync,
};

struct superblock *v6fs_superblock_create(struct bdev *bdev)
{
    assert(bdev->blksz == 512);
    struct v6fs_superblock *self = kmalloc(sizeof(struct v6fs_superblock), 0);
    self->ops = &v6fs_sb_ops;
    self->dev = bdev;
    self->inopool = RB_ROOT;
    self->fops = &v6fs_file_ops;
    
    // read superblock in
    bdev_oneblkio(bdev, self->ondisk.raw, 1, false);
    
    // dump superblock
    //kprintf("inode=%d total=%d\n", self->ondisk.s_isize, self->ondisk.s_fsize);
    //dump(&self->ondisk, sizeof(self->ondisk));
    
    // read root inode
    self->root = self->ops->get_inode(self, 1);


    return self;
}
