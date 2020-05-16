#include "update_machine.h"
#include "bsp.h"
#include "fsm.h"
#include "update_machine.h"
#define SEND_DATA_LEN  254
//extern BUFF_TYPE  sendcan_buffer; 
BUFF_TYPE  sendcan_buffer; 
enum updatesteptype m_updatestep = Idle;

char* Update_State[UPDATE_STATE_NUM] = {
	{"m_getdata"},
	{"m_register"},
	{"m_erase"},
	{"m_senddata"},
	{"m_updatesuccess"},
	{"Idle"},
	{"m_monitor"},
	{"m_failure"},
	{"m_debugelf"},
	{"m_codeok"},
	{"m_ackok"},
};

M_updateInfoType m_updateInfo;

/*
    将具体的程序数据发送到总线上
firstaddr:一级索引
secondaddr:二级索引
num:发送数据长度。
return：返回二级索引。

*/
                            
uint16_t send_updateto_can(uint16_t first_addr, uint16_t second_addr, uint8_t num)
{
    CanTxMsg m_Data;
	uint8_t auto_transit_cnt = 0;
    m_Data.DLC             = num;
    m_Data.RTR             = CAN_RTR_DATA;
    m_Data.IDE             = CAN_ID_EXT;
    m_Data.FunCode1        = REPLY_CODE_FUNC;
    m_Data.second_index    = second_addr;
    m_Data.first_index     = first_addr;//>>8&0xFF;
//    m_Data.first_index_l     = first_addr&0xFF;
    
    memcpy(m_Data.Data, &sendcan_buffer.DataBuf[second_addr*8], num);
    second_addr += 1;
//    while(CAN_TxStatus_NoMailBox == CAN_Transmit(CAN1 , &m_Data)){
	while(CAN_TxStatus_Failed == CAN_TransmitStatus(CAN1,CAN_Transmit(CAN1 , &m_Data))){
		if(auto_transit_cnt++ > AUTO_TRANSIT_LIMIT){
		if(dbgPrintf)(*dbgPrintf)("\n ************ tranmit failure 6************\n");
			CAN_WakeUp(CAN1);
			BSP_CAN_Init();
			break;
		}		
		OSTimeDly(DELAY_MS_SET);
    }
    return second_addr;
}

/*
公布需要升级的程序信息：
*/
void send_update_register(M_updateInfoType* m_updateInfo)
{
    CanTxMsg m_Data;
	uint8_t auto_transit_cnt = 0;
	memset(&m_Data,0,sizeof(CanTxMsg));
    m_Data.DLC      = 7;
    m_Data.RTR      = CAN_RTR_DATA;
    m_Data.IDE      = CAN_ID_EXT;
    m_Data.FunCode  = 0xC2;//状态功能码
    m_Data.M_Type   = 0x7F;//GPRS设备
    m_Data.Mac_H   = (uint8_t)(m_updateInfo->m_info[m_updateInfo->m_op_number].m_mac >> 8 & 0xFF);//模块Mac
    m_Data.Mac_L =  (uint8_t)(m_updateInfo->m_info[m_updateInfo->m_op_number].m_mac & 0xFF);//模块Mac
    m_Data.Data[0]  =  m_updateInfo->m_software_version;  //机组升级包版本信息
    if(updateinfo.update_pack_type == UPDATE_MACHINE_URGENT){
   		m_Data.Data[1]  =  URGENT_UPDATE;//准备紧急升级状态
    }else if(updateinfo.update_pack_type == UPDATE_MACHINE_NORMAL){
   		m_Data.Data[1]  =  READY_UPDATE;//准备升级状态
    }
    m_Data.Data[2]  =  0x00;//错误标志
    memcpy(&m_Data.Data[3],&m_updateInfo->paketsize,sizeof(&m_updateInfo->paketsize));
//    CAN_Transmit(CAN1 , &m_Data);
//    while(CAN_TxStatus_NoMailBox == CAN_Transmit(CAN1 , &m_Data)){
		while(CAN_TxStatus_Failed == CAN_TransmitStatus(CAN1,CAN_Transmit(CAN1 , &m_Data))){

		if(auto_transit_cnt++ > AUTO_TRANSIT_LIMIT){
		if(dbgPrintf)(*dbgPrintf)("\n ************ tranmit failure7 ************\n");
			CAN_WakeUp(CAN1);
			BSP_CAN_Init();
			break;
		}		
		OSTimeDly(DELAY_MS_SET);
	}
}


