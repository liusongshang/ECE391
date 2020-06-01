/* level swich contain the context switch*/

#ifndef _LEVEL_SWITCH_H
#define _LEVEL_SWITCH_H

#ifndef ASM

#include "types.h"

extern void context_switch(int entry);

#endif /* ASM */

#endif /* _x86_DESC_H */
