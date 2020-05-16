#include "common.h"
#include "drv_sms.h"
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
#include "statistic.h"
#include "bsp.h"
TypeSMS mySMS;

/*******************************************************
* Function Name: 	sendSMS
* Purpose: 		    发送一条短信到指定号码
* Params :          phone：要发送的手机号码
*                   text:  短信内容
* Return: 		    无
* Limitation: 	    短信内容只能为英文！！！
*******************************************************/
uint8_t sendSMS(char *phone , char *text)
{
    char buf[30];
    uint8_t cnt = 3;
    SendAT(&AT1,"AT+CMGD=1,4\r\n","OK",2);                                      //删除第一条短信内容                                           
    SendAT(&AT1,"AT+CMGF=1\r\n","OK",2);                                        //设置短信模式为 TXT模式                                
    sprintf(buf , "AT+CMGS=\"%s\"\r\n" , phone); 
    SendAT(&AT1,buf,">",2);                                                     //设置发送手机号码  
    SendAT(&AT1,text,">",2);                                                    //发送短信内容      
    while(cnt --){
        if(SendAT(&AT1,(void *)0x1A,"+CMGS",2)){                               //启动发送命令
            return 1;
        }
    }
    return 0;
}

/*******************************************************
* Function Name: 	smsCheck
* Purpose: 		    检查收到的短信内容
* Params :          msg：收到的短信；auth：管理员或普通用户帐号或无权限帐号
* Return: 		    无
* Limitation: 	    无
*******************************************************/
#define NONE_ACCSEE    0
#ifdef SMS_TEST
#define NONE_ACCSEE    0
#define ADMIN_ACCESS   1
#define USER_ACCESS    2
#endif
//uint8_t g_Hyaline = 0;
//uint8_t state_flag;
uint8_t smsCheck(TypeSMS *sms , uint8_t auth , TypeGPRS *para)
{
    uint8_t err = 0, ee_flag;
    char Text[400];
	char str[20];
    char temp[4];
    char temp1[5];
    char *p , *s;
    if(auth != NONE_ACCSEE){
        if(!strncasecmp(sms->cmd,"apn",3)){           					        //"*apn* gree.gd,gcha_Admin,1234*"
            p = s =  sms->param;								                //*123456*APN*gree.gd,gcha_Admin,1234*
            memset( Text , 0 ,sizeof(Text));						            //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){ 						        //APN参数查询
                sprintf(Text , "*apn*%s,%s,%s*" , para->ApnAccr , para->ApnUsr , para->ApnPwd);
                sendSMS( sms->phone , Text);						                //发送查询到的信息
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
//                    sscanf(s,"%*[^,],%[^,],%s",para->ApnAccr,para->ApnUsr);
                p = strstr(s , ",");					                    //查找第一个参数结束符
                    if(p){								                        //找到第一个参数结束符
                        memcpy(para->ApnAccr , s , p - s);
                        para->ApnAccr[p - s] = '\0';			                /****更新参数内容****/

                        s = strstr(++p , ",");					                //查找第二个参数结束符
                        if(s){							                        //找到第二个参数结束符
                            memcpy(para->ApnUsr , p , s - p);
                            para->ApnUsr[s - p] = '\0';	                        /****更新参数内容****/

                            sprintf(para->ApnPwd , "%s" , ++s);                 /****更新参数内容****/
                        }
                        sendSMS( sms->phone , "*apn*set ok*");	                //发送参数设置OK短信
                    }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));						            //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"serv",4)){    					    //*serv*support.gree.com.cn,5000*
            p = s =  sms->param;								                //*123456*serv*support.gree.com.cn,5000*
            memset( Text , 0 ,sizeof(Text));						            //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){ 						        //serv参数查询
                sprintf(Text , "*serv*%s,%d*" , para->SerName , para->SerPort );
                sendSMS( sms->phone , Text);						                //发送查询到的信息
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
                    p = strstr(s , ",");						                //查找第一个参数结束符
                    if(p){								                        //找到第一个参数结束符
                        memcpy(para->SerName , s , p - s);
                        para->SerName[p - s] = '\0';  		                    /****更新参数内容****/

                        if(str2dec(p + 1) != -1){
                            para->SerPort = str2dec(p + 1);                     /****更新参数内容****/
                        }
                        sendSMS( sms->phone , "*serv*set ok*");                  //发送参数设置OK短信
                    }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                 //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"pwd",3)){     					        //*pwd*123123*
            p = s =  sms->param;                                                //*123456*pwd*123123*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //pwd参数查询
                sprintf(Text , "*pwd*%s*" , para->SmsPwd);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }else{
                if(auth == 1){
                    strcpy(para->SmsPwd , sms->param);                          /*******管理员才能设置*******/
                    sendSMS( sms->phone , "*pwd*set ok*");                       //发送参数设置OK短信
                }
            }
           memset( sms , 0 ,sizeof(TypeSMS));	                                 //清空短信内容
           /*****************************************************************************************************************************/
        }
