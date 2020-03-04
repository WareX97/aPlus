#ifndef _APLUS_CORE_VFS_H
#define _APLUS_CORE_VFS_H

#ifndef __ASSEMBLY__

#define _GNU_SOURCE
#include <sched.h>

#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <sys/cdefs.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <sys/syslimits.h>
#include <sys/types.h>

#include <aplus/core/base.h>
#include <aplus/core/debug.h>
#include <aplus/core/ipc.h>



#define TMPFS_ID                    0xDEAD1000
#define EXT2_ID                     0xDEAD1001
#define VFAT_ID                     0xDEAD1002




typedef struct inode inode_t;



struct inode_ops {

    int (*open)  (inode_t*, int);
    int (*close) (inode_t*);
    int (*ioctl) (inode_t*, long, void*);

    /* Inode */
    int (*getattr) (inode_t*, struct stat*);
    int (*setattr) (inode_t*, struct stat*);

    int (*setxattr) (inode_t*, const char*, const void*, size_t, int);
    int (*getxattr) (inode_t*, const char*, void*, size_t);
    int (*listxattr) (inode_t*, char*, size_t);
    int (*removexattr) (inode_t*, const char*);

    int (*truncate) (inode_t*, off_t);
    int (*fsync) (inode_t*, int);
    
    ssize_t (*read) (inode_t*, void*, off_t, size_t);
    ssize_t (*write) (inode_t*, const void*, off_t, size_t);
    ssize_t (*readlink) (inode_t*, char*, size_t);


    /* Directory */
    inode_t* (*creat) (inode_t*, const char*, mode_t);
    inode_t* (*finddir) (inode_t*, const char*);
    ssize_t (*readdir) (inode_t*, struct dirent*, off_t, size_t);

    int (*rename) (inode_t*, const char*, const char*);
    int (*symlink) (inode_t*, const char*, const char*);
    int (*unlink) (inode_t*, const char*);

};


struct inode {

    char name[MAXNAMLEN];
    ino_t ino;

    struct superblock* sb;
    struct inode_ops ops;
    struct inode* parent;


    void* userdata;
    spinlock_t lock;

};


struct superblock {

    id_t fsid;
    int flags;

    inode_t* dev;
    inode_t* root;

    ino_t ino;
    struct statvfs st;
    struct inode_ops ops;
    struct vfs_cache cache;

    void* fsinfo;

};


__BEGIN_DECLS


void vfs_init(void);

//* os/kernel/fs/vfs.c
int vfs_mount(inode_t* dev, inode_t* dir, const char __user * fs, int flags, const char __user * args);

int vfs_open (inode_t*, int);
int vfs_close (inode_t*);
int vfs_ioctl (inode_t*, long, void*);

int vfs_getattr (inode_t*, struct stat*);
int vfs_setattr (inode_t*, struct stat*);

int vfs_setxattr (inode_t*, const char*, const void*, size_t, int);
int vfs_getxattr (inode_t*, const char*, void*, size_t);
int vfs_listxattr (inode_t*, char*, size_t);
int vfs_removexattr (inode_t*, const char*);

int vfs_truncate (inode_t*, off_t);
int vfs_fsync (inode_t*, int);

ssize_t vfs_read (inode_t*, void*, off_t, size_t);
ssize_t vfs_write (inode_t*, const void*, off_t, size_t);
ssize_t vfs_readlink (inode_t*, char*, size_t);

inode_t* vfs_creat (inode_t*, const char*, mode_t);
inode_t* vfs_finddir (inode_t*, const char*);
ssize_t vfs_readdir (inode_t*, struct dirent*, off_t, size_t);

int vfs_rename (inode_t*, const char*, const char*);
int vfs_symlink (inode_t*, const char*, const char*);
int vfs_unlink (inode_t*, const char*);


// os/kernel/fs/dcache.c
void dcache_init(void);
void vfs_dcache_add(inode_t*);
void vfs_dcache_remove(inode_t*);
inode_t* vfs_dcache_find(inode_t*, const char*);


// os/kernel/fs/rootfs.c
extern inode_t* vfs_root;
void rootfs_init(void);

__END_DECLS

#endif
#endif