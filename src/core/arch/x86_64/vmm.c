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
                                                                        
#include <stdint.h>
#include <string.h>
#include <aplus/core/base.h>
#include <aplus/core/debug.h>
#include <aplus/core/endian.h>
#include <aplus/core/memory.h>
#include <aplus/core/ipc.h>
#include <aplus/core/hal.h>

#include <arch/x86/asm.h>
#include <arch/x86/cpu.h>
#include <arch/x86/vmm.h>


static uintptr_t alloc_page(uintptr_t pagesize, int zero) {

    DEBUG_ASSERT(pagesize);
    DEBUG_ASSERT(X86_MMU_PAGESIZE == PML1_PAGESIZE);


    uintptr_t p;

    if(likely(pagesize == X86_MMU_PAGESIZE))
        p = pmm_alloc_block();
    else
        p = pmm_alloc_blocks_aligned(pagesize >> 12, pagesize);

    
    if(likely(zero))
        memset((void*) arch_vmm_p2v(p, ARCH_VMM_AREA_HEAP), 0, pagesize);

    return p;
}





#if 0
int x86_ptr_access(uintptr_t address, int mode) {

    x86_page_t* d;
    x86_page_t* aspace = (x86_page_t*) (CONFIG_KERNEL_BASE + x86_get_cr3());

    /* CR3-L4 */ 
    {
        d = &aspace[(address >> 39) & 0x1FF];
    }

    /* PML4-L3 */
    {
        if(unlikely(!(*d & X86_MMU_PG_P)))
            return 0;

        d = &((x86_page_t*) ((uintptr_t) (*d & ~0xFFF) + CONFIG_KERNEL_BASE)) [(address >> 30) & 0x1FF];
    }

    /* PDP-L2 */
    {
        if(unlikely(!(*d & X86_MMU_PG_P)))
            return 0;

        if(unlikely(*d & X86_MMU_PG_PS)) /* Check 1GiB Page */
            goto check;
            
        d = &((x86_page_t*) ((uintptr_t) (*d & ~0xFFF) + CONFIG_KERNEL_BASE)) [(address >> 21) & 0x1FF];
    }

    /* PD-L1 */
    {
        if(unlikely(!(*d & X86_MMU_PG_P)))
            return 0;

        if(unlikely(*d & X86_MMU_PG_PS)) /* Check 2MiB Page */
            goto check;

        d = &((x86_page_t*) ((uintptr_t) (*d & ~0xFFF) + CONFIG_KERNEL_BASE)) [(address >> 12) & 0x1FF];
    }

    /* Page Table */
check:
    if(mode & R_OK)
        if(unlikely(!(*d & X86_MMU_PG_P)))
            return 0;

    if(mode & W_OK)
        if(unlikely(!(*d & X86_MMU_PG_RW)))
            return 0;

    return 1;
}


uintptr_t x86_ptr_phys(uintptr_t address) {

    x86_page_t* d;
    x86_page_t* aspace = (x86_page_t*) (CONFIG_KERNEL_BASE + x86_get_cr3());

    uintptr_t addend = address & (PAGE_SIZE - 1);
    

    /* CR3-L4 */ 
    {
        d = &aspace[(address >> 39) & 0x1FF];
    }

    /* PML4-L3 */
    {
        if(unlikely(!(*d & X86_MMU_PG_P)))
            return 0;

        d = &((x86_page_t*) ((uintptr_t) (*d & ~0xFFF) + CONFIG_KERNEL_BASE)) [(address >> 30) & 0x1FF];
    }

    /* PDP-L2 */
    {
        if(unlikely(!(*d & X86_MMU_PG_P)))
            return 0;

        if(unlikely(*d & X86_MMU_PG_PS)) /* Check 1GiB Page */
            { addend = address & 0x3FFFFFFF; goto check; }
            
        d = &((x86_page_t*) ((uintptr_t) (*d & ~0xFFF) + CONFIG_KERNEL_BASE)) [(address >> 21) & 0x1FF];
    }

    /* PD-L1 */
    {
        if(unlikely(!(*d & X86_MMU_PG_P)))
            return 0;

        if(unlikely(*d & X86_MMU_PG_PS)) /* Check 2MiB Page */
            { addend = address & 0x1FFFFF; goto check; }

        d = &((x86_page_t*) ((uintptr_t) (*d & ~0xFFF) + CONFIG_KERNEL_BASE)) [(address >> 12) & 0x1FF];
    }

    /* Page Table */
check:    
    if(unlikely(!(*d & X86_MMU_PG_P)))
        return 0;

    return ((*d >> 12) << 12) + addend;
}
#endif





