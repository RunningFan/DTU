#include "common.h"
#include "drv_button.h"
#include "drv_led.h"
#include "drv_rtc.h"
#include "drv_usart.h"
#include "drv_timer.h"
#include "drv_can.h"
#include "drv_sms.h"
#include "drv_flash.h"
#include "drv_gprs.h"
#include "drv_net.h"
#include "fsm.h"
#include "duplicate.h"
#include "statistic.h"
varType var;

uint8_t losepower;
uint8_t idleMode(void * , void **);
uint8_t debugMode(void * , void **);
uint8_t buttonMode(void * , void **);
uint8_t outdoorMode(void * , void **);
uint8_t indoorMode(void * , void **);
uint8_t realMode(void * , void **);
uint8_t testMode(void * , void **);
uint8_t updatMode(void * , void **);
timetype timer_button,timer_real,timer_debug,button_time;
uint8_t testbuffer[830];

//updateInfoType updateinfo = 
//{
//  //receive flag
//  {0},
//  //update pack type
//  {0},
//  //success flag
//  {1},
//  //writeaddr
//  {0x00F00000},
//  //read addr
//  {0x00F00000},
//};


FuncPtr systemMode[]={idleMode , debugMode , buttonMode , outdoorMode , indoorMode , realMode , testMode , updatMode};

uint8_t NS = s_idle; //定义下一状态

uint8_t idleMode(void *argc , void *argv[])                                     //当前系统是空闲状态
{
    varType *p = (varType *)argc;
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
    if(dbgPrintf)(*dbgPrintf)("%d ->SystemIdle Mode!\r\n" , *((int*)argv));
    
    memset(&timer_debug,0,sizeof(timer_debug));
    memset(&timer_real,0,sizeof(timer_real));
    memset(&timer_button,0,sizeof(timer_button));
    if(!(g_Upload.OutErr || g_Upload.InErr)) var.Hyaline = 0;
	
//    if((g_Upload.SendFlag == 1)&&(losepower == 1))
//    {   
//        if(last_var.outdoorErr == 1) var.outdoorErr = 1;
//        else if(last_var.indoorErr == 1) var.indoorErr = 1;
//        losepower = 0;
//    }
    
        Errflg = 1;            //空闲状态的时候会往flash里面存储bb值
        
    if( p->onlineTest ){                                                        //在线测试优先级最高，先判断
        p->realMonitor = p->projDebug = p->outdoorErr = p->indoorErr = p->buttonPush = 0;
        return s_test;
    }
    if(((!g_Upload.DebugOver)&&(!g_Upload.Debug))||g_Upload.FastTest){
        return s_idle;
    }
    getServerTime(s_idle);
    sign_everyday(s_idle); 
    
    if(ConSta.Status){
        while(!disConnectToServer());
    }
    if((to_s_idle)&&((var.outdoorErr == 1 )||(var.indoorErr == 1)))
    {
        var.outdoorErr = 0;
        var.indoorErr  = 0;
    }
    
    if( p->fpt_update ){ 
        while(!disConnectToServer());
        return s_update;}
    if( p->realMonitor ){
        p->onlineTest = p->projDebug = p->outdoorErr = p->indoorErr = p->buttonPush = 0;
        return s_real;
    }
    if( p->projDebug ){
        p->onlineTest = p->realMonitor = p->outdoorErr = p->indoorErr = p->buttonPush = 0;
        return s_debug;
    }
    if( p->outdoorErr ){
        p->onlineTest = p->realMonitor = p->projDebug = p->indoorErr = p->buttonPush = 0;
        return s_outdoor;
    }
    if( p->indoorErr ){
        p->onlineTest = p->realMonitor = p->projDebug = p->outdoorErr = p->buttonPush = 0;
        return s_indoor;
    }
    if( p->buttonPush ){
        p->onlineTest = p->realMonitor = p->projDebug = p->outdoorErr = p->indoorErr = p->buttonPush = 0;
        return s_button;
    }

    return s_idle;
}

