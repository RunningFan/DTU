/****************************************Copyright (c)**************************************************
*                         Gree Electric Applicances,INC.OF ZHUHAI
*                      TECHNOLOGY DEPT.OF COMMERCIAL AIR CONDITIONING
*                                  http://www.gree.com.cn
*
*--------------------------------File Info--------------------------------------------------------------
* File Name:		BusIdentityConfigCommand.h
* Created by:		Running
* Created date:		2018-1-18
* Version:			V 0.0.1
* Descriptions:		数据统计相关宏定义
*
*--------------------------------------------------------------------------------------------------------
* Modified by:
* Modified date:
* Version:
* Descriptions:
*
********************************************************************************************************/

#ifndef _STATISTIC_H_
#define _STATISTIC_H_

#include "stm32f10x.h"
#include "duplicate.h"
#define CAN_LENTH 100
#define hot_mode   0
#define cold_mode  1
#define OK_IS_3S		1u
#define OK_NO_3S		0u

#define INDOOR_MAX	16u
#define OUTDOOR_MAX	1u
#define INDOOR_BASE 32

#define HOURS_TIME		3600


//#define TEST_TEST



/*-----------------------------------宏定义区------------------------------------*/

/* Temperature here shows Main-module's environment temperature at outdoor  */
/*******************************************************************************
 *********  COLD and C represent refrigerate  **********************************
 *********  HOT and H represent Heating    *************************************
 *********  L represent Low    *************************************************
 *********  M represent Mid    *************************************************
 *********  G represent Great   ************************************************
 *********  N represent Negative  **********************************************
 *********  TIMEn represent totals of time *************************************
 *********  EXE represent EXEcute   ********************************************
********************************************************************************/
/*
*	Be attention: About involving indoor-machine parameter statistics,
*	If no special instruction,and then we acquiesce in the system as 
*	a main indoor-machine.
*/

#define RESERVED1      	    			0

#define GPRS_GETELEC_TIME	    		1

#define MACH_WAIT_TIME					2

/*
*	COLD --- COLDing or refrigeration
*	L10--Low 10 degree -- <10
*	M10_13 --Mid 10-13 degree -- 10<Mid<13
*	G45 -- Great 45 degree -- >45
*/
#define COLD_L10_TIME1   				3
#define COLD_M10_13_TIME2		   		4
#define COLD_M13_16_TIME3		   		5
#define COLD_M16_19_TIME4		   		6
#define COLD_M19_22_TIME5		   		7
#define COLD_M22_25_TIME6		   		8
#define COLD_M25_28_TIME7		   		9
#define COLD_M28_31_TIME8		   		10
#define COLD_M31_34_TIME9		   		11
#define COLD_M34_37_TIME10		   		12
#define COLD_M37_40_TIME11		   		13
#define COLD_M40_43_TIME12		   		14
#define COLD_M43_45_TIME13		   		15
#define COLD_G45_TIME14			   		16

/*
*	HOT -- HOT or Heating
*	LN15 -- Low Negative 15 --- <-15
*	MN15_N12 -- Mid Negative 15 and Negative 12 -- -15<Min<-12
*/
#define HOT_LN15_TIME1		 	 	  	17
#define HOT_MN15_N12_TIME2 		 	  	18
#define HOT_MN12_N9_TIME3 	 		  	19
#define HOT_MN9_N6_TIME4 	 	  		20
#define HOT_MN6_N3_TIME5 	 	  		21
#define HOT_MN3_0_TIME6 	 	  		22
#define HOT_M0_3_TIME7		 	 	  	23
#define HOT_M3_6_TIME8 	 			  	24
#define HOT_M6_9_TIME9 	 	  			25
#define HOT_M9_12_TIME10 	 	  		26
#define HOT_M12_15_TIME11 	 	  		27
#define HOT_M15_18_TIME12 	 	  		28
#define HOT_M18_21_TIME13 	 	  		29
#define HOT_M21_23_TIME14 	 	  		30
#define HOT_G23_TIME15 	 	  			31

/*
*	ReTurn OIL frequency statistics in Heating or Colding EXEcution
*/
#define HEXE_RTOIL_COUNT				32
#define CEXE_RTOIL_COUNT				33

