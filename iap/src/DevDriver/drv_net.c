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
TcpType  g_TcpType;
uint8_t  g_CacheData[200];
OS_EVENT *MSemTCP;
struct tm nextDay;

TypeGPRS GprsParam = 
{
	.Version = 33,
	.SerPort = 7000,
	.SerName = "192.13.182.13",
//	.SerPort = 7200,
//	.SerName = "192.13.182.156",
	.HeartTime = 10,
	.OutErrTime = 30,
	.Out_Err_After_Time = 5,
	.HealtTime = 30,
	.ButtTime = 240,
	.InerrtTime = 5,
	.In_Err_After_Time = 2,
	.MaitTime = 315360000,
	.ApnAccr = "GREEAC.GD",
	.ApnUsr = "",
	.ApnPwd	= "",
	.SmsPwd = "123456",
	.Admin = {"10690370480","10698000065","18676489376","18676489371","18926932781"},
	.Usron = {"07566990221 ","18676489373","18676489376","",""},
	.IMEI = "861024020187777",
	.ICCID = "8986061509000050609N",
};
TypeFTP FtpParam = 
{
  /*FtpAdrr*/
  {"192.13.182.156"},
  /*Username*/
  {"390663"},
  /*PassWord*/
  {"qwe!234"},
  /*FileName*/
  {"12a.bin"},
  /*FilePath*/
  {"/"},
};
/*******************************************************
* Function Name:    handleRxFrame
* Purpose:          解析一�?E 7E类型的数据帧
* Params :          *m_frame�?  处理后的数据放的位置
*                   *m_dataarr�?需要处理的原始数据地址
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
        //memset(&m_frame->TxFrameArr[0] , 0 , sizeof(TcpType));
        memcpy(&m_frame->RxFrameArr[0] , m_dataarr , m_datalenth);

        m_frame->GuideCode = (uint16_t)(m_frame->RxFrameArr[0]<<8)+(uint16_t)m_frame->RxFrameArr[1];
        if(m_frame->GuideCode == 0x7E7E){
            m_datalenth = (uint16_t)(m_frame->RxFrameArr[16] << 8)+(uint16_t)(m_frame->RxFrameArr[17]);
            if(m_datalenth < 1490){
                m_frame->CrcValue = CalcCheckSum(&m_frame->RxFrameArr[2], m_datalenth + 16);
            }else{
                return NULL;
            }
            //m_frame->CrcValue = CalcCheckSum(&m_frame->RxFrameArr[2], m_datalenth - 3);
            //if(m_frame->CrcValue == m_frame->RxFrameArr[m_datalenth - 1]){
            if(m_frame->CrcValue == m_frame->RxFrameArr[m_datalenth + 18]){
                m_frame->DataPtr  = &m_frame->RxFrameArr[2];

                //m_frame->SrcAddr = (uint8_t *)g_SrcAddr;
                //memcpy(m_frame->SrcAddr , m_frame->DataPtr , sizeof(m_frame->SrcAddr));
                m_frame->DataPtr += sizeof(m_frame->SrcAddr);

                //m_frame->DstAddr = (uint8_t *)g_DstAddr;
                memcpy(m_frame->DstAddr , m_frame->DataPtr , sizeof(m_frame->DstAddr));
                m_frame->DataPtr += sizeof(m_frame->DstAddr);

                m_frame->DataLenth = (uint16_t)(*m_frame->DataPtr << 8)+(uint16_t)(*(m_frame->DataPtr + 1));   //高位在前，低位在�?
                //memcpy(&m_frame->DataLenth , m_frame->DataPtr , sizeof(m_frame->DataLenth));
                m_frame->DataPtr += sizeof(m_frame->DataLenth);

                m_frame->ValidData.FuncMode = *m_frame->DataPtr;
                switch(*m_frame->DataPtr)
                {
                    case 0x72:
                        memcpy(&m_frame->ValidData.frame72Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame72Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame72Rx.FuncMode);
                        break;
                        
                    case 0x88:
                        memcpy(&m_frame->ValidData.frame88Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame88Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame88Rx.FuncMode);
                        break;               
                        
                    case 0x89:
                        memcpy(&m_frame->ValidData.frame89Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame89Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame89Rx.FuncMode);
                        memcpy(&m_frame->ValidData.frame89Rx.CheckCode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame89Rx.CheckCode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame89Rx.CheckCode);
                        break;
                        
                    case 0x90:
                        memcpy(&m_frame->ValidData.frame90Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame90Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame90Rx.FuncMode);
                        memcpy(&m_frame->ValidData.frame90Rx.CheckCode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame90Rx.CheckCode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame90Rx.CheckCode);
                        break;
                        
                    case 0x91:
                        memcpy(&m_frame->ValidData.frame91Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame91Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame91Rx.FuncMode);
                        memcpy(&m_frame->ValidData.frame91Rx.Result , m_frame->DataPtr , sizeof(m_frame->ValidData.frame91Rx.Result));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame91Rx.Result);
                        break;
                        
                    case 0x92:
                        memcpy(&m_frame->ValidData.frame92Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame92Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame92Rx.FuncMode);
                        memcpy(&m_frame->ValidData.frame92Rx.StopMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame92Rx.StopMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame92Rx.StopMode);    
                        break;    
                        
                    case 0x93:
                        memcpy(&m_frame->ValidData.frame93Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame93Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame93Rx.FuncMode);
                        memcpy(&m_frame->ValidData.frame93Rx.Flag , m_frame->DataPtr , sizeof(m_frame->ValidData.frame93Rx.Flag));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame93Rx.Flag);    
                        break; 
                        
                    case 0x94:
                        memcpy(&m_frame->ValidData.frame94Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame94Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame94Rx.FuncMode);
                        m_frame->ValidData.frame94Rx.Param = m_frame->DataPtr;
                        //memcpy(m_frame->ValidData.frame94Rx.Param , m_frame->DataPtr , m_frame->DataLenth - 1);
                        m_frame->DataPtr += m_frame->DataLenth - 1;    
                        break;                       
                        
                    case 0x95:
                        memcpy(&m_frame->ValidData.frame95Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame95Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame95Rx.FuncMode);
                        m_frame->ValidData.frame94Rx.Param = m_frame->DataPtr;
                        //memcpy(m_frame->ValidData.frame95Rx.Param , m_frame->DataPtr , m_frame->DataLenth - 1);
                        m_frame->DataPtr += m_frame->DataLenth - 1;    
                        break; 
                        
                    case 0x96:
                        memcpy(&m_frame->ValidData.frame96Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame96Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame96Rx.FuncMode); 
                        memcpy(&m_frame->ValidData.frame96Rx.Res , m_frame->DataPtr , sizeof(m_frame->ValidData.frame96Rx.Res));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame96Rx.Res); 
                        break; 
                        
                    case 0x97:
                        memcpy(&m_frame->ValidData.frame97Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frame97Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame97Rx.FuncMode);
                        memcpy(&m_frame->ValidData.frame97Rx.Update , m_frame->DataPtr , sizeof(m_frame->ValidData.frame97Rx.Update));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frame97Rx.Update);     
                        break; 
                        
                    case 0xF3:
                        memcpy(&m_frame->ValidData.frameF3Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frameF3Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frameF3Rx.FuncMode);
                        memset(&m_frame->ValidData.frameF3Rx.Time , 0 , sizeof(struct tm));
                        m_frame->ValidData.frameF3Rx.Time.tm_year = *m_frame->DataPtr++ ;
                        m_frame->ValidData.frameF3Rx.Time.tm_mon  = *m_frame->DataPtr++ ;
                        m_frame->ValidData.frameF3Rx.Time.tm_mday = *m_frame->DataPtr++ ;
                        m_frame->ValidData.frameF3Rx.Time.tm_hour = *m_frame->DataPtr++ ;
                        m_frame->ValidData.frameF3Rx.Time.tm_min  = *m_frame->DataPtr++ ;
                        m_frame->ValidData.frameF3Rx.Time.tm_sec  = *m_frame->DataPtr++ ;
                        
                        break;          
                        
                    case 0xF4:
                        memcpy(&m_frame->ValidData.frameF4Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frameF4Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frameF4Rx.FuncMode);
                        memcpy(&m_frame->ValidData.frameF4Rx.Resault , m_frame->DataPtr , sizeof(m_frame->ValidData.frameF4Rx.Resault));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frameF4Rx.Resault);     
                        break; 
                        
                    case 0xA0:
                        memcpy(&m_frame->ValidData.frameA0Rx.FuncMode , m_frame->DataPtr , sizeof(m_frame->ValidData.frameA0Rx.FuncMode));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frameA0Rx.FuncMode);
                        m_frame->ValidData.frameA0Rx.Time.tm_year = *m_frame->DataPtr++ ;
                        m_frame->ValidData.frameA0Rx.Time.tm_mon  = *m_frame->DataPtr++ ;
                        m_frame->ValidData.frameA0Rx.Time.tm_mday = *m_frame->DataPtr++ ;
                        m_frame->ValidData.frameA0Rx.Time.tm_hour = *m_frame->DataPtr++ ;
                        m_frame->ValidData.frameA0Rx.Time.tm_min  = *m_frame->DataPtr++ ;
                        m_frame->ValidData.frameA0Rx.Time.tm_sec  = *m_frame->DataPtr++ ;
                        memcpy(&m_frame->ValidData.frameA0Rx.StartAddr , m_frame->DataPtr , sizeof(m_frame->ValidData.frameA0Rx.StartAddr));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frameA0Rx.StartAddr);
                        memcpy(&m_frame->ValidData.frameA0Rx.FinishAddr , m_frame->DataPtr , sizeof(m_frame->ValidData.frameA0Rx.FinishAddr));
                        m_frame->DataPtr += sizeof(m_frame->ValidData.frameA0Rx.FinishAddr);
                        
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


/*******************************************************
* Function Name:    handleTxFrame
* Purpose:          将结构体TcpType *m_frame填写完整后，调用该函数将组成相应的数据帧到TXBUF
* Params :          *m_frame：需要处理的数据地址
* Return:           uint16_t：返回组成发送的数据帧长�?
* Limitation: 	    注意检查和补充功能码类型及处理方式
*******************************************************/
uint16_t handleTxFrame(TcpType *m_frame)
{
    uint16_t i = 0;
    uint16_t frameA0_Lenth = 0;
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
    m_frame->DataPtr += sizeof(m_frame->DataLenth);

    switch(m_frame->ValidData.FuncMode)
    {
        case 0x72:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            break;
            
        case 0x88:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            break;
            
        case 0x89:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frame89Tx.CheckCode  , 2);
            m_frame->DataPtr += 2;
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frame89Tx.HardVesion , 2);
            m_frame->DataPtr += 2;
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frame89Tx.SoftVesion , 2);
            m_frame->DataPtr += 2;
            i = strlen((const char *)m_frame->ValidData.frame89Tx.IMEI) + 1;
            memcpy(m_frame->DataPtr , m_frame->ValidData.frame89Tx.IMEI , i);
            m_frame->DataPtr += i;
            i = strlen((const char *)m_frame->ValidData.frame89Tx.BaseStation) + 1;
            memcpy(m_frame->DataPtr , m_frame->ValidData.frame89Tx.BaseStation , i);
            m_frame->DataPtr += i;
            i = strlen((const char *)m_frame->ValidData.frame89Tx.ICCID) + 1;
            memcpy(m_frame->DataPtr , m_frame->ValidData.frame89Tx.ICCID , i);
            m_frame->DataPtr += i;
            break;      
            
        case 0x90:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frame90Tx.CheckCode , 2);
            m_frame->DataPtr += 2;
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frame90Tx.SoftVesion , 2);
            m_frame->DataPtr += 2;
            i = strlen((const char *)m_frame->ValidData.frame90Tx.IMEI) + 1;
            memcpy(m_frame->DataPtr , m_frame->ValidData.frame90Tx.IMEI , i);
            m_frame->DataPtr += i;
            i = strlen((const char *)m_frame->ValidData.frame90Tx.BaseStation) + 1;
            memcpy(m_frame->DataPtr , m_frame->ValidData.frame90Tx.BaseStation , i);
            m_frame->DataPtr += i;
            i = strlen((const char *)m_frame->ValidData.frame90Tx.ICCID) + 1;
            memcpy(m_frame->DataPtr , m_frame->ValidData.frame90Tx.ICCID , i);
            m_frame->DataPtr += i;
            break;

        case 0x91:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frame91Tx.TranMode , 1);
            m_frame->DataPtr += 1;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame91Tx.Time.tm_year;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame91Tx.Time.tm_mon;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame91Tx.Time.tm_mday;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame91Tx.Time.tm_hour;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame91Tx.Time.tm_min;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame91Tx.Time.tm_sec;
            break;
            
        case 0x92:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frame92Tx.Result , sizeof(m_frame->ValidData.frame92Tx.Result));
            m_frame->DataPtr += sizeof(m_frame->ValidData.frame92Tx.Result);
            break;

        case 0x93:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frame93Tx.Result , sizeof(m_frame->ValidData.frame93Tx.Result));
            m_frame->DataPtr += sizeof(m_frame->ValidData.frame93Tx.Result);
            break;
            
        case 0x94:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            memcpy(m_frame->DataPtr , m_frame->ValidData.frame94Tx.FailParam , m_frame->ValidData.frame94Tx.ParamLen);
            m_frame->DataPtr += m_frame->ValidData.frame94Tx.ParamLen;
            break;        
            
        case 0x95:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            memcpy(m_frame->DataPtr , m_frame->ValidData.frame95Tx.Param , m_frame->ValidData.frame95Tx.ParamLen);
            m_frame->DataPtr += m_frame->ValidData.frame95Tx.ParamLen;
            break; 
            
        case 0x96:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            *m_frame->DataPtr ++ = m_frame->ValidData.frame96Tx.Time.tm_year ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame96Tx.Time.tm_mon ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame96Tx.Time.tm_mday ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame96Tx.Time.tm_hour ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame96Tx.Time.tm_min ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame96Tx.Time.tm_sec ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame96Tx.TranMode ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frame96Tx.MachMode ;
            memcpy(m_frame->DataPtr , m_frame->ValidData.frame96Tx.Data , m_frame->ValidData.frame96Tx.DataLen);
            m_frame->DataPtr += m_frame->ValidData.frame96Tx.DataLen;
            break; 
            
        case 0x97:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frame97Tx.Result , sizeof(m_frame->ValidData.frame97Tx.Result));
            m_frame->DataPtr += sizeof(m_frame->ValidData.frame97Tx.Result);
            break; 
            
        case 0xEE:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frameEETx.ErrCode , sizeof(m_frame->ValidData.frameEETx.ErrCode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.frameEETx.ErrCode);
