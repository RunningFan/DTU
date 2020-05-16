/*********************************************************************************
* File Name          : app_can.c
* Author             : zhgx	
* Version            : V1.0
* Date               : 2014-06-13
* Description		 : can通讯处理
* Last Modefly Date	 : 
*********************************************************************************/

#include "ucos_ii.h"
#include "ProgramUpdate_Common.h"
#include "drv_net.h"
#include "drv_usart.h"
#include "statistic.h"
#include "drv_gprs.h"


void Can_SendData_CallOver_Device(INT8U device_type,INT8U device_attribute);


//CAN发送任务
//INT32U  CAN_Tx_Task_BitMap = 0x00;
//INT8U IDU_UserCtrl_PerOperation;
//INT16U UserCtrl_Tx_Head_Start;






//
//void App_Task_Can_Rx(CanMsgType *pdata);
//void App_Task_Can_Tx(void *pdata);
//
//
//
//
//
//uint8_t Can_TransmitData_Fill(CanMsgType *CAN_Msg,uint8_t type);
//
////void Can_TransmitData_Task(void);
//
//
//void Can_ReceiveData_IAP_Device_Ack(CanMsgType*ptr_can_msg);
//void Can_ReceiveData_IAP_Device_RxReady(CanMsgType * ptr_can_msg);
//void BinFile_Tx_IDU_ODU(void);
//void Can_SendData_StartUp_Device(INT8U device_type,INT8U device_attribute);




/******************************************************************************
函数名称	: App_Task_Can_Rx
函数功能	: CAN 数据接收处理				  
输入		: void
输出		: void
返回		: void
备注		: 
******************************************************************************/
void App_Task_Can_Rx(CanMsgType *pdata)
{
	INT8U err;
	INT8U i;

		switch(pdata->FunCode)
		{
			case CONTROLLER_SLAVE_ACK:
				Can_ReceiveData_IAP_Device_Ack(pdata);
				break;

			case CONTROLLER_SLAVE_DATA_RX_READY:
				Can_ReceiveData_IAP_Device_RxReady(pdata);
				break;

			default:
			break;

		}
}

/******************************************************************************
函数名称	: App_Task_Can_Tx
函数功能	: CAN 数据发送处理				  
输入		: void
输出		: void
返回		: void
备注		: 
******************************************************************************/
void App_Task_Can_Tx()
{
		INT16U i;
		static INT8U frame_index = 0x00;

//		pdata = pdata;
		IAP_Information_Status.Device_Type = DEVICE_TYPE_ODU;
		IAP_Information_Status.IAP_Cycle = 0;

//	while(1)
//	{
	
//		if(CAN_Config_Status == CAN_STATUS_IAP)
//		{
		//内外机程序升级相关
		if(IAP_Information_Status.IAP_Enable){// 需要置1 ，升级包下载以后，应该清零
			BinFile_Tx_IDU_ODU();
		}

		if(StartUp_Device){//程序烧写完成以后，由调试精灵发送启动机组指令 ，由人工交互
			Can_SendData_StartUp_Device(IAP_Information_Status.Device_Type, IAP_Information_Status.Device_Attribute);
			frame_index++;
			if(dbgPrintf)(*dbgPrintf)("\n<<<<<<... _______OPEN MACHINE_______ ...>>>>>>\n");
			if(frame_index >= 10){
				IAP_Info_Status_Init();
				StartUp_Device = NO;
				frame_index = RESET;

				m_updateInfo.urg_up = DISABLE;
				m_updatestep = Idle;
				if(dbgPrintf)(*dbgPrintf)("\n<<<<<< QUICK UPDATE EXPEND : <%d min> >>>>>>\n",time_account(&timerbf8[1])/60);//升级成功消耗时间
//					CAN_Config_Status = CAN_STATUS_DEBUGGER;
			}

//			Can_TransmitData_Task();
			OSTimeDly(30);// 100ms
		}else{
//			Can_TransmitData_Task(); //等待用户确认
//			OSTimeDly(1);// 4ms
			OSTimeDlyHMSM(0,0,0,5);
		}
//		}
//		else
//		{
//			Can_TransmitData_Task();
//			OSTimeDly(25);// 50ms
//		}
//	}
}





//以下为内外机程序升级相关