/*
*	DeFRoSting's EXPenses of time 
*/
#define	DFRST_EXP_L2_COUNT1				34
#define	DFRST_EXP_M2_3_COUNT2			35
#define	DFRST_EXP_M3_4_COUNT3			36
#define	DFRST_EXP_M4_5_COUNT4			37
#define	DFRST_EXP_M5_6_COUNT5			38
#define	DFRST_EXP_M6_7_COUNT6			39
#define	DFRST_EXP_M7_8_COUNT7			40
#define	DFRST_EXP_G8_COUNT8				41
#define	DFRST_SUM_COUNT					42

/*
*	in DeFRoSting time,Open machine's range of temperature fluctuation 
*	at indoor-machine count times.
*
*/

#define	DFRST_OPN_L3_COUNT1					43
#define	DFRST_OPN_M3_4_COUNT2				44
#define	DFRST_OPN_M4_5_COUNT3				45
#define	DFRST_OPN_M5_6_COUNT4				46
#define	DFRST_OPN_M6_7_COUNT5				47
#define	DFRST_OPN_G7_COUNT6					48

/*
*	TIME statistic of NoRMal EXEcutation LoaD-RaTe at OPEN state.
*/

#define NEXE_LDRT_OPEN_L10_TIME1				49
#define NEXE_LDRT_OPEN_M10_15_TIME2				50
#define NEXE_LDRT_OPEN_M15_20_TIME3				51
#define NEXE_LDRT_OPEN_M20_25_TIME4				52
#define NEXE_LDRT_OPEN_M25_30_TIME5				53
#define NEXE_LDRT_OPEN_M30_35_TIME6				54
#define NEXE_LDRT_OPEN_M35_40_TIME7				55
#define NEXE_LDRT_OPEN_M40_45_TIME8				56
#define NEXE_LDRT_OPEN_M45_50_TIME9				57
#define NEXE_LDRT_OPEN_M50_55_TIME10			58
#define NEXE_LDRT_OPEN_M55_60_TIME11			59
#define NEXE_LDRT_OPEN_M60_65_TIME12			60
#define NEXE_LDRT_OPEN_M65_70_TIME13			61
#define NEXE_LDRT_OPEN_M70_75_TIME14			62
#define NEXE_LDRT_OPEN_M75_80_TIME15			63
#define NEXE_LDRT_OPEN_M80_85_TIME16			64
#define NEXE_LDRT_OPEN_M85_90_TIME17			65
#define NEXE_LDRT_OPEN_M90_95_TIME18			66
#define NEXE_LDRT_OPEN_M95_100_TIME19			67
#define NEXE_LDRT_OPEN_M100_105_TIME20			68
#define NEXE_LDRT_OPEN_M105_110_TIME21			69
#define NEXE_LDRT_OPEN_M110_115_TIME22			70
#define NEXE_LDRT_OPEN_M115_120_TIME23			71


/*
*	The statistic of PaiQiGuoReDu at NoRMaL EXEcutation.
*	PQGRD ----- PaiQiGuoReDu
*/

#define NEXE_CMP1PQGRD_L5_TIME1					72
#define NEXE_CMP1PQGRD_M5_10_TIME2				73
#define NEXE_CMP1PQGRD_M10_25_TIME3				74
#define NEXE_CMP1PQGRD_M25_30_TIME4				75
#define NEXE_CMP1PQGRD_M30_35_TIME5				76
#define NEXE_CMP1PQGRD_M35_40_TIME6				77
#define NEXE_CMP1PQGRD_G40_TIME7				78

/*
*	The statistic of PaiQiGuoReDu at NoRMaL EXEcutation.
*	XQGRD ----- XiQiGuoReDu
*/
#define NEXE_CMP1XQGRD_LN3_TIME1			79
#define NEXE_CMP1XQGRD_MN3_N2_TIME2			80
#define NEXE_CMP1XQGRD_MN2_N1_TIME3			81
#define NEXE_CMP1XQGRD_MN1_0_TIME4			82
#define NEXE_CMP1XQGRD_M0_1_TIME5			83
#define NEXE_CMP1XQGRD_M1_2_TIME6			84
#define NEXE_CMP1XQGRD_M2_3_TIME7			85
#define NEXE_CMP1XQGRD_G3_TIME8				86


