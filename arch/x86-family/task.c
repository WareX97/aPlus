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
#include <string.h>
#include <time.h>

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/memory.h>
#include <aplus/ipc.h>
#include <aplus/task.h>
#include <aplus/hal.h>

#include <arch/x86/cpu.h>
#include <arch/x86/asm.h>
#include <arch/x86/vmm.h>
#include <arch/x86/intr.h>
#include <arch/x86/acpi.h>
#include <arch/x86/apic.h>


extern inode_t __vfs_root;


#define FRAME(p)                                    \
    ((interrupt_frame_t*) p->frame)

#define WRITE_SP0(cpu, ptr)                         \
    ((tss_t*) cpu->tss)->sp0 = (uintptr_t) cpu->kstack





void arch_task_switch_address_space(vmm_address_space_t* address_space) {
    x86_set_cr3(address_space->pm);
}


void arch_task_prepare_to_signal(siginfo_t* siginfo) {

    DEBUG_ASSERT(current_task);
    DEBUG_ASSERT(siginfo);
    DEBUG_ASSERT(siginfo->si_code >= 0);
    DEBUG_ASSERT(siginfo->si_code <= _NSIG);

    DEBUG_ASSERT(current_task->userspace.sigstack);
    DEBUG_ASSERT(current_task->userspace.siginfo);


    memcpy( ((interrupt_frame_t*) current_task->sstack), current_task->frame, sizeof(interrupt_frame_t));
    memcpy(&((interrupt_frame_t*) current_task->sstack)->__fpuregs, current_task->fpu, 512); // FIXME


    struct ksigaction* action = &current_task->sighand->action[siginfo->si_code];

    arch_task_context_set(current_task, ARCH_TASK_CONTEXT_PC,     (long) action->sigaction);
    arch_task_context_set(current_task, ARCH_TASK_CONTEXT_STACK,  (long) current_task->userspace.sigstack);

    arch_task_context_set(current_task, ARCH_TASK_CONTEXT_PARAM0, (long) siginfo->si_code);
    arch_task_context_set(current_task, ARCH_TASK_CONTEXT_PARAM1, (long) current_task->userspace.siginfo);
    arch_task_context_set(current_task, ARCH_TASK_CONTEXT_PARAM2, (long) NULL);


    memcpy(current_task->userspace.siginfo, siginfo, sizeof(siginfo_t));

}


void arch_task_return_from_signal(void) {

    memcpy(current_task->frame, ((interrupt_frame_t*) current_task->sstack), sizeof(interrupt_frame_t));
    memcpy(current_task->fpu,  &((interrupt_frame_t*) current_task->sstack)->__fpuregs, 512); // FIXME

}




void arch_task_switch(task_t* prev, task_t* next) {

    DEBUG_ASSERT(current_cpu->frame);

    DEBUG_ASSERT(prev);
    DEBUG_ASSERT(prev->frame);
    DEBUG_ASSERT(prev->address_space->pm);

    DEBUG_ASSERT(next);
    DEBUG_ASSERT(next->frame);
    DEBUG_ASSERT(next->address_space->pm);



    if(unlikely(next->flags & TASK_FLAGS_NO_FRAME)) {

        memcpy(next->frame, current_cpu->frame, sizeof(interrupt_frame_t));
        next->flags &= ~TASK_FLAGS_NO_FRAME;

        __asm__ __volatile__ ("fxsave (%0)" :: "r"(next->fpu));

    }




    if(likely(prev != next)) {
        
        memcpy(prev->frame, current_cpu->frame, sizeof(interrupt_frame_t));
        memcpy(current_cpu->frame, next->frame, sizeof(interrupt_frame_t));

        prev->kstack = current_cpu->kstack;
        prev->ustack = current_cpu->ustack;

        current_cpu->kstack = next->kstack;
        current_cpu->ustack = next->ustack;


        __asm__ __volatile__ ("fxsave (%0)"  :: "r"(prev->fpu));
        __asm__ __volatile__ ("fxrstor (%0)" :: "r"(next->fpu));

        if(unlikely(prev->address_space->pm != next->address_space->pm))
            arch_task_switch_address_space(next->address_space);

    }




    WRITE_SP0(current_cpu, next->kstack);

    x86_wrmsr(X86_MSR_FSBASE, next->userspace.thread_area);


    uint32_t m;

#if TASK_SCHEDULER_PERIOD_NS != 1000000
    m = (20LL - next->priority) / (TASK_SCHEDULER_PERIOD_NS / 1000000);
    m = m ? m : 1;
#else
    m = (20LL - next->priority);
#endif


    apic_timer_reset(m);

}



