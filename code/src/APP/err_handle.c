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
#include "statistic.h"

uint8_t onlineintimer[INDOOR_MCH_NUM];

/*******************************************************
* Function Name:    binarySearchData
* Purpose:          二分法查找一帧参数据的内容，放到相应的位置
* Params :          PageData:查找表
*                   m_Msg:需要查找的CAN数据
                    m_Data：需要放到的地方
* Return:           
* Limitation: 	    数据表为限定格式，同时表需要按顺序排序。
*******************************************************/
uint16_t binarySearchData(const PageTpye *PageData , const CanMsgType *m_Msg , uint8_t* m_Data)
{
    uint8_t  num = 0 , type = 0 ;
    uint16_t key = 0;                                                           	
    uint16_t low=0 , mid = 0 , high= ((PageData->TableSize) & 0x7FFF) - 1;        		

    if(!(PageData->Page) || !m_Msg || !m_Data||(m_Msg->Lenth == 0)){
        return 0xFFFF;																	
    }

    type = (TypeTable[m_Msg->ID&0x7F] > 1)?0:3;
    key  = ((m_Msg->ID&0x7F)<<8) + (m_Msg->Data[0]&0xFF);//纵坐标<<8 | 横坐标
    num  = ((m_Msg->Lenth & 0x7F) - 1)<<type;// 3.位数据 * 8 , 0.字节数据 * 1
  

    while(low <= high && high != 0){
        mid = (low+high) >> 1 ;
        if((PageData->Page[mid].SrcAdd >= key)&&(PageData->Page[mid].SrcAdd <= key+num)){    
            while(PageData->Page[mid].SrcAdd >= key){
                if(mid-- == 0){
                    mid = -1;
                    break;
                }
            }
            while((PageData->Page[++mid].SrcAdd < (key + num)) && (mid < (PageData->TableSize & 0x7FFF))){
//                if(type == 0){//字节类型
//                    if(m_Data[PageData->Page[mid].Value >> 4] != m_Msg->Data[(PageData->Page[mid].SrcAdd - key) + 1]){
//                        m_Data[PageData->Page[mid].Value >> 4] = m_Msg->Data[(PageData->Page[mid].SrcAdd - key) + 1];
//
//#if DEBUG
//					if(dbgPrintf)(*dbgPrintf)("AA AA 8%X %02X %02X %02X %02X %02X ",m_Msg->Lenth,m_Msg->FunCode,m_Msg->CAN2IP,m_Msg->CAN1IP,m_Msg->DataType,m_Msg->Data[0]);
//					for(uint8_t n=1;n<m_Msg->Lenth;n++){
//						if(dbgPrintf)(*dbgPrintf)("%02X ",m_Msg->Data[n]);
//					}
//					if(dbgPrintf)(*dbgPrintf)("\n");
//#endif
//
//                    }
//                }else 
                if(type){//位类型
                    if((m_Msg->Data[((PageData->Page[mid].SrcAdd - key)>>3)+1] & (1<<((PageData->Page[mid].SrcAdd - key)%8)))>0 ? 1: 0){

					   if(!(m_Data[(PageData->Page[mid].Value >> 4)-1]&(1<< (PageData->Page[mid].Value&0x000F)))){	//判断原来缓存的故障值是否为0，为0则为触发了故障。将对应的行列发出去。
							 Send9Edata[send9e_cnt].data_addr = PageData->Page[mid].SrcAdd;
							 Send9Edata[send9e_cnt].can2_can1_ip = m_Msg->CAN1IP | (m_Msg->CAN2IP<<8);
					   		 Send9Edata[send9e_cnt].error_time_tag = unixTimeStamp;
							 OSQPost(QSend9E,(void *)&Send9Edata[send9e_cnt]);
							 send9e_cnt++;
							 if(send9e_cnt>=QSEND9ELEN){
								 send9e_cnt = 0;
							 }
					 }

						
                    m_Data[(PageData->Page[mid].Value >> 4)-1] |= 1<< (PageData->Page[mid].Value&0x000F);
#if DEBUG
					if(dbgPrintf)(*dbgPrintf)("AA AA 8%X %02X %02X %02X %02X %02X ",m_Msg->Lenth,m_Msg->FunCode,m_Msg->CAN2IP,m_Msg->CAN1IP,m_Msg->DataType,m_Msg->Data[0]);
					for(uint8_t n=1;n<m_Msg->Lenth;n++){
						if(dbgPrintf)(*dbgPrintf)("%02X ",m_Msg->Data[n]);
					}
					if(dbgPrintf)(*dbgPrintf)("\n");
#endif
                    }else{
                            m_Data[(PageData->Page[mid].Value >> 4)-1] &= ~(1<< (PageData->Page[mid].Value&0x000F));
                    }
                }
            }
            return 0;
        }else if(key > (PageData->Page[mid].SrcAdd)){
            low  = mid + 1;                                                     
        }else{
            if(mid < 1) mid = 1;
            high = mid - 1;                                                    
        }
    }
    return 0xFFFE;                                                              
}