/*!
 * @brief arch_vmm_getpagesize().
 *        Get page size.
 */
uintptr_t arch_vmm_getpagesize() {
    return X86_MMU_PAGESIZE;
}


/*!
 * @brief arch_vmm_gethugepagesize().
 *        Get huge page size.
 */
uintptr_t arch_vmm_gethugepagesize() {
    return X86_MMU_HUGE_2MB_PAGESIZE;
}



/*!
 * @brief arch_vmm_p2v().
 *        Convert a physical address to virtual one.
 * 
 * @param physaddr: physical address.
 * @param type: type of memory area.
 */
uintptr_t arch_vmm_p2v(uintptr_t physaddr, int type) {

    switch(type) {

        case ARCH_VMM_AREA_HEAP:
            return physaddr + KERNEL_HEAP_AREA;
        
        case ARCH_VMM_AREA_KERNEL:
            return physaddr + KERNEL_HIGH_AREA;

    }

    BUG_ON(0);
    return -1;

}


/*!
 * @brief arch_vmm_v2p().
 *        Convert a virtual address to physical one.
 * 
 * @param virtaddr: virtual address.
 * @param type: type of memory area.
 */
uintptr_t arch_vmm_v2p(uintptr_t virtaddr, int type) {

    switch(type) {

        case ARCH_VMM_AREA_HEAP:
            return virtaddr - KERNEL_HEAP_AREA;
        
        case ARCH_VMM_AREA_KERNEL:
            return virtaddr - KERNEL_HIGH_AREA;

        //case ARCH_VMM_AREA_USER:
            //return __x86_ptr_phys(virtaddr);

    }

    BUG_ON(0);
    return -1;

}


/*!
 * @brief arch_vmm_map().
 *        Map virtual memory.
 * 
 * @param space: address space.
 * @param virtaddr: virtual address.
 * @param physaddr: physical address.
 * @param length: size of virtual space.
 * @param flags: @see include/arch/x86/vmm.h
 */