//            memcpy(m_frame->DataPtr , &m_frame->ValidData.frameEETx.TranMode , sizeof(m_frame->ValidData.frameEETx.TranMode));
//            m_frame->DataPtr += sizeof(m_frame->ValidData.frameEETx.TranMode);
            break;  
            
        case 0xF3:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            break;
            
        case 0xF4:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            *m_frame->DataPtr ++ = m_frame->ValidData.frameF4Tx.Time.tm_year ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frameF4Tx.Time.tm_mon  ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frameF4Tx.Time.tm_mday ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frameF4Tx.Time.tm_hour ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frameF4Tx.Time.tm_min  ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frameF4Tx.Time.tm_sec  ; 
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frameF4Tx.Times  , 2);
            m_frame->DataPtr += 2;
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frameF4Tx.StaAddr  , 2);
            m_frame->DataPtr += 2;
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frameF4Tx.Signal   , 1);
            m_frame->DataPtr += 1;
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frameF4Tx.DataType , 1);
            m_frame->DataPtr += 1;
            memcpy(m_frame->DataPtr , m_frame->ValidData.frameF4Tx.DataPtr , m_frame->ValidData.frameF4Tx.DataLenth);
            m_frame->DataPtr += m_frame->ValidData.frameF4Tx.DataLenth;
            break;
            
        case 0xA0:
            memcpy(m_frame->DataPtr , &m_frame->ValidData.FuncMode , sizeof(m_frame->ValidData.FuncMode));
            m_frame->DataPtr += sizeof(m_frame->ValidData.FuncMode);
            *m_frame->DataPtr ++ = m_frame->ValidData.frameA0Tx.Time.tm_year ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frameA0Tx.Time.tm_mon  ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frameA0Tx.Time.tm_mday ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frameA0Tx.Time.tm_hour ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frameA0Tx.Time.tm_min  ;
            *m_frame->DataPtr ++ = m_frame->ValidData.frameA0Tx.Time.tm_sec  ; 
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frameA0Tx.StartAddr , 2);
            m_frame->DataPtr += 2;
            memcpy(m_frame->DataPtr , &m_frame->ValidData.frameA0Tx.FinishAddr, 2);
            m_frame->DataPtr += 2;
            frameA0_Lenth = (m_frame->ValidData.frameA0Tx.FinishAddr-m_frame->ValidData.frameA0Tx.StartAddr+1);
            if(frameA0_Lenth > 1000){
                frameA0_Lenth = 1000;
            }
            memcpy(m_frame->DataPtr , m_frame->ValidData.frameA0Tx.data, frameA0_Lenth);
            m_frame->DataPtr += frameA0_Lenth;
        break;
        
        default:
            break;
    }

    m_frame->DataLenth = (m_frame->DataPtr - &(m_frame->TxFrameArr[18]));
    m_frame->TxFrameArr[16] = (uint8_t)(m_frame->DataLenth>>8); //高位在前，低位在�?
    m_frame->TxFrameArr[17] = (uint8_t)m_frame->DataLenth;      //高位在前，低位在�?
    //memcpy(m_frame->TxFrameArr[16] , &m_frame->DataLenth , sizeof(m_frame->DataLenth));

    m_frame->CrcValue = CalcCheckSum(&m_frame->TxFrameArr[2], (uint32_t)(m_frame->DataPtr - &m_frame->TxFrameArr[2]));
    memcpy(m_frame->DataPtr , &m_frame->CrcValue , sizeof(m_frame->CrcValue));

    return m_frame->DataLenth + 19;
}

