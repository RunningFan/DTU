#include "drv_button.h"
#include "drv_led.h"
#include "drv_rtc.h"
#include "drv_usart.h"
#include "drv_timer.h"
#include "drv_can.h"
#include "drv_flash.h"
#include "drv_gprs.h"
#include "drv_net.h"
#include "Duplicate.h"
#include "common.h"
#include "bsp.h"
#include "statistic.h"
ConType ConSta;
ATSendType AT1 = { uart1SendArry, uart1SendStr, NULL, 0 };
//	AT1(USART1) ---->   UART2(SIM800 三线串口)
ATSendType AT2 = { uart3SendArry, uart3SendStr, NULL, 0 };
//	AT2(USART3) ---->   UART1(SIM800 全功能串口)

const uint8_t gc_DowCrcTable[256] =
{
    // Maxim 1-Wire CRC of all bytes passed to it.The result accumulates in the global variable CRC.
    0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
    157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
    35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
    190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
    70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
    219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
    101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
    248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
    87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
    233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
    116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};


/*********************************************************************************/


/*******************************************************
* Function Name: 	chr2hex
* Purpose: 		    将1个字符转换为16进制数字
* Params :          chr:字符,0~9/A~F/a~F
* Return: 		    chr对应的16进制数值
* Limitation: 	    无
*******************************************************/
uint8_t chr2hex(uint8_t chr)
{
    if(chr>='0'&&chr<='9')return chr-'0';
    if(chr>='A'&&chr<='F')return (chr-'A'+10);
    if(chr>='a'&&chr<='f')return (chr-'a'+10);
    return 0;
}


/*******************************************************
* Function Name: 	hex2chr
* Purpose: 		    将1个16进制数字转换为字符
* Params :          hex:16进制数字,0~15;
* Return: 		    返回值:字符
* Limitation: 	    无
*******************************************************/
uint8_t hex2chr(uint8_t hex)
{
    if(hex<=9)return hex+'0';
    if(hex>=10&&hex<=15)return (hex-10+'A');
    return '0';
}


/*******************************************************
* Function Name: 	str2dec
* Purpose: 		    将1个字符串转换成数字
* Params :          hex:16进制数字,0~15;
* Return: 		    转换的数字
* Limitation: 	    无
*******************************************************/
int16_t str2dec(const char *s)
{
    int16_t num = 0;
    while(*s != '\0'){
        if((*s >= '0') && (*s <= '9')){
            num = num * 10 + (*s - '0');
        }else{
            return -1;
        }
        s++;
    }
    return num;
}

/*******************************************************
* Function Name: 	str2hex
* Purpose: 		    将1个字符串转换成数字
* Params :          hex:16进制数字,0~15;
* Return: 		    转换的数字
* Limitation: 	    无
*******************************************************/
uint16_t str2hex(const char *s)
{
    uint16_t num = 0;
    while(*s != '\0'){
        if((*s >= '0') && (*s <= '9')){
            num = num << 4;
            num += (*s - '0');
        }else if((*s >= 'A') && (*s <= 'F')){
            num = num << 4 ;
            num += ((*s - 'A')+10);
        }else if((*s >= 'a') && (*s <= 'f')){
            num = num << 4;
            num  += ((*s - 'a')+10);
        }else{
            return -1;
        }
        s++;
    }
    return num;
}


/******************************************************************************/

//char* SendAT_Mutex(ATSendType *AT , char *cmd , char *ack , uint16_t waittime)
char* SendAT_Mutex(ATSendType *AT , char *cmd , char *ack , uint32_t waittime)
{
    uint8_t  err = 0;
    char *res   = 0;
    uint8_t m_char = 0;
	uint32_t timeout;
    if(!AT || !cmd){
        return 0;
    }else{
        memset( AT->Buf , 0 ,sizeof( AT->Buf));
    }
    if((uint32_t)cmd<=0XFF){
        m_char = (uint32_t)cmd;
        (*AT->TxArr)(&m_char , 1);
    }else{
        (*AT->TxStr)(cmd);                                                      //发送命令
    }
    if(ack){		                                                        //需要等待应答
    	if(ASSERT_WAIT_TIME(waittime)){
	        while(waittime --){
	            OSMboxPend(AT->MSem,100,&err);
//					OSTimeDlyHMSM(0,0,0,100);
		//			OSMboxAccept(AT->MSem);
		//            uint32_t t = 1700000;
		//            while(t--);
		            res = strstr((const char*)AT->Buf,(const char*)ack);
		            if(res){
		                return res;
		            }
					OSTimeDlyHMSM(0,0,0,100);
		        }
    	}else{
    		memset(GET_BUFF_ADDR(waittime),0,SENDAT_BUFF_SIZE);
			timeout = GET_WAIT_TIME(waittime);
	        while(timeout--){
	            OSMboxPend(AT->MSem,100,&err);
//					OSTimeDlyHMSM(0,0,0,100);
		//			OSMboxAccept(AT->MSem);
		//            uint32_t t = 1700000;
		//            while(t--);
//		            res = strstr((const char*)AT->Buf,(const char*)ack);
					memcpy(GET_BUFF_ADDR(waittime),(const char*)AT->Buf,SENDAT_BUFF_SIZE);
					res = strstr((const char*)(GET_BUFF_ADDR(waittime)),(const char*)ack);			
		            if(res){
		                return res;
		            }
					OSTimeDlyHMSM(0,0,0,100);
		        }    		
    	}
    }
    return res;
}


