/*
 * flash.c
 *
 *  Created on: 2018/07/17
 *      Author: wangj
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tc1791.h>
#include <machine/wdtcon.h>
#include "bsp.h"
#include "flash.h"

static struct sber_s sber_info;

/*
 * check_wordbook
 *
 *
 * get data form word book
 */
static int wordbook_check(unsigned long key, struct wordbook_s *books, size_t size)
{
	unsigned long which = 0;

	for (which = 0; which < size; which++)  {
		if (books[which].key == key)
			break;
	}

	if (which >= size)
		return -1;

	return which;
}

/*
 * boot_arg
 *
 *
 * get boot arguments
 */
static void boot_arg(struct boot_s boot, unsigned long *base_addr, unsigned long *start_addr, unsigned long *end_addr)
{
	for (int i = 0; i < 4; i++) {
		*base_addr += boot.base_addr[i] << ( i * 8);
	}

	for (int i = 0; i < 4; i++) {
		*start_addr += boot.start_addr[i] << ( i * 8);
	}

	for (int i = 0; i < 4; i++) {
		*end_addr += boot.end_addr[i] << ( i * 8);
	}
}

/*
 * check_proction
 *
 *
 * check protection in fsr
 */
static int check_protection(const FLASHn_FSR_t *flash_fsr)
{
	int ret = 0;

	/* check protection */
	if (flash_fsr->bits.PROIN != 0)
		return -1;

	/* check read write protections */
	if (((flash_fsr->bits.RPROIN != 0) && (flash_fsr->bits.RPRODIS == 0)) ||
		((flash_fsr->bits.WPROIN0 != 0) && (flash_fsr->bits.WPRODIS0 == 0)) ||
		((flash_fsr->bits.WPROIN1 != 0) && (flash_fsr->bits.WPRODIS1 == 0)) ||
		(flash_fsr->bits.WPROIN2 != 0))
		return -1;

	return ret;
}

/*
 * disable_flash_protection
 *
 *
 * disable write/read protection
 */

