#ifndef CONFIG_H
#define CONFIG_H

/* ==============================================================================================
 */

#ifndef F_CPU
#error "F_CPU must be defined by the application, e.g. -DF_CPU=16000000UL " \
       "(see drivers/atmegaxx0_1/CMakeLists.txt)"
#endif

/* ==============================================================================================
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ==============================================================================================
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

/* ==============================================================================================
 */

#include "../../../libraries/embedded-hal/inc/embedded_hal.h"
#include "../../../libraries/inc/errno.h"

/* ==============================================================================================
 */

#endif  // CONFIG_H
