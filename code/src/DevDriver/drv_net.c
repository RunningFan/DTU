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
#include "fsm.h"
#include "Duplicate.h"
#include "real_handle.h"
#include "statistic.h"
#include "bsp.h"


TcpType  g_TcpType;
//uint8_t  g_CacheData[200];
uint16_t send_data_len;//wang
OS_EVENT *MSemTCP;
A2_data a2data_to_GMV;
uint8_t isGetTime;
//uint8_t isOpenSample;
uint8_t Getf3Time;
uint8_t Getf2Time;
uint8_t Getf2Heart;

//uint8_t sendbuff_flag;//是否需要发送到buff中的标志

TypeGPRS GprsParam = {
	.Version = 33,
#if 0		
	.SerPort = 7000,
	.SerName = "192.13.182.156",
#endif

#ifdef TEST_FRAME
//44服务器IP地址
	.SerName = "192.13.182.156",
	.SerPort = 6996,
#else
#ifdef TEST_7005_PORT
	.SerName = "192.13.182.243",
	.SerPort = 7005,
#else	
	.SerName = "192.13.182.243",
	.SerPort = 7001,
#endif	
#endif

#if 0
	.SerName = "192.13.182.243",
	.SerPort = 7001,
#endif

#if 0
		.SerName = "192.13.182.243",
		.SerPort = 7005,
#endif

#if 0
		.SerName = "192.13.182.243",
		.SerPort = 7002,
#endif


#if 0
			.SerName = "bigparse.gree.com",//"192.13.182.243",
			.SerPort = 7103,
#endif

	.bigdata_reduce_outdoor_V = 1,
	.bigdata_reduce_outdoor_Hz = 1,
	.bigdata_reduce_outdoor_100W = 1,
	.bigdata_reduce_outdoor_B = 1,
	.bigdata_reduce_outdoor_kPa = 1,
	.bigdata_reduce_outdoor_10W = 1,
	.bigdata_reduce_outdoor_C = 1,
	.bigdata_reduce_outdoor_A = 1,
	.bigdata_reduce_indoor_B = 1,
	.bigdata_reduce_indoor_rpm = 1,
	.bigdata_reduce_indoor_C = 1,
	.outdoor_power_accuracy = 25,
    .big_data_sampe_time = 60,
//	.big_data_reduce_valve = 1,
	.HeartTime = 10,
	.OutErrTime = 30,//上报外机故障前30分钟
	.Out_Err_After_Time = 5,//外机故障倒计时5分钟
	.HealtTime = 30,
#ifdef DEBUG_BUTT
	.ButtTime = 2,//按键调试持续时间4小时
#else	
	.ButtTime = 240,//按键调试持续时间4小时
#endif	
//	.InerrtTime = 5,//上报内机故障前5分钟
	.InerrtTime = 10,//上报内机故障前5分钟
	.In_Err_After_Time = 2,//内机故障倒计时2分钟
	.MaitTime = 315360000,
	.ApnAccr = "GREEAC.GD",//接入域名
	.ApnUsr = "",//用户名
	.ApnPwd	= "",
	.SmsPwd = "123456",
	.Admin = {"10690370480","10698000065","106550010646","18676489376","18676489371","18926932781","13138125426"},//管理员权限
	.Usron = {"07566990221 ","18676489373","18676489376","13138125426",""},//普通用户权限
	.IMEI = "861024020187777",//模块IMEI
	.ICCID = "8986061509000050609N",//SIM卡ID	106550010646
};


TypeUpdate UpdateParam = 
{
	.FilePath = "http://192.13.182.156:7003/upgrade/sk8.bin",
	.CheckSum = 0xFFFF,
};

/*******************************************************
* Function Name:    handleRxFrame
* Purpose:          解析一帧7E 7E类型的数据帧
* Params :          *m_frame：   处理后的数据放的位置
*                   *m_dataarr： 需要处理的原始数据地址
*                   m_datalenth：需要处理的数据长度
* Return:           TcpType *指向处理完数据的地址
* Limitation: 	    参数不能为空指针；长度不为零；数据有效性通过传入长度和校验码判断
*******************************************************/
TcpType *handleRxFrame(TcpType *m_frame ,const uint8_t *m_dataarr , uint16_t m_datalenth)
{
    
    if(!m_dataarr || !m_frame || !m_datalenth){
#if DEBUG
        if(dbgPrintf)(*dbgPrintf)("Param is NULL!\r\nPlease check input!\r\n");
#endif
        return NULL;
    }else{
        memcpy(&m_frame->RxFrameArr[0] , m_dataarr , m_datalenth % UART_MAX_RX_LEN);

        m_frame->GuideCode = (uint16_t)(m_frame->RxFrameArr[0]<<8)+(uint16_t)m_frame->RxFrameArr[1];
        if(m_frame->GuideCode == 0x7E7E){
            m_datalenth = (uint16_t)(m_frame->RxFrameArr[16] << 8)+(uint16_t)(m_frame->RxFrameArr[17]);//高位在前低位在后 location 1
            if(m_datalenth < UART_MAX_RX_LEN - 18){
                m_frame->CrcValue = CalcCheckSum(&m_frame->RxFrameArr[2], m_datalenth + 16);
            }else{
                return NULL;
            }
            if(m_frame->CrcValue == m_frame->RxFrameArr[m_datalenth + 18]){
                m_frame->DataPtr  = &m_frame->RxFrameArr[2];
                m_frame->DataPtr += sizeof(m_frame->SrcAddr);
			
                memcpy(m_frame->DstAddr , m_frame->DataPtr , sizeof(m_frame->DstAddr));
                m_frame->DataPtr += sizeof(m_frame->DstAddr);

                m_frame->DataLenth = (uint16_t)(*m_frame->DataPtr << 8)+(uint16_t)(*(m_frame->DataPtr + 1));   //高位在前，低位在后
                m_frame->DataPtr += sizeof(m_frame->DataLenth);

                m_frame->ValidDataRx.FuncMode = *m_frame->DataPtr;
                switch(*m_frame->DataPtr)
                {          
                    case 0x89:
                        memcpy(&m_frame->ValidDataRx.frame89Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame89Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame89Rx.FuncMode);
                        memcpy(&m_frame->ValidDataRx.frame89Rx.CheckCode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame89Rx.CheckCode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame89Rx.CheckCode);
                        break;
                        
                    case 0x91:
                        memcpy(&m_frame->ValidDataRx.frame91Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame91Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame91Rx.FuncMode);
                        memcpy(&m_frame->ValidDataRx.frame91Rx.Result , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame91Rx.Result));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame91Rx.Result);
                        break;
                        
                    case 0x92:
                        memcpy(&m_frame->ValidDataRx.frame92Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame92Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame92Rx.FuncMode);
                        memcpy(&m_frame->ValidDataRx.frame92Rx.StopMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame92Rx.StopMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame92Rx.StopMode);    
                        break;   
						
                    case 0x93:
                        memcpy(&m_frame->ValidDataRx.frame93Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame93Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame93Rx.FuncMode);
                        memcpy(&m_frame->ValidDataRx.frame93Rx.Flag , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame93Rx.Flag));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame93Rx.Flag);    
                        break; 
                        
//                    case 0x94:
//                        memcpy(&m_frame->ValidDataRx.frame94Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame94Rx.FuncMode));
//                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame94Rx.FuncMode);
//                        m_frame->ValidDataRx.frame94Rx.Param = m_frame->DataPtr;
//                        m_frame->DataPtr += m_frame->DataLenth - 1;    
//                        break;                       
                        
//                    case 0x95:
//                        memcpy(&m_frame->ValidDataRx.frame95Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame95Rx.FuncMode));
//                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame95Rx.FuncMode);
//                        m_frame->ValidDataRx.frame94Rx.Param = m_frame->DataPtr;
//                        m_frame->DataPtr += m_frame->DataLenth - 1;    
//                        break; 
                        
                    case 0x96:
                        memcpy(&m_frame->ValidDataRx.frame96Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame96Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame96Rx.FuncMode); 
                        memcpy(&m_frame->ValidDataRx.frame96Rx.Res , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame96Rx.Res));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame96Rx.Res); 
                        break; 
                        
                    case 0x97:
                        memcpy(&m_frame->ValidDataRx.frame97Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame97Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame97Rx.FuncMode);
                        memcpy(&m_frame->ValidDataRx.frame97Rx.Update , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame97Rx.Update));
                        m_frame->ValidDataRx.frame97Rx.Param = m_frame->DataPtr;
                        break; 
						
                    case 0x99:
                        memcpy(&m_frame->ValidDataRx.frame99Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame99Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame99Rx.FuncMode);
                        memcpy(&m_frame->ValidDataRx.frame99Rx.config_flag , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame99Rx.config_flag));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame99Rx.config_flag);
//						m_frame->ValidDataRx.frame99Rx.config_time = (uint16_t)(*m_frame->DataPtr++<<8 | *m_frame->DataPtr++);
						m_frame->ValidDataRx.frame99Rx.config_time = (uint16_t)(*m_frame->DataPtr++<<8);//++先用再加
						m_frame->ValidDataRx.frame99Rx.config_time |= (uint16_t)(*m_frame->DataPtr++);