uint8_t to_s_idle;
void tcpDataProcess(TcpType *m_TcpType)
{
    uint8_t err = 0;
    char s[50];
    uint16_t i = 0;
    uint16_t m_Lenth = 0;

    uint8_t * p = (uint8_t *)&ACParam;
    CPU_SR         cpu_sr;
    uint32_t t = 10000;
//    uint32_t k = 8000;
//    uint8_t buffer[10]={1,2,3,4,5,6,7,8,9,0};
    switch (m_TcpType->ValidData.FuncMode)
    {          
        case 0x72:
            /* 注意：先回复数据帧，再复位！�?*/ //20160411 zhh
            m_TcpType->ValidData.frame72Tx.FuncMode = 0x72; 
            memset(g_CacheData , 0 , sizeof(g_CacheData)); 
            m_Lenth = handleTxFrame(&g_TcpType);
            sprintf(s , "AT+CIPSEND=%d\r\n" , m_Lenth);
            SendAT(&AT2,s,">",2);
            if(AT2.TxArr)(*AT2.TxArr)(g_TcpType.TxFrameArr , m_Lenth);
            OSTimeDlyHMSM(0,0,2,0);
            if(var.onlineTest != 1){
                softReset();  
            }
            OSMboxPost(MSemTCP,(void *)0x72);
 
            break;
        
        case 0x88:
            OSMboxPost(MSemTCP,(void *)0x88);
            /* 主动发送帧 */
            return; 
            
        case 0x89:
            OSMboxPost(MSemTCP,(void *)0x89);
            /* 主动发送帧 */
            return;
            
        case 0x90:
            OSMboxPost(MSemTCP,(void *)0x90);
            /* 主动发送帧 */
            return;
        
        case 0x91:
            /* 主动发送帧 */
            OSMboxPost(MSemTCP,(void *)0x91);
            return;
        
        case 0x92:
            OSMboxPost(MSemTCP,(void *)0x92);
            m_TcpType->ValidData.frame92Tx.FuncMode = 0x92;
            switch(m_TcpType->ValidData.frame92Rx.StopMode)
            {
                case 0x00:
                    /* 该处判断当前传输模式，并对停止数据发送进行处�?*/
                    break;
                
                case 0x01:               
                    /* 该处放强制停止数据发送程�?*/

                    m_TcpType->ValidData.frame92Tx.Result = 0;
                    if(var.realMonitor == 1){
                    var.realMonitor = 0;
                    to_s_idle = 1;
                    }
                    break;
                 
                case 0x02:
                    /* Do Nothing */
                    break;
                
                default: 
                    return ;
            }
            OSMutexPend(MutexFlash , 0 , &err);
            sFLASH_EraseSubSector(VAR_START_ADDR);
            sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
            OSMutexPost(MutexFlash);
            break;
        
        case 0x93:
            OSMboxPost(MSemTCP,(void *)0x93);
            m_TcpType->ValidData.frame93Tx.FuncMode = 0x93;
            switch(m_TcpType->ValidData.frame93Rx.Flag)
            {
                case 0x00:
                        switch(NS)//s_idle, s_debug, s_button, s_outdoor , s_indoor , s_real , s_test
                        {
                            case s_outdoor:
                            case s_indoor:
                                m_TcpType->ValidData.frame93Tx.Result = 1;break;
                            case s_debug:
                                m_TcpType->ValidData.frame93Tx.Result = 2;break;
                            case s_button:
                                m_TcpType->ValidData.frame93Tx.Result = 3;break;
                            //case 亚健康上�?
                            //    m_TcpType->ValidData.frame93Tx.Result = 4;break;
                            //case 短信激活GPRS登录失败:
                            //    m_TcpType->ValidData.frame93Tx.Result = 5;break;
                            //case 机组关机状�?
                            //    m_TcpType->ValidData.frame93Tx.Result = 6;break;
                            default:break;
                        }
                    m_TcpType->ValidData.frame93Tx.Result = 0;
                    break;
                
                case 0x01:
                    
                    /* 该处放强制转换为实时监控模式 */
                    var.realMonitor = 1;
                    m_TcpType->ValidData.frame93Tx.Result = 0;
                    break;
                        
                default: 
                    return ;
            }
            OSMutexPend(MutexFlash , 0 , &err);
            sFLASH_EraseSubSector(VAR_START_ADDR);
            sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
            OSMutexPost(MutexFlash);
            break;
        
        case 0x94:
            OSMboxPost(MSemTCP,(void *)0x94);
            m_TcpType->ValidData.frame94Tx.FuncMode = 0x94;
            m_TcpType->ValidData.frame94Tx.ParamLen = \
            setGprsParam(m_TcpType->ValidData.frame94Rx.Param , 
                         m_TcpType->DataLenth - 1 ,
                         &GprsParam);
            OSMutexPend(MutexFlash , 0 , &err);
            sFLASH_EraseSubSector(GPRS_INFO_START_ADDR);
            sFLASH_WriteBuffer((uint8_t *)&GprsParam , GPRS_INFO_START_ADDR , sizeof(GprsParam));
            OSMutexPost(MutexFlash);
            break;
            
        case 0x95:
            OSMboxPost(MSemTCP,(void *)0x95);
            m_TcpType->ValidData.frame95Tx.FuncMode = 0x95;
            m_TcpType->ValidData.frame95Tx.Param = g_CacheData;
            m_TcpType->ValidData.frame95Tx.ParamLen = \
            getGprsParam(m_TcpType->ValidData.frame95Rx.Param , 
                         m_TcpType->DataLenth - 1 ,
                         m_TcpType->ValidData.frame95Tx.Param,
                         &GprsParam);
            break;
            
        case 0x96:
            OSMboxPost(MSemTCP,(void *)0x96);
            /* 主动发送帧 */
            return;
        
        case 0x97:
            OSMboxPost(MSemTCP,(void *)0x97);
            m_TcpType->ValidData.frame97Tx.FuncMode = 0x97;
            /* 升级没做，先不管 */
            m_TcpType->ValidData.frame97Tx.Result = 0;
            break;
        
        case 0xEE:
            OSMboxPost(MSemTCP,(void *)0xEE);
            /* 主动发送帧 */
            return;
        
        case 0xF3:
            setACTime(&m_TcpType->ValidData.frameF3Rx.Time);
            setLocalTime(&m_TcpType->ValidData.frameF3Rx.Time);
            OSMboxPost(MSemTCP,(void *)0xF3);
            /* 主动发送帧 */
            return;
            
        case 0xF4:
            OSMboxPost(MSemTCP,(void *)0xF4);
            /* 主动发送帧 */
            return;
            
        case 0xA0:
        m_TcpType->GuideCode = 0x7E7E;
        m_TcpType->ValidData.FuncMode = 0xA0;
        getLocalTime(&m_TcpType->ValidData.frameA0Tx.Time);
//        m_Frame->ValidData.frameA0Tx.Times = Times;
        m_TcpType->ValidData.frameA0Tx.StartAddr = m_TcpType->ValidData.frameA0Rx.StartAddr;
        m_TcpType->ValidData.frameA0Tx.FinishAddr = m_TcpType->ValidData.frameA0Rx.FinishAddr;
        if(m_TcpType->ValidData.frameA0Rx.FinishAddr >= m_TcpType->ValidData.frameA0Rx.StartAddr){
            if((m_TcpType->ValidData.frameA0Rx.FinishAddr-m_TcpType->ValidData.frameA0Rx.StartAddr) < 1000) 
            {
//                memset(m_TcpType->ValidData.frameA0Tx.data ,0,sizeof(m_TcpType->ValidData.frameA0Tx.data));
                memcpy(m_TcpType->ValidData.frameA0Tx.data , &p[m_TcpType->ValidData.frameA0Rx.StartAddr], 
                       m_TcpType->ValidData.frameA0Rx.FinishAddr - m_TcpType->ValidData.frameA0Rx.StartAddr +1);
            }else{
                memcpy(m_TcpType->ValidData.frameA0Tx.data,&p[m_TcpType->ValidData.frameA0Rx.StartAddr], 1000);
            }
        }
        else 
            return;
//        m_Lenth = handleTxFrame(m_TcpType);
//        sprintf(s , "AT+CIPSEND=%d\r\n" , m_Lenth);
//        SendAT(&AT2,s,">",2);
//        if(AT2.TxArr)(*AT2.TxArr)(m_TcpType->TxFrameArr , m_Lenth);
        LedValue.ConSta  = 3;
        LedValue.ConSpd  = 5;
        LedValue.BlinkTm = 6;
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_TcpType->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        
            OSMboxPost(MSemTCP,(void *)0xA0);
            /* 主动发送帧 */
//        return;
        break;
        
        default:
            return;
    }


    CPU_CRITICAL_ENTER();   /* Tell uC/OS-II that we are starting an ISR            */  
    memset(g_CacheData , 0 , sizeof(g_CacheData));
    m_Lenth = handleTxFrame(m_TcpType); 
    sprintf(s , "AT+CIPSEND=%d\r\n" , m_Lenth);
    SendAT(&AT2,s,">",2);

    while(t--);
    if(AT2.TxArr)(*AT2.TxArr)(m_TcpType->TxFrameArr , m_Lenth);
//    SendAT(&AT2,"nononononononononononononono","DATA ACCEPT",2);
//    while(k--);
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
        /* 暂无此参�?*/
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
        /* 暂无此参�?*/
    }
    p = strstr((char const *)m_Param , "TUM");
    if(p){
        /* 暂无此参�?*/
    }
    p = strstr((char const *)m_Param , "UDPPT");
    if(p){
        /* 暂无此参�?*/
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
        /* 暂无此参�?*/
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
        /* 暂无此参�?*/
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
        /* 暂无此参�?*/
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
        /* 暂无此参�?*/
    }
    p = strstr((char const *)m_Param , "TUM");
    if(p){
        /* 暂无此参�?*/
    }
    p = strstr((char const *)m_Param , "UDPPT");
    if(p){
        /* 暂无此参�?*/
    }
    p = strstr((char const *)m_Param , "ERRT");
    if(p){
        q += sprintf(q , "ERRT:%d" , m_GprsParam->OutErrTime);
        *q++ = '|';
    }
    p = strstr((char const *)m_Param , "DEBT");
    if(p){
        /* 暂无此参�?*/
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
        /* 暂无此参�?*/
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
        m_Frame->ValidData.FuncMode = 0x89;
        m_Frame->ValidData.frame89Tx.BaseStation = (uint8_t *)GprsParam.CellInfo;
        m_Frame->ValidData.frame89Tx.CheckCode = Mac[0]*3+Mac[1]*6+Mac[2]*0+Mac[3]*9+Mac[4]*7+Mac[5]*4+Mac[6]*10;
        m_Frame->ValidData.frame89Tx.ICCID = (uint8_t *)GprsParam.ICCID;
        m_Frame->ValidData.frame89Tx.IMEI  = (uint8_t *)GprsParam.IMEI;
        m_Frame->ValidData.frame89Tx.HardVesion = 0x20;
        m_Frame->ValidData.frame89Tx.SoftVesion = 0x14;
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s , "AT+CIPSEND=%d\r\n" , m_Lenth);
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
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 2000 , &err ) - 0);
        if(Times++ >= 3){
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
        m_Frame->ValidData.FuncMode = 0xF3;
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s , "AT+CIPSEND=%d\r\n" , m_Lenth);
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
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 2000 , &err ) - 0);
        if(Times++ >= 3){
            break;
        }
    }while(res != 0xF3);
    if(res == 0xF3){
        return 1;
    }else{
        return 0;
    }
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
        m_Frame->ValidData.FuncMode = 0x91;
        getLocalTime(&m_Frame->ValidData.frame91Tx.Time);
        m_Frame->ValidData.frame91Tx.TranMode = TranMode;
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s , "AT+CIPSEND=%d\r\n" , m_Lenth);
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
        res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 2000 , &err ) - 0);
        if(Times++ >= 3){
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
        uint16_t i = 0;
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidData.FuncMode = 0xEE;
        m_Frame->ValidData.frameEETx.ErrCode = ErrCode;
        m_Lenth = handleTxFrame(m_Frame);
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
}


