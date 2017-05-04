/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Header File for Model Random Function
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: modelRandom.h,v 1.1.1.1 2010/05/05 09:00:48 jiunming Exp $
*/

#ifndef _MODEL_RANDOM_
#define _MODEL_RANDOM_

#include "rtl_types.h"


enum ENUM_RANDOM_SEED_STRATAGEM
{
	ENUM_SEED_SYSTIME,
	ENUM_SEED_FIXED,
};

extern enum ENUM_RANDOM_SEED_STRATAGEM modelRandomSeedStratagem;
extern uint32 modelRandomSeed;


int32 runModelRandom(uint32 userId,  int32 argc,int8 **saved);


#endif
