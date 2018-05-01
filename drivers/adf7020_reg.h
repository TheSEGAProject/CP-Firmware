//! \file adf7020_reg.h
//!
//!	\brief Register definitions for the ADF7020
//!


#ifndef CP_FIRMWARETEMPORARY_DRIVERS_ADF7020_REG_H_
#define CP_FIRMWARETEMPORARY_DRIVERS_ADF7020_REG_H_


/*************************************************************************//**
 * @name ADF7020 register 0 values
 * @{
 *************************************************************************/
#define REG0_ADDRESS			(0x00000000L)

#define	REG0_RX_FRAC_N		(0x00045800L)
#define	REG0_TX_FRAC_N		(0x00047D10L)
#define	REG0_RX_INT_N			(0x02880000L)
#define	REG0_TX_INT_N			(0x02880000L)

#define	REG0_RX				(0x08000000L)
#define	REG0_TX				(0x00000000L)

#define	REG0_PLL_ON						(0x10000000L)

#define	REG0_MUXOUT_REGRDY		(0x00000000L)
#define	REG0_MUXOUT_RDIVID		(0x20000000L)
#define	REG0_MUXOUT_NDIVID		(0x40000000L)
#define	REG0_MUXOUT_DIGLCK		(0x60000000L)
#define	REG0_MUXOUT_ANALCK		(0x80000000L)
#define	REG0_MUXOUT_THREEST		(0xA0000000L)
#define	REG0_MUXOUT_PLLTST		(0xC0000000L)
#define	REG0_MUXOUT_SDTEST		(0xE0000000L)
//! @}

/*************************************************************************//**
 * @name ADF7020 register 1 values
 * @{
 *************************************************************************/
#define REG1_ADDRESS			(0x00000001L)

#define REG1_RCOUNT_1			(0x00000010L)
#define REG1_RCOUNT_2			(0x00000020L)
#define REG1_RCOUNT_3			(0x00000030L)
#define REG1_RCOUNT_4			(0x00000040L)
#define REG1_RCOUNT_5			(0x00000050L)
#define REG1_RCOUNT_6			(0x00000060L)
#define REG1_RCOUNT_7			(0x00000070L)

#define REG1_XTDBL_ON			(0x00000010L)

#define REG1_CLKOUTDIV_OFF	(0x00000000L)
#define REG1_CLKOUTDIV_2		(0x00000100L)
#define REG1_CLKOUTDIV_4		(0x00000200L)
#define REG1_CLKOUTDIV_6		(0x00000300L)
#define REG1_CLKOUTDIV_8		(0x00000400L)
#define REG1_CLKOUTDIV_10		(0x00000500L)
#define REG1_CLKOUTDIV_12		(0x00000600L)
#define REG1_CLKOUTDIV_14		(0x00000700L)
#define REG1_CLKOUTDIV_16		(0x00000800L)
#define REG1_CLKOUTDIV_18		(0x00000900L)
#define REG1_CLKOUTDIV_20		(0x00000A00L)
#define REG1_CLKOUTDIV_22		(0x00000B00L)
#define REG1_CLKOUTDIV_24		(0x00000C00L)
#define REG1_CLKOUTDIV_26		(0x00000D00L)
#define REG1_CLKOUTDIV_28		(0x00000E00L)
#define REG1_CLKOUTDIV_30		(0x00000F00L)

#define REG1_XOSC_EN				(0x00001000L)

#define REG1_VCO_LO					(0x00002000L)

#define REG1_ICP_300uA				(0x00000000L)
#define REG1_ICP_900uA				(0x00004000L)
#define REG1_ICP_1500uA				(0x00008000L)
#define REG1_ICP_2100uA				(0x0000C000L)

#define REG1_VCOBIAS_125uA		(0x00000000L)
#define REG1_VCOBIAS_375uA		(0x00010000L)
#define REG1_VCOBIAS_625uA		(0x00020000L)
#define REG1_VCOBIAS_875uA		(0x00030000L)
#define REG1_VCOBIAS_1125uA		(0x00040000L)
#define REG1_VCOBIAS_1375uA		(0x00050000L)
#define REG1_VCOBIAS_1625uA		(0x00060000L)
#define REG1_VCOBIAS_1875uA		(0x00070000L)
#define REG1_VCOBIAS_2125uA		(0x00080000L)
#define REG1_VCOBIAS_2375uA		(0x00090000L)
#define REG1_VCOBIAS_2625uA		(0x000A0000L)
#define REG1_VCOBIAS_2875uA		(0x000B0000L)
#define REG1_VCOBIAS_3125uA		(0x000C0000L)
#define REG1_VCOBIAS_3375uA		(0x000D0000L)
#define REG1_VCOBIAS_3625uA		(0x000E0000L)
#define REG1_VCOBIAS_3875uA		(0x000F0000L)