//						memcpy(&m_frame->ValidDataRx.frame99Rx.config_time,m_frame->DataPtr,sizeof(m_frame->ValidDataRx.frame99Rx.config_time));
//						m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame99Rx.config_time);
                        m_frame->ValidDataRx.frame99Rx.DataLen = m_frame->DataLenth - 4;//具体数值待核实
                        m_frame->ValidDataRx.frame99Rx.Data = m_frame->DataPtr;
	                    break; 
						
                    case 0x9A:
                        memcpy(&m_frame->ValidDataRx.frame9ARx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame9ARx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame9ARx.FuncMode);
                        memcpy(&m_frame->ValidDataRx.frame9ARx.result, m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame9ARx.result));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame9ARx.result);     
                        break;  
                        
                    case 0x9B:
                        memcpy(&m_frame->ValidDataRx.frame9BRx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame9BRx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame9BRx.FuncMode);
                        memcpy(&m_frame->ValidDataRx.frame9BRx.result, m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame9BRx.result));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame9BRx.result);     
                        break;  
						
					case 0x9D:
						memcpy(&m_frame->ValidDataRx.frame9DRx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame9DRx.FuncMode));
						m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame9DRx.FuncMode);
						memcpy(&m_frame->ValidDataRx.frame9DRx.Res, m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame9DRx.Res));
						m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame9DRx.Res);   
						break;

                    case 0x9E:
                            memcpy(&m_frame->ValidDataRx.frame9ERx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame9ERx.FuncMode));
                            m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame9ERx.FuncMode);
                            memcpy(&m_frame->ValidDataRx.frame9ERx.Res, m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame9ERx.Res));
                            m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame9ERx.Res);   
                            break;						

					case 0x9F:
						memcpy(&m_frame->ValidDataRx.frame9FRx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame9FRx.FuncMode));
						m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame9FRx.FuncMode);
						memcpy(&m_frame->ValidDataRx.frame9FRx.Res, m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame9FRx.Res));
						m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame9FRx.Res);   
						break;


					case 0xF2:
						memcpy(&m_frame->ValidDataRx.frameF2Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frameF2Rx.FuncMode));
						m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frameF2Rx.FuncMode);
						memcpy(&m_frame->ValidDataRx.frameF2Rx.result, m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frameF2Rx.result));
						m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frameF2Rx.result);   
						break;						
						
                    case 0xF3:
                        memcpy(&m_frame->ValidDataRx.frameF3Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frameF3Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frameF3Rx.FuncMode);
                        memset(&m_frame->ValidDataRx.frameF3Rx.Time , 0 , sizeof(struct tm));
                        m_frame->ValidDataRx.frameF3Rx.Time.tm_year = *m_frame->DataPtr++ ;
                        m_frame->ValidDataRx.frameF3Rx.Time.tm_mon  = *m_frame->DataPtr++ ;
                        m_frame->ValidDataRx.frameF3Rx.Time.tm_mday = *m_frame->DataPtr++ ;
                        m_frame->ValidDataRx.frameF3Rx.Time.tm_hour = *m_frame->DataPtr++ ;
                        m_frame->ValidDataRx.frameF3Rx.Time.tm_min  = *m_frame->DataPtr++ ;
                        m_frame->ValidDataRx.frameF3Rx.Time.tm_sec  = *m_frame->DataPtr++ ;
                        
                        break;
                        
                    case 0xF4:
                        memcpy(&m_frame->ValidDataRx.frameF4Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frameF4Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frameF4Rx.FuncMode);
                        memcpy(&m_frame->ValidDataRx.frameF4Rx.Resault , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frameF4Rx.Resault));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frameF4Rx.Resault);     
                        break; 
						
                    case 0xF5:
						//服务器主动打卡配置参数
						if(m_frame->DataLenth > 3){
	                        memcpy(&m_frame->ValidDataRx.ret.frameF5Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.ret.frameF5Rx.FuncMode));
	                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.ret.frameF5Rx.FuncMode);
	                        memcpy(&m_frame->ValidDataRx.ret.frameF5Rx.config_sign_time, m_frame->DataPtr , sizeof(m_frame->ValidDataRx.ret.frameF5Rx.config_sign_time));
	                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.ret.frameF5Rx.config_sign_time);
	                        memcpy(&m_frame->ValidDataRx.ret.frameF5Rx.config_statistic_time, m_frame->DataPtr , sizeof(m_frame->ValidDataRx.ret.frameF5Rx.config_statistic_time));
	                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.ret.frameF5Rx.config_statistic_time);
						}else{
							//模块接收到的F5回复帧
							memcpy(&m_frame->ValidDataRx.ret.frameF5Rx_sign.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.ret.frameF5Rx_sign.FuncMode));
							m_frame->DataPtr += sizeof(m_frame->ValidDataRx.ret.frameF5Rx_sign.FuncMode);
							
							memcpy(&m_frame->ValidDataRx.ret.frameF5Rx_sign.result,m_frame->DataPtr,sizeof(m_frame->ValidDataRx.ret.frameF5Rx_sign.result));
							m_frame->DataPtr += sizeof(m_frame->ValidDataRx.ret.frameF5Rx_sign.result);							
						}
                        break; 
                        
                   case 0xA3:
                        memcpy(&m_frame->ValidDataRx.frameA3Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frameA3Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frameA3Rx.FuncMode);
                        m_frame->ValidDataRx.frameA3Rx.data = m_frame->DataPtr;
                        m_frame->ValidDataRx.frameA3Rx.datalen = m_datalenth - 1;
                    break;

                    case 0xA2:
                        memcpy(&m_frame->ValidDataRx.frameA2Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frameA2Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frameA2Rx.FuncMode);
					
                        m_frame->ValidDataRx.frameA2Rx.data = a2data_to_GMV.valuedata.databuf;
                        a2data_to_GMV.receiveCnt = 1;
                        a2data_to_GMV.dataLenth = m_frame->DataLenth - (m_frame->DataPtr - &m_frame->RxFrameArr[18]);
                        memcpy(m_frame->ValidDataRx.frameA2Rx.data , m_frame->DataPtr , a2data_to_GMV.dataLenth%500);
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frameA2Rx.data);                        
                    break;					

	              case 0x98:
                        memcpy(&m_frame->ValidDataRx.frame98Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame98Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame98Rx.FuncMode);
                        memcpy(&m_frame->ValidDataRx.frame98Rx.result, m_frame->DataPtr , sizeof(m_frame->ValidDataRx.frame98Rx.result));
                        m_frame->DataPtr += sizeof(m_frame->ValidDataRx.frame98Rx.result);				  	
                   break; 
					   
                    default:
#if DEBUG
                        if(dbgPrintf)(*dbgPrintf)("FuncCode Error!\r\n");
                        if(dbgPrintf)(*dbgPrintf)("Error FuncCode is %02X\r\n" , *m_frame->DataPtr);
#endif
                        return NULL;

                        break;
                }
                return m_frame;
            }else{
#if DEBUG
                if(dbgPrintf)(*dbgPrintf)("Frame CrcValue Error!\r\n");
                if(dbgPrintf)(*dbgPrintf)("Input CrcValue is %02X\r\n" , m_frame->RxFrameArr[m_datalenth - 1]);
                if(dbgPrintf)(*dbgPrintf)("Calcu CrcValue is %02X\r\n" , m_frame->CrcValue);
#endif
                return NULL;
            }
        }else{
#if DEBUG
            if(dbgPrintf)(*dbgPrintf)("GuideCode Error!\r\n");
            if(dbgPrintf)(*dbgPrintf)("GuideCode [%04X] is not %04X\r\n" , m_frame->GuideCode , 0x7E7E);
#endif
            return NULL;
        }
    }
}

uint16_t handleTxFrame(TcpType *m_frame){
	uint8_t err=0;
	uint16_t ret_tx=0;
	OSSemPend(SemHandleTxFrame,0,&err);
	ret_tx = handleTxFrame_Sem(m_frame);
	return ret_tx;
}

/*******************************************************
* Function Name:    handleTxFrame
* Purpose:          将结构体TcpType *m_frame填写完整后，调用该函数将组成相应的数据帧到TXBUF
* Params :          *m_frame：需要处理的数据地址
* Return:           uint16_t：返回组成发送的数据帧长度
* Limitation: 	    注意检查和补充功能码类型及处理方式
*******************************************************/
uint16_t handleTxFrame_Sem(TcpType *m_frame){
    uint16_t i = 0;
//    uint16_t frameA0_Lenth = 0;
    if(!m_frame){
        return 0;
    }
    m_frame->DataPtr = m_frame->TxFrameArr;

    memcpy(m_frame->DataPtr , &m_frame->GuideCode , 2);
    m_frame->DataPtr += sizeof(m_frame->GuideCode);

    memcpy(m_frame->DataPtr , m_frame->DstAddr , sizeof(m_frame->DstAddr));
    m_frame->DataPtr += sizeof(m_frame->DstAddr);

    memcpy(m_frame->DataPtr , m_frame->SrcAddr , sizeof(m_frame->SrcAddr));
    m_frame->DataPtr += sizeof(m_frame->SrcAddr);

    memcpy(m_frame->DataPtr , &m_frame->DataLenth , sizeof(m_frame->DataLenth));
    m_frame->DataPtr += sizeof(m_frame->DataLenth);//暂时占位

    switch(m_frame->ValidDataTx.FuncMode)
    {
        case 0x89://DTU主动发送帧
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame89Tx.CheckCode  , 2);
            m_frame->DataPtr += 2;
//            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame89Tx.SoftVesion , 2);
//            m_frame->DataPtr += 2;			
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame89Tx.HardVesion , 2);
            m_frame->DataPtr += 2;
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame89Tx.SoftVesion , 2);
            m_frame->DataPtr += 2;
            i = strlen((const char *)m_frame->ValidDataTx.frame89Tx.IMEI) + 1;
            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame89Tx.IMEI , i);
            m_frame->DataPtr += i;
            i = strlen((const char *)m_frame->ValidDataTx.frame89Tx.BaseStation) + 1;
            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame89Tx.BaseStation , i);
            m_frame->DataPtr += i;
            i = strlen((const char *)m_frame->ValidDataTx.frame89Tx.ICCID) + 1;
            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame89Tx.ICCID , i);
            m_frame->DataPtr += i;
            break;      
            

        case 0x91://DTU主动发送帧
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame91Tx.TranMode , 1);
            m_frame->DataPtr += 1;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame91Tx.Time.tm_year;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame91Tx.Time.tm_mon;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame91Tx.Time.tm_mday;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame91Tx.Time.tm_hour;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame91Tx.Time.tm_min;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame91Tx.Time.tm_sec;
            break;
            
        case 0x92://服务器发送帧
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame92Tx.Result , sizeof(m_frame->ValidDataTx.frame92Tx.Result));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame92Tx.Result);
            break;

        case 0x93://服务器发送帧
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame93Tx.Result , sizeof(m_frame->ValidDataTx.frame93Tx.Result));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame93Tx.Result);
            break;
            
//        case 0x94://预留
//            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
//            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
//            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame94Tx.FailParam , m_frame->ValidDataTx.frame94Tx.ParamLen);
//            m_frame->DataPtr += m_frame->ValidDataTx.frame94Tx.ParamLen;
//            break;        
            
//        case 0x95://预留
//            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
//            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
//            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame95Tx.Param , m_frame->ValidDataTx.frame95Tx.ParamLen);
//            m_frame->DataPtr += m_frame->ValidDataTx.frame95Tx.ParamLen;
//            break; 
            
        case 0x96://主动发送帧
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame96Tx.Time.tm_year ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame96Tx.Time.tm_mon ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame96Tx.Time.tm_mday ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame96Tx.Time.tm_hour ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame96Tx.Time.tm_min ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame96Tx.Time.tm_sec ;
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame96Tx.TranMode ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame96Tx.MachMode ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame96Tx.TranMode ;
            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame96Tx.Data , m_frame->ValidDataTx.frame96Tx.DataLen);
            m_frame->DataPtr += m_frame->ValidDataTx.frame96Tx.DataLen;
            break; 
            
        case 0x97://返回服务器结果
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame97Tx.update_type , sizeof(m_frame->ValidDataTx.frame97Tx.update_type));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame97Tx.update_type);
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame97Tx.Result , sizeof(m_frame->ValidDataTx.frame97Tx.Result));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame97Tx.Result);
//            i = strlen((const char *)m_frame->ValidDataTx.frame97Tx.mac);
//            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame97Tx.mac , i);
//            m_frame->DataPtr += 7;
//            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame97Tx.program_version , sizeof(m_frame->ValidDataTx.frame97Tx.program_version));
//            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame97Tx.program_version);
            break; 
        case 0x98://主动发送帧：只有实时监控下大于30分钟才用到98帧，98帧与9A帧一样，9A和98并存是为了兼容以前模块
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame98Tx.Time.tm_year ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame98Tx.Time.tm_mon  ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame98Tx.Time.tm_mday ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame98Tx.Time.tm_hour ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame98Tx.Time.tm_min  ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame98Tx.Time.tm_sec  ;            
            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame98Tx.DataPtr , m_frame->ValidDataTx.frame98Tx.DataLenth);
            m_frame->DataPtr += m_frame->ValidDataTx.frame98Tx.DataLenth;
            break;
        case 0x99://服务器发送帧
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame99Tx.Res , sizeof(m_frame->ValidDataTx.frame99Tx.Res));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame99Tx.Res);		
            break;

        case 0x9A://主动发送帧
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9ATx.time.tm_year ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9ATx.time.tm_mon  ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9ATx.time.tm_mday ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9ATx.time.tm_hour ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9ATx.time.tm_min  ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9ATx.time.tm_sec  ;
            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame9ATx.machine_mac , sizeof(m_frame->ValidDataTx.frame9ATx.machine_mac));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame9ATx.machine_mac);
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame9ATx.pro_num , sizeof(m_frame->ValidDataTx.frame9ATx.pro_num));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame9ATx.pro_num);
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9ATx.can1ip ;
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9ATx.can2ip  ;            
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame9ATx.m_id , sizeof(m_frame->ValidDataTx.frame9ATx.m_id));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame9ATx.m_id);
            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame9ATx.data_ptr , m_frame->ValidDataTx.frame9ATx.data_length % 1100);//解决问题 0000023
            m_frame->DataPtr += m_frame->ValidDataTx.frame9ATx.data_length % 1100;
            break;
            
		/*行为习惯帧*/
		case 0x9B:
			memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
			m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
			*m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9BTx.Time.tm_year ;
			*m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9BTx.Time.tm_mon  ;
			*m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9BTx.Time.tm_mday ;
			*m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9BTx.Time.tm_hour ;
			*m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9BTx.Time.tm_min  ;
			*m_frame->DataPtr ++ = m_frame->ValidDataTx.frame9BTx.Time.tm_sec  ;			
