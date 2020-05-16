/* Includes ------------------------------------------------------------------*/
#include "iap.h"
#include "drv_button.h"
#include "drv_usart.h"
#include "drv_flash.h"
#include "common.h"
#include "stm32f10x_flash.h"
#include "drv_net.h"
#include "fsm.h"

/* Private variables ---------------------------------------------------------*/
pFunction Jump_To_Application;
uint32_t  JumpAddress;
uint32_t BlockNbr = 0, UserMemoryMask = 0;
__IO uint32_t FlashProtection = 0;

/////////////////////////////////
uint8_t update = 0,programversion;
uint32_t FlashDestination1 = 0x8003000,eraseaddress = 0x8020000,DowndloadDestination = 0;
uint32_t RamSource1;

///////////////////////////////////////////////////////////


void Int2Str(uint8_t* str, int32_t intnum) {
    uint32_t i, Div = 1000000000, j = 0, Status = 0;

    for (i = 0; i < 10; i++) {
        str[j++] = (intnum / Div) + 48;

        intnum = intnum % Div;
        Div /= 10;
        if ((str[j-1] == '0') & (Status == 0)) {
            j = 0;
        } else {
            Status++;
        }
    }
}

uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum) {
    uint32_t i = 0, res = 0;
    uint32_t val = 0;

    if (inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X')) {
        if (inputstr[2] == '\0') {
            return 0;
        }
        for (i = 2; i < 11; i++) {
            if (inputstr[i] == '\0') {
                *intnum = val;
                /* return 1; */
                res = 1;
                break;
            }
            if (ISVALIDHEX(inputstr[i])) {
                val = (val << 4) + CONVERTHEX(inputstr[i]);
            } else {
                /* return 0, Invalid input */
                res = 0;
                break;
            }
        }
        /* over 8 digit hex --invalid */
        if (i >= 11) {
            res = 0;
        }
    } else { /* max 10-digit decimal input */
        for (i = 0; i < 11; i++) {
            if (inputstr[i] == '\0') {
                *intnum = val;
                /* return 1 */
                res = 1;
                break;
            } else if ((inputstr[i] == 'k' || inputstr[i] == 'K') && (i > 0)) {
                val = val << 10;
                *intnum = val;
                res = 1;
                break;
            } else if ((inputstr[i] == 'm' || inputstr[i] == 'M') && (i > 0)) {
                val = val << 20;
                *intnum = val;
                res = 1;
                break;
            } else if (ISVALIDDEC(inputstr[i])) {
                val = val * 10 + CONVERTDEC(inputstr[i]);
            } else {
                /* return 0, Invalid input */
                res = 0;
                break;
            }
        }
        /* Over 10 digit decimal --invalid */
        if (i >= 11) {
            res = 0;
        }
    }

    return res;
}

/**
  * @brief  Get an integer from the HyperTerminal
  * @param  num: The inetger
  * @retval 1: Correct
  *         0: Error
  */
uint32_t GetIntegerInput(int32_t * num) 
{
    uint8_t inputstr[16];

    while (1) {
        GetInputString(inputstr);
        if (inputstr[0] == '\0') continue;
        if ((inputstr[0] == 'a' || inputstr[0] == 'A') && inputstr[1] == '\0') {
            SerialPutString("User Cancelled \r\n");
            return 0;
        }

        if (Str2Int(inputstr, num) == 0) {
            SerialPutString("Error, Input again: \r\n");
        } else {
            return 1;
        }
    }
}

/**
  * @brief  Test to see if a key has been pressed on the HyperTerminal
  * @param  key: The key pressed
  * @retval 1: Correct
  *         0: Error
  */
uint32_t SerialKeyPressed(uint8_t *key) 
{
    if ( USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET) {
        *key = (uint8_t)USART2->DR;
        return 1;
    } else {
        return 0;
    }
}

/**
  * @brief  Get a key from the HyperTerminal
  * @param  None
  * @retval The Key Pressed
  */
