/*********************************************************************************
* File Name          : App_ProgramUpdate.c
* Author             : zhgx	
* Version            : V1.0
* Date               : 2016-02-23
* Description		 : binfile 处理
* Last Modefly Date	 : 
*********************************************************************************/


#include "ucos_ii.h"
#include "ProgramUpdate_Common.h"



//#pragma arm section zidata = "VRAM"
////INT8U Programme_Data[PROGRAMME_SIZE_MAX];
//INT8U BinFile_Buffer_IDU[BINFILE_SIZE_MAX];
//INT8U BinFile_Buffer_ODU[BINFILE_SIZE_MAX];

//PROGRAMME_INFO   Controller_Programme_Info[PROGRAMME_NUM_MAX_DEBUGGER];
//IDUODU_PROGRAMMEUPDATE_INFO    IDU_ProgramUpdate_InfoRead[PROGRAMME_NUM_MAX_IDUODU];
//IDUODU_PROGRAMMEUPDATE_INFO    ODU_ProgramUpdate_InfoRead[PROGRAMME_NUM_MAX_IDUODU];
//
//IDUODU_PROGRAMMEUPDATE_INFO    IDU_ProgramUpdate_InfoMemory;
//IDUODU_PROGRAMMEUPDATE_INFO    ODU_ProgramUpdate_InfoMemory;
//#pragma arm section zidata




//INT8U BinFile_IDU_Read_Complete;//内机bin文件读取OK
//INT8U BinFile_ODU_Read_Complete;
IAP_INFO_STATUS IAP_Information_Status;
INT8U StartUp_Device = 0;//启动机组

//INT8U  Programme_Update_Type = 0;

//INT8U  Programme_Read_Enable_Controller = 0;//程序读允许(本机)
//INT8U  Programme_Write_Enable_Controller = 0;//程序写SPI FLASH允许(本机)
//INT8U  Programme_Write_Complete_Controller = 0;//程序写SPI FLASH完成(本机)


//INT8U  Programme_Read_Enable_IDU = 0;//程序读允许(内机)
//INT8U  Programme_Write_Enable_IDU = 0;//程序写SPI FLASH允许(内机)
//INT8U  Programme_Write_Complete_IDU = 0;//程序写SPI FLASH完成(内机)

//
//INT8U  Programme_Read_Enable_ODU = 0;//程序读允许(外机)
//INT8U  Programme_Write_Enable_ODU = 0;//程序写SPI FLASH允许(外机)
//INT8U  Programme_Write_Complete_ODU = 0;//程序写SPI FLASH完成(外机)



//INT8U Controller_Programme_Num = 0;//调试器程序总数
//INT8U Controller_Programme_Index = 0;//调试器程序列表序号
//
//
//INT8U IDU_Programme_Num = 0;//内机程序总数
//INT8U IDU_Programme_Index = 0;//内机程序列表序号
//
//INT8U ODU_Programme_Num = 0;//外机程序总数
//INT8U ODU_Programme_Index = 0;//外机程序列表序号
//
//INT8U IDU_Device_Attribute = 0x00;//内机属性
//INT8U IDU_IAP_Startup = 0x00;//内机程序升级开始


//INT8U IDUODU_StartUp_TimerCnt = 0x00;

//void Programme_Info_Init_Debugger(void);
//void Programme_Info_Init_IDU(void);
//void Programme_Info_Init_ODU(void);

//void Programme_Data_Init(INT8U programme_type);


void IAP_Info_Status_Init(void);


/******************************************************************************
函数名称	: Programme_Info_Init_Debugger
函数功能	: 程序信息缓冲区初始化				  
输入		: void
输出		: void
返回		: void
备注		: 
******************************************************************************/
//void Programme_Info_Init_Debugger(void)
//{
//	INT8U i,j;
//
//	Controller_Programme_Num = 0x00;
//	for(i = 0 ; i < PROGRAMME_NUM_MAX_DEBUGGER; i++)
//	{
//		Controller_Programme_Info[i].programme_checksum = 0x00;
//		Controller_Programme_Info[i].programme_size = 0x00;
//		for(j = 0 ; j < PROGRAMME_NAME_STRING_LEN_DEBUGGER; j++)
//		{
//			Controller_Programme_Info[i].programme_name[j]= 0x00;
//		}
//	}
//}

/******************************************************************************
函数名称	: Programme_Info_Init_IDU
函数功能	: 程序信息缓冲区初始化				  
输入		: void
输出		: void
返回		: void
备注		: 
******************************************************************************/
//void Programme_Info_Init_IDU(void)
//{
//	INT8U i,j;
//
//	IDU_Programme_Num = 0x00;
//	for(i = 0 ; i < PROGRAMME_NUM_MAX_IDUODU; i++)
//	{
//		IDU_ProgramUpdate_InfoRead[i].head = 0x00;
//		IDU_ProgramUpdate_InfoRead[i].attribution = 0x00;
//		IDU_ProgramUpdate_InfoRead[i].programme_checksum = 0x00;
//		IDU_ProgramUpdate_InfoRead[i].struct_checksum = 0x00;
//		IDU_ProgramUpdate_InfoRead[i].programme_size_byte = 0x00;
//		IDU_ProgramUpdate_InfoRead[i].programme_size_page = 0x00;
//		IDU_ProgramUpdate_InfoRead[i].programme_size_iap_1st = 0x00;
//		for(j = 0 ; j < PROGRAMME_NAME_STRING_LEN_IDUODU; j++)
//		{
//			IDU_ProgramUpdate_InfoRead[i].programme_name[j]= 0x00;
//		}
//		IDU_ProgramUpdate_InfoRead[i].file_modify_time.year = 0x00;
//		IDU_ProgramUpdate_InfoRead[i].file_modify_time.month = 0x00;
//		IDU_ProgramUpdate_InfoRead[i].file_modify_time.day = 0x00;
//		IDU_ProgramUpdate_InfoRead[i].file_modify_time.hour = 0x00;
//		IDU_ProgramUpdate_InfoRead[i].file_modify_time.minute = 0x00;
//		IDU_ProgramUpdate_InfoRead[i].file_modify_time.second = 0x00;
//
//	}
//}



