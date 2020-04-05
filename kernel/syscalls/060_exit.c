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
#include <sys/wait.h>

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/syscall.h>
#include <aplus/task.h>
#include <aplus/smp.h>
#include <aplus/hal.h>
#include <aplus/errno.h>



/***
 * Name:        exit
 * Description: terminate the calling process
 * URL:         http://man7.org/linux/man-pages/man2/exit.2.html
 *
 * Input Parameters:
 *  0: 0x3c
 *  1: int error_code
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(60, exit,
long sys_exit (int status) {

    BUG_ON(current_task->tid != 1);


    if(status & (1 << 31))
        current_task->exit.value = status & 0x7FFF;
    else
        current_task->exit.value = (status & 0377) << 8;


#if defined(DEBUG) && DEBUG_LEVEL >= 2
    kprintf("exit: task %d (%s) %s with %X\n", 
        current_task->tid, 
        current_task->argv[0],
        WIFSTOPPED(current_task->exit.value) ? "stopped" : "exited",
        current_task->exit.value & 0xFFFF
    );   
#endif


    current_task->status = (WIFSTOPPED(current_task->exit.value))
        ? TASK_STATUS_STOP
        : TASK_STATUS_ZOMBIE;


    // TODO: implement signal     
    // if(current_task->parent) {
    //     siginfo_t si;
    //     si.si_code = SI_KERNEL;
    //     si.si_pid = current_task->pid;
    //     si.si_uid = current_task->uid;
    //     si.si_value.sival_int = current_task->exit.value;

    //     sched_sigqueueinfo(current_task->parent, SIGCHLD, &si);
    // }



    list_each(current_task->wait_queue, q) {

        if(current_task->status == TASK_STATUS_STOP && !(q->wait_options & WUNTRACED))
            continue;
            
        if(q->wait_status)
            *q->wait_status = current_task->exit.value;

        if(q->wait_rusage)
            memcpy(q->wait_rusage, &current_task->rusage, sizeof(struct rusage));


        current_task->status = TASK_STATUS_DEAD;

        thread_wake_and_return(q, current_task->tid);

    }


    if(current_task->status != TASK_STATUS_STOP) {

        // TODO: Remove references from fs, fd, sighand, ecc...

        //arch_userspace_release();

    }


    thread_postpone_resched(current_task);
    return 0;

});