uint8_t GetKey(void) 
{
    uint8_t key = 0;

    /* Waiting for user input */
    while (1) {
        if (SerialKeyPressed((uint8_t*)&key)) break;
    }
    return key;

}

/**
  * @brief  Print a character on the HyperTerminal
  * @param  c: The character to be printed
  * @retval None
  */
void SerialPutChar(uint8_t c) 
{
    USART_ClearFlag(USART2,USART_FLAG_TC);
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    USART_SendData(USART2, (uint8_t)c);
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {
    }
}

/**
  * @brief  Print a string on the HyperTerminal
  * @param  s: The string to be printed
  * @retval None
  */
void Serial_PutString(uint8_t *s) {
    while (*s != '\0') {
        SerialPutChar(*s);
        s++;
    }
}

/**
  * @brief  Get Input string from the HyperTerminal
  * @param  buffP: The input string
  * @retval None
  */
void GetInputString (uint8_t * buffP) {
    uint32_t bytes_read = 0;
    uint8_t c = 0;
    do {
        c = GetKey();
        if (c == '\r')
            break;
        if (c == '\b') { /* Backspace */
            if (bytes_read > 0) {
                SerialPutString("\b \b");
                bytes_read --;
            }
            continue;
        }
        if (bytes_read >= CMD_STRING_SIZE ) {
            SerialPutString("Command string size overflow\r\n");
            bytes_read = 0;
            continue;
        }
        if (c >= 0x20 && c <= 0x7E) {
            buffP[bytes_read++] = c;
            SerialPutChar(c);
        }
    } while (1);
    SerialPutString(("\n\r"));
    buffP[bytes_read] = '\0';
}

/**
  * @brief  Calculate the number of pages
  * @param  Size: The image size
  * @retval The number of pages
  */
uint32_t FLASH_PagesMask(__IO uint32_t Size) {
    uint32_t pagenumber = 0x0;
    uint32_t size = Size;

    if ((size % PAGE_SIZE) != 0) {
        pagenumber = (size / PAGE_SIZE) + 1;
    } else {
        pagenumber = size / PAGE_SIZE;
    }
    return pagenumber;

}
#if 0
/**
  * @brief  Disable the write protection of desired pages
  * @param  None
  * @retval None
  */
void FLASH_DisableWriteProtectionPages(void) {
    uint32_t useroptionbyte = 0, WRPR = 0;
    uint16_t var1 = OB_IWDG_SW, var2 = OB_STOP_NoRST, var3 = OB_STDBY_NoRST;
    FLASH_Status status = FLASH_BUSY;

    WRPR = FLASH_GetWriteProtectionOptionByte();

    /* Test if user memory is write protected */
    if ((WRPR & UserMemoryMask) != UserMemoryMask) {
        useroptionbyte = FLASH_GetUserOptionByte();

        UserMemoryMask |= WRPR;

        status = FLASH_EraseOptionBytes();

        if (UserMemoryMask != 0xFFFFFFFF) {
            status = FLASH_EnableWriteProtection((uint32_t)~UserMemoryMask);
        }

        /* Test if user Option Bytes are programmed */
        if ((useroptionbyte & 0x07) != 0x07) {
            /* Restore user Option Bytes */
            if ((useroptionbyte & 0x01) == 0x0) {
                var1 = OB_IWDG_HW;
            }
            if ((useroptionbyte & 0x02) == 0x0) {
                var2 = OB_STOP_RST;
            }
            if ((useroptionbyte & 0x04) == 0x0) {
                var3 = OB_STDBY_RST;
            }

            FLASH_UserOptionByteConfig(var1, var2, var3);
        }

        if (status == FLASH_COMPLETE) {
            SerialPutString("Write Protection disabled...\r\n");

            SerialPutString("...and a System Reset will be generated to re-load the new option bytes\r\n");

            /* Generate System Reset to load the new option byte values */
            NVIC_SystemReset();
        } else {
            SerialPutString("Error: Flash write unprotection failed...\r\n");
        }
    } else {
        SerialPutString("Flash memory not write protected\r\n");
    }
}
#endif 

