#include <stdio.h>
#include <string.h>
#include "ite/ith.h"
#include "ctrlboard.h"

// need to install VS90SP1-KB980263-x86.exe for vs2008
#pragma execution_character_set("utf-8")

static const char* stringGuardSensorArray[] =
{
    "Emergency",
    "Infrared",
    "Door",
    "Window",
    "Smoke",
    "Gas",
    "Area",
    "Rob",
};

static const char* stringGuardSensorArrayChs[] =
{
    "紧急",
    "红外",
    "门磁",
    "窗感",
    "烟感",
    "瓦斯",
    "周界",
    "劫持",
};

static const char* stringGuardSensorArrayCht[] =
{
    "緊急",
    "紅外",
    "門磁",
    "窗感",
    "煙感",
    "瓦斯",
    "周界",
    "劫持",
};

const char* StringGetGuardSensor(GuardSensor sensor)
{
    if (theConfig.lang == LANG_CHS)
    {
        return stringGuardSensorArrayChs[sensor];
    }
    else if (theConfig.lang == LANG_CHT)
    {
        return stringGuardSensorArrayCht[sensor];
    }
    else
    {
        return stringGuardSensorArray[sensor];
    }
}

const char* StringGetWiFiConnected(void)
{
    if (theConfig.lang == LANG_CHS)
    {
        return "已连接";
    }
    else if (theConfig.lang == LANG_CHT)
    {
        return "已連接";
    }
    else
    {
        return "Connected";
    }
}
