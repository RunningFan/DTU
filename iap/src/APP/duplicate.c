
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
#include "duplicate.h"
#include "fsm.h"
#include "drv_sms.h"

uint8_t   sendCnt = 0;
OS_EVENT  *QSemSend = NULL;
void      *SendPtrArr[5];
BUFF_TYPE SendBuffer[5];

UpDataType g_Upload;
OS_EVENT *MutexFlash;
OS_EVENT *MutexSend1;
OS_EVENT *MutexSend3;
OS_EVENT *MutexSend2;
OS_EVENT *MutexSend4;

ParamType ACParam;
uint32_t  g_FlashWriteAddr;
uint16_t  packTime = 0;
PageTpye  PageData[3] = 
{
    { SysTable , 50  },
    { OdrTable , 187 },
    { IdrTable , 183 }
};

SrcDataTyp  SysTable[ ] = {
    {0x1004,0x0000},{0x1005,0x0010},{0x1109,0x0020},{0x113A,0x0030},{0x113B,0x0040},
    {0x113C,0x0050},{0x113D,0x0060},{0x113E,0x0070},{0x1C50,0x0080},{0x1C51,0x0090},
    {0x1C58,0x00A0},{0x1C5B,0x00B0},{0x1D6B,0x00C0},{0x1D6C,0x00D0},{0x1D6E,0x00E0},
    {0x1D70,0x00F0},{0x1D71,0x0100},{0x1D72,0x0110},{0x1D75,0x0120},{0x1E51,0x0130},
    {0x1E53,0x0140},{0x1F1A,0x0150},{0x1F51,0x0160},{0x1F54,0x0170},{0x1F57,0x0180},
    {0x1F58,0x0190},{0x1F59,0x01A0},{0x1F5B,0x01B0},{0x1F5C,0x01C0},{0x1F5D,0x01D0},
    {0x2152,0x01E0},{0x2154,0x01F0},{0x2155,0x0200},{0x2156,0x0210},{0x2157,0x0220},
    {0x215B,0x0230},{0x215D,0x0240},{0x2850,0x0250},{0x2851,0x0260},{0x2854,0x0270},
    {0x3150,0x0280},{0x3151,0x0290},{0x3152,0x02A0},{0x3153,0x02B0},{0x3154,0x02C0},
    {0x3155,0x02D0},{0x3156,0x02E0},{0x3157,0x02F0},{0x3250,0x0300},{0x3251,0x0310}
};

