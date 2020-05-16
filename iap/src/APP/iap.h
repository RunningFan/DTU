/***************************************************************************** 
Copyright: 2014 GREE ELECTRIC APPLIANCES,INC. 
File name: app.c
Description: 用于详细说明此程序文件完成的主要功能，与其他模块或函数的接口，输出
值、取值范围、含义及参数间的控制、顺序、独立或依赖等关系
Author:  Sean.Y
Version: V1.0 
Date:    2015/04/09
History: 修改历史记录列表，每条修改记录应包括修改日期、修改者及修改内容简述。 
*****************************************************************************/


//#ifndef _IAP_H_
//#define _IAP_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
typedef  void (*pFunction)(void);

/* Exported constants --------------------------------------------------------*/
/* Constants used by Serial Command Line Mode */
#define CMD_STRING_SIZE       128

#define ApplicationAddress    0x8008000


#if defined (STM32F10X_MD) || defined (STM32F10X_MD_VL)
 #define PAGE_SIZE                         (0x400)    /* 1 Kbyte */
 #define FLASH_SIZE                        (0x20000)  /* 128 KBytes */
#elif defined STM32F10X_CL
 #define PAGE_SIZE                         (0x800)    /* 2 Kbytes */
 #define FLASH_SIZE                        (0x40000)  /* 256 KBytes */
#elif defined STM32F10X_HD || defined (STM32F10X_HD_VL)
 #define PAGE_SIZE                         (0x800)    /* 2 Kbytes */
 #define FLASH_SIZE                        (0x80000)  /* 512 KBytes */
#elif defined STM32F10X_XL
 #define PAGE_SIZE                         (0x800)    /* 2 Kbytes */
 #define FLASH_SIZE                        (0x100000) /* 1 MByte */
#else 
 #error "Please select first the STM32 device to be used (in stm32f10x.h)"    
#endif

/* Compute the FLASH upload image size */  
#define FLASH_IMAGE_SIZE                   (uint32_t) (FLASH_SIZE - (ApplicationAddress - 0x08000000))

/* Exported macro ------------------------------------------------------------*/
/* Common routines */
#define IS_AF(c)  ((c >= 'A') && (c <= 'F'))
#define IS_af(c)  ((c >= 'a') && (c <= 'f'))
#define IS_09(c)  ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)  IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c)  IS_09(c)
#define CONVERTDEC(c)  (c - '0')

#define CONVERTHEX_alpha(c)  (IS_AF(c) ? (c - 'A'+10) : (c - 'a'+10))
#define CONVERTHEX(c)   (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))

#define SerialPutString(x) uart2SendStr((const char*)(x))


    /* STM32固件升级，大小256KB */
#define FIRMWARE1_UPDATE_START      0x00F00000
#define FIRMWARE1_UPDATE_END        0x00F3FFFF
    /* STM32固件升级，大小256KB */
#define FIRMWARE2_UPDATE_START      0x00F40000
#define FIRMWARE2_UPDATE_END        0x00F7FFFF

/*程序的新旧区分*/
#define New  (0x01)
#define Old  (0x02)
#include "stm32f10x.h"
////////////////////////////////////////
#define  updatecheck    0x01
#define  updateprogram  0x02
#define  get            0x01
#define  getfail        0x00
#define  PROGREAME_VERSION     0x01
#define  RUN            0x01
#define  STOP           0x00
typedef struct
{
  uint32_t addr1;
  uint8_t  addr1_version;
  uint8_t  addr1_version_run;
  uint32_t addr2;
  uint8_t  addr2_version;
  uint8_t  addr2_version_run;
  uint8_t  update_version;
} Version_Info;

typedef struct
{
    uint8_t  upate_receive_flag;  //接收完成标志
    uint8_t  update_pack_type;    //升级包类型
    uint8_t  update_success_flag; //升级成功标志
    uint8_t  updata_hisSOFTVERSION;//软件历史版本号
    uint8_t  recover_have_packet;//是否有备份程序标志
    uint8_t  begin_recover_time;//防止一次升级过程成功置位前断电就回滚问题。
    uint16_t upgrade_succ_count;//DTU自身升级成功的次数
	uint16_t upgrade_fail_count;//DTU自身升级失败的次数
    uint32_t update_readaddr;
    uint32_t update_writeaddr;

}updateInfoType;


/************借用300k做程序更新**************/
#define PROGRAM1_START_ADDR               0x00F00000
#define PROGRAM1_END_ADDR                 0x00F257FF
#define PROGRAM2_START_ADDR               0x00F25800
#define PROGRAM2_END_ADDR                 0x00F4AFFF
/**********程序版本信息区************/
#define PROGRAM_VERSION_START_ADDR        0x00F4B000
#define PROGRAM_VERSION_END_ADDR          0x00F4BFFF



int Flash_Read(uint32_t iAddress, uint8_t *buf, int32_t iNbrToRead);
void version_check(void);
void recoverto_old(void);
////////////////////////////////////////////////////////////////
extern uint8_t Addr1_version,Addr2_version;

/* Exported variables ------------------------------------------------------- */
extern pFunction Jump_To_Application;
extern uint32_t JumpAddress;
extern updateInfoType updateinfo;

/* Exported functions ------------------------------------------------------- */
void Int2Str(uint8_t* str,int32_t intnum);
uint32_t Str2Int(uint8_t *inputstr,int32_t *intnum);
uint32_t GetIntegerInput(int32_t * num);
uint32_t SerialKeyPressed(uint8_t *key);
uint8_t GetKey(void);
void SerialPutChar(uint8_t c);
void Serial_PutString(uint8_t *s);
void GetInputString(uint8_t * buffP);
uint32_t FLASH_PagesMask(__IO uint32_t Size);
void FLASH_DisableWriteProtectionPages(void);
void MainMenu(void);
void SerialDownload(void);
void SerialUpload(void);
uint8_t back_up(uint32_t readbegin_addr, uint32_t readend_addr, uint32_t dest_addr);

uint8_t updatefuc(updateInfoType* updateinfo);
uint8_t recover_old(uint32_t readbegin_addr, uint32_t readend_addr, uint32_t dest_addr);
/******************* (C) COPYRIGHT 2014 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