/*
*	Time statistic of ElECtricity heating belt opening
*/

#define CMP1_ELECH_OPEN_TIME				87

/*
*	The sum of ELECtricity Charge
*/

#define SUM_ELEC_CHRG						88

/*
*	Sum of Compressor Power						
*/

#define SUM_CMP_POWER						89

/*
*	The usage of those number is to compute power of FAN
*	NoRMaL EXEcutation FAN FReQuence 
*/

#define NEXE_FANFRQ_L10_TIME1				90
#define NEXE_FANFRQ_M10_12_TIME2			91
#define NEXE_FANFRQ_M12_14_TIME3			92
#define NEXE_FANFRQ_M14_16_TIME4			93
#define NEXE_FANFRQ_M16_18_TIME5			94
#define NEXE_FANFRQ_M18_20_TIME6			95
#define NEXE_FANFRQ_M20_22_TIME7			96
#define NEXE_FANFRQ_M22_24_TIME8			97
#define NEXE_FANFRQ_M24_26_TIME9			98
#define NEXE_FANFRQ_M26_28_TIME10			99
#define NEXE_FANFRQ_M28_30_TIME11			100
#define NEXE_FANFRQ_M30_32_TIME12			101
#define NEXE_FANFRQ_M32_34_TIME13			102
#define NEXE_FANFRQ_M34_36_TIME14			103
#define NEXE_FANFRQ_M36_38_TIME15			104
#define NEXE_FANFRQ_M38_40_TIME16			105
#define NEXE_FANFRQ_M40_42_TIME17			106
#define NEXE_FANFRQ_M42_44_TIME18			107
#define NEXE_FANFRQ_M44_46_TIME19			108
#define NEXE_FANFRQ_M46_48_TIME20			109
#define NEXE_FANFRQ_G48_TIME21				110


/*
*	DC-MoTor(indoor-motor) speed distribution time.      
*/
#define	DC_INMT_RPM_L200_TIME1						111
#define	DC_INMT_RPM_M200_400_TIME2					112
#define	DC_INMT_RPM_M400_600_TIME3					113
#define	DC_INMT_RPM_M600_800_TIME4					114
#define	DC_INMT_RPM_M800_1000_TIME5					115
#define	DC_INMT_RPM_M1000_1200_TIME6				116
#define	DC_INMT_RPM_M1200_1500_TIME7				117
#define	DC_INMT_RPM_G1500_TIME8						118


/*
*	indoor or outdoor-motor shutdown frequency
*	N -- Night
*   D -- Day
*/

#define INMT_SHTDWN_N_M20_8_COUNT1					119
#define INMT_SHTDWN_D_M8_20_COUNT2					120

#define OUTMT_SHTDWN_N_M20_8_COUNT1					121
#define OUTMT_SHTDWN_D_M8_20_COUNT2					122

/*
*	Linking-up,but it's states at high temperature.
*/

#define NEXE_CMP1PQWD_M100_115_TIME0				123



/*
*	OPEN-machine EXEcute,IPM module statistic times of temperature.
*/

#define OPEN_EXE_IPM_TMP_L60_TIME1					124
#define OPEN_EXE_IPM_TMP_M60_70_TIME2				125
#define OPEN_EXE_IPM_TMP_M70_80_TIME3				126
#define OPEN_EXE_IPM_TMP_M80_90_TIME4				127
#define OPEN_EXE_IPM_TMP_M90_100_TIME5				128
#define OPEN_EXE_IPM_TMP_G100_TIME6					129


/*
*	OPEN-machine EXEcute,PFC module statistic times of temperature.
*/

#define OPEN_EXE_PFC_TMP_L60_TIME1					130
#define OPEN_EXE_PFC_TMP_M60_70_TIME2				131
#define OPEN_EXE_PFC_TMP_M70_80_TIME3				132
#define OPEN_EXE_PFC_TMP_M80_90_TIME4				133
#define OPEN_EXE_PFC_TMP_M90_100_TIME5				134
#define OPEN_EXE_PFC_TMP_G100_TIME6					135

/*
*	OPEN-machine EXEcute,Time of BUSVoLT range statistic
*/