SrcDataTyp  OdrTable[ ] = {
    {0x1006,0x0000},{0x100E,0x0010},{0x100F,0x0020},{0x1108,0x0030},{0x1136,0x0040},
    {0x1137,0x0050},{0x120D,0x0060},{0x120E,0x0070},{0x120F,0x0080},{0x1210,0x0090},
    {0x1211,0x00A0},{0x1212,0x00B0},{0x1213,0x00C0},{0x1214,0x00D0},{0x1215,0x00E0},
    {0x1216,0x00F0},{0x1217,0x0100},{0x1218,0x0110},{0x1219,0x0120},{0x121A,0x0130},
    {0x121B,0x0140},{0x1351,0x0150},{0x1D6A,0x0160},{0x1D88,0x0170},{0x1D89,0x0180},
    {0x1D8A,0x0190},{0x1D8B,0x01A0},{0x1D8C,0x01B0},{0x1D8D,0x01C0},{0x1D90,0x01D0},
    {0x1D91,0x01E0},{0x1D92,0x01F0},{0x1D93,0x0200},{0x1D9F,0x0210},{0x1E55,0x0220},
    {0x1E56,0x0230},{0x1E59,0x0240},{0x1E5A,0x0250},{0x1E5B,0x0260},{0x1E5D,0x0270},
    {0x1E5F,0x0280},{0x1E60,0x0290},{0x1E63,0x02A0},{0x1E64,0x02B0},{0x1E65,0x02C0},
    {0x1E66,0x02D0},{0x1E67,0x02E0},{0x1E68,0x02F0},{0x1E69,0x0300},{0x1E6A,0x0310},
    {0x1E6B,0x0320},{0x1E70,0x0330},{0x1E71,0x0340},{0x1E72,0x0350},{0x1E74,0x0360},
    {0x1E75,0x0370},{0x1E76,0x0380},{0x1E77,0x0390},{0x1E78,0x03A0},{0x1E79,0x03B0},
    {0x1E7A,0x03C0},{0x1E7B,0x03D0},{0x1E80,0x03E0},{0x1E81,0x03F0},{0x1E82,0x0400},
    {0x1E83,0x0410},{0x1E84,0x0420},{0x1E85,0x0430},{0x1E86,0x0440},{0x1E87,0x0450},
    {0x1E89,0x0460},{0x1E8A,0x0470},{0x1E8C,0x0480},{0x1E8D,0x0490},{0x1E90,0x04A0},
    {0x1E91,0x04B0},{0x1E92,0x04C0},{0x1E93,0x04D0},{0x1E94,0x04E0},{0x1E95,0x04F0},
    {0x1E9A,0x0500},{0x1E9B,0x0510},{0x1EA5,0x0520},{0x1EAB,0x0530},{0x1EB8,0x0540},
    {0x1EB9,0x0550},{0x1EBA,0x0560},{0x1EBB,0x0570},{0x1EBC,0x0580},{0x1EBD,0x0590},
    {0x1EBE,0x05A0},{0x1EBF,0x05B0},{0x1EC0,0x05C0},{0x1EC2,0x05D0},{0x1EC3,0x05E0},
    {0x1EC5,0x05F0},{0x1EC7,0x0600},{0x1ECA,0x0610},{0x1ECB,0x0620},{0x1EDE,0x0630},
    {0x1F68,0x0640},{0x1F69,0x0650},{0x1F78,0x0660},{0x1F79,0x0670},{0x1F7A,0x0680},
    {0x1F82,0x0690},{0x1F83,0x06A0},{0x1F88,0x06B0},{0x1F8A,0x06C0},{0x2057,0x06D0},
    {0x2058,0x06E0},{0x205D,0x06F0},{0x205E,0x0700},{0x205F,0x0710},{0x2061,0x0720},
    {0x2062,0x0730},{0x2063,0x0740},{0x2064,0x0750},{0x2065,0x0760},{0x2069,0x0770},
    {0x206A,0x0780},{0x2073,0x0790},{0x207C,0x07A0},{0x2085,0x07B0},{0x20A0,0x07C0},
    {0x2161,0x07D0},{0x2162,0x07E0},{0x285F,0x07F0},{0x2860,0x0800},{0x2861,0x0810},
    {0x2862,0x0820},{0x2863,0x0830},{0x2864,0x0840},{0x2882,0x0850},{0x2883,0x0860},
    {0x2889,0x0870},{0x288C,0x0880},{0x288D,0x0890},{0x288E,0x08A0},{0x288F,0x08B0},
    {0x2895,0x08C0},{0x2898,0x08D0},{0x2899,0x08E0},{0x28A1,0x08F0},{0x28AC,0x0900},
    {0x28AD,0x0910},{0x28B3,0x0920},{0x3185,0x0930},{0x3186,0x0940},{0x3187,0x0950},
    {0x3189,0x0960},{0x318A,0x0970},{0x318B,0x0980},{0x318C,0x0990},{0x318D,0x09A0},
    {0x318E,0x09B0},{0x3E02,0x09C0},{0x3E07,0x09D0},{0x5258,0x09E0},{0x5350,0x09F0},
    {0x5351,0x0A00},{0x5352,0x0A10},{0x5353,0x0A20},{0x5354,0x0A30},{0x5355,0x0A40},
    {0x5356,0x0A50},{0x5357,0x0A60},{0x5358,0x0A70},{0x535A,0x0A80},{0x535B,0x0A90},
    {0x535E,0x0AA0},{0x5362,0x0AB0},{0x5363,0x0AC0},{0x539C,0x0AD0},{0x53AC,0x0AE0},
    {0x5450,0x0AF0},{0x5451,0x0B00},{0x5452,0x0B10},{0x5453,0x0B20},{0x5454,0x0B30},
    {0x5455,0x0B40},{0x5456,0x0B50},{0x5457,0x0B60},{0x5458,0x0B70},{0x5459,0x0B80},
    {0x545A,0x0B90},{0x545B,0x0BA0}
};