task_t* arch_task_get_empty_thread(size_t stacksize) {
    

    task_t* task = (task_t*) kcalloc (1, (sizeof(task_t)) + stacksize, GFP_KERNEL);



    task->argv = current_task->argv;
    task->environ = current_task->environ;

    task->tid   = sched_nextpid();
    task->tgid  = current_task->tid;
    task->pgid  = current_task->pgid;
    task->uid   = current_task->uid;
    task->euid  = current_task->euid;
    task->gid   = current_task->gid;
    task->egid  = current_task->egid;
    task->sid   = current_task->sid;

    task->status    = TASK_STATUS_READY;
    task->policy    = current_task->policy;
    task->priority  = current_task->priority;
    task->caps      = current_task->caps;
    task->flags     = 0;

    CPU_ZERO(&task->affinity);
    CPU_OR(&task->affinity, &task->affinity, &current_task->affinity);
    



    #define _(size, offset)     \
        (void*) ((uintptr_t) kmalloc(size, GFP_KERNEL) + offset)


    task->frame         = _(sizeof(interrupt_frame_t), 0);
    task->sstack        = _(sizeof(interrupt_frame_t) + 512, 0); // FIXME
    task->kstack        = _(KERNEL_SYSCALL_STACKSIZE, KERNEL_SYSCALL_STACKSIZE);
    task->ustack        = NULL;
    task->fpu           = _(512, 0); // FIXME

    #undef _


    FRAME(task)->cs     = KERNEL_CS;
    FRAME(task)->flags  = 0x202;
    FRAME(task)->sp     = (uintptr_t) task + sizeof(task_t) + stacksize;
    FRAME(task)->ss     = KERNEL_DS;


    spinlock_init(&task->lock);
    spinlock_init(&task->sched_lock);
    

    task->next   = NULL;
    task->parent = current_task;
    
    return task;
}


pid_t arch_task_spawn_init() {

    task_t* task = (task_t*) kcalloc (1, (sizeof(task_t)), GFP_KERNEL);


    static char* __argv[2] = { "init", NULL };
    static char* __envp[1] = { NULL };

    task->argv     = __argv;
    task->environ  = __envp;


    task->tid   = 
    task->tgid  = sched_nextpid();
    task->pgid  = 1;
    task->uid   =
    task->euid  =
    task->gid   =
    task->egid  = 0;
    task->sid   = 1;

    task->status    = TASK_STATUS_READY;
    task->policy    = TASK_POLICY_RR;
    task->priority  = TASK_PRIO_REG;
    task->caps      = TASK_CAPS_SYSTEM;
    task->flags     = TASK_FLAGS_NO_FRAME;

    CPU_ZERO(&task->affinity);
    CPU_SET(current_cpu->id, &task->affinity);

    
   #define _(size, offset)     \
        (void*) ((uintptr_t) kmalloc(size, GFP_KERNEL) + offset)


    task->frame         = _(sizeof(interrupt_frame_t), 0);
    task->sstack        = _(sizeof(interrupt_frame_t) + 512, 0); // FIXME
    task->kstack        = _(KERNEL_SYSCALL_STACKSIZE, KERNEL_SYSCALL_STACKSIZE);
    task->ustack        = NULL;
    task->fpu           = _(512, 0); // FIXME

    #undef _


    task->address_space = &core->bsp.address_space;
    core->bsp.address_space.refcount++;



    task->fs = (struct fs*) kcalloc(1, sizeof(struct fs), GFP_KERNEL);

    task->fs->cwd   =
    task->fs->root  = &__vfs_root;
    task->fs->exe   = NULL;
    task->fs->umask = 0;

    task->fs->refcount = 1;



    task->fd = (struct fd*) kcalloc(1, sizeof(struct fd), GFP_KERNEL);
    task->fd->refcount = 1;

    task->sighand = (struct sighand*) kcalloc(1, sizeof(struct sighand), GFP_KERNEL);
    task->sighand->refcount = 1;




    int j;
    for(j = 0; j < RLIM_NLIMITS; j++)
        task->rlimits[j].rlim_cur =
        task->rlimits[j].rlim_max = RLIM_INFINITY;

    task->rlimits[RLIMIT_STACK].rlim_cur = KERNEL_STACK_SIZE;



    task->next   = NULL;
    task->parent = NULL;

    spinlock_init(&task->lock);
    spinlock_init(&task->sched_lock);



    if(current_cpu->id != SMP_CPU_BOOTSTRAP_ID)
        task->parent = core->bsp.sched_running;
    

    current_cpu->sched_running = task;
    current_cpu->kstack = task->kstack;

    WRITE_SP0(current_cpu, task->kstack);



#if defined(DEBUG) && DEBUG_LEVEL >= 1
    kprintf("task: spawn init process pid(%d) cpu(%d) kstack(%p)\n", task->tid, arch_cpu_get_current_id(), task->kstack);
#endif



    sched_enqueue(task);

    return task->tid;
}