#ifdef SMS_TEST		
		else if(!strncasecmp(sms->cmd,"error",5)){     					        //*error*?*
            p = s =  sms->param;                                                //*123456*pwd*123123*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //pwd参数查询
                sprintf(Text , "*g_upload*DO:%d,SF:%d,EIC:%d,ION:%d,AFS:%d,WFC:%d,REOS:%d,IO:%d*",\
					g_Upload.DebugOver,g_Upload.SendFlag,g_Upload.ErrIsClr,g_Upload.indoor_off_noupflag,\
					g_Upload.alrd_find_start,g_Upload.write_flash_cross,g_Upload.read_empty_over_sector,g_Upload.IndOff);
                sendSMS(sms->phone , Text);                                     //发送查询到的信息
            }else{
                if(auth == 1){
					p = strstr(s,",");
					if(p){
						*p = '\0';
						g_Upload.SendFlag = str2dec(s) > 1 ? 1 : 0;
						g_Upload.ErrIsClr = str2dec(p+1) > 1 ? 1 : 0;
	                    sendSMS( sms->phone , "*error*set ok*");
					}
                }
            }
           memset( sms , 0 ,sizeof(TypeSMS));	                                 //清空短信内容
           /*****************************************************************************************************************************/
        }
#endif		
		   else if(!strncasecmp(sms->cmd,"start",5)){ //*start*set ok*添加对m_Param的判断  add by zhh
			if(!strncasecmp(sms->param,"?",1)){   /*增加*start*?*短信功能*/
				uint8_t state_flag = 0;
                if(var.realMonitor == 1){
				if(g_Upload.realup98flag == 1){
					if(var.Hyaline == 1){
						state_flag = 1;//stat_flag=1为强制透传模式
					}else{
						state_flag = 0;//stat_flag=0为非实时上报模式
					}
				}else if(var.realMonitor == 1){
						switch (g_Upload.real98flag)
							{
								case 0:
									state_flag = 2;
									break;
								case 1:
									state_flag = 3;
									break;
								case 2:
									state_flag = 4;
									break;
								case 3:
									state_flag = 5;
									break;
							}
				}
				sprintf(str,"*start*set %d*",state_flag);
				sendSMS(sms->phone,str);
            }else{
				sprintf(str,"*start*set %d*",state_flag);
				sendSMS(sms->phone,str);
            }
        }else{
	        	if(auth){
		            var.realMonitor = 1;
		            if(!strncasecmp(sms->param,"1",1)){//透传上报 add不用sj
		                var.Hyaline = 1;
		                g_Upload.realup98flag = 1;
		                sendSMS( sms->phone , "*start*set ok*");
		            }else if(!strncasecmp(sms->param,"2",1)){ //配置上报
		                var.Hyaline = 0;
		                g_Upload.real98flag = 0; 
		                g_Upload.realup98flag = 0;
		                g_Upload.indoor_off_noupflag = 0;   //内机全关标识默认为0
		                sendSMS( sms->phone , "*start*set ok*");
		            }else if(!strncasecmp(sms->param,"",1)){ //去重上报
		                var.Hyaline = 0;
		                g_Upload.realup98flag = 0;
		                sendSMS( sms->phone , "*start*set ok*");
		            }

					savePowerDownInfo(&g_Upload);
	        	}
	        }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if((!strncasecmp(sms->cmd,"stop",4)) &&  \
                 (!strncasecmp(sms->param,"",1))){    			                //* stop *set ok*添加对m_Param的判断  add by zhh
            var.realMonitor = 0;
			var.Hyaline = 0;
			g_Upload.real98flag = 0;			
			g_Upload.realup98flag = 0;
            to_s_idle = 1;
            sendSMS( sms->phone , "* stop *set ok*");			                //发送响应信息
            memset( sms , 0 ,sizeof(TypeSMS));
			savePowerDownInfo(&g_Upload);
			//清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"ver",3)){     			                //*ver*v1.0.0*
            p = s =  sms->param;                                                //*123456*ver*100*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //ver参数查询
                sprintf(Text , "*ver*%d*",(uint8_t)SOFTVERSION);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"adm",3)){     					        //*adm*1,13866668888*
            p = s =  sms->param;                                                //*123456*adm*1,13866668888*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //adm参数查询
                sprintf(Text , "*adm*1,%s;*adm*2,%s;*adm*3,%s;*adm*4,%s;*adm*5,%s;*adm*5,%s;" ,\
                        para->Admin[0], para->Admin[1], para->Admin[2], para->Admin[3], para->Admin[4],para->Admin[5]);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
                    p = strstr(s , ",");                                        //查找第一个参数结束符
                    if(p){                                                      //找到第一个参数结束符
                        memcpy(Text , s , p - s);
                        if((str2dec(Text) > 0) && (str2dec(Text) < 6))
                        {
                            *(p + 13) = '\0';                                   //只取前11位号码
                            memset(para->Admin[str2dec(Text) - 1],0,sizeof(para->Admin[str2dec(Text) - 1]));
                            strcpy( para->Admin[str2dec(Text) - 1] , p + 1 );	/****更新参数内容****/
                            sprintf(Text , "*adm*%d,set ok*" , str2dec(Text));
                            sendSMS( sms->phone , Text);                         //发送参数设置OK短信
                        }
                    }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"usron",5)){   					        //*usron*1,1065755507002610*
            p = s =  sms->param;                                                //*123456*usron*1,13866668888*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //gcha_Usron参数查询
                sprintf(Text , "*usron*1,%s;*usron*2,%s;*usron*3,%s;*usron*4,%s;*usron*5,%s;    \
                                *usron*6,%s;*usron*7,%s;*usron*8,%s;*usron*9,%s;*usron*10,%s;" ,\
                            para->Usron[0], para->Usron[1], para->Usron[2], para->Usron[3], para->Usron[4],
                            para->Usron[5], para->Usron[6], para->Usron[7], para->Usron[8], para->Usron[9]);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
                    p = strstr(s , ",");                                        //查找第一个参数结束符
                    if(p){                                                      //找到第一个参数结束符
                        memcpy(Text , s , p - s);
                        if((str2dec(Text) > 0) && (str2dec(Text) < 11))
                        {
                            *(p + 12) = '\0';                                   //只取前11位号码
                            strcpy( para->Usron[str2dec(Text) - 1] , p + 1 );	/****更新参数内容****/
                            sprintf(Text , "*usron*%d,set ok*" , str2dec(Text));
                            sendSMS( sms->phone , Text);                         //发送参数设置OK短信
                        }
                    }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"rst",3)){ //*rst*set ok*
            p = s =  sms->param;
            memset( Text , 0 ,sizeof(Text));
            sendSMS( sms->phone , "*rst*set ok*");						        //发送响应的信息
            memset( sms , 0 ,sizeof(TypeSMS));
            softReset();
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"f4time",6)){ //*rst*set ok*
            p = s =  sms->param;
            memset( Text , 0 ,sizeof(Text));
            if(!strncasecmp(sms->param ,"?",1)){
                sprintf(Text ,"*f4_time*%d's*",f4_time);
                sendSMS(sms->phone,Text);
            }else{
                if(auth){
                    f4_time = (uint32_t)str2dec_32(sms->param);
                    sendSMS( sms->phone , "*f4_time*set ok*");
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));
            /*****************************************************************************************************************************/
         }
