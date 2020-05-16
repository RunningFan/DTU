#include "common.h"
#include "drv_button.h"
#include "drv_led.h"
#include "drv_rtc.h"
#include "drv_usart.h"
#include "drv_timer.h"
#include "drv_can.h"
#include "drv_flash.h"
#include "drv_gprs.h"
#include "drv_net.h"
#include "duplicate.h"
#include "fsm.h"
#include "drv_sms.h"
#include "bsp.h"
#include "real_handle.h"
//#include "takedata.h"

uint8_t   sendCnt = 0;
OS_EVENT  *QSemSend = NULL;
void      *SendPtrArr[SEND_DATA_BUFF_SIZE];
BUFF_TYPE SendBuffer[SEND_DATA_BUFF_SIZE];
ErrParamType ACErrParam;


UpDataType g_Upload;
OS_EVENT *MutexFlash;
//OS_EVENT *MutexAT1Send;
//OS_EVENT *MutexAT2Send;
OS_EVENT *SemAT1Send;
OS_EVENT *SemAT2Send;
OS_EVENT *SemHandleTxFrame;

//OS_EVENT *SemAT1Buff;
//OS_EVENT *SemAT2Buff;


//OS_EVENT *MutexSend1;
//OS_EVENT *MutexSend3;
OS_EVENT *MutexSend2;
OS_EVENT *MutexSend4;
OS_EVENT *MutexSendData;

uint8_t  send9e_cnt;
OS_EVENT *QSend9E;     //9E帧数据队列信号量
void     *Send9EdSendPtrArr[QSEND9ELEN];
send9equeue Send9Edata[QSEND9ELEN];


uint8_t one_two_flag;//选择第一个小时区域还是第二个小时区域存储
uint8_t is_spill_flag;//每个小时区域存储是否溢出，超过 1 M
//uint8_t sample_ok_flag;//大数据采样完成标志
uint8_t one_hours_flag;//满一小时标志
uint8_t open_up_take_photo = 0;//开机拍照标志：0：开机状态时刻，1：正常运行状态




ParamType ACParam;
BigDataType BigDataParm;
BigDataType BigDataParm_pri;
BigData_Flash BigDataSampleOneHours = {
    .write_addr = BIGDATA_SAMPLE_START_ONE_ADDR,
    .read_addr = BIGDATA_SAMPLE_START_ONE_ADDR,
    .tmp_addr = BIGDATA_SAMPLE_START_ONE_ADDR,
};
BigData_Flash BigDataSampleTwoHours = {
    .write_addr = BIGDATA_SAMPLE_START_TWO_ADDR,
    .read_addr = BIGDATA_SAMPLE_START_TWO_ADDR,
    .tmp_addr = BIGDATA_SAMPLE_START_TWO_ADDR,
};

//待添加 查找到参数后，复位参数标志位！！！
//Param_ChangflagType ACParam_Changflag;   //二分查找时查找到的参数变化的标志
Param_ChangflagType ACParam_Existflag;   //参数自上电以来是否接收到过的标志
BigData_ChangeflagType BigDataParam_ChangeFlag;//一分钟内大数据参数表变化情况
BigData_ChangeflagType BigDataParam_ExistFlag;//大数据参数表存在标志
uint32_t  g_FlashWriteAddr;
uint16_t  packTime = 0;
uint16_t  real_packTime = 0;
//uint16_t  power_packtime = 0;

PageTpye  PageData[2] =
{
    { SysTable , SYS_TABLE_SIZE},
    { IdrTable , INDOOR_TABLE_SIZE}
};

PageTpye  PageErrData[2] = 
{
   { outErrTable ,   OUTLOCK_ERR_NUM},
   { indoorErrTable , INDOORLOCK_ERR_NUM},
};

//PageTpye PageSndErr[] = {
//	{SecondErrTable,SND_ERR_NUM}
//};

ParseType parseData[2] = {
	{SysParseTable,SYS_PARSE_TABLE_SIZE},
	{IdrParseTable,INDOOR_PARSE_TABLE_SIZE}
};


