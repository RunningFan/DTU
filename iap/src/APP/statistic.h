#ifndef _STATISTIC_H_
#define _STATISTIC_H_

#include "stm32f10x.h"
#include "duplicate.h"
#define CAN_LENTH 100
#define hot_mode   0
#define cold_mode  1

/*-----------------------------------宏定义区------------------------------------*/
#define FLAG      	    0
#define WORK_TIME	    1
#define COLD_TIME1   	2
#define COLD_TIME2   	3
#define COLD_TIME3   	4
#define HOT_TIME1    	5
#define HOT_TIME2    	6
#define HOT_TIME3    	7
#define HOT_TIME4    	8
#define HOT_TIME5    	9
#define CEXV0_TIME1  	10
#define CEXV0_TIME2  	11
#define CEXV0_TIME3  	12
#define CEXV1_TIME1  	13
#define CEXV1_TIME2  	14
#define CEXV1_TIME3  	15
#define CEXV2_TIME1  	16
#define CEXV2_TIME2  	17
#define CEXV2_TIME3  	18
#define CEXV3_TIME1  	19
#define CEXV3_TIME2  	20
#define CEXV3_TIME3  	21
#define HEXV0_TIME1  	22
#define HEXV0_TIME2  	23
#define HEXV0_TIME3  	24
#define HEXV1_TIME1  	25
#define HEXV1_TIME2  	26
#define HEXV1_TIME3  	27
#define HEXV2_TIME1  	28
#define HEXV2_TIME2  	29
#define HEXV2_TIME3  	30
#define HEXV3_TIME1  	31
#define HEXV3_TIME2  	32
#define HEXV3_TIME3  	33
#define HDXV0_TIME1  	34
#define HDXV0_TIME2  	35
#define HDXV0_TIME3  	36
#define HDXV1_TIME1  	37
#define HDXV1_TIME2  	38
#define HDXV1_TIME3  	39
#define HDXV2_TIME1  	40
#define HDXV2_TIME2  	41
#define HDXV2_TIME3  	42
#define HDXV3_TIME1  	43
#define HDXV3_TIME2  	44
#define HDXV3_TIME3  	45
#define HPRSN_TIME1  	46
#define HPRSN_TIME2  	47
#define HPRSN_TIME3  	48
#define HPRSN_TIME4    	49
#define LPRSN_TIME1    	50
#define LPRSN_TIME2    	51
#define LPRSN_TIME3    	52
#define LPRSN_TIME4    	53
#define LPRSN_TIME5    	54
#define HPRSC_TIME1    	55
#define HPRSC_TIME2    	56
#define HPRSC_TIME3    	57
#define HPRSC_TIME4    	58
#define LPRSC_TIME1    	59
#define LPRSC_TIME2    	60
#define LPRSC_TIME3    	61
#define LPRSC_TIME4    	62
#define LPRSC_TIME5    	63
#define HPRSH_TIME1    	64
#define HPRSH_TIME2    	65
#define HPRSH_TIME3    	66
#define LPRSH_TIME1    	67
#define LPRSH_TIME2    	68
#define LPRSH_TIME3    	69
#define WIND1_STATIC1  	70
#define WIND1_STATIC2  	71
#define WIND1_STATIC3  	72
#define WIND1_STATIC4  	73
#define WIND2_STATIC1  	74
#define WIND2_STATIC2  	75
#define WIND2_STATIC3  	76
#define WIND2_STATIC4  	77
#define WIND3_STATIC1  	78
#define WIND3_STATIC2  	79
#define WIND3_STATIC3  	80
#define WIND3_STATIC4  	81
#define WIND4_STATIC1  	82
#define WIND4_STATIC2  	83
#define WIND4_STATIC3  	84
#define WIND4_STATIC4  	85
#define MACHINE00_STATIC1  	86
#define MACHINE00_STATIC2  	87
#define MACHINE00_STATIC3  	88
#define MACHINE01_STATIC1  	89
#define MACHINE01_STATIC2  	90
#define MACHINE01_STATIC3  	91
#define MACHINE3_STATIC1   	92
#define MACHINE3_STATIC2   	93
#define MACHINE3_STATIC3   	94
#define MACHINE4_STATIC1   	95
#define MACHINE4_STATIC2   	96
#define MACHINE4_STATIC3   	97
#define MACHINE10_STATIC1  	98
#define MACHINE10_STATIC2  	99
#define MACHINE10_STATIC3  	100
#define MACHINE11_STATIC1  	101
#define MACHINE11_STATIC2  	102
#define MACHINE11_STATIC3  	103
#define MACHINE20_STATIC1  	104
#define MACHINE20_STATIC2  	105
#define MACHINE20_STATIC3  	106
#define MACHINE21_STATIC1  	107
#define MACHINE21_STATIC2  	108
#define MACHINE21_STATIC3  	109
#define MACHINE30_STATIC1  	110
#define MACHINE30_STATIC2  	111
#define MACHINE30_STATIC3  	112
#define MACHINE31_STATIC1  	113
#define MACHINE31_STATIC2  	114
#define MACHINE31_STATIC3  	115
#define BURTHEN_STATIC1    	116
#define BURTHEN_STATIC2    	117
#define BURTHEN_STATIC3    	118
#define BURTHEN_STATIC4    	119
#define BURTHEN_STATIC5    	120
#define BURTHEN_STATIC6    	121
#define WORKBURTHEN_STATIC1 	122
#define WORKBURTHEN_STATIC2 	123
#define WORKBURTHEN_STATIC3 	124
#define WORKBURTHEN_STATIC4 	125
#define WORKBURTHEN_STATIC5 	126
#define GUOREDUN1_STATIC1   	127
#define GUOREDUN1_STATIC2   	128
#define GUOREDUN1_STATIC3   	129
#define GUOREDUN1_STATIC4   	130
#define GUOREDUN1_STATIC5   	131
#define GUOREDUN1_STATIC6   	132
#define GUOREDUN1_STATIC7   	133
#define GUOREDUN1_STATIC8   	134
#define GUOREDUN13_STATIC1  	135
#define GUOREDUN13_STATIC2  	136
#define GUOREDUN13_STATIC3  	137
#define GUOREDUN13_STATIC4  	138
#define GUOREDUN14_STATIC5  	139
#define GUOREDUN14_STATIC6  	140
#define GUOREDUN14_STATIC7  	141
#define GUOREDUN14_STATIC8  	142
#define GUOREDUN2_STATIC1   	143
#define GUOREDUN2_STATIC2   	144
#define GUOREDUN2_STATIC3   	145
#define GUOREDUN2_STATIC4   	146
#define GUOREDUN2_STATIC5   	147
#define GUOREDUN2_STATIC6   	148
#define GUOREDUN2_STATIC7   	149
#define GUOREDUN2_STATIC8   	150
#define GUOREDUN3_STATIC1   	151
#define GUOREDUN3_STATIC2   	152
#define GUOREDUN3_STATIC3   	153
#define GUOREDUN3_STATIC4   	154
#define GUOREDUN3_STATIC5   	155
#define GUOREDUN3_STATIC6   	156
#define GUOREDUN3_STATIC7   	157
#define GUOREDUN3_STATIC8 	158
#define GUOREDUN4_STATIC1 	159
#define GUOREDUN4_STATIC2 	160
#define GUOREDUN4_STATIC3 	161
#define GUOREDUN4_STATIC4 	162
#define GUOREDUN4_STATIC5 	163
#define GUOREDUN4_STATIC6 	164
#define GUOREDUN4_STATIC7 	165
#define GUOREDUN4_STATIC8 	166
#define GUOREDUC1_STATIC1 	167
#define GUOREDUC1_STATIC2 	168
#define GUOREDUC1_STATIC3 	169
#define GUOREDUC1_STATIC4 	170
#define GUOREDUC1_STATIC5 	171
#define GUOREDUC1_STATIC6 	172
#define GUOREDUC1_STATIC7 	173
#define GUOREDUC1_STATIC8 	174
#define GUOREDUC13_STATIC1	175
#define GUOREDUC13_STATIC2	176
#define GUOREDUC13_STATIC3	177
#define GUOREDUC13_STATIC4	178
#define GUOREDUC14_STATIC5	179
#define GUOREDUC14_STATIC6	180
#define GUOREDUC14_STATIC7	181
#define GUOREDUC14_STATIC8	182
#define GUOREDUC2_STATIC1 	183
#define GUOREDUC2_STATIC2 	184
#define GUOREDUC2_STATIC3 	185
#define GUOREDUC2_STATIC4 	186
#define GUOREDUC2_STATIC5 	187
#define GUOREDUC2_STATIC6 	188
#define GUOREDUC2_STATIC7 	189
#define GUOREDUC2_STATIC8 	190
#define GUOREDUC3_STATIC1 	191
#define GUOREDUC3_STATIC2 	192
#define GUOREDUC3_STATIC3 	193
#define GUOREDUC3_STATIC4 	194
#define GUOREDUC3_STATIC5 	195
#define GUOREDUC3_STATIC6 	196
#define GUOREDUC3_STATIC7 	197
#define GUOREDUC3_STATIC8 	198
#define GUOREDUC4_STATIC1 	199
#define GUOREDUC4_STATIC2 	200
#define GUOREDUC4_STATIC3 	201
#define GUOREDUC4_STATIC4 	202
#define GUOREDUC4_STATIC5 	203
#define GUOREDUC4_STATIC6 	204
#define GUOREDUC4_STATIC7 	205
#define GUOREDUC4_STATIC8 	206
#define GUOREDUH1_STATIC1 	207
#define GUOREDUH1_STATIC2 	208
#define GUOREDUH1_STATIC3 	209
#define GUOREDUH1_STATIC4 	210
#define GUOREDUH1_STATIC5 	211
#define GUOREDUH1_STATIC6 	212
#define GUOREDUH1_STATIC7 	213
#define GUOREDUH1_STATIC8 	214
#define GUOREDUH13_STATIC1	215
#define GUOREDUH13_STATIC2	216
#define GUOREDUH13_STATIC3	217
#define GUOREDUH13_STATIC4	218
#define GUOREDUH14_STATIC5	219
#define GUOREDUH14_STATIC6	220
#define GUOREDUH14_STATIC7	221
#define GUOREDUH14_STATIC8	222
#define GUOREDUH2_STATIC1 	223
#define GUOREDUH2_STATIC2 	224
#define GUOREDUH2_STATIC3 	225
#define GUOREDUH2_STATIC4 	226
#define GUOREDUH2_STATIC5 	227
#define GUOREDUH2_STATIC6 	228
#define GUOREDUH2_STATIC7 	229
#define GUOREDUH2_STATIC8 	230
#define GUOREDUH3_STATIC1 	231
#define GUOREDUH3_STATIC2 	232
#define GUOREDUH3_STATIC3 	233
#define GUOREDUH3_STATIC4 	234
#define GUOREDUH3_STATIC5 	235
#define GUOREDUH3_STATIC6 	236
#define GUOREDUH3_STATIC7 	237
#define GUOREDUH3_STATIC8 	238
#define GUOREDUH4_STATIC1 	239
#define GUOREDUH4_STATIC2 	240
#define GUOREDUH4_STATIC3 	241
#define GUOREDUH4_STATIC4 	242
#define GUOREDUH4_STATIC5 	243
#define GUOREDUH4_STATIC6 	244
#define GUOREDUH4_STATIC7 	245
#define GUOREDUH4_STATIC8 	246
#define PAIQI1_STATIC1    	247
#define PAIQI1_STATIC2    	248
#define PAIQI1_STATIC3    	249
#define PAIQI1_STATIC4    	250
#define PAIQI1_STATIC5    	251
#define PAIQI1_STATIC6    	252
#define PAIQI1_STATIC7    	253
#define PAIQI1_STATIC8    	254
#define PAIQI13_STATIC1   	255
#define PAIQI13_STATIC2   	256
#define PAIQI13_STATIC3   	257
#define PAIQI13_STATIC4   	258
#define PAIQI14_STATIC5   	259
#define PAIQI14_STATIC6   	260
#define PAIQI14_STATIC7   	261
#define PAIQI14_STATIC8   	262
#define PAIQI2_STATIC1    	263
#define PAIQI2_STATIC2    	264
#define PAIQI2_STATIC3    	265
#define PAIQI2_STATIC4    	266
#define PAIQI2_STATIC5    	267
#define PAIQI2_STATIC6    	268
#define PAIQI2_STATIC7    	269
#define PAIQI2_STATIC8    	270
#define PAIQI3_STATIC1    	271
#define PAIQI3_STATIC2    	272
#define PAIQI3_STATIC3    	273
#define PAIQI3_STATIC4    	274
#define PAIQI3_STATIC5  	275
#define PAIQI3_STATIC6  	276
#define PAIQI3_STATIC7  	277
#define PAIQI3_STATIC8  	278
#define PAIQI4_STATIC1  	279
#define PAIQI4_STATIC2  	280
#define PAIQI4_STATIC3  	281
#define PAIQI4_STATIC4  	282
#define PAIQI4_STATIC5  	283
#define PAIQI4_STATIC6  	284
#define PAIQI4_STATIC7  	285
#define PAIQI4_STATIC8  	286
#define QIPANGTONG1_TIME	287
#define QIPANGTONG2_TIME	288
#define QIPANGTONG3_TIME	289
#define QIPANGTONG4_TIME	290
#define WARMBAND3_TIME  	291
#define WARMBAND11_TIME 	292
#define WARMBAND12_TIME 	293
#define WARMBAND21_TIME 	294
#define WARMBAND22_TIME 	295
#define WARMBAND31_TIME 	296
#define WARMBAND32_TIME 	297
#define WARMBAND41_TIME 	298
#define WARMBAND42_TIME 	299
#define JUNYOUFA11_TIME 	300
#define JUNYOUFA12_TIME 	301
#define JUNYOUFA21_TIME 	302
#define JUNYOUFA22_TIME 	303
#define JUNYOUFA31_TIME 	304
#define JUNYOUFA32_TIME 	305
#define JUNYOUFA41_TIME 	306
#define JUNYOUFA42_TIME 	307
#define HUIYOUFA11_TIME 	308
#define HUIYOUFA12_TIME 	309
#define HUIYOUFA21_TIME 	310
#define HUIYOUFA22_TIME 	311
#define HUIYOUFA31_TIME 	312
#define HUIYOUFA32_TIME 	313
#define HUIYOUFA41_TIME 	314
#define HUIYOUFA42_TIME 	315
#define HUIYOUFA3_TIME  	316
#define HUIYOUFA4_TIME  	317
#define PINGHENGFA11_TIME  	318
#define PINGHENGFA12_TIME  	319
#define PINGHENGFA21_TIME  	320
#define PINGHENGFA22_TIME  	321
#define PINGHENGFA31_TIME  	322
#define PINGHENGFA32_TIME  	323
#define PINGHENGFA41_TIME  	324
#define PINGHENGFA42_TIME  	325
#define PINGHENGFA3_TIME   	326
#define MACHINE1_VOLTAGE11 	327//系统1压缩机1
#define MACHINE1_VOLTAGE12 	328
#define MACHINE1_VOLTAGE13 	329
#define MACHINE1_VOLTAGE21 	330//系统2压缩机1
#define MACHINE1_VOLTAGE22 	331
#define MACHINE1_VOLTAGE23 	332
#define MACHINE1_VOLTAGE31 	333
#define MACHINE1_VOLTAGE32 	334
#define MACHINE1_VOLTAGE33 	335
#define MACHINE1_VOLTAGE41 	336
#define MACHINE1_VOLTAGE42 	337
#define MACHINE1_VOLTAGE43 	338
#define MACHINE1_VOLTAGE3  	339
#define MACHINE2_VOLTAGE11 	340
#define MACHINE2_VOLTAGE12 	341
#define MACHINE2_VOLTAGE13 	342
#define MACHINE2_VOLTAGE21 	343
#define MACHINE2_VOLTAGE22 	344
#define MACHINE2_VOLTAGE23 	345
#define MACHINE2_VOLTAGE31 	346
#define MACHINE2_VOLTAGE32 	347
#define MACHINE2_VOLTAGE33 	348
#define MACHINE2_VOLTAGE41 	349
#define MACHINE2_VOLTAGE42 	350
#define MACHINE2_VOLTAGE43 	351
#define MACHINE3_VOLTAGE1  	352
#define MACHINE3_VOLTAGE2   353
#define MACHINE3_VOLTAGE3  	354


