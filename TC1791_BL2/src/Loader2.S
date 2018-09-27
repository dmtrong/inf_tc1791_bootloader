/**************************************************************************
** 																		  *
**  INFINEON TriCore Bootloader, Loader2, Version 1.0                     *
**																		  *
**  Id: Loader2.s 2018-07-12 16:22:32 J.W                                 *
**                                                                        *
**  DESCRIPTION :                                                         *
**      -Initialization of the device                                     *
**      -Download n bytes from UART to SPRAM						      *
**		-Jump to download address                                         *
**  							                                          *
**************************************************************************/

#ifndef __TRICORE_NAME__
#ifdef __TC13__
#define __TRICORE_NAME__ 0x13
#else
#define __TRICORE_NAME__ 0x12
#endif
#endif
/* define the Derivate Name as a hexvalue
 * this name is defined in tricore.specs for specific derivate to
 * the derivate number as a hexvalue (e.g. TC1796 => 0x1796
 * This name will be used in the memory.x Memory description to
 * to confirm that the crt0.o and the memory.x will be get from
 * same directory
 */
.global __TRICORE_DERIVATE_NAME__
.type __TRICORE_DERIVATE_NAME__,@object
.set __TRICORE_DERIVATE_NAME__,__TRICORE_NAME__

.section ".startup_code", "awx", @progbits
.global _start
.type _start,@function

/* resiters */
.equ SCU_PLLCON1B, 0xf000051e
.equ SCU_PLLCON0B, 0xf0000518
.equ WDT_CON1,     0xf00005f4
.equ WDT_CON0,     0xf00005f0
.equ ASC0_CON,     0xf0000a10
.equ ASC0_TBUF,    0xf0000a20
.equ ASC0_RBUF,    0xf0000a24
.equ ASC0_RSRC,    0xf0000af5  // 0xf0000aF4 + 1 -> high byte
.equ ASC0_TBSRC,   0xf0000afd  // 0xf0000afc + 1 -> high byte

/* ram address and len */
.equ RAM_START_HI, 0xc000
.equ RAM_START_LO, 0x0080
.equ LOADER_LEN,   0x17FF      // 6KB for next stage bootloader

/* entry point */
_start:
.code32
    /* switch to prescaler mode: */
    /* SCU_PLLCON1.B.K1DIV = 0; */
	lea %a15, SCU_PLLCON1B
	mov %d15, 0x00
	st.b [%a15], %d15

	/* SCU_PLLCON0.B.VCOBYP =1; */
	lea %a15, SCU_PLLCON0B
	mov %d15, 0x01
	st.b [%a15], %d15

	/* disable watchdog: */
	mov %d15, 0x08
	st.w WDT_CON1, %d15

	/* set endinit bit: */
	ld.w %d0, WDT_CON0
	mov.u %d1, 0x7c
	insert %d0,%d0,%d1, 0x1, 0x7
	st.w WDT_CON0, %d0
	insert %d0,%d0, 0x3, 0x0, 0x4
	st.w WDT_CON0, %d0

pre_rcv_loader:
	movh.a %a14, RAM_START_HI      //load base address for
	lea	%a14,[%a14]RAM_START_LO    //tertiary loader into a14
	mov.aa %a14,%a14               //(0xC0000080)
	lea	%a15, ASC0_RSRC            //load address of HIBYTE(ASC0_RSRC) into a15
	//mov.u %d8, 0x00                //clear d8
	mov	%d15, LOADER_LEN           //set loop counter : size of code 0x1320

rcv_loader:                        //loop entry
wait_for_byte:
	ld.bu %d0, [%a15]              //load HIBYTE(ASC0_RSRC) into d0
	jz.t  %d0, 0x05, wait_for_byte //check bit ASC0_RSRC.SRR
	insert %d0, %d0, 0x1, 0x6, 0x1 //set bit ASC0_RSRC.CLRR
	st.b [%a15],%d0
	ld.b %d9, ASC0_RBUF
	st.b [%a14], %d9               //store received byte at address pointed by a14
	ld.b %d10, [%a14],0            //read the stored by again from this address
	//xor %d8, %d10                  //xor (checksum calculation)
	mov.aa %a14, %a14              //increment a14 (address pointer)
	add.a %a14, 0x1
	mov.aa %a14, %a14
	//st.b ASC0_TBUF, %d9            //send data back for debug only
	jned %d15, 0x00, rcv_loader    //reentry loop until counter=0
send_ack:
	lea	%a15, ASC0_TBSRC
	ld.bu %d0, [%a15]              //load HIBYTE(ASC0_RSRC) into d0
	jz.t  %d0, 0x05, send_ack      //check bit ASC0_RSRC.SRR
	insert %d0, %d0, 0x1, 0x6, 0x1 //set bit ASC0_RSRC.CLRR
	mov %d10, 0x55
	st.b ASC0_TBUF, %d10           //send 0x55 acknowledgement
	//st.b ASC0_TBUF, %d8          //send calculated checksum

exec_loader:
	//Execute downloaded code:
	movh.a %a15, RAM_START_HI      //load base address of
	lea	%a15, [%a15]RAM_START_LO   //downloaded code (0xC0000080)
	ji %a15                        //au revoir!





