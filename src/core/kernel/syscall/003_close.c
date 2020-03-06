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

#include <hal/cpu.h>
#include <hal/vmm.h>
#include <hal/debug.h>


/***
 * Name:        close
 * Description: close a file descriptor
 * URL:         http://man7.org/linux/man-pages/man2/close.2.html
 *
 * Input Parameters:
 *  0: 0x03
 *  1: unsigned int fd
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(3, close,
long sys_close (unsigned int fd) {

    if(unlikely(fd < 0))
        return -EBADF;

    if(unlikely(fd > OPEN_MAX))
        return -EBADF;

    DEBUG_ASSERT(current_task->fd[fd].inode);


    __lock(&current_task->lock, {
        
        vfs_close(current_task->fd[fd].inode);

        current_task->fd[fd].inode = NULL;
        current_task->fd[fd].flags = 0;
    
    });
    
    return 0;
});
