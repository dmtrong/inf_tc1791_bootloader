/**************************************************************************
** 																		  *
**  INFINEON TriCore Bootloader Loader, Loader 3, Version 1.0             *
**																		  *
**  Id: main.c 2018-07-12 09:02:32 John Wang                              *
**                                                                        *
**  DESCRIPTION :                                                         *
**      -Communication with HOST via ASC0                                 *
**      -Implementation of flash functions:         			  *
**		  -Erase flash                                            *
**		  -Program flash					  *
**		  -Verify flash						  *
**		  -Protect flash					  *
**  							                  *
**  REMARKS :                                                             *
**    -Stack and CSA initialized        				  *
**    -Interrupt and Traptable not initialized                            *
**  	                                                                  *
**  TODO: if protected read/write/erase operations will get no response   *
**  							                  *
**************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <TC1791.h>
#include <machine/wdtcon.h>
#include "bsp.h"
#include "flash.h"

int main(void)
{
	struct boot_s boot;
	enum BOOT_STATUS_E status = OP_SUCCESS;
	unsigned int baudrate = 500000;
	int ret = 0;

	while (true) {
		memset(&boot, 0x00, sizeof(struct boot_s));
		ret = recv_commands(&boot);
		status = ret < 0 ? RECV_ERR : OP_SUCCESS;
		if (ret < 0) {
			send_byte(status);
			continue;
		}

		switch (boot.command[0]) {
		case INIT:
			ret = flash_handler.init(boot);
			status = ret < 0 ? INIT_ERR : OP_SUCCESS;
			send_byte(status);
			break;

		case SETBAUDRATE:
			status = OP_SUCCESS;
			send_byte(status);
			baudrate = boot.data[0] | boot.data[1] << 8 | boot.data[2] << 16 | boot.data[3] << 24;
			asc0_init(baudrate);
			break;

		case IDCHECK:
			ret = flash_handler.id_check(boot);
			status = ret < 0 ? IDCHECK_ERR : OP_SUCCESS;
			send_byte(status);
			break;

		case BLANKCHECK:
			ret = flash_handler.blank_check(boot);
			status = ret < 0 ? BLANKCHECK_ERR : OP_SUCCESS;
			send_byte(status);
			break;

		case ERASE:
			ret = flash_handler.erase(boot);
			status = ret < 0 ? ERASE_ERR : OP_SUCCESS;
			send_byte(status);
			break;

		case PROGRAM:
			ret = flash_handler.program(boot);
			status = ret < 0 ? PROGRAM_ERR : OP_SUCCESS;
			send_byte(status);
			break;

		case VERIFY:
			ret = flash_handler.verify(boot);
			status = ret < 0 ? VERIFY_ERR : OP_SUCCESS;
			send_byte(status);
			break;

		case VERIFY_CRC32:
			ret = flash_handler.verify_crc32(boot);
			status = ret < 0 ? VERIFY_CRC32_ERR : OP_SUCCESS;
			send_byte(status);
			break;

		case READ:
			ret = flash_handler.read(&boot);
			status = ret < 0 ? READ_ERR : OP_SUCCESS;
			//send_byte(status);                 //do not send status, because uart fpga buffer size is 1KB only
			break;

		case SECURE:
			ret = flash_handler.secure(boot);
			status = ret < 0 ? SECURE_ERR : OP_SUCCESS;
			send_byte(status);
			break;

		default:
			status = RECV_ERR;
			send_byte(status);
			break;
		}
	}

	return 0;
}