SrcDataTyp  IdrTable[ ] = {
    {0x1004,0x0000},{0x1005,0x0010},{0x100E,0x0020},{0x100F,0x0030},{0x1010,0x0040},
    {0x1011,0x0050},{0x1012,0x0060},{0x1013,0x0070},{0x101B,0x0080},{0x101C,0x0090},
    {0x101D,0x00A0},{0x101E,0x00B0},{0x110A,0x00C0},{0x110B,0x00D0},{0x110C,0x00E0},
    {0x110D,0x00F0},{0x110E,0x0100},{0x110F,0x0110},{0x1110,0x0120},{0x1111,0x0130},
    {0x1112,0x0140},{0x1113,0x0150},{0x1114,0x0160},{0x1115,0x0170},{0x1132,0x0180},
    {0x1137,0x0190},{0x1139,0x01A0},{0x120D,0x01B0},{0x120E,0x01C0},{0x120F,0x01D0},
    {0x1210,0x01E0},{0x1211,0x01F0},{0x1212,0x0200},{0x1213,0x0210},{0x1214,0x0220},
    {0x1215,0x0230},{0x1216,0x0240},{0x1217,0x0250},{0x1218,0x0260},{0x1219,0x0270},
    {0x121A,0x0280},{0x121B,0x0290},{0x1C01,0x02A0},{0x1C03,0x02B0},{0x1C1B,0x02C0},
    {0x1C1C,0x02D0},{0x1C1F,0x02E0},{0x1C20,0x02F0},{0x1C21,0x0300},{0x1C24,0x0310},
    {0x1C25,0x0320},{0x1C26,0x0330},{0x1C27,0x0340},{0x1C28,0x0350},{0x1C29,0x0360},
    {0x1C2A,0x0370},{0x1C2B,0x0380},{0x1C2C,0x0390},{0x1C2D,0x03A0},{0x1C2E,0x03B0},
    {0x1C2F,0x03C0},{0x1C32,0x03D0},{0x1C38,0x03E0},{0x1E03,0x03F0},{0x1E04,0x0400},
    {0x1E05,0x0410},{0x1E06,0x0420},{0x1E06,0x0430},{0x1E07,0x0440},{0x1E07,0x0450},
    {0x1E09,0x0460},{0x1E0C,0x0470},{0x1E0E,0x0480},{0x1E0F,0x0490},{0x1E10,0x04A0},
    {0x1E13,0x04B0},{0x1E15,0x04C0},{0x1E16,0x04D0},{0x1E17,0x04E0},{0x1E1E,0x04F0},
    {0x1E1F,0x0500},{0x1E20,0x0510},{0x1E21,0x0520},{0x1E23,0x0530},{0x1E24,0x0540},
    {0x1E24,0x0550},{0x1E26,0x0560},{0x1E27,0x0570},{0x1E28,0x0580},{0x1E29,0x0590},
    {0x1E2B,0x05A0},{0x1E2C,0x05B0},{0x1E2D,0x05C0},{0x1E2F,0x05D0},{0x1E32,0x05E0},
    {0x1F00,0x05F0},{0x1F01,0x0600},{0x1F02,0x0610},{0x1F0C,0x0620},{0x1F0D,0x0630},
    {0x1F18,0x0640},{0x1F19,0x0650},{0x1F1A,0x0660},{0x1F1B,0x0670},{0x1F1C,0x0680},
    {0x1F54,0x0690},{0x2000,0x06A0},{0x2001,0x06B0},{0x2003,0x06C0},{0x200A,0x06D0},
    {0x2100,0x06E0},{0x2101,0x06F0},{0x2107,0x0700},{0x2109,0x0710},{0x210A,0x0720},
    {0x210C,0x0730},{0x2110,0x0740},{0x3100,0x0750},{0x3106,0x0760},{0x3110,0x0770},
    {0x3111,0x0780},{0x3116,0x0790},{0x3117,0x07A0},{0x3118,0x07B0},{0x3119,0x07C0},
    {0x315A,0x07D0},{0x315B,0x07E0},{0x3D00,0x07F0},{0x3D01,0x0800},{0x3D02,0x0810},
    {0x3D04,0x0820},{0x3D05,0x0830},{0x3D06,0x0840},{0x3D07,0x0850},{0x3D08,0x0860},
    {0x3D09,0x0870},{0x3D0A,0x0880},{0x3D0B,0x0890},{0x3D0C,0x08A0},{0x3D0D,0x08B0},
    {0x3D0E,0x08C0},{0x3D0F,0x08D0},{0x3D10,0x08E0},{0x3D11,0x08F0},{0x3D12,0x0900},
    {0x3D13,0x0910},{0x3D14,0x0920},{0x3D15,0x0930},{0x3D16,0x0940},{0x3D17,0x0950},
    {0x3E01,0x0960},{0x3E15,0x0970},{0x3F01,0x0980},{0x3F03,0x0990},{0x3F05,0x09A0},
    {0x3F06,0x09B0},{0x3F07,0x09C0},{0x3F08,0x09D0},{0x3F0A,0x09E0},{0x3F0C,0x09F0},
    {0x3F0F,0x0A00},{0x3F10,0x0A10},{0x4000,0x0A20},{0x4007,0x0A30},{0x4008,0x0A40},
    {0x4100,0x0A50},{0x4101,0x0A60},{0x4102,0x0A70},{0x4103,0x0A80},{0x4104,0x0A90},
    {0x4105,0x0AA0},{0x4106,0x0AB0},{0x4107,0x0AC0},{0x4108,0x0AD0},{0x4109,0x0AE0},
    {0x7100,0x0AF0},{0x7300,0x0B00},{0x730C,0x0B10},{0x730D,0x0B20},{0x7314,0x0B30},
    {0x7315,0x0B40},{0x731E,0x0B50},{0x731F,0x0B60}
};

//閺佺増宓佺猾璇茬€风悰顭掔礉闁俺绻冨銈堛€冮懢宄扮繁閺佺増宓佺猾璇茬€?
const uint8_t TypeTable[] =
{
/**    0          1          2          3          4          5          6          7          8          9    **/
    NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , //000
    NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , BYTETYPE , BYTETYPE , BYTETYPE , BITTYPE  , //010
    WORDTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , BITTYPE ,  BITTYPE  , //020
    BITTYPE  , BITTYPE  , BYTETYPE , BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , //030
    BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , WORDTYPE , WORDTYPE , //040
    BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , //050
    NULLTYPE , BITTYPE  , BITTYPE  , BITTYPE  , BYTETYPE , BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , //060
    NULLTYPE , NULLTYPE , BYTETYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , WORDTYPE , NULLTYPE , BITTYPE  , //070
    BYTETYPE , BYTETYPE , BITTYPE  , BITTYPE  , BITTYPE  , BYTETYPE , BYTETYPE , BYTETYPE , WORDTYPE , WORDTYPE , //080
    NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , BITTYPE  , BITTYPE  , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , //090
    NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , BITTYPE  , //100
    BITTYPE  , BYTETYPE , BYTETYPE , BYTETYPE , WORDTYPE , WORDTYPE , NULLTYPE , NULLTYPE , NULLTYPE , NULLTYPE , //110
    BYTETYPE , NULLTYPE , NULLTYPE , BITTYPE  , BYTETYPE , BYTETYPE , WORDTYPE , NULLTYPE , NULLTYPE , NULLTYPE   //120
};

