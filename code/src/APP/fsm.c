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
#include "bsp.h"
#include "drv_net.h"
#include "real_handle.h"


OS_EVENT * MSemQuickPhoto;
OS_EVENT * MSemSavePhoto;

varType var;

//uint16_t config_time = 30;
//uint8_t config_flag = 0;
uint8_t switch_one_two;//一二小时切换时间点
uint8_t five_min_flag;//五分钟计时标志

uint8_t tran_mode;
uint8_t bigdata_take_photo_over;
uint8_t bigdata_clear_buffer;

uint8_t LED_SIM;


uint8_t wait_update;//连接失败，进入等待状态
uint8_t losepower;
uint8_t idleMode(void * , void **);
uint8_t debugMode(void * , void **);
uint8_t buttonMode(void * , void **);
uint8_t outdoorMode(void * , void **);
uint8_t indoorMode(void * , void **);
uint8_t realMode(void * , void **);
uint8_t testMode(void * , void **);
uint8_t updatMode(void * , void **);
//uint32_t Real_sendTime;
uint16_t ModeState_time = 500;
uint16_t mode_time = 500;
uint8_t enter_state;//故障点拍照计数,只需要拍照一次
uint8_t enter_state_button;//故障点拍照计数,只需要拍照一次
uint8_t enter_state_monitor;//进入时间监控点,只需要拍照一次
uint8_t takeup98flag = 1;//实时监控模式下>30min间隔时，拍照标志
//uint8_t takeup98_flag = 0;

uint8_t erase_run_param_func();
uint8_t erase_real_error_flash_func();


void send_update_register(void);
void send_update_finish(void);
void send_update_erase(update_mac_type* m_update_mac);
uint16_t send_updateto_can(uint16_t first_addr, uint16_t second_addr, uint8_t num);
uint32_t read_can_update(BUFF_TYPE *m_sendcan_buffer, uint32_t flash_addr, uint16_t len);
//uint8_t  begin_real;
timetype timer_button,timer_real,timer_debug;
//button_time
uint8_t testbuffer[830];
BUFF_TYPE  sendcan_buffer; 
enum updatesteptype updatestep;
//uint8_t idleflag = 0;


updateInfoType updateinfo = 
{
	.upate_receive_flag = 0,
	.updata_hisSOFTVERSION = 0x24,
	.update_pack_type = 0,
	.update_success_flag = 1,
	.recover_have_packet = 0,
	.begin_recover_time = 5,
	.update_readaddr = PARAM_UPDATE_START_ADDR,
	.update_writeaddr = PARAM_UPDATE_START_ADDR,
};
//uint16_t  recieve_CheckSum = 0;

FuncPtr systemMode[]={idleMode , debugMode , buttonMode , outdoorMode , indoorMode , realMode , testMode, updatMode};

uint8_t NS = s_idle; //定义下一状态

uint8_t idleMode(void *argc , void *argv[])                                     //当前系统是空闲状态
{
    varType *p = (varType *)argc;
    mode_time = 500;
	uint8_t ret = 0,res = 0;
//	static uint8_t ConSta_count = 0;
	
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
    if( p->s_update ){ 
        while(!disConnectToServer());
        return s_update;}    
    if( p->onlineTest ){                                                        //在线测试优先级最高，先判断
        p->realMonitor = p->projDebug = p->outdoorErr = p->indoorErr = p->buttonPush = 0;
        return s_test;
    }
    if(((!g_Upload.DebugOver)&&(!g_Upload.Debug))||g_Upload.FastTest){
        return s_idle;
    }
    getServerTime(s_idle);
//#ifdef 	F2FRAME_DEBUG
//	heart_beat(s_idle);
//#endif
    sign_everyday(IDLE_MODE,s_idle);
	check_9E(IDLE_MODE,s_idle);

#ifdef weather
    askWeatherData(s_idle);
#endif
	g_Upload.alrd_find_start = 0;
	send_bigdata_frame(IDLE_MODE,s_idle);
//	if(!GprsParam.isOpenPower){
//		send_outdoor_power_data_frame(IDLE_MODE,s_idle);
//	}
#ifdef USING_USER_HABIT	
	send_user_habit_data_frame(IDLE_MODE,s_idle);
#endif

#ifdef 	F2FRAME_DEBUG
	heart_beat(IDLE_MODE,s_idle);
#endif

//	ret = read_user_data();
    check_EE_frame(IDLE_MODE,s_idle);//将EE帧转移到此任务下发送	

    if(ConSta.Status){
//		if(ConSta_count++ > 3){
//			ConSta_count = 0;
        	while(!disConnectToServer());
//		}
    }
	
    if((to_s_idle)&&((var.outdoorErr)||(var.indoorErr)))
    {
        var.outdoorErr = 0;
        var.indoorErr  = 0;
		g_Upload.alrd_find_start = 0;
		to_s_idle = 0;
    }

	if(to_s_idle){
		to_s_idle = 0;
	}
	
    if( p->s_update ){ 
        while(!disConnectToServer());
        return s_update;}
	
    if( p->realMonitor ){
        if(!(g_Upload.indoor_off_noupflag && g_Upload.IndOff)){
          p->onlineTest = p->projDebug = p->outdoorErr = p->indoorErr = p->buttonPush = 0;
          return s_real;
        }
    }

    if(g_Upload.erase_runparam_flash){
		to_s_idle = 0;
        erase_run_param_func();//erase_runparam_flash会在函数内部清零
    }

    if(g_Upload.erase_error_real){
		g_Upload.erase_error_real = 0;
        erase_real_error_flash_func();
		to_s_idle = 0;
    }
	
    if( p->projDebug ){
        p->onlineTest =  p->outdoorErr = p->indoorErr = p->buttonPush = 0;
        return s_debug;
    }
	
    if( p->outdoorErr ){
        p->onlineTest =  p->projDebug = p->indoorErr = p->buttonPush = 0;
        return s_outdoor;
    }
	
    if( p->indoorErr ){
        p->onlineTest =  p->projDebug = p->outdoorErr = p->buttonPush = 0;
        return s_indoor;
    }
	
    if( p->buttonPush ){
        p->onlineTest =  p->projDebug = p->outdoorErr = p->indoorErr = 0;
        return s_button;
    }
    return s_idle;
}

uint8_t fail_count;

uint8_t debugMode(void *argc , void *argv[])      //当前系统是调试上报状态
{
    uint8_t err = 0,res = 0;
    BUFF_TYPE *s_Ptr = NULL;
	if(g_Upload.taking_photo_flag){
		mode_time = 50;
	}else{
	    mode_time = 500;
	}
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
    if(dbgPrintf)(*dbgPrintf)("%d ->ProjectDebug Mode!\r\n" , *((int*)argv));
	
    if( p->onlineTest ){
         memset(&timer_debug,0,sizeof(timer_debug));
         return s_test;}	
	
    if( p->s_update ){ 
        while(!disConnectToServer());
        return s_update;}

    if(to_s_idle){ 
        return s_idle;
    }	
	
    if( p->realMonitor ){
      if(!(g_Upload.indoor_off_noupflag && g_Upload.IndOff)){
        memset(&timer_debug,0,sizeof(timer_debug));
        while(!disConnectToServer());
        return s_real;
      }
    }
	
    if( !p->projDebug ){
        memset(&timer_debug,0,sizeof(timer_debug));
        return s_idle;}

	
	send_bigdata_frame(PROJ_DBG_MODE,s_debug);
//	if(!GprsParam.isOpenPower){
//		send_outdoor_power_data_frame(PROJ_DBG_MODE,s_debug);
//	}
    
    sign_everyday(PROJ_DBG_MODE,s_debug);
	check_9E(PROJ_DBG_MODE,s_debug);


#ifdef weather	
    askWeatherData(s_debug);
#endif

#ifdef 	F2FRAME_DEBUG
		heart_beat(PROJ_DBG_MODE,s_debug);
#endif

	check_EE_frame(PROJ_DBG_MODE,s_debug);//将EE帧转移到此任务下发送	

    if((!ConSta.Status)||F4_CONNECT){
		SendDataToServer(PROJ_DBG_MODE,s_debug);
        F4_CONNECT = 0;
		OSMboxPostOpt(MSemQuickPhoto,(void *)0x9A,OS_POST_OPT_BROADCAST);
    }else if(ConSta.Status){
		if(take_photo_style){
			OSMboxPostOpt(MSemQuickPhoto,(void *)0x9A,OS_POST_OPT_BROADCAST);
		}else{
			res = (uint8_t)((uint32_t)OSMboxPend( MSemQuickPhoto , 1, &err ) - 0);
		}
    }
    
    s_Ptr = (BUFF_TYPE *)OSQPend(QSemSend,1,&err);
    if(s_Ptr){
        if((s_Ptr->DataLen & 0x8000)&& (ConSta.Status == 1)){
			if(!s_Ptr->data_type){
	            send96Frame(&g_TcpType , s_Ptr->DataBuf , (s_Ptr->DataLen & 0x7FFF) - 6 ,PROJ_DBG_MODE, CHANGE_MODE);
        	}else if(s_Ptr->data_type == 1){
				send9AFrame(&g_TcpType , &s_Ptr->DataBuf[0] , (s_Ptr->DataLen & 0x7FFF) - 18, 0x01, 0x01);
       		}
            memset(s_Ptr->DataBuf , 0 , 1024);
        	s_Ptr->DataLen = 0;
			OSMboxPostOpt(MSemSavePhoto,(void *)0x9B,OS_POST_OPT_BROADCAST);
        }else{
			memset(s_Ptr->DataBuf,0,1024);
			s_Ptr->DataLen = 0;
        }
    }
    return s_debug;

}
uint8_t buttonMode(void *argc , void *argv[])                                   //当前系统是按键上报状态
{
    uint8_t err = 0,res = 0;
    BUFF_TYPE *s_Ptr = NULL;
    varType *p = (varType *)argc;
	if(g_Upload.taking_photo_flag){
		mode_time = 50;
	}else{
	    mode_time = 500;
	}
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
	if(dbgPrintf)(*dbgPrintf)("%d ->PushButton Mode!\r\n" , *((int*)argv));
	
    if( p->onlineTest ){
        memset(&timer_button,0,sizeof(timer_button));
        return s_test;}
	
    if( p->s_update ){ 
        while(!disConnectToServer());
        return s_update;}

    if(to_s_idle){
        return s_idle;
    }	
	
    if( p->realMonitor){
      if(!(g_Upload.indoor_off_noupflag   && g_Upload.IndOff)){
        memset(&timer_button,0,sizeof(timer_button));
        while(!disConnectToServer());
        return s_real;
      }
    }
	
    if( p->projDebug ){
        memset(&timer_button,0,sizeof(timer_button));
        while(!disConnectToServer());
        return s_debug;}
	
    if( !p->buttonPush ){       
        memset(&timer_button,0,sizeof(timer_button));
        p->buttonPush = 0;
        return s_idle;}
	
     if((time_account(&timer_button))>=(GprsParam.ButtTime*60))
     {
//        memset(&button_time,0,sizeof(button_time));
		p->buttonPush = 0;
        return s_idle;
     }
	 
	send_bigdata_frame(BUTTON_MODE,s_button);
//	if(!GprsParam.isOpenPower){
//		send_outdoor_power_data_frame(BUTTON_MODE,s_button);
//	}
    sign_everyday(BUTTON_MODE,s_button);
	check_9E(BUTTON_MODE,s_button);

#ifdef weather	
	askWeatherData(s_button);
#endif

#ifdef 	F2FRAME_DEBUG
		heart_beat(BUTTON_MODE,s_button);
#endif

	check_EE_frame(BUTTON_MODE,s_button);//将EE帧转移到此任务下发送	

	
    if((!ConSta.Status)||F4_CONNECT){  
		SendDataToServer(BUTTON_MODE,s_button);
        F4_CONNECT = 0;
		OSMboxPostOpt(MSemQuickPhoto,(void *)0x9A,OS_POST_OPT_BROADCAST);
    }else if(ConSta.Status){
		if(take_photo_style){
			OSMboxPostOpt(MSemQuickPhoto,(void *)0x9A,OS_POST_OPT_BROADCAST);
		}else{
			res = (uint8_t)((uint32_t)OSMboxPend( MSemQuickPhoto , 1, &err ) - 0);
		}
    }

    s_Ptr = (BUFF_TYPE *)OSQPend(QSemSend,1,&err);
	if(s_Ptr){
	    if((s_Ptr->DataLen & 0x8000) && (ConSta.Status==1)){
	        if(s_Ptr->data_type == 0){ //去重数据
                send96Frame(&g_TcpType , &s_Ptr->DataBuf[0] , (s_Ptr->DataLen & 0x7FFF) - 6 ,BUTTON_MODE, CHANGE_MODE);    
	        }else if(s_Ptr->data_type == 1){
				send9AFrame(&g_TcpType , &s_Ptr->DataBuf[0] , (s_Ptr->DataLen & 0x7FFF) - 18, 0x01, 0x01);
	        }           
            memset(s_Ptr->DataBuf , 0 , 1024);
	        s_Ptr->DataLen = 0;
			OSMboxPostOpt(MSemSavePhoto,(void *)0x9B,OS_POST_OPT_BROADCAST);
	    }else{
            memset(s_Ptr->DataBuf , 0 , 1024);
            s_Ptr->DataLen = 0;
	    }
	}
    return s_button;
}