#define REG1_VCOADJUST_0			(0x00000000L)
#define REG1_VCOADJUST_1			(0x00100000L)
#define REG1_VCOADJUST_2			(0x00200000L)
#define REG1_VCOADJUST_3			(0x00300000L)

#define REG1_IFBW_100			(0x00000000L)
#define REG1_IFBW_150			(0x00400000L)
#define REG1_IFBW_200			(0x00800000L)
//! @}

/*************************************************************************//**
 * @name ADF7020 register 2 values
 * @{
 *************************************************************************/
#define REG2_ADDRESS			(0x00000002L)

#define REG2_PA_ON				(0x00000010L)
#define REG2_PA_MUTE			(0x00000020L)

#define REG2_MOD_FSK			(0x00000000)
#define REG2_MOD_GFSK			(1*0x00000040L)
#define REG2_MOD_ASK			(2*0x00000040L)
#define REG2_MOD_OOK			(3*0x00000040L)
#define REG2_MOD_GOOK			(7*0x000001C0L)

#define REG2_PA_13dBm			(0x00007E00L)

#define REG2_FSK_MP_PLL 	(0x00000000L)
#define REG2_FSK_MP_B1 		(1*0x00008000L)
#define REG2_FSK_MP_B2 		(2*0x00008000L)
#define REG2_FSK_MP_B3 		(4*0x00008000L)
#define REG2_FSK_MP_B4 		(8*0x00008000L)
#define REG2_FSK_MP_B5 		(16*0x00008000L)
#define REG2_FSK_MP_B6 		(32*0x00008000L)
#define REG2_FSK_MP_B7 		(64*0x00008000L)
#define REG2_FSK_MP_B8 		(128*0x00008000L)
#define REG2_FSK_MP_B9 		(256*0x00008000L)

#define REG2_GFSK_MC_0 		(0*0x01000000L)
#define REG2_GFSK_MC_1 		(1*0x01000000L)
#define REG2_GFSK_MC_2 		(2*0x01000000L)
#define REG2_GFSK_MC_3 		(3*0x01000000L)
#define REG2_GFSK_MC_4 		(4*0x01000000L)
#define REG2_GFSK_MC_5 		(5*0x01000000L)
#define REG2_GFSK_MC_6 		(6*0x01000000L)
#define REG2_GFSK_MC_7 		(7*0x01000000L)

#define REG2_GFSK_IC_16		(0*0x08000000L)
#define REG2_GFSK_IC_32		(1*0x08000000L)
#define REG2_GFSK_IC_64		(2*0x08000000L)
#define REG2_GFSK_IC_128	(3*0x08000000L)

#define REG2_TX_INVRT			(1*0x20000000L)

#define REG2_PA_5uA			(0*0x40000000L)
#define REG2_PA_7uA			(1*0x40000000L)
#define REG2_PA_9uA			(2*0x40000000L)
#define REG2_PA_11uA		(0xC0000000L)
//! @}

/*************************************************************************//**
 * @name ADF7020 register 3 values
 * @{
 *************************************************************************/
#define REG3_ADDRESS			(0x00000003L)

#define REG3_BBOS_CLKDIV_4 		(0*0x00000010L)
#define REG3_BBOS_CLKDIV_8 		(1*0x00000010L)
#define REG3_BBOS_CLKDIV_16 	(2*0x00000010L)

#define REG3_DEMOD_CLKDIV_4 	(0*0x00000040L)
#define REG3_DEMOD_CLKDIV_1 	(1*0x00000040L)
#define REG3_DEMOD_CLKDIV_2 	(2*0x00000040L)
#define REG3_DEMOD_CLKDIV_3 	(3*0x00000040L)

