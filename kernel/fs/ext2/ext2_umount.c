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
#include <sys/types.h>
#include <sys/mount.h>

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/smp.h>
#include <aplus/ipc.h>
#include <aplus/vfs.h>
#include <aplus/memory.h>

#include "ext2.h"




int ext2_umount(inode_t* dir) {

    DEBUG_ASSERT(dir);
    DEBUG_ASSERT(dir->sb);
    DEBUG_ASSERT(dir->sb->fsid == EXT2_ID);
    DEBUG_ASSERT(dir->sb->root == dir);


    vfs_cache_destroy(&dir->sb->cache);



    ext2_t* ext2 = (ext2_t*) dir->sb->fsinfo;

    kfree(ext2->cache);
    kfree(ext2);

    return 0;
}