uint8_t outdoorMode(void *argc , void *argv[])                                  //当前系统是外机故障上报状态
{
    varType *p = (varType *)argc;
    mode_time = 500;
    p->buttonPush = 0;
	
	static uint8_t save_param_flag = 0;
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>", 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
    if(dbgPrintf)(*dbgPrintf)("%d ->OutdoorErr Mode!\r\n" , *((int*)argv));
	
    if( p->onlineTest ) 
		return s_test;
	
    if( p->s_update ){ 
        while(!disConnectToServer());
        return s_update;}
	
    if( p->realMonitor ){
      if(!(g_Upload.indoor_off_noupflag   && g_Upload.IndOff)){
        while(!disConnectToServer());
        var.outdoorErr = 0;
        g_Upload.SendFlag = 0;//实时监控时清除故障标志
		g_Upload.alrd_find_start = 0;
        savePowerDownInfo(&g_Upload);
        return s_real;
      }
	}
	
    if( p->projDebug){
        while(!disConnectToServer());
        var.outdoorErr = 0 ;
        g_Upload.SendFlag = 0;//调试模式下清除故障标志
		g_Upload.alrd_find_start = 0;		
        savePowerDownInfo(&g_Upload);
        return s_debug;
	}
	
    if(to_s_idle)
    { 
        g_Upload.SendFlag = 0;
        var.outdoorErr = 0 ;
		g_Upload.alrd_find_start = 0;		
        savePowerDownInfo(&g_Upload);
        return s_idle;
    }
//    sendbuff_flag = 0;
#ifdef	F2FRAME_DEBUG
//	heart_beat(s_outdoor);
#endif
	
	send_bigdata_frame(OUTDOOR_ERR_MODE,s_outdoor);
//	if(!GprsParam.isOpenPower){
//		send_outdoor_power_data_frame(OUTDOOR_ERR_MODE,s_outdoor);
//	}
    sign_everyday(OUTDOOR_ERR_MODE,s_outdoor);
	check_9E(OUTDOOR_ERR_MODE,s_outdoor);	
//	g_Upload.alrd_find_start = 0;

#ifdef weather	
    askWeatherData(s_outdoor);	
#endif

#ifdef 	F2FRAME_DEBUG
		heart_beat(OUTDOOR_ERR_MODE,s_outdoor);
#endif
	check_EE_frame(OUTDOOR_ERR_MODE,s_outdoor);//将EE帧转移到此任务下发送	

    if(g_Upload.SendFlag && (g_Upload.curTimeCnt > g_Upload.saveFlashTime)){
        if((!ConSta.Status)||F4_CONNECT){
			
			SendDataToServer(OUTDOOR_ERR_MODE,s_outdoor);
//            if(!ConSta.Status){
//                if(!connectToServer()){
//                    while(!disConnectToServer());
//                    return s_outdoor;
//                }
//            }
//            if(!send89Frame(&g_TcpType)){
//                while(!disConnectToServer());
//                return s_outdoor;
//            }
//            if(!sendF3Frame(&g_TcpType)){
//                while(!disConnectToServer());
//                return s_outdoor;
//            }
//#if 1
//            if(!send91Frame(&g_TcpType , 0x00)){
//                while(!disConnectToServer());
//                return s_debug;
//            }
//#endif
            F4_CONNECT = 0;
        }
		
       	uint8_t res = ReadData2Flash(&g_Upload,OUTDOOR_ERR);
		if(!g_Upload.flag_flash_revert){
	        if(g_Upload.err_address <= g_Upload.readFlashAddr){  
	            if(res == 1){//普通帧
					if(dbgPrintf)(*dbgPrintf)("Outdoor 96frame err read flash time:%d\r\n",g_Upload.readFlashTime);
	                send96Frame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen & 0x7FFF) - 6, OUTDOOR_ERR_MODE , CHANGE_MODE);//拍照帧
	            }else if(res == 2){//拍照帧
					if(dbgPrintf)(*dbgPrintf)("Outdoor 9Aframe err read flash time:%d\r\n",g_Upload.readFlashTime);
					send9AFrame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen    & 0x7FFF) - 18, 0x01, 0x01);//拍照帧
	            }else if(res == 0){
					if(dbgPrintf)(*dbgPrintf)("Error NULL parameter transmit...\n");
	            }else if(res == 3){
					if(dbgPrintf)(*dbgPrintf)("Outdoor error exist a empty block!...\n");
	            }

				switch (((g_Upload.errPointTime + GprsParam.Out_Err_After_Time*60) - g_Upload.readFlashTime)%300)
					{
						case 0:
							save_param_flag = 1;
							break;
							
						default:
							save_param_flag = 0;
					}
				if(save_param_flag){
				    save_param_flag = 0;
					if(dbgPrintf)(*dbgPrintf)("-------------->Outdoor instant point save<-------------\r\n ");
//					savePowerDownInfo(&g_Upload);
				}			
	            OSTimeDlyHMSM(0,0,0,500);
	        }else{
				g_Upload.alrd_find_start = 0;
	            g_Upload.SendFlag = 0;
	            var.outdoorErr = 0;
	            savePowerDownInfo(&g_Upload);
				if(dbgPrintf)(*dbgPrintf)("Outerr data already be transported over over over over over...\r\n");
	            return s_idle;
	        }
		}else{
	        if((g_Upload.err_address != g_Upload.readFlashAddr)){  
	            if(res == 1){//普通帧
					if(dbgPrintf)(*dbgPrintf)("Outdoor 96frame err read flash time:%d\r\n",g_Upload.readFlashTime);
	                send96Frame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen & 0x7FFF) - 6, OUTDOOR_ERR_MODE , CHANGE_MODE);//拍照帧
	            }else if(res == 2){//拍照帧
					if(dbgPrintf)(*dbgPrintf)("Outdoor 9Aframe err read flash time:%d\r\n",g_Upload.readFlashTime);
					send9AFrame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen    & 0x7FFF) - 18, 0x01, 0x01);//拍照帧
	            }else if(res == 0){
					if(dbgPrintf)(*dbgPrintf)("Error NULL parameter transmit...\n");
	            }else if(res == 3){
					if(dbgPrintf)(*dbgPrintf)("Outdoor error exist a empty block!...\n");
	            }

				switch (((g_Upload.errPointTime + GprsParam.Out_Err_After_Time*60) - g_Upload.readFlashTime)%300)
					{
						case 0:
							save_param_flag = 1;
							break;
							
						default:
							save_param_flag = 0;
					}
				if(save_param_flag){
				    save_param_flag = 0;
					if(dbgPrintf)(*dbgPrintf)("-------------->Outdoor instant point save<-------------\r\n ");
					savePowerDownInfo(&g_Upload);
				}			
	            OSTimeDlyHMSM(0,0,0,500);
	        }else{
				g_Upload.alrd_find_start = 0;
	            g_Upload.SendFlag = 0;
				if(dbgPrintf)(*dbgPrintf)("Outerr data g_Upload.SendFlag = 0\r\n");
				
	            var.outdoorErr = 0;
				g_Upload.flag_flash_revert   = 0;
	            savePowerDownInfo(&g_Upload);
				if(dbgPrintf)(*dbgPrintf)("Outerr data already be transported over over over over over...\r\n");
	            return s_idle;
	        }		
		}
    }else{
		    g_Upload.flag_flash_revert   = 0;
        if(dbgPrintf)(*dbgPrintf)("OutErr_StartUp_load_Server  -> %d\r\n",(g_Upload.saveFlashTime - g_Upload.curTimeCnt));
			if(var.outdoorErr){
				return s_outdoor;
			}else if(var.indoorErr){
				return s_indoor;
			}else{
				return s_idle;
			}
    }
    return s_outdoor;
}

