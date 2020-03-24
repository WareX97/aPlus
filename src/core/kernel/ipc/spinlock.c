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

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/ipc.h>
#include <aplus/hal.h>




/*!
 * @brief Initialize Spinlock.
 */
void spinlock_init(spinlock_t* lock) {
    DEBUG_ASSERT(lock);

    __atomic_clear(&lock->value, __ATOMIC_RELAXED);
    __atomic_clear(&lock->flags, __ATOMIC_RELAXED);
}

/*!
 * @brief Lock a Spinlock.
 */
void spinlock_lock(spinlock_t* lock) {
    DEBUG_ASSERT(lock);

    while(__atomic_test_and_set(&lock->value, __ATOMIC_ACQUIRE))
#if defined(__i386__) || defined(__x86_64__)
        __builtin_ia32_pause();
#endif
        ;

    lock->flags = arch_intr_disable();
}


/*!
 * @brief Try to lock a Spinlock.
 */
int spinlock_trylock(spinlock_t* lock) {
    DEBUG_ASSERT(lock);

    int e; 
    if((e = !__atomic_test_and_set(&lock->value, __ATOMIC_ACQUIRE)))
        lock->flags = arch_intr_disable();

    return e;
}


/*!
 * @brief Release a Spinlock.
 */
void spinlock_unlock(spinlock_t* lock) {
    DEBUG_ASSERT(lock);

    long e = lock->flags;

    __atomic_clear(&lock->value, __ATOMIC_RELEASE);
    __atomic_clear(&lock->flags, __ATOMIC_RELEASE);

    arch_intr_enable(e);
    
}