uint16_t receive_count;
uint8_t fail_count;

uint8_t debugMode(void *argc , void *argv[])                                    //当前系统是调试上报状态
{
    uint8_t err = 0;
    BUFF_TYPE *s_Ptr = NULL;
    varType *p = (varType *)argc;

    p->buttonPush = 0;
    Errflg = 0;           //调试状态的时候不会往flash里面存储bb值
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
    if(dbgPrintf)(*dbgPrintf)("%d ->ProjectDebug Mode!\r\n" , *((int*)argv));
    if( p->onlineTest ){
         memset(&timer_debug,0,sizeof(timer_debug));
         receive_count = 0;
         return s_test;}
    if( p->fpt_update ){ 
        while(!disConnectToServer());
        return s_update;}
    if( p->realMonitor ){
        memset(&timer_debug,0,sizeof(timer_debug));
        receive_count = 0;
        while(!disConnectToServer());
        return s_real;}
    if( !p->projDebug ){
        memset(&timer_debug,0,sizeof(timer_debug));
        receive_count = 0;
        return s_idle;}
    
    sign_everyday(s_debug);
    if((!ConSta.Status)||F4_CONNECT){
        if(!ConSta.Status)
        {
            if(!connectToServer()){
                while(!disConnectToServer());
                return s_debug;
            }
        }
        if(!send89Frame(&g_TcpType)){
            while(!disConnectToServer());
            return s_debug;
        }
        if(!sendF3Frame(&g_TcpType)){
            while(!disConnectToServer());
            return s_debug;
        }
#if 1
        if(!send91Frame(&g_TcpType , 0x01)){
            while(!disConnectToServer());
            return s_debug;
        }
#endif
        F4_CONNECT = 0;
    }
    
    s_Ptr = (BUFF_TYPE *)OSQPend(QSemSend,1,&err);
    if(s_Ptr){
        if((s_Ptr->DataLen & 0x8000)&& (ConSta.Status==1)){
            var.Hyaline = 1;            
            send96Frame(&g_TcpType , s_Ptr->DataBuf , (s_Ptr->DataLen & 0x7FFF) - 6 , 0x01 , 0x00);
            s_Ptr->DataLen = 0;
        }else{
            memset(s_Ptr->DataBuf , 0 , 1024);
            s_Ptr->DataLen = 0;
        }
    }
    return s_debug;

}
uint8_t buttonMode(void *argc , void *argv[])                                   //当前系统是按键上报状态
{
    uint8_t err = 0;
    BUFF_TYPE *s_Ptr = NULL;
    varType *p = (varType *)argc;
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
    if(dbgPrintf)(*dbgPrintf)("%d ->PushButton Mode!\r\n" , *((int*)argv));
    Errflg = 0;          //按键状态的时候不会往flash里面存储bb值
    if( p->onlineTest ){
        memset(&timer_button,0,sizeof(timer_button));
        receive_count = 0 ;
        return s_test;}
    if( p->fpt_update ){ 
        while(!disConnectToServer());
        return s_update;}
    if( p->realMonitor ){ 
        memset(&timer_button,0,sizeof(timer_button));
        receive_count = 0 ;
        while(!disConnectToServer());
        return s_real;}
    if( p->projDebug ){
        memset(&timer_button,0,sizeof(timer_button));
        while(!disConnectToServer());
        receive_count = 0 ;
        return s_debug;}
    if( p->buttonPush ){       
        memset(&timer_button,0,sizeof(timer_button));
        p->buttonPush = 0;
        receive_count = 0 ;
        return s_idle;}
     if((time_account(&timer_button))>=(GprsParam.ButtTime*60))
     {
        memset(&button_time,0,sizeof(button_time));
        receive_count = 0 ;
        return s_idle;
     }
    sign_everyday(s_button);
    if((!ConSta.Status)||F4_CONNECT){  
        
        if(!ConSta.Status)
        {
            if(!connectToServer()){
                while(!disConnectToServer());
                return s_button;
            } 
        }

        if(!send89Frame(&g_TcpType)){
            while(!disConnectToServer());
            return s_button;
        }
        if(!sendF3Frame(&g_TcpType)){
            while(!disConnectToServer());
            return s_button;
        }  
#if 1
        if(!send91Frame(&g_TcpType , 0x02)){
            while(!disConnectToServer());
            return s_debug;
        }        
#endif
        F4_CONNECT = 0; 
    }
            
    s_Ptr = (BUFF_TYPE *)OSQPend(QSemSend,1,&err);
    if(s_Ptr){
        if((s_Ptr->DataLen & 0x8000) && (ConSta.Status==1)){
            if(receive_count >=400){ 
                 var.Hyaline = 0;    
                 send96Frame(&g_TcpType , &s_Ptr->DataBuf[0] , (s_Ptr->DataLen & 0x7FFF) - 6 , 0x02 , 0x01&(!g_Hyaline));     
            }else{
                 var.Hyaline = 1;
                 send96Frame(&g_TcpType , &s_Ptr->DataBuf[0] , (s_Ptr->DataLen & 0x7FFF) - 6 , 0x02 , 0x00&(!g_Hyaline));
            }           
            s_Ptr->DataLen = 0;
        }
    }
    return s_button;
}

