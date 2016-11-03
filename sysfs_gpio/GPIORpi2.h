#ifndef RPI2_PIN_H
#define RPI2_PIN_H

#define RPI2_PIN_3  2
#define RPI2_PIN_5  3
#define RPI2_PIN_7  4
#define RPI2_PIN_8  14
#define RPI2_PIN_10 15
#define RPI2_PIN_11 17
#define RPI2_PIN_12 18
#define RPI2_PIN_13 27
#define RPI2_PIN_15 22
#define RPI2_PIN_16 23
#define RPI2_PIN_18 24
#define RPI2_PIN_19 10
#define RPI2_PIN_21 9
#define RPI2_PIN_22 25
#define RPI2_PIN_23 11
#define RPI2_PIN_24 8
#define RPI2_PIN_26 7


/* RPI 2 pinmap
 *
 *          +--+--+  
 * 3.3 V    |1 |2 |  5 V
 * GPIO_2   |3 |4 |  5 V
 * GPIO_3   |5 |6 |  GDN
 * GPIO_4   |7 |8 |  GPIO_14
 * GND      |9 |10|  GPIO_15
 * GPIO_17  |11|12|  GPIO_18
 * GPIO_27  |13|14|  GND
 * GPIO_22  |15|16|  GPIO_23
 * 3.3 V    |17|18|  GPIO_24
 * GPIO_10  |19|20|  GND
 * GPIO_9   |21|22|  GPIO_25
 * GPIO_11  |23|24|  GPIO_8
 * GND      |25|26|  GPIO_7
 * (I2C)    |27|28|  (I2C)
 * GPIO_5   |29|30|  GDN
 * GPIO_6   |31|32|  GPIO_12
 * GPIO_13  |33|34|  GDN
 * GPIO_19  |35|36|  GPIO_16
 * GPIO_26  |37|38|  GPIO_20
 * GDN      |39|40|  GPIO_21
 *          +--+--+  
 */
#endif