/*
* Function Name: 	SendAT
* Purpose: 		    向通信模组发送控制指令和数据
* Params :          AT：选择发送串口 AT1、AT2 (AT1发送控制命令，AT2发送数据)
* Params :          cmd:模组指令
* Params :          ack:模组回复
* Params :          waittime:
* Params :          当需要使用到模组返回的值时，需要传递一个buffer指针使用 “SET_WAIT_TIME(timeout,AT_Buff)”作为参数传递
*										timeout:等待时间
*										AT_Buff:保持模组返回buff指针
*					不需要使用到模组返回值：
*										timeout:等待时间
* Return: 		    公用Buff指针
* Limitation: 	    无
*/

//char* SendAT(ATSendType *AT , char *cmd , char *ack , uint16_t waittime)
char* SendAT(ATSendType *AT , char *cmd , char *ack , uint32_t waittime)
{
	uint8_t err;
	char *AT_ret = 0, *AT1_ret = 0, *AT2_ret = 0;
	if(AT == &AT1){
//		OSMutexPend(MutexAT1Send,0,&err);
		OSSemPend(SemAT1Send,0,&err);
		AT1_ret = SendAT_Mutex(AT,cmd,ack,waittime);
		OSSemPost(SemAT1Send);
//		OSMutexPost(MutexAT1Send);		
//		if(waittime & 0x8000){
//			AT1_ret = Acess_AT_Buff(&AT1,AT1_ret,50);
//		}
		return AT1_ret;
	}else if(AT == &AT2){
//		OSMutexPend(MutexAT2Send,0,&err);
		OSSemPend(SemAT2Send,0,&err);
		AT2_ret = SendAT_Mutex(AT,cmd,ack,waittime);
		OSSemPost(SemAT2Send);
//		OSMutexPost(MutexAT2Send);		
//		if(waittime & 0x8000){
//			AT2_ret = Acess_AT_Buff(&AT2,AT2_ret,50);
//		}		
		return AT2_ret;
	}
	return AT_ret;
}

//char* Acess_AT_Buff(ATSendType *AT,char* buff_strstr,uint16_t timeout){
//	uint8_t err;
//	char *AT_ret = (char*)0, *AT1_ret = 0, *AT2_ret = 0;
//	if(AT == &AT1){
//		OSSemPend(SemAT1Buff,timeout,&err);
//		AT1_ret = buff_strstr;
//		return AT1_ret;
//	}else if(AT == &AT2){
//		OSSemPend(SemAT2Buff,timeout,&err);
//		AT2_ret = buff_strstr;
//		return AT2_ret;
//	}
//	return AT_ret;	
//}

//void* Release_AT_Buff(ATSendType *AT){
//	void *AT_ret = 0;
//	if(AT == &AT1){
//		OSSemPost(SemAT1Buff);
//	}else if(AT == &AT2){
//		OSSemPost(SemAT2Buff);
//	}
//	return (void*)0;
//}

