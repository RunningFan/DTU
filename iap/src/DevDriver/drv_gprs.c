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
ConType ConSta;
ATSendType AT1 = { uart1SendArry, uart1SendStr, NULL, 0 };
ATSendType AT2 = { uart3SendArry, uart3SendStr, NULL, 0 };

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


/******************************************************************************/

char* SendAT(ATSendType *AT , char *cmd , char *ack , uint16_t waittime)
{
    uint8_t  err = 0;
    char *res   = 0;
    uint8_t m_char = 0;
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
        while(waittime --){
            OSMboxPend(AT->MSem,50,&err);
            OSTimeDlyHMSM(0,0,0,100);
            res = strstr((const char*)AT->Buf,(const char*)ack);
            if(res){
                return res;
            }
        }
    }
    return res;
}

/******************************************************************************/
void gprsPowerUp(void)
{
    BSP_LED_CTL (ALL_LED , LED_ON);
    do{
        OSTimeDlyHMSM(0,0,1,0);
        PWR_KEY_LOW();
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT1,"AT\r\n","OK",1))break;
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT1,"AT\r\n","OK",1))break;
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT1,"AT\r\n","OK",1))break;
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT1,"AT\r\n","OK",1))break;
        PWR_KEY_HIGH();
        OSTimeDlyHMSM(0,0,2,0);
        if(SendAT(&AT1,"AT\r\n","OK",1))break;
    }while(!SendAT(&AT1,"AT\r\n","OK",2));
    PWR_KEY_HIGH();
    SendAT(&AT1,"AT+IPR=115200\r\n","OK",2);
    SendAT(&AT1,"AT+CREG=2\r\n","OK",2);                                        //设置信号显示方式为带基站信息
    SendAT(&AT1,"AT+CLCC=1\r\n","OK",2);                                        //开来电显示        
    SendAT(&AT1,"AT+CMGF=1\r\n","OK",2);  
    SendAT(&AT2,"AT+CFUN=1\r\n","OK",6);
    BSP_LED_CTL (ALL_LED , LED_OFF);
    BSP_IWDG_Init    ();
}

void gprsCheckReg(TypeGPRS *Ptr)
{
    uint8_t num = 0;
    char *p = NULL, *s = NULL;

    while(((num <= 30)||(g_Upload.TestSig))&&(!run_flag)){
        p = SendAT(&AT1,"AT+CREG?\r\n",",1",2);
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
            break;
        } else {
            p = SendAT(&AT1,"AT+CREG?\r\n",",5",2);
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
                break;
            }
        }
        LedValue.SimSig = 0;
        OSTimeDlyHMSM(0,0,1,0);
        num ++;
        if(num > 8)
            LedValue.ErrSta = LED_STA_BLINK;
        if((num > 15)&&(!g_Upload.TestSig))
            softReset();
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
            }else{
                writeSIMToCAN(0x01);
            }
        }
    }
    if(!LedValue.SimSig){
        softReset();
    }
}