uintptr_t arch_vmm_map(vmm_address_space_t* space, uintptr_t virtaddr, uintptr_t physaddr, size_t length, int flags) {

    DEBUG_ASSERT(space);
    DEBUG_ASSERT(length);


    uintptr_t pagesize;

    uintptr_t s = virtaddr;
    uintptr_t p = physaddr;
    uintptr_t e = virtaddr + length;


    if(flags & ARCH_VMM_MAP_HUGETLB) {

        if(flags & ARCH_VMM_MAP_HUGE_1GB)
            pagesize = X86_MMU_HUGE_1GB_PAGESIZE;
        else
            pagesize = X86_MMU_HUGE_2MB_PAGESIZE;
    
    } else
        pagesize = X86_MMU_PAGESIZE; 




    if(s & (pagesize - 1))
        s = (s & ~(pagesize - 1));

    if(p & (pagesize - 1))
        p = (p & ~(pagesize - 1));

    if(e & (pagesize - 1))
        e = (e & ~(pagesize - 1)) + pagesize;



    uint64_t b = X86_MMU_PG_P;

    if(flags & ARCH_VMM_MAP_RDWR)
        b |= X86_MMU_PG_RW;

    if(flags & ARCH_VMM_MAP_USER)
        b |= X86_MMU_PG_U;

    if(flags & ARCH_VMM_MAP_UNCACHED)
        b |= X86_MMU_PG_CD;

    if(flags & ARCH_VMM_MAP_SHARED)
        b |= X86_MMU_PG_G;





    spinlock_lock(&space->lock);

    for(; s < e; s += pagesize, p += pagesize) {

        x86_page_t* d;

        /* CR3-L4 */
        {
            d = &((x86_page_t*) arch_vmm_p2v(space->pm, ARCH_VMM_AREA_HEAP)) [(s >> 39) & 0x1FF];
        }

        /* PML4-L3 */
        {
            if(!(*d & X86_MMU_PG_P))
                *d = alloc_page(X86_MMU_PAGESIZE, 1) | X86_MMU_PG_P;

            d = &((x86_page_t*) arch_vmm_p2v(*d & X86_MMU_ADDRESS_MASK, ARCH_VMM_AREA_HEAP)) [(s >> 30) & 0x1FF];
        }


        if(pagesize != X86_MMU_HUGE_1GB_PAGESIZE) {

            /* PDP-L2 */
            {
                DEBUG_ASSERT(!(*d & X86_MMU_PG_PS) && "PDP-L2 is 1GiB Page");

                if(!(*d & X86_MMU_PG_P))
                    *d = alloc_page(X86_MMU_PAGESIZE, 1) | X86_MMU_PG_P;

                d = &((x86_page_t*) arch_vmm_p2v(*d & X86_MMU_ADDRESS_MASK, ARCH_VMM_AREA_HEAP)) [(s >> 21) & 0x1FF];

            }

            if(pagesize != X86_MMU_HUGE_2MB_PAGESIZE) {

                /* PD-L1 */
                {
                    DEBUG_ASSERT(!(*d & X86_MMU_PG_PS) && "PD-L1 is 2Mib Page");

                    if(!(*d & X86_MMU_PG_P))
                        *d = alloc_page(X86_MMU_PAGESIZE, 1) | b;

                    d = &((x86_page_t*) arch_vmm_p2v(*d & X86_MMU_ADDRESS_MASK, ARCH_VMM_AREA_HEAP)) [(s >> 12) & 0x1FF];
                }

            }

        }


        /* Page Table */
        {
            DEBUG_ASSERT(!(*d & X86_MMU_PG_P) && "Page already used, unmap first");


            //* Set Page Type
            switch((flags & ARCH_VMM_MAP_TYPE_MASK)) {

                case ARCH_VMM_MAP_TYPE_PAGE:
                    b |= X86_MMU_PG_AP_TP_PAGE;
                    break;

                case ARCH_VMM_MAP_TYPE_STACK:
                    b |= X86_MMU_PG_AP_TP_STACK;
                    break;

                case ARCH_VMM_MAP_TYPE_MMAP:
                    b |= X86_MMU_PG_AP_TP_MMAP;
                    break;

                case ARCH_VMM_MAP_TYPE_SWAP:
                    b |= X86_MMU_PG_AP_TP_SWAP;
                    break;

            }


            //* Set No-Execute Bit
            if(flags & ARCH_VMM_MAP_NOEXEC)
                if(core->bsp.xfeatures & X86_CPU_XFEATURES_XD)
                    b |= X86_MMU_PT_NX;             /* XD */



            if(flags & ARCH_VMM_MAP_HUGETLB) {

                b |= X86_MMU_PG_PS;

                if(flags & ARCH_VMM_MAP_VIDEO_MEMORY)  
                    if(core->bsp.features & X86_CPU_FEATURES_PAT)
                        b |= X86_MMU_PG_PAT;        /* WC */

            } else {

                if(flags & ARCH_VMM_MAP_VIDEO_MEMORY)  
                    if(core->bsp.features & X86_CPU_FEATURES_PAT)
                        b |= X86_MMU_PT_PAT;        /* WC */

            }


            if(flags & ARCH_VMM_MAP_FIXED)
                *d = p | b;
            else
                *d = alloc_page(pagesize, 0) | X86_MMU_PG_AP_PFB | b;



#if defined(DEBUG) && DEBUG_LEVEL >= 4
            kprintf("arch_vmm_map(): virtaddr(%p) physaddr(%p) flags(%p) pagesize(%p)\n", s, *d & X86_MMU_ADDRESS_MASK, b, pagesize);
#endif

        }


        __asm__ __volatile__ ("invlpg (%0)" :: "r"(s) : "memory");
        
        space->size += pagesize >> 12;

    }

    spinlock_unlock(&space->lock);

    return virtaddr;

}



/*!
 * @brief arch_vmm_unmap().
 *        Unmap virtual memory.
 * 
 * @param space: address space.
 * @param virtaddr: virtual address.
 * @param physaddr: physical address.
 * @param length: size of virtual space.
 * @param flags: @see include/arch/x86/vmm.h
 */