#ifdef SMS_TEST		
			else if(!strncasecmp(sms->cmd,"erase",5)){     					        //*rst*set ok*
			if(auth == 1){
				p = s = sms->param;
				memset(Text,0,sizeof(Text));
				if(!strncasecmp(sms->param,"1",1)){
					g_Upload.erase_runparam_flash = 1;
					to_s_idle = 1;
				}else if(!strncasecmp(sms->param,"2",1)){
					g_Upload.erase_error_real = 1;
//					to_s_idle = 1;
				}
				sendSMS(sms->phone,"*erase*set ok*");
			}
			memset( sms , 0 ,sizeof(TypeSMS));								   //清空短信内容
        }
#endif			
		else if(!strncasecmp(sms->cmd,"outerrt",7)){ 					        //*outerrt*30min*
            p = s =  sms->param;                                                //*123456*outerrt*30*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //outerrt参数查询
                sprintf(Text , "*outerrt*%dmin*" , para->OutErrTime);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }else{
                if(auth == 1){
                    if(str2dec(sms->param) != -1){                              /*******管理员才能设置*******/
                        para->OutErrTime = str2dec(sms->param);
                        if(para->OutErrTime >= 120){
                            para->OutErrTime = 120;//不能大于2小时
                        }
                        sendSMS( sms->phone , "* outerrt*set ok*");              //发送参数设置OK短信
                    }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"butt",4)){    					        //* butt *240min*
            p = s =  sms->param;                                                //*123456*butt*240*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //butt参数查询
                sprintf(Text , "*butt* %d min*" , para->ButtTime);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }else{	
                if(auth == 1){                                                  /*******管理员才能设置*******/
                    if((str2dec(sms->param) != -1) && (str2dec(sms->param) != 0)){
							para->ButtTime = (uint16_t)str2dec(sms->param);
							sendSMS( sms->phone , "* butt *set ok*");				 //发送参数设置OK短信
						}
                    }
	            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"mait",4)){    					        //* mait *540day*
            p = s =  sms->param;                                                //*123456*mait*540*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //mait参数查询
                sprintf(Text , "*mait* %d Day*" , (uint32_t)(para->MaitTime/86400));
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
		            if((str2dec(sms->param) != -1)&&(((uint16_t)str2dec(sms->param)) <= 3650)){
		                        para->MaitTime = ((uint16_t)str2dec(sms->param))*86400;
		                        sendSMS( sms->phone , "* mait *set ok*");                //发送参数设置OK短信
		                }else if((str2dec(sms->param) != -1)&&((uint16_t)str2dec(sms->param)>3650)){
		                        para->MaitTime = 3650*86400u;
		                        sendSMS( sms->phone , "* mait *set ok*");                //发送参数设置OK短信
	                    }
	                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"pause",5)){   					        //* g_Pause* True *
            p = s =  sms->param;                                                //*123456* g_Pause *0*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //g_Pause参数查询
                if(para->Pause){
                    sendSMS( sms->phone , "* pause* True *");                    //发送查询到的信息
                }else{
                    sendSMS( sms->phone , "* pause* False *");                   //发送查询到的信息
                }
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
                    if(!strncasecmp(sms->param ,"1",1)){
                        para->Pause = 1;
                        sendSMS( sms->phone , "* pause *set ok*");               //发送参数设置OK短信
                    }else if(!strncasecmp(sms->param ,"0",1)){
                        para->Pause = 0;
                        sendSMS( sms->phone , "* pause *set ok*");               //发送参数设置OK短信
                    }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"inerrt",6)){  					        //*inerrt*5min*
            p = s =  sms->param;                                                //*123456*inerrt*5*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //inerrt参数查询
                sprintf(Text , "*inerrt*%dmin*" , para->InerrtTime);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
                    if(str2dec(sms->param) != -1){
                        para->InerrtTime = str2dec(sms->param);
                        if(para->InerrtTime >= 120){
                            para->InerrtTime = 120;
                        }
                        sendSMS( sms->phone , "* inerrt *set ok*");              //发送参数设置OK短信
                    }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
        }else if(!strncasecmp(sms->cmd,"signal",6)){  					        //*inerrt*5min*
            p = s =  sms->param;                                                //*123456*inerrt*5*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //inerrt参数查询
                sprintf(Text , "*signal*%d*" , LedValue.Signal);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
        }else if(!strncasecmp(sms->cmd,"update",6)){
            if(auth == 1){
              p = s =  sms->param;                                              
                memset( Text , 0 ,sizeof(Text));                                 
                memset(temp, 0 ,sizeof(temp));
				memset(temp1,0,sizeof(temp1));
                sscanf(sms->param , "%[^,],%[^,],%s",  UpdateParam.FilePath, temp,temp1);
                UpdateParam.recieve_CheckSum = str2hex(temp1);                                 //将16进制的字符串校验码，转换为16进制
                updateinfo.update_pack_type = str2dec(temp);
		        sendSMS( sms->phone , "*update*set ok*");			   //发送响应的信息
                var.s_update = 1;//
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                  //清空短信内容
        }else if(!strncasecmp(sms->cmd,"recoverold",10)){                         //
            if(auth == 1){ 
                if(updateinfo.recover_have_packet!=0){                            //恢复上一版本程序
                    p = s =  sms->param;
                    updateinfo.update_success_flag = 0;
                    updateinfo.upate_receive_flag  = 0;
					updateinfo.begin_recover_time = 0;
					ee_flag = 0xBB;//revise by running
                    OSMutexPend(MutexFlash , 0 , &err);
                    sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
                    sFLASH_EraseSubSector(UPDATE_PARAM_START_ADDR);
                    sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
                    sFLASH_WriteBuffer((uint8_t *)&updateinfo , UPDATE_INFO_START_ADDR + 1 , sizeof(updateinfo));
                    sFLASH_WriteBuffer((uint8_t *)&UpdateParam ,UPDATE_PARAM_START_ADDR , sizeof(UpdateParam));
                    OSMutexPost(MutexFlash);
                    memset( Text , 0 ,sizeof(Text));
                    sendSMS( sms->phone , "*recoverold*set ok*");						        //发送响应的信息
                    memset( sms , 0 ,sizeof(TypeSMS));
                    softReset();
                }else{
                    memset( Text , 0 ,sizeof(Text));
                    sendSMS( sms->phone , "*have no old packet*");						        //发送响应的信息
                    memset( sms , 0 ,sizeof(TypeSMS));
                }
            }
        }else if(!strncasecmp(sms->cmd,"f5time",6)){                         //更改f4时间间隔 add by sj
             p = s =  sms->param;                                            //*123456*ftpuser*用户名，密码*
             memset( Text , 0 ,sizeof(Text));                                //清空Text内容
             if(!strncasecmp(sms->param ,"?",1)){                            //inerrt参数查询   
                sprintf(Text , "*f5time*%d.%d*" , stat_buffer.usr_hbt_time,stat_buffer.stat_data_time);
                sendSMS( sms->phone , Text);                                 //发送查询到的信息
            }else{                                       
                if(auth == 1){//DTU自身模块打卡时间配置，和机组统计时间配置 短信指令下发格式: *12345*f5time*24.720* ,兼容以往单参数格式
					char *f5_tmp = strstr(sms->param,".");
					if(f5_tmp){
						*f5_tmp = '\0';
						if((str2dec(sms->param) != -1) && (((uint16_t)str2dec(sms->param)) != 0) && ((str2dec(f5_tmp+1) != -1)) && (((uint16_t)str2dec(f5_tmp+1)) != 0)){
							stat_buffer.usr_hbt_time = str2dec(sms->param);
							stat_buffer.stat_data_time = str2dec(f5_tmp+1);
							sendSMS( sms->phone, "*f5time*set all ok*");
						}
					}else{
	                    if(str2dec(sms->param) != -1){                              /*******管理员才能设置*******/
	                        stat_buffer.usr_hbt_time = str2dec(sms->param);
	                        sendSMS( sms->phone , "*f5time*set ok*");              //发送参数设置OK短信
	                    }
					}
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                               //清空短信内容
        }else if(!strncasecmp(sms->cmd,"poweracc",8)){//power accurency
             p = s =  sms->param;                                            //*123456*ftpuser*用户名，密码*
             memset( Text , 0 ,sizeof(Text));                                //清空Text内容
             if(!strncasecmp(sms->param ,"?",1)){
	                sprintf(Text , "*power accurancy*%d*",GprsParam.outdoor_power_accuracy);
	                sendSMS( sms->phone , Text);				
            }else{                                       
                if(auth == 1){//DTU自身模块打卡时间配置，和机组统计时间配置 短信指令下发格式: *12345*f5time*24.720* ,兼容以往单参数格式
					if(str2dec(sms->param) != -1){
						GprsParam.outdoor_power_accuracy = str2dec(sms->param);
						sendSMS( sms->phone , "*samplepower*set ok*");			   //发送参数设置OK短信
					}else{
						sendSMS( sms->phone , "*samplepower*set failure*");//发送参数设置OK短信
					}
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                               //清空短信内容
        }else if(!strncasecmp(sms->cmd,"powerOS",7)){//sample O:open S: shutdown
             p = s =  sms->param;                                            //*123456*ftpuser*用户名，密码*
             memset( Text , 0 ,sizeof(Text));                                //清空Text内容
             if(!strncasecmp(sms->param ,"?",1)){
			 	if(GprsParam.isOpenPower){
	                sprintf(Text , "*powerOS*close*");
	                sendSMS( sms->phone , Text);                                 //发送查询到的信息
			 	}else{
	                sprintf(Text , "*powerOS*open*");
	                sendSMS( sms->phone , Text);  					
			 	}
            }else{                                       
                if(auth == 1){//DTU自身模块打卡时间配置，和机组统计时间配置 短信指令下发格式: *12345*f5time*24.720* ,兼容以往单参数格式
					if(str2dec(sms->param) != -1){
						GprsParam.isOpenPower = str2dec(sms->param);
						sendSMS( sms->phone , "*powerOS*set ok*");			   //发送参数设置OK短信
					}else{
						sendSMS( sms->phone , "*powerOS*set failure*");//发送参数设置OK短信
					}
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                               //清空短信内容
        }else if(!strncasecmp(sms->cmd,"sampleOS",8)){//sample O:open S: shutdown
             p = s =  sms->param;                                            //*123456*ftpuser*用户名，密码*
             memset( Text , 0 ,sizeof(Text));                                //清空Text内容
             if(!strncasecmp(sms->param ,"?",1)){
			 	if(GprsParam.isOpenSample){
	                sprintf(Text , "*sampleOS*close*");
	                sendSMS( sms->phone , Text);                                 //发送查询到的信息
			 	}else{
	                sprintf(Text , "*sampleOS*open*");
	                sendSMS( sms->phone , Text);  					
			 	}
            }else{                                       
                if(auth == 1){//DTU自身模块打卡时间配置，和机组统计时间配置 短信指令下发格式: *12345*f5time*24.720* ,兼容以往单参数格式
					if(str2dec(sms->param) != -1){
						GprsParam.isOpenSample = str2dec(sms->param);
						sendSMS( sms->phone , "*sampleOS*set ok*");			   //发送参数设置OK短信
					}else{
						sendSMS( sms->phone , "*sampleOS*set failure*");//发送参数设置OK短信
					}
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                               //清空短信内容
        }else if(!strncasecmp(sms->cmd,"isample",7)){//indoor sample 内机全样本采集精度配置
			 char *bigdata_indoor = (char*)0;			
             p = s =  sms->param;
			 uint8_t count_indoor = 0;
             memset( Text , 0 ,sizeof(Text));                                //清空Text内容
             if(!strncasecmp(sms->param ,"?",1)){                            //inerrt参数查询   
                sprintf(Text , "*IsampleConfig*%d's.%d'B.%d'rpm.%d'C.*",GprsParam.big_data_sampe_time,GprsParam.bigdata_reduce_indoor_B,\
					GprsParam.bigdata_reduce_indoor_rpm,GprsParam.bigdata_reduce_indoor_C);
                sendSMS( sms->phone , Text);                                 //发送查询到的信息
            }else{                                       
                if(auth == 1){//DTU自身模块打卡时间配置，和机组统计时间配置 短信指令下发格式: *12345*f5time*24.720* ,兼容以往单参数格式
				bigdata_indoor = sms->param;
                while(bigdata_indoor){
						p = strstr(bigdata_indoor+1,".");
						if(p){
							*p = '\0';
							if((str2dec(bigdata_indoor) != -1) && (((uint16_t)str2dec(bigdata_indoor)) != 0)){
								switch (count_indoor)
									{
										case 0:
											GprsParam.big_data_sampe_time = str2dec(bigdata_indoor);
											break;
										case 1:
											GprsParam.bigdata_reduce_indoor_B = str2dec(bigdata_indoor);
											break;
										case 2:
											GprsParam.bigdata_reduce_indoor_rpm = str2dec(bigdata_indoor);
											break;
										case 3:
											GprsParam.bigdata_reduce_indoor_C = str2dec(bigdata_indoor);
											break;
										default:
											break;
									}
									bigdata_indoor = p+1;
									count_indoor++;
							}else{
								sendSMS( sms->phone , "*isample*error usage*"); 			 //发送参数设置OK短信
								break;
							}
						}else{
								sendSMS( sms->phone, "*isample*set ok*");
								break;
						}
	                }
                }
            }

            memset( sms , 0 ,sizeof(TypeSMS));	                               //清空短信内容
        }else if(!strncasecmp(sms->cmd,"osample",7)){//outdoor sample 外机全样本采集精度配置
			 char *bigdata = (char*)0;			
             p = s =  sms->param;
			 uint8_t count = 0;
             memset( Text , 0 ,sizeof(Text));                                //清空Text内容
             if(!strncasecmp(sms->param ,"?",1)){                            //inerrt参数查询   
                sprintf(Text , "*OsampleConfig*%d's.%d'V.%d'Hz.%d'100W.%d'B.%d'kPa.%d'10W.%d'C.%d'A*",GprsParam.big_data_sampe_time,GprsParam.bigdata_reduce_outdoor_V,\
					GprsParam.bigdata_reduce_outdoor_Hz,GprsParam.bigdata_reduce_outdoor_100W,GprsParam.bigdata_reduce_outdoor_B,GprsParam.bigdata_reduce_outdoor_kPa,\
					GprsParam.bigdata_reduce_outdoor_10W,GprsParam.bigdata_reduce_outdoor_C,GprsParam.bigdata_reduce_outdoor_A);
                sendSMS( sms->phone , Text);                                 //发送查询到的信息
            }else{                                       
                if(auth == 1){//DTU自身模块打卡时间配置，和机组统计时间配置 短信指令下发格式: *12345*f5time*24.720* ,兼容以往单参数格式
				bigdata = sms->param;
                while(bigdata){
						p = strstr(bigdata+1,".");
						if(p){
							*p = '\0';
							if((str2dec(bigdata) != -1) && (((uint16_t)str2dec(bigdata)) != 0)){
								switch (count)
									{
										case 0:
											GprsParam.big_data_sampe_time = str2dec(bigdata);
											break;
										case 1:
											GprsParam.bigdata_reduce_outdoor_V = str2dec(bigdata);
											break;
										case 2:
											GprsParam.bigdata_reduce_outdoor_Hz = str2dec(bigdata);
											break;
										case 3:
											GprsParam.bigdata_reduce_outdoor_100W = str2dec(bigdata);
											break;
										case 4:
											GprsParam.bigdata_reduce_outdoor_B = str2dec(bigdata);
											break;
										case 5:
											GprsParam.bigdata_reduce_outdoor_kPa = str2dec(bigdata);
											break;
										case 6:
											GprsParam.bigdata_reduce_outdoor_10W = str2dec(bigdata);
											break;
										case 7:
											GprsParam.bigdata_reduce_outdoor_C = str2dec(bigdata);
											break;
										case 8:
											GprsParam.bigdata_reduce_outdoor_A = str2dec(bigdata);
											break;								
										default:
											break;
									}
									bigdata = p+1;
									count++;
							}else{
								sendSMS( sms->phone , "*osample*error usage*"); 			 //发送参数设置OK短信
								break;
							}
						}else{
								sendSMS( sms->phone, "*osample*set ok*");
								break;
						}
	                }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	            
        }
        /*****************************************************************************************************************************/
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_EraseSubSector(GPRS_INFO_START_ADDR);
        sFLASH_EraseSubSector(VAR_START_ADDR);
        sFLASH_WriteBuffer((uint8_t *)&GprsParam , GPRS_INFO_START_ADDR , sizeof(GprsParam));
        sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
        sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
        ee_flag = 0xBB;
        sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
        sFLASH_EraseSubSector(UPDATE_PARAM_START_ADDR);
        sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
        sFLASH_WriteBuffer((uint8_t *)&updateinfo , UPDATE_INFO_START_ADDR + 1 , sizeof(updateinfo));
        sFLASH_WriteBuffer((uint8_t *)&UpdateParam ,UPDATE_PARAM_START_ADDR , sizeof(UpdateParam));
        OSMutexPost(MutexFlash);
    }
    return 0;
}


/*******************************************************
* Function Name: 	gprsCheckPhone
* Purpose: 		    检查收到的短信内容
* Params :          m_phone：要检查的手机号码
* Return: 		    返回的手机号码权限：0：无权限；1：管理员；2：普通账户
* Limitation: 	    无
*******************************************************/
uint8_t sms_flg;
uint8_t checkPhoneAuth(char *phone , TypeGPRS *para)
{
    uint8_t i = 0;
    if(strcmp(phone , "")){//strcmp '\0' return 1
        for(i = 0; i < ADMIN_MAX_NUM; i++){
            if((!strncmp(phone , para->Admin[i],11))){
                return  1;
            }
        }
        for(i = 0; i < USER_MAX_NUM; i++){
            if((!strncmp(phone , para->Usron[i],11))){
                return  2;
            }
        }
        return  0;
    }else{
        return  0;
    }
}


/*******************************************************
* Function Name: 	parseSMS
* Purpose: 		    读取一条短信内容
* Params :          *sms：原始的AT短信内容
*                   *gcha_PassWord:  短信指令的密码
*                   *msg：解完的短信内容
* Return: 		    是否为指定短信指令
* Limitation: 	    无
*******************************************************/
/*
*在此处分为手机短信和短信接口软件模式，两者区别为手机为ascii码形式(单字节),和短信接口模式(双字节)
****************************************************************************************************************************************
*短信接口回帧形式：
*+CMGR: "REC UNREAD","1069800006512610","","18/05/09,17:16:41+32"  002A003100320033003400350036002A007500700064006100740065002A0068
*007400740070003A002F002F003100390032002E00310033002E003100380032002E003100350036003A0037003000300034002F0075007000670072006100640065
*002F00530054004D00330032002E00620069006E002C0032002C0042003900380031002A    OK
*
****************************************************************************************************************************************
*手机短信回帧形式：
*+CMGR: "REC UNREAD","18676489371","","18/05/09,17:42:42+32"  *123456*update*http://192.13.182.156:7004/upgrade/STM32.bin,2,B981*    OK
****************************************************************************************************************************************
*/
uint8_t parseSMS(char *smsStream , TypeGPRS *para ,TypeSMS *sms)
{
    char *s;
    uint8_t res;
	uint16_t num;
    s = strstr(smsStream, "READ");
    if( s ){
        res = sscanf(smsStream , "%*[^,],\"%[^\"]%*[^\r\n]\r\n%[^\r\n]"  , sms->phone ,sms->text);
        strcpy(smsStream , sms->text);
        if(res == 2){
            sms->Auth = checkPhoneAuth(sms->phone , para);
            if(sms->Auth){
                s = strstr(smsStream , "002A");
                if(s){
                    num = 0;
                    do{
                        if(((num++ % 4) == 0) && ( *s == '0') && (*(s + 1)=='0')){
                            sms->text[num/4] = chr2hex(*(s + 2))*16 + chr2hex(*(s + 3));
                            sms->text[num/4 + 1] = '\0';
                        }
                    }while( *(++s) != '\0' );
                }
                sprintf(sms->pwd,"*%s*",para->SmsPwd);
                s = strstr(sms->text, sms->pwd);
                if(s){
                    sscanf(s , "*%*[^*]*%[^*]*%[^*]*" , sms->cmd , sms->param);
                    return 1;
                }else{
                    memset(sms , 0 ,sizeof(TypeSMS));
                    return 0;
                }
            }else{
                memset(sms , 0 ,sizeof(TypeSMS));
                return 0;
            }
        }else{
            memset(sms , 0 ,sizeof(TypeSMS));
            return 0;
        }
    }else{
        memset(sms , 0 ,sizeof(TypeSMS));
        return 0;
    }
}