SrcDataTyp  SysTable[ ] = {

//{0x1000,0x0000},	{0x1001,0x0010},	{0x1004,0x0020},	{0x1005,0x0030},	{0x1006,0x0040},
//{0x100E,0x0050},	{0x100F,0x0060},	{0x1108,0x0070},	{0x1109,0x0080},	{0x1136,0x0090},	
//{0x113A,0x00A0},	{0x113B,0x00B0},	{0x113C,0x00C0},	{0x113D,0x00D0},	{0x113E,0x00E0},	
//{0x120D,0x00F0},	{0x120E,0x0100},	{0x120F,0x0110},	{0x1210,0x0120},	{0x1211,0x0130},//20
//{0x1212,0x0140},	{0x1213,0x0150},	{0x1214,0x0160},	{0x1215,0x0170},	{0x1216,0x0180},
//{0x1217,0x0190},	{0x1218,0x01A0},	{0x1219,0x01B0},	{0x121A,0x01C0},	{0x121B,0x01D0},
//{0x1351,0x01E0},	{0x1C50,0x01F0},	{0x1C51,0x0200},	{0x1C52,0x0210},	{0x1C58,0x0220},
//{0x1C5B,0x0230},	{0x1D6A,0x0240},	{0x1D71,0x0250},	{0x1D73,0x0260},	{0x1D74,0x0270},//40
//{0x1D75,0x0280},	{0x1D78,0x0290},	{0x1D88,0x02A0},	{0x1D89,0x02B0},	{0x1D8A,0x02C0},
//{0x1D8B,0x02D0},	{0x1D8C,0x02E0},	{0x1D8D,0x02F0},	{0x1D90,0x0300},	{0x1D93,0x0310},
//{0x1E51,0x0320},	{0x1E53,0x0330},	{0x1E55,0x0340},	{0x1E56,0x0350},	{0x1E59,0x0360},
//{0x1E5A,0x0370},	{0x1E5B,0x0380},	{0x1E5D,0x0390},	{0x1E60,0x03A0},	{0x1E63,0x03B0},//60
//{0x1E64,0x03C0},	{0x1E65,0x03D0},	{0x1E67,0x03E0},	{0x1E68,0x03F0},	{0x1E70,0x0400},
//{0x1E71,0x0410},	{0x1E72,0x0420},	{0x1E73,0x0430},	{0x1E74,0x0440},	{0x1E75,0x0450},
//{0x1E77,0x0460},	{0x1E79,0x0470},	{0x1E7A,0x0480},	{0x1E7B,0x0490},	{0x1E7C,0x04A0},
//{0x1E80,0x04B0},	{0x1E88,0x04C0},	{0x1E89,0x04D0},	{0x1E8A,0x04E0},	{0x1E8B,0x04F0},//80
//{0x1E90,0x0500},	{0x1E91,0x0510},	{0x1E92,0x0520},	{0x1E98,0x0530},	{0x1E99,0x0540},
//{0x1E9A,0x0550},	{0x1E9B,0x0560},	{0x1E9C,0x0570},	{0x1E9D,0x0580},	{0x1E9E,0x0590},
//{0x1E9F,0x05A0},	{0x1EA0,0x05B0},	{0x1EA2,0x05C0},	{0x1EA3,0x05D0},	{0x1EA5,0x05E0},
//{0x1EA7,0x05F0},	{0x1EAA,0x0600},	{0x1EAB,0x0610},	{0x1EB8,0x0620},	{0x1EB9,0x0630},//100
//{0x1EBA,0x0640},	{0x1EBB,0x0650},	{0x1EBC,0x0660},	{0x1EBD,0x0670},	{0x1EBE,0x0680},
//{0x1EBF,0x0690},	{0x1EC0,0x06A0},	{0x1EC2,0x06B0},	{0x1EC3,0x06C0},	{0x1EC5,0x06D0},
//{0x1EC7,0x06E0},	{0x1ECA,0x06F0},	{0x1ECB,0x0700},	{0x1EDE,0x0710},	{0x1F51,0x0720},
//{0x1F54,0x0730},	{0x1F58,0x0740},	{0x1F59,0x0750},	{0x1F5C,0x0760},	{0x1F5D,0x0770},//120
//{0x1F68,0x0780},	{0x1F78,0x0790},	{0x1F79,0x07A0},	{0x1F7A,0x07B0},	{0x1F87,0x07C0},
//{0x1F88,0x07D0},	{0x1F8A,0x07E0},	{0x1F98,0x07F0},	{0x2057,0x0800},	{0x205F,0x0810},
//{0x2060,0x0820},	{0x2061,0x0830},	{0x2062,0x0840},	{0x2063,0x0850},	{0x2064,0x0860},
//{0x2066,0x0870},	{0x2069,0x0880},	{0x206A,0x0890},	{0x2073,0x08A0},	{0x2074,0x08B0},//140
//{0x2075,0x08C0},	{0x207C,0x08D0},	{0x207D,0x08E0},	{0x2085,0x08F0},	{0x2086,0x0900},
//{0x20C8,0x0910},	{0x20CA,0x0920},	{0x2151,0x0930},	{0x2152,0x0940},	{0x2154,0x0950},
//{0x2155,0x0960},	{0x2156,0x0970},	{0x2157,0x0980},	{0x215B,0x0990},	{0x2161,0x09A0},
//{0x2167,0x09B0},	{0x2761,0x09C0},	{0x2765,0x09D0},	{0x2766,0x09E0},	{0x2767,0x09F0},//160
//{0x2768,0x0A00},	{0x2850,0x0A10},	{0x2851,0x0A20},	{0x2854,0x0A30},	{0x285F,0x0A40},
//{0x2860,0x0A50},	{0x2861,0x0A60},	{0x2863,0x0A70},	{0x2882,0x0A80},	{0x2883,0x0A90},
//{0x2885,0x0AA0},	{0x2886,0x0AB0},	{0x2887,0x0AC0},	{0x2888,0x0AD0},	{0x2889,0x0AE0},
//{0x288C,0x0AF0},	{0x288D,0x0B00},	{0x288E,0x0B10},	{0x288F,0x0B20},	{0x2895,0x0B30},//180
//{0x2898,0x0B40},	{0x2899,0x0B50},	{0x28A1,0x0B60},	{0x28C8,0x0B70},	{0x28C9,0x0B80},
//{0x3150,0x0B90},	{0x3151,0x0BA0},	{0x3152,0x0BB0},	{0x3153,0x0BC0},	{0x3154,0x0BD0},
//{0x3155,0x0BE0},	{0x3156,0x0BF0},	{0x3157,0x0C00},	{0x3185,0x0C10},	{0x3186,0x0C20},
//{0x3187,0x0C30},	{0x3189,0x0C40},	{0x318A,0x0C50},	{0x318B,0x0C60},	{0x318C,0x0C70},//200
//{0x318D,0x0C80},	{0x318E,0x0C90},	{0x3191,0x0CA0},	{0x3192,0x0CB0},	{0x3193,0x0CC0},
//{0x3194,0x0CD0},	{0x31A8,0x0CE0},	{0x31A9,0x0CF0},	{0x3250,0x0D00},	{0x3251,0x0D10},
//{0x325C,0x0D20},	{0x325D,0x0D30},	{0x325E,0x0D40},	{0x325F,0x0D50},	{0x5355,0x0D60},
//{0x5356,0x0D70},	{0x5357,0x0D80},	{0x5358,0x0D90},	{0x535A,0x0DA0},	{0x535B,0x0DB0},//220
//{0x535E,0x0DC0},	{0x5362,0x0DD0},	{0x539C,0x0DE0},	{0x53AC,0x0DF0},	{0x5456,0x0E00},
//{0x5457,0x0E10},	{0x5458,0x0E20},	{0x5459,0x0E30},	{0x6F67,0x0E40},	{0x6F75,0x0E50},//230
//{0x6FF4,0x0E60},	{0x6FF5,0x0E70},	{0x7854,0x0E80},	{0x7855,0x0E90} 					   //234	8*30=240

	
//	{0x1000,0x0000},	{0x1001,0x0010},	{0x1004,0x0020},	{0x1005,0x0030},	{0x1006,0x0040},	
//	{0x100E,0x0050},	{0x100F,0x0060},	{0x1108,0x0070},	{0x1109,0x0080},	{0x1136,0x0090},	
//	{0x113A,0x00A0},	{0x113B,0x00B0},	{0x113C,0x00C0},	{0x113D,0x00D0},	{0x113E,0x00E0},	
//	{0x120D,0x00F0},	{0x120E,0x0100},	{0x120F,0x0110},	{0x1210,0x0120},	{0x1211,0x0130},	//20
//	{0x1212,0x0140},	{0x1213,0x0150},	{0x1214,0x0160},	{0x1215,0x0170},	{0x1216,0x0180},	
//	{0x1217,0x0190},	{0x1218,0x01A0},	{0x1219,0x01B0},	{0x121A,0x01C0},	{0x121B,0x01D0},	
//	{0x1351,0x01E0},	{0x1C50,0x01F0},	{0x1C51,0x0200},	{0x1C52,0x0210},	{0x1C58,0x0220},		
//	{0x1C5B,0x0230},	{0x1D6A,0x0240},	{0x1D71,0x0250},	{0x1D73,0x0260},	{0x1D74,0x0270},	//40
//	{0x1D75,0x0280},	{0x1D78,0x0290},	{0x1D88,0x02A0},	{0x1D89,0x02B0},	{0x1D8A,0x02C0},	
//	{0x1D8B,0x02D0},	{0x1D8C,0x02E0},	{0x1D8D,0x02F0},	{0x1D90,0x0300},	{0x1D93,0x0310},	
//	{0x1E51,0x0320},	{0x1E53,0x0330},	{0x1E55,0x0340},	{0x1E56,0x0350},	{0x1E59,0x0360},	
//	{0x1E5A,0x0370},	{0x1E5B,0x0380},	{0x1E5D,0x0390},	{0x1E60,0x03A0},	{0x1E63,0x03B0},	//60
//	{0x1E64,0x03C0},	{0x1E65,0x03D0},	{0x1E67,0x03E0},	{0x1E68,0x03F0},	{0x1E70,0x0400},	
//	{0x1E71,0x0410},	{0x1E72,0x0420},	{0x1E73,0x0430},	{0x1E74,0x0440},	{0x1E75,0x0450},	
//	{0x1E77,0x0460},	{0x1E79,0x0470},	{0x1E7A,0x0480},	{0x1E7B,0x0490},	{0x1E7C,0x04A0},	
//	{0x1E80,0x04B0},	{0x1E88,0x04C0},	{0x1E89,0x04D0},	{0x1E8A,0x04E0},	{0x1E8B,0x04F0},	//80
//	{0x1E90,0x0500},	{0x1E91,0x0510},	{0x1E92,0x0520},	{0x1E98,0x0530},	{0x1E99,0x0540},	
//	{0x1E9A,0x0550},	{0x1E9B,0x0560},	{0x1E9C,0x0570},	{0x1E9D,0x0580},	{0x1E9E,0x0590},	
//	{0x1E9F,0x05A0},	{0x1EA0,0x05B0},	{0x1EA2,0x05C0},	{0x1EA3,0x05D0},	{0x1EA5,0x05E0},	
//	{0x1EA6,0x05F0},	{0x1EA7,0x0600},	{0x1EAA,0x0610},	{0x1EAB,0x0620},	{0x1EB8,0x0630},	//100
//	{0x1EB9,0x0640},	{0x1EBA,0x0650},	{0x1EBB,0x0660},	{0x1EBC,0x0670},	{0x1EBD,0x0680},	
//	{0x1EBE,0x0690},	{0x1EBF,0x06A0},	{0x1EC0,0x06B0},	{0x1EC2,0x06C0},	{0x1EC3,0x06D0},	
//	{0x1EC5,0x06E0},	{0x1EC7,0x06F0},	{0x1ECA,0x0700},	{0x1ECB,0x0710},	{0x1EDE,0x0720},	
//	{0x1F51,0x0730},	{0x1F54,0x0740},	{0x1F58,0x0750},	{0x1F59,0x0760},	{0x1F5C,0x0770},	//120
//	{0x1F5D,0x0780},	{0x1F68,0x0790},	{0x1F78,0x07A0},	{0x1F79,0x07B0},	{0x1F7A,0x07C0},	
//	{0x1F87,0x07D0},	{0x1F88,0x07E0},	{0x1F8A,0x07F0},	{0x1F98,0x0800},	{0x2057,0x0810},	
//	{0x205F,0x0820},	{0x2060,0x0830},	{0x2061,0x0840},	{0x2062,0x0850},	{0x2063,0x0860},	
//	{0x2064,0x0870},	{0x2066,0x0880},	{0x2069,0x0890},	{0x206A,0x08A0},	{0x2073,0x08B0},	//140
//	{0x2074,0x08C0},	{0x2075,0x08D0},	{0x207C,0x08E0},	{0x207D,0x08F0},	{0x2085,0x0900},	
//	{0x2086,0x0910},	{0x20C8,0x0920},	{0x20CA,0x0930},	{0x2151,0x0940},	{0x2152,0x0950},	
//	{0x2154,0x0960},	{0x2155,0x0970},	{0x2156,0x0980},	{0x2157,0x0990},	{0x215B,0x09A0},	
//	{0x2161,0x09B0},	{0x2167,0x09C0},	{0x2761,0x09D0},	{0x2765,0x09E0},	{0x2766,0x09F0},	//160
//	{0x2767,0x0A00},	{0x2768,0x0A10},	{0x2850,0x0A20},	{0x2851,0x0A30},	{0x2854,0x0A40},	
//	{0x285F,0x0A50},	{0x2860,0x0A60},	{0x2861,0x0A70},	{0x2881,0x0A80},	{0x2882,0x0A90},	
//	{0x2883,0x0AA0},	{0x2885,0x0AB0},	{0x2886,0x0AC0},	{0x2887,0x0AD0},	{0x2888,0x0AE0},	
//	{0x2889,0x0AF0},	{0x288C,0x0B00},	{0x288D,0x0B10},	{0x288E,0x0B20},	{0x288F,0x0B30},	//180
//	{0x2895,0x0B40},	{0x2898,0x0B50},	{0x2899,0x0B60},	{0x28A1,0x0B70},	{0x28C8,0x0B80},	
//	{0x28C9,0x0B90},	{0x3150,0x0BA0},	{0x3151,0x0BB0},	{0x3152,0x0BC0},	{0x3153,0x0BD0},	
//	{0x3154,0x0BE0},	{0x3155,0x0BF0},	{0x3156,0x0C00},	{0x3157,0x0C10},	{0x3185,0x0C20},	
//	{0x3186,0x0C30},	{0x3187,0x0C40},	{0x3188,0x0C50},	{0x3189,0x0C60},	{0x318A,0x0C70},	//200
//	{0x318B,0x0C80},	{0x318C,0x0C90},	{0x318D,0x0CA0},	{0x318E,0x0CB0},	{0x3191,0x0CC0},	
//	{0x3192,0x0CD0},	{0x3193,0x0CE0},	{0x3194,0x0CF0},	{0x31A8,0x0D00},	{0x31A9,0x0D10},	
//	{0x3250,0x0D20},	{0x3251,0x0D30},	{0x325C,0x0D40},	{0x325D,0x0D50},	{0x325E,0x0D60},	
//	{0x325F,0x0D70},	{0x535E,0x0D80},	{0x6F67,0x0D90},	{0x6F75,0x0DA0},	{0x6FF4,0x0DB0},	//220
//	{0x6FF5,0x0DC0},	{0x7854,0x0DD0},	{0x7855,0x0DE0},											//223 8*28 = 224

	{0x1000,0x0000},	{0x1001,0x0010},	{0x1004,0x0020},	{0x1005,0x0030},	{0x1006,0x0040},	
	{0x100E,0x0050},	{0x100F,0x0060},	{0x1108,0x0070},	{0x1109,0x0080},	{0x1136,0x0090},	
	{0x113A,0x00A0},	{0x113B,0x00B0},	{0x113C,0x00C0},	{0x113D,0x00D0},	{0x113E,0x00E0},	
	{0x120D,0x00F0},	{0x120E,0x0100},	{0x120F,0x0110},	{0x1210,0x0120},	{0x1211,0x0130},//20	
	{0x1212,0x0140},	{0x1213,0x0150},	{0x1214,0x0160},	{0x1215,0x0170},	{0x1216,0x0180},	
	{0x1217,0x0190},	{0x1218,0x01A0},	{0x1219,0x01B0},	{0x121A,0x01C0},	{0x121B,0x01D0},	
	{0x1351,0x01E0},	{0x1C50,0x01F0},	{0x1C51,0x0200},	{0x1C52,0x0210},	{0x1C58,0x0220},	
	{0x1C5B,0x0230},	{0x1D6A,0x0240},	{0x1D71,0x0250},	{0x1D72,0x0260},	{0x1D73,0x0270},//40	
	{0x1D74,0x0280},	{0x1D75,0x0290},	{0x1D78,0x02A0},	{0x1D88,0x02B0},	{0x1D89,0x02C0},	
	{0x1D8A,0x02D0},	{0x1D8B,0x02E0},	{0x1D8C,0x02F0},	{0x1D8D,0x0300},	{0x1D90,0x0310},	
	{0x1D93,0x0320},	{0x1DD8,0x0330},	{0x1DD9,0x0340},	{0x1DDA,0x0350},	{0x1E51,0x0360},	
	{0x1E53,0x0370},	{0x1E55,0x0380},	{0x1E56,0x0390},	{0x1E59,0x03A0},	{0x1E5A,0x03B0},//60
	{0x1E5B,0x03C0},	{0x1E5D,0x03D0},	{0x1E5E,0x03E0},	{0x1E5F,0x03F0},	{0x1E60,0x0400},	
	{0x1E63,0x0410},	{0x1E64,0x0420},	{0x1E65,0x0430},	{0x1E67,0x0440},	{0x1E68,0x0450},	
	{0x1E70,0x0460},	{0x1E71,0x0470},	{0x1E72,0x0480},	{0x1E73,0x0490},	{0x1E74,0x04A0},	
	{0x1E75,0x04B0},	{0x1E77,0x04C0},	{0x1E79,0x04D0},	{0x1E7A,0x04E0},	{0x1E7B,0x04F0},//80	
	{0x1E7C,0x0500},	{0x1E80,0x0510},	{0x1E88,0x0520},	{0x1E89,0x0530},	{0x1E8A,0x0540},	
	{0x1E8B,0x0550},	{0x1E90,0x0560},	{0x1E91,0x0570},	{0x1E92,0x0580},	{0x1E98,0x0590},	
	{0x1E99,0x05A0},	{0x1E9A,0x05B0},	{0x1E9B,0x05C0},	{0x1E9C,0x05D0},	{0x1E9D,0x05E0},	
	{0x1E9E,0x05F0},	{0x1E9F,0x0600},	{0x1EA0,0x0610},	{0x1EA2,0x0620},	{0x1EA3,0x0630},//100	
	{0x1EA5,0x0640},	{0x1EA6,0x0650},	{0x1EA7,0x0660},	{0x1EAA,0x0670},	{0x1EAB,0x0680},	
	{0x1EB8,0x0690},	{0x1EB9,0x06A0},	{0x1EBA,0x06B0},	{0x1EBB,0x06C0},	{0x1EBC,0x06D0},	
	{0x1EBD,0x06E0},	{0x1EBE,0x06F0},	{0x1EBF,0x0700},	{0x1EC0,0x0710},	{0x1EC2,0x0720},	
	{0x1EC3,0x0730},	{0x1EC5,0x0740},	{0x1EC6,0x0750},	{0x1EC7,0x0760},	{0x1ECA,0x0770},//120	
	{0x1ECB,0x0780},	{0x1ED8,0x0790},	{0x1ED9,0x07A0},	{0x1EDA,0x07B0},	{0x1EDB,0x07C0},	
	{0x1EDC,0x07D0},	{0x1EDD,0x07E0},	{0x1EDE,0x07F0},	{0x1EDF,0x0800},	{0x1EE0,0x0810},	
	{0x1EE2,0x0820},	{0x1EE3,0x0830},	{0x1EE5,0x0840},	{0x1F51,0x0850},	{0x1F54,0x0860},	
	{0x1F58,0x0870},	{0x1F59,0x0880},	{0x1F5A,0x0890},	{0x1F5C,0x08A0},	{0x1F5D,0x08B0},//140	
	{0x1F68,0x08C0},	{0x1F78,0x08D0},	{0x1F79,0x08E0},	{0x1F7A,0x08F0},	{0x1F87,0x0900},	
	{0x1F88,0x0910},	{0x1F8A,0x0920},	{0x1F98,0x0930},	{0x2057,0x0940},	{0x205F,0x0950},	
	{0x2060,0x0960},	{0x2061,0x0970},	{0x2062,0x0980},	{0x2063,0x0990},	{0x2064,0x09A0},	
	{0x2066,0x09B0},	{0x2069,0x09C0},	{0x206A,0x09D0},	{0x2073,0x09E0},	{0x2074,0x09F0},//160	
	{0x2075,0x0A00},	{0x207C,0x0A10},	{0x207D,0x0A20},	{0x2085,0x0A30},	{0x2086,0x0A40},	
	{0x20C8,0x0A50},	{0x20CA,0x0A60},	{0x2151,0x0A70},	{0x2152,0x0A80},	{0x2154,0x0A90},	
	{0x2155,0x0AA0},	{0x2156,0x0AB0},	{0x2157,0x0AC0},	{0x215B,0x0AD0},	{0x2161,0x0AE0},	
	{0x2167,0x0AF0},	{0x2761,0x0B00},	{0x2765,0x0B10},	{0x2766,0x0B20},	{0x2767,0x0B30},//180	
	{0x2768,0x0B40},	{0x2850,0x0B50},	{0x2851,0x0B60},	{0x2854,0x0B70},	{0x285F,0x0B80},	
	{0x2860,0x0B90},	{0x2861,0x0BA0},	{0x2881,0x0BB0},	{0x2882,0x0BC0},	{0x2883,0x0BD0},	
	{0x2885,0x0BE0},	{0x2886,0x0BF0},	{0x2887,0x0C00},	{0x2888,0x0C10},	{0x2889,0x0C20},	
	{0x288C,0x0C30},	{0x288D,0x0C40},	{0x288E,0x0C50},	{0x288F,0x0C60},	{0x2895,0x0C70},//200	
	{0x2898,0x0C80},	{0x2899,0x0C90},	{0x28A1,0x0CA0},	{0x28C8,0x0CB0},	{0x28C9,0x0CC0},	
	{0x3150,0x0CD0},	{0x3151,0x0CE0},	{0x3152,0x0CF0},	{0x3153,0x0D00},	{0x3154,0x0D10},	
	{0x3155,0x0D20},	{0x3156,0x0D30},	{0x3157,0x0D40},	{0x3185,0x0D50},	{0x3186,0x0D60},	
	{0x3187,0x0D70},	{0x3188,0x0D80},	{0x3189,0x0D90},	{0x318A,0x0DA0},	{0x318B,0x0DB0},//220	
	{0x318C,0x0DC0},	{0x318D,0x0DD0},	{0x318E,0x0DE0},	{0x3191,0x0DF0},	{0x3192,0x0E00},	
	{0x3193,0x0E10},	{0x3194,0x0E20},	{0x31A8,0x0E30},	{0x31A9,0x0E40},	{0x3250,0x0E50},	
	{0x3251,0x0E60},	{0x325C,0x0E70},	{0x325D,0x0E80},	{0x325E,0x0E90},	{0x325F,0x0EA0},	
	{0x535E,0x0EB0},	{0x6F67,0x0EC0},	{0x6F75,0x0ED0},	{0x6FF4,0x0EE0},	{0x6FF5,0x0EF0},//240	
	{0x7854,0x0F00},	{0x7855,0x0F10}			//242   8*31 = 248


};