uint8_t NewErrBuffer[20];
uint16_t binarySearch(const PageTpye *PageData , const CanMsgType *m_Msg , uint8_t *m_Data )
{
    uint8_t  num = 0 , type = 0 , sendFlag = 0;
    uint8_t  count = 0;
    uint16_t key = 0;                                                           		//鐎规矮绠熼弻銉﹀閸忔娊鏁€?
    uint16_t low=0 , mid = 0 , high= ((PageData->TableSize) & 0x7FFF) - 1;        		//闁瀚ㄩ弻銉﹀閻ㄥ嫭娓舵径褏娈戦懠鍐ㄦ纯

    if(!(PageData->Page) || !m_Msg || !m_Data||(m_Msg->Lenth == 0)){
        return 0xFFFF;																	//閸欏倹鏆熸导鐘插弳瀵倸鐖?
    }

    type = (TypeTable[m_Msg->ID&0x7F] > 1)?0:3;
    key  = ((m_Msg->ID&0x7F)<<8) + (m_Msg->Data[0]&0xFF);
    num  = ((m_Msg->Lenth & 0x7F) - 1)<<type;
  
    if(Errflg){
        for(count = 0;count <= (m_Msg->Lenth & 0x7F -1) ;count ++)
        {
            NewErrBuffer[count*2] = m_Msg->Data[count];
            NewErrBuffer[count*2 + 1] = m_Msg->Data[count]; 
        }
    }
    
    while(low <= high && high != 0){
        mid = (low+high) >> 1 ;
        if((PageData->Page[mid].SrcAdd >= key)&&(PageData->Page[mid].SrcAdd <= key+num)){     //閺佺増宓佸陇鍐婚弻銉嚄閺夆€叉
            while(PageData->Page[mid].SrcAdd >= key){
                if(mid-- == 0){
                    mid = -1;
                    break;
                }
            }
            while((PageData->Page[++mid].SrcAdd < (key + num)) && mid < (PageData->TableSize & 0x7FFF)){
                if(type == 0){
                    if(m_Data[PageData->Page[mid].Value >> 4] != m_Msg->Data[(PageData->Page[mid].SrcAdd - key) + 1]){
                        if(dbgPrintf){
                            if(PageData->Page[mid].SrcAdd == 0x1F5C){
                                (*dbgPrintf)("0x1F5C -- %d -> %d\r\n" , m_Data[PageData->Page[mid].Value >> 4] , m_Msg->Data[(PageData->Page[mid].SrcAdd - key) + 1]);
                            }else if(PageData->Page[mid].SrcAdd == 0x1136){
                                if(dbgPrintf)(*dbgPrintf)("0x1136 -- %d -> %d\r\n", m_Data[PageData->Page[mid].Value >> 4] , m_Msg->Data[(PageData->Page[mid].SrcAdd - key) + 1]);
                            }else if(PageData->Page[mid].SrcAdd == 0x200A){
                                if(dbgPrintf)(*dbgPrintf)("0x200A -- %d -> %d\r\n", m_Data[PageData->Page[mid].Value >> 4] , m_Msg->Data[(PageData->Page[mid].SrcAdd - key) + 1]);
                            }
                        }
                        if(m_Data[PageData->Page[mid].Value >> 4] != 0xFF){
                            NewErrBuffer[((PageData->Page[mid].SrcAdd - key) + 1)*2] = m_Data[PageData->Page[mid].Value >> 4];
                        }
                        m_Data[PageData->Page[mid].Value >> 4] = m_Msg->Data[(PageData->Page[mid].SrcAdd - key) + 1];                        
                        sendFlag ++;
                    }
                }else{
                    if((m_Msg->Data[((PageData->Page[mid].SrcAdd - key)>>3)+1] & (1<<((PageData->Page[mid].SrcAdd - key)%8)))>0?1:0){
                        if((m_Data[PageData->Page[mid].Value >> 4] != 1)){ 
                            if(dbgPrintf){
                                if(PageData->Page[mid].SrcAdd == 0x1F5C){
                                    (*dbgPrintf)("0x1F5C -- 1 -> 0\r\n");
                                }else if(PageData->Page[mid].SrcAdd == 0x1136){
                                    if(dbgPrintf)(*dbgPrintf)("0x1136 -- 1 -> 0\r\n");
                                }else if(PageData->Page[mid].SrcAdd == 0x200A){
                                    if(dbgPrintf)(*dbgPrintf)("0x200A -- 1 -> 0\r\n");
                                }
                            }
                            NewErrBuffer[(((PageData->Page[mid].SrcAdd - key)>>3)+1)*2] &= ~(1<<((PageData->Page[mid].SrcAdd - key)%8));
                            m_Data[PageData->Page[mid].Value >> 4] = 1;
                            sendFlag ++;
                        }

                    }else{
                        if((m_Data[PageData->Page[mid].Value >> 4] != 0)){
                            if(dbgPrintf){
                                if(PageData->Page[mid].SrcAdd == 0x1F5C){
                                    (*dbgPrintf)("0x1F5C -- 0 -> 1\r\n");
                                }else if(PageData->Page[mid].SrcAdd == 0x1136){
                                    if(dbgPrintf)(*dbgPrintf)("0x1136 -- 0 -> 1\r\n");
                                }else if(PageData->Page[mid].SrcAdd == 0x200A){
                                    if(dbgPrintf)(*dbgPrintf)("0x200A -- 0 -> 1\r\n");
                                }
                            }
                            m_Data[PageData->Page[mid].Value >> 4] = 0;
                            NewErrBuffer[(((PageData->Page[mid].SrcAdd - key)>>3)+1)*2] |= (1<<((PageData->Page[mid].SrcAdd - key)%8));
                            sendFlag ++;
                        }
                    }
                }
            }
            if(sendFlag){
                return 0;														//鐠囥儱鎶欳AN閺佺増宓侀崘鍛存付鐟曚胶娈戦崣鍌涙殶閺堝褰夐崠?
            }else{
                return 0xFFFD;                                                  //鐠囥儱鎶欳AN閺佺増宓侀張澶愭付鐟曚胶娈戦崣鍌涙殶閿涘奔绲鹃柈鑺ユ￥閸欐ê瀵?
            }
        }else if(key > (PageData->Page[mid].SrcAdd)){
            low  = mid + 1;                                                     //閸︺劌鎮楅崡濠傚隘闂傚瓨鐓￠幍?
        }else{
            if(mid < 1)mid = 1;
            high = mid - 1;                                                     //閸︺劌澧犻崡濠傚隘闂傚瓨鐓￠幍?
        }
    }
    return 0xFFFE;                                                              //鐠囥儱鎶欳AN閺佺増宓侀弮鐘绘付鐟曚胶娈戦崣鍌涙殶
}