#if 0
void UpdateFromFlash(void)
{
    uint8_t i = 0;
    uint32_t iapAddr = 0 , u32_Data = 0, appaddr = 0;
    FLASH_Unlock();
    WriteFlashAddr = 0;
    if(Addr1_version > Addr2_version){
        appaddr = FIRMWARE2_UPDATE_START;
    }
    else{
        appaddr = FIRMWARE1_UPDATE_START;
    }
    for (iapAddr = 0; iapAddr < 122; iapAddr++) {
        FLASH_ErasePage(ApplicationAddress + (0x800 * iapAddr));
    }
    for(i=0;i<244;i++){
        sFLASH_ReadBuffer((uint8_t*) SPIFlashBuff, appaddr + WriteFlashAddr, 1024);
        for(iapAddr = 0;iapAddr < 256;iapAddr++){
            memcpy(&u32_Data , &SPIFlashBuff[iapAddr] , 4);
//             SerialPutString("%s/n",u32_Data);
            
            if(FLASH_ProgramWord(0x8003000+i*1024+iapAddr*4,u32_Data) != FLASH_COMPLETE)//ÂÜôÂÖ•Âõõ‰∏™Â≠óËäÇ
            {
                break;
            }
        }
        WriteFlashAddr += 1024;
    }
    FLASH_Lock();
}
void Recover_old(void)
{
   uint8_t i = 0;
    uint32_t iapAddr = 0 , u32_Data = 0, appaddr = 0;
    FLASH_Unlock();
    WriteFlashAddr = 0;
    if(Addr1_version > Addr2_version){
        appaddr = FIRMWARE1_UPDATE_START;
    }
    else{
        appaddr = FIRMWARE2_UPDATE_START;
    }
    for (iapAddr = 0; iapAddr < 122; iapAddr++) {
        FLASH_ErasePage(ApplicationAddress + (0x800 * iapAddr));
    }
    for(i=0;i<244;i++){
        sFLASH_ReadBuffer((uint8_t*) SPIFlashBuff, appaddr + WriteFlashAddr, 1024);
        for(iapAddr = 0;iapAddr < 256;iapAddr++){
            memcpy(&u32_Data , &SPIFlashBuff[iapAddr] , 4);
//             SerialPutString("%s/n",u32_Data);
            
            if(FLASH_ProgramWord(0x8003000+i*1024+iapAddr*4,u32_Data) != FLASH_COMPLETE)//ÂÜôÂÖ•Âõõ‰∏™Â≠óËäÇ
            {
                break;
            }
        }
        WriteFlashAddr += 1024;
    }
    FLASH_Lock();
}

