#ifndef ATMEGAXX0_1_GPIO_H
#define ATMEGAXX0_1_GPIO_H

/* ========================================================================== */

#include "config.h"

/* ========================================================================== */

/**
 * @brief gpio_ops implementation for the GPIO ports of the
 *        ATmega640/1280/1281/2560/2561 family. Pass &atmegaxx0_1_gpio_ops
 *        to GPIO_DEFINE as the ops_ptr argument.
 *
 * @note  Ports A-G are present on every family member. Ports H, J, K and L
 *        exist only on the 100-pin variants (640/1280/2560). If a pin ID
 *        that maps to an absent port is used, the ops return -EINVAL.
 *
 * @note  gpio_type::GPIO_ANALOG is treated as an input with no pull-up.
 *        Analog channel selection is the ADC driver's responsibility.
 */
extern const struct gpio_ops atmegaxx0_1_gpio_ops;

/* ========================================================================== */

/**
 * @brief GPIO pin IDs. Pass these to GPIO_DEFINE as the pin_id argument.
 *        Layout: id / 8 = port index (A=0 .. L=10, skipping I), id % 8 = pin.
 */
#define GPIO_A0_ID 0
#define GPIO_A1_ID 1
#define GPIO_A2_ID 2
#define GPIO_A3_ID 3
#define GPIO_A4_ID 4
#define GPIO_A5_ID 5
#define GPIO_A6_ID 6
#define GPIO_A7_ID 7
#define GPIO_B0_ID 8
#define GPIO_B1_ID 9
#define GPIO_B2_ID 10
#define GPIO_B3_ID 11
#define GPIO_B4_ID 12
#define GPIO_B5_ID 13
#define GPIO_B6_ID 14
#define GPIO_B7_ID 15
#define GPIO_C0_ID 16
#define GPIO_C1_ID 17
#define GPIO_C2_ID 18
#define GPIO_C3_ID 19
#define GPIO_C4_ID 20
#define GPIO_C5_ID 21
#define GPIO_C6_ID 22
#define GPIO_C7_ID 23
#define GPIO_D0_ID 24
#define GPIO_D1_ID 25
#define GPIO_D2_ID 26
#define GPIO_D3_ID 27
#define GPIO_D4_ID 28
#define GPIO_D5_ID 29
#define GPIO_D6_ID 30
#define GPIO_D7_ID 31
#define GPIO_E0_ID 32
#define GPIO_E1_ID 33
#define GPIO_E2_ID 34
#define GPIO_E3_ID 35
#define GPIO_E4_ID 36
#define GPIO_E5_ID 37
#define GPIO_E6_ID 38
#define GPIO_E7_ID 39
#define GPIO_F0_ID 40
#define GPIO_F1_ID 41
#define GPIO_F2_ID 42
#define GPIO_F3_ID 43
#define GPIO_F4_ID 44
#define GPIO_F5_ID 45
#define GPIO_F6_ID 46
#define GPIO_F7_ID 47
#define GPIO_G0_ID 48
#define GPIO_G1_ID 49
#define GPIO_G2_ID 50
#define GPIO_G3_ID 51
#define GPIO_G4_ID 52
#define GPIO_G5_ID 53
#define GPIO_G6_ID 54
#define GPIO_G7_ID 55
#define GPIO_H0_ID 56
#define GPIO_H1_ID 57
#define GPIO_H2_ID 58
#define GPIO_H3_ID 59
#define GPIO_H4_ID 60
#define GPIO_H5_ID 61
#define GPIO_H6_ID 62
#define GPIO_H7_ID 63
#define GPIO_J0_ID 64
#define GPIO_J1_ID 65
#define GPIO_J2_ID 66
#define GPIO_J3_ID 67
#define GPIO_J4_ID 68
#define GPIO_J5_ID 69
#define GPIO_J6_ID 70
#define GPIO_J7_ID 71
#define GPIO_K0_ID 72
#define GPIO_K1_ID 73
#define GPIO_K2_ID 74
#define GPIO_K3_ID 75
#define GPIO_K4_ID 76
#define GPIO_K5_ID 77
#define GPIO_K6_ID 78
#define GPIO_K7_ID 79
#define GPIO_L0_ID 80
#define GPIO_L1_ID 81
#define GPIO_L2_ID 82
#define GPIO_L3_ID 83
#define GPIO_L4_ID 84
#define GPIO_L5_ID 85
#define GPIO_L6_ID 86
#define GPIO_L7_ID 87

/* ========================================================================== */

#endif  // ATMEGAXX0_1_GPIO_H
