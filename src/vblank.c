/*----------------------------------------------------------------------------
 *  V_Blank.c -- V-Blank���荞�ݏ��������[�`���T���v��
 *  Copyright(c) 1994 SEGA
 *  Written by K.M on 1994-05-16 Ver.1.00
 *  Updated by K.M on 1994-09-21 Ver.1.00
 *
 *  UsrVblankStart()	�FV-Blank�J�n���荞�ݏ����T���v��
 *  UsrVblankEnd()	�FV-Blank�I�����荞�ݏ����T���v��
 *
 *----------------------------------------------------------------------------
 */

#include "kbox.h"
#include	<machine.h>
#include	<sega_mth.h>
#include	<sega_spr.h>
#include	<sega_scl.h>
#include	<sega_xpt.h>
#include 	<sega_int.h>

void UsrVblankIn(void);
void UsrVblankOut(void);

void SetVblank(void) {	
	/* V-Blank���荞�݃��[�`���̓o�^ */
	INT_ChgMsk(INT_MSK_NULL,INT_MSK_VBLK_IN | INT_MSK_VBLK_OUT);
	INT_SetScuFunc(INT_SCU_VBLK_IN,UsrVblankIn);
	INT_SetScuFunc(INT_SCU_VBLK_OUT,UsrVblankOut);
	INT_ChgMsk(INT_MSK_VBLK_IN | INT_MSK_VBLK_OUT,INT_MSK_NULL);
}


void UsrVblankIn(void) {
	SCL_VblankStart();
}

void UsrVblankOut(void) {
	SCL_VblankEnd();
}