/////////////////////////æ…∞Ê±æª÷∏¥°¢°¢°¢°¢°¢°¢°¢°¢°¢°¢°¢°¢°¢°¢°¢°¢///////////////////
void recoverto_old(void)
{
  uint8_t version_buff[12];
  uint8_t buf[1024] = {0};
  uint16_t i = 0, j = 0;
  FLASH_Unlock();
  FlashDestination1 = 0x8003000;
  sFLASH_ReadBuffer(version_buff, PROGRAM_VERSION_START_ADDR, 12);
  version_info.addr1 = (version_buff[0]|((uint32_t)version_buff[1]<<8)|((uint32_t)version_buff[2]<<16)|((uint32_t)version_buff[3]<<24));
  version_info.addr1_version = version_buff[4];
  version_info.addr1_version_run = version_buff[5];
  version_info.addr2 = (version_buff[6]|((uint32_t)version_buff[7]<<8)|((uint32_t)version_buff[8]<<16)|((uint32_t)version_buff[9]<<24));
  version_info.addr2_version = version_buff[10];
  version_info.addr2_version_run = version_buff[11];
  sFLASH_EraseSector(PROGRAM_VERSION_START_ADDR);
   if(version_info.addr1 == version_info.addr2)
  { 
//    if(dbgPrintf)(*dbgPrintf)("NOT OLDVERSION!");/////////////////////////////
     SerialPutString("NOT OLDVERSION!");
  }
   else if(version_info.addr1_version < version_info.addr2_version )
    {
      if(version_info.addr1_version_run == 0)
      { 
        for(i = 0;i < 20 ; i++ )
        { 
          FLASH_ErasePage(FlashDestination1 + (i * 0x800));
        }
      //  FlashDestination1 = 0x8003000;
        for(i = 0; i< 128; i ++)
        {
          sFLASH_ReadBuffer(&buf[0], version_info.addr1, 1024);
          RamSource1 = (uint32_t)&buf[0];
          for(j = 0; j < 1024; j += 4)
          {
            FLASH_ProgramWord(FlashDestination1,*(uint32_t*)RamSource1);
            RamSource1 += 4;
            FlashDestination1 += 4;
          }
        }
//        sFLASH_EraseSector(PROGRAM_VERSION_START_ADDR);
        version_info.addr1_version_run = RUN;
        version_info.addr2_version_run = STOP;
      }
    }
    else 
    {
      if(version_info.addr2_version_run == 0)
      {
         for(i = 0;i < 20 ; i++ )
        { 
          FLASH_ErasePage(FlashDestination1 + (i * 0x800));
        }
//        FlashDestination1 = 0x8003000;
        for(i = 0; i< 128; i ++)
        {
          sFLASH_ReadBuffer(&buf[0], version_info.addr2, 1024);
          RamSource1 = (uint32_t)&buf[0];
          for(j = 0; j < 1024; j += 4)
          {
            FLASH_ProgramWord(FlashDestination1,*(uint32_t*)RamSource1);
            RamSource1 += 4;
            FlashDestination1 += 4;
          }
        }
 //       sFLASH_EraseSector(PROGRAM_VERSION_START_ADDR);
        version_info.addr2_version_run = RUN;
        version_info.addr1_version_run = STOP;
      }
    }
    memcpy(&version_buff[0],&version_info.addr1,sizeof(version_info.addr1));
    memcpy(&version_buff[4],&version_info.addr1_version,sizeof(version_info.addr1_version));
    memcpy(&version_buff[5],&version_info.addr1_version_run,sizeof(version_info.addr1_version_run));
    memcpy(&version_buff[6],&version_info.addr2,sizeof(version_info.addr2));
    memcpy(&version_buff[10],&version_info.addr2_version,sizeof(version_info.addr2_version));
    memcpy(&version_buff[11],&version_info.addr2_version_run,sizeof(version_info.addr2_version_run));
    sFLASH_WriteBuffer(&version_buff[0],PROGRAM_VERSION_START_ADDR , 12);
    FLASH_Lock();
}
//////////////////////////////////////////////////////////////////////////////////////
uint8_t version_buff[12];
uint8_t buf1[1024] = {0},kk=0;
///////∞Ê±æ»∑∂®///////////////////////////////////////////////////////////////////////
void version_check(void)
{

  uint16_t i = 0, j = 0;
  FLASH_Unlock();
  sFLASH_ReadBuffer(&version_buff[0], PROGRAM_VERSION_START_ADDR, 12);
  version_info.addr1 = (version_buff[0]|((uint32_t)version_buff[1]<<8)|((uint32_t)version_buff[2]<<16)|((uint32_t)version_buff[3]<<24));
  version_info.addr1_version = version_buff[4];
  version_info.addr1_version_run = version_buff[5];
  version_info.addr2 = (version_buff[6]|((uint32_t)version_buff[7]<<8)|((uint32_t)version_buff[8]<<16)|((uint32_t)version_buff[9]<<24));
  version_info.addr2_version = version_buff[10];
  version_info.addr2_version_run = version_buff[11];
  sFLASH_EraseSector(PROGRAM_VERSION_START_ADDR);
  if(version_info.addr1 == version_info.addr2)
  { 
    version_info.addr1 = PROGRAM1_START_ADDR;
    version_info.addr1_version = PROGREAME_VERSION;
    version_info.addr1_version_run = RUN;
    version_info.addr2 = PROGRAM2_START_ADDR  ;
    version_info.addr2_version = 0; 
    version_info.addr2_version_run = 0;
    version_info.addr2_version_run = STOP;
//    sFLASH_WriteBuffer(&version_buff[0], PROGRAM_VERSION_START_ADDR , 12);
  for(i=0; i<25;i++)
  {
    sFLASH_EraseSubSector(version_info.addr1+(i*0x1000));
  }
    FlashDestination1 = 0x8003000;
    for(i=0; i< 100; i ++)
    {
      Flash_Read((0x8003000 + (i*0x400)), &buf1[0], 1024);
      sFLASH_WriteBuffer(&buf1[0], (version_info.addr1 + (i*0x400)), 1024);
      kk=kk+1;
    }
  
    kk=0;
  }
   else if(version_info.addr1_version >version_info.addr2_version )
    {   
      if(version_info.addr1_version_run == 0)
      {
         for(i = 0;i < 100 ; i++ )
        { 
          FLASH_ErasePage(FlashDestination1 + (i * 0x400));
//          FLASH_ErasePage(FlashDestination1);
        }
        FlashDestination1 = 0x8003000;
        for(i = 0; i< 100; i ++)
        {
          kk+=1;
          sFLASH_ReadBuffer(&buf1[0], version_info.addr1 + (i*0x400), 1024);
          RamSource1 = (uint32_t)&buf1[0];
          for(j = 0; j < 1024; j += 4)
          {
            FLASH_ProgramWord(FlashDestination1,*(uint32_t*)RamSource1);
            RamSource1 += 4;
            FlashDestination1 += 4;
          } 
        }
//        sFLASH_EraseSector(PROGRAM_VERSION_START_ADDR);
        version_info.addr1_version_run = RUN;
        version_info.addr2_version_run = STOP;
      }
    }
    else 
    {
      if(version_info.addr2_version_run == 0)
      {

       for(i = 0;i < 10 ; i++ )
       { 
 //         FLASH_ErasePage(FlashDestination1 + (i * 0x400));
         FLASH_ErasePage(FlashDestination1);
        }
 
        FlashDestination1 = 0x8003000;
        for(i = 0; i< 100; i ++)
        {
          kk+=1;
          sFLASH_ReadBuffer(&buf1[0], version_info.addr2+ (i*400), 1024);
          RamSource1 = (uint32_t)&buf1[0];
          for(j = 0; j < 1024; j += 4)
          {
            FLASH_ProgramWord(FlashDestination1,*(uint32_t*)RamSource1);
            RamSource1 += 4;
            FlashDestination1 += 4;
          }
        }
 //       sFLASH_EraseSector(PROGRAM_VERSION_START_ADDR);
        version_info.addr2_version_run = RUN;
        version_info.addr1_version_run = STOP;
      }
    }
    memcpy(&version_buff[0],&version_info.addr1,sizeof(version_info.addr1));
    memcpy(&version_buff[4],&version_info.addr1_version,sizeof(version_info.addr1_version));
    memcpy(&version_buff[5],&version_info.addr1_version_run,sizeof(version_info.addr1_version_run));
    memcpy(&version_buff[6],&version_info.addr2,sizeof(version_info.addr2));
    memcpy(&version_buff[10],&version_info.addr2_version,sizeof(version_info.addr2_version));
    memcpy(&version_buff[11],&version_info.addr2_version_run,sizeof(version_info.addr2_version_run));
    sFLASH_WriteBuffer(&version_buff[0],PROGRAM_VERSION_START_ADDR , 12);
    FLASH_Lock();
}
///////////////////////////////////////////////////////////////////////////////////////////////////

