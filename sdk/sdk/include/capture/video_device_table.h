#ifndef __VIDEO_DEVICE_TABLE_H__
#define __VIDEO_DEVICE_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                Constant Definition
//=============================================================================


//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================


//=============================================================================
//                Global Data Definition
//=============================================================================


typedef struct CAP_HDMI_TIMINFO_TABLE_TAG
{
    MMP_UINT16 Index;
    MMP_UINT16 HActive;
    MMP_UINT16 VActive;
    MMP_UINT16 HTotal;
    MMP_UINT16 VTotal;
    MMP_UINT16 FrameRate;
    MMP_UINT16 Skippattern;
    MMP_UINT16 SkipPeriod;
    MMP_UINT16 HPolarity:1;
    MMP_UINT16 VPolarity:1;
    MMP_UINT16 HStar;
    MMP_UINT16 HEnd;
    MMP_UINT16 VStar1;
    MMP_UINT16 VEnd1;
    MMP_UINT16 VStar2;
    MMP_UINT16 VEnd2;       
        
}CAP_HDMI_TIMINFO_TABLE;

//IT6607/IT6604 HDMI Table
static CAP_HDMI_TIMINFO_TABLE HDMI_TABLE [] = {
    //Index, HActive, VActive, HTotal, VTotal, FrameRate,               SkipPattern,  SkipPeriod,  Hpor,   Vpor   HStar,     HEnd,   VStar1,  VEnd1,  VStar2,  VEnd2      
    {0,     720,    480,    858,    262,    MMP_CAP_FRAMERATE_29_97HZ,      0x0000,      0xF,       0,      0,      118,    837,        18,    257,     281,    520       },//480i60    
    {1,     720,    480,    858,    525,    MMP_CAP_FRAMERATE_59_94HZ,      0x0000,      0xF,       0,      0,      121,    840,        36,    515,       0,      0       },//480P60    
    {2,     720,    576,    864,    312,    MMP_CAP_FRAMERATE_25HZ,         0x0000,      0xF,       0,      0,      131,    850,        22,    309,     335,    622       },//576i50       
    {3,     720,    576,    864,    625,    MMP_CAP_FRAMERATE_50HZ,         0x0000,      0xF,       0,      0,      131,    850,        44,    619,       0,      0       },//576P50   
    {4,     1280,   720,   1650,    750,    MMP_CAP_FRAMERATE_59_94HZ,      0x0000,      0xF,       1,      1,      259,   1538,        25,    744,       0,      0       },//720P60       
    {5,     1280,   720,   1980,    750,    MMP_CAP_FRAMERATE_50HZ,         0x0000,      0xF,       1,      1,      259,   1538,        25,    744,       0,      0       },//720P50
    {6,     1920,  1080,   2200,   1125,    MMP_CAP_FRAMERATE_59_94HZ,      0x0000,      0xF,       1,      1,      191,   2110,        41,   1120,       0,      0       },//1080P60    
    {7,     1920,  1080,   2640,   1125,    MMP_CAP_FRAMERATE_50HZ,         0x0000,      0xF,       1,      1,      191,   2110,        41,   1120,       0,      0       },//1080P50    
    {8,     1920,  1080,   2200,    562,    MMP_CAP_FRAMERATE_29_97HZ,      0x0000,      0xF,       1,      1,      191,   2110,        20,    559,     583,   1122       },//1080i60    
    {9,     1920,  1080,   2640,    562,    MMP_CAP_FRAMERATE_25HZ,         0x0000,      0xF,       1,      1,      191,   2110,        20,    559,     583,   1122       },//1080i50
    {10,    1920,  1080,   2750,   1125,    MMP_CAP_FRAMERATE_24HZ,         0x0000,      0xF,       1,      1,      191,   2110,        41,   1120,       0,      0       },//1080P24
    {11,     640,   480,    800,    525,    MMP_CAP_FRAMERATE_59_94HZ,      0x0000,      0xF,       0,      0,      143,    782,        35,    514,       0,      0       },//640p60  
    {12,    1920,  1080,   2200,   1125,    MMP_CAP_FRAMERATE_29_97HZ,      0x0000,      0xF,       1,      1,      191,   2110,        41,   1120,       0,      0       },//1080P30    
    {13,    1920,  1080,   2640,   1125,    MMP_CAP_FRAMERATE_25HZ,         0x0000,      0xF,       1,      1,      191,   2110,        41,   1120,       0,      0       },//1080P25    
    //CAP_HDMI_INPUT_VESA   
};

