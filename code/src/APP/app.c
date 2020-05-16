#include "bsp.h"
#include "Compress.h"
#include "time.h"
#include "stdlib.h"
/*
*测试中文<你好>!
*/
//#define TIME_TEST

static void   AppStartTask(void *p_arg);
static OS_STK AppStartTaskStk[APP_TASK_START_STK_SIZE];

static void   AppTaskKey  (void *p_arg);
static OS_STK AppTaskKeyStk  [APP_TASK_KEY_STK_SIZE];

static void   AppInfoTask  (void *p_arg);
static OS_STK AppInfoTaskStk[APP_TASK_INFO_STK_SIZE];

static void   AppTaskCanRx(void *p_arg);
static OS_STK AppTaskCanRxStk[APP_TASK_CANRX_STK_SIZE];

static void   AppTaskQuickPhoto(void *p_arg);
static OS_STK AppTaskQuickPhotoStk[APPTASKQUICKPHOTO_STK_SIZE];

static void   AppTaskU1Rx (void *p_arg);
static OS_STK AppTaskU1RxStk[APP_TASK_U1RX_STK_SIZE]; 

static void   AppTaskU2Rx (void *p_arg);
static OS_STK AppTaskU2RxStk[APP_TASK_U2RX_STK_SIZE];

static void   AppTaskU3Rx (void *p_arg);
static OS_STK AppTaskU3RxStk[APP_TASK_U3RX_STK_SIZE];

static void   AppTaskGPRS   (void *p_arg);
static OS_STK AppTaskGPRSStk[APP_TASK_GPRS_STK_SIZE];

static void   AppTaskCANUPDATE   (void *p_arg);
static OS_STK AppTaskCANUPDATEStk[APP_TASK_CANUPDATE_STK_SIZE];

int main(void)
{  
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);
    OSInit();
    OSTaskCreate((void (*) (void *)) AppStartTask,                              //指向任务代码的指针
                 (void *) 0,                                                    //任务开始执行时，传递给任务的参数的指针
                 (OS_STK *) &AppStartTaskStk[APP_TASK_START_STK_SIZE - 1] ,    //分配给任务的堆栈的栈顶指针   从顶向下递减
                 (INT8U) APP_TASK_START_PRIO);                                  //分配给任务的优先级
    OSStart();
    return(0);
}

/*******************************************************
* Function Name:    AppStartTask
* Purpose:          第一个任务，初始化硬件BSP，创建其它任务，进行LED控制
* Params :          无
* Return:           无
* Limitation: 	    注意检查堆栈大小
*******************************************************/
uint32_t f4_time = 0;
uint8_t check_time = 0;
uint32_t hyaline_time=0;//刚上电或复位DTU所有数据不记录
uint8_t take_photo_flag = 0;
uint16_t current_indoor_number;
#ifdef TIME_TEST
clock_t start_t,finish_t;
double duration;
#endif

uint8_t SIM_op	= 0;//0:正常，1重启模组；2、关闭模组