/******************************************************************************
函数名称	: Can_ReceiveData_IAP
函数功能	: 接收设备答复			  
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
void Can_ReceiveData_IAP_Device_Ack(CanMsgType* ptr_can_msg)
{
//	CAN_MSG_Type can_msg;
//	INT8U *ptr_data;
//	INT8U data1_temp,data2_temp;
//	INT8U data_len;
	INT16U MAC_Addr;
	INT8U device_new = 0;//新的设备
	INT8U i;

	
//	can_msg = *ptr_can_msg;
//	data_len = can_msg.len;//数据长度
//	ptr_data = can_msg.data;

	if(ptr_can_msg->Lenth != 2)
	{
		return;
	}
	
//	data1_temp = *ptr_data++;
//	data2_temp = *ptr_data++;
	MAC_Addr = ptr_can_msg->Mac_L|(ptr_can_msg->Mac_H<<8);

	if((ptr_can_msg->Data[0] == IAP_Information_Status.Device_Type)&&(ptr_can_msg->Data[1] == IAP_Information_Status.Device_Attribute))
	{
		if(IAP_Information_Status.IAP_Step_Index == UPDATE_CALL_OVER_DEVICE)
		{
			//判断是否新设备
			device_new = 1;
//			for(i = 0; i < IAP_Information_Status.IAP_Device_Current; i++)
			for(i = 0; i < m_updateInfo.m_mac_num_tmp; i++)
			{
				if(MAC_Addr == IAP_Information_Status.MAC_Addr[i][0])
				{
					device_new = 0;
					if(dbgPrintf)(*dbgPrintf)("\n ************ RECEIVE PUBLISH MAC ADDRESS [%04X] ************\n",MAC_Addr);
					break;
				}
			}

			//添加新设备MAC地址
			if((device_new)&&(IAP_Information_Status.IAP_Device_Current < DEVICE_NUM_MAX_IAP))
			{
				IAP_Information_Status.MAC_Addr[IAP_Information_Status.IAP_Device_Current][0] = MAC_Addr;
				IAP_Information_Status.IAP_Device_Current++;
//				IAP_Information_Status.IAP_Device_Total_Online++;
				if(dbgPrintf)(*dbgPrintf)("\n ************ RECEIVE PUBLISH MAC ADDRESS [%04X] ************\n",MAC_Addr);

//				if(IAP_Information_Status.IAP_Device_Current == 0x01)//第一台
				if(IAP_Information_Status.IAP_Device_Current >= m_updateInfo.m_mac_num_tmp)//接收到所有在线机组公布地址
				{
					IAP_Information_Status.ACK_Timeout = 0;
					IAP_Information_Status.IAP_Startup = ENABLE;//在线机组全部响应,进入下一流程开始擦除
					if(dbgPrintf)(*dbgPrintf)("\n ************ ALL ACK ************\n");
				}
			}
		}

		IAP_Information_Status.IAP_Enter_TimeCnt = 0x00;
	}
}


/******************************************************************************
函数名称	: Device_IAP_Status_Switch
函数功能	: IAP状态切换		  
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
uint8_t resume_update_counts;
void Device_IAP_Status_Switch(void){
	uint8_t err;
	if(IAP_Information_Status.IAP_Step_Index == UPDATE_CALL_OVER_DEVICE){
		//切换至下一状态
		if(((IAP_Information_Status.IAP_Cycle)&&((IAP_Information_Status.IAP_Device_Current)&&(IAP_Information_Status.ACK_Timeout >= DEVICE_ACK_TIMEOUT)))\
				|| ((IAP_Information_Status.IAP_Cycle == 0x00)&&(IAP_Information_Status.IAP_Startup))){//假设IAP_Information_Status.IAP_Startup为人机交互的确认操作

			if(IAP_Information_Status.Handshake_OK == NO){
//				m_updateInfo.send_bootloader_op = ENABLE;//只发送一次
				IAP_Information_Status.Handshake_OK = OK;
				IAP_Information_Status.ACK_Timeout = RESET;
				if(dbgPrintf)(*dbgPrintf)("\n<<<<<<... Handshake OK ...>>>>>>\n");
//				save_update_machine_flash(&m_updateInfo,err);
			}

			if(IAP_Information_Status.IAP_Step_Index == UPDATE_CALL_OVER_DEVICE){
				IAP_Information_Status.IAP_Step_Index = UPDATE_SEND_ERASE_COMMAND;
			}
		}//点名未响应则一直点名，与调试精灵类似
//		else if(IAP_Information_Status.ACK_Timeout >= DEVICE_ACK_TIMEOUT){//超时直接退出
//			IAP_Information_Status.IAP_Enable = DISABLE;
//			StartUp_Device = ENABLE;
//		}
				
		if(IAP_Information_Status.IAP_Cycle){//暂不使用
			if(IAP_Information_Status.ACK_Timeout >= IAP_EXIT_TIMEOUT){
				IAP_Information_Status.IAP_Enable = DISABLE;
			}
		}
		
	}else if(IAP_Information_Status.IAP_Step_Index == UPDATE_SEND_WAIT_STT){
#ifdef 	ACK_ALL_ONLINE
		if((IAP_Information_Status.IAP_Device_ACK >= m_updateInfo.m_mac_num_tmp)){
#else
		if((IAP_Information_Status.IAP_Device_ACK >= UPDATE_ONLINE_COUNT)){

#endif
			//&&(IAP_Information_Status.ACK_Timeout >= DEVICE_ACK_TIMEOUT)){//必须得在线以上应答一台应答
			IAP_Information_Status.IAP_Step_Index = UPDATE_SEND_DATA_STT;
			if(dbgPrintf)(*dbgPrintf)("\n<<<<<<.... UPDATE_SEND_DATA_STT ...>>>>>>\n");
		}else if(IAP_Information_Status.ACK_Timeout >= DEVICE_ACK_TIMEOUT){
			IAP_Info_Status_Init();
			CAN_WakeUp(CAN1);
			BSP_CAN_Init();
			if(dbgPrintf)(*dbgPrintf)("\n<<<<<<... Expire Time ACK ---> RECALL DEVICE ...>>>>>>\n");
			IAP_Information_Status.IAP_Enable = ENABLE;
			StartUp_Device = DISABLE;
			m_updateInfo.urg_up = ENABLE;
			for(uint8_t count = 0;count < CALL_DEVICE_TOTAL;count++){
				IAP_Information_Status.Device_Type = DEVICE_TYPE_ODU;
				Can_SendData_CallOver_Device(IAP_Information_Status.Device_Type,IAP_Information_Status.Device_Attribute);// DEVICE_TYPE_ODU
			}
		}
	}
}


/******************************************************************************
函数名称	: Can_ReceiveData_IAP_IDU
函数功能	: 接收设备答复			  
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
void Can_ReceiveData_IAP_Device_RxReady(CanMsgType * ptr_can_msg)
{
//	CAN_MSG_Type can_msg;
	INT8U *ptr_data;
//	INT8U data_len;
	INT16U MAC_Addr;
//	INT8U data[2];
	INT8U i,err,res;
	
//	can_msg = *ptr_can_msg;
//	data_len = can_msg.len;//数据长度
//	ptr_data = can_msg.data;

	if(ptr_can_msg->Lenth != 2)
	{
		return;
	}
	
	MAC_Addr = ptr_can_msg->Mac_L|(ptr_can_msg->Mac_H<<8);
//	data[0] = *ptr_data++;
//	data[1] = *ptr_data++;

	if(IAP_Information_Status.Handshake_OK == OK)
	{
//		for(i = 0; i < IAP_Information_Status.IAP_Device_Current; i++)
		for(i = 0; i < m_updateInfo.m_mac_num_tmp; i++)
		{
			if(MAC_Addr == IAP_Information_Status.MAC_Addr[i][0])
			{
				if(IAP_Information_Status.MAC_Addr[i][1] == NO)//未应答
				{
					IAP_Information_Status.MAC_Addr[i][1] = OK;
					IAP_Information_Status.IAP_Device_ACK++;
#ifdef 	ACK_ALL_ONLINE				
					if(IAP_Information_Status.IAP_Device_ACK >= m_updateInfo.m_mac_num_tmp)
#else
					if(IAP_Information_Status.IAP_Device_ACK >= UPDATE_ONLINE_COUNT)
#endif
					{
						IAP_Information_Status.ACK_Timeout = RESET;
						if(dbgPrintf)(*dbgPrintf)("\n<<<<<<... RECEIVE ACK OK [%04X] ...>>>>>>\n",MAC_Addr);
					}
				}
//				break;
			}
		}

		//加延时防止多台设备延迟；
//		if(IAP_Information_Status.IAP_Device_ACK == IAP_Information_Status.IAP_Device_Current)//如果机组全部擦除完成
//		if(IAP_Information_Status.IAP_Device_ACK >= m_updateInfo.m_mac_num_tmp)//如果机组全部擦除完成
//			//||(IAP_Information_Status.ACK_Timeout >= DEVICE_ACK_TIMEOUT))
//		{
//			if(IAP_Information_Status.IAP_Step_Index == UPDATE_SEND_WAIT_STT)
//			{
//				IAP_Information_Status.IAP_Step_Index = UPDATE_SEND_DATA_STT;
//				if(dbgPrintf)(*dbgPrintf)("\n<<<<<<... SWITCH UPDATE_SEND_DATA_STT ...>>>>>>\n");
//			}
//		}
		
		//烧录完成设备
		//子设备烧写完成所有的编码且所有的编码正确（即本设备确认可以进行跳转操作）时发0xff，0xff
		if((ptr_can_msg->Data[0] == ACK_RCV_ALL_CODE) && (ptr_can_msg->Data[1] == ACK_RCV_ALL_CODE))
		{
//			for(i = 0; i < IAP_Information_Status.IAP_Device_Current; i++)
			for(i = 0; i < m_updateInfo.m_mac_num_tmp; i++)
			{
				if(MAC_Addr == IAP_Information_Status.MAC_Addr[i][0])
				{
					if(IAP_Information_Status.MAC_Addr[i][2] == NO)//未应答
					{
						IAP_Information_Status.MAC_Addr[i][2] = OK;//应答可以跳转,烧录完成
						IAP_Information_Status.IAP_Device_Total_ProgramOK++;
						if(dbgPrintf)(*dbgPrintf)("\n<<<<<<... RECEIVE ACK PROGRAM OK [%04X] ...>>>>>>\n",MAC_Addr);
					}

					if(IAP_Information_Status.IAP_Device_Total_ProgramOK   	>= m_updateInfo.m_mac_num_tmp){
						StartUp_Device = ENABLE;
						ConSta.update_type = UPDATE_RESET_TYPE;
						if(dbgPrintf)(*dbgPrintf)("\n<<<<<<... StartUp_Device ENABLE !!! ...>>>>>>\n");
						break;
					}
				}
			}
		}
		IAP_Information_Status.IAP_Enter_TimeCnt = 0x00;
	}
}

/******************************************************************************
函数名称	: Can_SendData_CallOver_Device
函数功能	: 发送点名帧			  
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
void Can_SendData_CallOver_Device(INT8U device_type,INT8U device_attribute)
{
	CanMsgType can_msg;
	uint8_t auto_transit_cnt = 0;
	memset(&can_msg,0,sizeof(CanMsgType));
//	INT8U function_code;
//	INT8U *ptr_data;
	
//	function_code = CONTROLLER_MASTER_CALLOVER;
//	can_msg.id = 0x00;
	can_msg.FunCode = CONTROLLER_MASTER_CALLOVER;
	can_msg.Lenth = 2;
//	can_msg.type = DATA_FRAME;
//	can_msg.format = EXT_ID_FORMAT;
	
	switch(device_type)
	{
//		case DEVICE_TYPE_IDU:
//			ptr_data = can_msg.data;
//			*ptr_data++ = DEVICE_TYPE_IDU;
//			*ptr_data++ = device_attribute;
//			break;
			
		case DEVICE_TYPE_ODU:
//			ptr_data = can_msg.data;
			can_msg.Data[0] = DEVICE_TYPE_ODU;
//			*ptr_data++ = DEVICE_TYPE_ODU;
//			*ptr_data++ = device_attribute;
			can_msg.Data[1] = device_attribute;
			break;

		default:
			break;
			
	}
//	while(CAN_TxStatus_NoMailBox == Can_TransmitData_Fill(&can_msg,UPDATE_CALL_OVER_DEVICE)){
	while(CAN_TxStatus_Failed == CAN_TransmitStatus(CAN1,Can_TransmitData_Fill(&can_msg,UPDATE_CALL_OVER_DEVICE))){
		if(auto_transit_cnt++ > AUTO_TRANSIT_LIMIT){
			if(dbgPrintf)(*dbgPrintf)("\n ************ tranmit failure1 ************\n");
			CAN_WakeUp(CAN1);
			BSP_CAN_Init();
			break;
		}
		OSTimeDlyHMSM(0,0,0,10);
//		OSTimeDly(DELAY_MS_SET);
	}
}

/******************************************************************************
函数名称	: Can_SendData_Erase_Device
函数功能	: 擦除命令			  
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
void Can_SendData_Erase_Device(INT8U device_type, INT8U device_attribute, INT16U page_num)
{
	CanMsgType can_msg;
	uint8_t auto_transit_cnt = 0;
//	INT8U function_code;
//	INT8U *ptr_data;
	memset(&can_msg,0,sizeof(CanMsgType));
//	function_code = SOFTWARE_ERASE_START;
//	can_msg.id = 0x00;
//	can_msg.id = (device_attribute & 0x7F)|((device_type & 0x7F) <<7)|(function_code << 21);
	can_msg.dev_attr = device_attribute;
	can_msg.dev_type = device_type;
	can_msg.FunCode3 = SOFTWARE_ERASE_START;
	can_msg.Lenth = 2;
//	can_msg.type = DATA_FRAME;
//	can_msg.format = EXT_ID_FORMAT;
	
	switch(device_type)
	{
//		case DEVICE_TYPE_IDU:
//			ptr_data = can_msg.data;
//			*ptr_data++ = page_num & 0xFF;
//			*ptr_data++ = (page_num >> 8) & 0xFF;
//			break;
			
		case DEVICE_TYPE_ODU:
			can_msg.Data[0] = page_num & 0xFF;
			can_msg.Data[1] = (page_num >> 8) & 0xFF;
//			ptr_data = can_msg.data;
//			*ptr_data++ = page_num & 0xFF;
//			*ptr_data++ = (page_num >> 8) & 0xFF;
			break;

		default:
			break;
			
	}
//	while(CAN_TxStatus_NoMailBox == Can_TransmitData_Fill	(&can_msg,UPDATE_SEND_ERASE_COMMAND)){
	while(CAN_TxStatus_Failed == CAN_TransmitStatus(CAN1,Can_TransmitData_Fill(&can_msg,UPDATE_SEND_ERASE_COMMAND))){

		if(auto_transit_cnt++ > AUTO_TRANSIT_LIMIT){
		if(dbgPrintf)(*dbgPrintf)("\n ************ tranmit failure2 ************\n");
			CAN_WakeUp(CAN1);
			BSP_CAN_Init();
			break;
		}
		OSTimeDly(DELAY_MS_SET);
	}
}



/******************************************************************************
函数名称	: Can_SendData_BinData_Device
函数功能	: 发送bin 
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
void Can_SendData_BinData_Device(INT16U first_degree,INT16U second_degree,INT8U* ptr_data)
{
	CanMsgType can_msg;
	uint8_t auto_transit_cnt = 0;
//	INT8U function_code;
//	INT8U* ptr_data_can;
//	INT8U i;
	
//	function_code = CONTROLLER_MASTER_DATA_TX;
//	can_msg.id = 0x00;
//	can_msg.id = (second_degree & 0x7FF)|((first_degree & 0x3FF) <<11)|(function_code << 21);
//	can_msg.len = 8;
//	can_msg.type = DATA_FRAME;
//	can_msg.format = EXT_ID_FORMAT;
	memset(&can_msg,0,sizeof(CanMsgType));
	can_msg.FunCode4 = CONTROLLER_MASTER_DATA_TX;
	can_msg.Lenth = 8;
	can_msg.boot_first_index = first_degree;
	can_msg.boot_second_index = second_degree;
	memcpy(can_msg.Data,ptr_data,can_msg.Lenth);

//	ptr_data_can = can_msg.data;
//	for(i = 0 ; i < 8 ; i++)
//	{
//		*ptr_data_can++ = *ptr_data++;
//	}

//	while(CAN_TxStatus_NoMailBox == Can_TransmitData_Fill(&can_msg,CONTROLLER_MASTER_DATA_TX))
	while(CAN_TxStatus_Failed == CAN_TransmitStatus(CAN1,Can_TransmitData_Fill(&can_msg,CONTROLLER_MASTER_DATA_TX)))
	{
		if(auto_transit_cnt++ > AUTO_TRANSIT_LIMIT){
		if(dbgPrintf)(*dbgPrintf)("\n ************ tranmit failure3 ************\n");
			CAN_WakeUp(CAN1);
			BSP_CAN_Init();		
			break;
		}
		if(dbgPrintf)(*dbgPrintf)("\n ************ retry failure 3 ************\n");
		OSTimeDly(DELAY_MS_SET);
	}
//	OSTimeDly(DELAY_MS_SET * 2);
	OSTimeDlyHMSM(0,0,0,10);
	//Can_TransmitData_Fill	(&can_msg);
}


/******************************************************************************
函数名称	: Can_SendData_BinData_Device
函数功能	: 发送bin 
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
void Can_SendData_CRC_CheckSum_Device(INT16U first_degree, INT16U second_degree, INT16U check_sum)
{
	CanMsgType can_msg;
	uint8_t auto_transit_cnt = 0;
//	INT8U function_code;
//	INT8U* ptr_data_can;
	memset(&can_msg,0,sizeof(CanMsgType));
//	function_code = CONTROLLER_MASTER_DATA_TX;
	can_msg.FunCode4 = CONTROLLER_MASTER_DATA_TX;
	can_msg.Lenth = 2;
	can_msg.boot_first_index = first_degree;
	can_msg.boot_second_index = second_degree;
	can_msg.Data[0] = check_sum & 0xFF;
	can_msg.Data[1] = (check_sum >> 8) & 0xFF;
//	can_msg.id = 0x00;
//	can_msg.id = (second_degree & 0x7FF)|((first_degree & 0x3FF) <<11)|(function_code << 21);
//	can_msg.len = 2;
//	can_msg.type = DATA_FRAME;
//	can_msg.format = EXT_ID_FORMAT;

//	ptr_data_can = can_msg.data;
//	*ptr_data_can++ = check_sum & 0xFF;
//	*ptr_data_can++ = (check_sum >> 8) & 0xFF;
//	while(CAN_TxStatus_NoMailBox == Can_TransmitData_Fill(&can_msg,CONTROLLER_MASTER_DATA_TX))
	while(CAN_TxStatus_Failed == CAN_TransmitStatus(CAN1,Can_TransmitData_Fill(&can_msg,CONTROLLER_MASTER_DATA_TX)))

		
	{
		if(auto_transit_cnt++ > AUTO_TRANSIT_LIMIT){
		if(dbgPrintf)(*dbgPrintf)("\n ************ tranmit failure4 ************\n");
			CAN_WakeUp(CAN1);
			BSP_CAN_Init();
			break;
		}
		OSTimeDly(DELAY_MS_SET);
	}
	//Can_TransmitData_Fill	(&can_msg);
}



/******************************************************************************
函数名称	: Can_SendData_StartUp_Device
函数功能	: 发送bin 
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
void Can_SendData_StartUp_Device(INT8U device_type,INT8U device_attribute)
{
	CanMsgType can_msg;
	uint8_t auto_transit_cnt = 0;
//	INT8U function_code;
//	INT8U *ptr_data_can;
	
//	function_code = CONTROLLER_STARTUP_DEVICE;
	memset(&can_msg,0,sizeof(CanMsgType));
	can_msg.FunCode3 = CONTROLLER_STARTUP_DEVICE;
	can_msg.Lenth = 2;
	can_msg.dev_attr = device_attribute;
	can_msg.dev_type = device_type;
	
//	can_msg.id = 0x00;
//	can_msg.id = (device_attribute & 0x7F)|((device_type & 0x7F) <<7)|(function_code << 21);
//	can_msg.len = 2;
//	can_msg.type = DATA_FRAME;
//	can_msg.format = EXT_ID_FORMAT;
//
//	ptr_data_can = can_msg.data;
//	*ptr_data_can++ = 0x00;
//	*ptr_data_can++ = 0x00;
//	while(CAN_TxStatus_NoMailBox == Can_TransmitData_Fill(&can_msg,UPDATE_SEND_ERASE_COMMAND)){
	while(CAN_TxStatus_Failed == CAN_TransmitStatus(CAN1,Can_TransmitData_Fill(&can_msg,UPDATE_SEND_ERASE_COMMAND))){
		
		if(auto_transit_cnt++ > AUTO_TRANSIT_LIMIT){
		if(dbgPrintf)(*dbgPrintf)("\n ************ tranmit failure5 ************\n");
			CAN_WakeUp(CAN1);
			BSP_CAN_Init();
			break;
		}		
		OSTimeDly(DELAY_MS_SET);
	}
}


/******************************************************************************
函数名称	: BinFile_Tx_IDU_ODU
函数功能	: 内外机binfile发送处理			  
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
void BinFile_Tx_IDU_ODU(void)
{
	INT16U check_sum;
	INT8U* ptr_crc_databuf;
	static INT8U* ptr_binfile_basebuf;
	static INT8U* ptr_binfile_databuf;
	//INT32U binfile_offset;
	INT8U i;

	
	switch(IAP_Information_Status.IAP_Step_Index)
	{
		case UPDATE_CALL_OVER_DEVICE://点名设备
			StartUp_Device = NO;
			Can_SendData_CallOver_Device(IAP_Information_Status.Device_Type,IAP_Information_Status.Device_Attribute);// DEVICE_TYPE_ODU
			Device_IAP_Status_Switch();
			if(dbgPrintf)(*dbgPrintf)("\n ************ CALL DEVICE ************\n");
			OSTimeDly(1);
			break;

		case UPDATE_SEND_ERASE_COMMAND://下发擦除命令
			//if((IAP_Information_Status.IAP_Cycle)\
			//	||((IAP_Information_Status.IAP_Cycle == 0x00)&&(IAP_Information_Status.IAP_Startup)))
			//{
				Can_SendData_Erase_Device(IAP_Information_Status.Device_Type,IAP_Information_Status.Device_Attribute,IAP_Information_Status.Data_Tx_Integral_Num);
				if(dbgPrintf)(*dbgPrintf)("\n ************ ERASE COMMAND DELIVER ************\n");
				IAP_Information_Status.IAP_Step_Index = UPDATE_SEND_WAIT_STT;
				//在线总设备
				//IAP_Information_Status.IAP_Device_Total_Online += IAP_Information_Status.IAP_Device_Current;

//				if(IAP_Information_Status.Device_Type == DEVICE_TYPE_IDU)
//				{
//					ptr_binfile_basebuf = (INT8U*)BinFile_Buffer_IDU;
//				}
//				else if(IAP_Information_Status.Device_Type == DEVICE_TYPE_ODU)
//				{
//					ptr_binfile_basebuf = (INT8U*)BinFile_Buffer_ODU;
//				}

				IAP_Information_Status.Data_Tx_First_Degree_Index = 0;
				IAP_Information_Status.Data_Tx_Second_Degree_Index = 0;
				IAP_Information_Status.IAP_Timeused = 0;
				
			//}
			break;

		case UPDATE_SEND_WAIT_STT://发送等待状态
			OSTimeDly(OS_TICKS_PER_SEC/10);
			Device_IAP_Status_Switch();
			OSTimeDlyHMSM(0,0,0,5);
			if(dbgPrintf)(*dbgPrintf)("\n ************ WAIT RESPONSE ************\n");
			break;

		case UPDATE_SEND_DATA_STT://发送数据状态
			if(IAP_Information_Status.Data_Tx_First_Degree_Index < IAP_Information_Status.Data_Tx_Integral_Num){
				ptr_binfile_databuf = read_can_update_boot(sendcan_buffer.DataBuf,PARAM_UPDATE_START_ADDR + \
										IAP_Information_Status.Data_Tx_First_Degree_Index*BYTE_NUM_PER_PAGE_IAP,1024);
//				ptr_binfile_databuf += IAP_Information_Status.Data_Tx_First_Degree_Index*BYTE_NUM_PER_PAGE_IAP;
				for(IAP_Information_Status.Data_Tx_Second_Degree_Index = 0 ; IAP_Information_Status.Data_Tx_Second_Degree_Index < \
															SECOND_DEGREE_INDEX_MAX; IAP_Information_Status.Data_Tx_Second_Degree_Index++)
				{
					Can_SendData_BinData_Device(IAP_Information_Status.Data_Tx_First_Degree_Index,\
												IAP_Information_Status.Data_Tx_Second_Degree_Index,ptr_binfile_databuf);
					ptr_binfile_databuf += 8;

				}

				//发送校验帧
				ptr_crc_databuf = sendcan_buffer.DataBuf;
//				ptr_crc_databuf += IAP_Information_Status.Data_Tx_First_Degree_Index*BYTE_NUM_PER_PAGE_IAP;
				IAP_Information_Status.IAP_Device_ACK = 0;

				check_sum = CRC16_Check(ptr_crc_databuf,BYTE_NUM_PER_PAGE_IAP);
				Can_SendData_CRC_CheckSum_Device(IAP_Information_Status.Data_Tx_First_Degree_Index, \
													IAP_Information_Status.Data_Tx_Second_Degree_Index, check_sum);
				IAP_Information_Status.Data_Tx_First_Degree_Index++;
				IAP_Information_Status.IAP_Step_Index = UPDATE_SEND_WAIT_STT;
//				IAP_Information_Status.IAP_Device_ACK = 0;
				IAP_Information_Status.ACK_Timeout = 0;
				for(i = 0; i < DEVICE_NUM_MAX_IAP; i++)
				{
					IAP_Information_Status.MAC_Addr[i][1] = 0x00;
				}
#if DEBUG
				uint8_t progress_bar = 0;
				progress_bar = (uint8_t)(IAP_Information_Status.Data_Tx_First_Degree_Index / (IAP_Information_Status.Data_Tx_Integral_Num * 1.0) * 100);
				switch (progress_bar/30)
					{
						case 0:
							if(dbgPrintf)(*dbgPrintf)("\n[==>%d                                   ]\n",progress_bar);
							g_Upload.update_stt = REABCK_UPDATE_BAR;
							g_Upload.cur_opt = 0;
							break;
						
						case 1:
							if(dbgPrintf)(*dbgPrintf)("\n[=========>%d                            ]\n",progress_bar);
							g_Upload.update_stt = REABCK_UPDATE_BAR;
							g_Upload.cur_opt = 1;							
							break;
						
						case 2:
							if(dbgPrintf)(*dbgPrintf)("\n[=======================>%d             ]\n",progress_bar);
							g_Upload.update_stt = REABCK_UPDATE_BAR;
							g_Upload.cur_opt = 2;							
							break;
						
						case 3:
							if(dbgPrintf)(*dbgPrintf)("\n[==================================>%d  ]\n",progress_bar);
							g_Upload.update_stt = REABCK_UPDATE_BAR;
							g_Upload.cur_opt = 3;							
							break;
						default:
							if(dbgPrintf)(*dbgPrintf)("\n[=====================================>%d]\n",progress_bar);
						break;
					}
									
#endif				

			}else if(IAP_Information_Status.Data_Tx_First_Degree_Index == IAP_Information_Status.Data_Tx_Integral_Num){
				IAP_Info_Status_Init();
//				IAP_Information_Status.IAP_Enable = DISABLE;
				IAP_Information_Status.IAP_Step_Index = UPDATE_CALL_OVER_DEVICE;
//				IAP_Information_Status.IAP_Cycle++;
//				IDUODU_StartUp_TimerCnt = 10;
				if(dbgPrintf)(*dbgPrintf)("\n<<<<<<...IAP SEND OVER ...>>>>>>\n");
//				StartUp_Device = OK;
			}
			break;

		default:
			break;

	}
}




/******************************************************************************
函数名称	: Can_TransmitData_Fill
函数功能	: 发送数据填充				  
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
uint8_t Can_TransmitData_Fill(CanMsgType *CAN_Msg,uint8_t type)
{
	CanTxMsg m_Data;
	uint8_t ret;
	memset(&m_Data,0,sizeof(CanTxMsg));
	switch (type)
		{
			case UPDATE_CALL_OVER_DEVICE:
				m_Data.FunCode 	   = CAN_Msg->FunCode;
				break;
			case UPDATE_SEND_ERASE_COMMAND:
				m_Data.dev_attr = CAN_Msg->dev_attr;
				m_Data.dev_type = CAN_Msg->dev_type;
				m_Data.FunCode3        = CAN_Msg->FunCode3;
				break;
			case CONTROLLER_MASTER_DATA_TX:
				m_Data.FunCode4 = CAN_Msg->FunCode4;
				m_Data.boot_first_index = CAN_Msg->boot_first_index;
				m_Data.boot_second_index = CAN_Msg->boot_second_index;
				break;
//			case CONTROLLER_STARTUP_DEVICE:
//				m_Data
//				break;
				
				
		}

    m_Data.DLC             = CAN_Msg->Lenth;
    m_Data.RTR             = CAN_RTR_DATA;
    m_Data.IDE             = CAN_ID_EXT;
//    m_Data.FunCode1        = CAN_Msg->FunCode;
//    m_Data.second_index    = second_addr;
//    m_Data.first_index     = first_addr;//>>8&0xFF;
////    m_Data.first_index_l     = first_addr&0xFF;
    
    memcpy(m_Data.Data,CAN_Msg->Data,CAN_Msg->Lenth);
//    second_addr += 1;
    ret = CAN_Transmit(CAN1 , &m_Data);
    return ret;

//	INT8U i;
//
//	CAN_Transmit_Task.CAN_Transmit_Msg[CAN_Transmit_Task.tx_head].id = 0x00;
//	CAN_Transmit_Task.CAN_Transmit_Msg[CAN_Transmit_Task.tx_head].type = 0x00;
//	CAN_Transmit_Task.CAN_Transmit_Msg[CAN_Transmit_Task.tx_head].format = 0x00;
//	CAN_Transmit_Task.CAN_Transmit_Msg[CAN_Transmit_Task.tx_head].len = 0x00;
//	
//	for(i = 0; i < 8; i++ )
//	{
//		CAN_Transmit_Task.CAN_Transmit_Msg[CAN_Transmit_Task.tx_head].data[i] = 0x00;
//	}
//
//	CAN_Transmit_Task.CAN_Transmit_Msg[CAN_Transmit_Task.tx_head].id = CAN_Msg->id;
//	CAN_Transmit_Task.CAN_Transmit_Msg[CAN_Transmit_Task.tx_head].type = CAN_Msg->type;
//	CAN_Transmit_Task.CAN_Transmit_Msg[CAN_Transmit_Task.tx_head].format = CAN_Msg->format;
//	CAN_Transmit_Task.CAN_Transmit_Msg[CAN_Transmit_Task.tx_head].len = CAN_Msg->len;
//	
//	for(i = 0; i < CAN_Msg->len; i++ )
//	{
//		CAN_Transmit_Task.CAN_Transmit_Msg[CAN_Transmit_Task.tx_head].data[i] = CAN_Msg->data[i];
//	}
//
//	if(CAN_Transmit_Task.tx_head < (CAN_TXBUFFER_SIZE - 1))
//	{
//		CAN_Transmit_Task.tx_head++;
//	}
//	else
//	{
//		CAN_Transmit_Task.tx_head = 0x00;
//	}
}


/******************************************************************************
函数名称	: Can_TransmitData_Fill
函数功能	: 发送数据填充				  
输入		: void
输出		: void
返回		: 
备注		: 
******************************************************************************/
//void Can_TransmitData_Task(void)
//{
//	//if((Can_Msg_Cnt)||((CAN_Config_Status == CAN_STATUS_IAP)&&(IAP_Information_Status.IAP_Enable == 0x01)))//收到数据确定波特率及极性后，才允许下发
//	//{
//		if(CAN_Transmit_Task.tx_tail!= CAN_Transmit_Task.tx_head)
//		{
//			while(ERROR == CAN_SendMsg(LPC_CAN1,(CAN_MSG_Type *)(&CAN_Transmit_Task.CAN_Transmit_Msg[CAN_Transmit_Task.tx_tail])))
//			{
//				OSTimeDly(OS_TICKS_PER_SEC/100);
//			}
//
//			if(CAN_Transmit_Task.tx_tail < (CAN_TXBUFFER_SIZE - 1))
//			{
//				CAN_Transmit_Task.tx_tail++;
//			}
//			else
//			{
//				CAN_Transmit_Task.tx_tail = 0x00;
//			}
//		}
//	//}
//}



/*****************************************************************
******************************************************************
******************************************************************
******************************************************************
*****************************************************************/
//	判断CAN发送邮箱buff是否为空,增加信号量机制