//			memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame9BTx.machine_mac , sizeof(m_frame->ValidDataTx.frame9BTx.machine_mac));
//			m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame9BTx.machine_mac); 		   
			memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame9BTx.DataPtr , m_frame->ValidDataTx.frame9BTx.DataLenth);
			m_frame->DataPtr += m_frame->ValidDataTx.frame9BTx.DataLenth;			 
			break;

		case 0x9D://大数据采集帧
			memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
			m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9DTx.Time.tm_year ;
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9DTx.Time.tm_mon  ;
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9DTx.Time.tm_mday ;
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9DTx.Time.tm_hour ;
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9DTx.Time.tm_min  ;
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9DTx.Time.tm_sec  ;
			memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame9DTx.Data , m_frame->ValidDataTx.frame9DTx.DataLen);
			m_frame->DataPtr += m_frame->ValidDataTx.frame9DTx.DataLen;		
			break;

        case 0x9E:
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9ETx.Time.tm_year ;
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9ETx.Time.tm_mon  ;
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9ETx.Time.tm_mday ;
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9ETx.Time.tm_hour ;
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9ETx.Time.tm_min  ;
            *m_frame->DataPtr++ = m_frame->ValidDataTx.frame9ETx.Time.tm_sec  ;
            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame9ETx.MAC , sizeof(m_frame->ValidDataTx.frame9ETx.MAC));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame9ETx.MAC);	
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frame9ETx.errtype , sizeof(&m_frame->ValidDataTx.frame9ETx.errtype));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frame9ETx.errtype);
        break; 			

		case 0x9F://功率采集帧
			memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
			m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
			*m_frame->DataPtr++ = m_frame->ValidDataTx.frame9FTx.Time.tm_year ;
			*m_frame->DataPtr++ = m_frame->ValidDataTx.frame9FTx.Time.tm_mon  ;
			*m_frame->DataPtr++ = m_frame->ValidDataTx.frame9FTx.Time.tm_mday ;
			*m_frame->DataPtr++ = m_frame->ValidDataTx.frame9FTx.Time.tm_hour ;
			*m_frame->DataPtr++ = m_frame->ValidDataTx.frame9FTx.Time.tm_min  ;
			*m_frame->DataPtr++ = m_frame->ValidDataTx.frame9FTx.Time.tm_sec  ;
			memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frame9FTx.Data , m_frame->ValidDataTx.frame9FTx.DataLen);
			m_frame->DataPtr += m_frame->ValidDataTx.frame9FTx.DataLen;
			break;

            
        case 0xEE://主动发送帧
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameEETx.ErrCode , sizeof(m_frame->ValidDataTx.frameEETx.ErrCode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frameEETx.ErrCode);
            break;  

        case 0xF2://模块主动发送帧
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF2Tx.exe_time, 4);
            m_frame->DataPtr += 4;
            break;   			
            
        case 0xF3://主动发送帧
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
            break;
            
//        case 0xF4://主动发送帧
//            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
//            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF4Tx.Time.tm_year ;
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF4Tx.Time.tm_mon  ;
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF4Tx.Time.tm_mday ;
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF4Tx.Time.tm_hour ;
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF4Tx.Time.tm_min  ;
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF4Tx.Time.tm_sec  ; 
//            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF4Tx.Times  , 2);
//            m_frame->DataPtr += 2;
//            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF4Tx.StaAddr  , 2);
//            m_frame->DataPtr += 2;
//            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF4Tx.Signal   , 1);
//            m_frame->DataPtr += 1;
//            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF4Tx.DataType , 1);
//            m_frame->DataPtr += 1;
//            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frameF4Tx.DataPtr , m_frame->ValidDataTx.frameF4Tx.DataLenth);
//            m_frame->DataPtr += m_frame->ValidDataTx.frameF4Tx.DataLenth;
//            break;
			
        case 0xF5://主动发送帧
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
			if(!m_frame->ValidDataTx.frameF5Tx.result){
	            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF5Tx.Time.tm_year ;
	            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF5Tx.Time.tm_mon  ;
	            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF5Tx.Time.tm_mday ;
	            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF5Tx.Time.tm_hour ;
	            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF5Tx.Time.tm_min  ;
	            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameF5Tx.Time.tm_sec  ; 
	            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF5Tx.Times  , 2);
	            m_frame->DataPtr += 2;
	            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF5Tx.StaAddr  , 2);
	            m_frame->DataPtr += 2;
	            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF5Tx.Signal   , 1);
	            m_frame->DataPtr += 1;
	            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF5Tx.Can2IP   , 1);
	            m_frame->DataPtr += 1;
	            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF5Tx.ProNum   , 1);
	            m_frame->DataPtr += 1;			
	            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF5Tx.DataType , 1);
	            m_frame->DataPtr += 1;
	            memcpy(m_frame->DataPtr , m_frame->ValidDataTx.frameF5Tx.DataPtr , m_frame->ValidDataTx.frameF5Tx.DataLenth);
            	m_frame->DataPtr += m_frame->ValidDataTx.frameF5Tx.DataLenth;
			}else{
 	           memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameF5Tx.result , sizeof(m_frame->ValidDataTx.frameF5Tx.result));
  	           m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frameF5Tx.result);				
			}
            break;
            
         case 0xA3:
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.frameA3Tx.FuncMode , sizeof(m_frame->ValidDataTx.frameA3Tx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.frameA3Tx.FuncMode);
            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameA3Tx.Res;
            
        break;

        case 0xA2:
            memcpy(m_frame->DataPtr , &m_frame->ValidDataTx.FuncMode , sizeof(m_frame->ValidDataTx.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidDataTx.FuncMode);
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameA2Tx.Time.tm_year ;
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameA2Tx.Time.tm_mon  ;
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameA2Tx.Time.tm_mday ;
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameA2Tx.Time.tm_hour ;
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameA2Tx.Time.tm_min  ;
//            *m_frame->DataPtr ++ = m_frame->ValidDataTx.frameA2Tx.Time.tm_sec  ;
        break;

		
        default:
            break;
    }

    m_frame->DataLenth = (m_frame->DataPtr - &(m_frame->TxFrameArr[18]));
    m_frame->TxFrameArr[16] = (uint8_t)(m_frame->DataLenth>>8); //高位在前，低位在后
    m_frame->TxFrameArr[17] = (uint8_t)m_frame->DataLenth;      //高位在前，低位在后

    m_frame->CrcValue = CalcCheckSum(&m_frame->TxFrameArr[2], (uint32_t)(m_frame->DataPtr - &m_frame->TxFrameArr[2]));
    memcpy(m_frame->DataPtr , &m_frame->CrcValue , sizeof(m_frame->CrcValue));

    return m_frame->DataLenth + 19;
}