static void AppStartTask(void *p_arg)
{
    (void)p_arg;   
    uint8_t err , cnt = 0, i = 0;
	static uint8_t open_count = 0;
//	static uint8_t open_flag = 0;
    /* Initialize BSP functions*/
//    MutexAT1Send	= OSMutexCreate(11 , &err);
//    MutexAT2Send	= OSMutexCreate(10 , &err);
	SemAT1Send = OSSemCreate(1);
	SemAT2Send = OSSemCreate(1);
	SemHandleTxFrame = OSSemCreate(1);
//	SemAT1Buff = OSSemCreate(1);
//	SemAT2Buff = OSSemCreate(1);	
    MutexFlash= OSMutexCreate(4 , &err);
//    MutexSend1 = OSMutexCreate(5 , &err);
//    MutexSend3 = OSMutexCreate(6 , &err);
    MutexSend2 = OSMutexCreate(7 , &err);
    MutexSend4 = OSMutexCreate(8 , &err);
    MutexSendData = OSMutexCreate(9 , &err);
    
    BSP_Init();
		
#if OS_TASK_STAT_EN > 0
    OSStatInit();                                                        
#endif
	MSemExitWait = OSMboxCreate((void*)0);//用来控制拍照过程
    MSemQuickPhoto = OSMboxCreate((void*)0);//用来控制拍照传逻辑
    MSemSavePhoto = OSMboxCreate((void*)0);//用来控制拍照存逻辑
    MSemKEY   = OSMboxCreate((void*)0);                                         //创建按键消息邮箱
    MSemTCP   = OSMboxCreate((void*)0);  
    AT1.MSem  = OSMboxCreate((void*)0);
    AT2.MSem  = OSMboxCreate((void*)0);
    QSemCAN   = OSQCreate(&canRxPtrArr[0],CAN_MAX_BUF_LEN);                 	//创建CAN接收消息队列
	QSem_CAN_Tx = OSQCreate(&canTxPtrArr[0],CANTX_MAX_BUF_LEN);
    QSemUART1 = OSQCreate(&uartRxPtrArr1[0],UART1_QUEUE_LEN);                   //创建串口1接收消息队列
    QSemUART2 = OSQCreate(&uartRxPtrArr2[0],UART2_QUEUE_LEN);                   //创建串口2接收消息队列
    QSemUART3 = OSQCreate(&uartRxPtrArr3[0],UART3_QUEUE_LEN);                   //创建串口3接收消息队列
    QSemSend  = OSQCreate(&SendPtrArr[0],SEND_DATA_BUFF_SIZE);                                    //创建发送数据缓存接收消息队列

	QSend9E   = OSQCreate(&Send9EdSendPtrArr[0],QSEND9ELEN);					//创建9E帧发送队列。

	
#if 1
    OSTaskCreate((void (*) (void *)) AppTaskKey,                                // 指向任务代码的指针
                 (void *) 0,                                                    // 任务开始执行时，传递给任务的参数的指针
                 (OS_STK *) &AppTaskKeyStk[APP_TASK_KEY_STK_SIZE - 1],          // 分配给任务的堆栈的栈顶指针
                 (INT8U) APP_TASK_KEY_PRIO);                                    // 分配给任务的优先级 
#endif
#if 1
    OSTaskCreate((void (*) (void *)) AppTaskCanRx,                              //指向任务代码的指针
                 (void *)    0,                                                 //任务开始执行时，传递给任务的参数的指针
                 (OS_STK *) &AppTaskCanRxStk[APP_TASK_CANRX_STK_SIZE - 1],      //分配给任务的堆栈的栈顶指针
                 (INT8U)     APP_TASK_CANRX_PRIO);
#endif

#if 1
    OSTaskCreate((void (*) (void *)) AppTaskQuickPhoto,                              //指向任务代码的指针
                 (void *)    0,                                                 //任务开始执行时，传递给任务的参数的指针
                 (OS_STK *) &AppTaskQuickPhotoStk[APPTASKQUICKPHOTO_STK_SIZE - 1],      //分配给任务的堆栈的栈顶指针
                 (INT8U)     APPTASKQUICKPHOTO_PRIO);
#endif

#if 1
    OSTaskCreate((void (*) (void *)) AppTaskU1Rx,                               //指向任务代码的指针
                 (void *)    0,                                                 //任务开始执行时，传递给任务的参数的指针
                 (OS_STK *) &AppTaskU1RxStk[APP_TASK_U1RX_STK_SIZE - 1],        //分配给任务的堆栈的栈顶指针
                 (INT8U)     APP_TASK_U1RX_PRIO);  
#endif
#if 0
    OSTaskCreate((void (*) (void *)) AppTaskU2Rx,                               //指向任务代码的指针
                 (void *)    0,                                                 //任务开始执行时，传递给任务的参数的指针
                 (OS_STK *) &AppTaskU2RxStk[APP_TASK_U2RX_STK_SIZE - 1],        //分配给任务的堆栈的栈顶指针
                 (INT8U)     APP_TASK_U2RX_PRIO);  
#endif
#if 1
    OSTaskCreate((void (*) (void *)) AppTaskU3Rx,                               //指向任务代码的指针
                 (void *)    0,                                                 //任务开始执行时，传递给任务的参数的指针
                 (OS_STK *) &AppTaskU3RxStk[APP_TASK_U3RX_STK_SIZE - 1],        //分配给任务的堆栈的栈顶指针
                 (INT8U)     APP_TASK_U3RX_PRIO);
#endif
#if 1
    OSTaskCreate((void (*) (void *)) AppTaskGPRS,                               // 指向任务代码的指针
                 (void *) 0,                                                    // 任务开始执行时，传递给任务的参数的指针
                 (OS_STK *) &AppTaskGPRSStk[APP_TASK_GPRS_STK_SIZE - 1],        // 分配给任务的堆栈的栈顶指针
                 (INT8U) APP_TASK_GPRS_PRIO);                                   // 分配给任务的优先级 
#endif
#if 1
    OSTaskCreate((void (*) (void *)) AppTaskCANUPDATE,                               // 指向任务代码的指针
                 (void *) 0,                                                    // 任务开始执行时，传递给任务的参数的指针
                 (OS_STK *) &AppTaskCANUPDATEStk[APP_TASK_CANUPDATE_STK_SIZE - 1],        // 分配给任务的堆栈的栈顶指针
                 (INT8U) APP_TASK_CANUPDATE_PRIO);                                // 分配给任务的优先级 
#endif    
    diable_IWDG();
#ifdef TIME_TEST
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
		CPU_SR		   cpu_sr;
#endif
	CPU_CRITICAL_ENTER();	/* Tell uC/OS-II that we are starting an ISR			*/
	uint32_t t = 1700000;
	start_t = clock();
//	while(t--);
	finish_t = clock();
	CPU_CRITICAL_EXIT();
	duration = (double)(finish_t - start_t)/CLOCKS_PER_SEC;
	printf("%f, seconds\n",duration);
#endif	

    OSTaskSuspend (APP_TASK_START_PRIO);
    BSP_IWDG_Init();
    while (1) {  
        OSTimeDlyHMSM(0,0,0,100);
//		f4_time++;
//		check_f4time();
        IWDG_ReloadCounter();
		Time_count();
        displayLED(&LedValue);
        CheckPolarRev(&PolarCAN1 , CAN_REVNUM_PLUS);
        if((g_Upload.OutErr || g_Upload.InErr) && isGetTime){ //有外机或内机故障
                if(!g_Upload.ErrIsClr && (g_Upload.SendFlag == 0)){//故障上报传输完成且故障清除       
                g_Upload.ErrIsClr = 1;//存在故障
				if(dbgPrintf)(*dbgPrintf)("----------->ErrIsClr = 1<------------\r\n");
                
                g_Upload.SendFlag = 1; //置位发送标志位，准备进行故障传输
				if(dbgPrintf)(*dbgPrintf)("err g_Upload.SendFlag = 1\r\n");
                
                to_s_idle = 0;
                if(g_Upload.OutErr){   //如果是外机故障,外机故障优先
                    g_Upload.errPointTime = getUnixTimeStamp();                 //获取故障点时间
                    g_Upload.err_address = g_Upload.writeFlashAddr;//获取故障点地址
                    g_Upload.saveFlashTime= GprsParam.Out_Err_After_Time*60;   //继续向Flash写入5分钟数据
                    g_Upload.curTimeCnt   = 0;                                  //故障点计时器清零
                    var.outdoorErr = 1;
                    if(dbgPrintf)(*dbgPrintf)("OutErrPointAddress- - -> %X\r\n",g_Upload.err_address);
#if DEBUG
	getErrorTime(&error_time_tag,g_Upload.errPointTime);
    if(dbgPrintf)(*dbgPrintf)("-------<Error-time log: %04d/%02d/%02d %02d:%02d:%02d>-------\n" , 
                              error_time_tag.tm_year + 1900,
                              error_time_tag.tm_mon  + 1,
                              error_time_tag.tm_mday,
                              error_time_tag.tm_hour,
                              error_time_tag.tm_min,
                              error_time_tag.tm_sec);
#endif						
                }else if(g_Upload.InErr){                                       //如果是内机故障
                    g_Upload.errPointTime = getUnixTimeStamp();                 //获取故障点时间
                    g_Upload.err_address = g_Upload.writeFlashAddr;
                    g_Upload.saveFlashTime= GprsParam.In_Err_After_Time*60;    //继续向Flash写入5分钟数据        
                    g_Upload.curTimeCnt   = 0;                                  //故障点计时器清零
                    var.indoorErr = 1;
                    if(dbgPrintf)(*dbgPrintf)("InErrPointAddress- - -> %X\r\n",g_Upload.err_address);
#if DEBUG
						getErrorTime(&error_time_tag,g_Upload.errPointTime);
					
						if(dbgPrintf)(*dbgPrintf)("-------<Error-point log: %04d/%02d/%02d %02d:%02d:%02d>-------\n" , 
												  error_time_tag.tm_year + 1900,
												  error_time_tag.tm_mon  + 1,
												  error_time_tag.tm_mday,
												  error_time_tag.tm_hour,
												  error_time_tag.tm_min,
												  error_time_tag.tm_sec);
#endif	

					
                }
				
				savePowerDownInfo(&g_Upload);
				
            }
        }else if(!(g_Upload.OutErr || g_Upload.InErr)){//如果内机和外机故障都被清除
            g_Upload.ErrIsClr = 0; //故障标志清除
//			if(dbgPrintf)(*dbgPrintf)("----------->ErrIsClr = 0<------------\r\n");
            
        }
		static uint32_t f4_time_tmp = 0;
		if(f4_time_tmp != f4_time){
			f4_time_tmp = f4_time;
//        if(cnt++ >= 8){
            //进行秒计时
//            cnt = 0;
//			if(!open_flag){
//				open_count++;
//			}

//			if(open_count > 10){
//	大数据采样--->每分钟(时间可配置)进行一次采样
				if(!open_up_take_photo && isGetTime && !GprsParam.isOpenSample && !g_Upload.IndOff && hyaline_time > 120){
					bigDataSample(take_photo_mode);
					open_up_take_photo = 1;//运行时拍照标志
//					memcpy(&BigDataParm_pri,&BigDataParm,sizeof(BigDataParm));
				}

//#ifndef TEST_TEST_OVER
				if(isGetTime && !g_Upload.IndOff && !GprsParam.isOpenSample && g_Upload.flag_sample && open_up_take_photo){
					g_Upload.flag_sample = 0;
					g_Upload.Sample_9D_Count++;
					bigDataSample(de_duplicate_mode);
				}
//				open_flag = 1;
//			}
			
            for(i=0;i<INDOOR_MCH_NUM;i++){
                if(onlineintimer[i]++ > INDOOR_ONLINE_LAST_TIME){
                    onlineintimer[i] = INDOOR_ONLINE_LAST_TIME;
                    indoorinfo[i].online_flag = 0;//内机两分钟时长在线判断
                }  
            }
//机组数据定时打包存储			 
			if(packTime & 0x8000){//去重数据量少的情况下，60s发送或存储一次
                if((packTime ++ & 0x7FFF) >= DE_DUPLICATE_PACK_TIME){
                    packTime = 0;
                    timeOutPack(&g_Upload);
                }
            }

//机组实时监控数据打包发送			 
			if(real_packTime & 0x8000){//去重数据量少的情况下，60s发送或存储一次
                if((real_packTime ++ & 0x7FFF) >= REAL_DE_DUPLICATE_PACK_TIME){
                    real_packTime = 0;
                    timeOutRealSend(&RealBuffer_time_L30);
                }
            }			

//用户习惯数据定时存储
#ifdef USING_USER_HABIT
			if(user_pack_time & 0x8000){ 
			    if((user_pack_time++ & 0x7FFF) >= USER_HABIT_PACK_TIME){
			        user_pack_time = 0;
			        timeOutPackUser();
			    }
			}
#endif		
            if((check_time >= 50)&&(check_time%10 == 0)&&(!run_flag)){
                off_gprs(&LedValue);//非飞行模式下，信号太差
            }
            if(check_time == 100){
                check_time = 50;
            }
            if(run_flag)//增加信号弱的时候开启飞行模式的功能
            {
				if(dbgPrintf)(*dbgPrintf)("..........fly mode.......\r\n");				
                if(LedValue.Signal > 0){
#if DEBUG_FLY
					if(1){
#else			
                    if(!SendAT(&AT2,"AT+CFUN=4\r\n","OK",6)){
#endif						
                        config_run_flag_fail++;
                        if(config_run_flag_fail > 4 && !SIM_op){
                            close_sim_mode = 1;//重启SIM800方法：1，重启模组，2、指令配置模组
                            config_run_flag_fail = 0;
                        	stat_buffer.dtu_param_buff[DTU_ENTER_FLY_MODE]++;
							SIM_op = 2;
//                            close_sim800c();
							LedValue.Signal = 0;
//							OSTimeDlyHMSM(0,0,30,0);
                        }
                        
                    }else{
                        config_run_flag_fail = 0;
                        close_sim_mode = 0;
						LedValue.Signal = 0;
                        stat_buffer.dtu_param_buff[DTU_ENTER_FLY_MODE]++;
                    }					
                }
                run_time++;
                if(run_time >=3600)//1小时飞行模式后，主动关闭飞行模式
                {
					if(dbgPrintf)(*dbgPrintf)("Exit fly mode.......\r\n"); 			
					
                    if(close_sim_mode){
						SIM_op = 1;
//						restart_sim800c();//转移到info中						
                        close_sim_mode = 0;
                        run_time = 0;//配置成功
//                        run_flag = 0;//关闭飞行模式						
//                        restart_sim800c();
//						OSTimeDlyHMSM(0,0,30,0);
					
                    }else{
                        if(!SendAT(&AT1,"AT+CFUN=1\r\n","OK",6)){//重新配置模组
                           run_time  = 3550;//配置失败
                        }else{ 
//                            run_time = 0;//配置成功
//                            run_flag = 0;//关闭飞行模式
//    						restart_sim800c();
							SIM_op = 1;
							run_time = 0;//配置成功
//                            run_flag = 0;//关闭飞行模式
//							OSTimeDlyHMSM(0,0,30,0);
							if(dbgPrintf)(*dbgPrintf)("Exit fly mode.......\r\n"); 			
							
                        }					
                    }
                   resetnum=0;
                }
            }
            hyaline_time++;//上电或复位前3分钟计时
            check_time++;
//		    f4_time++;
			check_f4time();
			stat_buffer.time_buffer[GPRS_GETELEC_TIME]++;
            if(g_Upload.can_cnnct_ok){
            	Statistic_CanData();
            }
			if(isGetTime){
            	check_Err(&g_Upload);
			}
            if(GprsParam.MaitTime) GprsParam.MaitTime --;
            if((GprsParam.MaitTime % 3600) == 0){
                OSMutexPend(MutexFlash , 0 , &err);
                sFLASH_EraseSubSector(GPRS_INFO_START_ADDR);
                sFLASH_WriteBuffer((uint8_t *)&GprsParam , GPRS_INFO_START_ADDR , sizeof(GprsParam));
                OSMutexPost(MutexFlash);
            }
			
            if((g_Upload.SendFlag)&&(g_Upload.saveFlashTime)&&(g_Upload.saveFlashTime<1000)){    //故障上报触发重新做逻辑
                if(g_Upload.curTimeCnt < g_Upload.saveFlashTime){             //还未达到故障点后数据缓存时间
                      g_Upload.curTimeCnt ++;
                      //故障后时间计时
                  }else if(g_Upload.curTimeCnt == g_Upload.saveFlashTime){
                      g_Upload.readFlashAddr = g_Upload.writeFlashAddr;//故障数据最后1k
                      g_Upload.curTimeCnt ++;
                      savePowerDownInfo(&g_Upload);
					  if(dbgPrintf)(*dbgPrintf)("The last data adress: readFlashAddr :%X\r\n",g_Upload.readFlashAddr);
                  }else if((g_Upload.curTimeCnt >= g_Upload.saveFlashTime)){
                      if((!g_Upload.OutErr)&&(!g_Upload.InErr)&&(g_Upload.SendFlag == 1))//故障被解除，并且故障上报完成(var.Err 清零)，此时将传输完成标志置位
                      {//故障被解除
                          if((var.outdoorErr == 0)&&(var.indoorErr == 0)){//(var.Err 在数据上报完成后，会被清零)重新开始捕捉故障上报事件
                          		g_Upload.SendFlag = 0;
								if(dbgPrintf)(*dbgPrintf)(" data over g_Upload.SendFlag = 0\r\n");
								
                          }
                      }
                  }
              }else{
                  g_Upload.curTimeCnt = 0;                                      //没有进行故障上报触发时，计时器清零
              }
        }
    }
}


/*******************************************************
* Function Name:    AppTaskKey
* Purpose:          按键处理任务，查询方式进行按键的扫描,可反馈单击，双击，多击和长按
* Params :          无
* Return:           无
* Limitation: 	    注意检查堆栈大小
*******************************************************/
static void AppTaskKey(void *p_arg)
{
    (void)p_arg;
    uint8_t value = 0 , m_Time = 0, m_Cnt = 0 , f_KeyValue = 0;
    while(1){
        value = 0;
        while(BSP_PB_GetStatus(1)){                                             
            value ++;
            m_Time = 20;
            OSTimeDlyHMSM(0,0,0,50);
            if(value > 60){
                f_KeyValue = 0xFF;
				if(var.buttonPush){
                	var.buttonPush   = 0;				
				}else{
                	var.buttonPush   = 1;
				}
                OSMboxPost(MSemKEY,(void *)f_KeyValue);
                while(BSP_PB_GetStatus(1)){
                    LedValue.BlinkTm = 12;
                    LedValue.ConSta  = LED_STA_BLINK;
                    LedValue.ConSpd  = LED_BLINK_FST;
                    OSTimeDlyHMSM(0,0,0,50);
                }
                value  = m_Time = m_Cnt = 0;
                break;
            }
        }
        if(value > 0){
            m_Cnt ++;
        }
        if(m_Time > 0){
            m_Time --;
        }else if(m_Cnt != 0){
            f_KeyValue = m_Cnt;
            OSMboxPost(MSemKEY,(void *)f_KeyValue);
            m_Cnt = 0;
        }
        OSTimeDlyHMSM(0,0,0,20);
    }
}


/*******************************************************
* Function Name:    AppTaskCanRx
* Purpose:          CAN数据接收任务，进行CAN数据解析存入到ParamTable
* Params :          无
* Return:           无
* Limitation: 	    注意检查堆栈大小
*******************************************************/

static void AppTaskCanRx(void *p_arg)
{
    (void)p_arg;
//    uint8_t err , res;
	uint8_t err = 0,res = 0;
    CanRxMsg *s_Ptr = NULL;
    static CanMsgType  s_CanMsg ;
    memset(&ACParam , 0 , sizeof(ACParam));
    while(1){
        s_Ptr = (CanRxMsg *)OSQPend(QSemCAN,0,&err);
#if 1
        if(!err && s_Ptr){
            EE_flag =1;//调试完成状态下，机组由通讯变为故障时上报1帧EE帧
            g_Upload.can_cnnct_ok = 1;
            CheckPolarRev(&PolarCAN1 , CAN_REVNUM_CLR);
            convCanData(s_Ptr , &s_CanMsg); //s_Ptr->s_CanMsg
            current_indoor_number = GetData_can(s_CanMsg);//该数据内机编号
            if(isGetTime && GprsParam.MaitTime && !GprsParam.isOpenPower){
            	sample_power_data_frame(outdoor_power);
            }
            checkMachStatus(&s_CanMsg , &g_Upload);
            if((s_CanMsg.CAN1IP >= OUTDOOR_BASE_IP)&&(s_CanMsg.CAN1IP < (OUTDOOR_BASE_IP + OUTDOOR_MCH_NUM))){
                res = binarySearch(&PageData[0] , &s_CanMsg,ACParam.System,OUTDOOR_PARAM);
                if((!res || var.Hyaline || (hyaline_time<OPEN_BUFFER_TIME)) && isGetTime){
                    fillErrData(&s_CanMsg , &g_Upload);
					if(real_ret){
                    	fillRealData(&s_CanMsg , &RealBuffer_time_L30);
						real_ret = 0;
					}
					
                }
				if(isGetTime){
                	binarySearchData(&PageErrData[0] , &s_CanMsg  , ACErrParam.outdoor_lock_err);   //故障检测
				}
            }else if((s_CanMsg.CAN1IP >= INDOOR_BASE)&&(s_CanMsg.CAN1IP < (INDOOR_BASE + INDOOR_MCH_NUM))){
                res = binarySearch(&PageData[1] , &s_CanMsg ,ACParam.Indoor[current_indoor_num],INDOOR_PARAM);
                if((!res || var.Hyaline || (hyaline_time<OPEN_BUFFER_TIME))&& isGetTime){
                    fillErrData(&s_CanMsg , &g_Upload);
					if(real_ret){
                    	fillRealData(&s_CanMsg , &RealBuffer_time_L30);
						real_ret = 0;
					}					
                }
				if(isGetTime){
                	binarySearchData(&PageErrData[1] , &s_CanMsg  , ACErrParam.indoor_lock_err[current_indoor_num]);
				}

            }
        }else{
            if(dbgPrintf)(*dbgPrintf)("TaskCanRx QSemCAN Error!\r\n");
      }
#endif
   }
}

/*******************************************************
* Function Name:    AppTaskCanTx
* Purpose:          
* Params :          
* Return:           
* Limitation: 	   
*******************************************************/

static void AppTaskQuickPhoto(void *p_arg)
{
    (void)p_arg;
	uint8_t err = 0,res = 0;
    while(1){
		if(isGetTime){
//数据存储原则：每五分钟拍照存
			if(g_Upload.flag_5  	|| 	 g_Upload.flag_30){
				if(g_Upload.flag_30){
					g_Upload.flag_30 = 0;
					g_Upload.flag_5 = 0;
					g_Upload.per_5_min = 1;
					g_Upload.per_30_min = 1;
					take_photo_style = 0;
					if(var.buttonPush || var.projDebug){
						take_photo_style = 1;
					}
					if(dbgPrintf)(*dbgPrintf)("Now take a picture in a 30 minuter...\r\n");
				}else{
					g_Upload.flag_5 = 0;
					g_Upload.flag_30 = 0;
					g_Upload.per_5_min = 1;
					g_Upload.per_30_min = 0;
					take_photo_style = 0;
					if(dbgPrintf)(*dbgPrintf)("Now take a picture in a 5 minuter...\r\n");
				}
			}else{
				g_Upload.per_5_min = 0;
				g_Upload.per_30_min = 0;
			}
			
			if(var.realMonitor && ConSta.Status){
					if(!enter_state_monitor){
						g_Upload.enter_a_state = 1;//实时监控刚进入时拍照不发96帧
						enter_state_monitor = 1;
						g_Upload.per_30_min = 1;
						take_photo_style = 1;
						if(dbgPrintf)(*dbgPrintf)("\r\nNow take a picture in a realmonitor start...\r\n");
					}else{
						if(!g_Upload.trans_mode){
							if(g_Upload.per_30_min){
								g_Upload.per_30_min = 1;
								take_photo_style = 1;
								if(dbgPrintf)(*dbgPrintf)("Now take a picture in a per_30_min of realmonitor...\r\n");
							}else{
								g_Upload.per_30_min = 0;
								take_photo_style = 0;
							}
						}else{
								g_Upload.per_config_min = 0;
								take_photo_style = 0;
						}
					}
					
			}else if(g_Upload.OutErr || g_Upload.InErr){
					if(!enter_state){//拍照存逻辑
						g_Upload.enter_a_state = 1;//故障上报不影响
						enter_state = 1;
						g_Upload.per_30_min = 1;//拍照存+拍照传，进入状态点
						take_photo_style = 0;
					}
			}else{
				enter_state = 0;
				enter_state_monitor = 0;
				g_Upload.enter_a_state = 0;
			}
	

		if((var.buttonPush || var.projDebug) && ConSta.Status){
				if(!enter_state_button){//拍照存逻辑
					g_Upload.enter_a_state = 1;//按键上报和调试模式刚进入时拍照不发96帧
					enter_state_button = 1;
					g_Upload.per_30_min = 1;//拍照存+拍照传，进入状态点
					take_photo_style = 1;
				}
				
		}else{
			enter_state_button = 0;
		}

	//拍照传原则：
			if((g_Upload.per_5_min   || g_Upload.per_30_min || g_Upload.per_config_min ||\
				g_Upload.enter_a_state)){
				g_Upload.taking_photo_flag = 1;
				Quick_Photo();
				g_Upload.taking_photo_flag  = 0;
				g_Upload.enter_a_state = 0; 	
				
				res = (uint8_t)((uint32_t)OSMboxPend( MSemExitWait, 6000, &err ) - 0);
			}else{
				g_Upload.taking_photo_flag  = 0;
			}

	    }
			OSTimeDlyHMSM(0,0,0,50);
    }
}


/*******************************************************
* Function Name:    AppTaskU1Rx
* Purpose:          串口发送任务
* Params :          无
* Return:           无
* Limitation: 	    注意检查堆栈大小
*******************************************************/
static void AppTaskU1Rx(void *p_arg)
{
    (void)p_arg;
    uint8_t  err;
    UARTTYPE *s_Ptr = NULL;
    while(1)
    {
        s_Ptr = (UARTTYPE *)OSQPend(QSemUART1,0,&err);  
        if(!err && s_Ptr){
            // 处理服务器数据 
            if((s_Ptr->DataBuf[0]==0x7E)&&(s_Ptr->DataBuf[1]==0x7E)){
                if(handleRxFrame(&g_TcpType ,s_Ptr->DataBuf , (s_Ptr->DataLen & 0x7FFF) % UART_MAX_RX_LEN)){
                    tcpDataProcess(&g_TcpType);
                }else{
                    if(dbgPrintf)(*dbgPrintf)("TCP Frame Error!\r\n");
                }
                memset(g_TcpType.TxFrameArr , 0 , sizeof(g_TcpType.TxFrameArr));
            }else{
                memcpy(AT1.Buf , s_Ptr->DataBuf , (s_Ptr->DataLen&0x7FFF) % MAX_UARTTYPE_LEN);
                if(strstr((const char*)AT1.Buf,(const char*)"CLOSED")){
                    ConSta.Status = 0;
                }else if(strstr((const char*)AT1.Buf,(const char*)"CONNECT OK")){
                    ConSta.Status = 1;
                }
                OSMboxPost(AT1.MSem,(void *)AT1.Buf);
#ifdef TEST_PRINT_TEST				
#else
                if(dbgPrintf)(*dbgPrintf)("<U1><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                                          localTime.tm_year + 1900,
                                          localTime.tm_mon  + 1,
                                          localTime.tm_mday,
                                          localTime.tm_hour,
                                          localTime.tm_min,
                                          localTime.tm_sec,
                                          localTime.tm_wday);
#endif				
                if(dgbSendArr)(*dgbSendArr)(s_Ptr->DataBuf , (s_Ptr->DataLen & 0x7FFF) % MAX_UARTTYPE_LEN);
//                if(dbgPrintf)(*dbgPrintf)("*/");
				
                /* 处理AT指令 */
            }            
            memset(s_Ptr->DataBuf , 0 , s_Ptr->DataLen & 0x7FFF);
            s_Ptr->DataLen = 0;
        }else{
            if(dbgPrintf)(*dbgPrintf)("TaskUART1 QSemUART1 Error!\r\n");
        }     
    }
}   


/*******************************************************
* Function Name:    AppTaskU2Rx
* Purpose:          
* Params :          无
* Return:           无
* Limitation: 	    注意检查堆栈大小
*******************************************************/

static void AppTaskU2Rx(void *p_arg)
{
    (void)p_arg;
    uint8_t  err = 0;
    UARTTYPE *Ptr;
    while(1)
    {
        Ptr = (UARTTYPE*)OSQPend(QSemUART2,0,&err);                      //接收UART2数据
        if(!err && Ptr){
            //if(AT1.TxArr)(*AT1.TxArr)(Ptr->DataBuf , (Ptr->DataLen & 0x7FFF));
            catMyVariable(Ptr->DataBuf , (Ptr->DataLen & 0x7FFF) , &myScanf);
            memset(Ptr->DataBuf , 0 , (Ptr->DataLen & 0x7FFF));
            Ptr->DataLen = 0;
        }else{
            if(dbgPrintf)(*dbgPrintf)("TaskUART2 QSemUART2 Error!\r\n");
        }  
    } 
}

/*******************************************************
* Function Name:    AppTaskU3Rx
* Purpose:          
* Params :          无
* Return:           无
* Limitation: 	    注意检查堆栈大小
*******************************************************/
static void AppTaskU3Rx(void *p_arg)
{
    (void)p_arg;
    uint8_t  err;
    UARTTYPE *s_Ptr = NULL;
    uint16_t i = 0;
	uint16_t ret_frame = 0;
    while(1)
    {
        s_Ptr = (UARTTYPE *)OSQPend(QSemUART3,0,&err);  
        if(!err && s_Ptr){ 
            /* 处理服务器数据 */
            if((s_Ptr->DataBuf[0]==0x7E)&&(s_Ptr->DataBuf[1]==0x7E)){
	            if(handleRxFrame(&g_TcpType ,s_Ptr->DataBuf , (s_Ptr->DataLen & 0x7FFF)%UART_MAX_RX_LEN)){
	                tcpDataProcess(&g_TcpType);
	            #if DEBUG
				ret_frame = ((s_Ptr->DataLen & 0x7FFF)) % UART_MAX_RX_LEN;
	            for(i = 0;i < ret_frame; i++){
	            if(i == 0){ if(dbgPrintf)(*dbgPrintf)("RX---->");}
	            if(dbgPrintf)(*dbgPrintf)("%02X ",s_Ptr->DataBuf[i]);
				}
	        	if(dbgPrintf)(*dbgPrintf)("\r\n");
	            #endif
	            }else{
	                if(dbgPrintf)(*dbgPrintf)("TCP Frame Error!\r\n");
	            }
	            memset(g_TcpType.TxFrameArr , 0 , sizeof(g_TcpType.TxFrameArr));
            }else{
                ConSta.Mode = 0;
                memcpy(AT2.Buf, s_Ptr->DataBuf , (s_Ptr->DataLen&0x7FFF) % MAX_UARTTYPE_LEN);
                if(strstr((const char*)AT2.Buf,(const char*)"CLOSED")){
                    ConSta.Status = 0;
                }else if(strstr((const char*)AT2.Buf,(const char*)"CONNECT OK")){
                    ConSta.Status = 1;
                }
                OSMboxPost(AT2.MSem,(void *)AT2.Buf);
#ifdef TEST_PRINT_TEST
#else
                if(dbgPrintf)(*dbgPrintf)("<U3><%04d/%02d/%02d %02d:%02d:%02d [%d]>" , 
                                          localTime.tm_year + 1900,
                                          localTime.tm_mon  + 1,
                                          localTime.tm_mday,
                                          localTime.tm_hour,
                                          localTime.tm_min,
                                          localTime.tm_sec,
                                          localTime.tm_wday);
#endif				
                if(dgbSendArr)(*dgbSendArr)(s_Ptr->DataBuf , (s_Ptr->DataLen & 0x7FFF)%MAX_UARTTYPE_LEN);
//                if(dbgPrintf)(*dbgPrintf)("*/");
				
                /* 处理AT指令 */
            }            
            memset(s_Ptr->DataBuf , 0 , s_Ptr->DataLen & 0x7FFF);
            s_Ptr->DataLen = 0;
        }else{
            if(dbgPrintf)(*dbgPrintf)("TaskUART3 QSemUART3 Error!\r\n");
        }     
    }
}


#define CONFIG_CAN(o,a,b,c,d,s,t,u,v,w,x,y,z) \
        o.IDE      = CAN_ID_EXT;\
        o.DLC      = CAN_RTR_DATA;\
        o.FunCode  = a;\
        o.CAN2IP   = b;\
        o.CAN1IP   = c;\
        o.DataType = d;\
        o.Data[0]  = s;\
        o.Data[1]  = t;\
        o.Data[2]  = u;\
        o.Data[3]  = v;\
        o.Data[4]  = w;\
        o.Data[5]  = x;\
        o.Data[6]  = y;\
        o.Data[7]  = z;
//CONFIG_CAN(TxData,0xF7,0x7F,0x08,0x13,0x50,0x55,0x00,0x00,0x00,0x00,0x00,0x00);  
        

static void AppInfoTask(void *p_arg)
{
    (void)p_arg;
    char *p = NULL;
	uint8_t tmp_buf[SENDAT_BUFF_SIZE];//保护读取短信的内容
    while(1)
    {
        LedValue.Signal = getSignalVal((void *)0);
        if(dbgPrintf)(*dbgPrintf)("Signal is ---------------------------->%d\r\n",LedValue.Signal);
        if(!run_flag){                                                          //在准备打开飞行模式的时候不再检查注册情况
            gprsCheckReg(&GprsParam);
            checkNetStatus();
        }                                  
        /****************************************获取短信********************************************/
        SendAT(&AT1,"AT+CMGF=1\r\n","OK",4);                                    //设置短信模式为TXT
        p = SendAT(&AT1,"AT+CMGR=1\r\n","READ",SET_WAIT_TIME(10,tmp_buf));                             //读取第一条短信内容
        if( p ){
            if(parseSMS(p, &GprsParam, &mySMS)){
                smsCheck(&mySMS , mySMS.Auth , &GprsParam);
                memset( &mySMS , 0 ,sizeof(mySMS));
            }
            SendAT(&AT1,"AT+CMGD=1,4\r\n","OK",4);
        }
		if(SIM_op){
			if(SIM_op == 1){
				restart_sim800c();
			}else if(SIM_op == 2){
				close_sim800c();
			}
			SIM_op = 0;
			run_flag = 0;
		}

#ifdef weather
		writeWeatherToCAN(&a2data_to_GMV);
#endif
        OSTimeDlyHMSM(0,0,3,0); 
    }
}

static void AppTaskGPRS(void *p_arg)
{
    (void)p_arg;
//    uint32_t i = 0;
    uint8_t ee_flag,err;
//	uint32_t writeupdate_addr = 0x8008000;
//	uint32_t readbumber = 0;
//	updateInfoType update_info_tmp;//临时保存升级信息
//	#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
//		CPU_SR		   cpu_sr;
//	#endif
    gprsPowerUp();
    OSTaskResume (APP_TASK_START_PRIO);
    gprsCheckReg(&GprsParam);
	if(dbgPrintf)(*dbgPrintf)("Current Soft-version : %d\r\n",SOFTVERSION);
//	GetBaseStation(&GprsParam);
    while(!getIMEI(&GprsParam));
    while(!getICCID(&GprsParam));
    srand(OSTimeGet());
    f4_time = rand()%3600;
#if 1
    OSTaskCreate((void (*) (void *)) AppInfoTask,                               // 指向任务代码的指针
                 (void *) 0,                                                    // 任务开始执行时，传递给任务的参数的指针
                 (OS_STK *) &AppInfoTaskStk[APP_TASK_INFO_STK_SIZE - 1],        // 分配给任务的堆栈的栈顶指针
                 (INT8U) APP_TASK_INFO_PRIO);                                   // 分配给任务的优先级 /
#endif
    sFLASH_ReadBuffer((uint8_t *)&ee_flag, UPDATE_INFO_START_ADDR, sizeof(ee_flag)); 
    if(ee_flag == 0xBB){
        sFLASH_ReadBuffer((uint8_t *)&updateinfo, UPDATE_INFO_START_ADDR + 1, sizeof(updateinfo)); 
        sFLASH_ReadBuffer((uint8_t *)&UpdateParam, UPDATE_PARAM_START_ADDR, sizeof(UpdateParam));
        UpdateParam.PacketSize = 0;
        UpdateParam.CheckSum   = 0xFFFF;
    }
     if((updateinfo.update_success_flag == 0)||(ee_flag != 0xBB)){
	 	if(updateinfo.begin_recover_time > 0){
			check_packet_crc(&updateinfo,&UpdateParam,upgrade_buf);
//			 OS_ENTER_CRITICAL();
//	 		 memcpy(&update_info_tmp,&updateinfo,sizeof(updateinfo));
//			 update_info_tmp.update_readaddr = writeupdate_addr;
//		 	 update_info_tmp.update_writeaddr = writeupdate_addr + (updateinfo.update_writeaddr - updateinfo.update_readaddr);
//			 UpdateParam.PacketSize = 0;
//			 UpdateParam.CheckSum	= 0xFFFF;
//			 readbumber = update_info_tmp.update_writeaddr - update_info_tmp.update_readaddr;
//			 while(readbumber){
//				 IWDG_ReloadCounter();
//				 readbumber = update_info_tmp.update_writeaddr - update_info_tmp.update_readaddr;
//				 if(readbumber >= CAN_WRITE_SIZE ){
//					 memcpy(upgrade_buf,(uint8_t *)update_info_tmp.update_readaddr,CAN_WRITE_SIZE);
//					 UpdateParam.CheckSum = CRC16_CheckSum(upgrade_buf, CAN_WRITE_SIZE, UpdateParam.CheckSum);
//					 update_info_tmp.update_readaddr += CAN_WRITE_SIZE;
//				 }else if(readbumber > 0){
//					 memcpy(upgrade_buf,(uint8_t *)update_info_tmp.update_readaddr,readbumber);
//					 UpdateParam.CheckSum = CRC16_CheckSum(upgrade_buf, readbumber, UpdateParam.CheckSum);
//					 update_info_tmp.update_readaddr += readbumber;
//				 }else{
//				 	if(UpdateParam.CheckSum == UpdateParam.recieve_CheckSum){
//						break;
//				 	}else{
//	                    updateinfo.update_success_flag = 0;
//	                    updateinfo.upate_receive_flag  = 0;
//						updateinfo.begin_recover_time = 0;
//						ee_flag = 0xBB;
//	                    OSMutexPend(MutexFlash , 0 , &err);
//	                    sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
//	                    sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
//	                    sFLASH_WriteBuffer((uint8_t *)&updateinfo , UPDATE_INFO_START_ADDR + 1 , sizeof(updateinfo));
//	                    OSMutexPost(MutexFlash);						
//						softReset();
//				 	}
//				 }
//			 }
//			 OS_EXIT_CRITICAL();
	 	}
	 
	      updateinfo.update_success_flag = 1;
	      updateinfo.begin_recover_time = 5;     //重启五次后恢复旧程序，在写Flash之前断电。
	      if(!ConSta.Status){
		  	SendDataToServer(IDLE_MODE,s_idle);
//	          if(!connectToServer()){
//	              while(!disConnectToServer());
//	          }
	      }
		  
	      if(SOFTVERSION != updateinfo.updata_hisSOFTVERSION ){   //当前版本 ！= 历史版本号
	          updateinfo.updata_hisSOFTVERSION = SOFTVERSION;
			  updateinfo.upgrade_succ_count++;
		  	  if(dbgPrintf)(*dbgPrintf)("-------->Send update success!...<-----------\r\n");
	          if(!send97Frame(&g_TcpType, REQ_FEEDBACK, UPGRADE_OK)){
	              while(!disConnectToServer());
	          }
	          if(dbgPrintf)(*dbgPrintf)("SOFTVERSION = %d\r\n",SOFTVERSION);
	      }else{
		  	  updateinfo.upgrade_fail_count++;
		  	  if(dbgPrintf)(*dbgPrintf)("-------->Send update failure!...<-----------\r\n");
	          if(!send97Frame(&g_TcpType, REQ_FEEDBACK, UPGRADE_FAIL)){
	              while(!disConnectToServer());
	          }    
	          if(dbgPrintf)(*dbgPrintf)("SOFTVERSION = %d\r\n",SOFTVERSION);
	      }
		  
	      if(ConSta.Status){
	          while(!disConnectToServer());
	      }
		  
	      OSMutexPend(MutexFlash , 0 , &err);
	      ee_flag = 0xBB;
	      sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
	      sFLASH_EraseSubSector(UPDATE_PARAM_START_ADDR);
	      sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
	      sFLASH_WriteBuffer((uint8_t *)&updateinfo , UPDATE_INFO_START_ADDR + 1 , sizeof(updateinfo));
	      sFLASH_WriteBuffer((uint8_t *)&UpdateParam , UPDATE_PARAM_START_ADDR , sizeof(UpdateParam));
	      OSMutexPost(MutexFlash);
    }

    updateinfo.update_readaddr = PARAM_UPDATE_START_ADDR;
    updateinfo.update_writeaddr = PARAM_UPDATE_START_ADDR;
    while(1)
    {
        var.onlineTest = g_Upload.TestSig;
        var.projDebug  = g_Upload.Debug;
        if((GprsParam.MaitTime && !GprsParam.Pause)&&(!run_flag)){              //在飞行模式即将打开的时候不在进入状态机
            ModeStateMachine(&var , (void **)&f4_time); 
        }else if(ConSta.Status){
            while(!disConnectToServer());
        }
        OSTimeDlyHMSM(0,0,0,mode_time);
    }
    
}

static void AppTaskCANUPDATE(void *p_arg)
{
    CanTxMsg *s_Ptr = NULL;
    //static CanMsgType  s_CanMsg;
    uint8_t err = 0;
    while(1)
    {
#if 1
        s_Ptr = (CanTxMsg *)OSQPend(QSem_CAN_Tx,0,&err);
        if(s_Ptr){
            CAN_Transmit(CAN1 , s_Ptr);
        }else{
            if(dbgPrintf)(*dbgPrintf)("CAN TX get wrong data\n");
        }
#endif		
//      OSTimeDlyHMSM(0,0,1,500);
      
//    case m_register:
//        LED_all_onoroff         = 1;
//        update_mac.second_index = 0;
//        update_mac.first_index  = 0;
//        send_update_register();                                                                         //发送点名帧
//        if(num > 50){
//            updatestep = m_erase;
//        }
//        if(dbgPrintf)(*dbgPrintf)("send register to can /n/r");
//        OSTimeDlyHMSM(0,0,0,500);
//        num++;
//        break;
//        
//    case m_erase:
//        send_update_erase(&update_mac);                                                                //发送擦除命令
//        if(update_sendflag == 1){
//            updatestep = m_senddata;
//        }
//        OSTimeDlyHMSM(0,0,0,100);
//        if(dbgPrintf)(*dbgPrintf)("send erasecmd to can the firstindex is %d/n/r ", update_mac.first_index );
//        break;
//        
//    case m_senddata:
//        
//        if(updateinfo.update_readaddr < updateinfo.update_writeaddr){
//            temp = updateinfo.update_writeaddr - updateinfo.update_readaddr;
//            if(temp > 0x400){
//                updateinfo.update_readaddr = read_can_update(&sendcan_buffer, updateinfo.update_readaddr, 0x400);
//            }else{
//                updateinfo.update_readaddr = read_can_update(&sendcan_buffer, updateinfo.update_readaddr, (uint16_t)temp);
//            }
//            while((update_mac.second_index*8) < sendcan_buffer.DataLen){
//                temp = sendcan_buffer.DataLen - (update_mac.second_index*8);
//                if(temp > 8){
//                    update_mac.second_index = send_updateto_can(update_mac.first_index, update_mac.second_index, 8); 
//                }else{
//                    update_mac.second_index = send_updateto_can(update_mac.first_index, update_mac.second_index, (uint8_t)temp); 
//                }
//                OSTimeDlyHMSM(0,0,0,5);
//            }
//            update_sendflag = 0;
//            update_mac.second_index = 0;        
//            update_mac.first_index += 1;
// //           updatestep = m_erase;
//            OSTimeDlyHMSM(0,0,0,100);
//        }else{
//            updatestep = m_updatesuccess;
//        }
//        break;
//        
//    case m_updatesuccess:
//      send_update_finish();                                                                                 //发送烧写完成帧
//      re_ns           = s_idle;
//      var.s_update  = 0;                                                                                  //退出程序升级模式
//      LED_all_onoroff = 0;
//      updateinfo.upate_receive_flag = 0;                                                                   //升级完成标志清楚
//      OSMutexPend(MutexFlash , 0 , &err);
//      sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
//      ee_flag = 0xBB;
//      sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
//      sFLASH_WriteBuffer((uint8_t *)&updateinfo , UPDATE_INFO_START_ADDR + 1 , sizeof(updateinfo));
//      sFLASH_WriteBuffer((uint8_t *)&UpdateParam , UPDATE_INFO_START_ADDR + sizeof(updateinfo) + 1 , sizeof(UpdateParam));
//      OSMutexPost(MutexFlash);
//      updatestep = m_getdata;            
//      break;
    }
}
/******************* (C) COPYRIGHT 2015 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/






