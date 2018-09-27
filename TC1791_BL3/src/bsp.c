/*
 * uart.c
 *
 *  Created on: 2018/07/13
 *      Author: wangj
 */

#include <stdlib.h>
#include <TC1791.h>
#include <machine/wdtcon.h>
#include "bsp.h"
#include "flash.h"


const unsigned long crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/*
 * set_cpu_frequency
 *
 *
 * set the frequency of the mcu
 */
void set_cpu_frequency(void)
{
	SCU_PLLCON0_t_nonv pllcon0;
	SCU_PLLCON1_t_nonv pllcon1;

	/* check whether we are already running at desired clockrate */
	pllcon0 = SCU_PLLCON0;
	pllcon1 = SCU_PLLCON1;
	if (((SYS_CFG_NDIV - 1)  == pllcon0.bits.NDIV) &&
		((SYS_CFG_PDIV - 1)  == pllcon0.bits.PDIV) &&
		((SYS_CFG_K1DIV - 1) == pllcon1.bits.K1DIV) &&
		((SYS_CFG_K2DIV - 1) == pllcon1.bits.K2DIV) &&
		SCU_PLLSTAT.bits.VCOLOCK)
		return;

	if (!SCU_PLLSTAT.bits.PWDSTAT) {
		/* set speed to selected value */
		pllcon0.reg = 0;
		pllcon1.reg = 0;
		pllcon0.bits.NDIV  = SYS_CFG_NDIV - 1;
		pllcon0.bits.PDIV  = SYS_CFG_PDIV - 1;
		pllcon1.bits.K2DIV = SYS_CFG_K2DIV - 1;
		pllcon1.bits.K1DIV = SYS_CFG_K1DIV - 1;
		pllcon0.bits.VCOBYP = 1;
		pllcon0.bits.CLRFINDIS = 1;

		pllcon0.bits.RESLD = 1;

		unlock_wdtcon();
		/* FPI at half CPU speed */
		SCU_CCUCON0.bits.FPIDIV = SYS_CFG_FPIDIV;

		/* force prescaler mode */
		SCU_PLLCON0.bits.VCOBYP = 1;

		/* wait for prescaler mode */
		while (!SCU_PLLSTAT.bits.VCOBYST);

		/* write new control values */
		SCU_PLLCON1 = pllcon1;
		SCU_PLLCON0 = pllcon0;
		lock_wdtcon();

		/* wait for stable VCO frequency */
		while (!SCU_PLLSTAT.bits.VCOLOCK);

		unlock_wdtcon();
		/* leave prescaler mode */
		SCU_PLLCON0.bits.VCOBYP = 0;
		lock_wdtcon();
	}
}

/*
 * get_cpu_frequency
 *
 *
 * get the frequency of the mcu
 */

unsigned int get_cpu_frequency(void)
{
	unsigned int frequency;
	unsigned int fpidiv;

	SCU_PLLCON0_t_nonv pllcon0;
	SCU_PLLCON1_t_nonv pllcon1;
	SCU_PLLSTAT_t_nonv pllstat;

	pllcon0 = SCU_PLLCON0;
	pllcon1 = SCU_PLLCON1;
	pllstat = SCU_PLLSTAT;

	/* read FPI divider value */
	fpidiv = SCU_CCUCON0.bits.FPIDIV;

	if (pllstat.bits.VCOBYST)
	{
		/* prescaler mode */
		unsigned int k_div;

		k_div = pllcon1.bits.K1DIV + 1;
		frequency = DEF_FRQ / k_div;
	}
	else if (pllstat.bits.FINDIS)
	{
		/* freerunning mode */
		unsigned int k_div;

		k_div = pllcon1.bits.K2DIV + 1;
		frequency = VCOBASE_FREQ / k_div;
	}
	else
	{
		/* normal mode */
		unsigned int k_div, n_div, p_div;

		n_div = pllcon0.bits.NDIV + 1;
		p_div = pllcon0.bits.PDIV + 1;
		k_div = pllcon1.bits.K2DIV + 1;

		frequency = DEF_FRQ * n_div / (k_div * p_div);
	}

	frequency /= (fpidiv + 1);

	return frequency;
}