/*******************************************************
* Function Name:    check_Err
* Purpose:          检测是否发生了故障，同时将对应的故障进行处理。
* Params :          err_buffer:储存故障状态的变量。             
* Return:           
* Limitation: 	    
*******************************************************/
void check_Err(UpDataType* err_buffer){
//    uint8_t i = 0,j = 0;
	uint8_t i = 0,j = 0, cnt = 0;

    static uint8_t pri_water_full_err[INDOOR_MCH_NUM] = {0};//前一水满保护状态 累计6次上报清零
    static uint8_t pri_freeze_protect[INDOOR_MCH_NUM]  = {0};//前一防冻结保护状态 累计6次上报清零
    static uint8_t pri_indoor_wind_protect[INDOOR_MCH_NUM]  = {0};//前一内风机保护状态 累计6次上报清零
	static uint8_t pri_line_control_problem[INDOOR_MCH_NUM] = {0};//前一线控器供电异常
//	static uint8_t pri_voltage_supply_short[INDOOR_MCH_NUM] = {0};//前一电源供电不足
	static uint8_t pri_swift_wind_error[INDOOR_MCH_NUM] = {0};//前一扫风部件故障
	static uint8_t pri_one_control_multi_machine[INDOOR_MCH_NUM] = {0};//前一控多机故障
	static uint8_t pri_water_switch_problem[INDOOR_MCH_NUM] = {0};//前一水流开关故障
	static uint8_t pri_divide_water_set_problem[INDOOR_MCH_NUM] = {0};//前一分水阀设置故障
	
    for(j=0; j< INDOOR_MCH_NUM; j++){
		if(indoorinfo[j].online_flag){
	        for(i=0; i<INDOORLOCK_ERR_DATA_LEN; i++){
		            if(ACErrParam.indoor_lock_err[j][i]){
		                err_buffer->indoor_lockerr_flag = 1;//存在一个故障，则退出这个内机的循环
		                break;
		            }
//				else if(i == INDOORLOCK_ERR_DATA_LEN-1){//到最后一位为止，标记无故障
	                err_buffer->indoor_lockerr_flag = 0;
	            }
//	        }
        	if(err_buffer->indoor_lockerr_flag == 1) break;//如果发现一台内机错误，则提前结束查找
		}else{
			memset(ACErrParam.indoor_lock_err[j],0,sizeof(ACErrParam.indoor_lock_err)/INDOOR_MCH_NUM);
//			if(dbgPrintf)(*dbgPrintf)("ACErrParam.indoor_lock_err:%d\n",sizeof(ACErrParam.indoor_lock_err)/INDOOR_MCH_NUM);
		}
    }
	
    for(i=0; i<OUTLOCK_ERR_DATA_LEN; i++){//查找外机锁机故障
	        if(ACErrParam.outdoor_lock_err[i]){
	            err_buffer->out_lockerr_flag = 1;//存在则置外机故障位
	            break;
	        }
//		else if(i == OUTLOCK_ERR_DATA_LEN-1){
            err_buffer->out_lockerr_flag = 0;
//        }
    }
	
    for(j=0; j< INDOOR_MCH_NUM; j++){
		if(indoorinfo[j].online_flag){
	        if((ACErrParam.water_full_protect_err[j])&&(!pri_water_full_err[j])){
	            ACErrParam.water_full_protect_err_num[j] += 1;
	        }
			
	        if((ACErrParam.indoor_wind_protect_err[j])&&(!pri_indoor_wind_protect[j])){
	            ACErrParam.indoor_wind_protect_err_num[j] += 1;
	        }
			
	        if((ACErrParam.freeze_protect_err[j])&&(!pri_freeze_protect[j])){
	            ACErrParam.freeze_protect_err_num[j] += 1;
	        }

             if(ACErrParam.divide_water_set_problem[j] && (!pri_divide_water_set_problem[j])){
                 ACErrParam.divide_water_set_problem_num[j]  += 1;    
             }
			 
             if(ACErrParam.line_control_problem[j] && (!pri_line_control_problem[j])){
                 ACErrParam.line_control_problem_num[j]  += 1;
             }
 
             if(ACErrParam.one_control_multi_machine[j] && (!pri_one_control_multi_machine[j])){
                 ACErrParam.one_control_multi_machine_num[j] += 1;
             }
 
             if(ACErrParam.swift_wind_error[j] && (!pri_swift_wind_error[j])){
                 ACErrParam.swift_wind_error_num[j]  += 1;
             }
 
//             if(ACErrParam.voltage_supply_short[j] && (!pri_voltage_supply_short[j])){
//                 ACErrParam.voltage_supply_short_num[j] += 1;
//             }
 
             if(ACErrParam.water_switch_problem[j] && (!pri_water_switch_problem[j])){
                 ACErrParam.water_switch_problem_num[j]   += 1;
             }

			 
	        pri_water_full_err[j] = ACErrParam.water_full_protect_err[j];
	        pri_indoor_wind_protect[j] = ACErrParam.indoor_wind_protect_err[j];
	        pri_freeze_protect[j] = ACErrParam.freeze_protect_err[j];
			pri_divide_water_set_problem[j] = ACErrParam.divide_water_set_problem[j];
			pri_line_control_problem[j] = ACErrParam.line_control_problem[j];
			pri_swift_wind_error[j] = ACErrParam.swift_wind_error[j];
			pri_water_switch_problem[j] = ACErrParam.water_switch_problem[j];
//			pri_voltage_supply_short[j] = ACErrParam.voltage_supply_short[j];
			pri_one_control_multi_machine[j] = ACErrParam.one_control_multi_machine[j];

		}
    }
	
    for(j=0; j< INDOOR_MCH_NUM; j++){
		if(indoorinfo[j].online_flag){
	        if((ACErrParam.water_full_protect_err[j])&&(ACErrParam.water_full_protect_err_num[j] > UNLOCK_ERR_UPLOAD_NUM)){
	            err_buffer->indoor_not_lockerr_flag = 1;
//				SecondErrTable[water_full_protect_err].Value = 1;
//				PageSndErr[0].Page[WATER_FULL_PROTECT_ERR].Value = 1;
				ACErrParam.water_full_protect_err_num[j] = 0;
				 Send9Edata[send9e_cnt].data_addr = 0x1e05;
				 cnt++;

			
	            break;
	        }
	        
	        if((ACErrParam.indoor_wind_protect_err[j])&&(ACErrParam.indoor_wind_protect_err_num[j] > UNLOCK_ERR_UPLOAD_NUM)){
	            err_buffer->indoor_not_lockerr_flag = 1;
//				SecondErrTable[water_full_protect_err].Value = 1;
//				PageSndErr[0].Page[INDOOR_WIND_PROTECT_ERR].Value = 1;
				ACErrParam.indoor_wind_protect_err_num[j] = 0;

                Send9Edata[send9e_cnt].data_addr = 0x1e03;
                cnt++;			
	            break;
	        }
	        if((ACErrParam.freeze_protect_err[j])&&(ACErrParam.freeze_protect_err_num[j] > UNLOCK_ERR_UPLOAD_NUM)){
	            err_buffer->indoor_not_lockerr_flag = 1;
//				SecondErrTable[freeze_protect_err].Value = 1;
//				PageSndErr[0].Page[FREEZE_PROTECT_ERR].Value = 1;
				ACErrParam.freeze_protect_err_num[j] = 0;

				 Send9Edata[send9e_cnt].data_addr = 0x1E07;
				 cnt++;

			
	            break;
	        }

	         if((ACErrParam.one_control_multi_machine[j]) && (ACErrParam.one_control_multi_machine_num[j] > UNLOCK_ERR_UPLOAD_NUM)){
	             err_buffer->indoor_not_lockerr_flag = 1;
//				 SecondErrTable[one_control_multi_machine].Value = 1;
//				 PageSndErr[0].Page[ONE_CONTROL_MULTI_MACHINE].Value = 1;
	             ACErrParam.one_control_multi_machine_num[j] = 0;

				   Send9Edata[send9e_cnt].data_addr = 0x1e18;
				  cnt++;

				 
			 	break;
	         }

	         if((ACErrParam.divide_water_set_problem[j]) && (ACErrParam.divide_water_set_problem_num[j] > UNLOCK_ERR_UPLOAD_NUM)){
	             err_buffer->indoor_not_lockerr_flag = 1;
//				 SecondErrTable[divide_water_set_problem].Value = 1;
//				 PageSndErr[0].Page[DIVIDE_WATER_SET_PROBLEM].Value = 1;
	             ACErrParam.divide_water_set_problem_num[j] = 0;

	             Send9Edata[send9e_cnt].data_addr = 0x1e31;
                cnt++;			 
			 	 break;
	         }

	         if((ACErrParam.swift_wind_error[j]) && (ACErrParam.swift_wind_error_num[j] > UNLOCK_ERR_UPLOAD_NUM)){
	             err_buffer->indoor_not_lockerr_flag = 1;
//				 SecondErrTable[swift_wind_error].Value = 1;
//				 PageSndErr[0].Page[SWIFT_WIND_ERROR].Value = 1;
	             ACErrParam.swift_wind_error_num[j] = 0;

                 Send9Edata[send9e_cnt].data_addr = 0x1e32;
                cnt++;				 
			 	 break;
	         }

//	         if((ACErrParam.voltage_supply_short[j]) && (ACErrParam.voltage_supply_short_num[j] > UNLOCK_ERR_UPLOAD_NUM)){
//	             err_buffer->indoor_not_lockerr_flag = 1;
//	             ACErrParam.voltage_supply_short_num[j] = 0;
//			 	break;
//	         }

	         if((ACErrParam.water_switch_problem[j]) && (ACErrParam.water_switch_problem_num[j] > UNLOCK_ERR_UPLOAD_NUM)){
	             err_buffer->indoor_not_lockerr_flag = 1;
//				 SecondErrTable[water_switch_problem].Value = 1;
//				 PageSndErr[0].Page[WATER_SWITCH_PROBLEM].Value = 1;
	             ACErrParam.water_switch_problem_num[j] = 0;

                 Send9Edata[send9e_cnt].data_addr = 0x1e30;
                cnt++;				 
			 	break;
	         }
			 
	         if((ACErrParam.line_control_problem[j]) && (ACErrParam.line_control_problem_num[j] > UNLOCK_ERR_UPLOAD_NUM)){
	             err_buffer->indoor_not_lockerr_flag = 1;
//				 SecondErrTable[line_control_problem].Value = 1;
//				 PageSndErr[0].Page[LINE_CONTROL_PROBLEM].Value = 1;
	             ACErrParam.line_control_problem_num[j] = 0;

	             Send9Edata[send9e_cnt].data_addr = 0x1e06;
                cnt++;				 
			 	break;
	         }			
			
		}
		
		if(j == (INDOOR_MCH_NUM - 1)){
	            err_buffer->indoor_not_lockerr_flag = 0;
//				for(uint8_t k=0;k<PageSndErr[0].TableSize;k++){
//					PageSndErr[0].Page[k].Value = 0;
//				}
	        }
    }

    if(cnt){                                          //发现 有触发故障。
        Send9Edata[send9e_cnt].can2_can1_ip = 0xFF&j;//三级故障的方式CAN2<<8|CAN1
        Send9Edata[send9e_cnt].error_time_tag = unixTimeStamp;
        OSQPost(QSend9E,(void *)&Send9Edata[send9e_cnt]);
        send9e_cnt++;
        if(send9e_cnt>=QSEND9ELEN){
            send9e_cnt = 0;
        }
    }	
    
    if((err_buffer->indoor_lockerr_flag)||(err_buffer->indoor_not_lockerr_flag)){//判断是否存在
        g_Upload.InErr = 1;
    }else{
        g_Upload.InErr = 0;
    }
    
    if(err_buffer->out_lockerr_flag == 1){
       g_Upload.OutErr = 1; 
    }else{
       g_Upload.OutErr = 0;
    }
}