/******************************************************************************/
void gprsPowerUp(void)
{
    BSP_LED_CTL (ALL_LED , LED_ON);
    do{
        OSTimeDlyHMSM(0,0,1,0);
        PWR_KEY_LOW();
	    if(dbgPrintf)(*dbgPrintf)("bsp1!\r\n");
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT1,"AT\r\n","OK",1))break;//测试通讯是否正常
        OSTimeDlyHMSM(0,0,1,0);
        if(SendAT(&AT1,"AT\r\n","OK",1))break;
        OSTimeDlyHMSM(0,0,1,0);
        if(SendAT(&AT1,"AT\r\n","OK",1))break;
        OSTimeDlyHMSM(0,0,1,0);
        if(SendAT(&AT1,"AT\r\n","OK",1))break;
		if(dbgPrintf)(*dbgPrintf)("bsp2!\r\n");
        PWR_KEY_HIGH();
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT1,"AT\r\n","OK",1))break;
    }while(!SendAT(&AT1,"AT\r\n","OK",2));
    PWR_KEY_HIGH();
	IWDG_ReloadCounter();
    SendAT(&AT1,"AT+IPR=115200\r\n","OK",2);
    SendAT(&AT2,"AT+IPR=115200\r\n","OK",2);
    SendAT(&AT1,"AT+CREG=2\r\n","OK",2); //设置信号显示方式为带基站信息
    SendAT(&AT1,"AT+CLCC=1\r\n","OK",2);  //开来电显示        
    SendAT(&AT1,"AT+CMGF=1\r\n","OK",2); //
    SendAT(&AT2,"AT+CFUN=1\r\n","OK",6);//模块进入全功能模式
    SendAT(&AT2,"AT+CIPMODE=0\r\n","OK",6);//非透传模式
    SendAT(&AT2,"AT+CIPQSEND=1\r\n","OK",6);//快传模式
    SendAT(&AT2,"AT+CIPMUX=0\r\n","OK",6);//多播查询
    BSP_LED_CTL (ALL_LED , LED_OFF);
}


/******************************
restart_sim800c
************************/

void close_sim800c(void)
{
    uint8_t times = 0;
//    update_downloading = 0;
	if(dbgPrintf)(*dbgPrintf)("Close sim800c module ......\r\n");
    while(!SendAT(&AT1,"AT+CPOWD=1\r\n","DOWN",6)){//1:正常关机，0:快速关机
        times++;
        if(times > 4){
		    PWR_KEY_LOW();
			OSTimeDlyHMSM(0,0,2,0);
            break;
        }
    }
}

/******************************
restart_sim800c
************************/



void restart_sim800c(void)
{
    uint8_t times = 0;
//    update_downloading = 0;
	if(dbgPrintf)(*dbgPrintf)("restart sim800c module ......\r\n");

    while(!SendAT(&AT1,"AT+CPOWD=1\r\n","DOWN",6)){//1:正常关机，0:快速关机
        times++;
        if(times > 2){
            break;
        }
    }
	stat_buffer.dtu_param_buff[DTU_SIM_RESET_COUNT]++;
//    OSTimeDlyHMSM(0,0,10,0);
    BSP_LED_CTL (ALL_LED , LED_ON);
    do{
        OSTimeDlyHMSM(0,0,1,0);
        PWR_KEY_LOW();
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT2,"AT\r\n","OK",1))break;
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT2,"AT\r\n","OK",1))break;
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT2,"AT\r\n","OK",1))break;
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT2,"AT\r\n","OK",1))break;
        PWR_KEY_HIGH();
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT2,"AT\r\n","OK",1))break;
    }while(!SendAT(&AT2,"AT\r\n","OK",2));
    PWR_KEY_HIGH();
    SendAT(&AT1,"AT+IPR=115200\r\n","OK",2);
    SendAT(&AT1,"AT+CREG=2\r\n","OK",2); //设置信号显示方式为带基站信息
    SendAT(&AT1,"AT+CLCC=1\r\n","OK",2);  //开来电显示        
    SendAT(&AT1,"AT+CMGF=1\r\n","OK",2); //
    SendAT(&AT2,"AT+CFUN=1\r\n","OK",3);//模块进入全功能模式
    SendAT(&AT2,"AT+CIPMODE=0\r\n","OK",6);//非透传模式
    SendAT(&AT2,"AT+CIPQSEND=1\r\n","OK",6);//快传模式
    SendAT(&AT2,"AT+CIPMUX=0\r\n","OK",6);//多播查询
    BSP_LED_CTL (ALL_LED , LED_OFF);
}