uint8_t outdoorMode(void *argc , void *argv[])                                  //当前系统是外机故障上报状态
{
    varType *p = (varType *)argc;

    p->buttonPush = 0;
    Errflg = 0;     //外机故障的时候不会往flash里面存储bb值
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>", 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
    if(dbgPrintf)(*dbgPrintf)("%d ->OutdoorErr Mode!\r\n" , *((int*)argv));
    if( p->onlineTest )return s_test;
    if( p->fpt_update ){ 
        while(!disConnectToServer());
        return s_update;}
    if( p->realMonitor ){
        while(!disConnectToServer());
        var.outdoorErr = 0 ;
        g_Upload.SendFlag = 0;
        savePowerDownInfo(&g_Upload);
        return s_real;
        }
    if( p->projDebug ){
        while(!disConnectToServer());
        var.outdoorErr = 0 ;
        g_Upload.SendFlag = 0;
        savePowerDownInfo(&g_Upload);
        return s_debug;}
    if(to_s_idle)
    { 
        g_Upload.SendFlag = 0;
        var.outdoorErr = 0 ;
        savePowerDownInfo(&g_Upload);
        return s_idle;
    }
    sign_everyday(s_outdoor);
    if(g_Upload.SendFlag && (g_Upload.curTimeCnt >= g_Upload.saveFlashTime)){
        if((!ConSta.Status)||F4_CONNECT){
            if(!ConSta.Status){
                if(!connectToServer()){
                    while(!disConnectToServer());
                    return s_outdoor;
                }
            }
            if(!send89Frame(&g_TcpType)){
                while(!disConnectToServer());
                return s_outdoor;
            }
            if(!sendF3Frame(&g_TcpType)){
                while(!disConnectToServer());
                return s_outdoor;
            }
#if 1
            if(!send91Frame(&g_TcpType , 0x00)){
                while(!disConnectToServer());
                return s_debug;
            }
#endif
            F4_CONNECT = 0;
        }
        ReadData2Flash(&g_Upload);

        if(g_Upload.errPointTime <= g_Upload.readFlashTime)
        {     
            send96Frame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen & 0x7FFF) - 6, 0x00 , 0x00);   
                OSTimeDlyHMSM(0,0,1,0);
        }
        else if((g_Upload.errPointTime > g_Upload.readFlashTime)\
            &&(g_Upload.errPointTime - g_Upload.readFlashTime) < (GprsParam.OutErrTime*60))
        {
            send96Frame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen & 0x7FFF) - 6, 0x00 , 0x01);    
            OSTimeDlyHMSM(0,0,1,0);
        }
        else{
            g_Upload.SendFlag = 0;
            var.outdoorErr = 0;
            savePowerDownInfo(&g_Upload);
            return s_idle;
        }
    }else{
        if(dbgPrintf)(*dbgPrintf)("OutErrStartUploadServer  -> %d\r\n",(g_Upload.saveFlashTime - g_Upload.curTimeCnt));
    }
    return s_outdoor;
}