/////**********************次数统计***************************////
#define HPRSH_COUNT1	        0
#define HPRSH_COUNT2  	        1
#define HPRSH_COUNT3  	        2
#define LPRSH_COUNT1  	        3
#define LPRSH_COUNT2  	        4
#define LPRSH_COUNT3  	        5
#define FREST1_COUNT1 	        6
#define FREST1_COUNT2 	        7
#define FREST1_COUNT3 	        8
#define FREST2_COUNT1 	        9
#define FREST2_COUNT2 	        10
#define FREST2_COUNT3 	        11
#define FREST3_COUNT1 	        12
#define FREST3_COUNT2 	        13
#define FREST3_COUNT3 	        14
#define FREST4_COUNT1 	        15
#define FREST4_COUNT2 	        16
#define FREST4_COUNT3 	        17
#define TIME_COUNT1   	        18
#define TIME_COUNT2   	        19
#define TIME_COUNT3   	        20
#define QIPANGTONG1_COUNT   	21
#define QIPANGTONG2_COUNT   	22
#define QIPANGTONG3_COUNT   	23
#define QIPANGTONG4_COUNT   	24
#define JUNYOUFA11_COUNT    	25
#define JUNYOUFA12_COUNT    	26
#define JUNYOUFA21_COUNT    	27
#define JUNYOUFA22_COUNT    	28
#define JUNYOUFA31_COUNT    	29
#define JUNYOUFA32_COUNT    	30
#define JUNYOUFA41_COUNT    	31
#define JUNYOUFA42_COUNT    	32
#define HUIYOUFA11_COUNT    	33
#define HUIYOUFA12_COUNT    	34
#define HUIYOUFA21_COUNT    	35
#define HUIYOUFA22_COUNT    	36
#define HUIYOUFA31_COUNT    	37
#define HUIYOUFA32_COUNT    	38
#define HUIYOUFA41_COUNT    	39
#define HUIYOUFA42_COUNT    	40
#define HUIYOUFA3_COUNT     	41
#define HUIYOUFA4_COUNT     	42
#define PINGHENGFA11_COUNT  	43
#define PINGHENGFA12_COUNT  	44
#define PINGHENGFA21_COUNT  	45
#define PINGHENGFA22_COUNT  	46
#define PINGHENGFA31_COUNT  	47
#define PINGHENGFA32_COUNT  	48
#define PINGHENGFA41_COUNT  	49
#define PINGHENGFA42_COUNT  	50
#define PINGHENGFA3_COUNT   	51
#define MACHINE11FAIL_COUNT 	52
#define MACHINE12FAIL_COUNT 	53
#define MACHINE21FAIL_COUNT 	54
#define MACHINE22FAIL_COUNT 	55
#define MACHINE31FAIL_COUNT 	56
#define MACHINE32FAIL_COUNT 	57
#define MACHINE41FAIL_COUNT 	58
#define MACHINE42FAIL_COUNT 	59
#define MACHINE3FAIL_COUNT  	60