SrcDataTyp  IdrTable[ ] = {
//	{0x1004,0x0000},	{0x1005,0x0010},	{0x100E,0x0020},	{0x100F,0x0030},	{0x1010,0x0040},
//	{0x1011,0x0050},	{0x1012,0x0060},	{0x1013,0x0070},	{0x101D,0x0080},	{0x101E,0x0090},
//	{0x1108,0x00A0},	{0x111D,0x00B0},	{0x1132,0x00C0},	{0x120D,0x00D0},	{0x120E,0x00E0},
//	{0x120F,0x00F0},	{0x1210,0x0100},	{0x1211,0x0110},	{0x1212,0x0120},	{0x1213,0x0130},//20
//	{0x1214,0x0140},	{0x1215,0x0150},	{0x1216,0x0160},	{0x1217,0x0170},	{0x1218,0x0180},
//	{0x1219,0x0190},	{0x121A,0x01A0},	{0x121B,0x01B0},	{0x1E03,0x01C0},	{0x1E04,0x01D0},
//	{0x1E05,0x01E0},	{0x1E06,0x01F0},	{0x1E07,0x0200},	{0x1E09,0x0210},	{0x1E0B,0x0220},
//	{0x1E0C,0x0230},	{0x1E0E,0x0240},	{0x1E0F,0x0250},	{0x1E13,0x0260},	{0x1E14,0x0270},//40
//	{0x1E15,0x0280},	{0x1E16,0x0290},	{0x1E17,0x02A0},	{0x1E18,0x02B0},	{0x1E1D,0x02C0},
//	{0x1E1E,0x02D0},	{0x1E1F,0x02E0},	{0x1E20,0x02F0},	{0x1E21,0x0300},	{0x1E23,0x0310},
//	{0x1E24,0x0320},	{0x1E26,0x0330},	{0x1E2F,0x0340},	{0x1E32,0x0350},	{0x1E35,0x0360},
//	{0x1F00,0x0370},	{0x1F01,0x0380},	{0x1F02,0x0390},	{0x1F04,0x03A0},	{0x1F06,0x03B0},//60
//	{0x1F0C,0x03C0},	{0x1F0D,0x03D0},	{0x1F0E,0x03E0},	{0x1F10,0x03F0},	{0x1F12,0x0400},
//	{0x1F15,0x0410},	{0x1F16,0x0420},	{0x1F18,0x0430},	{0x1F1B,0x0440},	{0x1F1C,0x0450},
//	{0x1F1D,0x0460},	{0x1F37,0x0470},	{0x1F38,0x0480},	{0x1F54,0x0490},	{0x2000,0x04A0},
//	{0x2001,0x04B0},	{0x2003,0x04C0},	{0x2005,0x04D0},	{0x2006,0x04E0},	{0x2007,0x04F0},//80
//	{0x2008,0x0500},	{0x2009,0x0510},	{0x200B,0x0520},	{0x200C,0x0530},	{0x2100,0x0540},
//	{0x2101,0x0550},	{0x2102,0x0560},	{0x2103,0x0570},	{0x2105,0x0580},	{0x2106,0x0590},
//	{0x2107,0x05A0},	{0x2109,0x05B0},	{0x210A,0x05C0},	{0x210C,0x05D0},	{0x2110,0x05E0},
//	{0x212E,0x05F0},	{0x212F,0x0600},	{0x3100,0x0610},	{0x3101,0x0620},	{0x3106,0x0630},//100
//	{0x3107,0x0640},	{0x3108,0x0650},	{0x3109,0x0660},	{0x310E,0x0670},	{0x310F,0x0680},
//	{0x3110,0x0690},	{0x3111,0x06A0},	{0x3116,0x06B0},	{0x3117,0x06C0},	{0x3118,0x06D0},
////	{0x3119,0x06E0},	{0x315B,0x06F0},	{0x3200,0x0700},	{0x3201,0x0710},	{0x3202,0x0720},	
////	{0x3203,0x0730},	{0x3208,0x0740},	{0x3209,0x0750},	{0x320A,0x0760},	{0x320B,0x0770},//120	
////	{0x320C,0x0780},	{0x320D,0x0790},	{0x6E34,0x07A0},	{0x7100,0x07B0},	{0x7106,0x07C0},//125	
////	{0x7300,0x07D0},	{0x7301,0x07E0},	{0x730C,0x07F0},	{0x730D,0x0800}						//129	8*17=136		
//	{0x3119,0x06E0},	{0x315A,0x06F0},	{0x315B,0x0700},	{0x3200,0x0710},	{0x3201,0x0720},
//	{0x3202,0x0730},	{0x3203,0x0740},	{0x3208,0x0750},	{0x3209,0x0760},	{0x320A,0x0770},//120
//	{0x320B,0x0780},	{0x320C,0x0790},	{0x320D,0x07A0},	{0x6E34,0x07B0},	{0x7100,0x07C0},
//	{0x7106,0x07D0},	{0x7300,0x07E0},	{0x7301,0x07F0},	{0x730C,0x0800},	{0x730D,0x0810}	//130	8*17=136

	
	{0x1004,0x0000},	{0x1005,0x0010},	{0x100E,0x0020},	{0x100F,0x0030},	{0x1010,0x0040},	
	{0x1011,0x0050},	{0x1012,0x0060},	{0x1013,0x0070},	{0x101D,0x0080},	{0x101E,0x0090},	
	{0x1108,0x00A0},	{0x111D,0x00B0},	{0x1132,0x00C0},	{0x120D,0x00D0},	{0x120E,0x00E0},	
	{0x120F,0x00F0},	{0x1210,0x0100},	{0x1211,0x0110},	{0x1212,0x0120},	{0x1213,0x0130},//20	
	{0x1214,0x0140},	{0x1215,0x0150},	{0x1216,0x0160},	{0x1217,0x0170},	{0x1218,0x0180},	
	{0x1219,0x0190},	{0x121A,0x01A0},	{0x121B,0x01B0},	{0x1E03,0x01C0},	{0x1E04,0x01D0},	
	{0x1E05,0x01E0},	{0x1E06,0x01F0},	{0x1E07,0x0200},	{0x1E09,0x0210},	{0x1E0B,0x0220},	
	{0x1E0C,0x0230},	{0x1E0E,0x0240},	{0x1E0F,0x0250},	{0x1E10,0x0260},	{0x1E13,0x0270},//40	
	{0x1E14,0x0280},	{0x1E15,0x0290},	{0x1E16,0x02A0},	{0x1E17,0x02B0},	{0x1E18,0x02C0},	
	{0x1E1D,0x02D0},	{0x1E1E,0x02E0},	{0x1E1F,0x02F0},	{0x1E20,0x0300},	{0x1E21,0x0310},	
	{0x1E22,0x0320},	{0x1E23,0x0330},	{0x1E24,0x0340},	{0x1E26,0x0350},	{0x1E27,0x0360},	
	{0x1E28,0x0370},	{0x1E29,0x0380},	{0x1E2B,0x0390},	{0x1E2C,0x03A0},	{0x1E2D,0x03B0},//60	
	{0x1E2E,0x03C0},	{0x1E2F,0x03D0},	{0x1E30,0x03E0},	{0x1E31,0x03F0},	{0x1E32,0x0400},	
	{0x1E33,0x0410},	{0x1E34,0x0420},	{0x1E35,0x0430},	{0x1E36,0x0440},	{0x1F00,0x0450},	
	{0x1F01,0x0460},	{0x1F02,0x0470},	{0x1F04,0x0480},	{0x1F06,0x0490},	{0x1F0C,0x04A0},	
	{0x1F0D,0x04B0},	{0x1F0E,0x04C0},	{0x1F10,0x04D0},	{0x1F12,0x04E0},	{0x1F15,0x04F0},//80	
	{0x1F16,0x0500},	{0x1F18,0x0510},	{0x1F1B,0x0520},	{0x1F1C,0x0530},	{0x1F1D,0x0540},	
	{0x1F37,0x0550},	{0x1F38,0x0560},	{0x1F54,0x0570},	{0x2000,0x0580},	{0x2001,0x0590},	
	{0x2003,0x05A0},	{0x2005,0x05B0},	{0x2006,0x05C0},	{0x2007,0x05D0},	{0x2008,0x05E0},	
	{0x2009,0x05F0},	{0x200B,0x0600},	{0x200C,0x0610},	{0x2100,0x0620},	{0x2101,0x0630},//100	
	{0x2102,0x0640},	{0x2103,0x0650},	{0x2105,0x0660},	{0x2106,0x0670},	{0x2107,0x0680},	
	{0x2109,0x0690},	{0x210A,0x06A0},	{0x210C,0x06B0},	{0x2110,0x06C0},	{0x212E,0x06D0},	
	{0x212F,0x06E0},	{0x3100,0x06F0},	{0x3101,0x0700},	{0x3106,0x0710},	{0x3107,0x0720},	
	{0x3108,0x0730},	{0x3109,0x0740},	{0x310E,0x0750},	{0x310F,0x0760},	{0x3110,0x0770},//120	
	{0x3111,0x0780},	{0x3116,0x0790},	{0x3117,0x07A0},	{0x3118,0x07B0},	{0x3119,0x07C0},	
	{0x315A,0x07D0},	{0x315B,0x07E0},	{0x3200,0x07F0},	{0x3201,0x0800},	{0x3202,0x0810},	
	{0x3203,0x0820},	{0x3208,0x0830},	{0x3209,0x0840},	{0x320A,0x0850},	{0x320B,0x0860},	
	{0x320C,0x0870},	{0x320D,0x0880},	{0x3D10,0x0890},	{0x3D16,0x08A0},	{0x3D21,0x08B0},//140	
	{0x3E00,0x08C0},	{0x3E01,0x08D0},	{0x3E02,0x08E0},	{0x3E03,0x08F0},	{0x3E04,0x0900},	
	{0x6E34,0x0910},	{0x7100,0x0920},	{0x7106,0x0930},	{0x7300,0x0940},	{0x7301,0x0950},//150	
	{0x730C,0x0960},	{0x730D,0x0970}						//152		8*19 =  152


};

/*
***	转译系统表位置：
***	Word:	0-23        24                      
***	Byte:	24-53       30                         8*7 
***	Bit:	54-107      54/8    =   7              7+7
*** Total:  24+30+7 = 61                           7+7=14
*** ChangeFlag:        108/8 = 14                  
*/

ParseTableType SysParseTable[] = {
//更换一个参数的位置，需要通知服务器同步更改

	{0x1000,0x00},	{0x1001,0x01},	{0x1004,0x02},	{0x1005,0x03},	{0x1C52,0x36},	
	{0x1D73,0x37},	{0x1D74,0x38},	{0x1D78,0x39},	{0x1D93,0x3A},	{0x1E53,0x3B},	
	{0x1E55,0x3C},	{0x1E56,0x3D},	{0x1E59,0x3E},	{0x1E5A,0x3F},	{0x1E5B,0x40},	
	{0x1E5D,0x41},	{0x1E60,0x42},	{0x1E64,0x43},	{0x1E67,0x44},	{0x1E68,0x45},	
	{0x1E70,0x46},	{0x1E71,0x47},	{0x1E72,0x48},	{0x1E73,0x49},	{0x1E74,0x4A},	
	{0x1E75,0x4B},	{0x1E77,0x4C},	{0x1E79,0x4D},	{0x1E7A,0x4E},	{0x1E7B,0x4F},	
	{0x1E7C,0x50},	{0x1E80,0x51},	{0x1E88,0x52},	{0x1E8A,0x53},	{0x1E8B,0x54},	
	{0x1E9B,0x55},	{0x1E9C,0x56},	{0x1E9D,0x57},	{0x1E9E,0x58},	{0x1E9F,0x59},	
	{0x1EA0,0x5A},	{0x1EA2,0x5B},	{0x1EA3,0x5C},	{0x1EA5,0x5D},	{0x1EA6,0x5E},	
	{0x1EA7,0x5F},	{0x1EAA,0x60},	{0x1EAB,0x61},	{0x1F5C,0x62},	{0x1F5D,0x63},	
	{0x1F78,0x64},	{0x1F7A,0x65},	{0x1F87,0x66},	{0x1F98,0x67},	{0x2057,0x18},	
	{0x205F,0x19},	{0x2060,0x1A},	{0x2061,0x1B},	{0x2062,0x1C},	{0x2063,0x1D},	
	{0x2064,0x1E},	{0x2066,0x1F},	{0x2069,0x20},	{0x206A,0x21},	{0x2073,0x22},	
	{0x2074,0x23},	{0x2075,0x24},	{0x20C8,0x25},	{0x20CA,0x26},	{0x2151,0x27},	
	{0x2167,0x28},	{0x285F,0x29},	{0x2860,0x2A},	{0x2861,0x2B},	{0x2881,0x2C},	
	{0x2882,0x04},	{0x2883,0x05},	{0x2885,0x2D},	{0x2886,0x2E},	{0x2887,0x2F},	
	{0x2888,0x30},	{0x2889,0x31},	{0x2895,0x32},	{0x28A1,0x33},	{0x28C8,0x34},	
	{0x28C9,0x35},	{0x3150,0x06},	{0x3151,0x07},	{0x3152,0x08},	{0x3153,0x09},	
	{0x3189,0x0A},	{0x318A,0x0B},	{0x318B,0x0C},	{0x318C,0x0D},	{0x318D,0x0E},	
	{0x318E,0x0F},	{0x3191,0x10},	{0x3192,0x11},	{0x3193,0x12},	{0x3194,0x13},	
	{0x31A8,0x14},	{0x31A9,0x15},	{0x6F67,0x68},	{0x6F75,0x69},	{0x6FF4,0x6A},	
	{0x6FF5,0x6B},	{0x7854,0x16},	{0x7855,0x17}
};

/*
***	转译内机表位置：
***	Word:	0-7         8               
***	Byte:	7-21        15              8*3
***	Bit:	22-47       26/8=4          8*4
*** Total:  8+15+4  =   27              3+4=7
*** ChangeFlag:         48/8 = 6        
*/

ParseTableType IdrParseTable[] = {
	{0x1E03,0x16},	{0x1E04,0x17},	{0x1E05,0x18},	{0x1E0B,0x19},	{0x1E0C,0x1A},
	{0x1E0E,0x1B},	{0x1E13,0x1C},	{0x1E14,0x1D},	{0x1E15,0x1E},	{0x1E18,0x1F},
	{0x1E1D,0x20},	{0x1E1F,0x21},	{0x1E26,0x22},	{0x1E35,0x23},	{0x1F00,0x24},
	{0x1F02,0x25},	{0x1F06,0x26},	{0x1F0C,0x27},	{0x1F0D,0x28},	{0x1F0E,0x29},//20
	{0x1F10,0x2A},	{0x1F12,0x2B},	{0x1F15,0x2C},	{0x1F1B,0x2D},	{0x1F1D,0x2E},
	{0x2000,0x08},	{0x2001,0x09},	{0x2003,0x0A},	{0x2005,0x0B},	{0x2006,0x0C},
	{0x2007,0x0D},	{0x2008,0x0E},	{0x2009,0x0F},	{0x2100,0x10},	{0x2101,0x11},
	{0x2102,0x12},	{0x2103,0x13},	{0x2109,0x14},	{0x3100,0x00},	{0x3101,0x01},//40
	{0x3108,0x02},	{0x3109,0x03},	{0x310E,0x04},	{0x310F,0x05},	{0x6E34,0x2F},
	{0x7106,0x15},	{0x7300,0x06},	{0x7301,0x07}									//48
};

SrcDataTyp  outErrTable[ ] = {


	{0x1D72,0x0010},	{0x1D75,0x0011},	{0x1D93,0x0012},	{0x1DD8,0x0013},	{0x1DD9,0x0014},	
	{0x1DDA,0x0015},	{0x1E53,0x0016},	{0x1E55,0x0017},	{0x1E56,0x0020},	{0x1E5A,0x0021},//10	
	{0x1E5B,0x0022},	{0x1E5D,0x0023},	{0x1E5E,0x0024},	{0x1E5F,0x0025},	{0x1E60,0x0026},	
	{0x1E63,0x0027},	{0x1E64,0x0030},	{0x1E67,0x0031},	{0x1E68,0x0032},	{0x1E70,0x0033},//20	
	{0x1E71,0x0034},	{0x1E72,0x0035},	{0x1E73,0x0036},	{0x1E74,0x0037},	{0x1E75,0x0040},	
	{0x1E7A,0x0041},	{0x1E7B,0x0042},	{0x1E80,0x0043},	{0x1E89,0x0044},	{0x1E90,0x0045},//30	
	{0x1E91,0x0046},	{0x1E92,0x0047},	{0x1E98,0x0050},	{0x1E99,0x0051},	{0x1E9A,0x0052},	
	{0x1E9B,0x0053},	{0x1E9C,0x0054},	{0x1E9D,0x0055},	{0x1E9E,0x0056},	{0x1E9F,0x0057},//40	
	{0x1EA0,0x0060},	{0x1EA2,0x0061},	{0x1EA3,0x0062},	{0x1EA5,0x0063},	{0x1EA6,0x0064},	
	{0x1EA7,0x0065},	{0x1EAA,0x0066},	{0x1EAB,0x0067},	{0x1EB8,0x0070},	{0x1EB9,0x0071},//50	
	{0x1EBA,0x0072},	{0x1EBB,0x0073},	{0x1EBC,0x0074},	{0x1EBD,0x0075},	{0x1EBE,0x0076},	
	{0x1EBF,0x0077},	{0x1EC0,0x0080},	{0x1EC2,0x0081},	{0x1EC3,0x0082},	{0x1EC5,0x0083},//60	
	{0x1EC6,0x0084},	{0x1EC7,0x0085},	{0x1ECA,0x0086},	{0x1ECB,0x0087},	{0x1ED8,0x0090},	
	{0x1ED9,0x0091},	{0x1EDA,0x0092},	{0x1EDB,0x0093},	{0x1EDC,0x0094},	{0x1EDD,0x0095},//70	
	{0x1EDE,0x0096},	{0x1EDF,0x0097},	{0x1EE0,0x00A0},	{0x1EE2,0x00A1},	{0x1EE3,0x00A2},	
	{0x1EE5,0x00A3},	{0x1EE6,0x00A4},	{0x1EEA,0x00A5},	{0x1EEB,0x00A6},	{0x1EEC,0x00A7},//80	
	{0x1F55,0x00B0},	{0x1F5A,0x00B1}		//82

};