void send_entry_bootloader(M_updateInfoType* m_updateInfo,uint8_t op_mac){
	CanTxMsg m_Data;
	uint8_t auto_transit_cnt = 0;
	memset(&m_Data,0,sizeof(CanTxMsg));
    m_Data.DLC      = 7;
    m_Data.RTR      = CAN_RTR_DATA;
    m_Data.IDE      = CAN_ID_EXT;
    m_Data.FunCode  = 0xC2;//状态功能码
    m_Data.M_Type   = 0x7F;//GPRS设备
    m_Data.Mac_H   = (uint8_t)(m_updateInfo->m_info[op_mac].m_mac >> 8 & 0xFF);//模块Mac
    m_Data.Mac_L =  (uint8_t)(m_updateInfo->m_info[op_mac].m_mac & 0xFF);//模块Mac
    m_Data.Data[0]  =  m_updateInfo->m_software_version;  //机组升级包版本信息
   	m_Data.Data[1]  =  ENTRY_BOOTLOADER_UPDATE;//准备紧急升级状态
    m_Data.Data[2]  =  0x00;//错误标志
    memcpy(&m_Data.Data[3],&m_updateInfo->paketsize,sizeof(&m_updateInfo->paketsize));
//    while(CAN_TxStatus_NoMailBox == CAN_Transmit(CAN1 , &m_Data)){
	while(CAN_TxStatus_Failed == CAN_TransmitStatus(CAN1,CAN_Transmit(CAN1 , &m_Data))){

		if(auto_transit_cnt++ > AUTO_TRANSIT_LIMIT){
		if(dbgPrintf)(*dbgPrintf)("\n ************ tranmit failure 8************\n");
			CAN_WakeUp(CAN1);
			BSP_CAN_Init();
			break;
		}				
		OSTimeDly(DELAY_MS_SET);
	}

}

void send_ack_ok(M_updateInfoType* m_updateInfo,uint8_t op_mac){
	CanTxMsg m_Data;
	uint8_t auto_transit_cnt = 0;
	memset(&m_Data,0,sizeof(CanTxMsg));
    m_Data.DLC      = 7;
    m_Data.RTR      = CAN_RTR_DATA;
    m_Data.IDE      = CAN_ID_EXT;
    m_Data.FunCode  = 0xC2;//状态功能码
    m_Data.M_Type   = 0x7F;//GPRS设备
    m_Data.Mac_H   = (uint8_t)(m_updateInfo->m_info[op_mac].m_mac >> 8 & 0xFF);//模块Mac
    m_Data.Mac_L =  (uint8_t)(m_updateInfo->m_info[op_mac].m_mac & 0xFF);//模块Mac
    m_Data.Data[0]  =  m_updateInfo->m_software_version;  //机组升级包版本信息
   	m_Data.Data[1]  =  MAKE_SURE_UPDATE;//准备紧急升级状态
    m_Data.Data[2]  =  0x00;//错误标志
    memcpy(&m_Data.Data[3],&m_updateInfo->paketsize,sizeof(&m_updateInfo->paketsize));
//    while(CAN_TxStatus_NoMailBox == CAN_Transmit(CAN1 , &m_Data)){
	while(CAN_TxStatus_Failed == CAN_TransmitStatus(CAN1,CAN_Transmit(CAN1 , &m_Data))){

		if(auto_transit_cnt++ > AUTO_TRANSIT_LIMIT){
		if(dbgPrintf)(*dbgPrintf)("\n ************ tranmit failure 9************\n");
			CAN_WakeUp(CAN1);
			BSP_CAN_Init();
			break;
		}				
		OSTimeDly(DELAY_MS_SET);
	}
	OSTimeDly(DELAY_MS_SET);
}