//uint8_t * ErrBufferPtr()
//{
//    
//}
/*******************************************************
* Function Name:    convCanData
* Purpose:          鐏忓棗鎮囬崢鍌氼啀閼奉亜绻侀惃鍑淎N閺佺増宓佺敮褎鐗稿蹇氭祮閹诡澀璐熺紒鐔剁閻ㄥ嫬褰熸径鏍︾缁夊秵鏆熼幑顔界壐瀵?
* Params :          m_CanFrame:閸樼喎顫愰惃鍑淎N閺佺増宓佺敮褎鐗稿?
*                   m_CanMsg:鏉烆剚宕查崥搴ｆ畱閺佺増宓佺敮褎鐗稿?
* Return:           閺?
* Limitation: 	    鐠囥儱鍤遍弫棰佸瘜鐟曚焦妲搁弬閫涚┒閸氬海鐢荤粙瀣碍缁夌粯顦?
*******************************************************/
void convCanData(const CanRxMsg *m_CanFrame , CanMsgType *m_CanMsg)
{
    if(!m_CanFrame || !m_CanMsg){
        return ;
    }
    m_CanMsg->ID = m_CanFrame->ExtId;
    m_CanMsg->Lenth = m_CanFrame->DLC;
    memcpy(m_CanMsg->Data , m_CanFrame->Data , m_CanFrame->DLC );
#if CANDBG
    printToUart(m_CanMsg);
#endif
}

void checkMachStatus(CanMsgType *p_CanMsg , UpDataType *p_ErrBuff)
{
    if(!p_CanMsg || !p_ErrBuff){
        return;
    }
    if((p_CanMsg->CAN1IP == 0x08)&&(p_CanMsg->DataType == 0x13)&&(p_CanMsg->Data[0] == 0x50)){
        p_ErrBuff->Debug  = (p_CanMsg->Data[1] & (1<<0))?1:0;
        p_ErrBuff->OutErr = (p_CanMsg->Data[1] & (1<<1))?1:0;
        p_ErrBuff->Healt  = (p_CanMsg->Data[1] & (1<<2))?1:0;
        p_ErrBuff->TestSig= (p_CanMsg->Data[1] & (1<<3))?1:0;
        p_ErrBuff->InErr  = (p_CanMsg->Data[1] & (1<<4))?1:0;
        p_ErrBuff->IndOff = (p_CanMsg->Data[1] & (1<<5))?1:0;
    }
    if((p_CanMsg->CAN1IP == 0x08)&&(p_CanMsg->DataType == 0x1F)&&(p_CanMsg->Data[0] == 0x50)){
        p_ErrBuff->FastTest  = (p_CanMsg->Data[1] & (1<<0))?1:0;
        p_ErrBuff->DebugOver = (p_CanMsg->Data[2] & (1<<1))?1:0;
    }
}

/*******************************************************
* Function Name: 	fillErrData
* Purpose: 		鐏忓挵AN閺佺増宓侀幐澶嬬壐瀵繐锝為崗鍛煂buff娑擃厼绶熼崣鎴︹偓?
* Params :              閺?
* Return: 		閺?
* Limitation: 	        閺?
*******************************************************/