uint8_t indoorMode(void *argc , void *argv[])                                   //当前系统是内机故障上报状态
{
    varType *p = (varType *)argc;
    mode_time = 500;
	static uint8_t save_param_flag = 0;
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
	
    if( p->onlineTest )
		return s_test;
	
    if( p->s_update ){ 
        while(!disConnectToServer());
        return s_update;}
	
    if( p->realMonitor ){
      if(!(g_Upload.indoor_off_noupflag && g_Upload.IndOff)){
        while(!disConnectToServer());
        var.indoorErr = 0 ;
        g_Upload.SendFlag = 0;
		g_Upload.alrd_find_start = 0;
        savePowerDownInfo(&g_Upload);
        return s_real;
      }
    }
	
    if( p->projDebug ){
        while(!disConnectToServer());
        var.indoorErr = 0 ;
        g_Upload.SendFlag = 0;
		g_Upload.alrd_find_start = 0;
        savePowerDownInfo(&g_Upload);
        return s_debug;}
	
    if(to_s_idle )
    { 
        g_Upload.SendFlag = 0;
        var.indoorErr = 0 ;
		g_Upload.alrd_find_start = 0;
        savePowerDownInfo(&g_Upload);
        return s_idle;
     }
//    sendbuff_flag = 0;
#ifdef F2FRAME_DEBUG	
//	heart_beat(s_indoor);
#endif
	
	send_bigdata_frame(INDOOR_ERR_MODE,s_indoor);
//	if(!GprsParam.isOpenPower){
//		send_outdoor_power_data_frame(INDOOR_ERR_MODE,s_indoor);
//	}

	
    sign_everyday(INDOOR_ERR_MODE,s_indoor);
	check_9E(INDOOR_ERR_MODE,s_indoor);	
//	g_Upload.alrd_find_start = 0;

#ifdef weather
    askWeatherData(s_indoor);
#endif

#ifdef 	F2FRAME_DEBUG
		heart_beat(INDOOR_ERR_MODE,s_indoor);
#endif

	check_EE_frame(INDOOR_ERR_MODE,s_indoor);//将EE帧转移到此任务下发送	

		
    if(g_Upload.SendFlag && (g_Upload.curTimeCnt > g_Upload.saveFlashTime)){
        if((!ConSta.Status)||F4_CONNECT){
			
		SendDataToServer(INDOOR_ERR_MODE,s_indoor);
//            if((!ConSta.Status)){
//                if(!connectToServer()){
//                    while(!disConnectToServer());
//                    return s_indoor;
//                }
//            }
//            if(!send89Frame(&g_TcpType)){
//                while(!disConnectToServer());
//                return s_indoor;
//            }
//            if(!sendF3Frame(&g_TcpType)){
//                while(!disConnectToServer());
//                return s_indoor;
//            }
//#if 1
//            if(!send91Frame(&g_TcpType , 0x05)){
//                while(!disConnectToServer());
//                return s_debug;
//            }
//#endif  
            F4_CONNECT = 0;
        }
		
	   uint8_t res = ReadData2Flash(&g_Upload,INDOOR_ERR);
		if(!g_Upload.flag_flash_revert){
			if(g_Upload.err_address <= g_Upload.readFlashAddr){	  
				if(res == 1){//普通帧
					if(dbgPrintf)(*dbgPrintf)("Indoor 96frame err read flash addr:%X\r\n",g_Upload.err_address);
					send96Frame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen & 0x7FFF) - 6,INDOOR_ERR_MODE, CHANGE_MODE);//拍照帧
				}else if(res == 2){//拍照帧
					if(dbgPrintf)(*dbgPrintf)("Indoor 9Aframe err read flash addr:%X\r\n",g_Upload.err_address);
					send9AFrame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen	  & 0x7FFF) - 18, 0x01, 0x01);//拍照帧
				}else if(res == 0){
					if(dbgPrintf)(*dbgPrintf)("Error NULL parameter transmit...\n");
				}else if(res == 3){
					if(dbgPrintf)(*dbgPrintf)("Indoor error exist a empty block!...\n");
				}
				switch (((g_Upload.errPointTime + GprsParam.In_Err_After_Time*60) - g_Upload.readFlashTime)%120)
					{
						case 0:
							save_param_flag = 1;
							break;					
							
						default:
							save_param_flag = 0;
					}
				if(save_param_flag){
					if(dbgPrintf)(*dbgPrintf)("-------------->Indoor instant point save<-------------\r\n ");
//					savePowerDownInfo(&g_Upload);
				}
				OSTimeDlyHMSM(0,0,0,500);
				
			}else{
				g_Upload.alrd_find_start = 0;
				g_Upload.SendFlag = 0;
				if(dbgPrintf)(*dbgPrintf)("Inerr data g_Upload.SendFlag = 0\r\n");
				
				var.indoorErr = 0;
				savePowerDownInfo(&g_Upload);
				if(dbgPrintf)(*dbgPrintf)("Inerr data already be transported over over over over over...\r\n");
				return s_idle;
			}
		}else{
			if(g_Upload.err_address != g_Upload.readFlashAddr){	  
				if(res == 1){//普通帧
					if(dbgPrintf)(*dbgPrintf)("Indoor 96frame err read flash addr:%X\r\n",g_Upload.err_address);
					send96Frame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen & 0x7FFF) - 6,INDOOR_ERR_MODE, CHANGE_MODE);//拍照帧
				}else if(res == 2){//拍照帧
					if(dbgPrintf)(*dbgPrintf)("Indoor 9Aframe err read flash addr:%X\r\n",g_Upload.err_address);
					send9AFrame(&g_TcpType , &g_Upload.ErrBuffer[0] , (g_Upload.ErrDataLen	  & 0x7FFF) - 18, 0x01, 0x01);//拍照帧
				}else if(res == 0){
					if(dbgPrintf)(*dbgPrintf)("Error NULL parameter transmit...\n");
				}else if(res == 3){
					if(dbgPrintf)(*dbgPrintf)("Indoor error exist a empty block!...\n");
				}
				switch (((g_Upload.errPointTime + GprsParam.In_Err_After_Time*60) - g_Upload.readFlashTime)%120)
					{
						case 0:
							save_param_flag = 1;
							break;					
							
						default:
							save_param_flag = 0;
					}
				if(save_param_flag){
					if(dbgPrintf)(*dbgPrintf)("-------------->Indoor instant point save<-------------\r\n ");
					savePowerDownInfo(&g_Upload);
				}
				OSTimeDlyHMSM(0,0,0,500);
				
			}else{
				g_Upload.alrd_find_start = 0;
				g_Upload.SendFlag = 0;
				var.indoorErr = 0;
				g_Upload.flag_flash_revert = 0;
				savePowerDownInfo(&g_Upload);
				if(dbgPrintf)(*dbgPrintf)("Inerr data already be transported over over over over over...\r\n");
				return s_idle;
			}			
		}
	}else{
		if(dbgPrintf)(*dbgPrintf)("IndoorErr_StartUp_load_Server  -> %d\r\n",(g_Upload.saveFlashTime - g_Upload.curTimeCnt));
			if(var.outdoorErr){
				return s_outdoor;
			}else if(var.indoorErr){//在故障上报未完成的情况下产生另一个故障，则此故障不会进行上报
				return s_indoor;
			}else{
				return s_idle;
			}
	}
    return s_indoor;
}



uint8_t realMode(void *argc , void *argv[])//当前系统是实时监控状态
{
    uint8_t err = 0,res = 0;
    BUFF_TYPE *s_Ptr = NULL;
    varType *p = (varType *)argc;
	if(g_Upload.taking_photo_flag){
    	mode_time = 50;
	}else{
		mode_time = 500;
	}
    p->buttonPush = 0;  
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
        return s_test;}
	
    if( p->s_update ){
        while(!disConnectToServer());
        memset(&timer_button,0,sizeof(timer_real));
        return s_update;
	}

    if(to_s_idle){
		g_Upload.real98flag = 0;
		g_Upload.realup98flag = 0;
		g_Upload.trans_mode = 0;
		var.realMonitor = 0;
		g_Upload.erase_error_real = 1;//从实时监控退出时，恢复模块默认的实时监控参数表
        return s_idle;
    }
	
    if( !p->realMonitor ){      
        while(!disConnectToServer());
        memset(&timer_button,0,sizeof(timer_real));
        var.Hyaline = 0;
		g_Upload.erase_error_real = 1;//从实时监控退出时，恢复模块默认的实时监控参数表
        savePowerDownInfo(&g_Upload);
        return s_idle;}
	
    if(g_Upload.indoor_off_noupflag && g_Upload.IndOff){//配置为开机监控 和 内机全关时
        return s_idle;
    }
	
	send_bigdata_frame(REALMONITOR_MODE,s_real);