uint8_t online_unit_count;//在线外机台数
uint8_t current_outdoor_num;//当前接收到的数据是第几台外机
uint8_t check_update(CanMsgType* m_Data,M_updateInfoType* m_updateInfo1)
{
    uint16_t mac = 0;
    uint8_t  i = 0, have_flag = 0, err = 0;
    if(UPDATE_TYPE_FUNC != (m_Data->FunCode&0xF0) && (UPDATE_TYPE_FUNC_GPRS != (m_Data->FunCode&0xF0))){//筛选升级协议功能码
      return 0;
    }

#ifdef QUICK_TEST_UPDATE
#else
	if(isGetTime){
#endif		
			if(!m_updateInfo1->urg_up){//非紧急升级
	   			mac = m_Data->Mac_L|(m_Data->Mac_H<<8);//接收到的机组mac
				    switch(m_Data->FunCode2){
				        case PUBLISH_STT_FUNC://机组公布状态数据功能码
				            if(m_Data->M_Type == OUTDOOR_DEV){//外机类别 
				                if(mac != 0){//确保mac不为0
									if(m_updateInfo1->m_mac_num <= MACHINE_NUMBERS){
											for(i=0; i < m_updateInfo1->m_mac_num; i++){//使用MACHINE_NUMBERS防止外机过多越界
												if(mac == m_updateInfo1->m_info[i].m_mac){//机组mac存在mac表中
													have_flag = ENABLE;
													current_outdoor_num = i;//查找到位mac表中的第	  n台外机
													break;
												}
											}
											if(!have_flag){
												if(online_unit_count < m_updateInfo1->m_mac_num){
													m_updateInfo1->m_info[online_unit_count].m_mac = mac;//动态建立在线机组Mac表
													m_updateInfo1->m_info[online_unit_count].m_Type = m_Data->M_Type;
													current_outdoor_num = online_unit_count++;
												}else{
//													if(dbgPrintf)(*dbgPrintf)("\nOVER OUTERIOR MACHINE\n");//不支持未断电时替换机组
													return 0;
												}
											}
										}
				                }

							switch (m_Data->Data[1])//状态功能码判断
								{
									case NEEDY_UPDATE:
										if(LED_all_onoroff){//必须是GPRS模块公布准备升级状态后，才会响应机组的升级请求
					                        m_updateInfo1->m_info[current_outdoor_num].have_needupdate = NEEDY_UPDATE;//有机组需要升级
					                        if(dbgPrintf)(*dbgPrintf)("\n<<<<<< RCV NEEDY UPDATE MAC:%04X>>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].m_mac);
											if(m_updatestep != m_monitor){//防止多次写入Flash
												 m_updatestep = m_monitor; //收到外机的公布的状态进入监控状态，等待请求数据
												save_update_machine_flash(m_updateInfo1,err);
											}
										}
										break;
										
									case NONE_UPDATE://任何情况下，只要模块收到无需升级后，即进入升级空闲状态，等待下一次指令，该命令优先级高，为强制性，不考虑模块是否正在发送数据
										m_updateInfo1->m_info[current_outdoor_num].have_needupdate = NORMAL_UPDATE;//机组不需要升级
										if(dbgPrintf)(*dbgPrintf)("\n<<<<<< RCV NONE UPDATE MAC:%04X>>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].m_mac);
										if(updateinfo.update_pack_type == UPDATE_MACHINE_NORMAL){
											m_updateInfo1->m_op_number++;
											m_updatestep = m_register;
										}else{
											m_updatestep = Idle;
										}
//										if(m_updatestep != Idle){
//											m_updatestep = Idle; 
//											save_update_machine_flash(m_updateInfo1,err);
//										}
										break;

									case PUBLISH_STT://在等待接收机组状态数据的时候,机组返回公布状态数据，如果此时机组已经公布需要升级则模块进入升级监控状态
										if(m_updateInfo1->m_info[current_outdoor_num].have_needupdate){//否则模块进入升级空闲状态
//											m_updatestep = m_monitor;
											if(dbgPrintf)(*dbgPrintf)("\n<<<<<< RCV-NEEDY PUBLISH_STT :%04X >>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].m_mac);
										}else{
											if(dbgPrintf)(*dbgPrintf)("\n<<<<<< RCV NORMAL PUBLISH_STT:[MAC]%04X [Ver]:%04X>>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].m_mac,m_Data->Data[0]);
//											if(m_updatestep != Idle){
//												m_updatestep = Idle;//在非升级状态下，直接返回空闲模式，在空闲模式下接受到该状态数据，模块不存储记忆
//												save_update_machine_flash(m_updateInfo1,err);
//											}
										}
										break;

									case RCV_CODE_OVER://强制性的状态，接受到马上转升级成功
										if(m_updateInfo1->m_info[current_outdoor_num].have_needupdate){
											if(m_updatestep != m_codeok){//避免多次写入
												m_updateInfo1->m_info[current_outdoor_num].have_needupdate = NORMAL_UPDATE;
												m_updatestep = m_codeok;
												if(dbgPrintf)(*dbgPrintf)("\n<<<<<< RECEIVE UPDATE CODE OK:%04X>>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].m_mac);
												save_update_machine_flash(m_updateInfo1,err);
											}
										}
										break;									

									case SUCC_UPDATE://机组接收完成以后，会在下一次断上电的时候重启
										if(!m_updateInfo1->m_info[current_outdoor_num].update_ok){
											if(m_updatestep != m_updatesuccess){//避免多次写入
												m_updateInfo1->m_info[current_outdoor_num].update_ok = OK;
												m_updateInfo1->m_info[current_outdoor_num].have_needupdate = NORMAL_UPDATE;
												m_updatestep = m_updatesuccess;
												if(dbgPrintf)(*dbgPrintf)("\n<<<<<< RECEIVE UPDATE SUCCESS :%04X >>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].m_mac);
												save_update_machine_flash(m_updateInfo1,err);
											}
										}
										break;

									case FAIL_UPDATE://升级失败在发生错误的情况下会发送升级失败
				//						if(m_updatestep != m_failure){//否则模块进入升级空闲状态
										if(m_updateInfo1->m_info[current_outdoor_num].have_needupdate){
											m_updateInfo1->m_info[current_outdoor_num].errno = m_Data->Data[2];
											m_updateInfo1->m_info[current_outdoor_num].have_needupdate = NORMAL_UPDATE;
											if(dbgPrintf)(*dbgPrintf)("\n<<<<<< RECEIVE UPDATE FAILURE :%04X>>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].m_mac);
											if(m_updatestep != m_failure){//避免多次写入
												m_updatestep = m_failure;
												save_update_machine_flash(m_updateInfo1,err);
											}
										}
										break;
										
									case BEING_UPDATE:
										if(dbgPrintf)(*dbgPrintf)("\n<<<<<< BEING UPDATE :%04X >>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].m_mac);
										break;

										
									case URGENT_UPDATE://跳转到调试精灵升级模式
										if(m_updatestep == m_register){
											if(check_online_machine_respond(m_updateInfo1)){
												if(dbgPrintf)(*dbgPrintf)("\n<<<<<< RECEIVE ALL URGENT_UPDATE NUMBERS RESPONSE !  >>>>>>\n",current_outdoor_num);
												ConSta.update_type = UPDATE_MACHINE_TYPE;
												m_updateInfo1->m_mac_num_tmp = m_updateInfo1->m_mac_num;
												m_updateInfo1->urg_up = OK;
												IAP_Information_Status.IAP_Enable = ENABLE;
		//										IAP_Information_Status.IAP_Startup = ENABLE;
												if(m_updatestep != m_debugelf){
													m_updatestep = m_debugelf;
													m_updateInfo1->send_bootloader_op = DISABLE;
	//												save_update_machine_flash(m_updateInfo1,err);
												}else{
													m_updatestep = m_debugelf;
												}
												IAP_Information_Status.ACK_Timeout = RESET;
												
											}else{
												m_updateInfo1->m_info[current_outdoor_num].respond_ok = OK;
												if(dbgPrintf)(*dbgPrintf)("\n<<<<<< RECEIVE ENTER URGENT_UPDATE NUMBERS:%04X >>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].m_mac);
											}
										}
										break;

				//					case RCV_CODE_OVER:
				//						if(dbgPrintf)(*dbgPrintf)("\n<<<<<< CODEING RECEIVE OVER UPDATE >>>>>>\n");
				//						break;

								   default://位置状态
										if(dbgPrintf)(*dbgPrintf)("\n<<<<<< UNKOWN STT >>>>>>\n");				   
								   		break;
									
								}
				            }
				            break;
				    case REQ_GET_FUNC:
#ifdef NO_JUDGE_MAC
					have_flag = OK;
					current_outdoor_num = 0;

#else
				     for(i=0; i < m_updateInfo1->m_mac_num; i++){//找到需要升级的外机
				          if(mac == m_updateInfo1->m_info[i].m_mac){
				              current_outdoor_num = i;
							  have_flag = OK;//找到
				              break;
				          }
				      }
#endif					 
					 
					 
				      if(m_Data->M_Type == OUTDOOR_DEV && LED_all_onoroff && have_flag){//外机设备类型,必须等GPRS模块公布状态以后才会响应机组的请求数据
				          if(m_updateInfo1->m_info[current_outdoor_num].have_needupdate == NEEDY_UPDATE){ //机组公布过状态
				              if(((m_updateInfo1->m_info[current_outdoor_num].senddata_flag == GRF_RE_LAST_INDEX)	   || \
							  	m_updateInfo1->m_info[current_outdoor_num].senddata_flag == GRF_BEGIN_INDEX) && \
								(m_updateInfo1->m_info[current_outdoor_num].second_index_respone == WAIT_SECOND_ACK_OK || \
							  		m_updateInfo1->m_info[current_outdoor_num].first_index_respone == WAIT_FIRST_ACK_OK)){//在发送一级索引数据过程中，不接收任何请求
						                  if(m_Data->Lenth <= 2){//收到没有二级索引的请求帧，为请求一级索引所有数据
						                        m_updateInfo1->m_info[current_outdoor_num].first_index = m_Data->Data[0]|(m_Data->Data[1]<<8); 
						                        m_updateInfo1->m_info[current_outdoor_num].senddata_flag = GRF_FIRST_INDEX;//请求一级索引的所有数据
		//				                        m_updateInfo1->m_info[current_outdoor_num].first_index_respone = WAIT_FIRST_ACK_OK;   //一级索引接收完成标志
						                        m_updatestep = m_senddata;                          //标志需要发数据
						                         if(dbgPrintf)(*dbgPrintf)("\n<<<<<< REQ FIRST INDEX:0x%04X >>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].first_index);
										 }else if(m_updateInfo1->m_info[current_outdoor_num].first_index_respone == WAIT_FIRST_ACK_OK &&\
												 m_updateInfo1->m_info[current_outdoor_num].second_index_respone == WAIT_SECOND_ACK_OK){//收到二级索引的请求帧，请求二级索引数据
					                        m_updateInfo1->m_info[current_outdoor_num].second_index = m_Data->Data[2];//请求具体的二级索引数据位置
					                        m_updateInfo1->m_info[current_outdoor_num].senddata_flag = GRF_SECOND_INDEX;//请求二级索引的数据
//						                        m_updateInfo1->m_info[current_outdoor_num].first_index_respone = WAIT_SECOND_ACK_NO;
					                        m_updatestep = m_senddata;                         //标志需要发数据
											if(dbgPrintf)(*dbgPrintf)("\n<<<<<< REQ FIRST INDEX:0x%04X >>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].first_index);                        
											if(dbgPrintf)(*dbgPrintf)("\n<<<<<< REQ SENCOND INDEX:0x%04X >>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].second_index);
										}else{
											  if(dbgPrintf)(*dbgPrintf)("\n<WARNNING>: NO RESPOND SEND-DATA >>>>>>\n");
					                  	}
				              }else{
									  if(dbgPrintf)(*dbgPrintf)("\n<WARNNING>: RECEIVE REQ IN SENDING >>>>>>\n"); 					   
				              }
				          }
				      }
				      break;
				    case REPLY_ACK_FUNC:
#ifdef NO_JUDGE_MAC						
						have_flag = OK;
						current_outdoor_num = 0;

#else
				       for(i=0; i < m_updateInfo1->m_mac_num; i++){
				          if(mac == m_updateInfo1->m_info[i].m_mac){
				              current_outdoor_num = i;
							  have_flag = OK;//找到
				              break;
				          }
				        }
#endif					   
				        if(m_Data->M_Type == OUTDOOR_DEV && LED_all_onoroff && have_flag){
				            
				            if(m_updateInfo1->m_info[current_outdoor_num].have_needupdate == NEEDY_UPDATE){               //机组公布过状态
								   if(m_updateInfo1->m_info[current_outdoor_num].senddata_flag == GRF_RE_LAST_INDEX ||\
											m_updateInfo1->m_info[current_outdoor_num].senddata_flag == GRF_SECOND_INDEX || \
											m_updateInfo1->m_info[current_outdoor_num].senddata_flag == GRF_FIRST_INDEX){//在发送一级索引的数据过程中不响应任何应答
									if(m_updateInfo1->m_info[current_outdoor_num].first_index == (m_Data->Data[0]|(m_Data->Data[1]<<8)) && m_Data->Lenth <= 2 &&\
											m_updateInfo1->m_info[current_outdoor_num].senddata_flag != GRF_SECOND_INDEX){//请求二级索引的过程中，不响应一级索引的响应
										if(dbgPrintf)(*dbgPrintf)("\n<<<<<< ACK FIRST INDEX [0x%04X] >>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].first_index);
					//					if(dbgPrintf)(*dbgPrintf)("\n<<<<<< ACK SECOND INDEX [0x%04X] >>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].second_index);
										m_updateInfo1->m_info[current_outdoor_num].first_index_respone = WAIT_FIRST_ACK_OK; 			  //收到外机发送的应答帧
										m_updateInfo1->m_info[current_outdoor_num].senddata_flag = GRF_BEGIN_INDEX;
										m_updatestep = m_monitor;											//收到外机的接收完一个第一索引的数据进入空闲，等待下个索引的请求数据
										OSSemPost(SemUpdate);
									}else if(m_updateInfo1->m_info[current_outdoor_num].first_index == (m_Data->Data[0]|(m_Data->Data[1]<<8)) && m_Data->Lenth > 2 &&\
										m_updateInfo1->m_info[current_outdoor_num].second_index == m_Data->Data[2]){//只响应特定请求的二级索引
										if(dbgPrintf)(*dbgPrintf)("\n<<<<<< ACK FIRST INDEX [0x%04X] >>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].first_index);
										if(dbgPrintf)(*dbgPrintf)("\n<<<<<< ACK SECOND INDEX [0x%04X] >>>>>>\n",m_updateInfo1->m_info[current_outdoor_num].second_index);
										m_updateInfo1->m_info[current_outdoor_num].second_index_respone = WAIT_SECOND_ACK_OK; //收到外机发送的应答帧
										m_updateInfo1->m_info[current_outdoor_num].senddata_flag = GRF_BEGIN_INDEX;
										m_updatestep = m_monitor;											//收到外机的接收完一个第一索引的数据进入空闲，等待下个索引的请求数据
										OSSemPost(SemUpdate);					
									}else{
										if(dbgPrintf)(*dbgPrintf)("\n<WARNNING>: RECEIVE ERROR-BIT ACK>>>>>>\n");
									}
									}else{
										if(dbgPrintf)(*dbgPrintf)("\n<WARNNING>: RECEIVE ACK IN SENDING >>>>>>\n"); 					 
									
									}
				            
				            }
				        }
				        break;
				       
				    default:
//						if(dbgPrintf)(*dbgPrintf)("\n<<<<<< ...receive machine other function code... >>>>>>\n");
				      break;
				        
				    }
			}else if(m_updateInfo1->urg_up){
				App_Task_Can_Rx(m_Data);//紧急升级状态
		}
#ifdef QUICK_TEST_UPDATE
#else
	}
#endif	
    return 0;
}

/*
single_or_no 是否只发一帧二级索引，1为发一帧，0发全部
*/
void send_firstindexdata(M_updateInfoType* mupdateInfo, uint8_t num, uint8_t single_or_no)
{
    uint32_t temp = 0;
    if(!single_or_no){//发送254B数据
        temp = mupdateInfo->m_update_writeaddr - (PARAM_UPDATE_START_ADDR+mupdateInfo->m_info[num].first_index*SEND_DATA_LEN);
        if(temp >= SEND_DATA_LEN){
            read_can_update(&sendcan_buffer, PARAM_UPDATE_START_ADDR+mupdateInfo->m_info[num].first_index*SEND_DATA_LEN, SEND_DATA_LEN);
        }else{
            read_can_update(&sendcan_buffer, PARAM_UPDATE_START_ADDR+mupdateInfo->m_info[num].first_index*SEND_DATA_LEN, temp);
        }
        while((mupdateInfo->m_info[num].second_index*8) < sendcan_buffer.DataLen){
              temp = sendcan_buffer.DataLen - (mupdateInfo->m_info[num].second_index*8);
              if(temp > 8){
                  mupdateInfo->m_info[num].second_index = send_updateto_can(mupdateInfo->m_info[num].first_index, mupdateInfo->m_info[num].second_index, 8); 
              }else{
                   mupdateInfo->m_info[num].second_index = send_updateto_can(mupdateInfo->m_info[num].first_index, mupdateInfo->m_info[num].second_index, (uint8_t)temp); 
              }
              OSTimeDlyHMSM(0,0,0,5);
        }
        
        mupdateInfo->m_info[num].second_index -= 1;                          //让二级索引指向最后一帧。
        send_updateto_can(mupdateInfo->m_info[num].first_index, mupdateInfo->m_info[num].second_index, (uint8_t)temp);          //重发一次最后一帧，告诉机组发送完毕。
    }else{//发送254B
        if((sendcan_buffer.DataLen - (mupdateInfo->m_info[num].second_index*8))>=8){
            temp = 8;
        }else{
            temp = sendcan_buffer.DataLen - (mupdateInfo->m_info[num].second_index*8);
        }
        send_updateto_can(mupdateInfo->m_info[num].first_index, mupdateInfo->m_info[num].second_index, (uint8_t)temp); 
    }
}



uint32_t read_can_update(BUFF_TYPE *m_sendcan_buffer, uint32_t flash_addr, uint16_t len)
{
    uint16_t checksum = 0, i;
    sFLASH_ReadBuffer((uint8_t *)m_sendcan_buffer->DataBuf, flash_addr, len);
//    flash_addr += len;
//    if(len != 1024){
//        for(i = len; i < 1024; i++){
//            m_sendcan_buffer->DataBuf[i] = 0xFF;
//        }
//    }
    checksum = CRC16_Check(&m_sendcan_buffer->DataBuf[0], len);
    m_sendcan_buffer->DataBuf[len] =(checksum >> 8)& 0xFF;
    m_sendcan_buffer->DataBuf[len+1] =  checksum & 0xFF;
    m_sendcan_buffer->DataLen = len + 2;
    return flash_addr;
}


uint8_t* read_can_update_boot(uint8_t* can_buf,uint32_t flash_address,uint16_t len){
	
	memset(can_buf,0,1024);
	sFLASH_ReadBuffer((uint8_t *)can_buf, flash_address, len);
	return can_buf;
	
	
}

uint32_t respond_can_update(uint8_t respond){
  if(!ConSta.Status){
      if(!connectToServer()){
          while(!disConnectToServer());
      }
  }
  send97Frame(&g_TcpType, REQ_FEEDBACK, respond);
  return 0;
}


void save_update_machine_flash(M_updateInfoType* m_updateInfo1,uint8_t err){
	OSMutexPend(MutexFlash , 0 , &err);
	sFLASH_EraseSubSector(UMACHINE_DATA_START_ADDR);
	sFLASH_WriteBuffer((uint8_t *)&m_updatestep,UMACHINE_DATA_START_ADDR,1);
	sFLASH_WriteBuffer((uint8_t *)&IAP_Information_Status.Data_Tx_Integral_Num,UMACHINE_DATA_START_ADDR + 1,2);
	sFLASH_WriteBuffer((uint8_t *)m_updateInfo1 ,UMACHINE_DATA_START_ADDR+3, sizeof(M_updateInfoType));
	OSMutexPost(MutexFlash);
	return;
}


uint8_t check_online_machine_respond(M_updateInfoType* m_updateInfo2){//检测所有的在线机组是否响应OK？
	uint8_t i;
	for(i=0;i<m_updateInfo2->m_mac_num;i++){
		if(!m_updateInfo2->m_info[i].respond_ok){
			if(m_updateInfo2->m_info[i].m_mac == m_updateInfo2->m_info[current_outdoor_num].m_mac){//判读是否为最后一台机组刚好发送OK的情况
				m_updateInfo2->m_info[i].respond_ok = OK;
				continue;//是最后一台机组刚好发送OK
			}
			return NO;
		}
	}
	if(dbgPrintf)(*dbgPrintf)("\n<<<<<< RECEIVE ENTER URGENT_UPDATE NUMBERS:%04X >>>>>>\n",m_updateInfo2->m_info[current_outdoor_num].m_mac);
	return OK;
}

void updateinfo_init(uint8_t* value1,uint32_t* value2){
	uint8_t i;
	for(i=0;i<m_updateInfo.m_mac_num;i++){
		m_updateInfo.m_info[i].have_needupdate = NORMAL_UPDATE;
		m_updateInfo.m_info[i].errno = ERRNO_NORMAL_ERR;
		m_updateInfo.m_info[i].senddata_flag = GRF_BEGIN_INDEX;
	}
	m_updateInfo.urg_up = DISABLE;
	m_updateInfo.m_op_number = DISABLE;
	*value1 = NO;
	*value2 = NO;
	return;
}

void init_time_clock(){
    memset(&timerbf8[0],0,sizeof(timetype));
	memset(&timerbf8[1],0,sizeof(timetype));
	time_account(&timerbf8[1]);
	return;
}


void init_monitor_stt(uint8_t* value1,uint32_t* value2){//初始化监控状态
	*value1 = NO;
	*value2 = NO;
    memset(&timerbf8[0],0,sizeof(timetype));//清除监控状态计数
    return;
}


uint8_t reback_update_stt(UpDataType* stt,uint8_t opt){
	static uint8_t reback_keep = 0xFF;
	switch (stt->update_stt)
		{
			case REBACK_UPDATE_SUCC:
				respond_can_update(UPGRADE_MACHINE_D_OK);
				break;

			case REABCK_UPDATE_FAIL:
				respond_can_update(UPGRADE_MACHINE_8_FAIL + opt);//反馈服务器升级失败
				break;

			case REBACK_UPDATE_OVER:
				respond_can_update(UPGRADE_MACHINE_8_OK + opt);//反馈服务器升级成功
				break;
			
			case REABCK_UPDATE_BAR:
				if(reback_keep != opt){
					reback_keep = opt;
					respond_can_update(opt*30);//反馈升级进度
				}
				break;
				
			default:
				break;
		}
	stt->update_stt = RESET;
	return 0;
}



