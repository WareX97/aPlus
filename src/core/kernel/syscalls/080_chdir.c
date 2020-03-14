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
#include <aplus/task.h>
#include <aplus/smp.h>
#include <aplus/errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


/***
 * Name:        chdir
 * Description: change working directory
 * URL:         http://man7.org/linux/man-pages/man2/chdir.2.html
 *
 * Input Parameters:
 *  0: 0x50
 *  1: const char __user * filename
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(80, chdir,
long sys_chdir (const char __user * filename) {

    int fd;
    if((fd = sys_open(filename, O_RDONLY, 0)) < 0)
        return fd;

    DEBUG_ASSERT(current_task->fd[fd].inode);
    

    __lock(&current_task->lock, {

        current_task->cwd = current_task->fd[fd].inode;

    });


    if((fd = sys_close(fd)) < 0)
        return fd;

    return 0;

});