uint8_t to_s_idle;
void tcpDataProcess(TcpType *m_TcpType)
{
    uint8_t err = 0,ee_flag= 0, len = 0;
	uint8_t *ptr = NULL;
    char s[50];
    uint16_t number=0;
    uint16_t m_Lenth = 0,receive_addr = 0, next_addr = 0, res = 0, xor = 0;

    CPU_SR         cpu_sr;
//    uint32_t t = 10000;

    switch (m_TcpType->ValidDataRx.FuncMode)
    {          
        case 0x89:
            OSMboxPost(MSemTCP,(void *)0x89);
            /* 主动发送帧 */
            return;//不回复
        
        case 0x91:
            /* 主动发送帧 */
            OSMboxPost(MSemTCP,(void *)0x91);
            return;//no reply
        
        case 0x92://服务器请求帧
//92帧需要回复          
            m_TcpType->ValidDataTx.frame92Tx.FuncMode = 0x92;
            switch(m_TcpType->ValidDataRx.frame92Rx.StopMode)
            {
                
                case 0x01:               
                    /* 该处为强制停止监控模式 */
//				m_TcpType->ValidDataTx.frame92Tx.Result = 0;
				m_TcpType->ValidDataTx.frame92Tx.Result = 1;//92帧回复结果，强制停止监控成功返回1
					
					if(var.realMonitor){
						var.realMonitor = 0;
                        g_Upload.real98flag = 0;
                        g_Upload.realup98flag = 0;
                        var.Hyaline = 0;
                    	to_s_idle = 1;//强制转空闲模式
                    }
                    break;//reply
                
                default:
					
				m_TcpType->ValidDataTx.frame92Tx.Result = 0;//92帧回复结果，未成功返回0
//                	if(dbgPrintf)(*dbgPrintf)("At 92 frame:case value not 0x01 ->....wrong!...\n");
//                    return ;
            }
            OSMutexPend(MutexFlash , 0 , &err);
            sFLASH_EraseSubSector(VAR_START_ADDR);
            sFLASH_EraseSubSector(ERR_INFO_START_ADDR);
            sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
            sFLASH_WriteBuffer((uint8_t *)&g_Upload , ERR_INFO_START_ADDR , sizeof(UpDataType)-G_UPLOAD_BUFFER_LEN*1024);
		    sFLASH_EraseSubSector(RECEIVE_PAGE_START_ADDR);//增加实时监控退出时，还原默认的监控参数表
		    memset(&Receive_Real_Page,0,sizeof(Receive_Real_Page));
		    memcpy(&Receive_Real_Page,&Real_Page,sizeof(Receive_Real_Page));
		    sFLASH_WriteBuffer((uint8_t *)&Receive_Real_Page , RECEIVE_PAGE_START_ADDR , sizeof(Receive_Real_Page));			
            OSMutexPost(MutexFlash);
            break;//reply
        
        case 0x93:
//            OSMboxPost(MSemTCP,(void *)0x93);//防止在发帧过程中接收到控制帧，使得发帧过程失败
            m_TcpType->ValidDataTx.frame93Tx.FuncMode = 0x93;
            switch(m_TcpType->ValidDataRx.frame93Rx.Flag)
            {
                
                case 0x01: 
                    /* 此处为强制进行透传监控 */
                    var.Hyaline = 1;
                    g_Upload.realup98flag = 1;//透传监控标志位
                    var.realMonitor = 1;
                    g_Upload.indoor_off_noupflag = 0;//强制透传情况下，内机开关机都透传数据
//                    m_TcpType->ValidDataTx.frame93Tx.Result = 0;
//                    break;
					 m_TcpType->ValidDataTx.frame93Tx.Result = 1;
					 break;//退出switch

              
                default: 
                m_TcpType->ValidDataTx.frame93Tx.Result = 0;
//                	if(dbgPrintf)(*dbgPrintf)("At 93 Frame,case value ->...wrong!...\n");
//                    return ;
            }
            OSMutexPend(MutexFlash , 0 , &err);
            sFLASH_EraseSubSector(VAR_START_ADDR);
            sFLASH_EraseSubSector(ERR_INFO_START_ADDR);
            sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
            sFLASH_WriteBuffer((uint8_t *)&g_Upload , ERR_INFO_START_ADDR , sizeof(UpDataType)-G_UPLOAD_BUFFER_LEN*1024);
            OSMutexPost(MutexFlash);

            break;//reply

//		case 0x94://预留 修改DTU模块参数
//			OSMboxPost(MSemTCP,(void*)0x94);
//			m_TcpType->ValidDataTx.frame94Tx.FuncMode = 0x94;
//			m_TcpType->ValidDataTx.frame94Tx.ParamLen = \
//			setGprsParam(m_TcpType->ValidDataRx.frame94Rx.Param, \
//							m_TcpType->DataLenth -1, &GprsParam);
//			OSMutexPend(MutexFlash,0,&err);
//			sFLASH_EraseSubSector(GPRS_INFO_START_ADDR);
//			sFLASH_WriteBuffer((uint8_t *)&GprsParam,GPRS_INFO_START_ADDR, \
//									sizeof(GprsParam));
//			sFLASH_ReadBuffer((uint8_t *)&GprsParam,GPRS_INFO_START_ADDR,\
//									sizeof(GprsParam));
//			OSMutexPost(MutexFlash);
//			return;
			
			
//        case 0x95://预留查询DTU模块参数
//            OSMboxPost(MSemTCP,(void *)0x95);
//            m_TcpType->ValidDataTx.frame95Tx.FuncMode = 0x95;
////            m_TcpType->ValidDataTx.frame95Tx.Param = g_CacheData;
//            m_TcpType->ValidDataTx.frame95Tx.ParamLen = \
//            getGprsParam(m_TcpType->ValidDataRx.frame95Rx.Param , 
//                         m_TcpType->DataLenth - 1 ,
//                         m_TcpType->ValidDataTx.frame95Tx.Param,
//                         &GprsParam);
//            return;
            
        case 0x96://变化帧或透传帧
            OSMboxPost(MSemTCP,(void *)0x96);
            /* 主动发送帧 */
            return;//no reply
        
        case 0x97://
        
//			m_TcpType->ValidDataTx.frame97Tx.FuncMode = 0x97;
            if((var.s_update != 1)&&(m_TcpType->ValidDataRx.frame97Rx.Update)){ 
                if(setupdatParam(m_TcpType->ValidDataRx.frame97Rx.Param ,m_TcpType->DataLenth - 1 ,&UpdateParam,&updateinfo)){
                    var.s_update = 1;
                    ee_flag = 0xBB;
				
//					m_TcpType->ValidDataTx.frame97Tx.Result = 1;
                    OSMutexPend(MutexFlash , 0 , &err);
                    sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
                    sFLASH_EraseSubSector(UPDATE_PARAM_START_ADDR);
                    sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
                    sFLASH_WriteBuffer((uint8_t *)&updateinfo , UPDATE_INFO_START_ADDR + 1 , sizeof(updateinfo));
                    sFLASH_WriteBuffer((uint8_t *)&UpdateParam ,UPDATE_PARAM_START_ADDR , sizeof(UpdateParam));
                    sFLASH_EraseSubSector(VAR_START_ADDR);
                    sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
                    OSMutexPost(MutexFlash);
//					OSMboxPost(MSemTCP,(void *)0x97);
					return;//reply
	             }else{
	//                   m_TcpType->ValidDataTx.frame97Tx.Result = 0;
				      if(!ConSta.Status){
					  	SendDataToServer(IDLE_MODE,s_idle);//如果链接未建立则为空闲模式
//				          if(!connectToServer()){
//				              while(!disConnectToServer());
//				          }
				      }				
					  
					  updateinfo.upgrade_fail_count++;
					  if(dbgPrintf)(*dbgPrintf)("-------->Send update failure!1...<-----------\r\n");
					  send97Frame(&g_TcpType, REQ_FEEDBACK, UPGRADE_FAIL);//收帧情况下，不需要断帧
	//				  if(!send97Frame(&g_TcpType, REQ_FEEDBACK, UPGRADE_FAIL)){
	//					  while(!disConnectToServer());
	//				  }
					  
					  return;//already reply
               	}
				 
            }
//            return ;
//			break;//reply
        case 0x98://变化帧或透传帧
//            OSMboxPost(MSemTCP,(void *)0x98);
            /* 主动发送帧 */
            return;//no reply 			

            
        case 0x99:
//              g_Hyaline = 0;                          
              var.Hyaline = 0;
			  m_TcpType->ValidDataTx.frame99Tx.FuncMode = 0x99;

              stat_buffer.config_flag = m_TcpType->ValidDataRx.frame99Rx.config_flag;
              stat_buffer.config_time = m_TcpType->ValidDataRx.frame99Rx.config_time; 
              if(stat_buffer.config_time > 30){
				  g_Upload.trans_mode = 1;//固定时间点拍照传模式             
              }else{
				  g_Upload.trans_mode = 0;//配置时间变化传+30分钟点拍照传模式                
              }
			  
              var.realMonitor = 1;
			  g_Upload.realup98flag = 0;
              
              if(stat_buffer.config_flag >= 2 && stat_buffer.config_flag <= 3){//有配置信息
                if(stat_buffer.config_flag == 2){
                      g_Upload.indoor_off_noupflag = 0;  //内机全关是否上传数据标识
                      g_Upload.real98flag = 2;
                }else if(stat_buffer.config_flag == 3){
                      g_Upload.indoor_off_noupflag = 1;
					  g_Upload.real98flag = 3;
                }else{
                      g_Upload.indoor_off_noupflag = 0;  //内机全关是否上传数据标识
                      g_Upload.real98flag = 2;					
                }
				take_photo_style = 0;
//				OSQFlush (QSemSend);
                if(m_TcpType->ValidDataRx.frame99Rx.DataLen > 0){
                memset(&Receive_Real_Page,0,sizeof(Receive_Real_Page));
                number = 0;
                while(number < m_TcpType->ValidDataRx.frame99Rx.DataLen){
                    receive_addr =  *(m_TcpType->ValidDataRx.frame99Rx.Data) << 8 | *(m_TcpType->ValidDataRx.frame99Rx.Data+1);
                    m_TcpType->ValidDataRx.frame99Rx.Data += 2;
                    number += 2;
                    res = checkDataAddrr(receive_addr, &Real_Page);
                    if(res != 0xFFFF){
                       if(res == LOW_BIT){                //发现该地址为低位
                            next_addr = *(m_TcpType->ValidDataRx.frame99Rx.Data) << 8 | *(m_TcpType->ValidDataRx.frame99Rx.Data+1);
                            if(next_addr == receive_addr+1){//此处已经对后一位是否为高位做了比较，所以不存在会重复的问题
                                  Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr;
                                  Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = res;
                                  Receive_Real_Page.cnt ++;
                                  Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = next_addr;
                                  Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = HIG_BIT;
                                  Receive_Real_Page.cnt ++;
                                  m_TcpType->ValidDataRx.frame99Rx.Data += 2;
                                  number += 2;
                            }else{
                                 Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr;
                                 Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = res;
                                 Receive_Real_Page.cnt ++;
                                 Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr+1;
                                 Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = HIG_BIT;
                                 Receive_Real_Page.cnt ++;
                            }
                        }else if(res == HIG_BIT){//发现该地址只有一个高位存在
                            Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr - 1;
                            Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = LOW_BIT;
                            Receive_Real_Page.cnt ++;
                            Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr;
                            Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = HIG_BIT;
                            Receive_Real_Page.cnt ++;
                        }else if(res == NOR_BIT){//发现为一个正常的无高低位的类型
                            Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr;
                            Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = res;
                            Receive_Real_Page.cnt ++;
                        }else if((res == ABNOR_LOW8_BIT) && (m_TcpType->ValidDataRx.frame99Rx.Data+6 != NULL)){
							 next_addr = *(m_TcpType->ValidDataRx.frame99Rx.Data + 6) << 8 | *(m_TcpType->ValidDataRx.frame99Rx.Data + 7);
							if(next_addr == receive_addr + 3){
                                  Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr;
                                  Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = res;
                                  Receive_Real_Page.cnt ++;
                                  Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr + 1;
                                  Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = ABNOR_MID16_BIT;
                                  Receive_Real_Page.cnt ++;
                                  Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr + 2;
                                  Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = ABNOR_MID24_BIT;
                                  Receive_Real_Page.cnt ++;
                                  Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr + 3;
                                  Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = ABNOR_HIG8_BIT;
                                  Receive_Real_Page.cnt ++;
                                  m_TcpType->ValidDataRx.frame99Rx.Data += 6;
                                  number += 6;
							}else{
                                  Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr;
                                  Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = res;
                                  Receive_Real_Page.cnt ++;
                                  Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr + 1;
                                  Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = ABNOR_MID16_BIT;
                                  Receive_Real_Page.cnt ++;
                                  Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr + 2;
                                  Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = ABNOR_MID24_BIT;
                                  Receive_Real_Page.cnt ++;
                                  Receive_Real_Page.SrcAdd[ Receive_Real_Page.cnt] = receive_addr + 3;
                                  Receive_Real_Page.hl_flag[ Receive_Real_Page.cnt] = ABNOR_HIG8_BIT;
                                  Receive_Real_Page.cnt ++;
							}
                        }

						if((Receive_Real_Page.cnt > (MAX_RECEIVE_REALPAGE-2))? 1 : 0){
							break;
						}
                    }
                } 
				bubble_sort(&Receive_Real_Page);
				GetRealPagehlflag(&Real_Page,&Receive_Real_Page);
                }           else{
                    if(g_Upload.indoor_off_noupflag){
                        g_Upload.real98flag = 1;
                    }else{
                        g_Upload.real98flag = 0;
                    }
                }
            }else{
				if(stat_buffer.config_flag   == 0){
	                g_Upload.indoor_off_noupflag = 0;
					g_Upload.real98flag = 0;
				}else if(stat_buffer.config_flag == 1){
					g_Upload.real98flag = 1;
					g_Upload.indoor_off_noupflag = 1;
				}else{
					g_Upload.real98flag = 0;
					g_Upload.indoor_off_noupflag = 0;
				}
            }
            g_Upload.EraserFlag = 0xBB;
            OSMutexPend(MutexFlash , 0 , &err);
            sFLASH_EraseSubSector(VAR_START_ADDR);
            sFLASH_EraseSubSector(ERR_INFO_START_ADDR);
            sFLASH_EraseSubSector(RECEIVE_PAGE_START_ADDR);
            sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
            sFLASH_WriteBuffer((uint8_t *)&g_Upload , ERR_INFO_START_ADDR , sizeof(UpDataType)-G_UPLOAD_BUFFER_LEN*1024);
            sFLASH_WriteBuffer((uint8_t *)&Receive_Real_Page , RECEIVE_PAGE_START_ADDR , sizeof(Receive_Real_Page));
            OSMutexPost(MutexFlash);
/*
*			由于使用了stat_buffer来存储配置时间和配置参数，所以此时需要保存该buff，以防断电导致配置不一致
*/
	        uint8_t flash_flag = 0xBB;
			stat_buffer.F4_COUNT = F4_COUNT;
			stat_buffer.F4_DTU_COUNT = F4_DTU_COUNT;
	        OSMutexPend(MutexFlash , 0 , &err);
	        sFLASH_EraseSubSector(STATISTIC_START_ADDR);//擦除一个扇区，大小4KB
	        sFLASH_WriteBuffer(&flash_flag ,STATISTIC_START_ADDR,1);
	        sFLASH_WriteBuffer((uint8_t *)&F4_COUNT ,STATISTIC_START_ADDR+1,2);
	        sFLASH_WriteBuffer((uint8_t *)&stat_buffer ,STATISTIC_START_ADDR+3,sizeof(stat_buffer));
	        sFLASH_WriteBuffer((uint8_t *)&is_lost_data_flag ,STATISTIC_START_ADDR+3,1);
			
	        sFLASH_EraseSubSector(BACKUP_STATISTIC_START_ADDR);//擦除一个扇区，大小4KB
	        sFLASH_WriteBuffer(&flash_flag ,BACKUP_STATISTIC_START_ADDR,1);
	        sFLASH_WriteBuffer((uint8_t *)&F4_COUNT ,BACKUP_STATISTIC_START_ADDR+1,2);
	        sFLASH_WriteBuffer((uint8_t *)&stat_buffer ,BACKUP_STATISTIC_START_ADDR+4,sizeof(stat_buffer));
	        sFLASH_WriteBuffer((uint8_t *)&is_lost_data_flag ,BACKUP_STATISTIC_START_ADDR+3,1);
	        OSMutexPost(MutexFlash);
			m_TcpType->ValidDataTx.frame99Tx.Res = 1;
            /* 主动发送帧 */
//            return;
			break;//reply

        case 0x9A:
            OSMboxPost(MSemTCP,(void *)0x9A);
            return ;//no reply
		
        case 0x9B:
            OSMboxPost(MSemTCP,(void *)0x9B);
            return ;//no reply         
		
        case 0x9D:
            OSMboxPost(MSemTCP,(void *)0x9D);
            return ;//no reply

        case 0x9E:
            OSMboxPost(MSemTCP,(void *)0x9E);
            return ;//no reply  			

        case 0x9F:
            OSMboxPost(MSemTCP,(void *)0x9F);
            return ;//no reply				
		
        case 0xEE:
            OSMboxPost(MSemTCP,(void *)0xEE);
            /* 主动发送帧 */
            return;//no reply


        case 0xF2:
            OSMboxPost(MSemTCP,(void *)0xF2);
            /* 主动发送帧 */
            return;//no reply             
		
        case 0xF3:/* 主动发送帧 */
//判断服务器发送的时间帧是否正常			

			 if(m_TcpType->ValidDataRx.frameF3Rx.Time.tm_year >= 118){//服务器给出的时间需要大于2018年才同步本地时间
				 setACTime(&m_TcpType->ValidDataRx.frameF3Rx.Time);
				 setLocalTime(&m_TcpType->ValidDataRx.frameF3Rx.Time);
				 isGetTime = 1;//同步服务器时间标志
				 OSMboxPost(MSemTCP,(void *)0xF3);
				 return;//no reply			
			 }else{
				 return;//reply
			 }

        case 0xF5:
            /* 主动或服务器发送帧 */
			if(m_TcpType->DataLenth > 3){
	            stat_buffer.usr_hbt_time = m_TcpType->ValidDataRx.ret.frameF5Rx.config_sign_time;
				stat_buffer.stat_data_time = m_TcpType->ValidDataRx.ret.frameF5Rx.config_statistic_time;			
				if(stat_buffer.usr_hbt_time == 0 || stat_buffer.stat_data_time == 0){
					stat_buffer.usr_hbt_time = 168;
					stat_buffer.stat_data_time = 720;
				}
//				        m_TcpType->GuideCode = 0x7E7E;
//        				m_TcpType->ValidDataTx.frameF5Tx.FuncMode = 0xF5;
//						m_TcpType->ValidDataTx.frameF5Tx.result = 1;
//				break;
				m_TcpType->ValidDataTx.frameF5Tx.FuncMode = 0xF5;
				m_TcpType->ValidDataTx.frameF5Tx.result = 1;
				break;//reply

			}else{
				OSMboxPost(MSemTCP,(void *)0xF5);
           		return;//no reply
				//F5帧主动上报的回复帧
			}

	   case 0xA2:
	   OSMboxPost(MSemTCP,(void *)0xA2);
	   return;//no reply

      case 0xA3:
        ptr = m_TcpType->ValidDataRx.frameA3Rx.data;
        while(m_TcpType->ValidDataRx.frameA3Rx.datalen > 0)
        {
            if((0xAA == *ptr)&&(0xAA == *(ptr+1))){
                len = (*(ptr+2))&0x0F;
                xor = *(ptr+len);
                res = XOR_Check(ptr, len+5);           //校验长度为数据加扩展ID
                if(res = xor){
                    ptr++;
                    TxMessage[canTxPut].DLC     = len;
                    TxMessage[canTxPut].RTR     = CAN_RTR_DATA;
                    TxMessage[canTxPut].IDE     = CAN_ID_EXT;
                    TxMessage[canTxPut].FunCode = *ptr++;
                    TxMessage[canTxPut].CAN2IP  = *ptr++;
                    TxMessage[canTxPut].CAN1IP  = *ptr++;
                    TxMessage[canTxPut].DataType= *ptr++;
                    memcpy(TxMessage[canTxPut].Data, ptr, len);            //copy data
                    err = OSQPost(QSem_CAN_Tx,(void *)&TxMessage[canTxPut]);
		            if(OS_ERR_NONE == err){
		                uartRxBuf1[g_UartCnt1].DataLen |= 1<<15;	//标记接收完成
		                g_UartCnt1++;
		            }
		            if(g_UartCnt1 >= UART1_QUEUE_LEN){
		                g_UartCnt1 = 0;
		            }
					if(OS_ERR_NONE == err){
						canTxPut++;
					}
					if(canTxPut >= CANTX_MAX_BUF_LEN){
						canTxPut = 0;
					}
                    ptr +=(len + 2);
                    m_TcpType->ValidDataRx.frameA3Rx.datalen -= (len + 9);
                    continue;
                }
                ptr++;
                m_TcpType->ValidDataRx.frameA3Rx.datalen--;
            }
        }
		
        m_TcpType->ValidDataTx.frameA3Tx.FuncMode = 0xA3;
        m_TcpType->ValidDataTx.frameA3Tx.Res      = 1;
        break;                
        
        default:
        if(dbgPrintf)(*dbgPrintf)("...No this communication frame!...\n");
		if(dbgPrintf)(*dbgPrintf)("This frame function code:%X\r\n",m_TcpType->ValidDataRx.FuncMode);
            return;
    }


    CPU_CRITICAL_ENTER();   /* Tell uC/OS-II that we are starting an ISR*/  
//    memset(g_CacheData , 0 , sizeof(g_CacheData));
    m_Lenth = handleTxFrame(m_TcpType);
    sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
    SendAT(&AT2,s,">",6);
	if(AT2.TxArr)(*AT2.TxArr)(m_TcpType->TxFrameArr , m_Lenth);
    CPU_CRITICAL_EXIT();

     
}