#if DEBUG_ERROR
SrcDataTyp  indoorErrTable[ ] = {
	{0x1E09,0x0010},	{0x1E0B,0x0011},	{0x1E0C,0x0012},	{0x1E0E,0x0013},	{0x1E0F,0x0014},	
	{0x1E10,0x0015},	{0x1E13,0x0016},	{0x1E14,0x0017},	{0x1E15,0x0020},	{0x1E16,0x0021},	//10
	{0x1E20,0x0022},	{0x1E21,0x0023},	{0x1E21,0x0024},	{0x1E26,0x0025},	{0x1E27,0x0026},	
	{0x1E28,0x0027},	{0x1E29,0x0030},	{0x1E2B,0x0031},	{0x1E2C,0x0032},	{0x1E2D,0x0033},	//20
	{0x1E2E,0x0034},	{0x1E33,0x0035},	{0x1E34,0x0036},	{0x1E36,0x0037},	{0x3D10,0x0040},	
	{0x3D16,0x0041},	{0x3D21,0x0042},	{0x3E00,0x0043},	{0x3E01,0x0044},	{0x3E02,0x0045},	//30
	{0x3E03,0x0046},	{0x3E04,0x0047}																	//32
};

#else
SrcDataTyp	indoorErrTable[ ] = {
	{0x1E09,0x0010},	{0x1E0B,0x0011},	{0x1E0C,0x0012},	{0x1E0E,0x0013},	{0x1E0F,0x0014},	
	{0x1E10,0x0015},	{0x1E13,0x0016},	{0x1E14,0x0017},	{0x1E15,0x0020},	{0x1E16,0x0021},	//10
	{0x1E20,0x0022},	{0x1E21,0x0023},	{0x1E22,0x0024},	{0x1E26,0x0025},	{0x1E27,0x0026},	
	{0x1E28,0x0027},	{0x1E29,0x0030},	{0x1E2B,0x0031},	{0x1E2C,0x0032},	{0x1E2D,0x0033},	//20
	{0x1E2E,0x0034},	{0x1E33,0x0035},	{0x1E34,0x0036},	{0x1E36,0x0037},	{0x3D10,0x0040},	
	{0x3D16,0x0041},	{0x3D21,0x0042},	{0x3E00,0x0043},	{0x3E01,0x0044},	{0x3E02,0x0045},	//30
	{0x3E03,0x0046},	{0x3E04,0x0047} 																//32
};

#endif

/*
*	二级故障对应的地址 		  		-> 		具体故障点上报
*/

//SrcDataTyp SecondErrTable[] = {
//	{0x1E03,0},{0x1E05,0},{0x1E06,0},{0x1E07,0},{0x1E18,0},{0x1E30,0},{0x1E31,0},{0x1E32,0},
//};


const uint8_t TypeTable[] =
{
/**    0          1          2          3          4          5          6          7          8          9    **/
    NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , //10
    NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , BYTETYPE , BYTETYPE , BYTETYPE , BITTYPE  , //20
    WORDTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , BITTYPE ,  BITTYPE  , //30
    BITTYPE  , BITTYPE  , BYTETYPE , BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , BYTETYPE , //40
    BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , WORDTYPE , WORDTYPE , //50
    BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , //60
    NULLTYPE , BITTYPE  , BITTYPE  , BITTYPE  , BYTETYPE , BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , //70
    NULLTYPE , NULLTYPE , BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , WORDTYPE , NULLTYPE , BITTYPE  , //80
    BYTETYPE , BYTETYPE , BITTYPE  , BITTYPE  , BITTYPE  , BYTETYPE , BYTETYPE , BYTETYPE , WORDTYPE , WORDTYPE , //90
    NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , BITTYPE  , BITTYPE  , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , //100
    NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , BITTYPE  , //110
    BITTYPE  , BYTETYPE , BYTETYPE , BYTETYPE , WORDTYPE , WORDTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , //120
    BYTETYPE , NULLTYPE , NULLTYPE , BITTYPE  , BYTETYPE , BYTETYPE , WORDTYPE , NULLTYPE , NULLTYPE , NULLTYPE   //130
};

/* Table of CRC values for high–order byte */
static const uint8_t auchCRCHi[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40
} ;
 
/* Table of CRC values for low–order byte */
static const uint8_t auchCRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
	0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
	0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
	0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
	0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
	0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
	0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
	0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
	0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
	0x40
} ;

uint8_t real_ret;
uint16_t binarySearch(const PageTpye *page_data , const CanMsgType *m_Msg,uint8_t *m_Data,uint8_t param_type)
{
    uint8_t  num = 0 , type = 0 , send_flag = 0,ret = 0,check_ret=0;
    uint16_t key = 0;                                                           		
    uint16_t low=0 , mid = 0 , high= ((page_data->TableSize) & 0x7FFF) - 1;        		

    if(!(page_data->Page) || !m_Msg || !m_Data||(m_Msg->Lenth == 0)){
        return 0xFFFF;																	
    }else if((m_Msg->FunCode != 0xF7) && (m_Msg->FunCode != 0xD6) && (m_Msg->FunCode != 0xE7)){
        return 0xFFFC;
    }
	
//AA AA 88 24 08 2D 42 16 02 17 11 42 44 03 02 DE FF
    type = (TypeTable[m_Msg->ID&0x7F] > 1) ? 0: 3;//Bytetype 和 Wordtype 值为0，NULLtype 和 bittype 值为 3
    key  = ((m_Msg->ID & 0x7F) << 8) + (m_Msg->Data[0] & 0xFF);// row << 8 | column(基地址) = Address(基址)
    num  = ((m_Msg->Lenth & 0x7F) - 1) << type;			//字节数据时，长度不变,为位数据时，长度左移3位 length*8
    while(low <= high && high != 0){
        mid = (low+high) >> 1 ;
        if((page_data->Page[mid].SrcAdd >= key)&&(page_data->Page[mid].SrcAdd <= key+num)){     
            while(page_data->Page[mid].SrcAdd >= key){
                if(mid-- == 0){
                    mid = -1;
                    break;
                }
            }
            while((page_data->Page[++mid].SrcAdd < (key + num)) && (mid < (page_data->TableSize & 0x7FFF))){
            	if(param_type == OUTDOOR_PARAM){
					ACParam_Existflag.System[mid/8] |= (1<<(mid%8));//去重表外机存在标志位
					bigDataBinarySearch(&PageData[0].Page[mid],BigDataParm.System,BigDataParam_ChangeFlag.System,OUTDOOR_PARAM_EXIST);//全样本存在标志判断
				}else if(param_type == INDOOR_PARAM){
					ACParam_Existflag.Indoor[current_indoor_num][mid/8] |= (1<<(mid%8));//去重表内机存在标志
					bigDataBinarySearch(&PageData[1].Page[mid],BigDataParm.Indoor[current_indoor_num],BigDataParam_ChangeFlag.Indoor[current_indoor_num],INDOOR_PARAM_EXIST);
				}
				
                if(type == WORD_TYPE_BINARY){
                    if(m_Data[page_data->Page[mid].Value >> 4] != m_Msg->Data[(page_data->Page[mid].SrcAdd - key) + 1]){
//						if(1){
						m_Data[page_data->Page[mid].Value >> 4] = m_Msg->Data[(page_data->Page[mid].SrcAdd - key) + 1];//填充变化数据
							send_flag++;
							//只有字节数据存在降精度处理
							if(param_type == OUTDOOR_PARAM){
								bigDataBinarySearch(&PageData[0].Page[mid],BigDataParm.System,BigDataParam_ChangeFlag.System,OUTDOOR_PARAM);
							}else if(param_type == INDOOR_PARAM){
								bigDataBinarySearch(&PageData[1].Page[mid],BigDataParm.Indoor[current_indoor_num],BigDataParam_ChangeFlag.Indoor[current_indoor_num],INDOOR_PARAM);
							}
							if(var.realMonitor && g_Upload.real98flag >= 2){
								check_ret = Find96Take(page_data->Page[mid].SrcAdd, &Receive_Real_Page);
								if(check_ret){
									real_ret++;
								}
							}
#ifdef USING_USER_HABIT
						 if(!NS){
						 	if((m_Msg->CAN1IP >= INDOOR_BASE) && (isGetTime) && (hyaline_time > OPEN_BUFFER_TIME)){
								user_habit(&page_data->Page[mid], m_Data,m_Msg, &indoorinfo[current_indoor_num],NONSENCE);
						 	}
						 }
#endif
                    }
                }else if(type == BIT_TYPE_BINARY){
                    if((m_Msg->Data[((page_data->Page[mid].SrcAdd - key)>>3)+1] & (1<<((page_data->Page[mid].SrcAdd - key)%8)))>0?1:0){
                        if((m_Data[page_data->Page[mid].Value >> 4] != 1)){
//							if(1){
                            m_Data[page_data->Page[mid].Value >> 4] = 1;
							if(param_type == OUTDOOR_PARAM){
								bigDataBinarySearch(&PageData[0].Page[mid],BigDataParm.System,BigDataParam_ChangeFlag.System,param_type);
							}else if(param_type == INDOOR_PARAM){
								bigDataBinarySearch(&PageData[1].Page[mid],BigDataParm.Indoor[current_indoor_num],BigDataParam_ChangeFlag.Indoor[current_indoor_num],param_type);
							}							
		                        send_flag++;
							 if(var.realMonitor && g_Upload.real98flag >= 2){
								 check_ret = Find96Take(page_data->Page[mid].SrcAdd, &Receive_Real_Page);
								 if(check_ret){
									 real_ret++;
								 }
							 }
#ifdef USING_USER_HABIT
							 if(!NS){
							 	if((m_Msg->CAN1IP >= INDOOR_BASE) && (isGetTime) && (hyaline_time > OPEN_BUFFER_TIME)){
									user_habit(&page_data->Page[mid], m_Data,m_Msg, &indoorinfo[current_indoor_num],((page_data->Page[mid].SrcAdd - key)%8));
							 	}
							 }
#endif
                        }
                    }else{
                        if((m_Data[page_data->Page[mid].Value >> 4] != 0)){
//							if(1){
                            m_Data[page_data->Page[mid].Value >> 4] = 0;
							if(param_type == OUTDOOR_PARAM){
								bigDataBinarySearch(&PageData[0].Page[mid],BigDataParm.System,BigDataParam_ChangeFlag.System,param_type);
							}else if(param_type == INDOOR_PARAM){
								bigDataBinarySearch(&PageData[1].Page[mid],BigDataParm.Indoor[current_indoor_num],BigDataParam_ChangeFlag.Indoor[current_indoor_num],param_type);
							}
		                        send_flag++;
								if(var.realMonitor && g_Upload.real98flag >= 2){
									check_ret = Find96Take(page_data->Page[mid].SrcAdd, &Receive_Real_Page);
									if(check_ret){
										real_ret++;
									}
								}
#ifdef USING_USER_HABIT
								if(!NS){
								   if((m_Msg->CAN1IP >= INDOOR_BASE) && (isGetTime) && (hyaline_time > OPEN_BUFFER_TIME)){
									   user_habit(&page_data->Page[mid], m_Data,m_Msg, &indoorinfo[current_indoor_num],((page_data->Page[mid].SrcAdd - key)%8));
								   }
								}
#endif							
                        }
                    }
                }
            }
            if(send_flag){
                return 0;														
            }else{
                return 0xFFFD;                                                  
            }
        }else if(key > (page_data->Page[mid].SrcAdd)){
            low  = mid + 1;                                                    
        }else{
            if(mid < 1) mid = 1;
            high = mid - 1;                                                     
        }
    }
    return 0xFFFE;                                                              
}


