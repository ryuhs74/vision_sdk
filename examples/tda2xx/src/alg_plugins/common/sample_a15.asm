/*---------------------------------------------------------------------------
* File: peformance_unit.asm   
* Author: vinoth: Modified the original SDO code for cortex-A15                           
*---------------------------------------------------------------------------

*---------------------------------------------------------------------------
*                               
* Functions to Enable, Reset & Read the Cycle Count Register (CCNT)
*                               
* CCNT enabled and reset via Performance Monitor Control Register (PMNC)
* The CCNT is divided by 64 (by setting Bit 3 of PMNC to 1)             
* CCNT is checked for overflow by looking at bit 10 of PMNC 
* CCNT Read returns the clock value divided by 64 cycles. 
* To get the actual CPU cycle multiple it with 64
*---------------------------------------------------------------------------*/

	.text


.set CCNT_CP1, C9
.set CCNT_CP2, C13

	.global  ARM_CCNT_Enable	
	.global  ARM_CCNT_Reset
	
	
ARM_CCNT_Enable:
ARM_CCNT_Reset:
	@Write Performance Monitor Control Register
	MRC P15, #0, R0, C9, C12, #0	
	ORR R0, R0, #0xD                          @PMCCNTR counts once every 64 clock cycles
	MCR P15, #0, R0, C9, C12, #0 @Reset Cycle count register
	MOV R1, #0x80000000
	MCR P15, #0, R1, C9, C12, #1	@Enable the cycle count
	BX  LR

	.global  ARM_CCNT_Read	
ARM_CCNT_Read:
	MRC P15, #0, R0, C9, C13, #0
	BX  LR