//	if(!GprsParam.isOpenPower){
//		send_outdoor_power_data_frame(REALMONITOR_MODE,s_real);
//	}
    sign_everyday(REALMONITOR_MODE,s_real);
	check_9E(REALMONITOR_MODE,s_real);


#ifdef weather	
    askWeatherData(s_real);
#endif

#ifdef 	F2FRAME_DEBUG
		heart_beat(REALMONITOR_MODE,s_real);
#endif

	check_EE_frame(REALMONITOR_MODE,s_real);//将EE帧转移到此任务下发送	

		
    if((!ConSta.Status)||F4_CONNECT){
		
		SendDataToServer(REALMONITOR_MODE,s_real);
        F4_CONNECT = 0;
		OSMboxPostOpt(MSemQuickPhoto,(void *)0x9A,OS_POST_OPT_BROADCAST);
    } else if(ConSta.Status){
		if(take_photo_style){
			OSMboxPostOpt(MSemQuickPhoto,(void *)0x9A,OS_POST_OPT_BROADCAST);
		}else{
			res = (uint8_t)((uint32_t)OSMboxPend( MSemQuickPhoto , 1, &err ) - 0);
		}
    }
	
	if(g_Upload.real98flag < 2){
		
	s_Ptr = (BUFF_TYPE *)OSQPend(QSemSend,1,&err);
	if(s_Ptr){
		if((s_Ptr->DataLen & 0x8000) && (ConSta.Status)){
		 	if(g_Upload.realup98flag){  //实时监控的透传模式
				var.Hyaline = 1;
				if(!s_Ptr->data_type){
					send96Frame(&g_TcpType,&s_Ptr->DataBuf[0],(s_Ptr->DataLen & 0x7FFF) - 6,REALMONITOR_MODE,HYALINE_MODE);
				}else if(s_Ptr->data_type){
					send9AFrame(&g_TcpType , &s_Ptr->DataBuf[0] , (s_Ptr->DataLen & 0x7FFF) - 18, 0x01, 0x01);
				}
				s_Ptr->DataLen = 0;
				OSTimeDlyHMSM(0,0,0,100);
		 	}else{
		 			var.Hyaline = 0;
					switch (g_Upload.real98flag)
						{
							case 0x00:

							case 0x01:
								if(!s_Ptr->data_type && !g_Upload.trans_mode){
									send96Frame(&g_TcpType,&s_Ptr->DataBuf[0],(s_Ptr->DataLen & 0x7FFF) - 6,REALMONITOR_MODE,CHANGE_MODE);
								}else if(s_Ptr->data_type){
#ifdef DEBUG_ING									
									if(s_Ptr->DataLen < 18){
										exit(1);
									}
#endif									
									send9AFrame(&g_TcpType , &s_Ptr->DataBuf[0] , (s_Ptr->DataLen & 0x7FFF) - 18, 0x01, 0x01);
								}
								s_Ptr->DataLen = 0;
								break;					
							default:
								break;
						}
		 		}
	            memset(s_Ptr->DataBuf , 0 , 1024);
	            s_Ptr->DataLen = 0;
				OSMboxPostOpt(MSemSavePhoto,(void *)0x9B,OS_POST_OPT_BROADCAST);			
		 	}else{
	            memset(s_Ptr->DataBuf , 0 , 1024);
	            s_Ptr->DataLen = 0;
		 	}
		}
	}else{
		mode_time = 500;
		if(!g_Upload.trans_mode){
			s_Ptr = (BUFF_TYPE *)OSQPend(QSemSend,1,&err);
			if(s_Ptr){
				if(!s_Ptr->data_type){
					send96Frame(&g_TcpType , &s_Ptr->DataBuf[0] , (s_Ptr->DataLen & 0x7FFF) - 6,REALMONITOR_MODE, CHANGE_MODE);
				}else if(s_Ptr->data_type){
					send9AFrame(&g_TcpType , &s_Ptr->DataBuf[0] , (s_Ptr->DataLen & 0x7FFF) - 18, 0x01, 0x01);
				}
				s_Ptr->DataLen = 0;
	            memset(s_Ptr->DataBuf , 0 , 1024);
				OSMboxPostOpt(MSemSavePhoto,(void *)0x9B,OS_POST_OPT_BROADCAST);			
				memset(RealBuffer.DataBuf, 0 , 1026);
			}
		}else if(g_Upload.trans_mode && takeup98flag){
				OSMboxPostOpt(MSemSavePhoto,(void *)0x9B,OS_POST_OPT_BROADCAST);			
				take_photo_style = 0;
				sendreal_data();
				takeup98flag = 0;
				memset(RealBuffer_time_L30.DataBuf, 0 , 1026);
		}else{
//				if(takeup98_flag){
				if(g_Upload.flag_98){
					takeup98flag = 1;
//					takeup98_flag = 0;
					g_Upload.flag_98 = 0;
				}
		}
		s_Ptr->DataLen = 0;		
	}
    return s_real;
} 


uint8_t testMode(void *argc , void *argv[])                                     //当前系统是在线测试状态
{
    varType *p = (varType *)argc;
	static uint8_t assem_line = 0;
    p->buttonPush = 0;
    mode_time = 500;
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
    if(dbgPrintf)(*dbgPrintf)("%d ->OnlineTest Mode!\r\n" , *((int*)argv));
	if(LedValue.SimSig && !assem_line){
		assem_line = 1;

		LED_SIM = 1;

	
	}
	
    if(assem_line){
        writeSIMToCAN(0x00);
    }else{
        writeSIMToCAN(0x01);
    }
    OSTimeDlyHMSM(0,0,1,500);
    if( !p->onlineTest ){
		return s_idle;
    }
    return s_test;
}