uint16_t findindoornum(const CanMsgType *m_CanMsg){
    uint16_t i,CanIP;
    uint8_t can2ip;
    if(m_CanMsg->CAN2IP == 0x7F){
       can2ip = 0;
    }else{
       can2ip = m_CanMsg->CAN2IP;
    }
    CanIP = (can2ip<<8)|m_CanMsg->CAN1IP;
    for(i=0; i < INDOOR_MCH_NUM; i++){
        if(CanIP == indoorinfo[i].Can1Can2IP){
            onlineintimer[i] = 0;
            return i;
        }
    }
    return 0x8000;
}

uint8_t check_9E(uint8_t trans_mode,uint8_t mode_state)
{
    send9equeue* p_data = NULL;
    uint8_t p_mac[6] = {0};
    uint8_t err = 0, num = 0, i;
    p_data = (send9equeue*) OSQPend(QSend9E, 1,&err);
    if(p_data){
      if(!ConSta.Status){
	  		SendDataToServer(trans_mode,mode_state);
//              if(!connectToServer()){
//                  while(!disConnectToServer());
//                                  
//                  return mode_state; 
//              }
//              if(!send89Frame(&g_TcpType)){
//                  while(!disConnectToServer());
//                  return mode_state;
//              }
//              
//              if(!sendF3Frame(&g_TcpType)){
//                  while(!disConnectToServer());
//                  return mode_state;
//              }
//  #if 1
//              if(!send91Frame(&g_TcpType , 0xFE)){
//                  while(!disConnectToServer());
//                  return mode_state;
//              }        
//  #endif    
	  }
      
        if(ConSta.Status){
            if((p_data->can2_can1_ip&0xFF00) == (0x0)){//二级故障
                num = (p_data->can2_can1_ip&0x00FF)%INDOOR_MCH_NUM;
                memcpy(p_mac, indoorinfo[num].MAC, sizeof(indoorinfo[num].MAC));
                send9EFrame(&g_TcpType, p_mac, p_data);
	        }else if((p_data->can2_can1_ip&0xFF00) == (0x7F00)){
                if(((p_data->can2_can1_ip&0x00FF))<OUTDOOR_MAX+OUTDOOR_BASE_IP){//外机故障
                     memcpy(p_mac, outdoorinfo.MAC, sizeof(indoorinfo[num].MAC));
                     send9EFrame(&g_TcpType, p_mac, p_data);
            }else if(((p_data->can2_can1_ip&0x00FF))>=INDOOR_BASE){//内机故障
                for(i=0; i<INDOOR_MCH_NUM; i++){
                    if((p_data->can2_can1_ip&0x00FF) == indoorinfo[i].Can1Can2IP){
                         memcpy(p_mac, indoorinfo[i].MAC, sizeof(indoorinfo[i].MAC));
                         send9EFrame(&g_TcpType, p_mac, p_data);
                   	 	}
                	}
                }
            }
#ifdef KEEP_TCP_LIVE
#else
	      if(ConSta.Status){
	          while(!disConnectToServer());
	          return mode_state;
	      }
#endif		  
        }
    }
    return mode_state;
}






