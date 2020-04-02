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
 * Name:        lseek
 * Description: reposition read/write file offset
 * URL:         http://man7.org/linux/man-pages/man2/lseek.2.html
 *
 * Input Parameters:
 *  0: 0x08
 *  1: unsigned int fd
 *  2: off_t offset
 *  3: unsigned int whence
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(8, lseek,
long sys_lseek (unsigned int fd, off_t offset, unsigned int whence) {

    if(unlikely(fd > CONFIG_OPEN_MAX)) /* TODO: Add Network Support */
        return -EBADF;

    if(unlikely(!current_task->fd->descriptors[fd].ref))
        return -EBADF;



    struct stat st;
    if(vfs_getattr(current_task->fd->descriptors[fd].ref->inode, &st) < 0)
        return -errno;


    if(S_ISFIFO(st.st_mode))
        return -ESPIPE;



    __lock(&current_task->fd->descriptors[fd].ref->lock, {
        
        switch(whence) {

            case SEEK_SET:
                current_task->fd->descriptors[fd].ref->position = offset;
                break;

            case SEEK_CUR:
                current_task->fd->descriptors[fd].ref->position += offset;
                break;

            case SEEK_END:
                current_task->fd->descriptors[fd].ref->position = st.st_size + offset;
                break;

            default:
                DEBUG_ASSERT(0 && "sys_lseek() invalid whence!");
                break;

        }

    });

    return current_task->fd->descriptors[fd].ref->position;
});
