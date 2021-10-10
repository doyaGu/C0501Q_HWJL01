#include <stdio.h>
#include <string.h>
#include "ite/ith.h"
#include "airconditioner.h"

// need to install VS90SP1-KB980263-x86.exe for vs2008
#pragma execution_character_set("utf-8")

static const char* stringWarnArray[] =
{
    "No Error",
    "Indoor temperature sensor failure",
    "Indoor evaporator sensor failure",
    "Condenser sensor failure",
    "Indoor E-side parameter failure",
    "Outdoor E side or temperature sensor failure",
    "Outdoor protection",
    "Outdoor fan stall",
    "Indoor and outdoor communication failure",
    "Lifting device failure",
    "IPM module failure",
    "Indoor DC fan failure",
    "Zero crossing fault detection",
    "Compressor phase failure",
    "Compressor phase sequence reverse fault",
    "Exhaust temperature protection",
    "A multi-mode conflict",
    "Voltage protection",
    "Press the top of the temperature protection",
    "Outdoor temperature is too low protection",
    "Inner tube temperature protection",
    "External temperature protection",
    "Compressor position protection",
    "Evaporator high and low temperature limit protection",
    "High temperature limit frequency protection of condenser",
    "Anti-cold",
    "Safety grid protection",
    "Compressor Exhaust High Temperature Limiting Protection",
    "Current limit frequency protection",
    "Compressor Current Overload Protection",
    "Voltage limiting protection",
    "Humidity sensor failure",
    "Meter module failure",
    "Outdoor ambient temperature sensor failure",
    "Outdoor exhaust temperature sensor failure",
    "The communication between the backlight board and the main control board is faulty",
    "The communication between the display and the backlight is faulty",
};

static const char* stringWarnArrayChs[] =
{
    "无",
    "室内温度传感器故障",
    "室内蒸发器传感器故障",
    "冷凝器传感器故障",
    "室内E方参数故障",
    "室外E方或温度传感器故障",
    "室外保护",
    "室外风机失速",
    "室内外通信故障",
    "升降装置故障",
    "IPM模块故障",
    "室内直流风机故障",
    "过零检测故障",
    "压缩机缺相故障",
    "压缩机相序反接故障",
    "排气温度保护",
    "一拖多模式冲突",
    "电压保护",
    "压机顶部温度保护",
    "室外温度过低保护",
    "内管温保护",
    "外管温保护",
    "压缩机位置保护",
    "蒸发器高低温限频保护",
    "冷凝器高温限频保护",
    "防冷风",
    "安全栅格保护",
    "压缩机排气高温限频保护",
    "电流限频保护",
    "压缩机电流过载保护",
    "电压限频保护",
    "湿度传感器故障",
    "电表模块故障",
    "室外环境温度传感器故障",
    "室外排气温度传感器故障",
    "背光板与主控板通讯故障",
    "显示屏与背光板通信故障",
};

static const char* stringWarnArrayCht[] =
{
    "無",
    "室內溫度傳感器故障",
    "室內蒸發器傳感器故障",
    "冷凝器傳感器故障",
    "室內E方參數故障",
    "室外E方或溫度傳感器故障",
    "室外保護",
    "室外風機失速",
    "室內外通信故障",
    "升降裝置故障",
    "IPM模塊故障",
    "室內直流風機故障",
    "過零檢測故障",
    "壓縮機缺相故障",
    "壓縮機相序反接故障",
    "排氣溫度保護",
    "一拖多模式衝突",
    "電壓保護",
    "壓機頂部溫度保護",
    "室外溫度過低保護",
    "內管溫保護",
    "外管溫保護",
    "壓縮機位置保護",
    "蒸發器高低溫限頻保護",
    "冷凝器高溫限頻保護",
    "防冷風",
    "安全柵格保護",
    "壓縮機排氣高溫限頻保護",
    "電流限頻保護",
    "壓縮機電流過載保護",
    "電壓限頻保護",
    "濕度傳感器故障",
    "電錶模塊故障",
    "室外環境溫度傳感器故障",
    "室外排氣溫度傳感器故障",
    "背光板與主控板通訊故障",
    "顯示屏與背光板通信故障",
};

static const char* stringDateArray[] =
{
    "%04d/%02d/%02d",
    "%04d年%02d月%02日",
    "%04d年%02d月%02日"
};

static const char* stringPowerOnArray[] =
{
    "Power On",
    "開機",
    "开机",
};

static const char* stringPowerOffArray[] =
{
    "Power Off",
    "關機",
    "关机",
};

const char* StringGetWarning(ExternalWarnType warn)
{
    if (theConfig.lang == LANG_CHS)
    {
        return stringWarnArrayChs[warn];
    }
    else if (theConfig.lang == LANG_CHT)
    {
        return stringWarnArrayCht[warn];
    }
    else
    {
        return stringWarnArray[warn];
    }
}

const char* StringGetDateFormat(void)
{
    return stringDateArray[theConfig.lang];
}

const char* StringGetPowerOn(void)
{
    return stringPowerOnArray[theConfig.lang];
}

const char* StringGetPowerOff(void)
{
    return stringPowerOffArray[theConfig.lang];
}
