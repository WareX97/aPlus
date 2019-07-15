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
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


/***
 * Name:        rt_sigqueueinfo
 * Description: queue a signal and data
 * URL:         http://man7.org/linux/man-pages/man2/rt_sigqueueinfo.2.html
 *
 * Input Parameters:
 *  0: 0x81
 *  1: pid_t pid
 *  2: int sig
 *  3: siginfo_t __user * uinfo
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */



SYSCALL(129, rt_sigqueueinfo,
long sys_rt_sigqueueinfo (pid_t pid, int sig, siginfo_t __user * uinfo) {
    return -ENOSYS;
});