#define OPEN_EXE_BUSVLT_L200_TIME1					136
#define OPEN_EXE_BUSVLT_M200_230_TIME2				137
#define OPEN_EXE_BUSVLT_M230_260_TIME3				138
#define OPEN_EXE_BUSVLT_M260_290_TIME4				139
#define OPEN_EXE_BUSVLT_M290_320_TIME5				140
#define OPEN_EXE_BUSVLT_M320_350_TIME6				141
#define OPEN_EXE_BUSVLT_M350_360_TIME7				142
#define OPEN_EXE_BUSVLT_M360_370_TIME8				143
#define OPEN_EXE_BUSVLT_M370_460_TIME9				144
#define OPEN_EXE_BUSVLT_M460_490_TIME10				145
#define OPEN_EXE_BUSVLT_M490_520_TIME11				146
#define OPEN_EXE_BUSVLT_M520_550_TIME12				147
#define OPEN_EXE_BUSVLT_M550_580_TIME13				148
#define OPEN_EXE_BUSVLT_G580_TIME14					149


/*
*	OPEN-machine EXEcute,INput times of VoLT ranges statistic.
*	Aim at single-phase machine-unit.
*/

#define OPEN_EXE_INVLT_L165_TIME1					150
#define OPEN_EXE_INVLT_M165_185_TIME2				151
#define OPEN_EXE_INVLT_M185_205_TIME3				152
#define OPEN_EXE_INVLT_M205_215_TIME4				153
#define OPEN_EXE_INVLT_M215_225_TIME5				154
#define OPEN_EXE_INVLT_M225_235_TIME6				155
#define OPEN_EXE_INVLT_M235_255_TIME7				156
#define OPEN_EXE_INVLT_M255_265_TIME8				157
#define OPEN_EXE_INVLT_G265_TIME9					158


/*
*	OPEN-machine EXEcute,PFC module statistic times of ELECtric Current.
*/

#define OPEN_EXE_PFC_ELEC_L5_TIME1					159
#define OPEN_EXE_PFC_ELEC_M5_9_TIME2				160
#define OPEN_EXE_PFC_ELEC_M9_12_TIME3				161
#define OPEN_EXE_PFC_ELEC_M12_15_TIME4				162
#define OPEN_EXE_PFC_ELEC_M15_18_TIME5				163
#define OPEN_EXE_PFC_ELEC_M18_21_TIME6				164
#define OPEN_EXE_PFC_ELEC_M21_24_TIME7				165
#define OPEN_EXE_PFC_ELEC_M24_27_TIME8				166
#define OPEN_EXE_PFC_ELEC_M27_30_TIME9				167
#define OPEN_EXE_PFC_ELEC_M30_33_TIME10				168
#define OPEN_EXE_PFC_ELEC_G33_TIME11				169


/*
*	OPEN-machine EXEcute,CoMPressor working-current's statistics of time.
*/

#define OPEN_EXE_CMP_WRKELEC_L5_TIME1				170
#define OPEN_EXE_CMP_WRKELEC_M5_8_TIME2				171
#define OPEN_EXE_CMP_WRKELEC_M8_11_TIME3			172
#define OPEN_EXE_CMP_WRKELEC_M11_14_TIME4			173
#define OPEN_EXE_CMP_WRKELEC_M14_17_TIME5			174
#define OPEN_EXE_CMP_WRKELEC_M17_20_TIME6			175
#define OPEN_EXE_CMP_WRKELEC_G20_TIME7				176


/*
*	when CoMPressor turn 6 by not 6 in working 1 state,
*	the difference value's statistic at OPEN-state.
*/

#define WRK1_EXE_CMP_PRESS_DIFF_L03_COUNT1			177
#define WRK1_EXE_CMP_PRESS_DIFF_M03_06_COUNT2		178
#define WRK1_EXE_CMP_PRESS_DIFF_M06_09_COUNT3		179
#define WRK1_EXE_CMP_PRESS_DIFF_M09_12_COUNT4		180
#define WRK1_EXE_CMP_PRESS_DIFF_M12_15_COUNT5		181
#define WRK1_EXE_CMP_PRESS_DIFF_G15_COUNT6			182