#ifdef COMPONENT_DEV
typedef struct CAP_CAT9883_TIMINFO_TABLE_TAG
{
    MMP_UINT16 Index;
    MMP_UINT16 HActive;
    MMP_UINT16 VActive;
    MMP_UINT16 Rate;
    MMP_UINT16 FrameRate;
    MMP_UINT16 Skippattern;
    MMP_UINT16 SkipPeriod;
    MMP_UINT16 ROIPosX;
    MMP_UINT16 ROIPosY;
    MMP_UINT16 ROIWidth;
    MMP_UINT16 ROIHeight;    
    MMP_UINT16 HPolarity:1;
    MMP_UINT16 VPolarity:1;
    MMP_UINT16 CrossLineDE;
    MMP_UINT16 DlyVS;
    MMP_UINT16 HSPosEdge;
    MMP_UINT16 YPbPrTopVSMode;  
    MMP_UINT16 UCLKInv;
    MMP_UINT16 HStar;
    MMP_UINT16 HEnd;
    MMP_UINT16 VStar1;
    MMP_UINT16 VEnd1;
    MMP_UINT16 VStar2;
    MMP_UINT16 VEnd2;

}CAP_CAT9883_TIMINFO_TABLE;


//CAT9883 Table
static CAP_CAT9883_TIMINFO_TABLE CAT9883_TABLE [] = {
//       Index, HActive, VActive, Rate, FrameRate, SkipPattern, SkipPeriod,  ROIX, ROIY,  ROIW,   ROIH,      Hpor, Vpor, CrossDE, DlyVS, HSPosEdge, VSMode, UCLKInv,   HStar,  HEnd, VStar1, VEnd1,  VStar2, VEnd2,
    {0,     720,    480,    2997,   MMP_CAP_FRAMERATE_29_97HZ, 0x0000, 0xF,     8,    0,   704,    480,        0,   0,   0,   1,   0,   0,   0,                         119,        838,    17,     256,  280,   519   },//480i60
    {1,     720,    480,    5994,   MMP_CAP_FRAMERATE_59_94HZ, 0x0000, 0xF,     8,    0,   704,    480,        1,   1,   1,   1,   1,   0,   0,                         850,        711,    29,     509,    0,     0   },//480P60, uneven h edge, better fine tune UV ordering                                                                                                                  
    {2,     720,    576,    2500,   MMP_CAP_FRAMERATE_25HZ,    0x0000, 0xF,     8,    0,   704,    576,        0,   1,   0,   1,   0,   0,   0,                         129,        848,    20,     307,  333,   620   },//576i50 
    {3,     720,    576,    5000,   MMP_CAP_FRAMERATE_50HZ,    0x0000, 0xF,     8,    0,   704,    576,        1,   1,   1,   1,   1,   0,   0,                         860,        715,    38,     614,    0,     0   },//576P50                                               
    {4,    1280,    720,    5994,   MMP_CAP_FRAMERATE_59_94HZ, 0x0000, 0xF,     8,    0,  1264,    720,        0,   1,   0,   1,   0,   0,   0,                         170,       1449,    24,     743,    0,     0   },//720P60    
    {5,    1280,    720,    5000,   MMP_CAP_FRAMERATE_50HZ,    0x0000, 0xF,     8,    0,  1264,    720,        0,   1,   0,   1,   0,   0,   0,                         172,       1451,    24,     743,    0,     0   },//720P50                                                                                                              
    {6,    1920,   1080,    5994,   MMP_CAP_FRAMERATE_59_94HZ, 0x0000, 0xF,     8,    0,  1904,   1080,        0,   1,   0,   1,   0,   0,   1,                          62,       1981,    35,    1114,    0,     0   },//1080P60
    {7,    1920,   1080,    5000,   MMP_CAP_FRAMERATE_50HZ,    0x0000, 0xF,     8,    0,  1904,   1080,        0,   1,   0,   1,   0,   0,   1,                          66,       1985,    35,    1114,    0,     0   },//1080P50                                                                                                             
    {8,    1920,   1080,    2997,   MMP_CAP_FRAMERATE_29_97HZ, 0x0000, 0xF,     8,    0,  1904,   1080,        0,   1,   0,   1,   0,   1,   0,                         104,       2023,    19,     558,  582,  1121   },//1080i60
    {9,    1920,   1080,    2500,   MMP_CAP_FRAMERATE_25HZ,    0x0000, 0xF,     8,    0,  1904,   1080,        0,   1,   1,   1,   0,   1,   0,                        2632,       1911,    18,     558,  581,  1121   },//1080i50
    {10,   1920,   1080,    2400,   MMP_CAP_FRAMERATE_24HZ,    0x0000, 0xF,     8,    0,  1904,   1080,        0,   1,   0,   1,   0,   0,   0,                          68,       1987,    35,    1114,    0,     0   },//1080p24
                                                                                                                                         
    {11,    640,    480,    6000,   MMP_CAP_FRAMERATE_60HZ,    0x0000, 0xF,     0,    0,   640,    480,        0,   0,   0,   0,   0,   0,   0,                       144+6,      783+6,  35-2,   514-2,    0,     0   },//640x480 60Hz
    {12,    640,    480,    7200,   MMP_CAP_FRAMERATE_72HZ,    0x0000, 0xF,     0,    0,   640,    480,        0,   0,   0,   0,   0,   0,   0,                       168+6,      807+6,  31-3,   510-3,    0,     0   },//640x480 72Hz
    {13,    640,    480,    7500,   MMP_CAP_FRAMERATE_75HZ,    0x0000, 0xF,     0,    0,   640,    480,        0,   0,   0,   0,   0,   0,   0,                       184+6,      823+6,  19-3,   498-3,    0,     0   },//640x480 75Hz
    {14,    640,    480,    8500,   MMP_CAP_FRAMERATE_85HZ,    0x0000, 0xF,     0,    0,   640,    480,        0,   0,   0,   0,   0,   0,   0,                       136+6,      775+6,  28-3,   507-3,    0,     0   },//640x480 85Hz
                                                                                                                                                                                         
    {15,    800,    600,    5600,   MMP_CAP_FRAMERATE_56HZ,    0x0000, 0xF,     0,    0,   800,    600,        0,   0,   0,   0,   0,   0,   0,                       200+6,      999+6,    24,     623,    0,     0   },//800x600 56Hz
    {16,    800,    600,    6000,   MMP_CAP_FRAMERATE_60HZ,    0x0000, 0xF,     0,    0,   800,    600,        0,   0,   0,   0,   0,   0,   0,                       216+6,     1015+6,    27,     626,    0,     0   },//800x600 60Hz
    {17,    800,    600,    7200,   MMP_CAP_FRAMERATE_72HZ,    0x0000, 0xF,     0,    0,   800,    600,        0,   0,   0,   0,   0,   0,   0,                       184+6,      983+6,    29,     628,    0,     0   },//800x600 72Hz
    {18,    800,    600,    7500,   MMP_CAP_FRAMERATE_75HZ,    0x0000, 0xF,     0,    0,   800,    600,        0,   0,   0,   0,   0,   0,   0,                       240+6,     1039+6,    24,     623,    0,     0   },//800x600 75Hz
    {19,    800,    600,    8500,   MMP_CAP_FRAMERATE_85HZ,    0x0000, 0xF,     0,    0,   800,    600,        0,   0,   0,   0,   0,   0,   0,                       216+6,     1015+6,    30,     629,    0,     0   },//800x600 85Hz
                                                                                                                                                                                         
    {20,    1024,   768,    6000,   MMP_CAP_FRAMERATE_60HZ,    0x0000, 0xF,     0,    0,   1024,   768,        0,   0,   0,   0,   0,   0,   0,                       296+6,     1319+6,  35-6,   802-6,    0,     0   },//1024x768 60Hz
    {21,    1024,   768,    7000,   MMP_CAP_FRAMERATE_70HZ,    0x0000, 0xF,     0,    0,   1024,   768,        0,   0,   0,   0,   0,   0,   0,                       280+6,     1303+6,  35-7,   802-7,    0,     0   },//1024x768 70Hz
    {22,    1024,   768,    7500,   MMP_CAP_FRAMERATE_75HZ,    0x0000, 0xF,     0,    0,   1024,   768,        0,   0,   0,   0,   0,   0,   0,                       272+6,     1295+6,    31,     798,    0,     0   },//1024x768 75Hz
    {23,    1024,   768,    8500,   MMP_CAP_FRAMERATE_85HZ,    0x0000, 0xF,     0,    0,   1024,   768,        0,   0,   0,   0,   0,   0,   0,                       304+6,     1327+6,    39,     806,    0,     0   },//1024x768 85Hz
    
    {24,    1280,  1024,    6000,   MMP_CAP_FRAMERATE_60HZ,    0x0000, 0xF,     0,    0,   1280,  1024,        0,   0,   0,   0,   0,   0,   0,                       360+6,     1639+6,    41,    1064,    0,     0   },//1280x1024 60Hz
    {25,    1280,  1024,    7500,   MMP_CAP_FRAMERATE_75HZ,    0x0000, 0xF,     0,    0,   1280,  1024,        0,   0,   0,   0,   0,   0,   0,                       392+6,     1671+6,    41,    1064,    0,     0   },//1280x1024 75Hz


};
#endif


