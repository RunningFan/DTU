/*********************************************************************************
* File Name          : ProgramUpdate_Common.h
* Author             : zhgx	
* Version            : V1.0
* Date               : 2015-05-06
* Description		 : binfile处理
* Last Modefly Date	 : 
*********************************************************************************/

#ifndef IDUODU_PROGRAMUPDATE_H
#define IDUODU_PROGRAMUPDATE_H

#include "os_cpu.h"
#include "duplicate.h"
#include "update_machine.h"
#include "common.h"
#include "fsm.h"


#define TRASMIT_MAIL_0		0x00	//发送邮箱1
#define TRASMIT_MAIL_1		0x01	//发送邮箱2
#define TRASMIT_MAIL_2		0x02	//发送邮箱3
#define TRASMIT_MAIL_NO		0x04	//no mail empty


#define    PROGRAMME_TYPE_CONTROLLER	0x00//程序类型:调试器
#define    PROGRAMME_TYPE_IDU			0x01//程序类型:内机
#define    PROGRAMME_TYPE_ODU			0x02//程序类型:外机


#define    PROGRAMME_NAME_STRING_LEN_DEBUGGER		45//程序名最长字符(含路径)
#define    PROGRAMME_NUM_MAX_DEBUGGER				8//程序个数

#define    PROGRAMME_SIZE_MAX					0x80000//程序大小512KBYtes
//extern INT8U Programme_Data[PROGRAMME_SIZE_MAX];



//内机预留3M空间，512KB * 6 个程序
#define  BIN_FILE_IDU_SECTOR_START   		184 //
#define  BIN_FILE_IDU_SECTOR_END   			191 //
#define  BIN_FILE_IDU_SECTOR_NUM   			8 // 8*(16*16*256) = 512Kbytes


//外机预留1M空间 512KB * 2个程序
#define  BIN_FILE_ODU_SECTOR_START   		232 //
#define  BIN_FILE_ODU_SECTOR_END   			239 //
#define  BIN_FILE_ODU_SECTOR_NUM   			8 // 8*(16*16*256) = 512Kbytes


//调试器预留1M空间 512KB * 2个程序
#define  BIN_FILE_DEBUGGER_SECTOR_START   		240 //
#define  BIN_FILE_DEBUGGER_SECTOR_END   		247 //
#define  BIN_FILE_DEBUGGER_SECTOR_NUM   		8 // 8*(16*16*256) = 512Kbytes


////文件修改时间
//typedef struct File_Date_Time_Modify
//{
//	INT16U year;//FININFO.fdate.[15:9]+1980(origin)
//	INT8U month;//FININFO.fdate.[8:5]
//	INT8U day;//FININFO.fdate.[4:0]
//	INT8U hour;//FININFO.ftime.[15:11]
//	INT8U minute;//FININFO.ftime.[10:5]
//	INT8U second;//FININFO.ftime.[4:0]
//}FILE_DATE_TIME_MODIFY;

////程序信息
//typedef struct Programme_Infomation
//{
//	char  programme_name[PROGRAMME_NAME_STRING_LEN_DEBUGGER];//程序名称
//	INT32U programme_size;//程序大小，字节数
//	INT32U programme_checksum;//程序校验码
//	FILE_DATE_TIME_MODIFY file_modify_time;//程序修改时间
//}PROGRAMME_INFO;



//extern PROGRAMME_INFO   Controller_Programme_Info[PROGRAMME_NUM_MAX_DEBUGGER];
extern INT8U Controller_Programme_Num;
extern INT8U Controller_Programme_Index;

//extern INT8U  Programme_Update_Type;


extern INT8U  Programme_Read_Enable_Controller;//程序读允许
extern INT8U  Programme_Write_Enable_Controller;//程序写SPI FLASH允许
extern INT8U  Programme_Write_Complete_Controller;//程序写SPI FLASH完成