uint16_t send96Frame(TcpType *m_Frame , uint8_t *dat , uint16_t datLen , uint8_t t_mode ,uint8_t m_mode)
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
        m_Frame->ValidData.FuncMode = 0x96;
//        if((t_mode == 0x00)||(t_mode == 0x05)){
            memcpy(&TimeStamp , &dat[2] , 4);
            getTime(&m_Frame->ValidData.frame96Tx.Time , TimeStamp);
            m_Frame->ValidData.frame96Tx.Data    = &dat[6];
//        }else{
//            getLocalTime(&m_Frame->ValidData.frame96Tx.Time);
//            m_Frame->ValidData.frame96Tx.Data    = dat;
//        }
//        getLocalTime(&m_Frame->ValidData.frame96Tx.Time);
        m_Frame->ValidData.frame96Tx.TranMode= t_mode;
        m_Frame->ValidData.frame96Tx.MachMode= m_mode;
        m_Frame->ValidData.frame96Tx.DataLen = datLen;
//        m_Frame->ValidData.frame96Tx.Data    = dat;
        m_Lenth = handleTxFrame(m_Frame);  
        sprintf(s , "AT+CIPSEND=%d\r\n" , m_Lenth);
        SendAT(&AT2,s,">",2);
        if(fail_count >= 5){
        OSTimeDlyHMSM(0,0,5,0);
        fail_count = 0;}
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
#if SRV_DEBUG
        for(i = 0;i<m_Lenth; i++){
            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
#endif
        if(m_mode == 1){
            res = (uint8_t)((uint32_t)OSMboxPend( MSemTCP , 5000 , &err ) - 0);
        }
        if(count ++ >= 2){
            break;}
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
    }while((res != 0x96)&&(m_mode == 1));
    if((var.Hyaline == 0)){
        //if(res == 0x96){
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;//}
    }
    else if(var.Hyaline == 1)
    {
        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
    }
    return m_Lenth;