/*******************************************************
* Function Name:    bigDataBinarySearch
* Purpose:         parse de-depulicate table translates BigData table
* Params :          
* Return:         0;
* Limitation: 	   
*******************************************************/
//const ParseType *CanParseData,
uint8_t bigDataBinarySearch(const SrcDataTyp *DataAddr,uint8_t *m_Data,uint8_t *bigdata_change_flag,uint8_t param_type){
    uint8_t low=0,mid=0,high=0;     //置当前查找区间上、下界的初值
    ParseType *CanParseData;
    if(!(DataAddr->SrcAdd) || !m_Data){
        return 0xFF;																	
    }
//内机位起始位置：22           8*4
//外机位起始位置：      54     8*7
   	if(param_type == OUTDOOR_PARAM || param_type == OUTDOOR_PARAM_EXIST){
   		CanParseData = &parseData[0];//外机转译表	
   	}else if(param_type == INDOOR_PARAM || param_type == INDOOR_PARAM_EXIST){
   		CanParseData = &parseData[1];//内机转译表
   	}
   	high = CanParseData->ParseSize - 1;

    while(low <= high && high != 0){
        mid = (low+high) >> 1 ;
        if(CanParseData->parse[mid].SrcAdd == DataAddr->SrcAdd){
            if(param_type == OUTDOOR_PARAM){

                if(CanParseData->parse[mid].ParsePlace >= OUTDOOR_BIT_SET){//位数据变化
//	                BigDataParam_ExistFlag.System[OUTDOOR_BASE_SET + (CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET)/8] |= \
//						(1<<(CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET)%8);

                
//用一位代表一个参数的变化状态，每8位位数据单独用一个字节表示，以便发送数据直接提取该字节
	                bigdata_change_flag[OUTDOOR_BASE_SET + (CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET)/8] |= \
						(1<<(CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET)%8);
//变化的值为1时：将1提取
                    if(ACParam.System[DataAddr->Value >> 4]){
                        m_Data[OUTDOOR_BIT_SET + (CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET) / 8]  |= \
                            ((ACParam.System[DataAddr->Value >> 4])   || 1)<<((CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET) % 8);
//变化的值为0时：将0提取
                    }else{
                        m_Data[OUTDOOR_BIT_SET + (CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET) / 8]  &= \
                         ~(((ACParam.System[DataAddr->Value >> 4]) || 1)<<((CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET) % 8));
                    }                   
                }else{//字节数据变化或字数据变化
//					BigDataParam_ExistFlag.System[CanParseData->parse[mid].ParsePlace /8] |= (1<<(CanParseData->parse[mid].ParsePlace%8)); //变化标志
                
    			    bigdata_change_flag[CanParseData->parse[mid].ParsePlace /8] |= (1<<(CanParseData->parse[mid].ParsePlace%8)); //变化标志
                    m_Data[CanParseData->parse[mid].ParsePlace] = ACParam.System[DataAddr->Value >> 4];
                }
            }else if(param_type == INDOOR_PARAM){
                if(CanParseData->parse[mid].ParsePlace >= INDOOR_BIT_SET){
//	                BigDataParam_ExistFlag.Indoor[current_indoor_num][INDOOR_BASE_SET + (CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET)/8] |= \
//						(1<<(CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET)%8);					

	                bigdata_change_flag[INDOOR_BASE_SET + (CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET)/8] |= \
						(1<<(CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET)%8);

                    if(ACParam.Indoor[current_indoor_num][DataAddr->Value >> 4]){
                        m_Data[INDOOR_BIT_SET + (CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET) / 8]  |= \
                            ((ACParam.Indoor[current_indoor_num][DataAddr->Value >> 4])   || 1)<<((CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET) % 8);
                    }else{
                        m_Data[INDOOR_BIT_SET + (CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET) / 8]  &= \
                        ~(((ACParam.Indoor[current_indoor_num][DataAddr->Value >> 4])    || 1)<<((CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET) % 8));
                    }                   
                }else{
//    			    BigDataParam_ExistFlag.Indoor[current_indoor_num][CanParseData->parse[mid].ParsePlace /8] |= (1<<(CanParseData->parse[mid].ParsePlace%8)); //变化标志
    			    bigdata_change_flag[CanParseData->parse[mid].ParsePlace /8] |= (1<<(CanParseData->parse[mid].ParsePlace%8)); //变化标志
                    m_Data[CanParseData->parse[mid].ParsePlace] = ACParam.Indoor[current_indoor_num][DataAddr->Value >> 4];
                }            
            }else if(param_type == OUTDOOR_PARAM_EXIST){
                if(CanParseData->parse[mid].ParsePlace >= OUTDOOR_BIT_SET){//位数据变化
	                BigDataParam_ExistFlag.System[OUTDOOR_BASE_SET + (CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET)/8] |= \
						(1<<(CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET)%8);

//                
////用一位代表一个参数的变化状态，每8位位数据单独用一个字节表示，以便发送数据直接提取该字节
//	                bigdata_change_flag[OUTDOOR_BASE_SET + (CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET)/8] |= \
//						(1<<(CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET)%8);
////变化的值为1时：将1提取
//                    if(ACParam.System[DataAddr->Value >> 4]){
//                        m_Data[OUTDOOR_BIT_SET + (CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET) / 8]  |= \
//                            ((ACParam.System[DataAddr->Value >> 4]) && 1)<<((CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET) % 8);
////变化的值为0时：将0提取
//                    }else{
//                        m_Data[OUTDOOR_BIT_SET + (CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET) / 8]  &= \
//                         ~(((ACParam.System[DataAddr->Value >> 4]) && 1)<<((CanParseData->parse[mid].ParsePlace - OUTDOOR_BIT_SET) % 8));
//                    }                   
                }else{//字节数据变化或字数据变化
					BigDataParam_ExistFlag.System[CanParseData->parse[mid].ParsePlace /8] |= (1<<(CanParseData->parse[mid].ParsePlace%8)); //变化标志
                
//    			    bigdata_change_flag[CanParseData->parse[mid].ParsePlace /8] |= (1<<(CanParseData->parse[mid].ParsePlace%8)); //变化标志
//                    m_Data[CanParseData->parse[mid].ParsePlace] = ACParam.System[DataAddr->Value >> 4];
                }            	
            }else if(param_type == INDOOR_PARAM_EXIST){
                if(CanParseData->parse[mid].ParsePlace >= INDOOR_BIT_SET){
	                BigDataParam_ExistFlag.Indoor[current_indoor_num][INDOOR_BASE_SET + (CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET)/8] |= \
						(1<<(CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET)%8);					

//	                bigdata_change_flag[INDOOR_BASE_SET + (CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET)/8] |= \
//						(1<<(CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET)%8);
//
//                    if(ACParam.Indoor[current_indoor_num][DataAddr->Value >> 4]){
//                        m_Data[INDOOR_BIT_SET + (CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET) / 8]  |= \
//                            ((ACParam.Indoor[current_indoor_num][DataAddr->Value >> 4]) && 1)<<((CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET) % 8);
//                    }else{
//                        m_Data[INDOOR_BIT_SET + (CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET) / 8]  &= \
//                        ~(((ACParam.Indoor[current_indoor_num][DataAddr->Value >> 4]) && 1)<<((CanParseData->parse[mid].ParsePlace - INDOOR_BIT_SET) % 8));
//                    }                   
                }else{
    			    BigDataParam_ExistFlag.Indoor[current_indoor_num][CanParseData->parse[mid].ParsePlace /8] |= (1<<(CanParseData->parse[mid].ParsePlace%8)); //变化标志
//    			    bigdata_change_flag[CanParseData->parse[mid].ParsePlace /8] |= (1<<(CanParseData->parse[mid].ParsePlace%8)); //变化标志
//                    m_Data[CanParseData->parse[mid].ParsePlace] = ACParam.Indoor[current_indoor_num][DataAddr->Value >> 4];
                }        				
            }
            return 0;
        }else if(CanParseData->parse[mid].SrcAdd < DataAddr->SrcAdd){
            low  = mid + 1;                                                     
        }else{
            if(mid < 1){
                 mid = 1;
            }
            high = mid - 1;                                                    
        }
    }
    return 0xFE;
}


/*******************************************************
* Function Name:    convCanData
* Purpose:          
* Params :          m_CanFrame:
*                   m_CanMsg:
* Return:         
* Limitation: 	   
*******************************************************/
void convCanData(const CanRxMsg *m_CanFrame , CanMsgType *m_CanMsg)
{
    if(!m_CanFrame || !m_CanMsg){
        return ;
    }
    m_CanMsg->ID = m_CanFrame->ExtId;
    m_CanMsg->Lenth = m_CanFrame->DLC;
    memcpy(m_CanMsg->Data , m_CanFrame->Data , m_CanFrame->DLC );
#define CANDBG 0
#if CANDBG
    printToUart(m_CanMsg);
#endif
}

void checkMachStatus(CanMsgType *p_CanMsg , UpDataType *p_ErrBuff)
{
    if(!p_CanMsg || !p_ErrBuff){
        return;
    }
    if((p_CanMsg->CAN1IP == 0x08)&&(p_CanMsg->DataType == 0x13)&&(p_CanMsg->Data[0] == 0x50)){
#ifdef DEBUG_PROJECT
        p_ErrBuff->Debug  = (p_CanMsg->Data[1] & (1<<0))?1:1;
#else
        p_ErrBuff->Debug  = (p_CanMsg->Data[1] & (1<<0))?1:0;
#endif

        p_ErrBuff->Healt  = (p_CanMsg->Data[1] & (1<<2))?1:0;
#ifdef DEBUG_ONLINE		
        p_ErrBuff->TestSig= (p_CanMsg->Data[1] & (1<<3))?1:1;
#else
		p_ErrBuff->TestSig= (p_CanMsg->Data[1] & (1<<3))?1:0;
#endif
        p_ErrBuff->IndOff = (p_CanMsg->Data[1] & (1<<5))?1:0;//所有内机全部关机标志位
    }
    if((p_CanMsg->CAN1IP == 0x08)&&(p_CanMsg->DataType == 0x1F)&&(p_CanMsg->Data[0] == 0x50)){
        p_ErrBuff->FastTest  = (p_CanMsg->Data[1] & (1<<0))?1:0;
        p_ErrBuff->DebugOver = (p_CanMsg->Data[2] & (1<<1))?1:0;
    }
}

/*******************************************************
* Function Name: 	fillErrData
* Purpose: 		
* Params :       
* Return: 		
* Limitation: 	 
*******************************************************/

uint16_t fillErrData(CanMsgType *p_CanMsg , UpDataType *p_ErrBuff)
{
    uint8_t err = 0;
    uint8_t i = 0 , check = 0;
    static uint32_t unixTime;
    
    if(!p_CanMsg || !p_ErrBuff){
        return 0;
    }else if(p_CanMsg->FunCode != 0xF7){
        return 0;
    }

    if(p_ErrBuff->RealDataLen < 2){
        p_ErrBuff->RealDataLen = 2;
    }
    if((p_ErrBuff->RealDataLen >= 2)&&(p_ErrBuff->RealDataLen < 6)){            //1KB Buff
    	packTime = 0x8000;
        unixTime = getUnixTimeStamp();
        memcpy(&p_ErrBuff->RealBuffer[2] , &unixTimeStamp , 4);
        p_ErrBuff->RealDataLen += 4;
    }
    if((p_ErrBuff->RealDataLen >= 6)&&(p_ErrBuff->RealDataLen < 800)){
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++] = (uint8_t)(getUnixTimeStamp() - unixTime);
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = 0xAA;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = 0xAA;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->Lenth | 0x80;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->FunCode;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->CAN2IP;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->CAN1IP;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->DataType;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            for(i=0;i<p_CanMsg->Lenth;i++){
                p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->Data[i];
                check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            }
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++] = check;
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++] = 0xFF;
    }else if((p_ErrBuff->RealDataLen >= 800)&&(p_ErrBuff->RealDataLen <= 1024)){
		memcpy(&p_ErrBuff->RealBuffer[0] , &p_ErrBuff->RealDataLen , 2);
//		if((var.buttonPush || var.projDebug || var.realMonitor) && g_Upload.real98flag < 2 && sendbuff_flag){
		if((var.buttonPush || var.projDebug || var.realMonitor) && g_Upload.real98flag < 2 && !g_Upload.enter_a_state && ConSta.Status){
			
			OSMutexPend(MutexSendData , 0 , &err);
	        if(SendBuffer[sendCnt].DataLen == 0){
				SendBuffer[sendCnt].data_type = 0;
	            memcpy(SendBuffer[sendCnt].DataBuf , p_ErrBuff->RealBuffer , p_ErrBuff->RealDataLen);
//                SendBuffer[sendCnt].DataLen = p_ErrBuff->RealDataLen | 0x8000;  //强制标记接收完成
	            err = OSQPost(QSemSend,(void *)&SendBuffer[sendCnt]);
	            if(OS_ERR_NONE == err){
	                SendBuffer[sendCnt].DataLen = p_ErrBuff->RealDataLen | 0x8000;  //强制标记接收完成
	                sendCnt++;
	            }else if(err == OS_ERR_Q_FULL){
					if(dbgPrintf)(*dbgPrintf)("Queue is FULL state! No OSQPend for QSemSend.......\r\n");
	            }else{
					if(dbgPrintf)(*dbgPrintf)("Lost Can Data! .... \r\n");
	            }

				if(dbgPrintf)(*dbgPrintf)("Send Data...\r\n");
	        }else{
				if(dbgPrintf)(*dbgPrintf)("Queue full...\r\n \r\n");				
	        }
			
	        if(sendCnt >= SEND_DATA_BUFF_SIZE){
	            sendCnt = 0;
	        }
			
			OSMutexPost(MutexSendData);
		}
        WriteData2Flash(p_ErrBuff,NORMAL_MODE_FILL);
        p_ErrBuff->RealDataLen  = 0;
        return 0;
    }else{
		p_ErrBuff->RealDataLen  = 0;
    }
    return 0;
}


/*******************************************************
* Function Name: 	fillErrData
* Purpose: 		
* Params :       
* Return: 		
* Limitation: 	 
*******************************************************/

uint16_t fillRealData(CanMsgType *p_CanMsg , BUFF_TYPE *real_handler)
{
    uint8_t err = 0;
    uint8_t i = 0 , check = 0;
    static uint32_t unixTime;
    
    if(!p_CanMsg || !real_handler){
        return 0;
    }else if(p_CanMsg->FunCode != 0xF7){
        return 0;
    }

    if(real_handler->DataLen < 2){
        real_handler->DataLen = 2;
    }
    if((real_handler->DataLen >= 2)&&(real_handler->DataLen< 6)){            //1KB Buff
    	real_packTime = 0x8000;
        unixTime = getUnixTimeStamp();
        memcpy(&real_handler->DataBuf[2] , &unixTimeStamp , 4);
        real_handler->DataLen+= 4;
    }
    if((real_handler->DataLen >= 6)&&(real_handler->DataLen < 800)){
            real_handler->DataBuf[real_handler->DataLen++] = (uint8_t)(getUnixTimeStamp() - unixTime);
            real_handler->DataBuf[real_handler->DataLen] = 0xAA;
            check ^= real_handler->DataBuf[real_handler->DataLen++];
            real_handler->DataBuf[real_handler->DataLen] = 0xAA;
            check ^= real_handler->DataBuf[real_handler->DataLen++];
            real_handler->DataBuf[real_handler->DataLen] = p_CanMsg->Lenth | 0x80;
            check ^= real_handler->DataBuf[real_handler->DataLen++];
            real_handler->DataBuf[real_handler->DataLen] = p_CanMsg->FunCode;
            check ^= real_handler->DataBuf[real_handler->DataLen++];
            real_handler->DataBuf[real_handler->DataLen] = p_CanMsg->CAN2IP;
            check ^= real_handler->DataBuf[real_handler->DataLen++];
            real_handler->DataBuf[real_handler->DataLen] = p_CanMsg->CAN1IP;
            check ^= real_handler->DataBuf[real_handler->DataLen++];
            real_handler->DataBuf[real_handler->DataLen] = p_CanMsg->DataType;
            check ^= real_handler->DataBuf[real_handler->DataLen++];
            for(i=0;i<p_CanMsg->Lenth;i++){
                real_handler->DataBuf[real_handler->DataLen] = p_CanMsg->Data[i];
                check ^= real_handler->DataBuf[real_handler->DataLen++];
            }
            real_handler->DataBuf[real_handler->DataLen++] = check;
            real_handler->DataBuf[real_handler->DataLen++] = 0xFF;
    }else if((real_handler->DataLen >= 800)&&(real_handler->DataLen <= 1024)){
		memcpy(&real_handler->DataBuf[0] , &real_handler->DataLen , 2);
		if(var.realMonitor && g_Upload.real98flag > 1){
			OSMutexPend(MutexSendData , 0 , &err);
	        if(SendBuffer[sendCnt].DataLen == 0){
				SendBuffer[sendCnt].data_type = 0;
	            memcpy(SendBuffer[sendCnt].DataBuf , real_handler->DataBuf , real_handler->DataLen);
                SendBuffer[sendCnt].DataLen = real_handler->DataLen | 0x8000;  //强制标记接收完成
	            err = OSQPost(QSemSend,(void *)&SendBuffer[sendCnt]);
	            if(OS_ERR_NONE == err){
	                sendCnt++;
	            }else if(err == OS_ERR_Q_FULL){
					if(dbgPrintf)(*dbgPrintf)("Queue is FULL state! No OSQPend for QSemSend.......\r\n");
	            }else{
					if(dbgPrintf)(*dbgPrintf)("Lost Can Data! .... \r\n");
	            }
	        }
			
	        if(sendCnt >= SEND_DATA_BUFF_SIZE){
	            sendCnt = 0;
	        }
			
			OSMutexPost(MutexSendData);
		}
//        WriteData2Flash(real_handler,NORMAL_MODE_FILL);
        real_handler->DataLen  = 0;
        memset(real_handler->DataBuf, 0 , 1026);
        return 0;
    }else{
		real_handler->DataLen  = 0;
    }
    return 0;
}


