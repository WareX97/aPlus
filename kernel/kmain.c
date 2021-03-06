/*                                                                      
 * GPL3 License                                                         
 *                                                                      
 * Author(s):                                                              
 *      Antonino Natale <antonio.natale97@hotmail.com>                  
 *                                                                      
 *                                                                      
 * Copyright (c) 2013-2019 Antonino Natale                              
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
#include <aplus/memory.h>
#include <aplus/smp.h>
#include <aplus/syscall.h>
#include <aplus/module.h>
#include <aplus/network.h>
#include <aplus/hal.h>




#define __init(fn, p)           \
    extern void fn##_init();    \
    fn##_init p 




void cmain(void* arg) {
    (void) arg;

    current_task->priority = TASK_PRIO_MIN;

    for(;;) {

        // TODO: ...

#if defined(__i386__) || defined(__x86_64__)
        __builtin_ia32_pause();
#endif
    }

}




void kmain() {

#if defined(CONFIG_HAVE_SMP)
    __init(smp,     ());
#endif

    __init(syscall, ());
    __init(vfs,     ());

#if defined(CONFIG_HAVE_NETWORK)
    __init(network, ());  // TODO: Rewrite network/sys.c
#endif


    int e;
    if((e = sys_mount(NULL, "/", "tmpfs", 0, NULL)) < 0)
        kpanicf("mount: could not mount fake root: errno(%s)", strerror(-e));

    __init(module,  ());
    __init(root,    ());


    kprintf ("core: %s %s-%s (%s)\n", CONFIG_SYSTEM_NAME,
                                      CONFIG_SYSTEM_VERSION,
                                      CONFIG_SYSTEM_CODENAME,
                                      CONFIG_COMPILER_HOST);
        
    kprintf("core: built with gcc %s (%s)\n", __VERSION__,
                                              __TIMESTAMP__);

    kprintf("core: boot completed in %d ms, %d KiB of memory used\n", arch_timer_generic_getms(), pmm_get_used_memory() >> 10);





    const char* __argv[2] = { "/sbin/init", NULL };
    const char* __envp[1] = { NULL };

    if((e = sys_execve(__argv[0], __argv, __envp)) < 0)
        kpanicf("init: PANIC! execve() failed with errno(%s)\n", strerror(-e));


    arch_reboot(ARCH_REBOOT_HALT);

}