/******************* (C) COPYRIGHT 2015 GREE ELECTRIC APPLIANCES,INC.  *****END OF FILE****/
#endif

//typedef struct
//{
//    uint8_t  upate_receive_flag;  //Êé•Êî∂ÂÆåÊàêÊ†áÂøó
//    uint8_t  update_pack_type;    //ÂçáÁ∫ßÂåÖÁ±ªÂûã
//    uint8_t  update_success_flag; //ÂçáÁ∫ßÊàêÂäüÊ†áÂøó
//    uint8_t  updata_hisSOFTVERSION;//ËΩØ‰ª∂ÂéÜÂè≤ÁâàÊú¨Âè∑
//    uint8_t  recover_have_packet;         //ÊòØÂê¶ÊúâÂ§á‰ªΩÁ®ãÂ∫èÊ†áÂøó
//    uint8_t  begin_recover_time;      //Èò≤Ê≠¢‰∏ÄÊ¨°ÂçáÁ∫ßËøáÁ®ãÊàêÂäüÁΩÆ‰ΩçÂâçÊñ≠ÁîµÂ∞±ÂõûÊªöÈóÆÈ¢ò„ÄÇ
//    uint32_t update_readaddr;
//    uint32_t update_writeaddr;
//}updateInfoType;



//updateInfoType updateinfo = 
//{
//  //receive flag
//  {0},
//  //update pack type
//  {0},
//  //success flag
//  {1},
//  {0},
//  //writeaddr
//  {0x00F00000},
//  //read addr
//  {0x00F00000},
//};