pid_t arch_task_spawn_kthread(const char* name, void (*entry) (void*), size_t stacksize, void* arg) {

    DEBUG_ASSERT(name);
    DEBUG_ASSERT(entry);
    DEBUG_ASSERT(stacksize);
    

    task_t* task = arch_task_get_empty_thread(stacksize);


    task->argv = (char**) kcalloc(1, (sizeof(char*) << 1) + strlen(name) + 1, GFP_KERNEL);

    task->argv[0] = (char*) &task->argv[2];
    task->argv[1] = (char*) NULL;

    strcpy(task->argv[0], name);


    CPU_ZERO(&task->affinity);
    
    int i;
    for(i = 0; i < (CPU_SETSIZE << 3); i++)
        CPU_SET(i, &task->affinity);
    


    task->address_space = (void*) current_task->address_space;
    current_task->address_space->refcount++;


    task->fs = (struct fs*) kcalloc(1, sizeof(struct fs), GFP_KERNEL);

    task->fs->cwd   = current_task->fs->cwd;
    task->fs->exe   = current_task->fs->exe;
    task->fs->root  = current_task->fs->root;
    task->fs->umask = current_task->fs->umask;
    task->fs->refcount = 1;


    task->fd = (struct fd*) kcalloc(1, sizeof(struct fd), GFP_KERNEL);
    task->fd->refcount = 1;

    task->sighand = (struct sighand*) kcalloc(1, sizeof(struct sighand), GFP_KERNEL);
    task->sighand->refcount = 1;
    
    
    
    memcpy(&task->rlimits, &current_task->rlimits, sizeof(struct rlimit) * RLIM_NLIMITS);


    // TODO: Signal


    FRAME(task)->ip = (uintptr_t) entry;

#if defined(__x86_64__)
    FRAME(task)->di = (uintptr_t) arg;
#else
    (*(uintptr_t*) FRAME(task)->sp)-- = (uintptr_t) NULL;
    (*(uintptr_t*) FRAME(task)->sp)-- = (uintptr_t) arg;
#endif


    spinlock_init(&task->lock);
    spinlock_init(&task->sched_lock);
    

    task->next   = NULL;
    task->parent = current_task;
    

#if defined(DEBUG) && DEBUG_LEVEL >= 1
    kprintf("task: spawn kthread %s pid(%d) ip(%p) kstack(%p) stacksize(%p)\n", task->argv[0], task->tid, entry, task->kstack, stacksize);
#endif

    sched_enqueue(task);

    return task->tid;
}





void arch_task_context_set(task_t* task, int options, long value) {

    DEBUG_ASSERT(task);
    DEBUG_ASSERT(task->frame);


    switch(options) {

        case ARCH_TASK_CONTEXT_COPY:
            memcpy(task->frame, (interrupt_frame_t*) value, sizeof(interrupt_frame_t));
            break;

        case ARCH_TASK_CONTEXT_PC:
            FRAME(task)->ip = value;
            break;

        case ARCH_TASK_CONTEXT_STACK:
            FRAME(task)->sp = value;
            break;

        case ARCH_TASK_CONTEXT_RETVAL:
            FRAME(task)->ax = value;
            break;


#if defined(__i386__)

        case ARCH_TASK_CONTEXT_PARAM0:
        case ARCH_TASK_CONTEXT_PARAM1:
        case ARCH_TASK_CONTEXT_PARAM2:
        case ARCH_TASK_CONTEXT_PARAM3:
        case ARCH_TASK_CONTEXT_PARAM4:
        case ARCH_TASK_CONTEXT_PARAM5:

            FRAME(task)->sp -= sizeof(long);
            uio_w32(FRAME(task)->sp, value);
            break;

#endif


#if defined(__x86_64__)

        case ARCH_TASK_CONTEXT_PARAM0:
            FRAME(task)->di = value;
            break;

        case ARCH_TASK_CONTEXT_PARAM1:
            FRAME(task)->si = value;
            break;

        case ARCH_TASK_CONTEXT_PARAM2:
            FRAME(task)->dx = value;
            break;

        case ARCH_TASK_CONTEXT_PARAM3:
            FRAME(task)->cx = value;
            break;

        case ARCH_TASK_CONTEXT_PARAM4:
            FRAME(task)->r8 = value;
            break;

        case ARCH_TASK_CONTEXT_PARAM5:
            FRAME(task)->r9 = value;
            break;

#endif

        default:
            kpanicf("x86-task: PANIC! invalid ARCH_TASK_CONTEXT_* %d\n", options);

    }

}


long arch_task_context_get(task_t* task, int options) {

    DEBUG_ASSERT(task);
    DEBUG_ASSERT(task->frame);


    switch(options) {

        case ARCH_TASK_CONTEXT_PC:
            return FRAME(task)->ip;

        case ARCH_TASK_CONTEXT_STACK:
            return FRAME(task)->sp;

        case ARCH_TASK_CONTEXT_RETVAL:
            return FRAME(task)->ax;

        default:
            kpanicf("x86-task: PANIC! invalid ARCH_TASK_CONTEXT_* %d\n", options);

    }

    return -1;
    
}