/*---------------------------统计部分的变量声明----------------------------*/
typedef struct 
{
    uint32_t bgTime;
    uint32_t ctTime;
}timetype;

typedef struct
{
//    uint16_t lenth1;
//    uint16_t lenth2;
    uint8_t flash_flag;
    uint16_t F4_COUNT;
    uint32_t timeCount;
    uint32_t time_buffer[355];
    uint16_t count_buffer[100];
}buffertype;
/*---------------------------变量定义----------------------------*/
//extern uint8_t out,num;//作为全局变量，局部变量会导致堆栈溢出
extern uint16_t cold_ability,hot_ability;
extern uint8_t oil_flag,frest_flag;
extern uint8_t sys_mode;
extern uint8_t F4_CONNECT;
extern int8_t temp_outside;
extern int8_t temp_frest[4];
extern int8_t H_press,L_press;
extern uint8_t machine1[4],machine2[4],machine3,machine4;
extern uint16_t subcooler[4],hotvalve[4];
extern uint8_t sys_machine;
extern uint8_t wind_frequence[4];
extern uint8_t machine_frequence1[4],machine_frequence2[4],machine_frequence3;
extern uint16_t machine1_voltage[4],machine2_voltage[4],machine3_voltage;
extern uint8_t work_station[125];
extern uint16_t capability[125];
extern uint16_t sys_ability;
extern uint16_t machineID;
extern int8_t keding1[4],keding2[4];
extern int8_t paiqi1[4],paiqi2[4],temp[10],tenp[10];
extern int8_t H_module[4],L_module[4];
extern uint8_t guoredu[10];
extern uint8_t qipangtong[4];
extern uint8_t warm_band1[4],warm_band2[4],warm_band3;
extern uint8_t junyoufa1[4],junyoufa2[4],huiyoufa1[4],huiyoufa2[4],pinghengfa1[4],pinghengfa2[4];
extern uint8_t huiyoufa3,huiyoufa4,pinghengfa3;
extern uint8_t machine1_fail[4],machine2_fail[4],machine3_fail;
extern timetype timer1,timer2,timer3,timer4,timer5,timer6,timer7,timer8,timer9,timer10, timer11,timer12;
extern timetype timerbf1[6],timerbf2[6],timerbf3[6],timerbf4[6],timerbf5[6],timerbf6[6],timerbf7[6],timerbf8[6],timerbf9[6],timerbf10[6],timerbf11[6],timerbf12[6],timerbf13[6],timerbf14[6];
extern uint8_t current_frest,oldflag_frest;
extern uint32_t start,time_s;
extern uint8_t new_state;
extern float capa_percent,work_percent;
extern uint8_t QPT_flag[4],JYF_flag1[4],JYF_flag2[4],JYF_flag3;
extern uint8_t HYF_flag1[4],HYF_flag2[4],HYF_flag3,HYF_flag4;
extern uint8_t PHF_flag1[4],PHF_flag2[4],PHF_flag3;
extern uint8_t oldflag1_fail[4],oldflag2_fail[4],oldflag3_fail;

extern uint32_t TIM2_count;
extern buffertype stat_buffer;
extern uint16_t F4_COUNT;

void GetData_can(CanMsgType    m_Convert);//获取CAN数据，请放在CAN数据接收处
void Statistic_CanData();//统计部分，一秒钟执行一次，对时间的精度要求较高。
uint16_t time_account(timetype * time);
uint8_t sign_everyday(uint8_t curSta);//每日打卡函数
#endif