uint8_t indoorMode(void *argc , void *argv[])                                   //当前系统是内机故障上报状态
{
    varType *p = (varType *)argc;
    Errflg = 0;                    //内机故障的时候不往flash里面储存bb值
    p->buttonPush = 0;
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
    if(dbgPrintf)(*dbgPrintf)("%d ->IndoorErr Mode!\r\n" , *((int*)argv));
    if( p->onlineTest )return s_test;
    if( p->fpt_update ){ 
        while(!disConnectToServer());
        return s_update;}
    if( p->realMonitor ){
        while(!disConnectToServer());
        var.indoorErr = 0 ;
        g_Upload.SendFlag = 0;
        savePowerDownInfo(&g_Upload);
        return s_real;
        }
    if( p->projDebug ){
        while(!disConnectToServer());
        var.indoorErr = 0 ;
        g_Upload.SendFlag = 0;
        savePowerDownInfo(&g_Upload);
        return s_debug;}
    if(to_s_idle )
    { 
        g_Upload.SendFlag = 0;
        var.indoorErr = 0 ;
        savePowerDownInfo(&g_Upload);
        return s_idle;
     }
    sign_everyday(s_indoor);
    if(g_Upload.SendFlag && (g_Upload.curTimeCnt >= g_Upload.saveFlashTime)){
        if((!ConSta.Status)||F4_CONNECT){
            if((!ConSta.Status)){
                if(!connectToServer()){
                    while(!disConnectToServer());
                    return s_indoor;
                }
            }
            if(!send89Frame(&g_TcpType)){
                while(!disConnectToServer());
                return s_indoor;
            }
            if(!sendF3Frame(&g_TcpType)){
                while(!disConnectToServer());
                return s_indoor;
            }
#if 1
            if(!send91Frame(&g_TcpType , 0x05)){
                while(!disConnectToServer());
                return s_debug;
            }
#endif  
            F4_CONNECT = 0;
        }
        ReadData2Flash(&g_Upload);
#if 0
        if((g_Upload.errPointTime < g_Upload.readFlashTime)||\
          ((g_Upload.errPointTime - g_Upload.readFlashTime) < (GprsParam.InErrTime*60))){
            send96Frame(&g_TcpType , &g_Upload.ErrBuffer[6] , (g_Upload.ErrDataLen & 0x7FFF) - 6 , 0x05 , 0x00);

        }else{
            g_Upload.SendFlag = 0;
            var.indoorErr = 0;
            savePowerDownInfo(&g_Upload);
            return s_idle;
        }
    }else{
            if(dbgPrintf)(*dbgPrintf)("InErrStartUploadServer  -> %d\r\n",(g_Upload.saveFlashTime - g_Upload.curTimeCnt));
        }
    return s_indoor;
    
#endif
    
        if(g_Upload.errPointTime <= g_Upload.readFlashTime)
        {     
            send96Frame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen & 0x7FFF) - 6, 0x05 , 0x00);   
            OSTimeDlyHMSM(0,0,1,0);
        }
        else if((g_Upload.errPointTime > g_Upload.readFlashTime)\
            &&(g_Upload.errPointTime - g_Upload.readFlashTime) < (GprsParam.InerrtTime*60))
        {
            send96Frame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen & 0x7FFF) - 6, 0x05 , 0x01);    
            OSTimeDlyHMSM(0,0,1,0);
        }
        else{
            g_Upload.SendFlag = 0;
            var.indoorErr = 0;
            savePowerDownInfo(&g_Upload);
            return s_idle;
        }
    }else{
        if(dbgPrintf)(*dbgPrintf)("InErrStartUploadServer  -> %d\r\n",(g_Upload.saveFlashTime - g_Upload.curTimeCnt));
    }
    return s_indoor;
}

