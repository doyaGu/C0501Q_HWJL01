/*
 *
 *  Copyright (C) 2012 ITE TECH. INC.   All Rights Reserved.
 *
 */

 #pragma once

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*IDBCALLBACK)(void* data);
int idbIsRunning();
int idbIsConnected();
int idbSend2Host(void* data, int size, int wait);
int idbSend2HostAsync(void* data, int size, int wait);
int idbSetCallback(IDBCALLBACK idbCallback);
#ifdef __cplusplus
extern "C" }
#endif
