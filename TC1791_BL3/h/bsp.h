/*
 * bsp.h
 *
 *  Created on: 2018��7��13��
 *      Author: wangj
 */

#ifndef BSP_H_
#define BSP_H_

#include "flash.h"

#ifndef DEF_FRQ
#define DEF_FRQ			20000000U	/* TC1791 quartz frequency is 20 MHz */
#endif /* DEF_FRQ */

#define VCOBASE_FREQ	400000000U	/* ?? */

/* divider values for 108MHz */
#define SYS_CFG_PDIV	 1
#define SYS_CFG_NDIV	54
#define SYS_CFG_K1DIV	 2
#define SYS_CFG_K2DIV	 5
#define SYS_CFG_FPIDIV	 1

/* set frequency 108MHz of the mcu */
void set_cpu_frequency(void);

/* get frequency of the mcu */
unsigned int get_cpu_frequency(void);

/* init asc0 */
int asc0_init(unsigned int baudrate);

/* send byte via asc0 */
void send_byte(unsigned char data);

/* send unsigned long via asc0 */
void send_long(unsigned long data);

/* recv byte from asc0 */
unsigned char recv_byte(void);

/* crc32 */
unsigned long crc32(const unsigned char *data, size_t len);

/* recv commands */
int recv_commands(struct boot_s *boot);

#endif /* UART_H_ */
