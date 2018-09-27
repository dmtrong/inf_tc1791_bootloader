/*
 * flash.h
 *
 *  Created on: 2018��7��17��
 *      Author: wangj
 */

#ifndef FLASH_H_
#define FLASH_H_

#define PFLASH_PAGE_SIZE    0x100
#define DFLASH_PAGE_SIZE    0x80
#define UCFLASH_PAGE_SIZE   0x100

#define BUF_SIZE            0x400
#define BLANK_STATE         0x00000000

#define MAGIC               0xDD10

enum BOOT_COMMAND_E {
	SETBAUDRATE = 0xA0,
	INIT = 0x12,
	IDCHECK = 0xF1,
	BLANKCHECK = 0xE2,
	ERASE = 0xD3,
	PROGRAM = 0xC4,
	VERIFY = 0xB5,
	VERIFY_CRC32 = 0xB6,
	SECURE = 0xA6,
	READ = 0x97
};

enum BOOT_STATUS_E {
	BAUDRATE_ERR = 0x0A,
	INIT_ERR = 0x21,
	IDCHECK_ERR = 0x1F,
	BLANKCHECK_ERR = 0x2E,
	BLANKCHECK_PROTECT_ERR = 0x6E,
	ERASE_ERR = 0x3D,
	PROGRAM_ERR = 0x4C,
	VERIFY_ERR = 0x5B,
	VERIFY_CRC32_ERR =0x6B,
	SECURE_ERR = 0x6A,
	READ_ERR = 0x79,
	RECV_ERR = 0x88,
	OP_SUCCESS = 0x06
};

enum FLASH_PART_E {
	PFLASHPART0 = 0x0A,
	DFLASHPART0 = 0x5A,
	PFLASHPART1 = 0x0B,
	DFLASHPART1 = 0x4B,
	UCBFLASHPART0 = 0x0C,
	UCBFLASHPART1 = 0x3C
};

enum FLASH_PRO_E {
	READ_PRO = 0x08,
	WRITE_PRO = 0x05
};

enum FLASH_MARGIN_E {
	DEFAULT = 0x00,
	TIGHT0 = 0x01,
	TIGHT1 = 0x04
};

struct wordbook_s {
	unsigned long key;
	unsigned long fsr_addr;
	unsigned long data[3];
};

struct boot_s {
	unsigned char magic[2];
	unsigned char command[2];
	unsigned char base_addr[4];
	unsigned char start_addr[4];
	unsigned char end_addr[4];
	unsigned char len[2];
	unsigned char crc32[4];
	unsigned char data[BUF_SIZE];
};

struct sber_s {
	unsigned long sber;
	unsigned long sber_max;
};

struct boot_handler_s {
	int (*init)(struct boot_s boot);
	int (*id_check)(struct boot_s boot);
	int (*blank_check)(struct boot_s boot);
	int (*erase)(struct boot_s boot);
	int (*program)(struct boot_s boot);
	int (*verify)(struct boot_s boot);
	int (*verify_crc32)(struct boot_s boot);
	int (*secure)(struct boot_s boot);
	int (*read)(struct boot_s *boot);
};

extern struct boot_handler_s flash_handler;

#endif /* FLASH_H_ */
