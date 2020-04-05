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
 * Name:        pread64
 * Description: read from or write to a file descriptor at a given
       offset
 * URL:         http://man7.org/linux/man-pages/man2/pread64.2.html
 *
 * Input Parameters:
 *  0: 0x11
 *  1: unsigned int fd
 *  2: char __user * buf
 *  3: size_t count
 *  4: loff_t pos
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(17, pread64,
long sys_pread64 (unsigned int fd, char __user * buf, size_t count, off_t pos) {

    if(unlikely(!uio_check(buf, R_OK | W_OK)))
        return -EFAULT;

    if(unlikely(fd > CONFIG_OPEN_MAX)) // TODO: Add Network Support */
        return -EBADF;

    if(unlikely(!current_task->fd->descriptors[fd].ref))
        return -EBADF;


    if(unlikely(!(
        !(current_task->fd->descriptors[fd].flags & O_WRONLY) ||
         (current_task->fd->descriptors[fd].flags & O_RDONLY)
    )))
        return -EPERM;


    // TODO */
    // // if(unlikely(current_task->fd->descriptors[fd].flags & O_NONBLOCK)) {
    // //     struct pollfd p;
    // //     p.fd = fd;
    // //     p.events = POLLIN;
    // //     p.revents = 0;

    // //     if(sys_poll(&p, 1, 0) < 0)
    // //         return -EIO;

    // //     if(!(p.revents & POLLIN))
    // //         return -EAGAIN;
    // // }


    current_task->iostat.rchar += (uint64_t) count;
    current_task->iostat.syscr += 1;


    int e = 0;

    __lock(&current_task->fd->descriptors[fd].ref->lock, {

        if((e = vfs_read(current_task->fd->descriptors[fd].ref->inode, buf, pos, count)) <= 0)
            break;

        current_task->iostat.read_bytes += (uint64_t) e;
        
    });


    if(e < 0)
        return -errno;

    return e;
});