#if 0        
        uint16_t m_Lenth=0;
        m_Frame->GuideCode = 0x7E7E;
        m_Frame->ValidData.FuncMode = 0x96;
        getLocalTime(&m_Frame->ValidData.frame96Tx.Time);
        m_Frame->ValidData.frame96Tx.TranMode= t_mode;
        m_Frame->ValidData.frame96Tx.MachMode= m_mode;
        m_Frame->ValidData.frame96Tx.DataLen = datLen;
        m_Frame->ValidData.frame96Tx.Data    = dat;
        m_Lenth = handleTxFrame(m_Frame);
        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);

        LedValue.ConSta = 3;
        LedValue.ConSpd = 5;
        LedValue.BlinkTm = 6;
        
        return m_Lenth;
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
        m_Frame->ValidData.FuncMode = 0xF4;
//        getLocalTime(&m_Frame->ValidData.frameF4Tx.Time);
        m_Frame->ValidData.frameF4Tx.Times = Times;
        m_Frame->ValidData.frameF4Tx.StaAddr = start_addr;
        m_Frame->ValidData.frameF4Tx.Signal = LedValue.Signal;
        m_Frame->ValidData.frameF4Tx.DataType = data_type; 
        m_Frame->ValidData.frameF4Tx.DataLenth = datLen; 
        m_Frame->ValidData.frameF4Tx.DataPtr  = dat + start_addr;
        m_Lenth = handleTxFrame(m_Frame);
        sprintf(s , "AT+CIPSEND=%d\r\n" , m_Lenth);
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
        if(count ++ >= 3){
            break;
        }
    }while(res != 0xF4);
    if(res == 0xF4){
        return 1;
    }else{
        return 0;
    }
}

