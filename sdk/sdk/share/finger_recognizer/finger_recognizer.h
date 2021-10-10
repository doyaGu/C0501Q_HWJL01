#ifndef _FINGER_RECOGNIZER_H__
#define _FINGER_RECOGNIZER_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#define REGION_X							(78-8)
#define REGION_Y							(60-8)
#define REGION_WIDTH				(176+16)
#define REGION_HEIGHT			(128+16)
#define WIN8								8

#define FINGER_GOOD_THRESHOLD		100000

#define MINUTIAE_MAX_NO 64
#define NEIGHBOR_RADIUS2		(40*40)
#define NEIGHBOR_NUM				5
#define FINGER_SHIFT_THRESHOLD			80
#define FINGER_MATCH_RADIUS					10
#define FINGER_MATCH_THRESHOLD			4
#define FINGER_MERGE_SCORE_THRESHOLD		1000
#define FINGER_PREDICT_SCORE_THRESHOLD	4500
#define FINGER_QUALITY_THRESHOLD		        900
#define FINGER_QUALITY_PERCENTANGE		      70
#define FINGER_PERIOD_THRESHOLD		        4

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct {
  short rel_x;
  short rel_y;
  char rel_dir;
  char rel_type; 
} RelMinutiaeType;

typedef struct {
  short x;
  short y;
  char dir;
  char type; 
  char quality;
} MatchMinutiaeType;

typedef struct {
  short x;
  short y;
  char dir;
  char type; 
  char quality;
  short neighborNum;
 RelMinutiaeType neighbor[NEIGHBOR_NUM];
} MinutiaeType;

typedef struct {
  unsigned short   minutiae_no;
  MinutiaeType minutiae[MINUTIAE_MAX_NO];
  unsigned char  period;
  unsigned char fingerDir[(REGION_WIDTH/WIN8)*(REGION_HEIGHT/WIN8)];
} FingerInfo;

typedef struct {
  unsigned short   match_no;
  short rotate;
  MatchMinutiaeType minutiae0[MINUTIAE_MAX_NO];
  MatchMinutiaeType minutiae1[MINUTIAE_MAX_NO];
} MatchInfo;

typedef struct
{
    int           pitchY;
    int           width;
    int           height;
    unsigned char *fingerData;
} FingerImage;

typedef struct
{
    int           period;
    int           quality;
    int           corner;
} FingerQualityInfo;

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Initialize finger recognizer
 *
 * @param recognizerSetup   finger recognizer setup. if the input is null, use
 *                          default setup.
 * @return              none
 */
//=============================================================================
void
fingerRecognizer_create(void);

void
fingerRecognizer_release(void);

//=============================================================================
/**
 * Check if the input finger data is stable or not.
 * @param fingerImage   finger sensor Y data and area information.
 * @return              1 - stable finger image, 0 - not stable.
 */
//=============================================================================
int
isStableFingerPrint(FingerImage *fingerImage, FingerQualityInfo *qualityInfo, int quality_threshold);

//=============================================================================
/**
 * extract and generate minutiaes of input finger data.
 * @param fingerImage   finger sensor Y data and area information.
 * @param fingerInfo    output of finger minutiaes info.
 * @return              1 - success, 0 - failed.
 */
//=============================================================================
int
getFingerInfo(
    FingerImage         *fingerImage,
    FingerInfo          *fingerInfo);

int 
updateFingerDB(int nTrainFingers, FingerInfo *fingerInfoArr);

//=============================================================================
/**
 * predict finger info from database
 * @param fingerInfo   test finger minutiaes info
 * @param index match index (-1 means match all the database)
 * @return              identified index (-1 means no one matched)
 */
//=============================================================================
int
predictFingerInfo(FingerInfo *fingerInfo, int index, int threshold);

//=============================================================================
/**
 * get match result.
 * @return               identified index (-1 means no one matched)
 */
//=============================================================================
int
getFingerMatchResult(int *score);

int
mergeFingerInfo(FingerInfo *dstFingerInfo, FingerInfo *srcFingerInfo0, FingerInfo *srcFingerInfo1, int threshold);

#ifdef __cplusplus
}
#endif

#endif