static int disable_flash_protection(enum FLASH_PRO_E pro_type, const FLASHn_FSR_t *flash_fsr, unsigned long base_addr, const unsigned char *data)
{
	int ret = 0;
	unsigned long psw0 = 0;
	unsigned long psw1 = 0;
	unsigned long ucbx = 0;

	for (int i = 0; i < 4; i++) {
		psw0 |= (data[i] << i);
		psw1 |= (data[i + 4] << i);
	}

	if (pro_type == READ_PRO)
		ucbx = 0x00;
	else if (pro_type == WRITE_PRO)
		ucbx = data[8];
	else
		return -1;

	/* clear FSR */
	(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00F5;

	/* remove write protections */
	(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00AA;
	(*(volatile unsigned long *) (base_addr + 0xAAA8)) = 0x0055;
	(*(volatile unsigned long *) (base_addr + 0x553C)) = ucbx;
	(*(volatile unsigned long *) (base_addr + 0xAAA8)) = psw0;
	(*(volatile unsigned long *) (base_addr + 0xAAA8)) = psw1;
	(*(volatile unsigned long *) (base_addr + 0x5558)) = pro_type;

	__asm("nop");
	__asm("nop");
	__asm("nop");

	if ((flash_fsr->bits.PROER != 0) || (flash_fsr->bits.VER != 0) || (flash_fsr->bits.PFOPER != 0)
		|| (flash_fsr->bits.DFOPER != 0) || (flash_fsr->bits.SQER != 0)) {
		ret = -1;
	}

	return ret;

}

/*
 * flash_init
 *
 *
 * init MARP/MARD
 */
static int flash_init(struct boot_s boot)
{
	if ((boot.data[0] != DEFAULT) && (boot.data[0] != TIGHT0) && (boot.data[0] != TIGHT1))
		return -1;

	unlock_wdtcon();
	FLASH0_MARP.reg = 0x8000 | boot.data[0];
	lock_wdtcon();

	unlock_wdtcon();
	FLASH1_MARP.reg = 0x8000 | boot.data[0];
	lock_wdtcon();

	unlock_wdtcon();
	FLASH0_MARD.reg = 0x8000 | boot.data[0];
	lock_wdtcon();

	unlock_wdtcon();
	FLASH1_MARD.reg = 0x8000 | boot.data[0];
	lock_wdtcon();

	if ((FLASH0_MARP.bits.TRAPDIS != 1) || (FLASH1_MARP.bits.TRAPDIS != 1) ||
			(FLASH0_MARD.bits.TRAPDIS != 1) || (FLASH1_MARD.bits.TRAPDIS != 1))
		return -1;

	sber_info.sber = 0;
	sber_info.sber_max = boot.data[1];

	return 0;
}

/*
 * check_bit_error
 *
 *
 * check bit error in fsr
 */
static int check_bit_error(struct boot_s boot)
{
	unsigned long base_addr = 0;
	unsigned long start_addr = 0;
	unsigned long end_addr = 0;
	volatile FLASHn_FSR_t *flash_fsr = NULL;
	struct wordbook_s word_books[4] = {{PFLASHPART0, FLASH0_FSR_ADDR, {0x00, 0x00, PFLASH_PAGE_SIZE}},
										{PFLASHPART1, FLASH1_FSR_ADDR, {0x00, 0x00, PFLASH_PAGE_SIZE}},
										{DFLASHPART0, FLASH0_FSR_ADDR, {0x00, 0x00, DFLASH_PAGE_SIZE}},
										{DFLASHPART1, FLASH0_FSR_ADDR, {0x00, 0x00, DFLASH_PAGE_SIZE}}};
	int which = 0;
	int ret = 0;

	boot_arg(boot, &base_addr, &start_addr, &end_addr);

	which = wordbook_check(boot.command[1], word_books, 4);
	if (which < 0)
		return -1;

	flash_fsr = (volatile FLASHn_FSR_t *)word_books[which].fsr_addr;

	/* check signal bit error */
	if ((flash_fsr->bits.PFSBER != 0) || (flash_fsr->bits.DFCBER != 0))
		sber_info.sber++;

	if (sber_info.sber > sber_info.sber_max)
		return -1;
	else
		/* clear FSR */
		(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00F5;

	/* check double or multi bit error */
	if ((flash_fsr->bits.PFDBER != 0) || (flash_fsr->bits.DFMBER != 0))
		return -1;

	return ret;
}

/*
 * flash_idcheck
 *
 *
 * check id for flash
 */
static int flash_idcheck(struct boot_s boot)
{
	unsigned short manid = 0;
	unsigned short chipid = 0;
	int ret = 0;

	manid = SCU_MANID.reg;
	chipid = SCU_CHIPID.bits.CHID;

	if ((manid != (boot.data[0] | (boot.data[1] << 8))) || (chipid != boot.data[3]))
		return -1;

	return ret;
}

/*
 * flash_write
 *
 * len check
 * write flash
 */
static int flash_write(struct boot_s boot)
{
	unsigned long base_addr = 0;
	unsigned long start_addr = 0;
	unsigned long end_addr = 0;
	unsigned long long data = 0;
	unsigned long enter_page = 0;
	unsigned int offset = 0;
	volatile FLASHn_FSR_t *flash_fsr = NULL;
	struct wordbook_s word_books[7] = {{PFLASHPART0, FLASH0_FSR_ADDR, {0x0050, 0xA0, PFLASH_PAGE_SIZE}},
										{PFLASHPART1, FLASH1_FSR_ADDR, {0x0050, 0xA0, PFLASH_PAGE_SIZE}},
										{DFLASHPART0, FLASH0_FSR_ADDR, {0x005D, 0xA0, DFLASH_PAGE_SIZE}},
										{DFLASHPART1, FLASH0_FSR_ADDR, {0x005D, 0xA0, DFLASH_PAGE_SIZE}},
										{UCBFLASHPART0, FLASH0_FSR_ADDR, {0x0050, 0xC0, UCFLASH_PAGE_SIZE}},
										{UCBFLASHPART1, FLASH1_FSR_ADDR, {0x0050, 0xC0, UCFLASH_PAGE_SIZE}}};
	unsigned long page_size = 0;
	unsigned long write_page = 0;
	int which = 0;
	int ret = 0;


	boot_arg(boot, &base_addr, &start_addr, &end_addr);
	if ((end_addr - start_addr + 1) > BUF_SIZE)
		return -1;

	which = wordbook_check(boot.command[1], word_books, 7);
	if (which < 0)
		return -1;

	flash_fsr = (volatile FLASHn_FSR_t *)word_books[which].fsr_addr;
	enter_page = word_books[which].data[0];
	write_page = word_books[which].data[1];
	page_size = word_books[which].data[2];

	for (offset = start_addr; offset < end_addr; offset += page_size) {
		/* clear FSR */
		(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00F5;

		/* enter page mode */
		(*(volatile unsigned long *) (base_addr + 0x5554)) = enter_page;

		/* load page */
		for (unsigned int i = 0; i < page_size; i += 8) {
			data = (unsigned long long)boot.data[offset - start_addr + i] & 0xFF;
			data += (unsigned long long)(boot.data[(offset - start_addr) + i + 1] & 0xFF) << 8;
			data += (unsigned long long)(boot.data[(offset - start_addr) + i + 2] & 0xFF) << 16;
			data += (unsigned long long)(boot.data[(offset - start_addr) + i + 3] & 0xFF) << 24;
			data += (unsigned long long)(boot.data[(offset - start_addr) + i + 4] & 0xFF) << 32;
			data += (unsigned long long)(boot.data[(offset - start_addr) + i + 5] & 0xFF) << 40;
			data += (unsigned long long)(boot.data[(offset - start_addr) + i + 6] & 0xFF) << 48;
			data += (unsigned long long)(boot.data[(offset - start_addr) + i + 7] & 0xFF) << 56;
			(*(unsigned long long*) (base_addr + 0x55F0)) = data;
		}

		/* write page */
		(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00AA;
		(*(volatile unsigned long *) (base_addr + 0xAAA8)) = 0x0055;
		(*(volatile unsigned long *) (base_addr + 0x5554)) = write_page;
		(*(volatile unsigned long *) (offset)) = 0x00AA;

		/* wait fsr valid to check */
		while ((flash_fsr->bits.PBUSY == 0) && (flash_fsr->bits.D0BUSY == 0) && (flash_fsr->bits.D1BUSY == 0));

		/* check FSR status */
		while ((flash_fsr->bits.PBUSY != 0) || (flash_fsr->bits.D0BUSY != 0) || (flash_fsr->bits.D1BUSY != 0));

		/* check error status */
		if ((flash_fsr->bits.VER != 0) || (flash_fsr->bits.PFOPER != 0) || (flash_fsr->bits.DFOPER != 0) || (flash_fsr->bits.SQER != 0))  {
			ret = -1;
			break;
		}
	}

	return ret;
}

/*
 * flash_erase
 *
 *
 * erase flash
 */
static int flash_erase(struct boot_s boot)
{
	unsigned long base_addr = 0;
	unsigned long start_addr = 0;
	unsigned long end_addr = 0;
	unsigned long erase_commnad = 0;
	volatile FLASHn_FSR_t *flash_fsr = NULL;
	struct wordbook_s word_books[7] = {{PFLASHPART0, FLASH0_FSR_ADDR, {0x0030, 0x00, PFLASH_PAGE_SIZE}},
										{PFLASHPART1, FLASH1_FSR_ADDR, {0x0030, 0x00, PFLASH_PAGE_SIZE}},
										{DFLASHPART0, FLASH0_FSR_ADDR, {0x0040, 0x00, DFLASH_PAGE_SIZE}},
										{DFLASHPART1, FLASH0_FSR_ADDR, {0x0040, 0x00, DFLASH_PAGE_SIZE}},
										{UCBFLASHPART0, FLASH0_FSR_ADDR, {0x00C0, 0x00, UCFLASH_PAGE_SIZE}},
										{UCBFLASHPART1, FLASH1_FSR_ADDR, {0x00C0, 0x00, UCFLASH_PAGE_SIZE}}};
	int which = 0;
	int ret = 0;

	boot_arg(boot, &base_addr, &start_addr, &end_addr);
	which = wordbook_check(boot.command[1], word_books, 7);
	if (which < 0)
		return -1;

	flash_fsr = (volatile FLASHn_FSR_t *)word_books[which].fsr_addr;
	erase_commnad = word_books[which].data[0];

	if ((word_books[which].key == UCBFLASHPART0) || (word_books[which].key == UCBFLASHPART1)) {
		if (check_protection(flash_fsr) < 0) {
			ret = disable_flash_protection(READ_PRO, flash_fsr, base_addr, boot.data);
			if (ret < 0)
				return -1;
			ret = disable_flash_protection(WRITE_PRO, flash_fsr, base_addr, boot.data);
			if (ret < 0)
				return -1;
		} else {
			return 0;
		}
	}

	/* clear FSR */
	(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00F5;

	/* flash erase */
	(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00AA;
	(*(volatile unsigned long *) (base_addr + 0xAAA8)) = 0x0055;
	(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x0080;
	(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00AA;
	(*(volatile unsigned long *) (base_addr + 0xAAA8)) = 0x0055;
	(*(volatile unsigned long *) (start_addr)) = erase_commnad;

	/* wait fsr valid to check */
	while ((flash_fsr->bits.PBUSY == 0) && (flash_fsr->bits.D0BUSY == 0) && (flash_fsr->bits.D1BUSY == 0));

	/* check FSR status */
	while ((flash_fsr->bits.PBUSY != 0) || (flash_fsr->bits.D0BUSY != 0) || (flash_fsr->bits.D1BUSY != 0));

	/* check error status */
	if ((flash_fsr->bits.VER != 0) || (flash_fsr->bits.PFOPER != 0) || (flash_fsr->bits.DFOPER != 0) || (flash_fsr->bits.SQER != 0))
		ret = -1;

	return ret;
}

/*
 * flash_compare
 *
 *
 * comare falsh with boot.data
 */

static int flash_compare(struct boot_s boot)
{
	unsigned long base_addr = 0;
	unsigned long start_addr = 0;
	unsigned long end_addr = 0;
	unsigned long data = 0;
	unsigned long expect;
	int ret = 0;

	boot_arg(boot, &base_addr, &start_addr, &end_addr);
	if ((end_addr - start_addr + 1) > BUF_SIZE)
		return -1;

	/* check error bits */
	if (check_bit_error(boot) < 0)
		return -1;

	/* reset to read */
	//(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00F0;

	/* read data */
	for (size_t i = 0; i < BUF_SIZE; i += 4) {
		expect = boot.data[i] | (boot.data[i + 1] << 8) | (boot.data[i + 2] << 16) | (boot.data[i + 3] << 24);
		data = (*(volatile unsigned long *) (start_addr + i));
		if (data != expect) {
			send_byte(VERIFY_ERR);
			send_long(data);
			send_long(expect);
			ret = -1;
			break;
		}
	}


	return ret;
}

/*
 * flash_compare
 *
 *
 * comare falsh with boot.data
 */

static int flash_crc32_compare(struct boot_s boot)
{
	unsigned long base_addr = 0;
	unsigned long start_addr = 0;
	unsigned long end_addr = 0;
	unsigned long data = 0;
	unsigned long crc32_recv = 0;
	int ret = 0;

	boot_arg(boot, &base_addr, &start_addr, &end_addr);
	if ((end_addr - start_addr + 1) > BUF_SIZE)
		return -1;

	/* check error bits */
	if (check_bit_error(boot) < 0)
		return -1;

	/* reset to read */
	//(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00F0;

	crc32_recv = boot.data[0] | ( boot.data[1] << 8) | ( boot.data[2] << 16) | ( boot.data[3] << 24);
	for (size_t i = 0; i < BUF_SIZE; i += 4) {
		data = (*(volatile unsigned long *) (start_addr + i));
		boot.data[i] = data & 0xFF;
		boot.data[i + 1] = (data >> 8) & 0xFF;
		boot.data[i + 2] = (data >> 16) & 0xFF;
		boot.data[i + 3] = (data >> 24) & 0xFF;
	}

	if (crc32_recv != crc32(boot.data, BUF_SIZE))
		return -1;

	return ret;
}

/*
 * flash_compare
 *
 *
 * comare flash with boot.data
 */

static int flash_blank_check(struct boot_s boot)
{

	unsigned long base_addr = 0;
	unsigned long start_addr = 0;
	unsigned long end_addr = 0;
	unsigned long offset = 0;
	unsigned long data = 0;
	int ret = 0;

	boot_arg(boot, &base_addr, &start_addr, &end_addr);

	/* check error bits */
	if (check_bit_error(boot) < 0)
		return -1;

	/* reset to read */
	//(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00F0;

	/* check protection */
	if (check_protection((FLASHn_FSR_t *)FLASH0_FSR_ADDR) < 0) {
		send_byte(BLANKCHECK_PROTECT_ERR);
		return -1;
	}
	if (check_protection((FLASHn_FSR_t *)FLASH1_FSR_ADDR) < 0) {
		send_byte(BLANKCHECK_PROTECT_ERR);
		return -1;
	}

	/* read data */
	for (offset = start_addr; offset < end_addr; offset += BUF_SIZE) {
		for (size_t i = 0; i < BUF_SIZE; i += 4) {
			data = (*(volatile unsigned long *) (offset + i));
			if (data != BLANK_STATE) {
				send_byte(BLANKCHECK_ERR);
				send_long(offset + i);
				send_long(data);
				return -1;
			}
		}
	}

	return ret;

}
/*
 * flash_read
 *
 *
 * read pflash and dflash
 */

static int flash_read(struct boot_s *boot)
{
	unsigned long base_addr = 0;
	unsigned long start_addr = 0;
	unsigned long end_addr = 0;
	unsigned long data = 0;
	int ret = 0;

	boot_arg(*boot, &base_addr, &start_addr, &end_addr);

	/* clear buffer */
	memset(boot->data, 0x00, BUF_SIZE);

	/* check error bits */
	if (check_bit_error(*boot) < 0)
		return -1;

	/* reset to read */
	//(*(volatile unsigned long *) (base_addr + 0x5554)) = 0x00F0;

	/* read data */
	for (size_t i = 0; i < BUF_SIZE; i += 4) {
		data = (*(volatile unsigned long *) (start_addr + i));
		boot->data[i] = data & 0xFF;
		boot->data[i + 1] = (data >> 8) & 0xFF;
		boot->data[i + 2] = (data >> 16) & 0xFF;
		boot->data[i + 3] = (data >> 24) & 0xFF;
	}

	/* send data */
	for (int i = 0; i < BUF_SIZE; i++) {
		send_byte(boot->data[i]);
	}

	return ret;
}

/*
 * flash_secure
 *
 *
 * security for pflash and dflash
 */

static int flash_secure(struct boot_s boot)
{
	unsigned long base_addr = 0;
	unsigned long start_addr = 0;
	unsigned long end_addr = 0;
	int ret = 0;

	boot_arg(boot, &base_addr, &start_addr, &end_addr);

	/* only allow to write UCB page here */
	if ((boot.command[1] != UCBFLASHPART0) && (boot.command[1] != UCBFLASHPART1))
		return -1;

	ret = flash_write(boot);

	return ret;
}

/* export methods */
struct boot_handler_s flash_handler =  {
	.init = flash_init,
	.id_check = flash_idcheck,
	.blank_check = flash_blank_check,
	.program = flash_write,
	.erase = flash_erase,
	.read = flash_read,
	.verify = flash_compare,
	.verify_crc32 = flash_crc32_compare,
	.secure =  flash_secure,
};