updateInfoType updateinfo = 
{
	.update_success_flag = 1,
	.update_readaddr = 0x00EB0000,
	.update_writeaddr = 0x00EB0000,
};



uint8_t buffer[1024];
uint8_t updatefuc(updateInfoType* m_updateinfo)
{
    uint16_t i;
    uint32_t writeupdate_addr = 0x8008000;
    uint32_t old_program_endaddr = 0x803ffff;
    uint8_t ee_flag ;
    sFLASH_ReadBuffer(&ee_flag, UPDATE_INFO_START_ADDR, sizeof(ee_flag));

    if(ee_flag == 0xBB){
        sFLASH_ReadBuffer((uint8_t *)m_updateinfo, UPDATE_INFO_START_ADDR+1, sizeof(updateInfoType));
        if(m_updateinfo->upate_receive_flag){
            if(m_updateinfo->update_pack_type == 0x02){
                SerialPutString("begin to update new program! \n\r");
                back_up(writeupdate_addr, old_program_endaddr, PARAM_BACKUP_START_ADDR);
			    m_updateinfo->upate_receive_flag = 0;
                m_updateinfo->update_success_flag = 0;
                m_updateinfo->recover_have_packet = 1;
				m_updateinfo->upgrade_succ_count++;
                sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
                ee_flag = 0xBB;
                sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
                sFLASH_WriteBuffer((uint8_t *)m_updateinfo, UPDATE_INFO_START_ADDR + 1, sizeof(updateInfoType));
                
                FLASH_Unlock();
                while(m_updateinfo->update_readaddr < m_updateinfo->update_writeaddr){
                   IWDG_ReloadCounter();
                    if(!(writeupdate_addr%0x800)){
                      FLASH_ErasePage(writeupdate_addr);
                    }
                    RamSource1 = (uint32_t)&buffer[0];
                    sFLASH_ReadBuffer(&buffer[0], m_updateinfo->update_readaddr, 1024);
                    SerialPutString("write to flash\r\n");
                     if(dbgPrintf)(*dbgPrintf)("read back ----> ");
                    for(i = 0; i < 1024; i+=4){
                      if(dbgPrintf)(*dbgPrintf)("%2X " , buffer[i]);
                      FLASH_ProgramWord(writeupdate_addr,*(uint32_t*)RamSource1);
                      RamSource1 += 4;
                      writeupdate_addr += 4;
                      if(writeupdate_addr > (old_program_endaddr - 4)){
                          break;
                      }
                    }
                     if(dbgPrintf)(*dbgPrintf)("\n\r");
                    m_updateinfo->update_readaddr += 0x400;
                    
                }
                FLASH_Lock();
//                m_updateinfo->upate_receive_flag = 0;
//                m_updateinfo->update_success_flag = 0;
//                m_updateinfo->recover_have_packet = 1;
//				  m_updateinfo->upgrade_succ_count++;
//                sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
//                ee_flag = 0xBB;
//                sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
//                sFLASH_WriteBuffer((uint8_t *)m_updateinfo, UPDATE_INFO_START_ADDR + 1, sizeof(updateInfoType));
                return 0;
            }
        }else if(m_updateinfo->update_success_flag != 0x01){
            if(m_updateinfo->begin_recover_time == 0){
                SerialPutString("begin to recover the old program! \n\r");
                recover_old(PARAM_BACKUP_START_ADDR, PARAM_BACKUP_END_ADDR, 0x8008000);
				return 0;
            }else{
                m_updateinfo->begin_recover_time -= 1;
				m_updateinfo->upgrade_fail_count++;
                sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
                ee_flag = 0xBB;
                sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
                sFLASH_WriteBuffer((uint8_t *)m_updateinfo, UPDATE_INFO_START_ADDR + 1, sizeof(updateInfoType));
				return 0;
            }
        }
    }else{
                ee_flag = 0xBB;
                sFLASH_EraseSubSector(UPDATE_INFO_START_ADDR);
                sFLASH_WriteBuffer((uint8_t *)&ee_flag , UPDATE_INFO_START_ADDR , sizeof(ee_flag));
				return 1;
    }
	return 0;
}