extern INT8U  Programme_Read_Enable_IDU;//程序读允许(内机)
extern INT8U  Programme_Write_Enable_IDU;//程序写SPI FLASH允许(内机)
extern INT8U  Programme_Write_Complete_IDU;//程序写SPI FLASH完成(内机)


extern INT8U  Programme_Read_Enable_ODU;//程序读允许(外机)
extern INT8U  Programme_Write_Enable_ODU;//程序写SPI FLASH允许(外机)
extern INT8U  Programme_Write_Complete_ODU;//程序写SPI FLASH完成(外机)


#define    PROGRAMME_NAME_STRING_LEN_IDUODU		40//程序名最长字符(含路径)
#define    PROGRAMME_NUM_MAX_IDUODU				8//程序个数



////内外机程序信息
//typedef struct IDUODU_ProgrammeUpdate_Information
//{
//	INT8U head;//头
//	INT8U attribution;//属性
//	char  programme_name[PROGRAMME_NAME_STRING_LEN_IDUODU];//程序名称
//	INT32U programme_size_byte;//程序大小，字节数
//	INT16U programme_size_page; //程序大小:page
//	INT16U programme_size_iap_1st; //程序大小:IAP升级 一级索引
//	INT32U programme_checksum;//程序校验码
//	FILE_DATE_TIME_MODIFY file_modify_time;//程序修改时间
//	INT16U struct_checksum;//该数据块校验码
//}IDUODU_PROGRAMMEUPDATE_INFO;

//#define   STRUCTSIZE_IDUODU_IAP_INFO          sizeof(IDUODU_PROGRAMMEUPDATE_INFO)


//extern INT8U BinFile_IDU_Read_Complete;//内机bin文件读取OK
//extern INT8U BinFile_ODU_Read_Complete;//内机bin文件读取OK

extern INT8U Controller_Call_IDU;// 点名内机
extern INT8U Controller_Call_ODU;//点名外机




//extern IDUODU_PROGRAMMEUPDATE_INFO    IDU_ProgramUpdate_InfoRead[PROGRAMME_NUM_MAX_IDUODU];
//extern  IDUODU_PROGRAMMEUPDATE_INFO    ODU_ProgramUpdate_InfoRead[PROGRAMME_NUM_MAX_IDUODU];
//extern IDUODU_PROGRAMMEUPDATE_INFO    IDU_ProgramUpdate_InfoMemory;
//extern IDUODU_PROGRAMMEUPDATE_INFO    ODU_ProgramUpdate_InfoMemory;


//
//extern INT8U IDU_Programme_Num;//内机程序总数
//extern INT8U IDU_Programme_Index;//内机程序列表序号
//
//extern INT8U ODU_Programme_Num;//外机程序总数
//extern INT8U ODU_Programme_Index;//外机程序列表序号


//extern INT8U IDU_Device_Attribute;//内机属性
//extern INT8U IDU_IAP_Startup;//内机程序升级开始

//#define  BINFILE_SIZE_MAX			0x80000 //预留512KB空间


//#define  BINFILE_NUM_IDU			1 //内机文件个数
//#define  BINFILE_NUM_ODU			1 //外机文件个数
//
//#define  BIN_FILENAME_SIZE			30 //bin 文件名长度



//extern INT8U BinFile_Buffer_IDU[BINFILE_SIZE_MAX];
//extern INT8U BinFile_Buffer_ODU[BINFILE_SIZE_MAX];


//extern INT8U* Ptr_Controller_Bin_File_Data_IDU[];
//extern INT8U* Ptr_Controller_Bin_File_Data_ODU[];



//IAP相关
//#define IAP_IDU									0x01//
//#define IAP_ODU									0x02//

