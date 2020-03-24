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




/***
 * Name:        mount
 * Description: mount filesystem
 * URL:         http://man7.org/linux/man-pages/man2/mount.2.html
 *
 * Input Parameters:
 *  0: 0xa5
 *  1: char __user * dev_name
 *  2: char __user * dir_name
 *  3: char __user * type
 *  4: unsigned long flags
 *  5: void __user * data
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(165, mount,
long sys_mount (char __user * dev_name, char __user * dir_name, char __user * type, unsigned long flags, void __user * data) {

    if(unlikely(!dir_name))
        return -EINVAL;

    if(unlikely(!type))
        return -EINVAL;


    if(unlikely(!ptr_check(dir_name, R_OK)))
        return -EFAULT;

    if(unlikely(!ptr_check(type, R_OK)))
        return -EFAULT;

    if(likely(dev_name))
        if(unlikely(!ptr_check(dev_name, R_OK)))
            return -EFAULT;


    inode_t* s = NULL;
    inode_t* d = NULL;

    int fd;
    int e;



    if((fd = sys_open(dir_name, O_RDONLY, 0)) < 0)
        return fd;

    DEBUG_ASSERT(current_task->fd[fd].ref);
    DEBUG_ASSERT(current_task->fd[fd].ref->inode);

    d = current_task->fd[fd].ref->inode;

    if((e = sys_close(fd)) < 0)
        return e;


    
    if(likely(dev_name)) {
        if((fd = sys_open(dev_name, O_RDONLY, 0)) < 0)
            return fd;

        DEBUG_ASSERT(current_task->fd[fd].ref);
        DEBUG_ASSERT(current_task->fd[fd].ref->inode);

        s = current_task->fd[fd].ref->inode;

        if((e = sys_close(fd)) < 0)
            return e;
    }
   
   
   
    DEBUG_ASSERT(d);

    if(vfs_mount(s, d, type, flags, (const char*) data) < 0)
        return -errno;

    return 0;
});
