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

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/smp.h>
#include <aplus/syscall.h>



void root_init(void) {


    char cmd[CONFIG_BUFSIZ]  = { 0 };
    char root[CONFIG_MAXNAMLEN]   = { 0 };
    char rootfs[CONFIG_MAXNAMLEN] = { 0 };


    strncpy(cmd, core->boot.cmdline, sizeof(cmd));


    for(char* s = strtok(cmd, " "); s; s = strtok(NULL, " ")) {

        char* p;
        if((p = strstr(s, "root=")))
            strncpy(root, &s[5], CONFIG_MAXNAMLEN);

        if((p = strstr(s, "rootfs=")))
            strncpy(rootfs, &s[7], CONFIG_MAXNAMLEN);

        // TODO: Add Kernel Arguments
    
    }



    if(root[0] == '\0')
        kpanicf("root: PANIC! 'root' param not found: %s", core->boot.cmdline);

    if(rootfs[0] == '\0')
        kpanicf("root: PANIC! 'rootfs' param not found: %s", core->boot.cmdline);



    int e;
    if((e = sys_mkdir("/root", 0644)) < 0)
        kpanicf("root: PANIC! mkdir() failed: errno(%s)", strerror(-e));

    if((e = sys_mount(root, "/root", rootfs, 0, NULL)) < 0)
        kpanicf("root: PANIC! mount %s in %s failed: errno(%s)", "/root", root, strerror(-e));

    if((e = sys_mount("/dev", "/root/dev", "bind", 0, NULL)))
        kpanicf("root: PANIC! mount %s in %s failed: errno(%s)", "/root/dev", "/dev", strerror(-e));

    if((e = sys_chroot("/root")) < 0)
        kpanicf("root: PANIC! chroot() failed: errno(%s)", strerror(-e));
    
    
}