uint16_t setGprsParam(uint8_t *m_Param , uint16_t m_ParamLen ,TypeGPRS *m_GprsParam)
{
    char *p = NULL, *q = NULL;
    uint16_t i = 0;
    if(!m_Param){
        return 0;
    }
    for(i = 0; i < m_ParamLen; i++){
        if(m_Param[i] == '\0'){
            m_Param[i] = '|';
        }
        //m_Param[m_ParamLen - 1] = '\0';
    }
    p = strstr((char const *)m_Param , "PWD");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->SmsPwd , p + 4 , q - p - 4);
    }
    p = strstr((char const *)m_Param , "APN");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->SmsPwd , p + 4 , q - p - 4);
    }
    p = strstr((char const *)m_Param , "APNU");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->ApnUsr , p + 5 , q - p - 5);
    }
    p = strstr((char const *)m_Param , "APNP");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->ApnPwd , p + 5 , q - p - 5);
    }
    p = strstr((char const *)m_Param , "IP");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->SerName , p + 3 , q - p - 3);
    }
    p = strstr((char const *)m_Param , "PORT");
    if(p){
        q = strstr(p , "|");
        *q = '\0';
        m_GprsParam->SerPort = atoi(p + 5);
        *q = '|';
    }
    p = strstr((char const *)m_Param , "IPR");
    if(p){
        /* 暂无此参数 */
    }
    p = strstr((char const *)m_Param , "WT");
    if(p){
        q = strstr(p , "|");
        *q = '\0';
        m_GprsParam->HeartTime = atoi(p + 3);
        *q = '|';
    }
    p = strstr((char const *)m_Param , "ADM*1");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Admin[0] , p + 6 , q - p - 6);
    }
    p = strstr((char const *)m_Param , "ADM*2");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Admin[1] , p + 6 , q - p - 6);
    }
    p = strstr((char const *)m_Param , "ADM*3");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Admin[2] , p + 6 , q - p - 6);
    }
    p = strstr((char const *)m_Param , "ADM*4");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Admin[3] , p + 6 , q - p - 6);
    }
    p = strstr((char const *)m_Param , "ADM*5");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Admin[4] , p + 6 , q - p - 6);
    }
    p = strstr((char const *)m_Param , "USRON*1");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Usron[0] , p + 8 , q - p - 8);
    }
    p = strstr((char const *)m_Param , "USRON*2");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Usron[1] , p + 8 , q - p - 8);
    }
    p = strstr((char const *)m_Param , "USRON*3");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Usron[2] , p + 8 , q - p - 8);
    }
    p = strstr((char const *)m_Param , "USRON*4");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Usron[3] , p + 8 , q - p - 8);
    }
    p = strstr((char const *)m_Param , "USRON*5");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Usron[4] , p + 8 , q - p - 8);
    }
    p = strstr((char const *)m_Param , "USRON*6");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Usron[5] , p + 8 , q - p - 8);
    }
    p = strstr((char const *)m_Param , "USRON*7");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Usron[6] , p + 8 , q - p - 8);
    }
    p = strstr((char const *)m_Param , "USRON*8");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Usron[7] , p + 8 , q - p - 8);
    }
    p = strstr((char const *)m_Param , "USRON*9");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Usron[8] , p + 8 , q - p - 8);
    }
    p = strstr((char const *)m_Param , "USRON*10");
    if(p){
        q = strstr(p , "|");
        memcpy(m_GprsParam->Usron[9] , p + 9 , q - p - 9);
    }
    p = strstr((char const *)m_Param , "INT");
    if(p){
        /* 暂无此参数 */
    }
    p = strstr((char const *)m_Param , "TUM");
    if(p){
        /* 暂无此参数 */
    }
    p = strstr((char const *)m_Param , "UDPPT");
    if(p){
        /* 暂无此参数 */
    }
    p = strstr((char const *)m_Param , "ERRT");
    if(p){
        q = strstr(p , "|");
        *q = '\0';
        m_GprsParam->OutErrTime = atoi(p + 5);
        *q = '|';
    }
    p = strstr((char const *)m_Param , "DEBT");
    if(p){
        /* 暂无此参数 */
    }
    p = strstr((char const *)m_Param , "BUTT");
    if(p){
        q = strstr(p , "|");
        *q = '\0';
        m_GprsParam->ButtTime = atoi(p + 5);
        *q = '|';
    }
    p = strstr((char const *)m_Param , "HEALT");
    if(p){
        q = strstr(p , "|");
        *q = '\0';
        m_GprsParam->HealtTime = atoi(p + 6);
        *q = '|';
    }
    p = strstr((char const *)m_Param , "MAIT");
    if(p){
        q = strstr(p , "|");
        *q = '\0';
        m_GprsParam->MaitTime = atoi(p + 5);
        *q = '|';
    }
    p = strstr((char const *)m_Param , "FTP");
    if(p){
        /* 暂无此参数 */
    }
    return 0;
}