#define REG3_CDR_CLKDIV_B1 		(1*0x00000100L)
#define REG3_CDR_CLKDIV_B2 		(2*0x00000100L)
#define REG3_CDR_CLKDIV_B3 		(4*0x00000100L)
#define REG3_CDR_CLKDIV_B4 		(8*0x00000100L)
#define REG3_CDR_CLKDIV_B5 		(16*0x00000100L)
#define REG3_CDR_CLKDIV_B6 		(32*0x00000100L)
#define REG3_CDR_CLKDIV_B7 		(64*0x00000100L)
#define REG3_CDR_CLKDIV_B8 		(128*0x00000100L)

#define REG3_SEQ_CLKDIV_B1 		(1*0x00010000L)
#define REG3_SEQ_CLKDIV_B2 		(2*0x00010000L)
#define REG3_SEQ_CLKDIV_B3 		(4*0x00010000L)
#define REG3_SEQ_CLKDIV_B4 		(8*0x00010000L)
#define REG3_SEQ_CLKDIV_B5 		(16*0x00010000L)
#define REG3_SEQ_CLKDIV_B6 		(32*0x00010000L)
#define REG3_SEQ_CLKDIV_B7 		(64*0x00010000L)
#define REG3_SEQ_CLKDIV_B8 		(128*0x00010000L)

//! @}

/*************************************************************************//**
 * @name ADF7020 register 4 values
 * @{
 *************************************************************************/
#define REG4_ADDRESS			(0x00000004L)

#define REG4_DEMODSEL_LIN			(0*0x00000010L)
#define REG4_DEMODSEL_CORR		(1*0x00000010L)
#define REG4_DEMODSEL_ASKOOK	(2*0x00000010L)
#define REG4_DEMODSEL_INVLD		(3*0x00000010L)

#define REG4_PSTDEMODBW_B1		(1*0x00000040L)
#define REG4_PSTDEMODBW_B2		(2*0x00000040L)
#define REG4_PSTDEMODBW_B3		(4*0x00000040L)
#define REG4_PSTDEMODBW_B4		(8*0x00000040L)
#define REG4_PSTDEMODBW_B5		(16*0x00000040L)
#define REG4_PSTDEMODBW_B6		(32*0x00000040L)
#define REG4_PSTDEMODBW_B7		(64*0x00000040L)
#define REG4_PSTDEMODBW_B8		(128*0x00000040L)
#define REG4_PSTDEMODBW_B9		(256*0x00000040L)
#define REG4_PSTDEMODBW_B10		(512*0x00000040L)

#define REG4_DEMODLOCK_B1			(1*0x00010000L)
#define REG4_DEMODLOCK_B2			(2*0x00010000L)
#define REG4_DEMODLOCK_B3			(4*0x00010000L)
#define REG4_DEMODLOCK_B4			(8*0x00010000L)
#define REG4_DEMODLOCK_B5			(16*0x00010000L)
#define REG4_DEMODLOCK_B6			(32*0x00010000L)
#define REG4_DEMODLOCK_B7			(64*0x00010000L)
#define REG4_DEMODLOCK_B8			(128*0x00010000L)

#define REG4_SPC_FREE				(0*0x00800000L)
#define REG4_SPC_LOCK				(1*0x00800000L)
#define REG4_SWD_FREE				(2*0x00800000L)
#define REG4_SWD_LOCK				(3*0x00800000L)
#define REG4_INT_LOCK				(4*0x00800000L)
#define REG4_BITCOUNT_LOCK	(6*0x00800000L)

//! @}

/*************************************************************************//**
 * @name ADF7020 register 5 values
 * @{
 *************************************************************************/
#define REG5_ADDRESS			(0x00000005L)

#define REG5_SYNCLEN_12		(0*0x00000010L)
#define REG5_SYNCLEN_16		(1*0x00000010L)
#define REG5_SYNCLEN_20		(2*0x00000010L)
#define REG5_SYNCLEN_24		(3*0x00000010L)

#define REG5_SYNCERR_0		(0*0x00000040L)
#define REG5_SYNCERR_1		(1*0x00000040L)
#define REG5_SYNCERR_2		(2*0x00000040L)
#define REG5_SYNCERR_3		(3*0x00000040L)

#define REG5_SYNCWORD			(0x123456*0x00000100L)

#define REG5_DEFAULT			(REG5_SYNCBYTE | REG5_SYNCERR_1 | REG5_SYNCLEN_24) | REG5_ADDRESS)
//! @}

/*************************************************************************//**
 * @name ADF7020 register 6 values
 * @{
 *************************************************************************/
#define REG6_ADDRESS			(0x00000006L)

