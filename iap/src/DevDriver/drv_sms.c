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
        if(SendAT(&AT1,(void *)0x1A,"+CMGS",10)){                               //启动发送命令
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
#define ADMIN_ACCESS   1
#define USER_ACCESS    2
uint8_t g_Hyaline = 0;
uint8_t smsCheck(TypeSMS *sms , uint8_t auth , TypeGPRS *para)
{
    uint8_t err = 0;
    char Text[400];
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
        }else if(!strncasecmp(sms->cmd,"hb",2)){                                //*hb*heart,0,300*
            p = s =  sms->param;                                                //*123456*hb*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //heart参数查询
                sprintf(Text , "*hb*heart,0,%d" , para->HeartTime);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
                    if(str2dec(sms->param) != -1){
                        para->HeartTime = str2dec(sms->param);
                        sendSMS( sms->phone , "*hb*set ok*");                    //发送参数设置OK短信
                    }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));                                  //清空短信内容
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
        }else if(!strncasecmp(sms->cmd,"start",5)){ //*start*set ok*添加对m_Param的判断  add by zhh
            var.realMonitor = 1;
            if(!strncasecmp(sms->param,"1",1))    {
                var.Hyaline = 1;
                g_Hyaline = 1;
                sendSMS( sms->phone , "*start*set ok*");}
            else if(!strncasecmp(sms->param,"",1))  {
                var.Hyaline = 0;
                g_Hyaline = 0;
                sendSMS( sms->phone , "*start*set ok*");}
            if(0){                                                              //所有内机关机
                sendSMS( sms->phone , "All inside devices are closed!");
            }else{
//                sendSMS( sms->phone , "*start*set ok*");			                //发送响应信息
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if((!strncasecmp(sms->cmd,"stop",4))&&
                 (!strncasecmp(sms->param,"",1))){    			                //* stop *set ok*添加对m_Param的判断  add by zhh
            var.realMonitor = 0;
            to_s_idle = 1;
            sendSMS( sms->phone , "* stop *set ok*");			                //发送响应信息
            memset( sms , 0 ,sizeof(TypeSMS));	 				                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"ver",3)){     			                //*ver*v1.0.0*
            p = s =  sms->param;                                                //*123456*ver*100*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //ver参数查询
                sprintf(Text , "*ver*v1.4*");
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"adm",3)){     					        //*adm*1,13866668888*
            p = s =  sms->param;                                                //*123456*adm*1,13866668888*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //adm参数查询
                sprintf(Text , "*adm*1,%s;*adm*2,%s;*adm*3,%s;*adm*4,%s;*adm*5,%s;" ,\
                        para->Admin[0], para->Admin[1], para->Admin[2], para->Admin[3], para->Admin[4]);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
                    p = strstr(s , ",");                                        //查找第一个参数结束符
                    if(p){                                                      //找到第一个参数结束符
                        memcpy(Text , s , p - s);
                        if((str2dec(Text) > 0) && (str2dec(Text) < 6))
                        {
                            *(p + 12) = '\0';                                   //只取前11位号码
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
        }else if(!strncasecmp(sms->cmd,"rst",3)){     					        //*rst*set ok*
            p = s =  sms->param;
            memset( Text , 0 ,sizeof(Text));
            sendSMS( sms->phone , "*rst*set ok*");						        //发送响应的信息
            memset( sms , 0 ,sizeof(TypeSMS));
            softReset();
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"outerrt",7)){ 					        //*outerrt*30min*
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
                            para->OutErrTime = 120;
                        }
                        sendSMS( sms->phone , "* outerrt*set ok*");              //发送参数设置OK短信
                    }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"healt",5)){   					        //* healt *30min*
            p = s =  sms->param;                                                //*123456*healt*30*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //healt参数查询
                sprintf(Text , "*healt*%dmin*" , para->HeartTime);
                sendSMS( sms->phone , Text );						            //发送查询到的信息
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
                    if(str2dec(sms->param) != -1){
                        para->HeartTime = str2dec(sms->param);
                        sendSMS( sms->phone , "* healt *set ok*");               //发送参数设置OK短信
                    }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"butt",4)){    					        //* butt *240min*
            p = s =  sms->param;                                                //*123456*butt*240*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //butt参数查询
                sprintf(Text , "*butt*%dmin*" , para->ButtTime);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
                    if(str2dec(sms->param) != -1){
                        para->ButtTime = str2dec(sms->param);
                        sendSMS( sms->phone , "* butt *set ok*");                //发送参数设置OK短信
                    }
                }
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
            /*****************************************************************************************************************************/
        }else if(!strncasecmp(sms->cmd,"mait",4)){    					        //* mait *540day*
            p = s =  sms->param;                                                //*123456*mait*540*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //mait参数查询
                sprintf(Text , "*mait*%dDay*" , (uint32_t)(para->MaitTime/86400));
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }else{
                if(auth == 1){                                                  /*******管理员才能设置*******/
                    if((str2dec(sms->param) != -1)&&(str2dec(sms->param) <= 3650)){
                        para->MaitTime = str2dec(sms->param)*86400;
                        sendSMS( sms->phone , "* mait *set ok*");                //发送参数设置OK短信
                    }
                    else if((str2dec(sms->param) != -1)&&(str2dec(sms->param)>3650))
                    {
                        para->MaitTime = 3650*86400;
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
        }else if(!strncasecmp(sms->cmd,"signal",6)){  					      
            p = s =  sms->param;                                                //*123456*inerrt*5*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"?",1)){                                //inerrt参数查询
                sprintf(Text , "*signal*%d*" , LedValue.Signal);
                sendSMS( sms->phone , Text);                                     //发送查询到的信息
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
        }else if(!strncasecmp(sms->cmd,"ftpupdate",9)){  					       
            p = s =  sms->param;                                                //*123456*inerrt*5*
            memset( Text , 0 ,sizeof(Text));                                    //清空Text内容
            if(!strncasecmp(sms->param ,"1",1)){                                //
                var.fpt_update = 1;
                sendSMS( sms->phone , "*fpt*set ok*");				//发送响应的信息
            }
            memset( sms , 0 ,sizeof(TypeSMS));	                                //清空短信内容
        }
        /*****************************************************************************************************************************/
        OSMutexPend(MutexFlash , 0 , &err);
        sFLASH_EraseSubSector(GPRS_INFO_START_ADDR);
        sFLASH_EraseSubSector(VAR_START_ADDR);
        sFLASH_WriteBuffer((uint8_t *)&GprsParam , GPRS_INFO_START_ADDR , sizeof(GprsParam));
        sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
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
    if(strcmp(phone , "")){
        for(i = 0; i < 5; i++){
            if((!strncmp(phone , para->Admin[i],11))){
                return  1;
            }
        }
        for(i = 0; i < 10; i++){
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
/*【格力电器】*123456*start**【格力电器】
+CMGR: "REC UNREAD","18578292200","","15/12/08,00:28:09+32"  3010683C529B753556683011002A003100320033003400350036002A00730074006100720074002A002A3010683C529B753556683011    OK
*/
uint8_t parseSMS(char *smsStream , TypeGPRS *para ,TypeSMS *sms)
{
    char *s;
    uint8_t res , num;
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