/*
*	some of miscellaneous statistics.
*	CoMPressor DRiVe BoaRD LiMiTFReQuence 
*	DRVBRD_LMTFRQ--drive board limit frequence
*     ...._RDFRQ--reduce frequence
*	  ...._WKMGNT--weak magnetic
*/

#define CMP_DRVBRD_LMTFRQ_COUNT						183
#define CMP_DRVBRD_LMTFRQ_TIME						184

#define CMP_DRVBRD_RDFRQ_COUNT						185
#define CMP_DRVBRD_RDFRQ_TIME						186

#define CMP_DRVBRD_WKMGNT_COUNT						187
#define CMP_DRVBRD_WKMGNT_TIME						188


/*
*	ELECtricity current statistic of FAN at outdoor.
*/

#define OPEN_EXE_FAN_ELEC_L01_TIME1					189
#define OPEN_EXE_FAN_ELEC_M01_02_TIME2				190
#define OPEN_EXE_FAN_ELEC_M02_03_TIME3				191
#define OPEN_EXE_FAN_ELEC_M03_04_TIME4				192
#define OPEN_EXE_FAN_ELEC_M04_07_TIME5				193
#define OPEN_EXE_FAN_ELEC_M07_11_TIME6				194
#define OPEN_EXE_FAN_ELEC_M11_2_TIME7				195
#define OPEN_EXE_FAN_ELEC_M2_3_TIME8				196
#define OPEN_EXE_FAN_ELEC_M3_4_TIME9				197
#define OPEN_EXE_FAN_ELEC_M4_5_TIME10				198
#define OPEN_EXE_FAN_ELEC_M5_6_TIME11				199
#define OPEN_EXE_FAN_ELEC_G6_TIME12					200


/*
*	OPEN out-lay FAN-machine EXEcute,Time of FAN-BUSVoLT range statistic.
*	Aim at out-lay driven FAN
*/

#define OPEN_EXE_FAN_BUSVLT_L200_TIME1			201
#define OPEN_EXE_FAN_BUSVLT_M200_220_TIME2		202
#define OPEN_EXE_FAN_BUSVLT_M220_240_TIME3		203
#define OPEN_EXE_FAN_BUSVLT_M240_260_TIME4		204
#define OPEN_EXE_FAN_BUSVLT_M260_280_TIME5		205
#define OPEN_EXE_FAN_BUSVLT_M280_300_TIME6		206
#define OPEN_EXE_FAN_BUSVLT_M300_320_TIME7		207
#define OPEN_EXE_FAN_BUSVLT_M320_340_TIME8		208
#define OPEN_EXE_FAN_BUSVLT_M340_360_TIME9		209
#define OPEN_EXE_FAN_BUSVLT_M360_370_TIME10		210
#define OPEN_EXE_FAN_BUSVLT_G370_TIME11			211

/*
*	OPEN out-lay FAN-machine EXEcute,PFC module statistic times of temperature.
*/

#define OPEN_EXE_FAN_PFC_TMP_L50_TIME1					212
#define OPEN_EXE_FAN_PFC_TMP_M50_60_TIME2				213
#define OPEN_EXE_FAN_PFC_TMP_M60_70_TIME3				214
#define OPEN_EXE_FAN_PFC_TMP_M70_80_TIME4				215
#define OPEN_EXE_FAN_PFC_TMP_M80_90_TIME5				216
#define OPEN_EXE_FAN_PFC_TMP_M90_100_TIME6				217
#define OPEN_EXE_FAN_PFC_TMP_G100_TIME7					218


/*
*	OPEN out-lay FAN-machine EXEcute,IPM module statistic times of temperature.
*/

#define OPEN_EXE_FAN_IPM_TMP_L50_TIME1					219
#define OPEN_EXE_FAN_IPM_TMP_M50_60_TIME2				220
#define OPEN_EXE_FAN_IPM_TMP_M60_70_TIME3				221
#define OPEN_EXE_FAN_IPM_TMP_M70_80_TIME4				222
#define OPEN_EXE_FAN_IPM_TMP_M80_90_TIME5				223
#define OPEN_EXE_FAN_IPM_TMP_M90_100_TIME6				224
#define OPEN_EXE_FAN_IPM_TMP_G100_TIME7					225