void WriteData2Flash(UpDataType *writrBuff,uint8_t data_mode)
{
    uint8_t err;
    if(!writrBuff){
        return ;
    }

#if 1
	switch (data_mode)
		{
			case 0:
				if(dbgPrintf)(*dbgPrintf)("Normal data fill ---->write 1k data to flash address:%X\r\n",writrBuff->writeFlashAddr);
				break;
			case 1:
				if(dbgPrintf)(*dbgPrintf)("Timeout data fill ---->write 1k data to flash address:%X\r\n",writrBuff->writeFlashAddr);
				break;
			case 2:
				if(dbgPrintf)(*dbgPrintf)("take-photo data fill ---->write 1k data to flash address:%X\r\n",writrBuff->writeFlashAddr);
				break;			
			default:
				if(dbgPrintf)(*dbgPrintf)("err data mode...\r\n");
				break;
		}
#endif
    if((writrBuff->writeFlashAddr % SECTOR_SIZE) == 0){
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_EraseSector(writrBuff->writeFlashAddr);
        OSMutexPost(MutexFlash);
//		savePowerDownInfo(&g_Upload);
    }else if(writrBuff->writeFlashAddr % CAN_WRITE_SIZE){//此处判断是否是整 K格式
        writrBuff->writeFlashAddr -= writrBuff->writeFlashAddr % CAN_WRITE_SIZE;
		if(dbgPrintf)(*dbgPrintf)("Attention: writeFlashAddr isn't a 1k-integer format!...\r\n");	
    }
    OSMutexPend(MutexFlash , 0 , &err);
	
	if(g_Upload.data_type){
	    sFLASH_WriteBuffer(writrBuff->TakeBuffer, writrBuff->writeFlashAddr , (writrBuff->TakeDataLen & 0x7FFF));
	}else{
		sFLASH_WriteBuffer(writrBuff->RealBuffer, writrBuff->writeFlashAddr , (writrBuff->RealDataLen & 0x7FFF));
	}

	writrBuff->writeFlashAddr += CAN_WRITE_SIZE;
    if(writrBuff->writeFlashAddr >= ERR_CAHCE_END_ADDR){
        writrBuff->writeFlashAddr  = 0;
		g_Upload.write_flash_cross = 1;
		if(dbgPrintf)(*dbgPrintf)("\nFlash write over 13M......at %d\r\n",unixTimeStamp);
    }
//	else{
//        writrBuff->writeFlashAddr += CAN_WRITE_SIZE;
//    }
	
    OSMutexPost(MutexFlash);
}


/*
*	查找故障点前30分钟内第一次故障点数据
*	传输从第一次拍照数据到故障点后5分钟或2分钟数据
*	data_type:0：内机；1：外机
*	retval:
*			0:参数错误;
*			1:普通96帧
*			2:拍照9A帧
*			3:存在空块，或错块
*/
//uint8_t flash_revert_flag;//标记故障起始位置是否高于故障起始位置

uint8_t ReadData2Flash(UpDataType *readBuff,uint8_t data_type)
{
    uint8_t err;
	uint32_t temp_time;
	uint32_t temp_address;
	static  uint8_t empty_block_count;
    if(!readBuff){
        return 0;
    }
//查找故障点30分钟内第一次拍照数据	
	if(!readBuff->alrd_find_start){//起始寻找故障上报起点
		temp_address = readBuff->err_address;//记录故障点地址
		if(data_type){//外机故障
			
		if(!readBuff->last_err_time){//上次不存在故障时间记录
			readBuff->last_err_addr = readBuff->readFlashAddr;//上次故障数据最后1k地址
			readBuff->last_err_time = readBuff->errPointTime + GprsParam.Out_Err_After_Time * 60;//上次故障数据最后1k地址处的记录时间
		}else{//上次存在故障时间记录
			if(readBuff->errPointTime - readBuff->last_err_time > GprsParam.OutErrTime * 60){//该次故障点时间满足和上次故障点时间的上报时间
				readBuff->last_err_addr = readBuff->readFlashAddr;//将本次故障点时间更新到上次故障点地址
				readBuff->last_err_time = readBuff->errPointTime + GprsParam.Out_Err_After_Time * 60;//将本次故障点时间更新到上次故障点时间
			}else{//该次故障点时间和上次故障点时间间隔小于故障上报时间，存在重复段
				readBuff->err_address = readBuff->last_err_addr + CAN_WRITE_SIZE;//故障上报起始地址为上次故障地址最后1k+1k
#ifdef DEBUG_ING				
				readBuff->last_err_addr = readBuff->readFlashAddr;
				readBuff->last_err_time = readBuff->errPointTime + GprsParam.Out_Err_After_Time * 60;
#endif				
				readBuff->alrd_find_start = 1;//标记已经找到起始故障点
			}
		}
		
			temp_time = readBuff->errPointTime - GprsParam.OutErrTime*60;//该次故障上报起始时间点
			while(!readBuff->alrd_find_start){//最多查找一遍Flash，且上次故障点时间间隔和该次故障点时间间隔大于故障上报时间
			//向后寻找故障上报时间点
				if(readBuff->err_address == ERR_CAHCE_START_ADDR){//向后查找到0地址时
					if(readBuff->write_flash_cross){//如果写FLash超过一轮，则尾地址存在数据
						readBuff->err_address = ERR_CAHCE_END_ADDR - CAN_WRITE_SIZE + 1;//从尾地址开始寻找
						readBuff->write_flash_cross = 0;//写一轮标记清零
//						savePowerDownInfo(readBuff);
					}else{//如果未写满过一轮
						readBuff->alrd_find_start = 1;//标记已经找到故障数据上报起始点
						readBuff->err_address = 0;//从0位置开始上报
						break;
					}
				}else{
					readBuff->err_address -= CAN_WRITE_SIZE;//继续向前查找
				}
				
			    OSMutexPend(MutexFlash , 0 , &err);
			    sFLASH_ReadBuffer((uint8_t*)&readBuff->ErrDataLen,readBuff->err_address, 2);//拿出该地址的数据
			    if(readBuff->ErrDataLen <= 1024){//数据长度正常
			        sFLASH_ReadBuffer(readBuff->ErrBuffer, readBuff->err_address, 6);//读取该数据
			        memcpy(&readBuff->readFlashTime , &readBuff->ErrBuffer[2] , 4);//获取该数据记录时间点
			    }else{//存在空块或错误数据
					if(dbgPrintf)(*dbgPrintf)("Flash error data  in ErrDataLen exist empty address!...\n");
					OSMutexPost(MutexFlash);
					empty_block_count++;//空块计数累计
					if(empty_block_count > 64){//当空块累计超过64块时
						readBuff->read_empty_over_sector = 1;//标记读到空块
						empty_block_count = 0;//空块清零
						break;
					}
					continue;//跳过空块
			    }
			    OSMutexPost(MutexFlash);
				if(readBuff->readFlashTime < temp_time){//第一次查找到故障数据上报起始点
					readBuff->err_address += CAN_WRITE_SIZE;//故障点数据向前移1K为起始地址
					readBuff->alrd_find_start = 1;//标记查找到故障数据上报起始点地址
					empty_block_count = 0;//清零空块标记
					break;
				}
			}
			if(readBuff->read_empty_over_sector){
				readBuff->err_address = temp_address;//如果空块标记超过64块，则故障起始数据起始点更新为故障点位置
				readBuff->alrd_find_start = 1;//标记找到故障数据起始点
				if(dbgPrintf)(*dbgPrintf)("read_empty_over_section...\r\n");
			}
//	        savePowerDownInfo(readBuff);
			if(dbgPrintf)(*dbgPrintf)("Already find start err point address:%X...\r\n",readBuff->err_address);
				
		}else{

				if(!readBuff->last_err_time){
					readBuff->last_err_addr = readBuff->readFlashAddr;
					readBuff->last_err_time = readBuff->errPointTime + GprsParam.In_Err_After_Time * 60;
				}else{
					if(readBuff->errPointTime - readBuff->last_err_time > GprsParam.InerrtTime * 60){
						readBuff->last_err_addr = readBuff->readFlashAddr;
						readBuff->last_err_time = readBuff->errPointTime + GprsParam.In_Err_After_Time * 60;
					}else{
						readBuff->err_address = readBuff->last_err_addr + CAN_WRITE_SIZE;
#ifdef DEBUG_ING						
						readBuff->last_err_addr = readBuff->readFlashAddr;
						readBuff->last_err_time = readBuff->errPointTime + GprsParam.In_Err_After_Time * 60;
#endif						
						readBuff->alrd_find_start = 1;
					}
				}

		
				temp_time = readBuff->errPointTime - GprsParam.InerrtTime*60;
				while(!readBuff->alrd_find_start){//最多查找一遍Flash
					if(readBuff->err_address == ERR_CAHCE_START_ADDR){
						if(readBuff->write_flash_cross){
							readBuff->err_address = ERR_CAHCE_END_ADDR - CAN_WRITE_SIZE + 1;
							readBuff->write_flash_cross = 0;
//							savePowerDownInfo(readBuff);						
						}else{
							readBuff->alrd_find_start = 1;
							readBuff->err_address = 0;
							empty_block_count = 0;
							break;
						}
					}else{
						readBuff->err_address -= CAN_WRITE_SIZE;
					}
					
					OSMutexPend(MutexFlash , 0 , &err);
					sFLASH_ReadBuffer((uint8_t*)&readBuff->ErrDataLen,readBuff->err_address, 2);
					if(readBuff->ErrDataLen <= 1024){
						sFLASH_ReadBuffer(readBuff->ErrBuffer, readBuff->err_address, 6);
						memcpy(&readBuff->readFlashTime , &readBuff->ErrBuffer[2] , 4);
					}else{
						if(dbgPrintf)(*dbgPrintf)("Flash error data  in ErrDataLen exist empty address!...\n");
						OSMutexPost(MutexFlash);
						empty_block_count++;
						if(empty_block_count > 64){
							readBuff->read_empty_over_sector = 1;
							empty_block_count = 0;
							break;
						}
						continue;
					}
					OSMutexPost(MutexFlash);
					if(readBuff->readFlashTime < temp_time){
						readBuff->err_address += CAN_WRITE_SIZE;
						readBuff->alrd_find_start = 1;
						empty_block_count = 0;
						break;
					}
				}
				if(readBuff->read_empty_over_sector){
					readBuff->err_address = temp_address;
					readBuff->alrd_find_start = 1;//标记找到故障数据起始点
					if(dbgPrintf)(*dbgPrintf)("read_empty_over_section...\r\n");					
				}
//				savePowerDownInfo(readBuff);
				if(dbgPrintf)(*dbgPrintf)("Already find start err point address:%X...\r\n",readBuff->err_address);
		}

		if(readBuff->err_address > readBuff->readFlashAddr){//当故障地址在写二轮Flash的低位，故障上报数据点在写一轮Flash的高地址
			readBuff->flag_flash_revert = 1;
		}
		savePowerDownInfo(readBuff);
	}
    OSMutexPend(MutexFlash , 0 , &err);
    sFLASH_ReadBuffer((uint8_t*)&readBuff->ErrDataLen,readBuff->err_address,2);
	if(dbgPrintf)(*dbgPrintf)("------>ErrDataLen:%d<-------\r\n",readBuff->ErrDataLen);
    sFLASH_ReadBuffer(readBuff->ErrBuffer, readBuff->err_address , (readBuff->ErrDataLen > 1024 ? 1024 : readBuff->ErrDataLen));
	OSMutexPost(MutexFlash);
    memcpy(&readBuff->readFlashTime , &readBuff->ErrBuffer[2] , 4);
#if DEBUG
	getErrorTime(&error_time_tag,readBuff->readFlashTime);

    if(dbgPrintf)(*dbgPrintf)("-------<Error-time log:><%04d/%02d/%02d %02d:%02d:%02d>-------\n" , 
                              error_time_tag.tm_year + 1900,
                              error_time_tag.tm_mon  + 1,
                              error_time_tag.tm_mday,
                              error_time_tag.tm_hour,
                              error_time_tag.tm_min,
                              error_time_tag.tm_sec);
#endif							  
	
    if((readBuff->ErrDataLen <= 1024) && (readBuff->readFlashTime != 0xFFFFFFFF)){
		if((readBuff->ErrBuffer[7] == 0xAA) && (readBuff->ErrBuffer[8] == 0xAA)){
			readBuff->err_address += CAN_WRITE_SIZE;
			if(readBuff->err_address >= ERR_CAHCE_END_ADDR){
				readBuff->err_address = 0;
			}
			return 0x01;//普通帧
		}else if(readBuff->ErrDataLen > 0){//此时为拍照数据	
			readBuff->err_address += CAN_WRITE_SIZE;
			if(readBuff->err_address >= ERR_CAHCE_END_ADDR){
				readBuff->err_address	= 0;
			}
			return 0x02;//拍照帧
		}else{
			readBuff->err_address += CAN_WRITE_SIZE;
			if(readBuff->err_address >= ERR_CAHCE_END_ADDR){
				readBuff->err_address	= 0;
			}
			return 0x03;//拍照帧
		}
    }else{
			readBuff->err_address += CAN_WRITE_SIZE;
			if(readBuff->err_address >= ERR_CAHCE_END_ADDR){
				readBuff->err_address = 0;
			}
			if(dbgPrintf)(*dbgPrintf)("----------->Empty block data in Error:%d<----------------\r\n",readBuff->readFlashTime);
			return 0x03;//Empty block 、Error block
    }
		
}

void savePowerDownInfo(UpDataType *saveData)
{
    uint8_t err;
    if(!saveData){
        return ;
    } 
    OSMutexPend(MutexFlash , 0 , &err);
    sFLASH_EraseSubSector(ERR_INFO_START_ADDR);
    sFLASH_WriteBuffer((uint8_t *)saveData , ERR_INFO_START_ADDR , sizeof(UpDataType)-G_UPLOAD_BUFFER_LEN*1024);
    OSMutexPost(MutexFlash);
    
    OSMutexPend(MutexFlash , 0 , &err);
    sFLASH_EraseSubSector(VAR_START_ADDR);
    sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
    OSMutexPost(MutexFlash);
}


/*******************************************************
* Function Name:    dec2BCD
* Purpose:          
* Params :          
* Return:           
* Limitation: 	    
*******************************************************/
uint32_t dec2BCD(uint32_t m_DEC)
{
    uint32_t m_rData = 0;
    if(m_DEC > 0xFFFF){
        m_rData = ((m_DEC/10000000%10)<<28)+((m_DEC/1000000%10)<<24)+((m_DEC/100000%10)<<20)+((m_DEC/10000%10)<<16)+
                  ((m_DEC/1000%10)<<12)+((m_DEC/100%10)<<8)+((m_DEC/10%10)<<4)+(m_DEC%10);

    }else if(m_DEC > 0xFF){
        m_rData = ((m_DEC/1000%10)<<12)+((m_DEC/100%10)<<8)+((m_DEC/10%10)<<4)+(m_DEC%10);
    }else{
        m_rData = ((m_DEC/10%10)<<4)+(m_DEC%10);
    }
    return m_rData;
}

/*******************************************************
* Function Name:    bcd2DEC
* Purpose:          
* Params :         
* Return:           
* Limitation: 	    
*******************************************************/
uint32_t bcd2DEC(uint32_t m_BCD)
{
    uint32_t m_rData = 0;
    if(m_BCD > 0x9999){
        m_rData += (((m_BCD >> 28)&0xF)%10 * 10000000) + (((m_BCD >> 24)&0xF)%10 * 1000000) + (((m_BCD >> 20)&0xF)%10 * 100000)+ (((m_BCD >> 16)&0xF)%10 * 10000);
        m_BCD &= 0xFFFF;
    }
    if(m_BCD > 0x99){
        m_rData += (((m_BCD >> 12)&0xF)%10 * 1000) + (((m_BCD >> 8)&0xF)%10 * 100);
        m_BCD &= 0xFF;
    }
    m_rData += (((m_BCD >> 4)&0xF)%10 * 10) + (m_BCD &0xF)%10;
    return m_rData;
}


