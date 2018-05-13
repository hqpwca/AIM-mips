// by ZBY
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <libc/string.h>
#include <raim/fs.h>
#include <raim/device.h>
#include <raim/vmm.h>

// Unix V6 File System

struct v6fs_superblock {
    struct superblock;

    union {
        char raw[512];
        struct {
	        int16_t s_isize;        /* size in blocks of I list */
	        int16_t s_fsize;        /* size in blocks of entire volume */
	        int16_t s_nfree;        /* number of in core free blocks (0-100) */
	        int16_t s_free[100];    /* in core free blocks */
	        int16_t s_ninode;       /* number of in core I nodes (0-100) */
	        int16_t s_inode[100];   /* in core free I nodes */
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
};
#define inodesz(ino) ((ino)->ondisk.i_size1 | ((ino)->ondisk.i_size0<<16))


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

///// inode


// logical block-id to physical block-id
static uint64_t inode_bmap(struct v6fs_inode *self, uint64_t lbid)
{
    if ((self->ondisk.i_mode & ILARG) == 0) {
        // direct reference
        assert(0 <= lbid && lbid < 8);
        return self->ondisk.i_addr[lbid];
    } else {
        uint16_t ref[256];
        static_assert(sizeof(ref)==512, "error");
        struct bdev *bdev = (void*)self->sb->dev;
        
        if (lbid < 256*7) {
            // first-level indirect reference
            bdev_oneblkio(bdev, ref, self->ondisk.i_addr[lbid / 256], false);
            return ref[lbid % 256];
        } else {
            // second-level indirect reference
            
            assert(0);
            lbid -= 256*7;
            
            bdev_oneblkio(bdev, ref, self->ondisk.i_addr[7], false);
            bdev_oneblkio(bdev, ref, ref[lbid / 256], false);
            
            return ref[lbid % 256];
        }
    }
}


static struct inode *v6fs_ino_lookup(struct inode *bself, const char *filename, int flags)
{
    DECLSELF(struct v6fs_inode);
    assert((self->ondisk.i_mode & IFDIR));
    
    char target[14];
    strncpy(target, filename, 14);
    
    struct bdev *bdev = (void*)self->sb->dev;
    
    uint64_t i, j;
    for (i = 0; i < self->length; i++) {
        uint64_t blkid = inode_bmap(self, i/512);
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






///// superblock

static void v6fs_ino_decref(struct inode *self)
{
    if (inode_decref(self) == 0) {
        kfree(self);
    }
}

static struct inode *v6fs_sb_get_inode(struct superblock *self, uint64_t id)
{
    struct v6fs_inode *ino;
    // FIXME: find in pool
    ino = new_inode(self);
    
    ino->id = id;
    DECLBDEV(self->dev);
    
    struct v6fs_inode_ondisk buf[16]; // 16 inode per sector
    static_assert(sizeof(buf)==512, "error");
    
    bdev_oneblkio(bdev, &buf, 2 + ((id-1)/16), false);
    
    ino->ondisk = buf[(id-1)%16];    
    ino->length = inodesz(ino);
    
    // FIXME: add to pool
    
    kprintf("len=%d\n",ino->length);
    dump(&ino->ondisk, sizeof(ino->ondisk));
    
    return inode_addref(ino);
}




static struct superblock_ops v6fs_sb_ops = { // virtual function table
    .get_inode = v6fs_sb_get_inode,
};

struct superblock *v6fs_superblock_create(struct bdev *bdev)
{
    assert(bdev->blksz == 512);
    struct v6fs_superblock *self = kmalloc(sizeof(struct v6fs_superblock), 0);
    self->ops = &v6fs_sb_ops;
    self->dev = bdev;
    
    // read superblock in
    bdev_oneblkio(bdev, self->ondisk.raw, 1, false);
    
    // dump superblock
    kprintf("inode=%d total=%d\n", self->ondisk.s_isize, self->ondisk.s_fsize);
    dump(&self->ondisk, sizeof(self->ondisk));
    
    // read root inode
    self->root = self->ops->get_inode(self, 1);
    
    struct inode *ino = self->root->ops->lookup(self->root, "rkunix", 0);


    char buf[512];
    uint64_t x=inode_bmap((void*)ino, 0x6200/512);
    bdev_oneblkio(bdev, buf, x, false);
    dump(buf,sizeof(buf));

    return self;
}