#define REG6_DISCBW_B1		(1*0x00000010L)
#define REG6_DISCBW_B2		(2*0x00000010L)
#define REG6_DISCBW_B3		(4*0x00000010L)
#define REG6_DISCBW_B4		(8*0x00000010L)
#define REG6_DISCBW_B5		(16*0x00000010L)
#define REG6_DISCBW_B6		(32*0x00000010L)
#define REG6_DISCBW_B7		(64*0x00000010L)
#define REG6_DISCBW_B8		(128*0x00000010L)
#define REG6_DISCBW_B9		(256*0x00000010L)
#define REG6_DISCBW_B10		(512*0x00000010L)

#define REG6_DOTPROD			(1*0x00004000L)

#define REG6_LNA_LOGAIN		(1*0x00008000L)

#define REG6_LNA_800uA		(0*0x00010000L)

#define REG6_MIXLIN_HI		(1*0x00040000L)

#define REG6_IF_CAL				(1*0x00080000L)

#define REG6_IFDIV_B1		(1*0x00100000L)
#define REG6_IFDIV_B2		(2*0x00100000L)
#define REG6_IFDIV_B3		(4*0x00100000L)
#define REG6_IFDIV_B4		(8*0x00100000L)
#define REG6_IFDIV_B5		(16*0x00100000L)
#define REG6_IFDIV_B6		(32*0x00100000L)
#define REG6_IFDIV_B7		(64*0x00100000L)
#define REG6_IFDIV_B8		(128*0x0100000L)

#define REG6_RX_INVRT		(2*0x10000000L)

#define REG6_DEMOD_RST	(1*0x4000000L)
#define REG6_CDR_RST		(1*0x4000000L)

//! @}

/*************************************************************************//**
 * @name ADF7020 register 7 values
 * @{
 *************************************************************************/
#define REG7_ADDRESS			(0x00000007L)

#define REG7_ADCMODE_RSSI	(0*0x00000010L)
#define REG7_ADCMODE_VBAT	(1*0x00000010L)
#define REG7_ADCMODE_TEMP	(2*0x00000010L)
#define REG7_ADCMODE_EXT	(3*0x00000010L)

#define REG7_RDBCKSEL_AFC		(0*0x00000040L)
#define REG7_RDBCKSEL_ADC		(1*0x00000040L)
#define REG7_RDBCKSEL_FIL		(2*0x00000040L)
#define REG7_RDBCKSEL_SIL		(3*0x00000040L)

#define REG7_RDBCK_EN		(1*0x00000100L)
//! @}

/*************************************************************************//**
 * @name ADF7020 register 8 values
 * @{
 *************************************************************************/
#define REG8_ADDRESS			(0x00000008L)

#define REG8_SYNTH_EN			(1*0x00000010L)

#define REG8_VCO_EN				(2*0x00000010L)

#define REG8_LNAMIX_EN		(4*0x00000010L)

#define REG8_FILTER_EN		(8*0x00000010L)

#define REG8_DEMOD_EN			(1*0x00000100L)

#define REG8_RSSI_EN			(1*0x00000400L)

#define REG8_TXRXSWTCH_EN	(1*0x00001000L)

#define REG8_PA_EN				(1*0x00002000L)

//! @}

/*************************************************************************//**
 * @name ADF7020 register 9 values
 * @{
 *************************************************************************/
#define REG9_ADDRESS			(0x00000009L)

#define REG9_GL_B1			(1*0x00000010L)
#define REG9_GL_B2			(2*0x00000010L)
#define REG9_GL_B3			(4*0x00000010L)
#define REG9_GL_B4			(8*0x00000010L)
#define REG9_GL_B5			(16*0x00000010L)
#define REG9_GL_B6			(32*0x00000010L)
#define REG9_GL_B7			(64*0x00000010L)

#define REG9_GH_B1			(1*0x00000800L)
#define REG9_GH_B2			(2*0x00000800L)
#define REG9_GH_B3			(4*0x00000800L)
#define REG9_GH_B4			(8*0x00000800L)
#define REG9_GH_B5			(16*0x00000800L)
#define REG9_GH_B6			(32*0x00000800L)
#define REG9_GH_B7			(64*0x00000800L)

#define REG9_GS_HOLD		(1*0x00040000L)

#define REG9_GC_USER		(1*0x00080000L)