/*
*	some of miscellaneous statistics aim to FAN-machine.
*	CoMPressor DRiVe BoaRD LiMiTFReQuence 
*	DRVBRD_LMTFRQ--drive board limit frequence
*     ...._RDFRQ--reduce frequence
*	  ...._WKMGNT--weak magnetic
*/

#define FAN_DRVBRD_LMTFRQ_COUNT			226
#define FAN_DRVBRD_LMTFRQ_TIME			227

#define FAN_DRVBRD_RDFRQ_COUNT			228
#define FAN_DRVBRD_RDFRQ_TIME			229

#define FAN_DRVBRD_WKMGNT_COUNT			230
#define FAN_DRVBRD_WKMGNT_TIME			231


/*
*	CAN-BUS QuaLiTy of CoMMunication.(每天)
*/

#define CAN_CMM_QLT						232

/*
*	outdoor-machine parameters.
*/

#define MACUNIT_H_L_SENSOR_ACCURACY				233
#define	MACUNIT_RESET_COUNT						234


/*
*	indoor-machine parameters.
*/

#define MACUINT_NOFAULT_WORK_TIME				235
#define MASK_SAVEENERGY_FUNC_COUNT	    		236
#define MASK_TMP_FUNC_COUNT						237
#define MASK_MODE_FUNC_COUNT					238
#define MASK_SWITCH_FUNC_COUNT					239
#define MASK_LOCK_FUNC_COUNT					240
#define SET_TIME_FUNC_COUNT						241
#define CHILD_LOCK_FUNC_COUNT					242
#define UP_DOWN_WIND_FUNC_COUNT					243
#define LEFT_RIGHT_WIND_FUNC_COUNT				244
#define SLEEP_FUNC_COUNT						245
#define CHANGE_AIR_FUNC_COUNT					246
#define MUTE_FUNC_COUNT							247
#define GO_OUT_FUNC_COUNT						248
#define SAVEENERGY_FUNC_COUNT					249
#define FILT_NET_REMIND_FUNC_COUNT				250
#define DRY_FUNC_COUNT							251
#define AID_HOT_FUNC_COUNT						252
#define SUPER_WIND_LEVEL_FUNC_COUNT				253

/*
*	indoor-motor 1 
*/ 

#define INMT1_FAN_FAULT_PRESS_LEVEL				254
#define INMT1_OPEN_M10_FLL_WTR_COUNT			255
#define INMT1_FLL_WTR_RECOVER_TIME				256

/*
*	indoor-motor 2 
*/ 

#define INMT2_FAN_FAULT_PRESS_LEVEL				257
#define INMT2_OPEN_M10_FLL_WTR_COUNT			258
#define INMT2_FLL_WTR_RECOVER_TIME				259

/*
*	indoor-motor 3 
*/ 

#define INMT3_FAN_FAULT_PRESS_LEVEL				260
#define INMT3_OPEN_M10_FLL_WTR_COUNT			261
#define INMT3_FLL_WTR_RECOVER_TIME				262

/*
*	indoor-motor 4 
*/ 

#define INMT4_FAN_FAULT_PRESS_LEVEL				263
#define INMT4_OPEN_M10_FLL_WTR_COUNT			264
#define INMT4_FLL_WTR_RECOVER_TIME				265

/*
*	indoor-motor 5 
*/ 

#define INMT5_FAN_FAULT_PRESS_LEVEL				266
#define INMT5_OPEN_M10_FLL_WTR_COUNT			267
#define INMT5_FLL_WTR_RECOVER_TIME				268

/*
*	indoor-motor 6 
*/

#define INMT6_FAN_FAULT_PRESS_LEVEL				269
#define INMT6_OPEN_M10_FLL_WTR_COUNT			270
#define INMT6_FLL_WTR_RECOVER_TIME				271

/*
*	Strong power parameters.
*/

#define IN_ELEC_L16_TIME1						272
#define IN_ELEC_L16_RATIO						273
#define IN_ELEC_M16_25_TIME2					274
#define IN_ELEC_M16_25_RATIO					275
#define IN_ELEC_M25_32_TIME3					276
#define IN_ELEC_M25_32_RATIO					277
#define IN_ELEC_M32_40_TIME4					278
#define IN_ELEC_M32_40_RATIO					279
#define IN_ELEC_G40_TIME5						280
//#define IN_ELEC_G40_RATIO						280