void setACTime(struct tm *m_Time)
{
    uint8_t i = 0;
    CanTxMsg CanTime;
    time_t  n_Time = 0;
    CanTime.IDE = CAN_ID_EXT;
    CanTime.RTR = CAN_RTR_DATA;
    CanTime.FunCode  = 0x24;
    CanTime.CAN2IP   = 0x14;
    CanTime.CAN1IP   = 0x00;
    CanTime.DataType = 0x00;
    CanTime.DLC      = 8;
    CanTime.Data[0]  = (uint8_t)dec2BCD(m_Time->tm_year - 100);
    CanTime.Data[1]  = (uint8_t)dec2BCD(m_Time->tm_mon + 1);
    CanTime.Data[2]  = (uint8_t)dec2BCD(m_Time->tm_mday);
    CanTime.Data[3]  = (uint8_t)dec2BCD(m_Time->tm_hour);
    CanTime.Data[4]  = (uint8_t)dec2BCD(m_Time->tm_min);
    CanTime.Data[5]  = (uint8_t)dec2BCD(m_Time->tm_sec); 
    n_Time = mktime(m_Time);
    m_Time = gmtime(&n_Time);    
    CanTime.Data[6]  = (uint8_t)dec2BCD(m_Time->tm_wday);
    CanTime.Data[7]  = 0x02;
    for(i=0;i<5;i++){
        CAN_Transmit(CAN1 , &CanTime);
        OSTimeDlyHMSM(0,0,0,10);
    }
}

struct tm* getACTime(CanMsgType *p_CanMsg , struct tm *p_Time)
{
    static  struct tm s_Time = {0};
    if((p_CanMsg->CAN2IP == 0x08)&&(p_CanMsg->FunCode == 0x24)){                
        if(p_CanMsg->Data[7] & 0x02){
            if(p_CanMsg->Data[0] > 0x35)return NULL;
            if(p_CanMsg->Data[1] > 0x12)return NULL;
            if(p_CanMsg->Data[2] > 0x31)return NULL;
            if(p_CanMsg->Data[3] > 0x23)return NULL;
            if(p_CanMsg->Data[4] > 0x59)return NULL;
            if(p_CanMsg->Data[5] > 0x59)return NULL;
            if(p_CanMsg->Data[6] > 0x06)return NULL;
            s_Time.tm_year = bcd2DEC(p_CanMsg->Data[0]) + 100;
            s_Time.tm_mon  = bcd2DEC(p_CanMsg->Data[1]) - 1;
            s_Time.tm_mday = bcd2DEC(p_CanMsg->Data[2]);
            s_Time.tm_hour = bcd2DEC(p_CanMsg->Data[3]);
            s_Time.tm_min  = bcd2DEC(p_CanMsg->Data[4]);
            s_Time.tm_sec  = bcd2DEC(p_CanMsg->Data[5]);
            s_Time.tm_wday = bcd2DEC(p_CanMsg->Data[6]);
            setLocalTime(&s_Time);
            return p_Time;
        }
    }
    return NULL;
}

void findbarcode(uint8_t CAN2IP ,uint8_t CAN1IP)
{
    CanTxMsg SimData;
    SimData.DLC      = 2;
    SimData.RTR      = CAN_RTR_DATA;
    SimData.IDE      = CAN_ID_EXT;
    SimData.FunCode  = 0xF6;
    SimData.CAN1IP   = CAN1IP;
    SimData.CAN2IP   = CAN2IP;
    SimData.DataType =  0x12;
    SimData.Data[0]  =  0x0D;
    SimData.Data[1]  =  0x07;
    CAN_Transmit(CAN1 , &SimData);
    OSTimeDlyHMSM(0,0,0,50);    
    SimData.DataType =  0x12;
    SimData.Data[0]  =  0x14;
    SimData.Data[1]  =  0x06;
    CAN_Transmit(CAN1 , &SimData);
}


void writeSIMToCAN(uint8_t simStatus)
{
    CanTxMsg SimData;
    SimData.DLC      = 2;
    SimData.RTR      = CAN_RTR_DATA;
    SimData.IDE      = CAN_ID_EXT;
    SimData.FunCode  = 0xF7;
    SimData.CAN1IP   = 0x7E;
    SimData.CAN2IP   = 0x7F;
    SimData.DataType =  0x13;
    SimData.Data[0]  =  0x00;
    SimData.Data[1]  =  (simStatus > 0)?1:0;
    CAN_Transmit(CAN1 , &SimData);
}

void writeWeatherToCAN(A2_data *WeatherData)
{
    uint8_t i, j;
	CanTxMsg Data;
	if(WeatherData->valuedata.realtime_value_flag || WeatherData->valuedata.daytime_value_flag \
	   ||WeatherData->valuedata.hourtime_value_flag){
		Data.RTR      = CAN_RTR_DATA;
		Data.IDE      = CAN_ID_EXT;
		Data.FunCode  = 0xF5;
		Data.CAN1IP   = 0x7E;
		Data.CAN2IP   = 0x7F;
		Data.DataType =  0x16;
		for(j = 0; j < 3; j++){  //重复发三遍                                     
			if(WeatherData->valuedata.realtime_value_flag){
				Data.DLC      = 8;
				Data.Data[0]  = 0x01;
				memcpy(&Data.Data[1], &WeatherData->valuedata.real_time.weather_status, 7);
				CAN_Transmit(CAN1 , &Data);
				OSTimeDlyHMSM(0,0,0,20);
				Data.DLC      = 4;
				Data.Data[0]  = 0x08;
				memcpy(&Data.Data[1], &WeatherData->valuedata.real_time.wind_direction, 3);
				CAN_Transmit(CAN1 , &Data);
				OSTimeDlyHMSM(0,0,0,20);
			}
			
			if(WeatherData->valuedata.daytime_value_flag){
				for(i = 0; i < 2; i++){
					Data.DLC      = 8;
					Data.Data[0]  = i*30+0x15;
					memcpy(&Data.Data[1], &WeatherData->valuedata.databuf[i*30+22], 7);
					CAN_Transmit(CAN1 , &Data);
					OSTimeDlyHMSM(0,0,0,20);
					Data.Data[0]  = i*30+0x1C;
					memcpy(&Data.Data[1], &WeatherData->valuedata.databuf[i*30+29], 7);
					CAN_Transmit(CAN1 , &Data);
					OSTimeDlyHMSM(0,0,0,20);
					Data.DLC      = 3;
					Data.Data[0]  = i*30+0x23;
					memcpy(&Data.Data[1], &WeatherData->valuedata.databuf[i*30+36], 2);
					CAN_Transmit(CAN1 , &Data);
					OSTimeDlyHMSM(0,0,0,20);
				}
			}
			
			if(WeatherData->valuedata.hourtime_value_flag){
				for(i = 0; i < 6; i++){
					Data.DLC      = 8;
					Data.Data[0]  = i*20+0x51;
					memcpy(&Data.Data[1], &WeatherData->valuedata.databuf[i*20+83], 7);
					CAN_Transmit(CAN1 , &Data);
					OSTimeDlyHMSM(0,0,0,20);
				}
			}
			if(dbgPrintf)(*dbgPrintf)("send Weather data    !\r\n");
			OSTimeDlyHMSM(0,0,10,0);
			
	    }
		WeatherData->valuedata.realtime_value_flag = 0; //发到can总线后，清空标志
		WeatherData->valuedata.daytime_value_flag  = 0;
		WeatherData->valuedata.hourtime_value_flag = 0;
	}
}

uint8_t askWeatherData(uint8_t curSta)
{
    uint16_t res;
    static uint16_t nextTwoHour = 0;
    static uint16_t prevHour_weather = 0;
    nextTwoHour = ((f4_time)/ 7200)+ 1;
    if((nextTwoHour - prevHour_weather)){
        if(!ConSta.Status){
            if(!connectToServer()){
                while(!disConnectToServer());
                return curSta; 
            }
            if(!send89Frame(&g_TcpType)){
                while(!disConnectToServer());
                return curSta;
            }
            if(!sendF3Frame(&g_TcpType)){
                while(!disConnectToServer());
                return curSta;
            }
#if 1
            if(!send91Frame(&g_TcpType , 0xFF)){
                while(!disConnectToServer());
                return curSta;
            }        
#endif    
        }
        if(ConSta.Status){  
            res = sendA2Frame(&g_TcpType);
            if(!res){
	            prevHour_weather = nextTwoHour;//待确定
                return curSta;
            }
            prevHour_weather = nextTwoHour;
        }
    }
    return curSta;
}

uint16_t timeOutPack(UpDataType *p_ErrBuff)
{        
    uint8_t err = 0;
    if(!p_ErrBuff){
        return 0;
    }
    if(p_ErrBuff->RealDataLen > 6){
		OSMutexPend(MutexSendData , 0 , &err);		
        memcpy(&p_ErrBuff->RealBuffer[0] , &p_ErrBuff->RealDataLen , 2);
		
//		if((var.buttonPush || var.projDebug || var.realMonitor) && g_Upload.real98flag < 2 && sendbuff_flag){
		if((var.buttonPush || var.projDebug || var.realMonitor) && g_Upload.real98flag < 2 && !g_Upload.enter_a_state){
//			OSMutexPend(MutexSendData , 0 , &err);	
	        memcpy(SendBuffer[sendCnt].DataBuf , p_ErrBuff->RealBuffer , p_ErrBuff->RealDataLen);
			SendBuffer[sendCnt].data_type = 0;
	        SendBuffer[sendCnt].DataLen = p_ErrBuff->RealDataLen | 0x8000;  //强制标记接收完成
	        err = OSQPost(QSemSend,(void *)&SendBuffer[sendCnt]);
	        if(OS_ERR_NONE == err){
	            sendCnt++;
	        }
	        if(sendCnt >= SEND_DATA_BUFF_SIZE){
	            sendCnt = 0;
	        }
//			OSMutexPost(MutexSendData);
		}
        
        WriteData2Flash(p_ErrBuff,TIMEOUT_MODE_FILL);
        p_ErrBuff->RealDataLen  = 0x0000;
		OSMutexPost(MutexSendData);
        return 0;
    }
    return 0;
}


uint16_t timeOutRealSend(BUFF_TYPE *Real_Buff_Data)
{        
    uint8_t err = 0;
    if(!Real_Buff_Data){
        return 0;
    }
    if(Real_Buff_Data->DataLen > 6){
		OSMutexPend(MutexSendData , 0 , &err);		
        memcpy(&Real_Buff_Data->DataBuf[0] , &Real_Buff_Data->DataLen , 2);
		
		if(var.realMonitor && g_Upload.real98flag > 1){		
//			OSMutexPend(MutexSendData , 0 , &err);	
	        memcpy(SendBuffer[sendCnt].DataBuf , Real_Buff_Data->DataBuf , Real_Buff_Data->DataLen);
			SendBuffer[sendCnt].data_type = 0;
	        SendBuffer[sendCnt].DataLen = Real_Buff_Data->DataLen | 0x8000;  //强制标记接收完成
	        err = OSQPost(QSemSend,(void *)&SendBuffer[sendCnt]);
	        if(OS_ERR_NONE == err){
	            sendCnt++;
	        }
	        if(sendCnt >= SEND_DATA_BUFF_SIZE){
	            sendCnt = 0;
	        }
//			OSMutexPost(MutexSendData);
		}
        
//        WriteData2Flash(p_ErrBuff,TIMEOUT_MODE_FILL);
        Real_Buff_Data->DataLen  = 0x0000;
        memset(Real_Buff_Data->DataBuf, 0 , 1026);
		OSMutexPost(MutexSendData);
        return 0;
    }
    return 0;
}


#ifdef USING_USER_HABIT
uint16_t timeOutPackUser()
{        
    uint8_t err = 0;
    if(RealBuffer.DataLen > 6){
        RealBuffer.DataBuf[RealBuffer.DataLen++] = 0x20;
		RealBuffer.DataBuf[RealBuffer.DataLen++] = 0x5F;
		RealBuffer.DataBuf[RealBuffer.DataLen++] = ACParam.System[106];
        memcpy(&RealBuffer.DataBuf[0] , &RealBuffer.DataLen , 2);    
        if(!(user_data.write_addr%SECTOR_SIZE)){
            OSMutexPend(MutexFlash , 0 , &err);
            sFLASH_EraseSubSector(user_data.write_addr);   //擦除一个扇区，大小4KB
            OSMutexPost(MutexFlash);
        }
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_WriteBuffer(&RealBuffer.DataBuf[0] , user_data.write_addr, RealBuffer.DataLen);        
        OSMutexPost(MutexFlash);
        user_data.write_addr += CAN_WRITE_SIZE;
        if(user_data.write_addr >= USERHABIT_END_ADDR){
            user_data.write_addr  = USERHABIT_START_ADDR;
        }       
        RealBuffer.DataLen  = 0x0000;
    }
    return 0;
}

#endif



/*******************************************************
* Function Name:    printToUart
* Purpose:          
* Params :          
* Return:          
* Limitation: 	    
*******************************************************/
void printToUart(const CanMsgType *m_CanMsg)
{
    uint8_t i = 0 , m_Check = 0;
    m_Check = 0xAA^0xAA^(0x80|m_CanMsg->Lenth)^(m_CanMsg->FunCode)^(m_CanMsg->CAN2IP)^(m_CanMsg->CAN1IP)^(m_CanMsg->DataType);
    
    if(dbgPrintf)(*dbgPrintf)("\r\n--> AA AA %02X %02X %02X %02X %02X" , 0x80|m_CanMsg->Lenth , m_CanMsg->FunCode , m_CanMsg->CAN2IP , m_CanMsg->CAN1IP , m_CanMsg->DataType);
    for( i=0; i<m_CanMsg->Lenth; i++ ){
        m_Check ^= m_CanMsg->Data[i];
        if(dbgPrintf)(*dbgPrintf)(" %02X" , m_CanMsg->Data[i]);
    }
    if(dbgPrintf)(*dbgPrintf)(" %02X FF\r\n" , m_Check);
}
indoorinfoType indoorinfo[INDOOR_MCH_NUM];
outdoorinfoType outdoorinfo;

void write_weatherToCan(){
    CanTxMsg SimData;
    SimData.DLC      = 2;
    SimData.RTR      = CAN_RTR_DATA;
    SimData.IDE      = CAN_ID_EXT;
    SimData.FunCode  = 0xF7;
    SimData.CAN1IP   = 0x7E;
    SimData.CAN2IP   = 0x7F;
    SimData.DataType = 0x13;
    SimData.Data[0]  = 0x00;
    SimData.Data[1]  = 0;
    CAN_Transmit(CAN1 , &SimData);
}


