// by ZBY
// Virtual File System for RAIM

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/fs.h>

struct inode *inode_ctor(struct inode *ino, struct superblock *sb)
{
    ino->ref = 0;
    ino->sb = sb;
    return inode_addref(ino);
}

struct inode *inode_addref(struct inode *ino)
{
    ino->ref++; // FIXME: use atomic increase
    return ino;
}
atomic_t inode_decref(struct inode *ino)
{
    return --ino->ref; //FIXME: use atomic decrease
}