uint8_t realMode(void *argc , void *argv[])                                     //当前系统是实时监控状态
{
    uint8_t err = 0;
    BUFF_TYPE *s_Ptr = NULL;
    varType *p = (varType *)argc;
    p->buttonPush = 0;  
    Errflg = 0;                     //实时上报的时候不往flash里面写bb值
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
    if(dbgPrintf)(*dbgPrintf)("%d ->RealMonitor Mode!\r\n" , *((int*)argv));
    if( p->onlineTest ){       
        memset(&timer_button,0,sizeof(timer_real));
        receive_count = 0 ;
        return s_test;}
    if( p->fpt_update ){
        while(!disConnectToServer());
        memset(&timer_button,0,sizeof(timer_real));
        receive_count = 0 ;
        return s_update;}
    if( !p->realMonitor ){      
        while(!disConnectToServer());
        receive_count = 0 ;
        memset(&timer_button,0,sizeof(timer_real));
        var.Hyaline = 0;
        savePowerDownInfo(&g_Upload);
        return s_idle;}
    
    sign_everyday(s_real);

    if((!ConSta.Status)||F4_CONNECT){
        if((!ConSta.Status))
        {
            if(!connectToServer()){
                while(!disConnectToServer());
                return s_real;
            }
        }
        if(!send89Frame(&g_TcpType)){
            while(!disConnectToServer());
            return s_real;
        }
        if(!sendF3Frame(&g_TcpType)){
            while(!disConnectToServer());
            return s_real;
        }
#if 1
        if(!send91Frame(&g_TcpType , 0x03)){
            while(!disConnectToServer());
            return s_debug;
        }
#endif
        F4_CONNECT = 0;
    }
    
    s_Ptr = (BUFF_TYPE *)OSQPend(QSemSend,1,&err);
    if(s_Ptr){
        if((s_Ptr->DataLen & 0x8000)&&(ConSta.Status == 1)){
           if(receive_count >=400){ 
                 var.Hyaline = 0;    
                 send96Frame(&g_TcpType , &s_Ptr->DataBuf[0] , (s_Ptr->DataLen & 0x7FFF) - 6 , 0x03 , 0x01&(!g_Hyaline));     
            }   
            else{
                 var.Hyaline = 1;
                 send96Frame(&g_TcpType , &s_Ptr->DataBuf[0] , (s_Ptr->DataLen & 0x7FFF) - 6 , 0x03 , 0x00&(!g_Hyaline));
            } 
            s_Ptr->DataLen = 0; 
        }
    }

    return s_real;
}

uint8_t testMode(void *argc , void *argv[])                                     //当前系统是在线测试状态
{
    varType *p = (varType *)argc;
    p->buttonPush = 0;
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
    if(dbgPrintf)(*dbgPrintf)("%d ->OnlineTest Mode!\r\n" , *((int*)argv));
    if(LedValue.SimSig){
        writeSIMToCAN(0x00);
    }else{
        writeSIMToCAN(0x01);
    }
    OSTimeDlyHMSM(0,0,1,500);
    if( !p->onlineTest )return s_idle;
    return s_test;
}
                              