uint16_t getGprsParam(uint8_t *m_Param , uint16_t m_ParamLen ,uint8_t *putParam , TypeGPRS *m_GprsParam)
{
    char *p = NULL, *q = NULL;
    uint16_t i = 0;
    if(!m_Param){
        return 0;
    }
    for(i = 0; i < m_ParamLen; i++){
        if(m_Param[i] == '\0'){
            m_Param[i] = '|';
        }
        //m_Param[m_ParamLen - 1] = '\0';
    }
    q = (char *)putParam;
    p = strstr((char const *)m_Param , "PWD");
    if(p){
        q += sprintf(q , "PWD:%s" , m_GprsParam->SmsPwd);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "APN");
    if(p){
        q += sprintf(q , "APN:%s" , m_GprsParam->ApnAccr);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "APNU");
    if(p){
        q += sprintf(q , "APNU:%s" , m_GprsParam->ApnUsr);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "APNP");
    if(p){
        q += sprintf(q , "APNP:%s" , m_GprsParam->ApnPwd);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "IP");
    if(p){
        q += sprintf(q , "IP:%s" , m_GprsParam->SerName);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "PORT");
    if(p){
        q += sprintf(q , "PORT:%d" , m_GprsParam->SerPort);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "IPR");
    if(p){
        /* 暂无此参数 */
    }
    p = strstr((char const *)m_Param , "WT");
    if(p){
        q += sprintf(q , "WT:%d" , m_GprsParam->HeartTime);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "ADM*1");
    if(p){
        q += sprintf(q , "ADM*1:%s" , m_GprsParam->Admin[0]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "ADM*2");
    if(p){
        q += sprintf(q , "ADM*2:%s" , m_GprsParam->Admin[1]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "ADM*3");
    if(p){
        q += sprintf(q , "ADM*3:%s" , m_GprsParam->Admin[2]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "ADM*4");
    if(p){
        q += sprintf(q , "ADM*4:%s" , m_GprsParam->Admin[3]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "ADM*5");
    if(p){
        q += sprintf(q , "ADM*5:%s" , m_GprsParam->Admin[4]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "USRON*1");
    if(p){
        q += sprintf(q , "USRON*1:%s" , m_GprsParam->Usron[0]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "USRON*2");
    if(p){
        q += sprintf(q , "USRON*2:%s" , m_GprsParam->Usron[1]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "USRON*3");
    if(p){
        q += sprintf(q , "USRON*3:%s" , m_GprsParam->Usron[2]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "USRON*4");
    if(p){
        q += sprintf(q , "USRON*4:%s" , m_GprsParam->Usron[3]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "USRON*5");
    if(p){
        q += sprintf(q , "USRON*5:%s" , m_GprsParam->Usron[4]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "USRON*6");
    if(p){
        q += sprintf(q , "USRON*6:%s" , m_GprsParam->Usron[5]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "USRON*7");
    if(p){
        q += sprintf(q , "USRON*7:%s" , m_GprsParam->Usron[6]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "USRON*8");
    if(p){
        q += sprintf(q , "USRON*8:%s" , m_GprsParam->Usron[7]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "USRON*9");
    if(p){
        q += sprintf(q , "USRON*9:%s" , m_GprsParam->Usron[8]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "USRON*10");
    if(p){
        q += sprintf(q , "USRON*10:%s" , m_GprsParam->Usron[9]);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "INT");
    if(p){
        /* 暂无此参数 */
    }
    p = strstr((char const *)m_Param , "TUM");
    if(p){
        /* 暂无此参数 */
    }
    p = strstr((char const *)m_Param , "UDPPT");
    if(p){
        /* 暂无此参数 */
    }
    p = strstr((char const *)m_Param , "ERRT");
    if(p){
        q += sprintf(q , "ERRT:%d" , m_GprsParam->OutErrTime);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "DEBT");
    if(p){
        /* 暂无此参数 */
    }
    p = strstr((char const *)m_Param , "BUTT");
    if(p){
        q += sprintf(q , "BUTT:%d" , m_GprsParam->ButtTime);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "HEALT");
    if(p){
        q += sprintf(q , "HEALT:%d" , m_GprsParam->HealtTime);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "MAIT");
    if(p){
        q += sprintf(q , "MAIT:%d" , m_GprsParam->MaitTime);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "FTP");
    if(p){
        /* 暂无此参数 */
    }
    return ((uint32_t)q - (uint32_t)putParam);
}

uint16_t send89Frame(TcpType *m_Frame)
{
    uint8_t *Mac = m_Frame->SrcAddr;
    uint8_t err = 0, res = 0 , Times = 0 ;
    uint16_t m_Lenth = 0;
    uint16_t i = 0;
    char * s = NULL;
    char q[50];
    s = q;
    do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0x89;
        m_Frame->ValidDataTx.frame89Tx.BaseStation = (uint8_t *)GprsParam.CellInfo;
        m_Frame->ValidDataTx.frame89Tx.CheckCode = Mac[0]*3+Mac[1]*6+Mac[2]*0+Mac[3]*9+Mac[4]*7+Mac[5]*4+Mac[6]*10;
        m_Frame->ValidDataTx.frame89Tx.ICCID = (uint8_t *)GprsParam.ICCID;
        m_Frame->ValidDataTx.frame89Tx.IMEI  = (uint8_t *)GprsParam.IMEI;
        m_Frame->ValidDataTx.frame89Tx.HardVesion = HARDVERSION;
        m_Frame->ValidDataTx.frame89Tx.SoftVesion = SOFTVERSION;
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s ,"AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x89){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}		
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 2000 , &err ) - 0);
        if(Times++ >= 2){
            break;
        }
    }while(res != 0x89);
    if(res == 0x89){

        return 1;
    }else{
        return 0;
    }
}

uint16_t sendF3Frame(TcpType *m_Frame)
{
    uint8_t err = 0, res = 0 , Times = 0 ;
    uint16_t m_Lenth = 0;
    uint16_t i = 0;
    char * s =NULL;
    char q[50];
    s = q;
    do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0xF3;
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0xF3){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}		
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 2000 , &err ) - 0);
        if(Times++ >= 2){
            break;
        }
    }while(res != 0xF3);
    if(res == 0xF3){
        return 1;
    }else{
        return 0;
    }
}


uint16_t sendF2Frame(TcpType *m_Frame)
{
    uint8_t err = 0, res = 0 , Times = 0 ;
    uint16_t m_Lenth = 0;
    uint16_t i = 0;
    char * s =NULL;
    char q[50];
    s = q;
#ifdef TEST_FRAME
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0xF2;
        m_Frame->ValidDataTx.frameF2Tx.exe_time = stat_buffer.DTU_GET_ELEC_TIME;
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        return 1;
#else
    do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0xF2;
        m_Frame->ValidDataTx.frameF2Tx.exe_time = stat_buffer.DTU_GET_ELEC_TIME;
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0xF2){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}		
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 500 , &err ) - 0);
        if(Times++ >= 2){
            break;
        }
    }while(res != 0xF2);
    if(res == 0xF2){
        return 1;
    }else{
        return 0;
    }

#endif
}




uint16_t send91Frame(TcpType *m_Frame , uint8_t TranMode)
{
    uint8_t err = 0, res = 0 , Times = 0 ;
    uint16_t m_Lenth;
    uint16_t i = 0;
    char * s = NULL;
    char q[50];
    s = q;
    do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0x91;
        getLocalTime(&m_Frame->ValidDataTx.frame91Tx.Time);
        m_Frame->ValidDataTx.frame91Tx.TranMode = TranMode;
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x91){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}		
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP, 2000 , &err ) - 0);
        if(Times++ >= 2){
            break;
        }
    }while(res != 0x91);
    if(res == 0x91){
        return 1;
    }else{
        return 0;
    }
}

void sendEEFrame(TcpType *m_Frame , uint8_t ErrCode)
{
        uint16_t m_Lenth;
		uint8_t res=0,Times=0,err=0;
        uint16_t i = 0;
	    char * s = NULL;
	    char q[50];
	    s = q;		
	do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0xEE;
        m_Frame->ValidDataTx.frameEETx.ErrCode = ErrCode;
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0xEE){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}			
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
        LedValue.ConSta  = 3;
        LedValue.ConSpd  = 5;
        LedValue.BlinkTm = 6;
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP, 500 , &err ) - 0);
        if(Times++ >= 0){
            break;
        }
	    }while(res != 0xEE);
}


uint16_t send96Frame(TcpType *m_Frame , uint8_t *dat , uint16_t datLen , uint8_t m_mode ,uint8_t t_mode)
{
    uint16_t m_Lenth=0;
    uint8_t res = 0;
    uint8_t err = 0;
    uint8_t count = 0;
    uint16_t i = 0;
    char * s = NULL;
    char q[50];
    s = q;
    time_t TimeStamp;
#ifdef TEST_FRAME	
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0x96;
        memcpy(&TimeStamp , &dat[2] , 4);
        getTime(&m_Frame->ValidDataTx.frame96Tx.Time , TimeStamp);
        m_Frame->ValidDataTx.frame96Tx.Data = &dat[6];
        m_Frame->ValidDataTx.frame96Tx.MachMode= m_mode;
        m_Frame->ValidDataTx.frame96Tx.TranMode= t_mode;
        m_Frame->ValidDataTx.frame96Tx.DataLen = datLen;
        m_Lenth = handleTxFrame(m_Frame);  
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
        if(fail_count >= 5){
        OSTimeDlyHMSM(0,0,10,0);
        fail_count = 0;}
        	if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
#else
    do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0x96;
        memcpy(&TimeStamp , &dat[2] , 4);
        getTime(&m_Frame->ValidDataTx.frame96Tx.Time , TimeStamp);
        m_Frame->ValidDataTx.frame96Tx.Data = &dat[6];
        m_Frame->ValidDataTx.frame96Tx.MachMode= m_mode;
        m_Frame->ValidDataTx.frame96Tx.TranMode= t_mode;
        m_Frame->ValidDataTx.frame96Tx.DataLen = datLen;
        m_Lenth = handleTxFrame(m_Frame);  
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
        if(fail_count >= 2){//96帧透传时候使用
	        OSTimeDlyHMSM(0,0,5,0);
	        fail_count = 0;
		}
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x96){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}
    	if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        if(t_mode == 1){
            res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP, 5000 , &err) - 0);
        }
        if(count ++ >= 2){
            break;
		}
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
    }while((res != 0x96)&&(t_mode == 1));
#endif
	
    if((var.Hyaline == 0)){
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
    }else if(var.Hyaline == 1){
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
    }
    return m_Lenth;
}



//具体故障上报帧

uint16_t send9EFrame(TcpType *m_Frame , uint8_t *mac , send9equeue* ptr_9e)
{
    uint16_t m_Lenth=0;
    uint8_t res = 0;
    uint8_t err = 0;
    uint8_t count = 0;
    uint16_t i = 0;
    char * s = NULL;
    char q[50];
    s = q;	

#ifdef TEST_FRAME

        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0x9E;
		getTime(&m_Frame->ValidDataTx.frame9ETx.Time,ptr_9e->error_time_tag);
//        memcpy(&TimeStamp , &dat[2] , 4);
//        getTime(&m_Frame->ValidDataTx.frame9ETx.Time , TimeStamp);
//        m_Frame->ValidDataTx.frame9ETx.MAC[] = &mac[0];
        memcpy(m_Frame->ValidDataTx.frame9ETx.MAC,mac,sizeof(m_Frame->ValidDataTx.frame9ETx.MAC));
        m_Frame->ValidDataTx.frame9ETx.errtype = ((ptr_9e->data_addr&0xFF) << 8) | ((ptr_9e->data_addr>>8)&0xFF);
//        m_Frame->ValidDataTx.frame9ETx.DataLen = 9;
        m_Lenth = handleTxFrame(m_Frame);  
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x9E){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}	
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
			
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
//        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP, 5000 , &err) - 0);
//        if(count++ >= 2){
//            break;
//	}
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
		return 1;

#else

    do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0x9E;
		getTime(&m_Frame->ValidDataTx.frame9ETx.Time,ptr_9e->error_time_tag);
//        memcpy(&TimeStamp , &dat[2] , 4);
//        getTime(&m_Frame->ValidDataTx.frame9ETx.Time , TimeStamp);
//        m_Frame->ValidDataTx.frame9ETx.MAC[] = &mac[0];
        memcpy(m_Frame->ValidDataTx.frame9ETx.MAC,mac,sizeof(m_Frame->ValidDataTx.frame9ETx.MAC));
        m_Frame->ValidDataTx.frame9ETx.errtype = ((ptr_9e->data_addr&0xFF) << 8) | ((ptr_9e->data_addr>>8)&0xFF);
//        m_Frame->ValidDataTx.frame9ETx.DataLen = 9;
        m_Lenth = handleTxFrame(m_Frame);  
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x9E){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}	
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
			
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP, 5000 , &err) - 0);
        if(count++ >= 2){
            break;
	}
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
    }while(res != 0x9E);
    if(res == 0x9E){
        return 1;
    }else{
        return 0;
    }
#endif	
}


//uint16_t send9EFrame(TcpType *m_Frame , uint8_t *dat , uint16_t datLen)
//{
//    uint16_t m_Lenth=0;
//    uint8_t res = 0;
//    uint8_t err = 0;
//    uint8_t count = 0;
//    uint16_t i = 0;
//    char * s = NULL;
//    char q[50];
//    s = q;
//    time_t TimeStamp;
//	do{
//		m_Frame->GuideCode = 0x7E7E;
//		m_Frame->ValidDataTx.FuncMode = 0x9E;
//		memcpy(&TimeStamp , &dat[2] , 4);
//		getTime(&m_Frame->ValidDataTx.frame9FTx.Time , TimeStamp);
//		m_Frame->ValidDataTx.frame9FTx.Data = &dat[6];
//		m_Frame->ValidDataTx.frame9FTx.DataLen = datLen;
//		m_Lenth = handleTxFrame(m_Frame);  
//		sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
//		SendAT(&AT2,s,">",2);
////		if(fail_count >= 5){
////			OSTimeDlyHMSM(0,0,5,0);
////			fail_count = 0;
////		}
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x9F){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}		
//		if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
//			
//#if SRV_DEBUG
//		for(i = 0;i<m_Lenth; i++){
//			if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
//			if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
//			if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
//#endif
//
//		res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP, 2000 , &err) - 0);
//		if(count++ >= 2){
//			break;
//		}
//		LedValue.ConSta = 3;
//		LedValue.ConSpd = 5;
//		LedValue.BlinkTm = 6;
//	}while(res != 0x9F);
//	if(res == 0x9F){
//		return 0;
//	}else{
//		return 1;
//	}
//}


//功率帧
uint16_t send9FFrame(TcpType *m_Frame , uint8_t *dat , uint16_t datLen)
{
    uint16_t m_Lenth=0;
    uint8_t res = 0;
    uint8_t err = 0;
    uint8_t count = 0;
    uint16_t i = 0;
    char * s = NULL;
    char q[50];
    s = q;
    time_t TimeStamp;
	do{
		m_Frame->GuideCode = 0x7E7E;
		m_Frame->ValidDataTx.FuncMode = 0x9F;
		memcpy(&TimeStamp , &dat[4] , 4);
		getTime(&m_Frame->ValidDataTx.frame9FTx.Time , TimeStamp);
		m_Frame->ValidDataTx.frame9FTx.Data = &dat[8];
		m_Frame->ValidDataTx.frame9FTx.DataLen = datLen;
		m_Lenth = handleTxFrame(m_Frame);  
		sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
		SendAT(&AT2,s,">",2);
//		if(fail_count >= 5){
//			OSTimeDlyHMSM(0,0,5,0);
//			fail_count = 0;
//		}
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x9F){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}		
		if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
			
#if SRV_DEBUG
		for(i = 0;i<m_Lenth; i++){
			if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
			if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
			if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif

		res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP, 2000 , &err) - 0);
		if(count++ >= 2){
			break;
		}
		LedValue.ConSta = 3;
		LedValue.ConSpd = 5;
		LedValue.BlinkTm = 6;
	}while(res != 0x9F);
	if(res == 0x9F){
		return 0;
	}else{
		return 1;
	}
}