/*
 * asc0_init
 *
 *
 * init asc0, the same settings(baudrate stop bits data bits ...) with loader2
 */
int asc0_init(unsigned int baudrate)
{
	unsigned int frequency, reload_value, fdv;
	unsigned int dfreq;

	/* set cpu frequency to 108MHz */
	set_cpu_frequency();

	/* Compute system frequency and reload value for ASC0 */
	frequency = get_cpu_frequency();

	/*  reload_value = fdv/512 * freq/16/baudrate -1  ==>
		reload_value = (512*freq)/(baudrate * 512*16) - 1
		fdv = (reload_value + 1) * (baudrate*512*16/freq)
	*/
	if (frequency < baudrate * 16)
		return -1;

	reload_value = (frequency / (baudrate * 16)) - 1;
	dfreq = frequency / (16 * 512);
	fdv = (reload_value + 1) * (unsigned int)baudrate / dfreq;

	/* Enable ASC0 */
	unlock_wdtcon();
	ASC0_CLC.bits.RMC = 1;
	ASC0_CLC.bits.DISR = 0;
	lock_wdtcon();

	/* Program ASCn */
	ASC0_CON.reg = 0;
	ASC0_BG.reg  = reload_value;
	ASC0_FDV.reg = fdv;

	ASC0_CON.bits.M = 1;
	ASC0_CON.bits.R = 1;
	ASC0_CON.bits.REN = 1;
	ASC0_CON.bits.FDE = 1;

	ASC0_TBSRC.bits.SETR = 1;

	return 0;
}

/*
 * send_byte
 *
 *
 * send byte via asc0
 */
void send_byte(unsigned char data)
{
	while (!(ASC0_TBSRC.bits.SRR != 0));
	ASC0_TBSRC.bits.CLRR = 1;
	ASC0_TBUF.reg = data;
	while (!(ASC0_TSRC.bits.SRR != 0));
	ASC0_TSRC.bits.CLRR = 1;

}

/*
 * send_long
 *
 *
 * send unsigned long data via asc0
 */
void send_long(unsigned long data)
{
	send_byte(data & 0xFF);
	send_byte((data >> 8) & 0xFF);
	send_byte((data >> 16) & 0xFF);
	send_byte((data >> 24) & 0xFF);
}

/*
 * recv_byte
 *
 *
 * recv byte from asc0
 */
unsigned char recv_byte(void)
{
	unsigned char data = 0;

	while (!(ASC0_RSRC.bits.SRR != 0));
	ASC0_RSRC.bits.CLRR = 1;
	data = ASC0_RBUF.reg;

#if 0
	send_byte(data);
#endif

	return data;
}

/*
 * crc32
 *
 *
 * caculate crc32 of the recv data
 */
unsigned long crc32(const unsigned char *data, size_t len)
{
	unsigned long crc;
	const unsigned char *ptr;
	size_t i;

	crc = 0xFFFFFFFFul;
	ptr = data;

	if ( ptr != NULL )
		for (i = 0; i < len; i++) {
			crc = (crc >> 8) ^ crc32_tab[ (crc ^ (unsigned long) *ptr++) & 0x000000FFul ];
		}

	return (crc ^ 0xFFFFFFFFul);

}

/*
 * recv_boots
 *
 *
 * recv command from host
 */
int recv_commands(struct boot_s *boot)
{
	int ret = 0;
	size_t len = 0;
	unsigned long crc32_recv = 0;

	volatile unsigned char *data = (unsigned char *)boot;

	for (size_t i = 0; i < (sizeof(struct boot_s) - BUF_SIZE); i++) {
		*data++ = recv_byte();
	}

	len = boot->len[0] | (boot->len[1] << 8);
	if (len > BUF_SIZE)
		return -1;

	for (size_t i = 0; i < len; i++) {
		*data++ = recv_byte();
	}

	if (len > 0) {
		crc32_recv = boot->crc32[0] | ( boot->crc32[1] << 8) | ( boot->crc32[2] << 16) | ( boot->crc32[3] << 24);

		if (crc32_recv != crc32(boot->data, len))
			return -1;
	}

	return ret;

}