uint8_t Errflg = 1,preSta = 1;
#if 1
uint16_t fillErrData(CanMsgType *p_CanMsg , UpDataType *p_ErrBuff)
{
    uint8_t err = 0;
    uint8_t i = 0 , check = 0;
    static uint32_t unixTime;
    if(!p_CanMsg || !p_ErrBuff){
        return 0;
    }else if(p_CanMsg->FunCode != 0xF7){
        return 0;
    }
    if(preSta != Errflg){
        preSta = Errflg;
        timeOutPack(&g_Upload);
        return 0;
    }
    if(p_ErrBuff->RealDataLen < 2){
        p_ErrBuff->RealDataLen = 2;
    }else if((p_ErrBuff->RealDataLen >= 2)&&(p_ErrBuff->RealDataLen < 6)){            //1KB Buff鐎涙ɑ鏂侀弽鐓庣础娑撶閺佺増宓侀梹鍨][閺冨爼妫块幋鐮瀃閺堝鏅ラ弫鐗堝祦]
        unixTime = getUnixTimeStamp();
        packTime = 0x8000;
        memcpy(&p_ErrBuff->RealBuffer[2] , &unixTimeStamp , 4);
        p_ErrBuff->RealDataLen += 4;
    }else if((p_ErrBuff->RealDataLen >= 6)&&(p_ErrBuff->RealDataLen < 800)){
        if(Errflg == 0){
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++] = (uint8_t)(getUnixTimeStamp() - unixTime);
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = 0xAA;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = 0xAA;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->Lenth | 0x80;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->FunCode;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->CAN2IP;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->CAN1IP;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->DataType;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            for(i=0;i<p_CanMsg->Lenth;i++){
                p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->Data[i];
                check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            }
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++] = check;
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++] = 0xFF;
        }else if(Errflg == 1){
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++] = (uint8_t)(getUnixTimeStamp() - unixTime);
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = 0xBB;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = 0xBB;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->Lenth | 0x80;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->FunCode;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->CAN2IP;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->CAN1IP;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = p_CanMsg->DataType;
            check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            for(i=0;i<p_CanMsg->Lenth*2;i++){
                p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen] = NewErrBuffer[i];
                check ^= p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++];
            }

            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++] = check;
            p_ErrBuff->RealBuffer[p_ErrBuff->RealDataLen++] = 0xFF;
        }
    }else if((p_ErrBuff->RealDataLen >= 800)&&(p_ErrBuff->RealDataLen <= 1024)){
        memcpy(&p_ErrBuff->RealBuffer[0] , &p_ErrBuff->RealDataLen , 2);
        if(SendBuffer[sendCnt].DataLen == 0){
            memcpy(SendBuffer[sendCnt].DataBuf , p_ErrBuff->RealBuffer , p_ErrBuff->RealDataLen);
            err = OSQPost(QSemSend,(void *)&SendBuffer[sendCnt]);
            if(OS_ERR_NONE == err){
                SendBuffer[sendCnt].DataLen = p_ErrBuff->RealDataLen | 0x8000;  //强制标记接收完成
                sendCnt++;
            }else{
                if(dbgPrintf)(*dbgPrintf)("Lost Can Data!\r\n");
            }
        }
        if(sendCnt >= 5){
            sendCnt = 0;
        }
        WriteData2Flash(p_ErrBuff);
        p_ErrBuff->RealDataLen  = 0x0000;
        return 0;
    }else p_ErrBuff->RealDataLen  = 0;
    return 0;
}
#endif

void WriteData2Flash(UpDataType *writrBuff)
{
    uint8_t err;
    if(!writrBuff){
        return ;
    }   
    if((writrBuff->writeFlashAddr % SECTOR_SIZE) == 0){
        sFLASH_EraseSector(writrBuff->writeFlashAddr);
    }else if(writrBuff->writeFlashAddr % CAN_WRITE_SIZE){
        writrBuff->writeFlashAddr -= writrBuff->writeFlashAddr % CAN_WRITE_SIZE;
    }
    OSMutexPend(MutexFlash , 0 , &err);
    sFLASH_WriteBuffer(writrBuff->RealBuffer, writrBuff->writeFlashAddr , (writrBuff->RealDataLen & 0x7FFF));
    OSMutexPost(MutexFlash);   
    if(writrBuff->writeFlashAddr >= ERR_CAHCE_END_ADDR){
        writrBuff->writeFlashAddr  = 0;
    }else{
        writrBuff->writeFlashAddr += CAN_WRITE_SIZE;
    }
}

void ReadData2Flash(UpDataType *readBuff)
{
    uint8_t err;
    if(!readBuff){
        return ;
    }   
    OSMutexPend(MutexFlash , 0 , &err);
    sFLASH_ReadBuffer((uint8_t*)&readBuff->ErrDataLen,   readBuff->readFlashAddr , 2);
    if(readBuff->ErrDataLen <= 1024 ){
        sFLASH_ReadBuffer(readBuff->ErrBuffer, readBuff->readFlashAddr , (readBuff->ErrDataLen & 0x7FFF));
        memcpy(&readBuff->readFlashTime , &readBuff->ErrBuffer[2] , 4);
    }else{
        readBuff->ErrDataLen    = 0;
        readBuff->readFlashTime = 0;
    }
    OSMutexPost(MutexFlash);
    if(readBuff->readFlashTime == 0xFFFFFFFF){
        readBuff->readFlashTime = 0;
    }
    if(readBuff->readFlashAddr <= ERR_CAHCE_START_ADDR){
        readBuff->readFlashAddr  = ERR_CAHCE_END_ADDR - CAN_WRITE_SIZE;
    }else{
        readBuff->readFlashAddr -= CAN_WRITE_SIZE;
    }
}