uint8_t resetnum =0; 
void gprsCheckReg(TypeGPRS *Ptr)
{
    uint8_t num = 0;
    char *p = NULL, *s = NULL;
	char AT_Buff[SENDAT_BUFF_SIZE];
    while(((num <= 30)||(g_Upload.TestSig))&&(!run_flag)){
        p = SendAT(&AT1,"AT+CREG?\r\n",",1",SET_WAIT_TIME(2,AT_Buff));
        if( p ){
            s = strstr( p , "\"");
            if( s ){
                p = strstr( s , "\r\n");
                if( p ){
                    memcpy(Ptr->CellInfo , s , (p - s));
                    Ptr->CellInfo[p - s] = '\0';
                }
                LedValue.SimSig = 1;
                if(LedValue.ErrSta != LED_STA_ON){
                    LedValue.ErrSta = LED_STA_OFF;
                }
            }
            if(LedValue.ErrSta != LED_STA_ON){
                LedValue.ErrSta = LED_STA_OFF;
            }
            LedValue.SimSig = 1;
			resetnum = 0;
            break;
        }else{
            p = SendAT(&AT1,"AT+CREG?\r\n",",5",SET_WAIT_TIME(2,AT_Buff));
            if( p ){
                s = strstr( p , "\"");
                if( s ){
                    p = strstr( s , "\r\n");
                    if( p ){
                        memcpy(Ptr->CellInfo , s , (p - s));
                        Ptr->CellInfo[p - s] = '\0';
                    }
                }
                LedValue.SimSig = 1;
                if(LedValue.ErrSta != LED_STA_ON){
                    LedValue.ErrSta = LED_STA_OFF;
                }
				resetnum = 0;
                break;
            }
        }
        LedValue.SimSig = 0;
		if(hyaline_time > 60){
			stat_buffer.dtu_param_buff[DTU_FIND_SIM_REGIST_FAIL_COUNT]++;
		}
        OSTimeDlyHMSM(0,0,1,0);
        num ++;
        if(num > 8)
            LedValue.ErrSta = LED_STA_BLINK;
        if((num > 15)&&(!g_Upload.TestSig))
        {
            if((resetnum++>=4)&&(!g_Upload.TestSig)){
				if(dbgPrintf)(*dbgPrintf)("Enter fly mode (reset num).......\r\n");				
                run_flag=1;
            }else{
                  restart_sim800c();
                  OSTimeDlyHMSM(0,0,30,0);
                  break;
            }
            
        }
        if(g_Upload.TestSig){
            if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                                      localTime.tm_year + 1900,
                                      localTime.tm_mon  + 1,
                                      localTime.tm_mday,
                                      localTime.tm_hour,
                                      localTime.tm_min,
                                      localTime.tm_sec,
                                      localTime.tm_wday);
            if(dbgPrintf)(*dbgPrintf)(" ->OnlineTest Mode!\r\n");
            if(LedValue.SimSig){
                writeSIMToCAN(0x00);
			}else if(!LED_SIM){
                writeSIMToCAN(0x01);
            }
        }
    }
}

//模块mac除掉IMEI的首位8后，剩余的作为MAC
uint8_t getIMEI(TypeGPRS *Ptr){
    char   *s , *p;
    s = SendAT(&AT1,"AT+CGSN\r\n","AT+CGSN",4);
    if(s){
        p = strstr(s , "ERROR");
        if(p){
            return 0;
        }else{
            s += 10;
            memcpy(Ptr->IMEI , s , 15);
            Ptr->IMEI[15] = '\0';
            g_TcpType.SrcAddr[0] = (chr2hex(Ptr->IMEI[1])<<4)|chr2hex(Ptr->IMEI[2]);
            g_TcpType.SrcAddr[1] = (chr2hex(Ptr->IMEI[3])<<4)|chr2hex(Ptr->IMEI[4]);
            g_TcpType.SrcAddr[2] = (chr2hex(Ptr->IMEI[5])<<4)|chr2hex(Ptr->IMEI[6]);
            g_TcpType.SrcAddr[3] = (chr2hex(Ptr->IMEI[7])<<4)|chr2hex(Ptr->IMEI[8]);
            g_TcpType.SrcAddr[4] = (chr2hex(Ptr->IMEI[9])<<4)|chr2hex(Ptr->IMEI[10]);
            g_TcpType.SrcAddr[5] = (chr2hex(Ptr->IMEI[11])<<4)|chr2hex(Ptr->IMEI[12]);
            g_TcpType.SrcAddr[6] = (chr2hex(Ptr->IMEI[13])<<4)|chr2hex(Ptr->IMEI[14]);
            return 1;
        }
    }else{
        return 0;
    }
}

uint8_t getICCID(TypeGPRS *Ptr)
{
    char   *s , *p;
    s = SendAT(&AT1,"AT+CCID\r\n","AT+CCID",4);
    if(s){
        p = strstr(s , "ERROR");
        if(p){
            return 0;
        }else{
            s += 10;
            memcpy(Ptr->ICCID , s , 20);
            Ptr->ICCID[20] = '\0';
            return 1;
        }
    }else{
        return 0;
    }
}
/*
*	this function is used to get gprs basestation information!!!
*	Author: Running
*	Date:	2018/1/12
*/

uint32_t base_station;

uint8_t GetBaseStation(TypeGPRS *ptr)
{
	TypeGPRS *p = ptr;
	char* s=NULL;
	int8_t i=28;
	s = strstr(p->CellInfo,"\"");
	if(s)
	{
		while(*++s != '\0'){
			if(*s != '\"'){
				base_station |= chr2hex(*s)<<i;
				i -= 4;
			}else{
				s += 3;
				base_station |= chr2hex(*s)<<i;
				i -= 4;
			}
		}
	}else{
		if(dbgPrintf)(*dbgPrintf)("can't get CellInfo!!!\r\n");	
		return 1;
	}
	return 0;
}

