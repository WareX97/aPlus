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
 * Name:        rt_sigaction
 * Description: examine and change a signal action
 * URL:         http://man7.org/linux/man-pages/man2/rt_sigaction.2.html
 *
 * Input Parameters:
 *  0: 0x0d
 *  1: int
 *  2: const struct sigaction __user *
 *  3: struct sigaction __user *
 *  4: size_t
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

struct sigaction;

SYSCALL(13, rt_sigaction,
long sys_rt_sigaction (int signo, const struct sigaction __user * act, struct sigaction __user * oldact, size_t size) {
    return -ENOSYS;
});