uint16_t send9DFrame(TcpType *m_Frame , uint8_t *dat , uint16_t datLen)
{
    uint16_t m_Lenth=0;
    uint8_t res = 0;
    uint8_t err = 0;
    uint8_t count = 0;
    uint16_t i = 0;
    char * s = NULL;
    char q[50];
    s = q;
    time_t TimeStamp;
	
#ifdef TEST_FRAME	
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0x9D;
        memcpy(&TimeStamp , &dat[2] , 4);
        getTime(&m_Frame->ValidDataTx.frame9DTx.Time , TimeStamp);
        m_Frame->ValidDataTx.frame9DTx.Data = &dat[6];
        m_Frame->ValidDataTx.frame9DTx.DataLen = datLen;
        m_Lenth = handleTxFrame(m_Frame);  
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
        	if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
			
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif

        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
		return 0;

		
#else


    do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0x9D;
        memcpy(&TimeStamp , &dat[2] , 4);
        getTime(&m_Frame->ValidDataTx.frame9DTx.Time , TimeStamp);
        m_Frame->ValidDataTx.frame9DTx.Data = &dat[6];
        m_Frame->ValidDataTx.frame9DTx.DataLen = datLen;
        m_Lenth = handleTxFrame(m_Frame);  
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
//        if(fail_count >= 5){
//	        OSTimeDlyHMSM(0,0,5,0);
//	        fail_count = 0;
//		}
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x9D){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}		
        	if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
			
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif



        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP, 2000 , &err) - 0);
        if(count++ >= 3){
            break;
		}
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
    }while(res != 0x9D);
	if(res == 0x9D){
		return 0;
	}else{
	    return 1;
	}

#endif
}


uint16_t sendF5Frame(TcpType *m_Frame , uint16_t start_addr,uint8_t data_type,uint8_t *dat , uint16_t datLen ,uint16_t Times,uint8_t can2ip)
{
    uint16_t m_Lenth;
    uint16_t i = 0;
    uint8_t err = 0, res = 0 , count = 0 ;
    char * s = NULL;
    char q[50];
    s = q;
#ifdef TEST_FRAME	
//    do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0xF5;
        m_Frame->ValidDataTx.frameF5Tx.Times = Times;
        m_Frame->ValidDataTx.frameF5Tx.StaAddr = start_addr;
        m_Frame->ValidDataTx.frameF5Tx.Signal = LedValue.Signal;
//        m_Frame->ValidDataTx.frameF5Tx.DataType = data_type; 
		m_Frame->ValidDataTx.frameF5Tx.DataType = data_type;
		m_Frame->ValidDataTx.frameF5Tx.result = 0;//区分打卡帧和打卡配置帧：0打卡帧，1打卡配置时间帧
		
        if(data_type == 0x00){
            m_Frame->ValidDataTx.frameF5Tx.DataLenth = 0; 
        }else{
            m_Frame->ValidDataTx.frameF5Tx.DataLenth = datLen;
        }
        m_Frame->ValidDataTx.frameF5Tx.DataPtr  = dat + start_addr;
        m_Frame->ValidDataTx.frameF5Tx.Can2IP   = can2ip;
        m_Frame->ValidDataTx.frameF5Tx.ProNum   = 0x01;
        m_Lenth = handleTxFrame(m_Frame);

		sprintf(s,"AT+CIPSEND=%d\r",m_Lenth);
		SendAT(&AT2,s,">",2);
//		if(fail_count >= 5){
//			OSTimeDlyHMSM(0,0,5,0);
//			fail_count = 0;
//		}
		if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr,m_Lenth);

#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif        
        send_data_len = m_Lenth;
        LedValue.ConSta  = 3;
        LedValue.ConSpd  = 5;
        LedValue.BlinkTm = 6;
//        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 2000 , &err ) - 0);
//        if(count ++ >= 3){
//            break;
//        }
//    }while(res != 0xF5);
//    if(res == 0xF5){
        return 1;
//    }else{
//        return 0;
//    }
#else
    do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0xF5;
        m_Frame->ValidDataTx.frameF5Tx.Times = Times;
        m_Frame->ValidDataTx.frameF5Tx.StaAddr = start_addr;
        m_Frame->ValidDataTx.frameF5Tx.Signal = LedValue.Signal;
//        m_Frame->ValidDataTx.frameF5Tx.DataType = data_type; 
		m_Frame->ValidDataTx.frameF5Tx.DataType = data_type;
		m_Frame->ValidDataTx.frameF5Tx.result = 0;//区分打卡帧和打卡配置帧：0打卡帧，1打卡配置时间帧
		
        if(data_type == 0x00){
            m_Frame->ValidDataTx.frameF5Tx.DataLenth = 0; 
        }else{
            m_Frame->ValidDataTx.frameF5Tx.DataLenth = datLen;
        }
        m_Frame->ValidDataTx.frameF5Tx.DataPtr  = dat + start_addr;
        m_Frame->ValidDataTx.frameF5Tx.Can2IP   = can2ip;
        m_Frame->ValidDataTx.frameF5Tx.ProNum   = 0x01;
        m_Lenth = handleTxFrame(m_Frame);

		sprintf(s,"AT+CIPSEND=%d\r",m_Lenth);
		SendAT(&AT2,s,">",2);
		
//		if(fail_count >= 5){
//			OSTimeDlyHMSM(0,0,5,0);
//			fail_count = 0;
//		}
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0xF5){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}		
		if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr,m_Lenth);

#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif        
        send_data_len = m_Lenth;
        LedValue.ConSta  = 3;
        LedValue.ConSpd  = 5;
        LedValue.BlinkTm = 6;
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 2000 , &err ) - 0);
        if(count ++ >= 2){
            break;
        }
    }while(res != 0xF5);
    if(res == 0xF5){
        return 1;
    }else{
        return 0;
    }

#endif
}


uint16_t sendF4Frame(TcpType *m_Frame , uint16_t start_addr,uint8_t data_type,uint8_t *dat , uint16_t datLen ,uint16_t Times)
{
    uint16_t m_Lenth;
    uint16_t i = 0;
    uint8_t err = 0, res = 0 , count = 0 ;
    char * s = NULL;
    char q[50];
    s = q;
    do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0xF4;
        m_Frame->ValidDataTx.frameF4Tx.Times = Times;
        m_Frame->ValidDataTx.frameF4Tx.StaAddr = start_addr;
        m_Frame->ValidDataTx.frameF4Tx.Signal = LedValue.Signal;
        m_Frame->ValidDataTx.frameF4Tx.DataType = data_type; 
        m_Frame->ValidDataTx.frameF4Tx.DataLenth = datLen; 
        m_Frame->ValidDataTx.frameF4Tx.DataPtr  = dat + start_addr;
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
        SendAT(&AT2,s,">",2);
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
        LedValue.ConSta  = 3;
        LedValue.ConSpd  = 5;
        LedValue.BlinkTm = 6;
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 5000 , &err ) - 0);
        if(count++ >= 3){
            break;
        }
    }while(res != 0xF4);
    if(res == 0xF4){
        return 1;
    }else{
        return 0;
    }
}

uint16_t send97Frame(TcpType *m_Frame, uint8_t update_type, uint8_t result)
{
    uint16_t m_Lenth;
    uint8_t res,err,times=0;
    uint16_t i = 0;
    char * s = NULL;
    char q[50];
    s = q;
//	do{
	    m_Frame->GuideCode = 0x7E7E;
	    m_Frame->ValidDataTx.FuncMode = 0x97;
	    m_Frame->ValidDataTx.frame97Tx.update_type = update_type;
	    m_Frame->ValidDataTx.frame97Tx.Result = result;
	    if(update_type == 1){
	            m_Frame->ValidDataTx.frame97Tx.mac = out_mac;
	            memcpy(m_Frame->ValidDataTx.frame97Tx.program_version, (uint8_t *)&machineID, sizeof(machineID));
	            m_Frame->ValidDataTx.frame97Tx.program_version[2] = program_version;
	            m_Lenth = handleTxFrame(m_Frame);
	            sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
	            SendAT(&AT2,s,">",2);
//				if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x97){
//					OSSemPost(SemHandleTxFrame);	
//					continue;
//				}					
	            if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
	    }else if(update_type == 2){
		        m_Frame->ValidDataTx.frame97Tx.program_version[2] = SOFTVERSION;
		        memset(&m_Frame->ValidDataTx.frame97Tx.program_version[0], 0, 2);
		        m_Frame->ValidDataTx.frame97Tx.mac = m_Frame->SrcAddr;
		        m_Lenth = handleTxFrame(m_Frame);
		        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
		        SendAT(&AT2,s,">",2);
//				if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x97){
//					OSSemPost(SemHandleTxFrame);	
//					continue;
//				}				
		        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
	    }else if(update_type == 0){
		        m_Frame->ValidDataTx.frame97Tx.mac = (uint8_t *)s;
		        memset(m_Frame->ValidDataTx.frame97Tx.program_version, 0, 3);
		        m_Lenth = handleTxFrame(m_Frame);
		        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);
		        SendAT(&AT2,s,">",2);
//				if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x97){
//					OSSemPost(SemHandleTxFrame);	
//					continue;
//				}				
		        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
	    }
		
//		res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 500 , &err ) - 0);
//		if(times++ >= 0){//不需要等待回复
//			break;
//		}
//	}while(res != 0x97);
    LedValue.ConSta  = 3;
    LedValue.ConSpd  = 5;
    LedValue.BlinkTm = 6;
    
#if SRV_DEBUG
    for(i = 0;i<m_Lenth; i++){
        if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
        if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
        if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
//    res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 5000 , &err ) - 0);
//    if(res == 0x97){
//        return 1;
//    }else{
        return 0;
//    }
}

