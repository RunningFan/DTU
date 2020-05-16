#ifndef _MYSCANF_H_
#define _MYSCANF_H_
#include "stm32f10x.h"

typedef struct 
{
    char identify[20];
    char comand[20];
    char contant[100]; 
    char text[200]; 
}SCANF_TYPE;

extern SCANF_TYPE myScanf;

uint32_t catMyVariable(uint8_t *pData , uint16_t dataLen , SCANF_TYPE *sScanf);



#endif