uint8_t get_signal_val;

uint8_t getSignalVal(uint8_t *res)
{
    uint8_t signalVal = 0;
    char *p = NULL, *s = NULL;
	char tmp_buf[SENDAT_BUFF_SIZE];
    p = SendAT(&AT1,"AT+CSQ\r\n","+CSQ: ",SET_WAIT_TIME(4,tmp_buf));
    if(p){
        p += 6;
        s = strstr(p , ",");
        while((s != p) && (s != 0)){
            if((s - p) > 1){
                signalVal  = chr2hex(*p++)*10;
                signalVal += chr2hex(*p++);
            }else{
                signalVal = chr2hex(*p++);
            }
        }
    }
    if(res){
        *res = signalVal;
    }
	get_signal_val = signalVal;//新增get_signal_val用来取实时信号值
	ok_flag = OK_IS_3S;
    return signalVal;
}

void checkNetStatus(void)
{
    char *p = NULL,*s = NULL,*t = NULL;
	char AT_Buff[SENDAT_BUFF_SIZE];
    p = SendAT(&AT1,"AT+CIPSTATUS\r\n","AT+CIPSTATUS",SET_WAIT_TIME(4,AT_Buff));
    if(p){
        s = strstr(p , "CONNECT OK");
        if(!s){
            if(ConSta.Status){
                ConSta.Status = 0;
            }                
        }else{
            ConSta.Status = 1;
        }
        t = strstr(p , "TCP CLOSED");
        if(t)  
        {
//            gprsDisConnect();
			QCgprsDisConnect();

        }      
    }
}

/******************************************************************************/

uint8_t gprsConnect(TypeGPRS *Ptr)
{
    char  q[50], *s = NULL , *p = NULL;
    static uint8_t reconTimes = 0;
    s = q;
    if(!SendAT(&AT2,"AT+CIPSHUT\r\n","OK",6)) return 0;
    if(!SendAT(&AT2,"AT+CIPMODE=1\r\n","OK",6))  return 0;
    if(!SendAT(&AT2,"AT+CIPCCFG=3,2,850,1\r\n","OK",6))  return 0;
    if(!SendAT(&AT2,"AT+CGATT?\r\n" , "+CGATT:" , 6))  return 0;
    sprintf(s , "AT+CSTT=\"%s\"\r\n" , Ptr->ApnAccr);
    if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
    if(!SendAT(&AT2,"AT+CIICR\r\n","OK",8))  return 0;
    if(!SendAT(&AT2,"AT+CIFSR\r\n",".",6))  return 0;
    sprintf(s , "AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r\n" , Ptr->SerName , Ptr->SerPort);
    p = SendAT(&AT2, s ,"CONNECT",10 );
    if(p){
        s = strstr(p , "FAIL");
        if(!s){
            LedValue.ConSta = LED_STA_ON;
            ConSta.Mode    = 1;
            ConSta.Status  = 1;
            reconTimes = 0;
            return 1;
        }else{
            if(reconTimes++ >= 10){
                if(dbgPrintf)(*dbgPrintf)("Reconnect Server Exceed Set Times!\r\n");
                restart_sim800c();
                OSTimeDlyHMSM(0,1,0,0);
            }
            return 0;
        }
    }else{
        if(reconTimes++ >= 10){
            if(dbgPrintf)(*dbgPrintf)("Reconnect Server Exceed Set Times!\r\n");
            restart_sim800c();
            OSTimeDlyHMSM(0,1,0,0);
        }
        return 0;
    }
}
//uint32_t connect_fail;//add by Running