#define REG9_LNA_GAIN_1			(0*0x00100000L)
#define REG9_LNA_GAIN_3			(1*0x00100000L)
#define REG9_LNA_GAIN_10		(2*0x00100000L)
#define REG9_LNA_GAIN_30		(3*0x00100000L)

#define REG9_FIL_GAIN_8			(0*0x00400000L)
#define REG9_FIL_GAIN_24		(1*0x00400000L)
#define REG9_FIL_GAIN_72		(2*0x00400000L)
#define REG9_FIL_GAIN_INVLD	(3*0x00400000L)

#define REG9_FI_HI		(1*0x01000000L)

//! @}

/*************************************************************************//**
 * @name ADF7020 register A values
 * @{
 *************************************************************************/
#define REGA_ADDRESS			(0x0000000AL)

#define REGA_PR_DEFAULT		(2*0x00000010L)

#define REGA_GL_DEFAULT		(10*0x00000100L)

#define REGA_DH_DEFAULT		(10*0x00001000L)

#define REGA_GC_B1		(1*0x00010000L)
#define REGA_GC_B2		(2*0x00010000L)
#define REGA_GC_B3		(4*0x00010000L)
#define REGA_GC_B4		(8*0x00010000L)
#define REGA_GC_B5		(16*0x00010000L)

#define REGA_ATTEN		(1*0x00200000L)

#define REGA_GAIN_Q		(1*0x00400000L)

#define REGA_PHASEADJ_B1		(1*0x01000000L)
#define REGA_PHASEADJ_B2		(2*0x01000000L)
#define REGA_PHASEADJ_B3		(4*0x01000000L)
#define REGA_PHASEADJ_B4		(8*0x01000000L)

#define REGA_SEL_Q		(1*0x10000000L)
//! @}

/*************************************************************************//**
 * @name ADF7020 register B values
 * @{
 *************************************************************************/
#define REGB_ADDRESS			(0x0000000BL)

#define REGB_AFCSCALE_B1		(1*0x00000010L)
#define REGB_AFCSCALE_B2		(2*0x00000010L)
#define REGB_AFCSCALE_B3		(4*0x00000010L)
#define REGB_AFCSCALE_B4		(8*0x00000010L)
#define REGB_AFCSCALE_B5		(1*0x00000100L)
#define REGB_AFCSCALE_B6		(2*0x00000100L)
#define REGB_AFCSCALE_B7		(4*0x00000100L)
#define REGB_AFCSCALE_B8		(8*0x00000100L)
#define REGB_AFCSCALE_B9		(1*0x00001000L)
#define REGB_AFCSCALE_B10		(2*0x00001000L)
#define REGB_AFCSCALE_B11		(4*0x00001000L)
#define REGB_AFCSCALE_B12		(8*0x00001000L)
#define REGB_AFCSCALE_B13		(1*0x00010000L)
#define REGB_AFCSCALE_B14		(2*0x00010000L)
#define REGB_AFCSCALE_B15		(4*0x00010000L)
#define REGB_AFCSCALE_B16		(8*0x00010000L)

#define REGB_AFC_ON			(1*0x00100000L)
//! @}

/*************************************************************************//**
 * @name ADF7020 register C values
 * @{
 *************************************************************************/
#define REGC_ADDRESS			(0x0000000CL)

#define REGC_PLLTEST_B1		(1*0x00000010L)
#define REGC_PLLTEST_B2		(2*0x00000010L)
#define REGC_PLLTEST_B3		(4*0x00000010L)
#define REGC_PLLTEST_B4		(8*0x00000010L)
#define REGC_PLLTEST_B5		(8*0x00000010L)

#define REGC_SDTEST_B1		(1*0x00000200L)
#define REGC_SDTEST_B2		(2*0x00000200L)
#define REGC_SDTEST_B3		(4*0x00000200L)
#define REGC_SDTEST_B4		(8*0x00000200L)

#define REGC_CNTRST		(1*0x00002000L)

#define REGC_DTEST_B1		(1*0x00004000L)
#define REGC_DTEST_B2		(2*0x00004000L)
#define REGC_DTEST_B3		(4*0x00004000L)
#define REGC_DTEST_B4		(8*0x00004000L)

#define REGC_FILCAL_B1		(1*0x00040000L)
#define REGC_FILCAL_B2		(2*0x00040000L)
#define REGC_FILCAL_B3		(4*0x00040000L)
#define REGC_FILCAL_B4		(8*0x00040000L)
#define REGC_FILCAL_B5		(16*0x00400000L)
#define REGC_FILCAL_B6		(32*0x00400000L)

