/*
 * Author:
 *      Antonino Natale <antonio.natale97@hotmail.com>
 * 
 * Copyright (c) 2013-2019 Antonino Natale
 * 
 * 
 * This file is part of aPlus.
 * 
 * aPlus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * aPlus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with aPlus.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/syscall.h>
#include <aplus/memory.h>
#include <aplus/vfs.h>
#include <aplus/smp.h>
#include <aplus/errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <aplus/hal.h>





#ifndef PATH_MAX
#define PATH_MAX        4096
#endif

#define __MIN(a, b)     ((a) < (b) ? (a) : (b))
#define MIN(a, b)       __MIN((uintptr_t) (a), (uintptr_t) (b))

#define ZERO(p)         memset(p, 0, sizeof(p))



static inode_t* path_symlink(inode_t*, size_t);
static inode_t* path_find(inode_t*, const char*, size_t);
static inode_t* path_lookup(inode_t*, const char*, int, mode_t);


static inode_t* path_symlink(inode_t* inode, size_t size) {

    DEBUG_ASSERT(inode);
    DEBUG_ASSERT(size);


    char s[size + 1];
    ZERO(s);

    if(vfs_readlink(inode, s, size) <= 0)
        return NULL;

    return path_lookup(inode->parent, s, O_RDONLY, 0);

}


static inode_t* path_find(inode_t* inode, const char* path, size_t size) {

    DEBUG_ASSERT(inode);
    DEBUG_ASSERT(path);
    DEBUG_ASSERT(size);


    char s[size + 1]; 
    ZERO(s);

    strncpy(s, path, size);


    if((inode = vfs_finddir(inode, s)) == NULL)
        return NULL;

    struct stat st;
    if(vfs_getattr(inode, &st) < 0)
        return NULL;


    if(S_ISLNK(st.st_mode))
        inode = path_symlink(inode, st.st_size);

    return inode;
}


static inode_t* path_lookup(inode_t* cwd, const char* path, int flags, mode_t mode) {


    inode_t* c;

    if(path[0] == '/')
        { c = current_task->fs->root; path++; }
    else
        c = cwd;

    DEBUG_ASSERT(c);



    while(strchr(path, '/') && c) {
         
        c = path_find(c, path, strcspn(path, "/")); 
        path = strchr(path, '/') + 1;
    
    }


    if(unlikely(!c))
        return errno = ENOENT, NULL;

    
    inode_t* r;

    if(path[0] != '\0')
        r = path_find(c, path, strlen(path));
    else
        r = c;


    if(unlikely(!r)) {
        if(flags & O_CREAT) {
            r = vfs_creat(c, path, mode);

            if(unlikely(!r))
                return NULL;
        }
        else
            return errno = ENOENT, NULL;
    } else
        if((flags & O_EXCL) && (flags & O_CREAT))
            return errno = EEXIST, NULL;


    return r;   
}




/***
 * Name:        openat
 * Description: open and possibly create a file
 * URL:         http://man7.org/linux/man-pages/man2/openat.2.html
 *
 * Input Parameters:
 *  0: 0x101
 *  1: int dfd
 *  2: const char __user * filename
 *  3: int flags
 *  4: umode_t mode
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(257, openat,
long sys_openat (int dfd, const char __user * filename, int flags, mode_t mode) {
    
    if(dfd < 0) {
       
        if(dfd != AT_FDCWD)
            return -EBADF;
    
    } else {

        if(dfd > CONFIG_OPEN_MAX)
            return -EBADF;

        if(unlikely(!current_task->fd->descriptors[dfd].ref))
            return -EBADF;

    }


    if(unlikely(!filename))
        return -EINVAL;
    
    if(unlikely(!uio_check(filename, R_OK)))
        return -EFAULT;



    inode_t* cwd;
    if(dfd == AT_FDCWD)
        cwd = current_task->fs->cwd;
    else
        cwd = current_task->fd->descriptors[dfd].ref->inode;



    inode_t* r;

    if((r = path_lookup(cwd, filename, flags, mode)) == NULL)
        return -errno;



    struct stat st;
    if(vfs_getattr(r, &st) < 0)
        return (errno == ENOSYS) ? -EACCES : -errno;



    if (
#ifdef O_NOFOLLOW
        !(flags & O_NOFOLLOW) &&
#endif
        S_ISLNK(st.st_mode)
    ) {
        if((r = path_symlink(r, st.st_size)) == NULL)
            return -errno;
    }



#ifdef O_DIRECTORY
    if(flags & O_DIRECTORY)
        if(!(S_ISDIR(st.st_mode)))
            return -ENOTDIR;
#endif



    if(current_task->uid != 0) {

        if(st.st_uid == current_task->uid) {
            if(!(
                (flags & O_RDONLY ? (mode & S_IRUSR) : 1) &&
                (flags & O_WRONLY ? (mode & S_IWUSR) : 1) &&
                (flags & O_RDWR   ? (mode & S_IRUSR) && (mode & S_IWUSR) : 1)
            )) return -EACCES;

        } else if(st.st_gid == current_task->gid) {
            if(!(
                (flags & O_RDONLY ? (mode & S_IRGRP) : 1) &&
                (flags & O_WRONLY ? (mode & S_IWGRP) : 1) &&
                (flags & O_RDWR   ? (mode & S_IRGRP) && (mode & S_IWGRP) : 1)
            )) return -EACCES;
        
        } else {
            if(!(
                (flags & O_RDONLY ? (mode & S_IROTH) : 1) &&
                (flags & O_WRONLY ? (mode & S_IWOTH) : 1) &&
                (flags & O_RDWR   ? (mode & S_IROTH) && (mode & S_IWOTH) : 1)
            )) return -EACCES;
        
        }
    
    }


    if(vfs_open(r, flags) < 0)
        if(errno != ENOSYS)
            return -errno;



    int fd;

    __lock(&current_task->lock, {

        for(fd = 0; fd < CONFIG_OPEN_MAX; fd++)
            if(!current_task->fd->descriptors[fd].ref)
                break;

        if(fd == CONFIG_OPEN_MAX)
            break;

        
        struct file* ref;
        
        if((ref = fd_append(r, 0, 0)) == NULL)
            fd = FILE_MAX;

        else {

            if(flags & O_APPEND)
                ref->position = st.st_size;
            else
                ref->position = 0;

        
            current_task->fd->descriptors[fd].ref = ref;
            current_task->fd->descriptors[fd].flags = flags;
        
        }

    });
    

    if(fd == CONFIG_OPEN_MAX)
        return -EMFILE;

    if(fd == FILE_MAX)
        return -ENFILE;


    DEBUG_ASSERT(fd >= 0);
    DEBUG_ASSERT(fd <= CONFIG_OPEN_MAX - 1);
    DEBUG_ASSERT(current_task->fd->descriptors[fd].ref);
    DEBUG_ASSERT(current_task->fd->descriptors[fd].ref->inode);

    return fd;

});