uint8_t getIMEI(TypeGPRS *Ptr)
{
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

uint8_t getSignalVal(uint8_t *res)
{
    uint8_t signalVal = 0;
    char *p = NULL, *s = NULL;
    p = SendAT(&AT1,"AT+CSQ\r\n","+CSQ: ",4);
    if(p){
        p += 6;
        s = strstr(p , ",");
        while(s != p){
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
    return signalVal;
}

void checkNetStatus(void)
{
    char *p = NULL,*s = NULL,*t = NULL;
//    static uint8_t  cnt;
    p = SendAT(&AT1,"AT+CIPSTATUS\r\n","AT+CIPSTATUS",4);
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
            gprsDisConnect();
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
                softReset();
            }
            return 0;
        }
    }else{
        if(reconTimes++ >= 10){
            if(dbgPrintf)(*dbgPrintf)("Reconnect Server Exceed Set Times!\r\n");
            softReset();
        }
        return 0;
    }
}
uint8_t connect_flag = 1;
uint8_t QCgprsConnect(TypeGPRS *Ptr)
{
    char  q[50], *s = NULL , *p = NULL;
    uint8_t err;
    static uint8_t reconTimes = 0;
    s = q;
    if(!SendAT(&AT2,"AT+CIPRDTIMER=100,100\r\n","OK",6)) return 0;  //防止断线不反馈问题
    if((connect_flag)&&(!run_flag)){
        if(!SendAT(&AT2,"AT+CIPSHUT\r\n","OK",6)) return 0;
        if(!SendAT(&AT2,"AT+CIPMODE=0\r\n","OK",6))  return 0;
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
                if(!SendAT(&AT2,"ATE0\r\n","OK",6))  return 0;
                if(!SendAT(&AT2,"at+cipqsend=1\r\n","OK",6))  return 0;
                reconTimes = 0;
                return 1;
            }else{
                if(reconTimes++ > 10){
                    if(dbgPrintf)(*dbgPrintf)("Reconnect Server Exceed Set Times!\r\n");              
                    if(Connect_time<5){
                    Connect_time++;
                    if(dbgPrintf)(*dbgPrintf)("Reset times  -> %d\r\n",Connect_time);
                    OSMutexPend(MutexFlash , 0 , &err);
                    sFLASH_EraseSubSector(RECONNECT_START_ADDR);
                    sFLASH_WriteBuffer((uint8_t *)&Connect_time , RECONNECT_START_ADDR , sizeof(Connect_time));
                    OSMutexPost(MutexFlash); 
                    softReset();
                    }else 
                    {
                        Connect_time = 0;
                        reconTimes = 0;
                        OSMutexPend(MutexFlash , 0 , &err);
                        sFLASH_EraseSubSector(RECONNECT_START_ADDR);
                        sFLASH_WriteBuffer((uint8_t *)&Connect_time , RECONNECT_START_ADDR , sizeof(Connect_time));
                        OSMutexPost(MutexFlash); 
                        connect_flag = 0;
                    }
                }
                return 0;
            }
        }else{
            if(reconTimes++ > 10){
                if(dbgPrintf)(*dbgPrintf)("Reconnect Server Exceed Set Times!\r\n");
                    if(Connect_time<5){
                    Connect_time++;
                    OSMutexPend(MutexFlash , 0 , &err);
                    sFLASH_EraseSubSector(RECONNECT_START_ADDR);
                    sFLASH_WriteBuffer((uint8_t *)&Connect_time , RECONNECT_START_ADDR , sizeof(Connect_time));
                    OSMutexPost(MutexFlash); 
                    softReset();
                    }else 
                    {
                        Connect_time = 0;
                        reconTimes = 0;
                        OSMutexPend(MutexFlash , 0 , &err);
                        sFLASH_EraseSubSector(RECONNECT_START_ADDR);
                        sFLASH_WriteBuffer((uint8_t *)&Connect_time , RECONNECT_START_ADDR , sizeof(Connect_time));
                        OSMutexPost(MutexFlash); 
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
//    OSTimeDlyHMSM(0,0,2,0);
//    (*AT2.TxArr)("+++" , 3);
//    OSTimeDlyHMSM(0,0,2,0);
    SendAT(&AT2,"AT+CIPCLOSE\r\n","OK",4); 
    res = SendAT(&AT2,"AT+CIPSHUT\r\n","OK",4);
    if(discon_count >= 15)
    {
        softReset();
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

uint8_t gprsDisConnect(void)
{
    char *res;
    static uint8_t discon_count;
    OSTimeDlyHMSM(0,0,2,0);
    (*AT2.TxArr)("+++" , 3);
    OSTimeDlyHMSM(0,0,2,0);
//    SendAT(&AT2,"AT+CIPCLOSE\r\n","OK",4);
    res = SendAT(&AT2,"AT+CIPSHUT\r\n","OK",4);
    if(discon_count >= 15)
    {
        softReset();
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
    sprintf(buf , "AT+CIPSEND=%d\r\n" , dataLen);
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

/*******************************************************
* Function Name: 	ftpdata_handle
* Purpose: 		 处理ftp获取的数据
* Params :          
*                   
* Return: 		    
* Limitation: 	    无
*******************************************************/

uint8_t ftpconnect(TypeFTP *ftpParam)
{
    uint8_t err , cnt = 0;
    char * s = NULL;
    char q[50];
    s = q;
    SendAT(&AT2,"AT+CGATT?\r\n" , "+CGATT:" , 6);
    SendAT(&AT2, "AT+CSTT=\"GREEAC.GD\"\r\n" ,"OK", 6 );
    SendAT(&AT2,"AT+CIFSR\r\n" , "OK" , 6);
    SendAT(&AT2, "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n" ,"OK", 6 );
    SendAT(&AT2, "AT+SAPBR=3,1,\"APN\",\"GREEAC.GD\"\r\n" ,"OK", 6 );
    SendAT(&AT2, "AT+SAPBR=1,1\r\n" ,"OK", 6 );  
    SendAT(&AT2,"AT+FTPCID=1\r\n","OK",6);
    SendAT(&AT2, "AT+FTPTYPE=\"I\"\r\n" ,"OK", 6 );
    
    SendAT(&AT2,"AT+FTPSERV=\"192.13.182.156\"\r\n","OK",6);//设置ftp地址
//    sprintf(s , "AT+FTPSERV=\"%s\"\r\n" , ftpParam->FtpAdrr);
//    if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
    
    SendAT(&AT2,"AT+FTPUN=\"390663\"\r\n","OK",6);//登陆用户名
//    sprintf(s , "AT+FTPUN=\"%s\"\r\n" , ftpParam->Username);
//    if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
    
    SendAT(&AT2,"AT+FTPPW=\"qwe!234\"\r\n","OK",6);//登陆密码
//    sprintf(s , "AT+FTPPW=\"%s\"\r\n" , ftpParam->PassWord);
//    if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
    
    SendAT(&AT2,"AT+FTPGETNAME=\"12a.bin\"\r\n","OK",6);//获取文件名
//    sprintf(s , "AT+FTPGETNAME=\"%s\"\r\n" , ftpParam->FileName);
//    if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
    
    SendAT(&AT2,"AT+FTPGETPATH=\"/\"\r\n","OK",6);//获取路径
//    sprintf(s , "AT+FTPGETPATH=\"%s\"\r\n" , ftpParam->FilePath);
//    if(!SendAT(&AT2, s ,"OK",6 ))  return 0;
    
    if(!SendAT(&AT2,"AT+FTPGET=1\r\n","FTPGET: 1,1",20)) return 0;//打开ftp get会话
    ConSta.ftpStatus = 1;
    return 1;
}