/******************************************************************************
函数名称	: Programme_Info_Init_ODU
函数功能	: 程序信息缓冲区初始化				  
输入		: void
输出		: void
返回		: void
备注		: 
******************************************************************************/
//void Programme_Info_Init_ODU(void)
//{
//	INT8U i,j;
//
//	ODU_Programme_Num = 0x00;
//	for(i = 0 ; i < PROGRAMME_NUM_MAX_IDUODU; i++)
//	{
//		ODU_ProgramUpdate_InfoRead[i].head = 0x00;
//		ODU_ProgramUpdate_InfoRead[i].attribution = 0x00;
//		ODU_ProgramUpdate_InfoRead[i].programme_checksum = 0x00;
//		ODU_ProgramUpdate_InfoRead[i].struct_checksum = 0x00;
//		ODU_ProgramUpdate_InfoRead[i].programme_size_byte = 0x00;
//		ODU_ProgramUpdate_InfoRead[i].programme_size_page = 0x00;
//		ODU_ProgramUpdate_InfoRead[i].programme_size_iap_1st = 0x00;
//		for(j = 0 ; j < PROGRAMME_NAME_STRING_LEN_IDUODU; j++)
//		{
//			ODU_ProgramUpdate_InfoRead[i].programme_name[j]= 0x00;
//		}
//		ODU_ProgramUpdate_InfoRead[i].file_modify_time.year = 0x00;
//		ODU_ProgramUpdate_InfoRead[i].file_modify_time.month = 0x00;
//		ODU_ProgramUpdate_InfoRead[i].file_modify_time.day = 0x00;
//		ODU_ProgramUpdate_InfoRead[i].file_modify_time.hour = 0x00;
//		ODU_ProgramUpdate_InfoRead[i].file_modify_time.minute = 0x00;
//		ODU_ProgramUpdate_InfoRead[i].file_modify_time.second = 0x00;
//		
//	}
//}


/******************************************************************************
函数名称	: Programme_Data_Init
函数功能	: 程序数据缓冲区初始化				  
输入		: void
输出		: void
返回		: void
备注		: 
******************************************************************************/
//void Programme_Data_Init(INT8U programme_type)
//{
//	INT32U i;
//	INT8U* ptr_buff;
//
//	switch(programme_type )
//	{
//		case PROGRAMME_TYPE_CONTROLLER:
//			ptr_buff = Programme_Data;
//			break;
//			
//		case PROGRAMME_TYPE_IDU:
//			ptr_buff = BinFile_Buffer_IDU;
//			break;
//
//		case PROGRAMME_TYPE_ODU:
//			ptr_buff = BinFile_Buffer_ODU;
//			break;
//	}
//
//	if(programme_type <= PROGRAMME_TYPE_ODU)
//	{
//		for(i = 0 ; i < PROGRAMME_SIZE_MAX; i++)
//		{
//			*ptr_buff++ = 0x00;
//		}
//	}
//
//}



/******************************************************************************
函数名称	: IAP_Info_Status_Init
函数功能	: iap_info_status初始化			  
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
void IAP_Info_Status_Init(void)
{
	INT8U i,j;
	
	IAP_Information_Status.IAP_Enable = 0;
	IAP_Information_Status.IAP_Step_Index = UPDATE_CALL_OVER_DEVICE;
	IAP_Information_Status.Master_Called = 0;
	IAP_Information_Status.Handshake_OK = 0;
	IAP_Information_Status.IAP_Startup = 0;
	IAP_Information_Status.IAP_Startup_TimeCnt = 0;
	IAP_Information_Status.IAP_Device_ACK = 0;
	IAP_Information_Status.ACK_Timeout = 0;
	IAP_Information_Status.CAN_Swintch_Delay = 0;
	IAP_Information_Status.IAP_Device_Current = 0;
//	IAP_Information_Status.IAP_Device_ACK = 0;
	for(i = 0; i < DEVICE_NUM_MAX_IAP; i++)
	{
		for(j = 0; j < 3; j++)
		{
			IAP_Information_Status.MAC_Addr[i][j]= 0;
		}
	}
	//IAP_Information_Status.Data_Tx_Integral_Num = 0;
	IAP_Information_Status.Data_Tx_Residual = 0;
	IAP_Information_Status.CRC_CheckSum = 0;
	IAP_Information_Status.Data_Tx_First_Degree_Index = 0;
	IAP_Information_Status.Data_Tx_Second_Degree_Index = 0;
	IAP_Information_Status.Ptr_BinFile_Buf = NULL;
	IAP_Information_Status.IAP_Device_Total_ProgramOK = 0;

}