uint8_t connect_flag = 1;//一小时标志，在连接服务器过程中，如果服务器连接失败，则会等待一小时后再连接
uint8_t QCgprsConnect(TypeGPRS *Ptr)
{
    char  q[50], *s = NULL , *p = NULL;
	char AT_Buff[SENDAT_BUFF_SIZE];
    static uint8_t reconTimes = 0;
    s = q;
    if((connect_flag)&&(!run_flag)){
		
//SGSN:Serving GPRS Support Node(local network)
//UE:User Equipment,UE发送Activate PDP context request 给SGSN,附带信息APN(想要访问的外部网络，对外部PDN(Public Data Network)的一个标识),
//SGSN接收到UE发送的请求后，做核对，在附着过程中拿到了签约数据，签约数据包括用户允许访问的APN等信息，判断UE请求的APN是否在签约数据中
//如果该请求的APN不在签约数据内，将直接拒绝这个PDP上下文的激活，流程结束。
//如果SGSN核对权限OK，则给GGSN发送 Create PDP Context Request消息。并从MS中复制大量的请求信息：
//其中4个为必须存在的信息：1、Tunnel Endpoint Identifier Data 2、NSAPI(NetWork Service Access Point Identifier)
//3、SGSN Address for signalling 4、SGSN Address for user traffic 5、Quality of Service Profile.
//分别是：用户面TEID，NSAPI，控制面SGSN地址，用户面SGSN地址和QoS Profile。
//GGSN在收到SGSN的激活请求后，做核对，模式选择，APN的检查，PDP地址类型检查，如果通过，则激活PDP Context，
//GGSN 回送Create PDP Context Response 消息给SGSN，并且给UE分配IP地址。
//SGSN接收到GGSN的激活响应后，建立RAB(3G)，并给UE发送Activate PDP Context Accept消息，将分配的IP地址下发，和QoS Profile.
//GGSN:Gateway GPRS Support Node(internet)

        if(!SendAT(&AT2,"AT+CIPSHUT\r\n","OK",8)) return 0;//Deactivate packet data protocol(PDP) Context
//        if(!SendAT(&AT2,"AT+CIPMODE?\r\n","OK",6))  return 0;//设置模式为正常模式(透传模式为不做任何的处理：编码，分发等)
        if(!SendAT(&AT2,"AT+CIPMODE=0\r\n","OK",6))  return 0;//设置模式为正常模式(透传模式为不做任何的处理：编码，分发等)
        if(!SendAT(&AT2,"AT+CGATT?\r\n" , "+CGATT:" , 6))  return 0;//附着GPRS服务
        sprintf(s , "AT+CSTT=\"%s\"\r\n" , Ptr->ApnAccr);//start task and set APN,username,password
        if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
        if(!SendAT(&AT2,"AT+CIICR\r\n","OK",8))  return 0;//激活移动场景(PDP Context)
        if(!SendAT(&AT2,"AT+CIFSR\r\n",".",6))  return 0;//获取分配IP
        sprintf(s , "AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r\n" , Ptr->SerName , Ptr->SerPort);//开始TCP、UDP连接
//这个命令只有在当前状态为 IP INITIAL 或者 IP Status in single state。可以建立TCP/UDP连接，
//所以当状态不是 IP INITIAL or IP STATUS时，执行AT+CIPSHUT使之进入该状态是必要的。
        p = SendAT(&AT2, s ,"CONNECT",SET_WAIT_TIME(10,AT_Buff));
        if(p){
            s = strstr(p , "FAIL");
            if(!s){
                LedValue.ConSta = LED_STA_ON;
                ConSta.Mode    = 1;
                ConSta.Status  = 1;
				stat_buffer.dtu_param_buff[DTU_OK_CNNCT_NET_COUNT]++;
                if(!SendAT(&AT2,"ATE0\r\n","OK",6))  return 0;
                if(!SendAT(&AT2,"AT+CIPQSEND=1\r\n","OK",6))  return 0;
                reconTimes = 0;
                return 1;
            }else{
				stat_buffer.dtu_param_buff[DTU_RE_CNNCT_NET_COUNT]++;
                if(reconTimes++ >= 10){
                    if(dbgPrintf)(*dbgPrintf)("Reconnect Server Exceed Set Times!\r\n");              
                    if(Connect_time<3){
                        Connect_time++;
                        if(dbgPrintf)(*dbgPrintf)("Reset times  -> %d\r\n",Connect_time);
                        restart_sim800c();
                        reconTimes = 0;
                        OSTimeDlyHMSM(0,0,30,0);
                    }else{
                        Connect_time = 0;
                        reconTimes = 0;
                        connect_flag = 0;
                    }
                }
                return 0;
            }
        }else{
			stat_buffer.dtu_param_buff[DTU_RE_CNNCT_NET_COUNT]++;
            if(reconTimes++ > 10){
                if(dbgPrintf)(*dbgPrintf)("Reconnect Server Exceed Set Times!\r\n");
                    if(Connect_time<3){
                        Connect_time++;
                        restart_sim800c();
                        OSTimeDlyHMSM(0,0,30,0);
                    }else 
                    {
                        Connect_time = 0;
                        reconTimes = 0;
                        connect_flag = 0;
                    }
            }
            return 0;
        }
    }
    return 0;
}