//uint16_t sendA0Frame(TcpType *m_Frame ,uint8_t *dat )
//{
//    uint16_t m_Lenth;
//    uint16_t i = 0;
//    uint8_t err = 0, res = 0 , count = 0 ;
//    char * s = NULL;
//    char q[50];
//    s = q;
//
//        m_Frame->GuideCode = 0x7E7E;
//        m_Frame->ValidData.FuncMode = 0xA0;
//        getLocalTime(&m_Frame->ValidData.frameF4Tx.Time);
////        m_Frame->ValidData.frameA0Tx.Times = Times;
//        m_Frame->ValidData.frameA0Tx.StartAddr = m_Frame->ValidData.frameA0Rx.StartAddr;
//        m_Frame->ValidData.frameA0Tx.FinishAddr = m_Frame->ValidData.frameA0Rx.FinishAddr;
//        memcpy(m_Frame->ValidData.frameA0Tx.data,
//        m_Lenth = handleTxFrame(m_Frame);
//        sprintf(s , "AT+CIPSEND=%d\r\n" , m_Lenth);
//        SendAT(&AT2,s,">",2);
//        if(AT2.TxArr)(*AT2.TxArr)(m_Frame->TxFrameArr , m_Lenth);
//        LedValue.ConSta  = 3;
//        LedValue.ConSpd  = 5;
//        LedValue.BlinkTm = 6;
//#if SRV_DEBUG
//        for(i = 0;i<m_Lenth; i++){
//            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("TX---->");}
//            if(dbgPrintf)(*dbgPrintf)("%02X ",m_Frame->TxFrameArr[i]);
//            if(i == m_Lenth){ if(dbgPrintf)(*dbgPrintf)("\r\n");}}
//#endif
//
//}
uint8_t isGetTime = 0;
uint8_t getServerTime(uint8_t curSta)
{

    if(GprsParam.MaitTime && !GprsParam.Pause && g_Upload.DebugOver && !isGetTime){
      if(!connectToServer()){
          while(!disConnectToServer());
          return curSta;
      }
      if(!sendF3Frame(&g_TcpType)){
          while(!disConnectToServer());
          return curSta;
      }else{
          isGetTime = 1;
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


/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/