#ifdef COMPOSITE_DEV
typedef struct CAP_ADV7180_TIMINFO_TABLE_TAG
{
    MMP_UINT16 Index;
    MMP_UINT16 HActive;
    MMP_UINT16 VActive;
    MMP_UINT16 Rate;
    MMP_UINT16 FrameRate;
    MMP_UINT16 Skippattern;
    MMP_UINT16 SkipPeriod;
    MMP_UINT16 ROIPosX;
    MMP_UINT16 ROIPosY;
    MMP_UINT16 ROIWidth;
    MMP_UINT16 ROIHeight;    
    MMP_UINT16 HPolarity:1;
    MMP_UINT16 VPolarity:1;
    MMP_UINT16 HStar;
    MMP_UINT16 HEnd;
    MMP_UINT16 VStar1;
    MMP_UINT16 VEnd1;
    MMP_UINT16 VStar2;
    MMP_UINT16 VEnd2;       
        
}CAP_ADV7180_TIMINFO_TABLE;

//ADV7180 Table
#if (CFG_CHIP_FAMILY == 9920)
static CAP_ADV7180_TIMINFO_TABLE ADV7180_TABLE [] = {
    //Index, HActive, VActive,  Rate,       FrameRate,                               SkipPattern,  SkipPeriod,  ROIX, ROIY, ROIW, ROIH,   Hpor,   Vpor,  HStar,           HEnd,         VStar1,   VEnd1,  VStar2,   VEnd2, 
    {0,     720,    480,    2997,   MMP_CAP_FRAMERATE_29_97HZ,  0x001A,     0x4,        0,    0,  720,  480,     0,      0, 238+32,   1677+32,  22-7,   261-7,   285-7,   524-7   },//480i60    
    {1,     720,    576,    2500,   MMP_CAP_FRAMERATE_25HZ,     0x0000,     0xF,        8,    8,  704,  560,     0,      0, 264+18,   1703+18,  23-4,   310-4,   336-4,   623-4   },//576i50    
#else
static CAP_ADV7180_TIMINFO_TABLE ADV7180_TABLE [] = {
    //Index, HActive, VActive,  Rate,       FrameRate,           			SkipPattern,  SkipPeriod,  ROIX, ROIY, ROIW, ROIH,   Hpor,    Vpor,  HStar,        HEnd,           VStar1,   VEnd1,  VStar2,   VEnd2, 
    {0,     720,    480,    2997,   MMP_CAP_FRAMERATE_29_97HZ,  0x0000,     0xF,       8,    8,  704,  460,     0,      0, 238+32,   1677+32,   22-7,   261-7,  285-7,   524-7   },//480i60    
    {1,     720,    576,    2500,   MMP_CAP_FRAMERATE_25HZ,     0x0000,     0xF,       8,    8,  704,  560,     0,      0, 264+18,   1703+18,   23-4,   310-4,  336-4,   623-4   },//576i50  
#endif
};
#endif


//=============================================================================
//                Private Function Definition
//=============================================================================


//=============================================================================
//                Public Function Definition
//=============================================================================
  
    

#ifdef __cplusplus
}
#endif

#endif