#ifdef DEBUG_ELF_VERSION
#define CONTROLLER_MASTER_CALLOVER					0x8E//上位机点名					上位机点名
#define CONTROLLER_SLAVE_ACK						0x88//公布自身地址				公布自身地址
#define CONTROLLER_SLAVE_DATA_RX_READY				0x81//数据接收准备 				准备接收
#define CONTROLLER_MASTER_DATA_TX					0x8F//数据发送		   			代码数据
#define SOFTWARE_ERASE_START						0x84//开始擦除		   			开始擦除
#define CONTROLLER_STARTUP_DEVICE					0x82//烧录器启动机组运行				烧写完成
#else
#define CONTROLLER_MASTER_CALLOVER					0x65//上位机点名					上位机点名
#define CONTROLLER_SLAVE_ACK						0x64//公布自身地址				公布自身地址
#define CONTROLLER_SLAVE_DATA_RX_READY				0x61//数据接收准备 				准备接收
#define CONTROLLER_MASTER_DATA_TX					0x66//数据发送		   			代码数据
#define SOFTWARE_ERASE_START						0x63//开始擦除		   			开始擦除
#define CONTROLLER_STARTUP_DEVICE					0x62//烧录器启动机组运行				烧写完成
#endif

#define UPDATE_CALL_OVER_DEVICE						0x00//点名设备
#define UPDATE_SEND_ERASE_COMMAND					0x01//下发擦除指令
#define UPDATE_SEND_WAIT_STT						0x02//发送等待状态
#define UPDATE_SEND_DATA_STT						0x03//发送数据状态


#define AUTO_TRANSIT_LIMIT							0x32	//重传100次 合计为1S
#define AUTO_CHECK_LIMIT							0x0A	//重传100次 合计为1S

#define DELAY_MS_SET								0x01	//设置重发间隔

#define ACK_ERASE_SECTOR					0x00	//擦除应答回复
#define ACK_RCV_ALL_CODE					0xFF	//接收完成可以跳转回复

#define IAP_DEVICE_ACK			0x03	//重试三次后退出

//设备类型
#define DEVICE_TYPE_IDU					0xF7//设备类型:内机
#define DEVICE_TYPE_ODU					0x7F//设备类型:外机

//外机
#define DEVICE_ATTRIBUTION_ODU						0x00 //设备属性:外机

//内机

#define   DEVICE_ATTRIBUTION_IDU_NUM                                  9//内机属性数量

#define DEVICE_ATTRIBUTION_IDU_GENERAL					1 //设备属性:内机(通用内机)
#define DEVICE_ATTRIBUTION_IDU_WALL						2 //设备属性:内机(壁挂内机)
#define DEVICE_ATTRIBUTION_IDU_FRESH					3 //设备属性:内机(新风内机)
#define DEVICE_ATTRIBUTION_IDU_SECRECY2					4 //设备属性:内机(保密2内机)
#define DEVICE_ATTRIBUTION_IDU_MV2FANPIPE				5 //设备属性:内机(MV2风管)
#define DEVICE_ATTRIBUTION_IDU_MV2SECRECY2				6 //设备属性:内机(MV2保密2)
#define DEVICE_ATTRIBUTION_IDU_EXTDRIVER					7 //设备属性:内机(外置驱动内机)

#define DEVICE_ATTRIBUTION_IDU_HOTWATER_GENERATOR		50 //设备属性:内机(热水发生器)
#define DEVICE_ATTRIBUTION_IDU_HOTWATER_TANK			51 //设备属性:内机(热水转接器)


//一次发送1024字节；
#define FIRST_DEGREE_INDEX_MIN				0x00//一级索引 最小值
#define FIRST_DEGREE_INDEX_MAX				0x200//一级索引 最大值  (暂预留512K FLASH)


//1024字节内索引，每帧最多8字节
#define SECOND_DEGREE_INDEX_MIN				0x00//二级索引 最小值
#define SECOND_DEGREE_INDEX_MAX				0x80//二级索引 最大值 (1024字节代码+2字节CRC校验值)

#define BYTE_NUM_PER_PAGE_IAP			0x400//每页1024字节


