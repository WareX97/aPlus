#ifndef _APLUS_HAL_DEBUG_H
#define _APLUS_HAL_DEBUG_H

#ifndef __ASSEMBLY__
#include <sys/cdefs.h>
#include <aplus.h>
#include <aplus/debug.h>

__BEGIN_DECLS

void arch_debug_init(void);
void arch_debug_putc(char);
void arch_debug_stacktrace(uintptr_t*, size_t);

__END_DECLS

#endif
#endif