uint8_t QCgprsDisConnect(void)
{
    char *res;
    static uint8_t discon_count;
	char AT_Buff[SENDAT_BUFF_SIZE];
//    SendAT(&AT1,"AT+CIPCLOSE\r\n","OK",2);
    res = SendAT(&AT1,"AT+CIPSHUT\r\n","OK",SET_WAIT_TIME(8,AT_Buff));
    if(discon_count >= 15)
    {
        restart_sim800c();
        discon_count = 0;
        OSTimeDlyHMSM(0,1,0,0);
    }
    if(res){
        ConSta.Mode   = 0;
        ConSta.Status = 0;
        discon_count = 0;
		
        return 1;
    }else{
        discon_count++ ;
		
        return 0;        
    }
}

uint8_t gprsDisConnect(void)
{
    char *res;
    static uint8_t discon_count;
    OSTimeDlyHMSM(0,0,2,0);
    (*AT2.TxArr)("+++" , 3);
    OSTimeDlyHMSM(0,0,2,0);
    res = SendAT(&AT2,"AT+CIPSHUT\r\n","OK",4);
    if(discon_count >= 15)
    {
        restart_sim800c();
        discon_count = 0;
    }
    if(res){
        ConSta.Mode   = 0;
        ConSta.Status = 0;

        discon_count = 0;
        return 1;
    }else{
        discon_count++ ;
        return 0;        
    }
}

uint16_t tcpSendData(uint8_t *data , uint16_t dataLen)
{
    char buf[50];
    SendAT(&AT2,"AT+CIPQSEND=1\r\n","OK",6);
    sprintf(buf , "AT+CIPSEND=%d\r" , dataLen);
    SendAT(&AT2, buf , "DATA ACCEPT" ,6 );
    if(!ConSta.Status || !ConSta.Mode){
        if(AT2.TxArr)(*AT2.TxArr)(data , dataLen);
        return dataLen;
    }else{
        while(!gprsConnect(&GprsParam));
    }
    return 0;
}

/*******************************************************
* Function Name: 	Calc_CheckSum
* Purpose: 		    用串口3向模块发送一个字符串
* Params :          *pkt:要进行校验的数据包地址
*                   *size:要进行校验的数据包大小
* Return: 		    检验和
* Limitation: 	    无
*******************************************************/
uint8_t CalcCheckSum(uint8_t *pkt, int size)
{
    int i ;
    uint8_t m_CheckSum = 0;
    if (size == 0){
        return 0;
    }
    for (i = 0; i < size; i++){
        m_CheckSum = gc_DowCrcTable[m_CheckSum ^ pkt[i]];
    }
    return m_CheckSum;
}
#if 0
/*******************************************************
* Function Name: 	ftpdata_handle
* Purpose: 		 处理ftp获取的数据
* Params :          
*                   
* Return: 		    
* Limitation: 	    无
*******************************************************/

uint8_t ftpconnect(TypeUpdate *UpdateParam)
{
    uint8_t  i;
    char * s = NULL;
    char readbumber_buff[20];
    char q[50];
    s = q;
    char tempdata[150];
    memcpy(tempdata, UpdateParam->FtpAdrr, 20);
    memcpy(&tempdata[20], UpdateParam->Username, 20);
    memcpy(&tempdata[40], UpdateParam->PassWord, 10);
    memcpy(&tempdata[50], UpdateParam->FileName, 20);
    memcpy(&tempdata[70], UpdateParam->FilePath, 20);
    if(dbgPrintf)(*dbgPrintf)("the ftp info is: ");
    for(i=0; i<150;i++){
        if(dbgPrintf)(*dbgPrintf)("%c ", tempdata[i]);
    }
    if(dbgPrintf)(*dbgPrintf)("\r\n ");
    SendAT(&AT2,"AT+CGATT?\r\n" , "+CGATT:" , 6);
    SendAT(&AT2, "AT+CSTT=\"GREEAC.GD\"\r\n" ,"OK", 6 );
    SendAT(&AT2,"AT+CIFSR\r\n" , "OK" , 6);
    SendAT(&AT2, "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n" ,"OK", 6 );
    SendAT(&AT2, "AT+SAPBR=3,1,\"APN\",\"GREEAC.GD\"\r\n" ,"OK", 6 );
    SendAT(&AT2, "AT+SAPBR=1,1\r\n" ,"OK", 6 );  
    SendAT(&AT2,"AT+FTPCID=1\r\n","OK",6);
    SendAT(&AT2, "AT+FTPTYPE=\"I\"\r\n" ,"OK", 6 );
    
//    SendAT(&AT2,"AT+FTPSERV=\"192.13.182.156\"\r\n","OK",6);//设置ftp地址
    sprintf(s , "AT+FTPPORT=%d \r\n" , UpdateParam->FtpPort);
    if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
    sprintf(s , "AT+FTPSERV=\"%s\"\r\n" , UpdateParam->FtpAdrr);
    if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
    
//    SendAT(&AT2,"AT+FTPUN=\"390663\"\r\n","OK",6);//登陆用户名
    sprintf(s , "AT+FTPUN=\"%s\"\r\n" , UpdateParam->Username);
    if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
    
//    SendAT(&AT2,"AT+FTPPW=\"qwe!234\"\r\n","OK",6);//登陆密码
    sprintf(s , "AT+FTPPW=\"%s\"\r\n" , UpdateParam->PassWord);
    if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
    
    if(!update_downloading){                                       //设置了断点无需重新获取文件名
    //    SendAT(&AT2,"AT+FTPGETNAME=\"mv2.bin\"\r\n","OK",6);//获取文件名
        sprintf(s , "AT+FTPGETNAME=\"%s\"\r\n" , UpdateParam->FileName);
        if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
        
    //    SendAT(&AT2,"AT+FTPGETPATH=\"/\"\r\n","OK",6);//获取路径
        sprintf(s , "AT+FTPGETPATH=\"%s\"\r\n" , UpdateParam->FilePath);
        if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
//        
//        s = SendAT(&AT2, "AT+FTPSIZE", "+FTPSIZE:1,0", 20);
//        if(!s)  return 0;
//        sscanf(s,",%[^\r\n]", readbumber_buff);
//        UpdateParam.Rev_PacketSize = str2dec_32(readbumber_buff);
    }
    if(!SendAT(&AT2,"AT+FTPGET=1\r\n","FTPGET: 1,1",20)) return 0;//打开ftp get会话
    ConSta.updateStatus = 1;
    return 1;
}
#endif