uint8_t back_up(uint32_t readbegin_addr, uint32_t readend_addr, uint32_t dest_addr)
{
    uint32_t data32, read_adrr, end_addr, write_addr,i;
    read_adrr = readbegin_addr;
    end_addr = readend_addr;
    write_addr = dest_addr;
    uint8_t data[1024];
    while(read_adrr < end_addr){
        IWDG_ReloadCounter();
        data32 = *(__IO uint32_t*)read_adrr;
        if(!(write_addr%0x1000)){
            sFLASH_EraseSubSector(write_addr);
        }
        sFLASH_WriteBuffer((uint8_t *)&data32, write_addr, sizeof(uint32_t));
        write_addr +=4;
        read_adrr += 4;
        if(write_addr > 0x00FF0000) return 0;
    }
    sFLASH_ReadBuffer(data, PARAM_BACKUP_START_ADDR, 1024);
    if(dbgPrintf)(*dbgPrintf)("read back ----> ");
    for(i=0; i<1024; i++){
        if(dbgPrintf)(*dbgPrintf)("%2X " , data[i]);
    }
    if(dbgPrintf)(*dbgPrintf)("\n\r");
    return 1;
}

int aa;
uint8_t recover_old(uint32_t readbegin_addr, uint32_t readend_addr, uint32_t dest_addr)
{
    uint32_t read_adrr, end_addr, write_addr, i;
    uint32_t* data;
    uint8_t buffer[1024];
    read_adrr = readbegin_addr;
    end_addr = readend_addr;
    write_addr = dest_addr;
    FLASH_Unlock();
    while(read_adrr < end_addr){
        if(!(write_addr%0x800)){
          FLASH_ErasePage(write_addr);
        }
        IWDG_ReloadCounter();
        sFLASH_ReadBuffer(buffer, read_adrr, sizeof(buffer));
        if(dbgPrintf)(*dbgPrintf)("read back ----> ");
       
        for(i = 0; i < 1024; i += 4){
            if(dbgPrintf)(*dbgPrintf)("%2X " , buffer[i]);
            if(dbgPrintf)(*dbgPrintf)("%2X " , buffer[i+1]);
            if(dbgPrintf)(*dbgPrintf)("%2X " , buffer[i+2]);
            if(dbgPrintf)(*dbgPrintf)("%2X " , buffer[i+3]);
            data = (uint32_t *)&buffer[i];
            FLASH_ProgramWord(write_addr,*data);
            write_addr += 4;
        }
        read_adrr += 0x400;
        if(dbgPrintf)(*dbgPrintf)("\n\r");
        if(dbgPrintf)(*dbgPrintf)("the ROM address is %2X\n\r", write_addr);
        if(write_addr > 0x8030FFF) break;  //‘ΩΩÁ¥¶¿Ì
    }
    FLASH_Lock();
    return 1;
}

