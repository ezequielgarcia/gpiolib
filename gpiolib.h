/*
 * Userspace GPIO output driver for the TI AM335x chip family
 *
 * GNU GPL?
 */

#ifndef __GPIO_LIB_H
#define __GPIO_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Init function for the library, must be called before any other */
int gpio_init(unsigned bank, unsigned pin);

/* Teardown function */
int gpio_finish();

/* Set gpio value to low/high */
int gpio_low();
int gpio_high();

#ifdef __cplusplus
}
#endif

#endif