uint8_t httpconnect(void)
{
      char * s = NULL;
      char readbumber_buff1[20];
      char q[120];
       s = q; 
      SendAT(&AT2,"AT+HTTPTERM\r\n","OK",4);//关闭HTTP服务
      SendAT(&AT2, "AT+SAPBR=0,1\r\n","OK",6);
//关闭CID：1的连接	 
      if(!SendAT(&AT2, "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n","OK",6 ));// return 0;
      if(!SendAT(&AT2, "AT+SAPBR=3,1,\"APN\",\"CMNET\"\r\n","OK",6 ));// return 0;
      //设置接入点名称
      if(!SendAT(&AT2, "AT+SAPBR=1,1\r\n","OK",6 ));// return 0;
      //打开数据连接
      
//      SendAT(&AT2, "AT+CREG=?\r\n","OK",6 );
      if(!SendAT(&AT2, "AT+HTTPINIT\r\n" ,"OK",6 )) {//HTTP初始化
        SendAT(&AT2,"AT+HTTPTERM\r\n","OK",10);//
         return 0;
      }
      if(!SendAT(&AT2, "AT+HTTPPARA=\"CID\",1\r\n" ,"OK",6 )) return 0;//设置参数
//CID 为强制标志，为链接上下文的ID      
//      SendAT(&AT2, "AT+HTTPPARA=\"URL\",\"http://192.13.182.156:7002/gree/program-upgrade-mobile!upgrade.do\"\r\n" ,"OK",6 );//gastest.gree.com:7003/upgrade/test.txt
      sprintf(s , "AT+HTTPPARA=\"URL\",\"%s\"\r\n" , UpdateParam.FilePath);//设置访问链接
      if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
      s = SendAT(&AT2, "AT+HTTPACTION=0\r\n" ,"+HTTPACTION: 0,200",100);
//激活HTTP请求，0表示get方式，1表示POST方式，2表示HEAD方式	  
          if(s){
              memset(readbumber_buff1,0,sizeof(readbumber_buff1));
              sscanf(s,"%*[^,],%*[^,],%s", readbumber_buff1);
              UpdateParam.Rev_PacketSize = str2dec_32(readbumber_buff1);
          }else{
              return 0;
          } 
      ConSta.updateStatus = 1;
      return 1;

}
uint8_t HttpconnectToServer(void)
{
    uint8_t Times = 0;
    do{
        httpconnect();
        if(Times++ >= 2){
            break;
        }
    }while(!(ConSta.updateStatus));
    if(ConSta.updateStatus){
        return 1;
    }else{
        return 0;
    }
}

/*******************************************************
* Function Name: 	str2dec
* Purpose: 		    将1个字符串转换成数字
* Params :          hex:16进制数字,0~15;
* Return: 		    转换的数字
* Limitation: 	    无
*******************************************************/
int32_t str2dec_32(const char *s)
{
    int32_t num = 0;
    while(*s != '\0'){
        if((*s >= '0') && (*s <= '9')){
            num = num * 10 + (*s - '0');
        }else{
            return -1;
        }
        s++;
    }
    return num;
}