uint8_t updatMode(void *argc , void *argv[])                                     //当前系统是实时监控状态
{
    uint8_t err = 0;

    char *s, *q;
//    BUFF_TYPE *s_Ptr = NULL;
    varType *p = (varType *)argc;
    p->buttonPush = 0;  
    Errflg = 0;                     //实时上报的时候不往flash里面写bb值
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
    if(dbgPrintf)(*dbgPrintf)("%d ->RealMonitor Mode!\r\n" , *((int*)argv));
    if( p->onlineTest ){       
        memset(&timer_button,0,sizeof(timer_real));
        receive_count = 0 ;
        return s_test;}
    if(!ConSta.ftpStatus){
        while(!ftpconnect(&FtpParam));
    }
    if(ConSta.ftpStatus){
        char readbumber_buff[20];
        uint16_t readbumber = 0, i;
        uint8_t upgrade_buf[1024];
        s = SendAT(&AT2,"AT+FTPGET=2,1024\r\n","\r\n",10);
        if(strstr(s , "ERROR")){
            SendAT(&AT2,"AT+FTPQUIT\r\n","OK",6);                                                   //退出ftp会话
            var.fpt_update = 0;
            return s_idle;
        }
        if(strstr(s , "FTPGET: 2")){
          sscanf(s,"%*[^,],%[^\r\n]", readbumber_buff);
          readbumber = str2dec(readbumber_buff);
          if(readbumber){
              q = strstr(s,"\r\n");
              q += 2;
              if(!(updateinfo.update_writeaddr%0x10000)){
                  OSMutexPend(MutexFlash , 0 , &err);
                  sFLASH_EraseSector(updateinfo.update_writeaddr);
                  OSMutexPost(MutexFlash);
              }
              OSMutexPend(MutexFlash , 0 , &err);
              sFLASH_WriteBuffer((uint8_t *)q , updateinfo.update_writeaddr , readbumber);        //将收到的升级包写到Flash里面
              sFLASH_ReadBuffer(upgrade_buf , updateinfo.update_writeaddr , readbumber);
              OSMutexPost(MutexFlash);
              updateinfo.update_writeaddr += readbumber;
              if(dbgPrintf)(*dbgPrintf)("update_writeaddr is------>%d\r\n", updateinfo.update_writeaddr);
              
              //memcpy(upgrade_buf, q, readbumber);
              if(dbgPrintf)(*dbgPrintf)("read buff--->");
              for(i = 0; i < readbumber; i++){
                  if(dbgPrintf)(*dbgPrintf)("%2X " , upgrade_buf[i]);
              }
              if(dbgPrintf)(*dbgPrintf)("\r\n");
          }
        }else if(strstr(s, "FTPGET: 1,0")){                                                     //文件读完
            updateinfo.upate_receive_flag = 1;
            updateinfo.update_pack_type = 0x01;
            OSMutexPend(MutexFlash , 0 , &err);
            sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
            sFLASH_WriteBuffer((uint8_t *)&updateinfo , UPDATE_INFO_START_ADDR , sizeof(updateinfo));
            OSMutexPost(MutexFlash);
            var.fpt_update = 0;
            SendAT(&AT2,"AT+FTPQUIT\r\n","OK",6);                                              //退出ftp会话
            return s_idle;
          
        }else{
            ;
        }
    }
    return s_update;
}


void ModeStateMachine(void *argc , void *argv[])
{
    uint8_t prevNS = NS;            //获取上一次的状态
    NS = systemMode[NS](argc , argv);
    if(prevNS != NS){               //比较上一次和下一次状态，当模式有切换时，清空队列中的数据，防止AA和BB帧数据混乱
        OSQFlush (QSemSend);
        memset(SendBuffer , 0 , 5*sizeof(BUFF_TYPE));
        memset(g_Upload.RealBuffer , 0 , 1024);
        memset(g_Upload.ErrBuffer , 0 , 1024);
        g_Upload.RealDataLen = 0 ;
        g_Upload.ErrDataLen = 0 ;
    }
}

uint8_t disConnectToServer(void)
{
    LedValue.ConSta = 1;
    return (QCgprsDisConnect());
}

uint8_t connectToServer(void)
{
    uint8_t Times = 0;
    do{
        QCgprsConnect(&GprsParam);
        if(Times++ >= 5){
            break;
        }
    }while(!(ConSta.Status));
    if(ConSta.Status){
        return 1;
    }else{
        return 0;
    }
}