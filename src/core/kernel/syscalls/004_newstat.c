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

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/syscall.h>
#include <aplus/memory.h>
#include <aplus/vfs.h>
#include <aplus/smp.h>
#include <aplus/errno.h>
#include <aplus/hal.h>




/***
 * Name:        newstat
 * Description: 
 * URL:         http://man7.org/linux/man-pages/man2/newstat.2.html
 *
 * Input Parameters:
 *  0: 0x04
 *  1: const char __user * filename
 *  2: struct stat __user * statbuf
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(4, newstat,
long sys_newstat (const char __user * filename, struct stat __user * statbuf) {

    if(!filename)
        return -EINVAL;

    if(!statbuf)
        return -EINVAL;

    if(!ptr_check(filename, R_OK))
        return -EFAULT;

    if(!ptr_check(statbuf, R_OK | W_OK))
        return -EFAULT;

    
    int fd;
    if((fd = sys_open(filename, O_RDONLY, 0)) < 0)
        return fd;
    

    DEBUG_ASSERT(current_task->fd[fd].ref);


    int e;

    __lock(&current_task->fd[fd].ref->lock, {

        e = vfs_getattr(current_task->fd[fd].ref->inode, statbuf);

    });


    if((fd = sys_close(fd)) < 0)
        return fd;


    if(e < 0)
        return -errno;

    return 0;
    
});
