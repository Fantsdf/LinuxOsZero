/*
 * LinuxOSZero - Stage 2 C Loader
 */

#include "multiboot.h"
#include "../kernel/kernel.h"

void stage2_entry(void) {
    /* Set up stack and call kernel_main */
    kernel_main();

    /* In standalone mode, loop forever or halt */
    while (1) {
        hlt();
    }
}