uint16_t send98Frame(TcpType *m_Frame , uint8_t *m_data, uint16_t datLen, uint8_t send_flag)  //发送实时参数
{
    uint16_t m_Lenth,both_len;
    uint16_t i = 0;
    char * s = NULL;
    char q[50];
    s = q;
//	do{
      m_Frame->GuideCode = 0x7E7E;
      m_Frame->ValidDataTx.FuncMode = 0x98;
      getLocalTime(&m_Frame->ValidDataTx.frame98Tx.Time);
      m_Frame->ValidDataTx.frame98Tx.DataLenth = datLen; 
      m_Frame->ValidDataTx.frame98Tx.DataPtr = m_data;
      m_Lenth = handleTxFrame(m_Frame);
      both_len = m_Lenth + g_Upload.ErrDataLen;

      if((both_len <= 800)&&(!send_flag)){
        if(datLen > 12){
          memcpy(&g_Upload.ErrBuffer[g_Upload.ErrDataLen],m_Frame->TxFrameArr,m_Lenth);
          g_Upload.ErrDataLen += m_Lenth;
        }
			OSSemPost(SemHandleTxFrame);	
          return 1;
      }else if(both_len > 26){
          sprintf(s , "AT+CIPSEND=%d\r" , g_Upload.ErrDataLen); 
          SendAT(&AT2,s,">",2);
//			if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x98){
//				continue;
//			}		  
          if(AT2.TxArr)(*AT2.TxArr)(g_Upload.ErrBuffer, g_Upload.ErrDataLen);
    #if SRV_DEBUG
          for(i = 0;i<g_Upload.ErrDataLen; i++){
              if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
              if(dbgPrintf)(*dbgPrintf)("%02X ",g_Upload.ErrBuffer[i]);
              if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
    #endif          
          g_Upload.ErrDataLen = 0;
          LedValue.ConSta  = 3;
          LedValue.ConSpd  = 5;
          LedValue.BlinkTm = 6;
          if(datLen > 12){
              memcpy(&g_Upload.ErrBuffer[g_Upload.ErrDataLen],m_Frame->TxFrameArr,m_Lenth);   
              g_Upload.ErrDataLen += m_Lenth;                                                 
          }
          send_data_len = m_Lenth;
      }  
      return 0;
}

uint16_t send9AFrame(TcpType *m_Frame , uint8_t *m_data, uint16_t datLen, uint8_t send_flag, uint8_t datatype)
{
    uint16_t m_Lenth = 0;
    uint8_t err = 0, res = 0 , Times = 0 ;	
    uint16_t i = 0;
    time_t TimeStamp;
    char * s = NULL;
    char q[50];
    s = q;
#ifdef TEST_FRAME
//	do{
      m_Frame->GuideCode = 0x7E7E;
      m_Frame->ValidDataTx.FuncMode = 0x9A;
      memcpy(&TimeStamp , &m_data[2] , 4);
      getTime(&m_Frame->ValidDataTx.frame9ATx.time , TimeStamp);     
      m_Frame->ValidDataTx.frame9ATx.can1ip = m_data[6];
      m_Frame->ValidDataTx.frame9ATx.can2ip = m_data[7];         
      memcpy(&m_Frame->ValidDataTx.frame9ATx.machine_mac , &m_data[8] , 6);
      memcpy(&m_Frame->ValidDataTx.frame9ATx.pro_num , &m_data[14] , 2);
      memcpy(&m_Frame->ValidDataTx.frame9ATx.m_id , &m_data[16] , 2);      
      m_Frame->ValidDataTx.frame9ATx.data_length = datLen; 
      m_Frame->ValidDataTx.frame9ATx.data_ptr  = &m_data[18];
      m_Lenth = handleTxFrame(m_Frame);          
		sprintf(s,"AT+CIPSEND=%d\r",m_Lenth);
		SendAT(&AT2,s,">",2);
		if(fail_count >= 5){
			OSTimeDlyHMSM(0,0,5,0);
			fail_count = 0;
		}
		if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr,m_Lenth);
		
          LedValue.ConSta  = 3;
          LedValue.ConSpd  = 5;
          LedValue.BlinkTm = 6;
		  
#if SRV_DEBUG
          for(i = 0;i<m_Lenth; i++){
              if(i == 0){
			  	if(dbgPrintf)(*dbgPrintf)("TX---->");
			  }
			  
              if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
              if(i == m_Lenth){
			  	if(dbgPrintf)(*dbgPrintf)("\r\n");
			  }
		  }
#endif
//        	res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 2000 , &err ) - 0);
//			if(Times++ >= 3){
//				break;
//			}

//		}while(res != 0x9A);
//		if(res == 0x9A){
			return 1;
//		}else{
//		    return 0;
//		}
#else
	do{
      m_Frame->GuideCode = 0x7E7E;
      m_Frame->ValidDataTx.FuncMode = 0x9A;
      memcpy(&TimeStamp , &m_data[2] , 4);
      getTime(&m_Frame->ValidDataTx.frame9ATx.time , TimeStamp);     
      m_Frame->ValidDataTx.frame9ATx.can1ip = m_data[6];
      m_Frame->ValidDataTx.frame9ATx.can2ip = m_data[7];         
      memcpy(&m_Frame->ValidDataTx.frame9ATx.machine_mac , &m_data[8] , 6);
      memcpy(&m_Frame->ValidDataTx.frame9ATx.pro_num , &m_data[14] , 2);
      memcpy(&m_Frame->ValidDataTx.frame9ATx.m_id , &m_data[16] , 2);      
      m_Frame->ValidDataTx.frame9ATx.data_length = datLen; 
      m_Frame->ValidDataTx.frame9ATx.data_ptr  = &m_data[18];
      m_Lenth = handleTxFrame(m_Frame);          
		sprintf(s,"AT+CIPSEND=%d\r",m_Lenth);
		SendAT(&AT2,s,">",2);
//		if(fail_count >= 5){
//			OSTimeDlyHMSM(0,0,5,0);
//			fail_count = 0;
//		}
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x9A){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}		
		if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr,m_Lenth);
		
          LedValue.ConSta  = 3;
          LedValue.ConSpd  = 5;
          LedValue.BlinkTm = 6;
		  
#if SRV_DEBUG
          for(i = 0;i<m_Lenth; i++){
              if(i == 0){
			  	if(dbgPrintf)(*dbgPrintf)("TX---->");
			  }
			  
              if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
              if(i == m_Lenth){
			  	if(dbgPrintf)(*dbgPrintf)("\r\n");
			  }
		  }
#endif
        	res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 2000 , &err ) - 0);
			if(Times++ >= 3){
				break;
			}

		}while(res != 0x9A);
		if(res == 0x9A){
			return 1;
		}else{
		    return 0;
		}

	
#endif
}

uint16_t send9BFrame(TcpType *m_Frame , uint8_t *m_data, uint16_t datLen, uint8_t send_flag, uint8_t datatype)
{
	uint16_t m_Lenth = 0;
	uint16_t i = 0;
	time_t TimeStamp;
	char * s = NULL;
	char q[50];
	s = q;
	
	  m_Frame->GuideCode = 0x7E7E;
	  m_Frame->ValidDataTx.FuncMode = 0x9B;
	  memcpy(&TimeStamp , &m_data[2] , 4);
	  getTime(&m_Frame->ValidDataTx.frame9BTx.Time , TimeStamp);           
//	  memcpy(&m_Frame->ValidDataTx.frame9BTx.machine_mac , &m_data[8] , 6);    
	  m_Frame->ValidDataTx.frame9BTx.DataLenth = datLen; 
	  m_Frame->ValidDataTx.frame9BTx.DataPtr  = &m_data[6];
	  m_Lenth = handleTxFrame(m_Frame);

	  sprintf(s,"AT+CIPSEND=%d\r",m_Lenth);
	  SendAT(&AT2,s,">",2);
//	  if(fail_count >= 5){
//		  OSTimeDlyHMSM(0,0,5,0);
//		  fail_count = 0;
//	  }
//	  if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0x9B){
//		  continue;
//	  }
	  if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr,m_Lenth);
 
    #if SRV_DEBUG
          for(i = 0;i<m_Lenth; i++){
              if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
              if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
              if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
    #endif          
       
          LedValue.ConSta  = 3;
          LedValue.ConSpd  = 5;
          LedValue.BlinkTm = 6;
	      return 0;
}


uint16_t sendA2Frame(TcpType *m_Frame)
{
    uint8_t err = 0, res = 0 , Times = 0 ;
    uint16_t m_Lenth = 0;
    uint16_t i = 0;
    char * s =NULL;
    char q[50];
    s = q;
	time_t TimeStamp;
    do{
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidDataTx.FuncMode = 0xA2;
		TimeStamp = getUnixTimeStamp();
		getTime(&m_Frame->ValidDataTx.frameA2Tx.Time , TimeStamp);
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s , "AT+CIPSEND=%d\r" , m_Lenth);

//		if(fail_count >= 5){
//			OSTimeDlyHMSM(0,0,5,0);
//			fail_count = 0;
//		}
//		if(m_Frame->TxFrameArr[FUNCODE_LOCATION] != 0xA2){
//			OSSemPost(SemHandleTxFrame);	
//			continue;
//		}		
        SendAT(&AT2,s,">",2);		
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 5000 , &err ) - 0);
		if(dbgPrintf)(*dbgPrintf)("send A2 to server ",m_Frame->TxFrameArr[i]);
        if(Times++ >= 2){
            break;
        }
    }while(res != 0xA2);
    if(res == 0xA2){
        return 1;
    }else{
        return 0;
    }
}

uint8_t heart_beat(uint8_t trans_mode,uint8_t curSta)
{
    static uint8_t tmp_hours  = 0;
    if(tmp_hours != one_two_flag){
       if(GprsParam.MaitTime && !GprsParam.Pause && g_Upload.DebugOver && (Getf2Heart < MAX_GETF2_TIME)){//连接获取三次
              Getf2Heart++;
			  if(!ConSta.Status){
			  	SendDataToServer(trans_mode,curSta);
			  }
//              if(!connectToServer() && !ConSta.Status){
//                  while(!disConnectToServer());
//                  return curSta;
//              }
              if(!sendF2Frame(&g_TcpType)){
                  while(!disConnectToServer());
                  return curSta;
              }else{
                  Getf2Heart = 0;
                  tmp_hours = one_two_flag;
              }
#ifdef KEEP_TCP_LIVE
#else
              if(ConSta.Status){
                  while(!disConnectToServer());
                  return curSta;
              }
#endif			  
        }else{
            tmp_hours = one_two_flag;
        }
    }else{
        return 0x08;
    }
    return curSta;
}




uint8_t getServerTime(uint8_t curSta)
{
    if(GprsParam.MaitTime && !GprsParam.Pause && g_Upload.DebugOver && !isGetTime && (Getf3Time < MAX_GETF3_TIME)){//连接获取三次
      Getf3Time++;
      if(!connectToServer()){
          while(!disConnectToServer());
          return curSta;
      }
      if(!sendF3Frame(&g_TcpType)){
          
          while(!disConnectToServer());
          return curSta;
      }else{
          isGetTime = 1;
          Getf3Time = 0;
      }
      if(ConSta.Status){
          while(!disConnectToServer());
          return curSta;
      }
    }
    return curSta;
}

void getRandTime(struct tm *pTime)
{
    pTime->tm_hour = rand() % 8;
    pTime->tm_min  = rand() % 60;
    pTime->tm_sec  = rand() % 60;
}

uint8_t isGetupdat = 0;
uint8_t getupdate(uint8_t curSta)
{
    static uint32_t cur_hour = 0, pri_hour = 0;
    cur_hour = f4_time/3600 + 24;
    if((cur_hour - pri_hour) == 24){
        if(GprsParam.MaitTime && !GprsParam.Pause && g_Upload.DebugOver && !isGetupdat){
          if(!connectToServer()){
              while(!disConnectToServer());
              return curSta;
          }
		  
          if(!send97Frame(&g_TcpType, REQ_DTU_CODE, UPGRADE_FEEDBACK)){
              while(!disConnectToServer());
          }else{
              isGetupdat = 1;
          }
        }
        pri_hour = cur_hour;
    }
    isGetupdat = 1;
    return curSta;
}



uint8_t setupdatParam(uint8_t *m_Param , uint16_t m_ParamLen, TypeUpdate *mUpdateParam,updateInfoType *mupdateinfo)
{
/*
*FilePath:http://192.13.182.156:7004/upgrade/STM32.bin+0x00+Packtype:02+0x00+CheckSum:E12D+0x00+01+CRC8
*升级97帧：
*7E 7E 44 39 C4 56 42 74 00 00 0C 29 13 54 EC 00 00 50 97 01 46 69 6C 65 50 61 74 68 3A 68 74 74 70 3A 2F 
*2F 31 39 32 2E 31 33 2E 31 38 322E 31 35 36 3A 37 30 30 34 2F 75 70 67 72 61 64 65 2F 53 54 4D 33 32 2E 
*62 69 6E 00 50 61 63 6B 74 79 70 65 3A 02 00 43 68 65 63 6B 53 756D 3A b9 81 00 01 11
*/
	char *p = NULL, *q = NULL;
	uint8_t temp[6];

	if(!m_Param){
		return 0;
	}
	
	p = strstr((char const *)m_Param,"th:");
	q = strstr((char const *)m_Param,"bin");
	if(p){
		if(q-p > 0){
			memset(mUpdateParam->FilePath,0,sizeof(mUpdateParam->FilePath));
			memcpy(mUpdateParam->FilePath,p+3,q-p);
		}else{
			return 0;
		}
		q = strstr(q+5,"pe:");
		if(q){
			memset(temp,0,sizeof(temp));
			memcpy(temp,q+3,1);
			mupdateinfo->update_pack_type = temp[0];
		}else{
			return 0;
		}
		
		p = strstr(q+6,"Sum");
		if(p){
			memset(temp,0,sizeof(temp));
			memcpy(temp,p+4,2);
			mUpdateParam->recieve_CheckSum =(uint16_t)(((uint8_t)temp[0] << 8) | (uint8_t)temp[1]);
		}else{
			return 0;
		}

		memset(temp,0,sizeof(temp));
		memcpy(temp,p+7,1);
		mUpdateParam->update_after_reconnect = temp[0];
		return 1;

	}else{
		return 0;
	}

}


/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
