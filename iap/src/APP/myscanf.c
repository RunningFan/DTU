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
#include "duplicate.h"
#include "statistic.h"
#include "myscanf.h"


SCANF_TYPE myScanf;

uint32_t catMyVariable(uint8_t *pData , uint16_t dataLen , SCANF_TYPE *sScanf)
{
    uint8_t res = 0;
    if(!pData || !dataLen){
        return 0;
    }else{
        pData[dataLen + 1] = '\0';
        strcpy(sScanf->text , (char *)pData);
    }
    //sscanf(s , "*%*[^*]*%[^*]*%[^*]*" , sms->cmd , sms->param);
    res = sscanf(sScanf->text , "%[^+]+%[^=]=%s" ,sScanf->identify , sScanf->comand , sScanf->contant);
    if(res == 3){
        /* 当前采用字符串匹配方式处理，后续进行hash提高程序执行效率 */
        if(!strncmp(sScanf->identify , "at" , strlen("at"))){
            if(AT1.TxArr)(*AT1.TxStr)(sScanf->text);
        }else if(!strncmp(sScanf->identify , "cat" , strlen("cat"))){
            if(!strncmp(sScanf->comand , "gprs" , strlen("gprs"))){
                if(!strncmp(sScanf->contant , "pause" , strlen("pause"))){
                    if(dbgPrintf)(*dbgPrintf)("<pause> --> [%d]\r\n" , GprsParam.Pause);
                }else if(!strncmp(sScanf->contant , "version" , strlen("version"))){
                    if(dbgPrintf)(*dbgPrintf)("<version> --> [%d]\r\n" , GprsParam.Version);
                }else if(!strncmp(sScanf->contant , "serport" , strlen("serport"))){
                    if(dbgPrintf)(*dbgPrintf)("<serport> --> [%d]\r\n" , GprsParam.SerPort);
                }else if(!strncmp(sScanf->contant , "hearttime" , strlen("hearttime"))){
                    if(dbgPrintf)(*dbgPrintf)("<hearttime> --> [%d]\r\n" , GprsParam.HeartTime);
                }else if(!strncmp(sScanf->contant , "outerrtime" , strlen("outerrtime"))){
                    if(dbgPrintf)(*dbgPrintf)("<outerrtime> --> [%d]\r\n" , GprsParam.OutErrTime);
                }else if(!strncmp(sScanf->contant , "healttime" , strlen("healttime"))){
                    if(dbgPrintf)(*dbgPrintf)("<healttime> --> [%d]\r\n" , GprsParam.HealtTime);
                }else if(!strncmp(sScanf->contant , "buttontime" , strlen("buttontime"))){
                    if(dbgPrintf)(*dbgPrintf)("<buttontime> --> [%d]\r\n" , GprsParam.ButtTime);
                }else if(!strncmp(sScanf->contant , "inerrtime" , strlen("inerrtime"))){
                    if(dbgPrintf)(*dbgPrintf)("<inerrtime> --> [%d]\r\n" , GprsParam.InerrtTime);
                }else if(!strncmp(sScanf->contant , "maittime" , strlen("maittime"))){
                    if(dbgPrintf)(*dbgPrintf)("<maittime> --> [%d]\r\n" , GprsParam.MaitTime);
                }else if(!strncmp(sScanf->contant , "apnaccr" , strlen("apnaccr"))){
                    if(dbgPrintf)(*dbgPrintf)("<apnaccr> --> [%s]\r\n" , GprsParam.ApnAccr);
                }else if(!strncmp(sScanf->contant , "apnusr" , strlen("apnusr"))){
                    if(dbgPrintf)(*dbgPrintf)("<apnusr> --> [%s]\r\n" , GprsParam.ApnUsr);
                }else if(!strncmp(sScanf->contant , "apnpwd" , strlen("apnpwd"))){
                    if(dbgPrintf)(*dbgPrintf)("<apnpwd> --> [%s]\r\n" , GprsParam.ApnPwd);
                }else if(!strncmp(sScanf->contant , "sername" , strlen("sername"))){
                    if(dbgPrintf)(*dbgPrintf)("<sername> --> [%s]\r\n" , GprsParam.SerName);
                }else if(!strncmp(sScanf->contant , "smspwd" , strlen("smspwd"))){
                    if(dbgPrintf)(*dbgPrintf)("<smspwd> --> [%s]\r\n" , GprsParam.SmsPwd);
                }else if(!strncmp(sScanf->contant , "admin" , strlen("admin"))){
                    if(dbgPrintf)(*dbgPrintf)("<admin1> --> [%s]\r\n" , GprsParam.Admin[0]);
                    if(dbgPrintf)(*dbgPrintf)("<admin2> --> [%s]\r\n" , GprsParam.Admin[1]);
                    if(dbgPrintf)(*dbgPrintf)("<admin3> --> [%s]\r\n" , GprsParam.Admin[2]);
                    if(dbgPrintf)(*dbgPrintf)("<admin4> --> [%s]\r\n" , GprsParam.Admin[3]);
                    if(dbgPrintf)(*dbgPrintf)("<admin5> --> [%s]\r\n" , GprsParam.Admin[4]);
                }else if(!strncmp(sScanf->contant , "usron" , strlen("usron"))){
                    if(dbgPrintf)(*dbgPrintf)("<usron1> --> [%s]\r\n" , GprsParam.Usron[0]);
                    if(dbgPrintf)(*dbgPrintf)("<usron2> --> [%s]\r\n" , GprsParam.Usron[1]);
                    if(dbgPrintf)(*dbgPrintf)("<usron3> --> [%s]\r\n" , GprsParam.Usron[2]);
                    if(dbgPrintf)(*dbgPrintf)("<usron4> --> [%s]\r\n" , GprsParam.Usron[3]);
                    if(dbgPrintf)(*dbgPrintf)("<usron5> --> [%s]\r\n" , GprsParam.Usron[4]);
                    if(dbgPrintf)(*dbgPrintf)("<usron6> --> [%s]\r\n" , GprsParam.Usron[5]);
                    if(dbgPrintf)(*dbgPrintf)("<usron7> --> [%s]\r\n" , GprsParam.Usron[6]);
                    if(dbgPrintf)(*dbgPrintf)("<usron8> --> [%s]\r\n" , GprsParam.Usron[7]);
                    if(dbgPrintf)(*dbgPrintf)("<usron9> --> [%s]\r\n" , GprsParam.Usron[8]);
                    if(dbgPrintf)(*dbgPrintf)("<usron10> --> [%s]\r\n" , GprsParam.Usron[9]);
                }else if(!strncmp(sScanf->contant , "imei" , strlen("imei"))){
                    if(dbgPrintf)(*dbgPrintf)("<imei> --> [%s]\r\n" , GprsParam.IMEI);
                }else if(!strncmp(sScanf->contant , "iccid" , strlen("iccid"))){
                    if(dbgPrintf)(*dbgPrintf)("<iccid> --> [%s]\r\n" , GprsParam.ICCID);
                }else if(!strncmp(sScanf->contant , "cellinfo" , strlen("cellinfo"))){
                    if(dbgPrintf)(*dbgPrintf)("<cellinfo> --> [%s]\r\n" , GprsParam.CellInfo);
                }
            }else if(!strncmp(sScanf->comand , "upload" , strlen("upload"))){
                if(!strncmp(sScanf->contant , "reallen" , strlen("reallen"))){
                    if(dbgPrintf)(*dbgPrintf)("<reallen> --> [%d]\r\n" , g_Upload.RealDataLen);
                }else if(!strncmp(sScanf->contant , "errlen" , strlen("errlen"))){
                    if(dbgPrintf)(*dbgPrintf)("<errlen> --> [%d]\r\n" , g_Upload.ErrDataLen);
                }else if(!strncmp(sScanf->contant , "wflashaddr" , strlen("wflashaddr"))){
                    if(dbgPrintf)(*dbgPrintf)("<wflashaddr> --> [%d]\r\n" , g_Upload.writeFlashAddr);
                }else if(!strncmp(sScanf->contant , "rflashaddr" , strlen("rflashaddr"))){
                    if(dbgPrintf)(*dbgPrintf)("<rflashaddr> --> [%d]\r\n" , g_Upload.readFlashAddr);
                }else if(!strncmp(sScanf->contant , "sflashtime" , strlen("sflashtime"))){
                    if(dbgPrintf)(*dbgPrintf)("<sflashtime> --> [%d]\r\n" , g_Upload.saveFlashTime);
                }else if(!strncmp(sScanf->contant , "errptime" , strlen("errptime"))){
                    if(dbgPrintf)(*dbgPrintf)("<errptime> --> [%d]\r\n" , g_Upload.errPointTime);
                }else if(!strncmp(sScanf->contant , "curtimen" , strlen("curtimen"))){
                    if(dbgPrintf)(*dbgPrintf)("<curtimen> --> [%d]\r\n" , g_Upload.curTimeCnt);
                }else if(!strncmp(sScanf->contant , "dbug" , strlen("dbug"))){
                    if(dbgPrintf)(*dbgPrintf)("<dbug> --> [%d]\r\n" , g_Upload.Debug);
                }else if(!strncmp(sScanf->contant , "outerr" , strlen("outerr"))){
                    if(dbgPrintf)(*dbgPrintf)("<outerr> --> [%d]\r\n" , g_Upload.OutErr);
                }else if(!strncmp(sScanf->contant , "healt" , strlen("healt"))){
                    if(dbgPrintf)(*dbgPrintf)("<healt> --> [%d]\r\n" , g_Upload.Healt);
                }else if(!strncmp(sScanf->contant , "testsig" , strlen("testsig"))){
                    if(dbgPrintf)(*dbgPrintf)("<testsig> --> [%d]\r\n" , g_Upload.TestSig);
                }else if(!strncmp(sScanf->contant , "inerr" , strlen("inerr"))){
                    if(dbgPrintf)(*dbgPrintf)("<inerr> --> [%d]\r\n" , g_Upload.InErr);
                }else if(!strncmp(sScanf->contant , "inoff" , strlen("inoff"))){
                    if(dbgPrintf)(*dbgPrintf)("<inoff> --> [%d]\r\n" , g_Upload.IndOff);
                }else if(!strncmp(sScanf->contant , "fasttest" , strlen("fasttest"))){
                    if(dbgPrintf)(*dbgPrintf)("<fasttest> --> [%d]\r\n" , g_Upload.FastTest);
                }else if(!strncmp(sScanf->contant , "debugover" , strlen("debugover"))){
                    if(dbgPrintf)(*dbgPrintf)("<debugover> --> [%d]\r\n" , g_Upload.DebugOver);
                }else if(!strncmp(sScanf->contant , "sendflag" , strlen("sendflag"))){
                    if(dbgPrintf)(*dbgPrintf)("<sendflag> --> [%d]\r\n" , g_Upload.SendFlag);
                }else if(!strncmp(sScanf->contant , "errclr" , strlen("errclr"))){
                    if(dbgPrintf)(*dbgPrintf)("<errclr> --> [%d]\r\n" , g_Upload.ErrIsClr);
                }
            }else if(!strncmp(sScanf->comand , "var" , strlen("var"))){
                if(!strncmp(sScanf->contant , "prjdbug" , strlen("prjdbug"))){
                    if(dbgPrintf)(*dbgPrintf)("<prjdbug> --> [%d]\r\n" , var.projDebug);
                }else if(!strncmp(sScanf->contant , "online" , strlen("online"))){
                    if(dbgPrintf)(*dbgPrintf)("<online> --> [%d]\r\n" , var.onlineTest);
                }else if(!strncmp(sScanf->contant , "outerr" , strlen("outerr"))){
                    if(dbgPrintf)(*dbgPrintf)("<outerr> --> [%d]\r\n" , var.outdoorErr);
                }else if(!strncmp(sScanf->contant , "inerr" , strlen("inerr"))){
                    if(dbgPrintf)(*dbgPrintf)("<inerr> --> [%d]\r\n" , var.indoorErr);
                }else if(!strncmp(sScanf->contant , "button" , strlen("button"))){
                    if(dbgPrintf)(*dbgPrintf)("<button> --> [%d]\r\n" , var.buttonPush);
                }else if(!strncmp(sScanf->contant , "real" , strlen("real"))){
                    if(dbgPrintf)(*dbgPrintf)("<real> --> [%d]\r\n" , var.realMonitor);
                }else if(!strncmp(sScanf->contant , "hyaline" , strlen("hyaline"))){
                    if(dbgPrintf)(*dbgPrintf)("<hyaline> --> [%d]\r\n" , var.Hyaline);
                }
            }else if(!strncmp(sScanf->comand , "NS" , strlen("NS"))){
                if(!strncmp(sScanf->contant , "?" , strlen("?"))){
                    if(dbgPrintf)(*dbgPrintf)("<NS> --> [%d]\r\n" , NS);
                }
            }else if(!strncmp(sScanf->comand , "time" , strlen("time"))){
                if(!strncmp(sScanf->contant , "?" , strlen("?"))){
                    if(dbgPrintf)(*dbgPrintf)("[%04d/%02d/%02d %02d:%02d:%02d] [%d]\r\n" , 
                              localTime.tm_year + 1900,
                              localTime.tm_mon  + 1,
                              localTime.tm_mday,
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              localTime.tm_wday);
                }
            }
        }
    }
    return 0;
}