#define REGC_SRC_SERIAL		(1*0x01000000L)

#define REGC_OSCTEST			(1*0x02000000L)

#define REGC_FRLDHI			(1*0x04000000L)

#define REGC_MUXTEST_B1		(1*0x08000000L)
#define REGC_MUXTEST_B2		(2*0x08000000L)
#define REGC_MUXTEST_B3		(4*0x08000000L)
#define REGC_MUXTEST_B4		(8*0x08000000L)

#define REGC_PRESCALER		(1*0x80000000L)
//! @}

/*************************************************************************//**
 * @name ADF7020 register D values
 * @{
 *************************************************************************/
#define REGD_ADDRESS			(0x0000000DL)

#define REGD_KP_1			(1*0x00000010L)
#define REGD_KP_2			(2*0x00000010L)
#define REGD_KP_3			(3*0x00000010L)
#define REGD_KP_4			(4*0x00000010L)
#define REGD_KP_5			(5*0x00000010L)
#define REGD_KP_6			(6*0x00000010L)
#define REGD_KP_7			(7*0x00000010L)
#define REGD_KP_8			(8*0x00000010L)
#define REGD_KP_9			(9*0x00000010L)
#define REGD_KP_10		(10*0x00000010L)
#define REGD_KP_11		(11*0x00000010L)
#define REGD_KP_12		(12*0x00000010L)
#define REGD_KP_13		(13*0x00000010L)
#define REGD_KP_14		(14*0x00000010L)
#define REGD_KP_15		(15*0x00000010L)

#define REGD_KI_1			(1*0x00000100L)
#define REGD_KI_2			(2*0x00000100L)
#define REGD_KI_3			(3*0x00000100L)
#define REGD_KI_4			(4*0x00000100L)
#define REGD_KI_5			(5*0x00000100L)
#define REGD_KI_6			(6*0x00000100L)
#define REGD_KI_7			(7*0x00000100L)
#define REGD_KI_8			(8*0x00000100L)
#define REGD_KI_9			(9*0x00000100L)
#define REGD_KI_10		(10*0x00000100L)
#define REGD_KI_11		(11*0x00000100L)
#define REGD_KI_12		(12*0x00000100L)
#define REGD_KI_13		(13*0x00000100L)
#define REGD_KI_14		(14*0x00000100L)
#define REGD_KI_15		(15*0x00000100L)

#define REGD_PE_1			(0*0x00000100L)
#define REGD_PE_2			(1*0x00000100L)
#define REGD_PE_3			(2*0x00000100L)
#define REGD_PE_4			(3*0x00000100L)
#define REGD_PE_5			(4*0x00000100L)
#define REGD_PE_6			(5*0x00000100L)
#define REGD_PE_7			(6*0x00000100L)
#define REGD_PE_8			(7*0x00000100L)
#define REGD_PE_9			(8*0x00000100L)
#define REGD_PE_10		(9*0x00000100L)
#define REGD_PE_11		(10*0x00000100L)
#define REGD_PE_12		(11*0x00000100L)
#define REGD_PE_13		(12*0x00000100L)
#define REGD_PE_14		(13*0x00000100L)
#define REGD_PE_15		(14*0x00000100L)

#define REGD_PE_B1		(1*0x00001000L)
#define REGD_PE_B2		(2*0x00001000L)
#define REGD_PE_B3		(4*0x00001000L)
#define REGD_PE_B4		(8*0x00001000L)
#define REGD_PE_B5		(16*0x00001000L)
#define REGD_PE_B6		(32*0x00001000L)
#define REGD_PE_B7		(64*0x00001000L)
#define REGD_PE_B8		(128*0x00001000L)
#define REGD_PE_B9		(256*0x00001000L)
#define REGD_PE_B10		(512*0x00001000L)

#define REGD_DG_B1		(1*0x04000000L)
#define REGD_DG_B2		(2*0x04000000L)
#define REGD_DG_B3		(4*0x04000000L)
#define REGD_DG_B4		(8*0x04000000L)
#define REGD_DG_B5		(16*0x40000000L)
#define REGD_DG_B6		(32*0x40000000L)


//! @}

#endif /* CP_FIRMWARETEMPORARY_DRIVERS_ADF7020_REG_H_ */