/*****************************************
* Function Name:    CRC16_CheckSum
* Purpose:          计算标准modbus——CRC16校验
* Params :          CRC_Buff--------需要校验的数据
                    Buff_Len--------数据的长
                    crc_code--------校验码的初始值（为FFFF）当需要计算多个数据区域的校验时可以分步计算，传递此参数
* Return:           返回校验
* Limitation: 	    

******************************************/
uint16_t CRC16_CheckSum(uint8_t *CRC_Buff, uint16_t Buff_Len, uint16_t crc_code)
{
	uint8_t temp, i;
	
//	crc_code = 0xFFFF;
	
	while( Buff_Len-- )
	{
		crc_code = crc_code ^ *CRC_Buff++;
		for( i = 0; i < 8; i++ )
		{
			temp = 0x01 & crc_code;
			crc_code = crc_code >> 1;
			if( temp == 1 )
                          crc_code = crc_code ^ 0xA001;							// 0xA0001为预置多项式
		}
	}
	return( crc_code );
}

uint16_t CRC16_Check(uint8_t *puchMsg, uint16_t usDataLen)
{
     uint8_t uchCRCHi = 0xFF;                       /* 初始化高字节*/
     uint8_t uchCRCLo = 0xFF;                        /* 初始化低字节*/
     uint16_t uIndex;                                 //把CRC
    
     while (usDataLen--)                           //通过数据缓
     {
                   uIndex = uchCRCHi ^ *puchMsg++;                                     /*计算CRC */
                   uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
                   uchCRCLo = auchCRCLo[uIndex];
     }
     return (uint16_t)(uchCRCHi << 8 | uchCRCLo);
}

uint8_t XOR_Check(uint8_t* data, uint16_t len)
{
    uint8_t temp = *data;
    for(uint16_t i=1; i<len; i++){
        temp ^= *data++;
    }
    return temp;
}

uint8_t read_bigdata_data(BigData_Flash *readBigData,uint32_t start_addr){
    uint8_t err;
	if(readBigData->read_addr < readBigData->tmp_addr){
	    OSMutexPend(MutexFlash , 0 , &err);
	    sFLASH_ReadBuffer((uint8_t*)&g_Upload.SendBigDataLen, readBigData->read_addr, 2);
	    if(g_Upload.SendBigDataLen <= 1024){
	        sFLASH_ReadBuffer(g_Upload.BigDataSendBuff, readBigData->read_addr,g_Upload.SendBigDataLen);//正常读取大数据缓存区
	    }else{
                g_Upload.SendBigDataLen  = 0;
				readBigData->read_addr += CAN_WRITE_SIZE;//可能存在未知块存在 或 1小时flash缓存的第一K字节为空拍照帧
			    OSMutexPost(MutexFlash);
				return 2u;			
	    }
	    OSMutexPost(MutexFlash);
    	readBigData->read_addr += CAN_WRITE_SIZE;
		return 0;	
	}else{
		readBigData->read_addr = start_addr;
		readBigData->tmp_addr = start_addr;
		readBigData->write_addr = start_addr;
		return 3u;
	}
}

uint8_t try_time=0;

uint8_t send_bigdata_frame(uint8_t trans_mode,uint8_t mode_state){
#ifdef TEST_FRAME	
	static uint8_t res_9d = 0;
#else
	uint8_t res_9d = 0;
#endif

	uint8_t net_ret = 0;
	static uint8_t five_tmp_flag = 0;
	static uint8_t ret_9d = 1;
	five_tmp_flag = five_min_flag;

	if(!try_time){
		if(one_two_flag){
			ret_9d = read_bigdata_data(&BigDataSampleOneHours,(uint32_t)BIGDATA_SAMPLE_START_ONE_ADDR);
		}else{
			ret_9d = read_bigdata_data(&BigDataSampleTwoHours,(uint32_t)BIGDATA_SAMPLE_START_TWO_ADDR);
		}
	}

	if((!ret_9d) && (!ConSta.Status)){
			if(try_time < 2 && five_tmp_flag == five_min_flag){
				net_ret = SendDataToServer(trans_mode,mode_state);
				if(net_ret == 0x08){
					try_time = 0;
				}else{
					try_time++;
					five_tmp_flag = !five_tmp_flag;
				}
			}else if(try_time == 2){
				bigDataSample(take_photo_mode);
				try_time++;
				return 0;	
			}else if(try_time > 2){
				return 0;
			}
	}
	
	if((ConSta.Status) && (!ret_9d)){
	    if((g_Upload.BigDataSendBuff[2] == 0xF7) && (g_Upload.BigDataSendBuff[3] == 0xF7)){
	        send9FFrame(&g_TcpType, &g_Upload.BigDataSendBuff[0],g_Upload.SendBigDataLen - 8);
	    }else{
    		res_9d = send9DFrame(&g_TcpType, &g_Upload.BigDataSendBuff[0],g_Upload.SendBigDataLen - 6);
    		try_time  = 0;
    		if(res_9d){
    			bigDataSample(take_photo_mode);
    			return 0;
    		}
		}
	}
	return 10u;
}


//uint8_t outdoor_data_number = 1;
//uint16_t outdoor_data_tmp_len = 0;
////uint8_t power_switch = 0;
//
//uint8_t send_outdoor_power_data_frame(uint8_t trans_mode,uint8_t mode_state){
//	uint8_t ret = 0;
//	static uint8_t res_power = 0;
//	static uint8_t other_net_problem = 0;
//	static uint8_t tmp_state;
////	if(g_Upload.PowerDataLen > 100*outdoor_data_number || tmp_state != one_two_flag){
//			if(outdoor_data_tmp_len > 960 || tmp_state != one_two_flag){
//				if(g_Upload.PowerDataLen > 6){
//		//			power_sample_into_flash(NONSENCE,YES_SEND);
//					ret = 1;
//				}
//				if(tmp_state != one_two_flag){
//					tmp_state = one_two_flag;
//				}
//			}
//			if((ret) && (!ConSta.Status) && other_net_problem < 5){
//			    if(SendDataToServer(trans_mode,mode_state) != 0x08){
//			        other_net_problem++;
//			    }
//			}else if(other_net_problem >= 5 && other_net_problem < 20){
//				other_net_problem = 25;
//				return 0;
//			}else if(other_net_problem > 24){
//				if(tmp_state != one_two_flag){
//					other_net_problem = 0;
//					tmp_state = one_two_flag;
//				}
//				return 0;
//			}
//
//			
//			if((ConSta.Status) && (ret)){
//				power_sample_into_flash(NONSENCE,YES_SEND);		
//				res_power = send9FFrame(&g_TcpType, &g_Upload.PowerDataBuffer[0],outdoor_data_tmp_len - 6);
//				if(!res_power){
//		//			outdoor_data_number++;
//		//			if(outdoor_data_number > 8){
//		//				outdoor_data_number = 0;
//		//			}
//		//		}else{
//					outdoor_data_number = 1;
//					outdoor_data_tmp_len=0;
//					memset(g_Upload.PowerDataBuffer,0,sizeof(g_Upload.PowerDataBuffer));
//					g_Upload.PowerDataLen = 0;			
//				}		
//			}
//
//			return 10u;
//}


uint8_t sample_power_data_frame(uint16_t power_param){
	if(outdoor_power_change_flag){
		power_sample_into_flash(power_param,NO_SEND);
		outdoor_power_change_flag = 0;
	}
	return 0;
}
//每10分钟存储一次功率采集的数据
uint8_t power_sample_into_flash(uint16_t power_param,uint8_t send_flag){
		uint8_t err;
		static uint32_t time_tmp = 0;
		uint16_t time_stamp = 0;
		if(g_Upload.PowerDataLen < 4){
			g_Upload.PowerDataLen = 4;
		}
		if((g_Upload.PowerDataLen >= 4)&&(g_Upload.PowerDataLen < 8)){
//			power_packtime =0x8000;
	        memcpy(&g_Upload.PowerDataBuffer[4], &unixTimeStamp , 4);
	        g_Upload.PowerDataLen += 4;
			time_tmp = unixTimeStamp;//时间基准
		}
		
		if((g_Upload.PowerDataLen >= 8) && (g_Upload.PowerDataLen < 1000) && !send_flag){
			time_stamp = (uint16_t)(unixTimeStamp - time_tmp);
			memcpy(&g_Upload.PowerDataBuffer[g_Upload.PowerDataLen],&time_stamp,2);
			g_Upload.PowerDataLen += 2;
			memcpy(&g_Upload.PowerDataBuffer[g_Upload.PowerDataLen],&power_param,2);
			g_Upload.PowerDataLen += 2;
		}else if((g_Upload.PowerDataLen >= 1000) && (g_Upload.PowerDataLen <= 1024) || send_flag){
			memcpy(&g_Upload.PowerDataBuffer[0],&g_Upload.PowerDataLen,2);
		    g_Upload.PowerDataBuffer[2] = 0xF7;
		    g_Upload.PowerDataBuffer[3] = 0xF7;
//			outdoor_data_tmp_len = g_Upload.PowerDataLen;
            if(!one_two_flag){
//                if(tran_mode && open_up_take_photo && !bigdata_clear_buffer){
//                    OSMutexPend(MutexFlash , 0 , &err);
//                    sFLASH_WriteBuffer(&g_Upload.BigDataSaveBuffer[0], (uint32_t)BIGDATA_SAMPLE_START_ONE_ADDR, g_Upload.BigDataLen);        
//    //                  OSMutexPost(MutexFlash);
//                    OSMutexPost(MutexFlash);
//                    g_Upload.BigDataLen = 0;
//                    tran_mode = 0;
//                    bigdata_take_photo_over = 0;                
//                }else{
    //                  BigDataSampleTwoHours.tmp_addr = BigDataSampleTwoHours.write_addr;
    //                  BigDataSampleTwoHours.write_addr = BIGDATA_SAMPLE_START_TWO_ADDR;
                    if(!(BigDataSampleOneHours.write_addr % SUB_SECTOR_SIZE)){
                        if(BigDataSampleOneHours.write_addr >= BIGDATA_SAMPLE_END_ONE_ADDR){
                            BigDataSampleOneHours.write_addr = BIGDATA_SAMPLE_START_ONE_ADDR;
                        }
                        OSMutexPend(MutexFlash , 0 , &err);
                        sFLASH_EraseSubSector(BigDataSampleOneHours.write_addr);         //擦除一个扇区，大小4KB
                        OSMutexPost(MutexFlash);
                        if(BigDataSampleOneHours.write_addr == BIGDATA_SAMPLE_START_ONE_ADDR){
                            BigDataSampleOneHours.write_addr += CAN_WRITE_SIZE;
                        }
                    }
                    OSMutexPend(MutexFlash , 0 , &err);
                    sFLASH_WriteBuffer(&g_Upload.PowerDataBuffer[0], BigDataSampleOneHours.write_addr, g_Upload.PowerDataLen);        
                    OSMutexPost(MutexFlash);
                    
                    BigDataSampleOneHours.write_addr += CAN_WRITE_SIZE;
                    
                    BigDataSampleOneHours.tmp_addr = BigDataSampleOneHours.write_addr;
    //                  BigDataSampleOneHours.write_addr = BIGDATA_SAMPLE_START_ONE_ADDR;
                    
                    if(BigDataSampleOneHours.write_addr >= BIGDATA_SAMPLE_END_ONE_ADDR){
                        BigDataSampleOneHours.write_addr = BIGDATA_SAMPLE_START_ONE_ADDR;
                        is_spill_flag = 1;
                    }
//                }
            }else{
//                if(tran_mode && open_up_take_photo && !bigdata_clear_buffer){
//                    OSMutexPend(MutexFlash , 0 , &err);
//                    sFLASH_WriteBuffer(&g_Upload.BigDataSaveBuffer[0], (uint32_t)BIGDATA_SAMPLE_START_TWO_ADDR, g_Upload.BigDataLen);        
//                    OSMutexPost(MutexFlash);
//                    g_Upload.BigDataLen = 0;
//                    tran_mode = 0;
//                    bigdata_take_photo_over = 0;                    
//                }else{
    //                  BigDataSampleOneHours.tmp_addr = BigDataSampleOneHours.write_addr;
    //                  BigDataSampleOneHours.write_addr = BIGDATA_SAMPLE_START_ONE_ADDR;
                    if(!(BigDataSampleTwoHours.write_addr % SUB_SECTOR_SIZE)){
                        if(BigDataSampleTwoHours.write_addr >= BIGDATA_SAMPLE_END_TWO_ADDR){
                            BigDataSampleTwoHours.write_addr = BIGDATA_SAMPLE_START_TWO_ADDR;
                        }
                        OSMutexPend(MutexFlash , 0 , &err);
                        sFLASH_EraseSubSector(BigDataSampleTwoHours.write_addr);         //擦除一个扇区，大小4KB
                        OSMutexPost(MutexFlash);
                        if(BigDataSampleTwoHours.write_addr == BIGDATA_SAMPLE_START_TWO_ADDR){
                            BigDataSampleTwoHours.write_addr += CAN_WRITE_SIZE;
                        }
                    }
                    OSMutexPend(MutexFlash , 0 , &err);
                    sFLASH_WriteBuffer(&g_Upload.PowerDataBuffer[0], BigDataSampleTwoHours.write_addr, g_Upload.PowerDataLen);        
                    OSMutexPost(MutexFlash);
                    
                    BigDataSampleTwoHours.write_addr += CAN_WRITE_SIZE;

                    BigDataSampleTwoHours.tmp_addr = BigDataSampleTwoHours.write_addr;
    //                  BigDataSampleTwoHours.write_addr = BIGDATA_SAMPLE_START_TWO_ADDR;                   
                    
                    if(BigDataSampleTwoHours.write_addr >= BIGDATA_SAMPLE_END_TWO_ADDR){
                        BigDataSampleTwoHours.write_addr = BIGDATA_SAMPLE_START_TWO_ADDR;
                    }   
//                }
            }   

			g_Upload.PowerDataLen = 0;
		}else if(g_Upload.PowerDataLen > 1024){
			g_Upload.PowerDataLen = 0;
		}
		
	    return 0;
}

#ifdef USING_USER_HABIT
uint8_t send_user_habit_data_frame(uint8_t trans_mode,uint8_t mode_state){
	uint8_t res = 0;
	res = read_user_data();
	if((!res) && (!ConSta.Status)){
		SendDataToServer(trans_mode,mode_state);
	}

	
	if((ConSta.Status) && (!res)){
		send9BFrame(&g_TcpType, &g_Upload.ErrBuffer[0],g_Upload.ErrDataLen - 6,NONSENCE,NONSENCE);
	}
	return mode_state;
}
#endif



void bubble_sort(Real_PageTpye *rcv_monitor_page)
{
    uint16_t i, j, temp;
	uint8_t hlflag;
    for (j = 0; j < rcv_monitor_page->cnt ; j++)
        for (i = 0; i < rcv_monitor_page->cnt - 1 - j; i++){
            if(rcv_monitor_page->SrcAdd[i] > rcv_monitor_page->SrcAdd[i+1]){
                temp = rcv_monitor_page->SrcAdd[i];
				hlflag = rcv_monitor_page->hl_flag[i];
                rcv_monitor_page->SrcAdd[i] = rcv_monitor_page->SrcAdd[i+1];
				rcv_monitor_page->hl_flag[i] = rcv_monitor_page->SrcAdd[i+1];
                rcv_monitor_page->SrcAdd[i + 1] = temp;
				rcv_monitor_page->hl_flag[i+1] = hlflag;
            }
        }
}