/*
*	New version 1.0(针对2g模块)
*
*	GPRS 模块自身参数 (每日一次的打卡)
*	1、DTU模块上电次数。(DTU上电次数：机组得电和DTU复位认为是上电)
*	2、DTU模块上电时间
*	3、DTU模块SIM模组复位次数。(SIM模组)
*	4、DTU模块复位次数。(DTU模块)
*	5、DTU模块重连服务器次数。
*	6、DTU模块连接服务器次数。
*	7、DTU模块发送数据丢失率(以打卡间隔进行统计，每次打卡进行更新)。
*	8、DTU模块自身升级成功次数。
*	9、DTU模块自身升级失败次数。

*	12、DTU模块查询SIM卡注册失败次数(上电1分钟后开始计次)。


*	15、DTU模块进入飞行模式的次数
*	16、DTU模块信号分布低概率。(0-10)
*	17、DTU模块信号分布中概率。(11-20)
*	18、DTU模块信号分布高概率。(21-31)
*
*/


#define DTU_GET_ELEC_COUNT						0
//#define DTU_GET_ELEC_TIME						1
#define DTU_SIM_RESET_COUNT						1
#define DTU_RESET_COUNT							2
#define DTU_RE_CNNCT_NET_COUNT					3
#define DTU_OK_CNNCT_NET_COUNT					4
#define DTU_DATA_LOST_RATIO						5
#define DTU_UPGRADE_SUCC_COUNT					6
#define DTU_UPGRADE_FAIL_COUNT					7
//#define DTU_CAN_NET_ERROR_RATIO					9
//#define DTU_CAN_NET_DATA_SUM					10
#define DTU_FIND_SIM_REGIST_FAIL_COUNT			8
//#define DTU_BASE_STATION_LOCATION				12
//#define DTU_SIGNAL_INTENSITY_VALUE				13
#define DTU_ENTER_FLY_MODE						9
#define DTU_SIGNAL_RATE_LOW						10
#define DTU_SIGNAL_RATE_MID						11
#define DTU_SIGNAL_RATE_HIG						12


#define LOCK_ERR_SIZE          10
/*---------------------------统计部分的变量声明----------------------------*/
typedef struct 
{
    uint32_t bgTime;
    uint32_t ctTime;
}timetype;


typedef struct
{
    uint8_t flash_flag;
    uint16_t usr_hbt_time;//用户习惯打卡时间 更正为 模块每日打卡时间
    uint16_t stat_data_time;//统计数据打卡时间
	uint16_t config_time;//实时监控配置时间
    uint8_t config_flag;//实时监控配置标志
    uint16_t F4_COUNT;
	uint16_t F4_DTU_COUNT;
	uint32_t DTU_GET_ELEC_TIME;
    uint32_t time_buffer[281];
	uint32_t dtu_param_buff[13];
}buffertype;



/*---------------------------变量定义----------------------------*/
extern uint16_t cold_ability,hot_ability;
extern uint8_t oil_flag,frest_flag;
extern uint8_t sys_mode;
extern uint8_t F4_CONNECT;
extern uint32_t start,time_s;
extern uint32_t TIM2_count;
extern buffertype stat_buffer;
extern uint16_t F4_COUNT;
extern uint16_t F4_DTU_COUNT;
extern uint8_t is_lost_data_flag;
extern uint16_t outdoor_power;
extern uint16_t outdoor_power_pri;
extern uint8_t outdoor_power_change_flag;


extern uint8_t out_mac[7];
extern uint8_t program_version;
extern uint16_t machineID;

extern uint8_t sign_over_flag;
extern uint8_t sign_over_flag_month;
//extern uint8_t a_hour_flag;
extern uint8_t ok_flag;

uint16_t GetData_can(CanMsgType    m_Convert);//获取CAN数据，请放在CAN数据接收处
void Statistic_CanData();//统计部分，一秒钟执行一次，对时间的精度要求较高。
uint16_t time_account(timetype * time);
//uint8_t sign_everyday(uint8_t curSta);//每日打卡函数
uint8_t sign_everyday(uint8_t trans_mode,uint8_t curSta);

void check_f4time();

extern uint8_t indoor_cnt;

#endif