#define  DEVICE_NUM_MAX_IAP				4//iap设备最大数量
//#define  DEVICE_ACK_TIMEOUT				30//iap设备应答超时 3S
#define  DEVICE_ACK_TIMEOUT				30//iap设备应答超时 3S
//#define  IAP_EXIT_TIMEOUT				100//iap设备应答超时 10S
#define  IAP_EXIT_TIMEOUT				10//iap设备应答超时 10S
#define  IAP_ESC_TIMEOUT				100//iap返回计时超时10S
#define  CAN_SWITCH_DELAY				100//CAN从IAP切换为调试器模式延时时间10S
#define  IAP_STARTUP_TIMECNT			50//IAP开始升级计时

#define UPDATE_ONLINE_COUNT		1		//应答台数需求


typedef struct IAP_Info_Status_ODU
{
	INT8U IAP_Enable;
	INT8U Device_Type;//设备类型
	INT8U Device_Attribute;//设备属性
	INT8U IAP_Step_Index;//烧录步骤
	INT8U Master_Called;//上位机点名
	INT8U Handshake_OK;//握手成功
	INT8U Erased;//擦除命令
	INT8U Data_Tx_Enable;//数据发送允许
	INT8U IAP_Startup;//开始升级允许
	INT8U IAP_Cycle;//IAP 轮数计数
	INT16U IAP_Startup_TimeCnt;//IAP启动计时
	INT16U ACK_Timeout;//应答超时
	INT16U IAP_Timeused;//烧录用时
	INT16U IAP_Enter_TimeCnt;//进入IAP页面计时
	INT16U CAN_Swintch_Delay;//CAN切换延时
	INT8U IAP_Device_Current;//当前类型、属性设备数量
	INT8U IAP_Device_ACK;//当前设备应答数量
	INT8U IAP_Device_Total_Online;//累计设备数
	INT8U IAP_Device_Total_ProgramOK;//累计烧录设备数
	INT16U MAC_Addr[DEVICE_NUM_MAX_IAP][3];//[0]MAC地址；[1]是否应答
	INT16U Data_Tx_Integral_Num;// 文件发送次数
	INT16U Data_Tx_Residual;//文件发送余数
	INT16U CRC_CheckSum;//当前1024个数据CRC校验码
	INT16U Data_Tx_First_Degree_Index;//数据发送一级索引
	INT8U Data_Tx_Second_Degree_Index;//数据发送二级索引
	INT8U* Ptr_BinFile_Buf;//binfile 文件缓冲区指针

}IAP_INFO_STATUS;

extern const INT8U IDU_DeviceAttribution_Name_English[DEVICE_ATTRIBUTION_IDU_NUM][32];
extern const INT16U IDU_DeviceAttribution_Name_Chinese[DEVICE_ATTRIBUTION_IDU_NUM][12]; 



extern IAP_INFO_STATUS IAP_Information_Status;
extern INT8U StartUp_Device;//启动机组



//extern INT8U IDUODU_StartUp_TimerCnt;

//extern void Programme_Info_Init_Debugger(void);
//extern void Programme_Info_Init_IDU(void);
//extern void Programme_Info_Init_ODU(void);

//extern void Programme_Data_Init(INT8U programme_type);

extern void IAP_Info_Status_Init(void);



void App_Task_Can_Rx(CanMsgType *pdata);
void App_Task_Can_Tx();





uint8_t Can_TransmitData_Fill(CanMsgType *CAN_Msg,uint8_t type);

//void Can_TransmitData_Task(void);


void Can_ReceiveData_IAP_Device_Ack(CanMsgType*ptr_can_msg);
void Can_ReceiveData_IAP_Device_RxReady(CanMsgType * ptr_can_msg);
void BinFile_Tx_IDU_ODU(void);
void Can_SendData_StartUp_Device(INT8U device_type,INT8U device_attribute);


#endif