uintptr_t arch_vmm_unmap(vmm_address_space_t* space, uintptr_t virtaddr, size_t length) {

    DEBUG_ASSERT(space);
    DEBUG_ASSERT(length);


    uintptr_t pagesize = X86_MMU_PAGESIZE;

    uintptr_t s = virtaddr;
    uintptr_t e = virtaddr + length;


    if(s & (X86_MMU_PAGESIZE - 1))
        s = (s & ~(X86_MMU_PAGESIZE - 1));

    if(e & (X86_MMU_PAGESIZE - 1))
        e = (e & ~(X86_MMU_PAGESIZE - 1)) + X86_MMU_PAGESIZE;



    spinlock_lock(&space->lock);

    for(; s < e; s += X86_MMU_PAGESIZE) {


        x86_page_t* d;

        /* CR3-L4 */ 
        {
            d = &((x86_page_t*) arch_vmm_p2v(space->pm, ARCH_VMM_AREA_HEAP)) [(s >> 39) & 0x1FF];
        }

        /* PML4-L3 */
        {
            DEBUG_ASSERT((*d & X86_MMU_PG_P) && "PML4-L3 not exist");

            d = &((x86_page_t*) arch_vmm_p2v(*d & X86_MMU_ADDRESS_MASK, ARCH_VMM_AREA_HEAP)) [(s >> 30) & 0x1FF];
        }


        /* HUGE_1GB */
        if(!(*d & X86_MMU_PG_PS)) {

            /* PDP-L2 */
            {
                DEBUG_ASSERT((*d & X86_MMU_PG_P) && "PDP-L2 not exist");

                d = &((x86_page_t*) arch_vmm_p2v(*d & X86_MMU_ADDRESS_MASK, ARCH_VMM_AREA_HEAP)) [(s >> 21) & 0x1FF];
            }

            /* HUGE_2MB */
            if(!(*d & X86_MMU_PG_PS)) {

                /* PD-L1 */
                {
                    DEBUG_ASSERT((*d & X86_MMU_PG_P) && "PDT-L1 not exist");

                    d = &((x86_page_t*) arch_vmm_p2v(*d & X86_MMU_ADDRESS_MASK, ARCH_VMM_AREA_HEAP)) [(s >> 12) & 0x1FF];
                }

            } else
                pagesize = X86_MMU_HUGE_2MB_PAGESIZE;

        } else
            pagesize = X86_MMU_HUGE_1GB_PAGESIZE;


        /* Page Table */
        {
            DEBUG_ASSERT((*d & X86_MMU_PG_P) && "Page already unmapped");

            if(*d & X86_MMU_PG_AP_PFB)
                pmm_free_blocks(*d & X86_MMU_ADDRESS_MASK, pagesize >> 12);


#if defined(DEBUG) && DEBUG_LEVEL >= 4
            kprintf("arch_vmm_unmap(): virtaddr(%p) physaddr(%p) pagesize(%p)\n", s, *d & X86_MMU_ADDRESS_MASK, pagesize);
#endif

            *d = X86_MMU_CLEAR;
    
        }


        __asm__ __volatile__ ("invlpg (%0)" :: "r"(virtaddr) : "memory");

        space->size -= pagesize >> 12;

    }

    spinlock_unlock(&space->lock);

    return virtaddr;

}


void arch_vmm_clone(vmm_address_space_t* dest, vmm_address_space_t* src) {

    DEBUG_ASSERT(src);
    DEBUG_ASSERT(src->pm);
    DEBUG_ASSERT(dest);

    if(dest->pm == 0ULL)
        dest->pm = alloc_page(X86_MMU_PAGESIZE, 1);

    


    void __clone_pt(uintptr_t __s, uintptr_t __d, int level) {

        DEBUG_ASSERT(__s);
        DEBUG_ASSERT(__d);


        uint64_t* s = (uint64_t*) arch_vmm_p2v(__s, ARCH_VMM_AREA_HEAP);
        uint64_t* d = (uint64_t*) arch_vmm_p2v(__d, ARCH_VMM_AREA_HEAP);

        int i;
        for(i = 0; i < X86_MMU_PT_ENTRIES; i++) {

            //? Skip unallocated pages
            if(s[i] == X86_MMU_CLEAR)
                continue;

            //? Skip special pages: stack, mmap, swap
            if((s[i] & X86_MMU_PG_AP_TP_MASK) != X86_MMU_PG_AP_TP_PAGE)
                continue;


            if((s[i] & X86_MMU_PG_PS) || (level == 1))
                d[i] = s[i];

            else {

                d[i] = alloc_page(X86_MMU_PAGESIZE, 1) | (s[i] & ~X86_MMU_ADDRESS_MASK) | X86_MMU_PG_AP_PFB;

                __clone_pt(((uintptr_t) s[i]) & X86_MMU_ADDRESS_MASK, ((uintptr_t) d[i]) & X86_MMU_ADDRESS_MASK, level - 1);

            }

        }

    }


    __lock(&src->lock,
        __clone_pt(src->pm, dest->pm, 4));


    src->refcount++;
    dest->size = 0;
    dest->refcount = 1;
    
    spinlock_init(&dest->lock);

}