uint8_t upgrade_buf[1024];
uint8_t updatMode(void *argc , void *argv[])                                     //当前系统是实时监控状态
{
    static uint8_t  connect_time = 0;
    uint8_t err = 0, re_ns, ee_flag;
    char *s, *q;
    char buf[50];
    varType *p = (varType *)argc;
    mode_time = 200;
    p->buttonPush = 0;  
    re_ns = s_update;
    if(dbgPrintf)(*dbgPrintf)("<SYS><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
    if(dbgPrintf)(*dbgPrintf)("%d ->HTTP:update Mode!\r\n" , *((int*)argv));
    if(!p->s_update){
		SendAT(&AT2,"AT+HTTPTERM\r\n","OK",10);//关闭HTTP服务
		ConSta.updateStatus = 0;
//		sendbuff_flag = 1;
        return s_idle;
    }
    if( p->onlineTest ){       
        memset(&timer_button,0,sizeof(timer_real));
        return s_test;
	}

    if(to_s_idle){ 
		p->s_update = 0;
		to_s_idle = 0;
        return s_update;
//		to_s_idle = 0;
//        return s_idle;
    }
#ifdef F2FRAME_DEBUG	
//	heart_beat(s_update);
#endif
    switch(updatestep){
        case m_getdata:
          if(!ConSta.updateStatus){
              do{
                  err = HttpconnectToServer();
                  if(!err){
                      restart_sim800c();    //连接不上复位模组
                      OSTimeDlyHMSM(0,0,30,0);
	                  connect_time ++;
                  }
				  if(dbgPrintf)(*dbgPrintf)("connect_time = %d\n",connect_time);
                  if(connect_time > 0){
                      var.s_update = 0;
                      wait_update = 1;
                      UpdateParam.try_time++;
                      if(UpdateParam.try_time > 2){
                          UpdateParam.update_downloading = 0;
                          var.s_update = 0;//退出获取升级数据模式
                          wait_update = 0;
                          updateinfo.update_readaddr  = PARAM_UPDATE_START_ADDR;
                          updateinfo.update_writeaddr = PARAM_UPDATE_START_ADDR;                                            //从新初始化信息  
                          UpdateParam.PacketSize = 0;                      
                          UpdateParam.CheckSum   = 0xFFFF;
                          ee_flag = 0xBB;
                          OSMutexPend(MutexFlash , 0 , &err);
                          sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
                          sFLASH_EraseSubSector(UPDATE_PARAM_START_ADDR);
                          sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
                          sFLASH_WriteBuffer((uint8_t *)&updateinfo , UPDATE_INFO_START_ADDR + 1 , sizeof(updateinfo));
                          sFLASH_WriteBuffer((uint8_t *)&UpdateParam ,UPDATE_PARAM_START_ADDR , sizeof(UpdateParam));
                          OSMutexPost(MutexFlash);
                      }else{
                          OSMutexPend(MutexFlash , 0 , &err);
                          sFLASH_EraseSubSector(UPDATE_PARAM_START_ADDR);
                          sFLASH_WriteBuffer((uint8_t *)&UpdateParam ,UPDATE_PARAM_START_ADDR , sizeof(UpdateParam));
                          OSMutexPost(MutexFlash);
                      }
					  connect_time = 0;
                      re_ns = s_idle;      //重连三次退出update状态,等待下一小时重连。
                      break;
                  }
              }while(!err);
              if(UpdateParam.update_downloading == 0){
                  connect_time = 0;
                  UpdateParam.PacketSize = 0;
                  UpdateParam.CheckSum   = 0xFFFF;                                                               //
              }
          }
          if(ConSta.updateStatus){
              char readbumber_buff[20];
              int readbumber = 0, i;
			  connect_time = 0;
              readbumber = UpdateParam.Rev_PacketSize-UpdateParam.PacketSize;
              if(readbumber>=1024){
                 s = buf;
                 sprintf(s , "AT+HTTPREAD=%d,1024\r\n" , UpdateParam.PacketSize);
                 s = SendAT(&AT2, s ,"\r\n",6);  
              }else if(readbumber >0){
                 s = buf;
                 sprintf(s , "AT+HTTPREAD=%d,%d\r\n" , UpdateParam.PacketSize,readbumber);
                 s = SendAT(&AT2, s ,"\r\n",6); 
              }else{                                             //结束获取文件
                  UpdateParam.update_downloading = 0;
                  var.s_update = 0;                              //退出获取升级数据模式
                  SendAT(&AT2,"AT+HTTPTERM\r\n","OK",10);
                  if(UpdateParam.CheckSum == UpdateParam.recieve_CheckSum){
                      if(dbgPrintf)(*dbgPrintf)("write the update info to flash\r\n");
                      updateinfo.upate_receive_flag = 1;
                      ee_flag = 0xBB;
                      OSMutexPend(MutexFlash , 0 , &err);
                      sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
                      sFLASH_EraseSubSector(UPDATE_PARAM_START_ADDR);
                      sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
                      sFLASH_WriteBuffer((uint8_t *)&updateinfo , UPDATE_INFO_START_ADDR + 1 , sizeof(updateinfo));
                      sFLASH_WriteBuffer((uint8_t *)&UpdateParam ,UPDATE_PARAM_START_ADDR , sizeof(UpdateParam));
                      OSMutexPost(MutexFlash);
#if 1                      
                      if(dbgPrintf)(*dbgPrintf)("ifno is------->" );
                      for(i = 0; i<sizeof(updateinfo); i++){
                          if(dbgPrintf)(*dbgPrintf)("%2X " , upgrade_buf[i]);
                      }
                      if(dbgPrintf)(*dbgPrintf)("\r\n");
#endif                      

                      if(updateinfo.update_pack_type == 0x01){                      //机组升级
  						  
                      }else if(updateinfo.update_pack_type == 0x02){                                                                                //gprs模块升级
                      		if(!UpdateParam.update_after_reconnect){
								var.realMonitor = 0;
                      		}
                            OSMutexPend(MutexFlash , 0 , &err);
				            sFLASH_EraseSubSector(VAR_START_ADDR);
				            sFLASH_EraseSubSector(ERR_INFO_START_ADDR);
				            sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
				            sFLASH_WriteBuffer((uint8_t *)&g_Upload , ERR_INFO_START_ADDR , sizeof(UpDataType)-G_UPLOAD_BUFFER_LEN*1024);
				            OSMutexPost(MutexFlash);
                          if(dbgPrintf)(*dbgPrintf)("reboot to update the program\r\n");
                          softReset();
                      }
                  }else{                                                                                               //校验失败               
                      if(dbgPrintf)(*dbgPrintf)("get wrong update package! \r\n");
                      updateinfo.update_readaddr  = PARAM_UPDATE_START_ADDR;
                      updateinfo.update_writeaddr = PARAM_UPDATE_START_ADDR;                                            //从新初始化信息  
                      UpdateParam.PacketSize = 0;                      
                      UpdateParam.CheckSum   = 0xFFFF;
                      ee_flag = 0xBB;
                      OSMutexPend(MutexFlash , 0 , &err);
                      sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
                      sFLASH_EraseSubSector(UPDATE_PARAM_START_ADDR);
                      sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
                      sFLASH_WriteBuffer((uint8_t *)&updateinfo , UPDATE_INFO_START_ADDR + 1 , sizeof(updateinfo));
                      sFLASH_WriteBuffer((uint8_t *)&UpdateParam ,UPDATE_PARAM_START_ADDR , sizeof(UpdateParam));
                      OSMutexPost(MutexFlash);
  				      if(!ConSta.Status){
					  	SendDataToServer(IDLE_MODE,s_update);//如果链接未建立则为空闲模式
				      }	
                      send97Frame(&g_TcpType, REQ_FEEDBACK, UPGRADE_PACKET_ERROR);
                  }
				  
                  return s_update;
              }
			  if(strstr(s , "\r\nERROR\r\n") || strstr(s , "+CME ERROR:")){//修改存在的问题AT+HTTPREAD=94208,1024	ERROR
//              if(strstr(s , "+CME ERROR:")){
                  SendAT(&AT2,"AT+HTTPTERM\r\n","OK",10);
                  var.s_update = 0;
                  ConSta.updateStatus = 0;
                  return re_ns;   
              }else if(strstr(s , "+HTTPREAD:")){//返回的数据为：/r/n+HTTPREAD: 12/r/n1233/r/nOK/r/n
                    memset(readbumber_buff,0,sizeof(readbumber_buff));
                    sscanf(s,"%*[^:]: %[^\r\n]", readbumber_buff);//注意：后面是有个空格符！
 
                    readbumber = (int)str2dec_32(readbumber_buff);
                    if(readbumber){
                        s += 2;
                        q = strstr(s,"\r\n");
                        q += 2;
                        if((!(updateinfo.update_writeaddr%0x10000))&&(updateinfo.update_writeaddr < PARAM_UPDATE_END_ADDR)){
                            OSMutexPend(MutexFlash , 0 , &err);
                            sFLASH_EraseSector(updateinfo.update_writeaddr);
                            OSMutexPost(MutexFlash);
                        }
                        OSMutexPend(MutexFlash , 0 , &err);
                        memcpy(upgrade_buf, (uint8_t *)q, readbumber);
                        sFLASH_WriteBuffer((uint8_t *)q ,PARAM_UPDATE_START_ADDR+UpdateParam.PacketSize , readbumber);        //将收到的升级包写到Flash里面
                        sFLASH_ReadBuffer(upgrade_buf , updateinfo.update_writeaddr , readbumber);
                        OSMutexPost(MutexFlash);
                        updateinfo.update_writeaddr += readbumber;
                        UpdateParam.PacketSize += readbumber;
                        UpdateParam.CheckSum = CRC16_CheckSum(upgrade_buf, readbumber, UpdateParam.CheckSum);
                        if(dbgPrintf)(*dbgPrintf)("update_writeaddr is------>%d\r\n", updateinfo.update_writeaddr);
    #if 1                   
                        if(dbgPrintf)(*dbgPrintf)("read buff--->");
                        for(i = 0; i < readbumber; i++){
                            if(dbgPrintf)(*dbgPrintf)("%2X " , upgrade_buf[i]);
                        }
                        if(dbgPrintf)(*dbgPrintf)("\r\n");
    #endif                    
                   }
              }
             
              LedValue.ConSta  = 3;
              LedValue.ConSpd  = 5;
              LedValue.BlinkTm = 6;
          }
          break;
          
  
    default:;
    }   
    return re_ns;
}
                              

void ModeStateMachine(void *argc , void *argv[])
{
    uint8_t prevNS = NS;            //获取上一次的状态
    NS = systemMode[NS](argc , argv);
    if(prevNS != NS){               //比较上一次和下一次状态，当模式有切换时，清空队列中的数据，防止AA和BB帧数据混乱
        OSQFlush (QSemSend);
        memset(SendBuffer , 0 , SEND_DATA_BUFF_SIZE*sizeof(BUFF_TYPE));
        memset(g_Upload.RealBuffer , 0 , 1024);
        memset(g_Upload.ErrBuffer , 0 , 1024);
        memset(RealBuffer.DataBuf, 0 , 1026);
        g_Upload.RealDataLen = 0 ;
        g_Upload.ErrDataLen = 0 ;
        connect_flag = 1;//模式切换时，重新连接服务器
		
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
        if(Times++ >= 4){
            break;
        }
    }while(!(ConSta.Status));
    if(ConSta.Status){
        return 1;
    }else{
        return 0;
    }
}


uint8_t SendDataToServer(uint8_t trans_mode,uint8_t mode_state)
{
		
		if(!ConSta.Status)
		{
			if(!connectToServer()){
				while(!disConnectToServer());
				return mode_state;
			}
		}
		if(!send89Frame(&g_TcpType)){
			while(!disConnectToServer());
			return mode_state;
		}
		if(!sendF3Frame(&g_TcpType)){
			while(!disConnectToServer());
			return mode_state;
		}
		if(!send91Frame(&g_TcpType , trans_mode)){
			while(!disConnectToServer());
			return mode_state;
		}
		return 0x08;

}


void send_update_register(void)
{
    CanTxMsg m_Data;
    m_Data.DLC      = 2;
    m_Data.RTR      = CAN_RTR_DATA;
    m_Data.IDE      = CAN_ID_EXT;
    m_Data.FunCode  = 0x8E;
    m_Data.CAN1IP   = 0x00;
    m_Data.CAN2IP   = 0x00;
    m_Data.DataType =  0x00;
    m_Data.Data[0]  =  0x7f;
    m_Data.Data[1]  =  0x00;
    CAN_Transmit(CAN1 , &m_Data);
}

void send_update_finish(void)
{
    CanTxMsg m_Data;
    m_Data.DLC      = 2;
    m_Data.RTR      = CAN_RTR_DATA;
    m_Data.IDE      = CAN_ID_EXT;
    m_Data.FunCode  = 0x82;
    m_Data.CAN1IP   = 0x7f;
    m_Data.CAN2IP   = 0x00;
    m_Data.DataType =  0x00;
    m_Data.Data[0]  =  0x00;
    m_Data.Data[1]  =  0x00;
    CAN_Transmit(CAN1 , &m_Data);
}

void send_update_erase(update_mac_type* m_update_mac)
{
    CanTxMsg m_Data;
    m_Data.DLC      = 2;
    m_Data.RTR      = CAN_RTR_DATA;
    m_Data.IDE      = CAN_ID_EXT;
    m_Data.FunCode  = 0x84;
    m_Data.CAN1IP   = 0x7f;
    m_Data.CAN2IP   = 0x00;
    m_Data.DataType =  0x00;
    m_Data.Data[0]  =  m_update_mac->first_index&0xFF;
    m_Data.Data[1]  =  (m_update_mac->first_index>>8)&0xFF;
    CAN_Transmit(CAN1 , &m_Data);
}

                            
uint16_t send_updateto_can(uint16_t first_addr, uint16_t second_addr, uint8_t num)
{
    CanTxMsg m_Data;
    m_Data.DLC             = num;
    m_Data.RTR             = CAN_RTR_DATA;
    m_Data.IDE             = CAN_ID_EXT;
    m_Data.FunCode1        = 0x8F;
    m_Data.second_index    = second_addr;
    m_Data.first_index     = first_addr;   
    
    memcpy(m_Data.Data, &sendcan_buffer.DataBuf[second_addr*8], num);
    second_addr += 1;
    CAN_Transmit(CAN1 , &m_Data);
    return second_addr;
}

uint32_t read_can_update(BUFF_TYPE *m_sendcan_buffer, uint32_t flash_addr, uint16_t len)
{
    uint16_t checksum = 0, i;
    sFLASH_ReadBuffer((uint8_t *)m_sendcan_buffer->DataBuf, flash_addr, len);
    flash_addr += len;
    if(len != 1024){
        for(i = len; i < 1024; i++){
            m_sendcan_buffer->DataBuf[i] = 0xFF;
        }
    }
    checksum = CRC16_Check(&m_sendcan_buffer->DataBuf[0], 1024);
    m_sendcan_buffer->DataBuf[1024] = checksum & 0xFF;
    m_sendcan_buffer->DataBuf[1025] = (checksum >> 8)& 0xFF;
    m_sendcan_buffer->DataLen = 1026;
    return flash_addr;
}

uint8_t erase_run_param_func(){
      uint8_t err;
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0;
#endif
		OS_ENTER_CRITICAL();
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_EraseSubSector(VAR_START_ADDR);
        sFLASH_EraseSubSector(ERR_INFO_START_ADDR);
        memset((uint8_t *)&var,0,sizeof(var));
        sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
        memset(&g_Upload,0,sizeof(g_Upload));
        g_Upload.EraserFlag = 0xBB;
        sFLASH_WriteBuffer((uint8_t *)&g_Upload , ERR_INFO_START_ADDR , sizeof(UpDataType)-G_UPLOAD_BUFFER_LEN*1024);
        OSMutexPost(MutexFlash);
        OS_EXIT_CRITICAL();
        return 0;
}

uint8_t erase_real_error_flash_func(){
      uint8_t err;
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0;
#endif

    OS_ENTER_CRITICAL();
    OSMutexPend(MutexFlash , 0 , &err);
    sFLASH_EraseSubSector(RECEIVE_PAGE_START_ADDR);
    memset(&Receive_Real_Page,0,sizeof(Receive_Real_Page));
    memcpy(&Receive_Real_Page,&Real_Page,sizeof(Receive_Real_Page));
    sFLASH_WriteBuffer((uint8_t *)&Receive_Real_Page , RECEIVE_PAGE_START_ADDR , sizeof(Receive_Real_Page));
    OSMutexPost(MutexFlash);            
    OS_EXIT_CRITICAL();
    return 0;

}


//分三种采样方式：
//1、字数据：判断其中一个变化，则上传两个值，高低位一并上传
//2、字节数据：判断其中一个变化，则上传该值
//3、位数据：判断8位中变化大于1个，则整个8位全部上传
//---->  BigDataParam_ChangeFlag
//      内机：3*8 = 24 => 22 
//            4*8 = 32 => 26

//   	外机：7*8 = 56 => 54
//            7*8 = 56 => 55
//内机：0-7     word
//      8-21 byte
//      22-47 bit			26-bit 	+6	4*8=32

//外机:   0-23    word
//      24-53   byte
//      54-107  byte		54-bit	+2	7*8=56


uint8_t bigDataSample(uint8_t sample_mode){
	uint8_t can1ip = 0;
	if(sample_mode){
		tran_mode = 1;//拍照模式
		if(open_up_take_photo && g_Upload.BigDataLen > 6){//拍照模式 且非刚开机状态的大数据拍照
			bigdata_take_photo_over = 1;
			bigdata_clear_buffer = 1;
	        bigData_sample_into_flash(NONSENCE,NONSENCE,NONSENCE,NONSENCE);
			bigdata_clear_buffer = 0;
			bigdata_take_photo_over = 0;
		}
	}else{
		tran_mode = 0;
	}
	
	bigData_Sample_Data(&parseData[0],BigDataParm.System,BigDataParm_pri.System,BigDataParam_ChangeFlag.System,BigDataParam_ExistFlag.System,OUTDOOR_PARAM | (OUTDOOR_BASE_IP<<8));
	for(uint8_t i=0;i<INDOOR_MCH_NUM;i++){
		if(indoorinfo[i].online_flag){
			can1ip = (uint8_t)(indoorinfo[i].Can1Can2IP & 0xFF);
			bigData_Sample_Data(&parseData[1],BigDataParm.Indoor[i],BigDataParm_pri.Indoor[i], BigDataParam_ChangeFlag.Indoor[i],BigDataParam_ExistFlag.Indoor[i],INDOOR_PARAM | can1ip<<8);
		}
	}

    if(tran_mode && open_up_take_photo){
		bigdata_take_photo_over = 1;
        bigData_sample_into_flash(NONSENCE,NONSENCE,NONSENCE,NONSENCE);
		tran_mode = 0;
	}else{
		tran_mode = 0;
		bigdata_take_photo_over = 0;
	}
//#ifdef TEST_TEST_OVER	
	memset(&BigDataParam_ChangeFlag,0,sizeof(BigDataParam_ChangeFlag));//清除大数据采样的变化标志
//#endif
	
//	sample_ok_flag = 1;
    return 0;
}

uint8_t bigData_Sample_Data(const ParseType *parse_type,uint8_t *bigdata_param,uint8_t *bigdata_param_pri,uint8_t *bigdata_change_flag,uint8_t *bigdata_exist_flag,uint16_t param_type){
	uint8_t j = 0,i = 0,ret_change = 0;

	if((uint8_t)(param_type & 0xFF) == OUTDOOR_PARAM){//外机参数时：
		for(j=0;j<parse_type->ParseSize;j++){//轮询外机转译参数表
			if(j < OUTDOOR_BYTE_ADDR){	//转译参数位于 word 区域
				if(((bigdata_change_flag[j/8]&(1<<(j%8))) || tran_mode) && (bigdata_exist_flag[j/8]&(1<<(j%8)))){
					if(tran_mode){
						if(!(j%2)){//word数据 偶数坐标
							bigData_sample_into_flash(bigdata_param[j+1]<<8 | bigdata_param[j],j,param_type,WORDTYPE);
							bigdata_param_pri[j+1] = bigdata_param[j+1];
							bigdata_param_pri[j] = bigdata_param[j];
							j += 1;//防止重复高低字节写入
						}else{//word 数据基数坐标
							bigData_sample_into_flash(bigdata_param[j]<<8 | bigdata_param[j-1],j-1,param_type,WORDTYPE);
							bigdata_param_pri[j] = bigdata_param[j];
							bigdata_param_pri[j-1] = bigdata_param[j-1];
						}
					}else{
						if(!(j%2)){//word数据 偶数坐标
							if((bigdata_param[j+1]<<8 | bigdata_param[j]) != (bigdata_param_pri[j+1]<<8 | bigdata_param_pri[j])){
								ret_change = judge_precision_second(match_parse(j,OUTDOOR_PARAM),bigdata_param,j,bigdata_param_pri,j,OUTDOOR_PARAM);
								if(ret_change){
									bigData_sample_into_flash(bigdata_param[j+1]<<8 | bigdata_param[j],j,param_type,WORDTYPE);
									bigdata_param_pri[j+1] = bigdata_param[j+1];
									bigdata_param_pri[j] = bigdata_param[j];									
								}
							}
//								bigData_sample_into_flash(bigdata_param[j+1]<<8 | bigdata_param[j],j,param_type,WORDTYPE);
								j += 1;//防止重复高低字节写入
						}else{//word 数据基数坐标
							if((bigdata_param[j]<<8 | bigdata_param[j-1]) != (bigdata_param_pri[j]<<8 | bigdata_param_pri[j-1])){
								ret_change = judge_precision_second(match_parse(j,OUTDOOR_PARAM),bigdata_param,j,bigdata_param_pri,j,OUTDOOR_PARAM);
								if(ret_change){
									bigData_sample_into_flash(bigdata_param[j]<<8 | bigdata_param[j-1],j-1,param_type,WORDTYPE);
									bigdata_param_pri[j] = bigdata_param[j];
									bigdata_param_pri[j-1] = bigdata_param[j-1];
								}								
							}						
//							bigData_sample_into_flash(bigdata_param[j]<<8 | bigdata_param[j-1],j-1,param_type,WORDTYPE);
						}					
//						ret_change = judge_precision_second();
//						if(ret_change)
					}
					
				}
			}else if(j >= OUTDOOR_BIT_ADDR){ //转译参数位于 bit 区域
				if(((bigdata_change_flag[OUTDOOR_BASE_SET + (j - OUTDOOR_BIT_ADDR)/8]) || tran_mode) && (bigdata_exist_flag[OUTDOOR_BASE_SET + (j - OUTDOOR_BIT_ADDR)/8])){//bit 区域内 8 个位置一起判断，数据也一起存储
//				if(((bigdata_change_flag[OUTDOOR_BASE_SET + (j - OUTDOOR_BIT_ADDR)/8]) || tran_mode)){//bit 区域内 8 个位置一起判断，数据也一起存储
					if(tran_mode){
						bigData_sample_into_flash(bigdata_param[(j - OUTDOOR_BIT_ADDR)/8 + OUTDOOR_BIT_ADDR],j,param_type,BITTYPE);
						bigdata_param_pri[(j - OUTDOOR_BIT_ADDR)/8 + OUTDOOR_BIT_ADDR] = bigdata_param[(j - OUTDOOR_BIT_ADDR)/8 + OUTDOOR_BIT_ADDR];
	//					(j - OUTDOOR_BIT_ADDR)/8 + OUTDOOR_BIT_ADDR,param_type,BITTYPE);
						j += 7;
					}else{
						if(bigdata_param[(j - OUTDOOR_BIT_ADDR)/8 + OUTDOOR_BIT_ADDR] != bigdata_param_pri[(j - OUTDOOR_BIT_ADDR)/8 + OUTDOOR_BIT_ADDR]){
							bigData_sample_into_flash(bigdata_param[(j - OUTDOOR_BIT_ADDR)/8 + OUTDOOR_BIT_ADDR],j,param_type,BITTYPE);
							bigdata_param_pri[(j - OUTDOOR_BIT_ADDR)/8 + OUTDOOR_BIT_ADDR] = bigdata_param[(j - OUTDOOR_BIT_ADDR)/8 + OUTDOOR_BIT_ADDR];
							
		//					(j - OUTDOOR_BIT_ADDR)/8 + OUTDOOR_BIT_ADDR,param_type,BITTYPE);
							j += 7;							
						}else{
							j += 7;
						}
					}
				}else{
					j += 7;
				}
			}else{	//转译参数位于 byte 区域
				if(((bigdata_change_flag[j/8]&(1<<(j%8))) || tran_mode) && (bigdata_exist_flag[j/8]&(1<<(j%8)))){
					if(tran_mode){
						bigData_sample_into_flash(bigdata_param[j],j,param_type,BYTETYPE);
						bigdata_param_pri[j] = bigdata_param[j];
					}else{
						if(bigdata_param[j] != bigdata_param_pri[j]){
							ret_change = judge_precision_second(match_parse(j,OUTDOOR_PARAM),bigdata_param,j,bigdata_param_pri,j,OUTDOOR_PARAM);
							if(ret_change){
								bigData_sample_into_flash(bigdata_param[j],j,param_type,BYTETYPE);
								bigdata_param_pri[j] = bigdata_param[j];
							}
						}
					}
				}
			}
		}
	}else{	//内机参数时：
		for(i=0;i<parse_type->ParseSize;i++){
			if(i < INDOOR_BTYE_ADDR){
//				if(((bigdata_change_flag[i/8]&(1<<(i%8)))|| tran_mode) && (BigDataParam_ExistFlag.Indoor[current_indoor_num][i/8]&(1<<(i%8)))){
//					if(!(i%2)){
//						bigData_sample_into_flash(bigdata_param[i+1]<<8 | bigdata_param[i],i,param_type,WORDTYPE);
//						i += 1;//防止重复高低字节写入
//					}else{
//						bigData_sample_into_flash(bigdata_param[i]<<8 | bigdata_param[i-1],i-1,param_type,WORDTYPE);
//					}
//				}

				if(((bigdata_change_flag[i/8]&(1<<(i%8))) || tran_mode) && (bigdata_exist_flag[i/8]&(1<<(i%8)))){
					if(tran_mode){
						if(!(i%2)){//word数据 偶数坐标
							bigData_sample_into_flash(bigdata_param[i+1]<<8 | bigdata_param[i],i,param_type,WORDTYPE);
							bigdata_param_pri[i+1] = bigdata_param[i+1];
							bigdata_param_pri[i] = bigdata_param[i];						
							i += 1;//防止重复高低字节写入
						}else{//word 数据基数坐标
							bigData_sample_into_flash(bigdata_param[i]<<8 | bigdata_param[i-1],i-1,param_type,WORDTYPE);
							bigdata_param_pri[i] = bigdata_param[i];
							bigdata_param_pri[i-1] = bigdata_param[i-1];						
						}
					}else{
						if(!(i%2)){//word数据 偶数坐标
							if((bigdata_param[i+1]<<8 | bigdata_param[i]) != (bigdata_param_pri[i+1]<<8 | bigdata_param_pri[i])){
								ret_change = judge_precision_second(match_parse(j,INDOOR_PARAM),bigdata_param,i,bigdata_param_pri,i,INDOOR_PARAM);
								if(ret_change){
									bigData_sample_into_flash(bigdata_param[i+1]<<8 | bigdata_param[i],i,param_type,WORDTYPE);
									bigdata_param_pri[i+1] = bigdata_param[i+1];
									bigdata_param_pri[i] = bigdata_param[i];									
								}
							}
	//								bigData_sample_into_flash(bigdata_param[i+1]<<8 | bigdata_param[i],i,param_type,WORDTYPE);
								i += 1;//防止重复高低字节写入
						}else{//word 数据基数坐标
							if((bigdata_param[i]<<8 | bigdata_param[i-1]) != (bigdata_param_pri[i]<<8 | bigdata_param_pri[i-1])){
								ret_change = judge_precision_second(match_parse(j,INDOOR_PARAM),bigdata_param,i,bigdata_param_pri,i,INDOOR_PARAM);
								if(ret_change){
									bigData_sample_into_flash(bigdata_param[i]<<8 | bigdata_param[i-1],i-1,param_type,WORDTYPE);
									bigdata_param_pri[i] = bigdata_param[i];
									bigdata_param_pri[i-1] = bigdata_param[i-1];
								}								
							}						
	//							bigData_sample_into_flash(bigdata_param[i]<<8 | bigdata_param[i-1],i-1,param_type,WORDTYPE);
						}					
	//						ret_change = judge_precision_second();
	//						if(ret_change)
					}
					
				}

				
			}else if(i >= INDOOR_BIT_ADDR){
				if(((bigdata_change_flag[INDOOR_BASE_SET + (i - INDOOR_BIT_ADDR)/8]) || tran_mode) && (bigdata_exist_flag[INDOOR_BASE_SET + (i - INDOOR_BIT_ADDR)/8])){
					if(tran_mode){					
						bigData_sample_into_flash(bigdata_param[(i - INDOOR_BIT_ADDR)/8 + INDOOR_BIT_ADDR],i,param_type,BITTYPE);
	//					(i - INDOOR_BIT_ADDR)/8 + INDOOR_BIT_ADDR,param_type,BITTYPE);
						bigdata_param_pri[(i - INDOOR_BIT_ADDR)/8 + INDOOR_BIT_ADDR] = bigdata_param[(i - INDOOR_BIT_ADDR)/8 + INDOOR_BIT_ADDR];
						i += 7;
					}else{
						if(bigdata_param[(i - INDOOR_BIT_ADDR)/8 + INDOOR_BIT_ADDR] != bigdata_param_pri[(i - INDOOR_BIT_ADDR)/8 + INDOOR_BIT_ADDR]){
							bigData_sample_into_flash(bigdata_param[(i - INDOOR_BIT_ADDR)/8 + INDOOR_BIT_ADDR],i,param_type,BITTYPE);
							bigdata_param_pri[(i - INDOOR_BIT_ADDR)/8 + INDOOR_BIT_ADDR] = bigdata_param[(i - INDOOR_BIT_ADDR)/8 + INDOOR_BIT_ADDR];
		//					(i - INDOOR_BIT_ADDR)/8 + INDOOR_BIT_ADDR,param_type,BITTYPE);
							i += 7;							
						}else{
							i += 7;
						}
					}
				}else{
					i += 7;
				}
			}else{
//				if(((bigdata_change_flag[i/8]&(1<<(i%8))) || tran_mode) && (BigDataParam_ExistFlag.Indoor[current_indoor_num][i/8]&(1<<(i%8)))){
//					bigData_sample_into_flash(bigdata_param[i],i,param_type,BYTETYPE);
//				}

				if(((bigdata_change_flag[i/8]&(1<<(i%8))) || tran_mode) && (bigdata_exist_flag[i/8]&(1<<(i%8)))){
					if(tran_mode){
						bigData_sample_into_flash(bigdata_param[i],i,param_type,BYTETYPE);
						bigdata_param_pri[i] = bigdata_param[i];
					}else{
						if(bigdata_param[i] != bigdata_param_pri[i]){
							ret_change = judge_precision_second(match_parse(j,INDOOR_PARAM),bigdata_param,i,bigdata_param_pri,i,INDOOR_PARAM);
							if(ret_change){
								bigData_sample_into_flash(bigdata_param[i],i,param_type,BYTETYPE);
								bigdata_param_pri[i] = bigdata_param[i];
							}
						}
					}
				}
				
			}
		}
		
	}
	return 0;
}



uint8_t bigData_sample_into_flash(uint16_t bigdata_param,uint8_t id,uint16_t can_ip,uint8_t data_type){

		uint16_t static length_tmp;
		uint8_t static ip_tmp;
		uint8_t static len_count;
	    uint32_t static time_tmp;
		static uint32_t sampe_count_9D;
		uint8_t err;
		if(g_Upload.BigDataLen < 2){
			g_Upload.BigDataLen = 2;
		}
		if((g_Upload.BigDataLen >= 2)&&(g_Upload.BigDataLen < 6)){
//			bigdata_packtime =0x8000;
	        memcpy(&g_Upload.BigDataSaveBuffer[2], &unixTimeStamp , 4);
	        g_Upload.BigDataLen += 4;
			time_tmp = f4_time/60;//时间基准
		}
		
		if(((g_Upload.BigDataLen >= 6) && (g_Upload.BigDataLen < 950)) && !bigdata_take_photo_over && !switch_one_two){
			
			if((ip_tmp != (uint8_t)(can_ip>>8)) || (g_Upload.Sample_9D_Count != sampe_count_9D)){//ip_tmp 初始值为0，第一帧数据采集时或者内外机之间切换时
				if(length_tmp){//
					g_Upload.BigDataSaveBuffer[length_tmp] = (uint8_t)(len_count);
					len_count = 0;
					length_tmp = 0;
				}
				g_Upload.BigDataSaveBuffer[g_Upload.BigDataLen++] = (uint8_t)(f4_time/60 - time_tmp);//时间偏移/min
				length_tmp = g_Upload.BigDataLen++;//长度填充位置定位符
				len_count++;
				g_Upload.BigDataSaveBuffer[g_Upload.BigDataLen++] = (uint8_t)(can_ip>>8);
				len_count++;
				ip_tmp = (uint8_t)(can_ip>>8);
				sampe_count_9D = g_Upload.Sample_9D_Count;
			}
			
			g_Upload.BigDataSaveBuffer[g_Upload.BigDataLen++] = id;
			len_count++;
			if(data_type == WORDTYPE){
				g_Upload.BigDataSaveBuffer[g_Upload.BigDataLen++] = (uint8_t)(bigdata_param & 0xFF);
				g_Upload.BigDataSaveBuffer[g_Upload.BigDataLen++] = (uint8_t)(bigdata_param >> 8 & 0xFF);
				len_count += 2;
			}else if(data_type == BYTETYPE){
				g_Upload.BigDataSaveBuffer[g_Upload.BigDataLen++] = (uint8_t)(bigdata_param);
				len_count++;
			}else{
				g_Upload.BigDataSaveBuffer[g_Upload.BigDataLen++] = (uint8_t)(bigdata_param);
				len_count++;
			}
//		}else if(((g_Upload.BigDataLen >= 800) && (g_Upload.BigDataLen <= 1024)) || (bigdata_take_photo_over && tran_mode)){
		}else if(((g_Upload.BigDataLen >= 950) && (g_Upload.BigDataLen <= 1024)) || bigdata_take_photo_over || switch_one_two){
		
			g_Upload.BigDataSaveBuffer[length_tmp] = (uint8_t)(len_count);
			len_count = 0;
			length_tmp = 0;
			ip_tmp = 0;
			memcpy(&g_Upload.BigDataSaveBuffer[0],&g_Upload.BigDataLen,2);
            if(!one_two_flag){
				if(tran_mode && open_up_take_photo && !bigdata_clear_buffer){
	                OSMutexPend(MutexFlash , 0 , &err);
	                sFLASH_WriteBuffer(&g_Upload.BigDataSaveBuffer[0], (uint32_t)BIGDATA_SAMPLE_START_ONE_ADDR, g_Upload.BigDataLen);        
//	                OSMutexPost(MutexFlash);
	                OSMutexPost(MutexFlash);
					g_Upload.BigDataLen = 0;
	                tran_mode = 0;
	                bigdata_take_photo_over = 0;				
				}else{
//					BigDataSampleTwoHours.tmp_addr = BigDataSampleTwoHours.write_addr;
//					BigDataSampleTwoHours.write_addr = BIGDATA_SAMPLE_START_TWO_ADDR;
	                if(!(BigDataSampleOneHours.write_addr % SUB_SECTOR_SIZE)){
	                    if(BigDataSampleOneHours.write_addr >= BIGDATA_SAMPLE_END_ONE_ADDR){
	                        BigDataSampleOneHours.write_addr = BIGDATA_SAMPLE_START_ONE_ADDR;
	                    }
	                    OSMutexPend(MutexFlash , 0 , &err);
	                    sFLASH_EraseSubSector(BigDataSampleOneHours.write_addr);         //擦除一个扇区，大小4KB
	                    OSMutexPost(MutexFlash);
	                    if(BigDataSampleOneHours.write_addr == BIGDATA_SAMPLE_START_ONE_ADDR){
	                        BigDataSampleOneHours.write_addr += CAN_WRITE_SIZE;
	                    }
	                }
	                OSMutexPend(MutexFlash , 0 , &err);
	                sFLASH_WriteBuffer(&g_Upload.BigDataSaveBuffer[0], BigDataSampleOneHours.write_addr, g_Upload.BigDataLen);        
	                OSMutexPost(MutexFlash);
	                
	                BigDataSampleOneHours.write_addr += CAN_WRITE_SIZE;
					
					BigDataSampleOneHours.tmp_addr = BigDataSampleOneHours.write_addr;
//					BigDataSampleOneHours.write_addr = BIGDATA_SAMPLE_START_ONE_ADDR;
					
	                if(BigDataSampleOneHours.write_addr >= BIGDATA_SAMPLE_END_ONE_ADDR){
	                    BigDataSampleOneHours.write_addr = BIGDATA_SAMPLE_START_ONE_ADDR;
						is_spill_flag = 1;
	                }
	    	        g_Upload.BigDataLen  = 0;
				}
	        }else{
	        	if(tran_mode && open_up_take_photo && !bigdata_clear_buffer){
	                OSMutexPend(MutexFlash , 0 , &err);
	                sFLASH_WriteBuffer(&g_Upload.BigDataSaveBuffer[0], (uint32_t)BIGDATA_SAMPLE_START_TWO_ADDR, g_Upload.BigDataLen);        
	                OSMutexPost(MutexFlash);
					g_Upload.BigDataLen = 0;
	                tran_mode = 0;
	                bigdata_take_photo_over = 0;					
	        	}else{
//					BigDataSampleOneHours.tmp_addr = BigDataSampleOneHours.write_addr;
//					BigDataSampleOneHours.write_addr = BIGDATA_SAMPLE_START_ONE_ADDR;
	                if(!(BigDataSampleTwoHours.write_addr % SUB_SECTOR_SIZE)){
	                    if(BigDataSampleTwoHours.write_addr >= BIGDATA_SAMPLE_END_TWO_ADDR){
	                        BigDataSampleTwoHours.write_addr = BIGDATA_SAMPLE_START_TWO_ADDR;
	                    }
	                    OSMutexPend(MutexFlash , 0 , &err);
	                    sFLASH_EraseSubSector(BigDataSampleTwoHours.write_addr);         //擦除一个扇区，大小4KB
	                    OSMutexPost(MutexFlash);
	                    if(BigDataSampleTwoHours.write_addr == BIGDATA_SAMPLE_START_TWO_ADDR){
	                        BigDataSampleTwoHours.write_addr += CAN_WRITE_SIZE;
	                    }
	                }
	                OSMutexPend(MutexFlash , 0 , &err);
	                sFLASH_WriteBuffer(&g_Upload.BigDataSaveBuffer[0], BigDataSampleTwoHours.write_addr, g_Upload.BigDataLen);        
	                OSMutexPost(MutexFlash);
	                
	                BigDataSampleTwoHours.write_addr += CAN_WRITE_SIZE;

					BigDataSampleTwoHours.tmp_addr = BigDataSampleTwoHours.write_addr;
//					BigDataSampleTwoHours.write_addr = BIGDATA_SAMPLE_START_TWO_ADDR;					
	                
	                if(BigDataSampleTwoHours.write_addr >= BIGDATA_SAMPLE_END_TWO_ADDR){
	                    BigDataSampleTwoHours.write_addr = BIGDATA_SAMPLE_START_TWO_ADDR;
	                }   
	    	        g_Upload.BigDataLen  = 0;
	        	}
	        }			

		}else if(g_Upload.BigDataLen > 1024){
			g_Upload.BigDataLen = 0;
		}
		
	    return 0;
}


uint16_t match_parse(uint8_t id,uint8_t param_type){
	ParseType *parse_data;
   	if(param_type == OUTDOOR_PARAM || param_type == OUTDOOR_PARAM_EXIST){
   		parse_data = &parseData[0];//外机转译表	
   	}else if(param_type == INDOOR_PARAM || param_type == INDOOR_PARAM_EXIST){
   		parse_data = &parseData[1];//内机转译表
   	}

	for(uint8_t i=0;i<parse_data->ParseSize;i++){
		if(id == parse_data->parse[i].ParsePlace){
#if DEBUG
			if(parse_data->parse[i].SrcAdd == 0x3150){
				if(dbgPrintf)(*dbgPrintf)("0x3150...\r\n");
			}
#endif			
			return parse_data->parse[i].SrcAdd;
		}
	}

	if(dbgPrintf)(*dbgPrintf)("Parse table can't find...\r\n");
	return 0xFF;

}


void check_packet_crc(updateInfoType *updateinfo1,TypeUpdate *UpdateParam1,uint8_t * buf_1024_size){//做成封装，其他模块直接调用
	updateInfoType update_info_tmp;//临时保存升级信息
	uint32_t writeupdate_addr = 0x8008000;
	uint32_t readbumber = 0;
	uint8_t ee_flag,err;
	#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
		CPU_SR		   cpu_sr;
	#endif	
	 OS_ENTER_CRITICAL();
	 memcpy(&update_info_tmp,updateinfo1,sizeof(updateInfoType));
	 update_info_tmp.update_readaddr = writeupdate_addr;
 	 update_info_tmp.update_writeaddr = writeupdate_addr + (updateinfo1->update_writeaddr - updateinfo1->update_readaddr);
	 UpdateParam1->PacketSize = 0;
	 UpdateParam1->CheckSum	= 0xFFFF;
	 readbumber = update_info_tmp.update_writeaddr - update_info_tmp.update_readaddr;
	 while(readbumber){
		 IWDG_ReloadCounter();
		 readbumber = update_info_tmp.update_writeaddr - update_info_tmp.update_readaddr;
		 if(readbumber >= CAN_WRITE_SIZE ){
			 memcpy(buf_1024_size,(uint8_t *)update_info_tmp.update_readaddr,CAN_WRITE_SIZE);
			 UpdateParam1->CheckSum = CRC16_CheckSum(buf_1024_size, CAN_WRITE_SIZE, UpdateParam1->CheckSum);
			 update_info_tmp.update_readaddr += CAN_WRITE_SIZE;
		 }else if(readbumber > 0){
			 memcpy(buf_1024_size,(uint8_t *)update_info_tmp.update_readaddr,readbumber);
			 UpdateParam1->CheckSum = CRC16_CheckSum(buf_1024_size, readbumber, UpdateParam1->CheckSum);
			 update_info_tmp.update_readaddr += readbumber;
		 }else{
			 if(dbgPrintf)(*dbgPrintf)("CHECK SUM:%04X,RCV_SUM:%04X\r\n",UpdateParam1->CheckSum,UpdateParam1->recieve_CheckSum);
		 	if(UpdateParam1->CheckSum == UpdateParam1->recieve_CheckSum){
				break;
		 	}else{
                updateinfo1->update_success_flag = 0;
                updateinfo1->upate_receive_flag  = 0;
				updateinfo1->begin_recover_time = 0;
				ee_flag = 0xBB;
                OSMutexPend(MutexFlash , 0 , &err);
                sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
                sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
                sFLASH_WriteBuffer((uint8_t *)updateinfo1, UPDATE_INFO_START_ADDR + 1 , sizeof(updateInfoType));
                OSMutexPost(MutexFlash);						
				softReset();
		 	}
		 }
	 }
	 OS_EXIT_CRITICAL();
	 return;
}