void savePowerDownInfo(UpDataType *saveData)
{
    uint8_t err;
    if(!saveData){
        return ;
    } 
    OSMutexPend(MutexFlash , 0 , &err);
    sFLASH_EraseSubSector(ERR_INFO_START_ADDR);
    sFLASH_WriteBuffer((uint8_t *)saveData , ERR_INFO_START_ADDR , sizeof(UpDataType));
    OSMutexPost(MutexFlash);
    
    OSMutexPend(MutexFlash , 0 , &err);
    sFLASH_EraseSubSector(VAR_START_ADDR);
    sFLASH_WriteBuffer((uint8_t *)&var , VAR_START_ADDR , sizeof(var));
    OSMutexPost(MutexFlash);
}


/*******************************************************
* Function Name:    dec2BCD
* Purpose:          鐏忓棔绔存稉顏勫磩鏉╂稑鍩楅弫鐗堝祦鏉烆剚宕叉稉绡塁D閻焦鐗稿?
* Params :          m_DEC閿涙岸娓剁憰浣芥祮閹广垻娈戦崡浣界箻閸掕埖鏆熼幑?
* Return:           uint32_t閿涙俺娴嗛幑銏犵暚閹存劗娈戦弫鐗堝祦
* Limitation: 	    婢跺嫮鎮婇崡浣界箻閸掓儼瀵栭崶缈犺礋閿?~99999999
*******************************************************/
uint32_t dec2BCD(uint32_t m_DEC)
{
    uint32_t m_rData = 0;
    if(m_DEC > 0xFFFF){
        m_rData = ((m_DEC/10000000%10)<<28)+((m_DEC/1000000%10)<<24)+((m_DEC/100000%10)<<20)+((m_DEC/10000%10)<<16)+
                  ((m_DEC/1000%10)<<12)+((m_DEC/100%10)<<8)+((m_DEC/10%10)<<4)+(m_DEC%10);

    }else if(m_DEC > 0xFF){
        m_rData = ((m_DEC/1000%10)<<12)+((m_DEC/100%10)<<8)+((m_DEC/10%10)<<4)+(m_DEC%10);
    }else{
        m_rData = ((m_DEC/10%10)<<4)+(m_DEC%10);
    }
    return m_rData;
}

/*******************************************************
* Function Name:    bcd2DEC
* Purpose:          鐏忓棔绔存稉鐙烠D閻焦鐗稿蹇氭祮閹诡澀璐熼崡浣界箻閸掕埖鏆熼幑?
* Params :          m_BCD閿涙岸娓剁憰浣芥祮閹广垻娈態CD閻焦鏆熼幑?
* Return:           uint32_t閿涙俺娴嗛幑銏犵暚閹存劗娈戦弫鐗堝祦
* Limitation: 	    婢跺嫮鎮婇崡浣界箻閸掓儼瀵栭崶缈犺礋閿?~99999999
*******************************************************/
uint32_t bcd2DEC(uint32_t m_BCD)
{
    uint32_t m_rData = 0;
    if(m_BCD > 0x9999){
        m_rData += (((m_BCD >> 28)&0xF)%10 * 10000000) + (((m_BCD >> 24)&0xF)%10 * 1000000) + (((m_BCD >> 20)&0xF)%10 * 100000)+ (((m_BCD >> 16)&0xF)%10 * 10000);
        m_BCD &= 0xFFFF;
    }
    if(m_BCD > 0x99){
        m_rData += (((m_BCD >> 12)&0xF)%10 * 1000) + (((m_BCD >> 8)&0xF)%10 * 100);
        m_BCD &= 0xFF;
    }
    m_rData += (((m_BCD >> 4)&0xF)%10 * 10) + (m_BCD &0xF)%10;
    return m_rData;
}


void setACTime(struct tm *m_Time)
{
    uint8_t i = 0;
    CanTxMsg CanTime;
    time_t  n_Time = 0;
    CanTime.IDE = CAN_ID_EXT;
    CanTime.RTR = CAN_RTR_DATA;
    CanTime.FunCode  = 0x24;
    CanTime.CAN2IP   = 0x14;
    CanTime.CAN1IP   = 0x00;
    CanTime.DataType = 0x00;
    CanTime.DLC      = 8;
    CanTime.Data[0]  = (uint8_t)dec2BCD(m_Time->tm_year - 100);
    CanTime.Data[1]  = (uint8_t)dec2BCD(m_Time->tm_mon + 1);
    CanTime.Data[2]  = (uint8_t)dec2BCD(m_Time->tm_mday);
    CanTime.Data[3]  = (uint8_t)dec2BCD(m_Time->tm_hour);
    CanTime.Data[4]  = (uint8_t)dec2BCD(m_Time->tm_min);
    CanTime.Data[5]  = (uint8_t)dec2BCD(m_Time->tm_sec); 
    n_Time = mktime(m_Time);
    m_Time = gmtime(&n_Time);    
    CanTime.Data[6]  = (uint8_t)dec2BCD(m_Time->tm_wday);
    CanTime.Data[7]  = 0x02;
    for(i=0;i<5;i++){
        CAN_Transmit(CAN1 , &CanTime);
        OSTimeDlyHMSM(0,0,0,10);
    }
}

