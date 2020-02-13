#ifndef _APLUS_BASE_H
#define _APLUS_BASE_H

#include <stdint.h>



#define __KERNEL__      1


#ifdef DEBUG
#undef DEBUG
#endif

#ifdef CONFIG_HAVE_DEBUG
#define DEBUG           1
#define DEBUG_LEVEL     CONFIG_DEBUG_LEVEL
#endif



#ifndef likely
#define likely(cond)    __builtin_expect(!!(cond), 1)
#endif

#ifndef unlikely
#define unlikely(cond)  __builtin_expect(!!(cond), 0)
#endif

#ifndef barrier
#define barrier()       __asm__ __volatile__("" ::: "memory")
#endif



#ifndef __packed
#define __packed        __attribute__((packed))
#endif

#ifndef __pure
#define __pure          __attribute__((pure))
#endif

#ifndef __weak
#define __weak          __attribute__((weak))
#endif

#ifndef __deprecated
#define __deprecated    __attribute__((deprecated))
#endif

#ifndef __noreturn
#define __noreturn      __attribute__((noreturn))
#endif

#ifndef __alias
#define __alias(s)      __attribute__((alias(#s)))
#endif


#if defined(DEBUG) && DEBUG_LEVEL >= 1

#define __user          __attribute__((noderef, address_space(1)))
#define __kernel        __attribute__((address_space(0)))
#define __safe          __attribute__((safe))
#define __force         __attribute__((force))
#define __nocast        __attribute__((nocast))
#define __iomem         __attribute__((noderef, address_space(2)))
#define __percpu        __attribute__((noderef, address_space(3)))

#define __must_hold(x)  __attribute__((context(x,1,1)))
#define __acquires(x)   __attribute__((context(x,0,1)))
#define __releases(x)   __attribute__((context(x,1,0)))
#define __acquire(x)    __context__(x,1)
#define __release(x)    __context__(x,-1)

#define __cond_lock(x, c)    \
    ((c) ? ({ __acquire(x); 1; }) : 0)


extern void __chk_user_ptr(const volatile void __user *);
extern void __chk_io_ptr(const volatile void __iomem *);

#else

#define __user
#define __kernel
#define __safe
#define __force
#define __nocast
#define __iomem
#define __chk_user_ptr(x)               (void) 0
#define __chk_io_ptr(x)                 (void) 0
#define __builtin_warning(x, y...)      (1)
#define __must_hold(x)
#define __acquires(x)
#define __releases(x)
#define __acquire(x)                    (void) 0
#define __release(x)                    (void) 0
#define __cond_lock(x, c)               (c)
#define __percpu

#endif



#define CORE_BUFSIZ                     1024

struct syscore {

    struct {
        uintptr_t phys_upper;
        uintptr_t phys_lower;
    } memory;

    struct {
        char cmdline[CORE_BUFSIZ];
        char bootloader[CORE_BUFSIZ];
    } boot;

    struct {

        uint16_t width;
        uint16_t height;
        uint16_t depth;
        uint32_t pitch;
        uintptr_t address;

        uint8_t red_field_position;
        uint8_t green_field_position;
        uint8_t blue_field_position;

        uint8_t red_mask_size;
        uint8_t green_mask_size;
        uint8_t blue_mask_size;

    } framebuffer;


    struct {
        uintptr_t load_base_address;
    } exe; 

};

//?
//? See src/core/kmain.c
//?
extern struct syscore* core;

#endif