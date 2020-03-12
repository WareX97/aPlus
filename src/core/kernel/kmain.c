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

#include <hal/timer.h>

#define __init(fn, p)   \
    fn##_init p 



static struct syscore __core;
struct syscore* core = &__core;


int n = 0;

void cmain() {

    current_task->priority = TASK_PRIO_MIN;

    for(;;)
#if defined(__i386__) || defined(__x86_64__)
        __builtin_ia32_pause()
#endif
    ;

}




void kmain() {

#if defined(CONFIG_HAVE_SMP)
    __init(smp,     ());
#endif

    __init(syscall, ());
    __init(vfs,     ());
    __init(network, ());



    int e;
    if((e = sys_mount(NULL, "/", "tmpfs", 0, NULL)) < 0)
        kpanicf("mount: could not mount fake root: errno(%d)", -e);

    __init(module,  ());
    //__init(root,    ());



    kprintf ("core: %s %s-%s (%s)\n", CONFIG_SYSTEM_NAME,
                                      CONFIG_SYSTEM_VERSION,
                                      CONFIG_SYSTEM_CODENAME,
                                      CONFIG_COMPILER_HOST);
        
    kprintf("core: built with gcc %s (%s)\n", __VERSION__,
                                              __TIMESTAMP__);

    kprintf("core: initialization completed in %d ms, %d KiB of memory used\n", 
            arch_timer_getms(), 
            pmm_get_used_memory() >> 10
    );


    int fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    DEBUG_ASSERT(fd >= 0);

    struct sockaddr_in in;
    in.sin_family = AF_INET;
    in.sin_addr.s_addr = INADDR_ANY;
    in.sin_port = __builtin_bswap16(8888);

    lwip_bind(fd, &in, sizeof(in));
    lwip_listen(fd, 3);
        kprintf("LISTEN!\n");

    do {

        int len = sizeof(in);
        int p = lwip_accept(fd, &in, &len);
        kprintf("NEW CLIENT!\n");

        DEBUG_ASSERT(p >= 0);

        char buf[32];
        
        lwip_read(p, buf, 32);
        //lwip_close(p);

        buf[31] = 0;

        kprintf("Received %s\n", buf);

    } while(0);

    //static char* __argv[2] = { "/sbin/init", NULL };
    //static char* __envp[1] = { NULL };

    // TODO: ...

}