struct tm* getACTime(CanMsgType *p_CanMsg , struct tm *p_Time)
{
    static  struct tm s_Time = {0};
    //struct tm s_Time = {0};
    if((p_CanMsg->CAN2IP == 0x08)&&(p_CanMsg->FunCode == 0x24)){                
        if(p_CanMsg->Data[7] & 0x02){
            if(p_CanMsg->Data[0] > 0x35)return NULL;
            if(p_CanMsg->Data[1] > 0x12)return NULL;
            if(p_CanMsg->Data[2] > 0x31)return NULL;
            if(p_CanMsg->Data[3] > 0x23)return NULL;
            if(p_CanMsg->Data[4] > 0x59)return NULL;
            if(p_CanMsg->Data[5] > 0x59)return NULL;
            if(p_CanMsg->Data[6] > 0x06)return NULL;
            s_Time.tm_year = bcd2DEC(p_CanMsg->Data[0]) + 100;
            s_Time.tm_mon  = bcd2DEC(p_CanMsg->Data[1]) - 1;
            s_Time.tm_mday = bcd2DEC(p_CanMsg->Data[2]);
            s_Time.tm_hour = bcd2DEC(p_CanMsg->Data[3]);
            s_Time.tm_min  = bcd2DEC(p_CanMsg->Data[4]);
            s_Time.tm_sec  = bcd2DEC(p_CanMsg->Data[5]);
            s_Time.tm_wday = bcd2DEC(p_CanMsg->Data[6]);
            setLocalTime(&s_Time);
            return p_Time;
        }
    }
    return NULL;
}

void writeSIMToCAN(uint8_t simStatus)
{
    CanTxMsg SimData;
    SimData.DLC      = 2;
    SimData.RTR      = CAN_RTR_DATA;
    SimData.IDE      = CAN_ID_EXT;
    SimData.FunCode  = 0xF7;
    SimData.CAN1IP   = 0x7E;
    SimData.CAN2IP   = 0x7F;
    SimData.DataType =  0x13;
    SimData.Data[0]  =  0x00;
    SimData.Data[1]  =  (simStatus > 0)?1:0;
    CAN_Transmit(CAN1 , &SimData);
}

uint16_t timeOutPack(UpDataType *p_ErrBuff)
{        
    uint8_t err = 0;
    if(!p_ErrBuff){
        return 0;
    }
    if(p_ErrBuff->RealDataLen > 6){
        memcpy(&p_ErrBuff->RealBuffer[0] , &p_ErrBuff->RealDataLen , 2);

        memcpy(SendBuffer[sendCnt].DataBuf , p_ErrBuff->RealBuffer , p_ErrBuff->RealDataLen);
        err = OSQPost(QSemSend,(void *)&SendBuffer[sendCnt]);
        if(OS_ERR_NONE == err){
            SendBuffer[sendCnt].DataLen = p_ErrBuff->RealDataLen | 0x8000;  //强制标记接收完成
            sendCnt++;
        }
        if(sendCnt >= 5){
            sendCnt = 0;
        }
        
        WriteData2Flash(p_ErrBuff);
        p_ErrBuff->RealDataLen  = 0x0000;
        return 0;
    }
    return 0;
}

/*******************************************************
* Function Name:    printToUart
* Purpose:          閹垫挸宓冩稉鈧敮顪堿N閺佺増宓侀崚棰佽閸欙絼绗?
* Params :          m_CanMsg閿涙岸娓剁憰浣瑰ⅵ閸楁壆娈慍AN閺佺増宓侀崷鏉挎絻
* Return:           閺?
* Limitation: 	    閻劋绨拫鍐槸
*******************************************************/
void printToUart(const CanMsgType *m_CanMsg)
{
    uint8_t i = 0 , m_Check = 0;
    m_Check = 0xAA^0xAA^(0x80|m_CanMsg->Lenth)^(m_CanMsg->FunCode)^(m_CanMsg->CAN2IP)^(m_CanMsg->CAN1IP)^(m_CanMsg->DataType);
    
    if(dbgPrintf)(*dbgPrintf)("\r\n--> AA AA %02X %02X %02X %02X %02X" , 0x80|m_CanMsg->Lenth , m_CanMsg->FunCode , m_CanMsg->CAN2IP , m_CanMsg->CAN1IP , m_CanMsg->DataType);
    for( i=0; i<m_CanMsg->Lenth; i++ ){
        m_Check ^= m_CanMsg->Data[i];
        if(dbgPrintf)(*dbgPrintf)(" %02X" , m_CanMsg->Data[i]);
    }
    if(dbgPrintf)(*dbgPrintf)(" %02X FF\r\n" , m_Check);
}


