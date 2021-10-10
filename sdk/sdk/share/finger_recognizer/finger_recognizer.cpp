#include "string.h"
#include "stdlib.h"
#include "malloc.h"
#include "finger_extract.h"
#include "finger_recognizer.h"

//#define DEBUG
//=============================================================================
//                Constant Definition
//=============================================================================
#define ABS(x) (((x) < 0) ? -(x) : (x))

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct 
{
	int			nTrainFingers;			// the number of training images
	int			fingerId;
	int			score;
	FingerInfo		*fingerInfoArr;
} FingerRecognizer;

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static FingerRecognizer *finger_recognizer = 0;
extern char dir_diff[16][16];
//=============================================================================
//                Private Function Definition
//=============================================================================
#ifdef DEBUG
static int write_pgm(char* filename, int width, int height, unsigned char *data)
{
    FILE *fout;
	int i,j;
	char path_name[128];

	sprintf(path_name, "./result/%s\0", filename);
	fout = fopen(path_name, "wb");
	if(!fout) {
		printf("Can not open %s!\n", path_name);
		return -1;
	}

    fprintf(fout, "%s\n", "P5");
	//fprintf(fout, "%s\n", commen);
	fprintf(fout, "%d %d\n", width, height);
	fprintf(fout, "%d\n", 255);

	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
            fputc(data[i*width+j], fout);
		}
	}
	fclose(fout);
	return 0;
}

int write_ppm(char* filename, int width, int height, unsigned char *bgdata,
				unsigned char *rdata, unsigned char *gdata, unsigned char *bdata)
{
    FILE *fout;
	int i,j;
	char path_name[128];

	sprintf(path_name, "./result/%s\0", filename);
	fout = fopen(path_name, "wb");
	if(!fout) {
		printf("Can not open %s!\n", path_name);
		return -1;
	}

    fprintf(fout, "%s\n", "P6");
	//fprintf(fout, "%s\n", commen);
	fprintf(fout, "%d %d\n", width, height);
	fprintf(fout, "%d\n", 255);

	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			unsigned char r=0, g=0, b=0;
			if(bgdata) {
				r = bgdata[i*width+j];
				g = bgdata[i*width+j];
				b = bgdata[i*width+j];
			}
			if(rdata && rdata[i*width+j]) {
				r = rdata[i*width+j];
				g = b = 0;
			}
			if(gdata && gdata[i*width+j]) {
				g = gdata[i*width+j];
				r = b = 0;
			}
			if(bdata && bdata[i*width+j]) {
				b = bdata[i*width+j];
				r = g = 0;
			}
            fputc(r, fout);
            fputc(g, fout);
            fputc(b, fout);
		}
	}
	fclose(fout);
	return 0;
}
#endif

static void neighborMinutiae(FingerInfo *fingerInfo)
{
	int i, j, num;
	MinutiaeType *m = fingerInfo->minutiae;
	RelMinutiaeType	nb[64];
	
	for (i=0;i<fingerInfo->minutiae_no; i++) {
		num = 0;
		for (j=0;j<fingerInfo->minutiae_no; j++) {
			int dx, dy, dist2;
			if (i==j) continue;
			dx = m[j].x - m[i].x;
			dy = m[j].y - m[i].y;
			dist2 = dx*dx + dy*dy;
			if (dist2<NEIGHBOR_RADIUS2) {
				nb[num].rel_x = dx;
				nb[num].rel_y = dy;
				nb[num].rel_dir = m[j].dir;
				num++;
			}
			if (num==64) break;
		}
		if (num>NEIGHBOR_NUM) num = NEIGHBOR_NUM;
		for (j=0;j<num;j++) {
			m[i].neighbor[j].rel_x = nb[j].rel_x;
			m[i].neighbor[j].rel_y = nb[j].rel_y;
			m[i].neighbor[j].rel_dir = nb[j].rel_dir;
		}
		m[i].neighborNum = num;
	}
}

typedef struct {
	int	f0;
	int	f1;
} MatchPair;

static int sin5[3] = {-89, 0, 89}; // sin(-5)*1024, 0, sin(5)*1024
static int cos5[3] = {1020, 1024, 1020}; // sin(5)*1024, 0, sin(-5)*1024
static char matched[MINUTIAE_MAX_NO][MINUTIAE_MAX_NO] = {0};
static unsigned short matched_var[MINUTIAE_MAX_NO][MINUTIAE_MAX_NO] = {0};

int compareFinger(FingerInfo *f0, FingerInfo *f1, MatchInfo *matchInfo)
{
	int i0, i1, j0, j1, k, pair_num = 0, match_num=0;
	MatchPair	match_list[MINUTIAE_MAX_NO*4];
	char score_list[MINUTIAE_MAX_NO] = {0};
	MinutiaeType rel0, rel1;
	int find;
	int max_f0, max_f1;
	int max = 0, var_min = 0x7fffffff;
	int score = 0, rotate, dir_score=0;

	matchInfo->match_no = 0;
	
	// step 0: check period
	if (f0->period > f1->period+1 || f0->period < f1->period-1) {
		score = 0;
		goto quit;
	}

	memset(matched, 0, sizeof(matched));
	memset(matched_var, 0, sizeof(matched_var));
	
	// step 1: compare local minutiae
	for (i0=0;i0<f0->minutiae_no;i0++) {
		for (i1=0;i1<f1->minutiae_no;i1++) {
			find = 0;
			if (ABS(f0->minutiae[i0].x - f1->minutiae[i1].x) < FINGER_SHIFT_THRESHOLD &&
				ABS(f0->minutiae[i0].y - f1->minutiae[i1].y) < FINGER_SHIFT_THRESHOLD  &&
				dir_diff[f0->minutiae[i0].dir][f1->minutiae[i1].dir] < 3 ) {
				for (j0=0;j0<f0->minutiae[i0].neighborNum;j0++) {
				    for (j1=0;j1<f1->minutiae[i1].neighborNum;j1++) {
						if (ABS(f0->minutiae[i0].neighbor[j0].rel_x - f1->minutiae[i1].neighbor[j1].rel_x) < FINGER_MATCH_RADIUS &&
							ABS(f0->minutiae[i0].neighbor[j0].rel_y - f1->minutiae[i1].neighbor[j1].rel_y) < FINGER_MATCH_RADIUS  &&
							dir_diff[f0->minutiae[i0].neighbor[j0].rel_dir][f1->minutiae[i1].neighbor[j1].rel_dir]<2 ) {
							match_list[pair_num].f0 = i0;
							match_list[pair_num].f1 = i1;
							find = 1;
							break;
						} 
					}
					if (find==1) break;
				}
			}
			if (find==1)  {
				pair_num++;
				if (pair_num == MINUTIAE_MAX_NO*4) break;
			}
		}
		if (pair_num == MINUTIAE_MAX_NO*4) break;
	}

	if (pair_num<FINGER_MATCH_THRESHOLD)  {
		score = pair_num;
		goto quit;
	}
	
	// step2: compare globel minutiae
	for (k=0;k<pair_num;k++) {
		//printf("0: (%d, %d) vs 0: (%d, %d)\n", 
		//	f0->minutiae[match_list[k].f0].x, f0->minutiae[match_list[k].f0].y,
		//	f1->minutiae[match_list[k].f1].x, f1->minutiae[match_list[k].f1].y);
		matched[match_list[k].f0][match_list[k].f1]++;
		for (i0=0;i0<f0->minutiae_no;i0++) {
			if (i0==match_list[k].f0) continue;
			rel0.x = f0->minutiae[i0].x - f0->minutiae[match_list[k].f0].x;
			rel0.y = f0->minutiae[i0].y - f0->minutiae[match_list[k].f0].y;
			if (rel0.x==0 && rel0.y==0) continue;
			rel0.dir = dir_diff[f0->minutiae[i0].dir][f0->minutiae[match_list[k].f0].dir];
			for (i1=0;i1<f1->minutiae_no;i1++) {
				if (i1==match_list[k].f1) continue;
				rel1.x = f1->minutiae[i1].x - f1->minutiae[match_list[k].f1].x;
				rel1.y = f1->minutiae[i1].y - f1->minutiae[match_list[k].f1].y;
				if (rel1.x==0 && rel1.y==0) continue;
				rel1.dir = dir_diff[f1->minutiae[i1].dir][f1->minutiae[match_list[k].f1].dir];

				if (ABS(rel0.x- rel1.x) < FINGER_MATCH_RADIUS &&
					 ABS(rel0.y- rel1.y) < FINGER_MATCH_RADIUS  &&
					 dir_diff[rel0.dir][rel1.dir]<2 ) {
					int diff_x = rel0.x-rel1.x;
					int diff_y = rel0.y-rel1.y;
					//printf("%d: (%d, %d) vs %d: (%d, %d)\n", 
					//	i0,f0->minutiae[i0].x, f0->minutiae[i0].y,
					//   i1, f1->minutiae[i1].x, f1->minutiae[i1].y);
					matched[i0][i1]++;
					matched_var[i0][i1] += diff_x*diff_x + diff_y*diff_y;
				}
			}
		}
	}

	// step3: only left the maxmal matched minutiaes
	//              if maxmal matched is same, then chose the small var
	for (i0=0;i0<f0->minutiae_no;i0++) {
		int idx = -1, max_row = 0, min_var = 0x7fffffff;
		for (i1=0;i1<f1->minutiae_no;i1++) {
			if(matched[i0][i1]>=FINGER_MATCH_THRESHOLD) {
				if ( matched[i0][i1]>max_row ||
					  (matched[i0][i1]==max_row && matched_var[i0][i1]<min_var) ) {
					max_row = matched[i0][i1];
					min_var = matched_var[i0][i1];
					if (idx>=0) matched[i0][idx] = 0;
					idx = i1;
				} else {
					matched[i0][i1] = 0;
				}
			} else
				matched[i0][i1] = 0;
		}
	}

	for (i1=0;i1<f1->minutiae_no;i1++) {
		int idx = -1, max_col = 0, min_var = 0x7fffffff;
		for (i0=0;i0<f0->minutiae_no;i0++) {
			if(matched[i0][i1]>=FINGER_MATCH_THRESHOLD) {
				if ( matched[i0][i1]>max_col || 
					  (matched[i0][i1]==max_col && matched_var[i0][i1]<min_var) ) {
					max_col = matched[i0][i1];
					min_var = matched_var[i0][i1];
					if (idx>=0) matched[idx][i1] = 0;				
					idx = i0;
					if (max_col>=max) {
						max_f0 = i0;
						max_f1 = i1;
						max = max_col;
					}
				} else {
					matched[i0][i1] = 0;
				}
			} else
				matched[i0][i1] = 0;
		}
	}


	// generate match minutiae
	matchInfo->minutiae0[0].x  = f0->minutiae[max_f0].x;
	matchInfo->minutiae0[0].y  = f0->minutiae[max_f0].y;
	matchInfo->minutiae0[0].dir  = f0->minutiae[max_f0].dir;
	matchInfo->minutiae0[0].type  = f0->minutiae[max_f0].type;
	matchInfo->minutiae0[0].quality  = f0->minutiae[max_f0].quality;
	matchInfo->minutiae1[0].x  = f1->minutiae[max_f1].x;
	matchInfo->minutiae1[0].y  = f1->minutiae[max_f1].y;
	matchInfo->minutiae1[0].dir  = f1->minutiae[max_f1].dir;
	matchInfo->minutiae1[0].type  = f1->minutiae[max_f1].type;
	match_num = 1;
	for (i1=0;i1<f1->minutiae_no;i1++) {
		for (i0=0;i0<f0->minutiae_no;i0++) {
			if(matched[i0][i1]>0 && i0!=max_f0 && i1!=max_f1) {
				rel0.x = f0->minutiae[i0].x - f0->minutiae[max_f0].x;
				rel0.y = f0->minutiae[i0].y - f0->minutiae[max_f0].y;
				rel0.dir = f0->minutiae[i0].dir;
				rel1.x = f1->minutiae[i1].x - f1->minutiae[max_f1].x;
				rel1.y = f1->minutiae[i1].y - f1->minutiae[max_f1].y;
				rel1.dir = f1->minutiae[i1].dir;
				if (ABS(rel0.x-rel1.x)<=10 && ABS(rel0.y-rel1.y<=10) && dir_diff[rel0.dir][rel1.dir]<2) {
					matchInfo->minutiae0[match_num].x  = f0->minutiae[i0].x;
					matchInfo->minutiae0[match_num].y  = f0->minutiae[i0].y;
					matchInfo->minutiae0[match_num].dir  = f0->minutiae[i0].dir;
					matchInfo->minutiae0[match_num].type  = f0->minutiae[i0].type;
					matchInfo->minutiae0[match_num].quality  = f0->minutiae[i0].quality;
					matchInfo->minutiae1[match_num].x  = f1->minutiae[i1].x;
					matchInfo->minutiae1[match_num].y  = f1->minutiae[i1].y;
					matchInfo->minutiae1[match_num].dir  = f1->minutiae[i1].dir;
					matchInfo->minutiae1[match_num].type  = f1->minutiae[i1].type;
					score_list[match_num] = matched[i0][i1];
					match_num++;
				}
			}
		}
	}
	matchInfo->match_no = match_num;

	if (match_num<FINGER_MATCH_THRESHOLD) {
		score = match_num;
		goto quit;
	}

	//printf("n = %d, mean_x = %d, mean_y = %d, mean_dir = %d ", n, mean_x, mean_y, mean_dir);


	// Rotate +- 5 degree to find the min_var;	score = 0;
	for (i0=0;i0<3;i0++) {
		int var = 0;
		score = 0;
		for (k=0;k<match_num;k++) {
			int varx, vary, var_dir, rotate_x, rotate_y;
			rel0.x = matchInfo->minutiae0[k].x - matchInfo->minutiae0[0].x;
			rel0.y = matchInfo->minutiae0[k].y - matchInfo->minutiae0[0].y;
			rel0.dir = matchInfo->minutiae0[k].dir;
			rel1.x = matchInfo->minutiae1[k].x - matchInfo->minutiae1[0].x;
			rel1.y = matchInfo->minutiae1[k].y - matchInfo->minutiae1[0].y;
			rel1.dir = matchInfo->minutiae1[k].dir;
			rotate_x = (rel1.x * cos5[i0] - rel1.y * sin5[i0])>>10;
			rotate_y = (rel1.x * sin5[i0] + rel1.y * cos5[i0])>>10;
			varx = rel0.x- rotate_x;
			vary = rel0.y- rotate_y;
			if (matchInfo->minutiae0[k].quality>1)
				score += score_list[k]*2;
			else
				score += score_list[k];
			var_dir = dir_diff[rel0.dir][rel1.dir]<<2;
			var += varx*varx + vary*vary + var_dir;
		}
		//printf("var[%d] = %d, ", i0, var);
		if (var<var_min) {
			var_min = var;
			rotate = i0-1;
		}
	}
	//printf("\n");
	matchInfo->match_no = match_num;
	matchInfo->rotate = rotate;

	if ( var_min < score) var_min = score;
	//printf("var=%d\n", var/(n-1));
	//for (k=0;k<n;k++) {
	//	printf("(%d, %d) vs (%d, %d)\n", f0->minutiae[match_list[k].f0].x,  f0->minutiae[match_list[k].f0].y,
	//																f1->minutiae[match_list[k].f1].x,  f1->minutiae[match_list[k].f1].y);
	//}
	score = score * 1000 * match_num/ var_min;

	{
		// step4: match with two direction blocks
			int i,j;
			int f0_x = matchInfo->minutiae0[0].x;
			int f0_y = matchInfo->minutiae0[0].y;
			int f1_x = matchInfo->minutiae1[0].x;
			int f1_y = matchInfo->minutiae1[0].y;
			int dir_width = (f0_x<f1_x) ? ((f0_x/WIN8)+ (REGION_WIDTH/WIN8-f1_x/WIN8)-2) :  ((f1_x/WIN8)+ (REGION_WIDTH/WIN8-f0_x/WIN8)-2);
			int dir_height = (f0_y<f1_y) ? ((f0_y/WIN8)+ (REGION_HEIGHT/WIN8-f1_y/WIN8)-2) :  ((f1_y/WIN8)+ (REGION_HEIGHT/WIN8-f0_y/WIN8)-2);
			int dir0_x = (f0_x<f1_x) ? 1 : (REGION_WIDTH/WIN8 - dir_width-1);
			int dir0_y = (f0_y<f1_y) ? 1 : (REGION_HEIGHT/WIN8 - dir_height-1);
			int dir1_x = (f0_x<f1_x) ? (REGION_WIDTH/WIN8 - dir_width-1) : 1;
			int dir1_y = (f0_y<f1_y) ? (REGION_HEIGHT/WIN8 - dir_height-1) : 1;

			//if (dir_width*2<REGION_WIDTH/WIN8 || dir_height*2<REGION_HEIGHT/WIN8)
			//	continue;

			dir_score = 0;
			for (i=0;i<dir_height;i++) {
				for (j=0;j<dir_width;j++) {
					int dir0 = f0->fingerDir[(i+dir0_y)*REGION_WIDTH/WIN8+j+dir0_x];
					int dir1 = f1->fingerDir[(i+dir1_y)*REGION_WIDTH/WIN8+j+dir1_x];
					int diff = ABS(f0->fingerDir[(i+dir0_y)*REGION_WIDTH/WIN8+j+dir0_x] - f1->fingerDir[(i+dir1_y)*REGION_WIDTH/WIN8+j+dir1_x]);
					if (diff==0) dir_score++;
					//else if (diff==1 || diff==7) score++;
					//else if (diff==2 || diff==6) score+=0;
					//else score -=3;
				}
			}
			dir_score = (dir_score<<8)/(dir_height*dir_width);
			//printf("dir_score = %f, width = %d, height = %d\n", (float)dir_score/256.0, dir_width, dir_height);
	}
	score = (score*dir_score)>>7;

quit:
	return score;
}

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * create finger recognizer
 *
 * @return              none
 */
//=============================================================================
void
fingerRecognizer_create(void)
{
	finger_recognizer = (FingerRecognizer *)malloc(sizeof(FingerRecognizer));
	memset(finger_recognizer, 0, sizeof(FingerRecognizer));
	finger_recognizer->fingerId = -1;
}

//=============================================================================
/**
 * release finger recognizer
 *
 * @return              none
 */
//=============================================================================
void
fingerRecognizer_release(void)
{
	if (finger_recognizer->fingerInfoArr)
		free(finger_recognizer->fingerInfoArr);
	free(finger_recognizer);
}

//=============================================================================
/**
 * Check if the input finger data is stable or not.
 * @param fingerImage   finger sensor Y data and area information.
 * @return              true - stable finger image, false - not stable.
 */
//=============================================================================
int
isStableFingerPrint(FingerImage *fingerImage, FingerQualityInfo *qualityInfo, int quality_threshold)
{
    int width = fingerImage->width, height = fingerImage->height;
	int sx[4] = {REGION_X, REGION_X+REGION_WIDTH-WIN8*2, REGION_X, REGION_X+REGION_WIDTH-WIN8*2};
	int sy[4] = {REGION_Y, REGION_Y, REGION_Y+REGION_HEIGHT-WIN8*2, REGION_Y+REGION_HEIGHT-WIN8*2};
	short gx[WIN8*2][WIN8*2];
	short gy[WIN8*2][WIN8*2];
    unsigned char *clipping, *dir;
	unsigned short *quality;
	int i,j,k,var, good;
	int ret = 0;
#ifdef DEBUG
	unsigned char *expand = (unsigned char *)malloc(REGION_WIDTH*REGION_HEIGHT);
#endif

	qualityInfo->corner = 0;
	qualityInfo->quality = 0;
	qualityInfo->period = 0;

	// check 4 corner
	for (k=0;k<4;k++) {
		var = 0;
		gradient_filter(sx[k], sy[k], WIN8*2, WIN8*2, width, fingerImage->fingerData, &gx[0][0], &gy[0][0]);
		for (i=0;i<WIN8*2;i++) {
			for (j=0;j<WIN8*2;j++) {
				var += gx[i][j]*gx[i][j] + gy[i][j]*gy[i][j];
			}
		}
		qualityInfo->corner<<=1;
		if (var>FINGER_GOOD_THRESHOLD) 
			qualityInfo->corner++;
	}

	if (qualityInfo->corner != 0xf)
		goto quit;

    clipping = (unsigned char*) malloc(REGION_WIDTH * REGION_HEIGHT);
    dir = (unsigned char *)malloc((REGION_WIDTH/WIN8)*(REGION_HEIGHT/WIN8));
    quality = (unsigned short *)malloc((REGION_WIDTH/WIN16)*(REGION_HEIGHT/WIN16)*sizeof(unsigned short));

	// step 0: clip
	clipping_window(width, height, fingerImage->fingerData, clipping, REGION_X, REGION_Y, REGION_WIDTH, REGION_HEIGHT );

    // step 1: find direction
    find_dir(REGION_WIDTH, REGION_HEIGHT, clipping, dir);

    // step 2: find average period
	qualityInfo->period = find_period(REGION_WIDTH, REGION_HEIGHT, clipping, dir,  quality);
	good = 0;
	for (i=1;i<REGION_HEIGHT/WIN16;i++) {
		for (j=1;j<REGION_WIDTH/WIN16;j++) {
			if (quality[i*REGION_WIDTH/WIN16+j]>FINGER_QUALITY_THRESHOLD) 
				good++;
		}
	}
	qualityInfo->quality = good*100/((REGION_HEIGHT/WIN16-1)*(REGION_WIDTH/WIN16-1));

#ifdef DEBUG
	memset(expand, 0, REGION_WIDTH*REGION_HEIGHT);
	for (i=1;i<REGION_HEIGHT/WIN16;i++) {
		for (j=1;j<REGION_WIDTH/WIN16;j++) {
			if (quality[i*REGION_WIDTH/WIN16+j]>FINGER_QUALITY_THRESHOLD) 
				quality[i*REGION_WIDTH/WIN16+j] = 1;
			else 
				quality[i*REGION_WIDTH/WIN16+j] = 0;
		}
	}
	ret = expand_quality(REGION_WIDTH, REGION_HEIGHT, quality, expand);
	write_ppm("quality.ppm", REGION_WIDTH, REGION_HEIGHT, clipping, NULL, expand, NULL);
#endif
	
	if (qualityInfo->quality>=quality_threshold && qualityInfo->period>FINGER_PERIOD_THRESHOLD)
		ret = 1;

    free(clipping);
	free(dir);
	free(quality);
#ifdef DEBUG
	free(expand);
#endif
quit:
	return ret;
}

/*static int dist2_cmp(const void *a, const void *b)
{
	return ((*(MinutiaType *)a).dist2 - (*(MinutiaType *)b).dist2);
}*/
//=============================================================================
/**
 * extract and generate minutiaes of input finger data.
 * @param fingerImage   finger sensor Y data and area information.
 * @param fingerInfo    output of finger minutiaes info.
 * @return              true - success, false - failed.
 */
//=============================================================================
int
getFingerInfo(
    FingerImage         *fingerImage,
    FingerInfo          *fingerInfo)
{
    // do finger extract
    int ret, i, j, activeNimutiae, size, period;
    int f_width = fingerImage->width, f_height = fingerImage->height;
    unsigned char *capture, *clipping, *median, *mean, *variance, *dir, *enhdir, *normal;
	unsigned short *quality;
    unsigned char *image0, *image1;
    unsigned char *block0, *block1, *block2, *block3;
    unsigned char *binary, *smooth, *thinning, *tmp;
	//int quality, period;

#ifdef DEBUG
	unsigned char *source = (unsigned char *)malloc(REGION_WIDTH*REGION_HEIGHT);
	unsigned char *expand = (unsigned char *)malloc(REGION_WIDTH*REGION_HEIGHT);
	unsigned char *end = (unsigned char *)malloc(REGION_WIDTH*REGION_HEIGHT);
	unsigned char *fork = (unsigned char *)malloc(REGION_WIDTH*REGION_HEIGHT);
#endif

    image0 = (unsigned char*) malloc(REGION_WIDTH * REGION_HEIGHT);
    image1 = (unsigned char*) malloc(REGION_WIDTH * REGION_HEIGHT);
    block0 = (unsigned char *)malloc((REGION_WIDTH/WIN8)*(REGION_HEIGHT/WIN8));
    block1 = (unsigned char *)malloc((REGION_WIDTH/WIN8)*(REGION_HEIGHT/WIN8));
    block2 = (unsigned char *)malloc((REGION_WIDTH/WIN8)*(REGION_HEIGHT/WIN8));
    block3 = (unsigned char *)malloc((REGION_WIDTH/WIN8)*(REGION_HEIGHT/WIN8));
    quality = (unsigned short *)malloc((REGION_WIDTH/WIN16)*(REGION_HEIGHT/WIN16)*sizeof(unsigned short));

	// step 0: clip
    capture = fingerImage->fingerData;
	clipping = image0; // out
	clipping_window( f_width, f_height, capture, clipping, REGION_X, REGION_Y, REGION_WIDTH, REGION_HEIGHT );
#ifdef DEBUG
	memcpy(source, clipping, REGION_WIDTH*REGION_HEIGHT);
	write_pgm("clip.pgm", REGION_WIDTH, REGION_HEIGHT, clipping);
#endif

    // step 1: find direction
	dir = block0;
    ret = find_dir(REGION_WIDTH, REGION_HEIGHT, clipping, dir);
#ifdef DEBUG
	memset(expand, 0, REGION_WIDTH*REGION_HEIGHT);
	ret = expand_dir(REGION_WIDTH, REGION_HEIGHT, dir, expand);
	write_ppm("dir.ppm", REGION_WIDTH, REGION_HEIGHT, clipping, NULL, expand, NULL);
#endif

    // step 2: find average period
	period = find_period(REGION_WIDTH, REGION_HEIGHT, clipping, dir, quality);

    // step 3: find mean and variance
    mean = block3;
    ret = find_mean(REGION_WIDTH, REGION_HEIGHT, clipping, mean);
    imageBlur(block1, mean, REGION_WIDTH / WIN8, REGION_HEIGHT / WIN8);
    mean = block1;
    variance = block3;
    ret = find_variance(REGION_WIDTH, REGION_HEIGHT, clipping, mean, variance);
    imageBlur(block2, variance, REGION_WIDTH / WIN8, REGION_HEIGHT / WIN8);
    variance = block2;

#ifdef DEBUG
	ret = write_pgm("mean.pgm", REGION_WIDTH/WIN8, REGION_HEIGHT/WIN8, mean);
	ret = write_pgm("variance.pgm", REGION_WIDTH/WIN8, REGION_HEIGHT/WIN8, variance);
#endif

	// step 4: local normalization
    normal = image1;
	local_normalize(REGION_WIDTH, REGION_HEIGHT, clipping, mean, variance, normal);
#ifdef DEBUG
	ret = write_pgm("normal.pgm", REGION_WIDTH, REGION_HEIGHT, normal);
#endif

    // step 5: median filter
    median = image0;  // out
	ret = median_filter(REGION_WIDTH, REGION_HEIGHT, normal, median);
#ifdef DEBUG
	write_pgm("median.pgm", REGION_WIDTH, REGION_HEIGHT, median);
#endif

    // step 6: direction  filter
    enhdir = image1;
	if (period>8)
		ret = dir8_filter(REGION_WIDTH, REGION_HEIGHT, median, dir, mean, variance, enhdir);
	else if (period>6)
		ret = dir6_filter(REGION_WIDTH, REGION_HEIGHT, median, dir, mean, variance, enhdir);
	else
		ret = dir5_filter(REGION_WIDTH, REGION_HEIGHT, median, dir, mean, variance, enhdir);

#ifdef DEBUG
	ret = write_pgm("enhdir.pgm", REGION_WIDTH, REGION_HEIGHT, enhdir);
#endif

    // step 7: local segmentation
    binary = image0;
	if (period>8)
		ret = binary8_filter(REGION_WIDTH, REGION_HEIGHT, enhdir, dir, binary);
	else if (period>6)
		ret = binary6_filter(REGION_WIDTH, REGION_HEIGHT, enhdir, dir, binary);
	else
		ret = binary5_filter(REGION_WIDTH, REGION_HEIGHT, enhdir, dir, binary);

#ifdef DEBUG
	for(i=0;i<REGION_WIDTH*REGION_HEIGHT;i++)
		expand[i] = binary[i]*255;
	ret = write_pgm("binary.pgm", REGION_WIDTH, REGION_HEIGHT, expand);
#endif
    // step 8: smooth filter
    smooth = image1;
	ret = smooth_filter(REGION_WIDTH, REGION_HEIGHT, binary, smooth);

#ifdef DEBUG
	for(i=0;i<REGION_WIDTH*REGION_HEIGHT;i++)
		expand[i] = smooth[i]*255;
	ret = write_pgm("smooth.pgm", REGION_WIDTH, REGION_HEIGHT, expand);
#endif

    // step 9: thinning filter
    thinning = image1;
    tmp = image0;
	ret = thinning_filter(REGION_WIDTH, REGION_HEIGHT, thinning, tmp);

#ifdef DEBUG
	for(i=0;i<REGION_WIDTH*REGION_HEIGHT;i++)
		expand[i] = thinning[i]*255;
	ret = write_pgm("thinning.pgm", REGION_WIDTH, REGION_HEIGHT, expand);
#endif

    // step 10: find minutiae
    size = find_minutiae(REGION_WIDTH, REGION_HEIGHT, thinning, NULL, minutiaes);

#ifdef DEBUG
	memset(end, 0, REGION_WIDTH*REGION_HEIGHT);
	memset(fork, 0, REGION_WIDTH*REGION_HEIGHT);
	expand_minutiae(REGION_WIDTH, REGION_HEIGHT, size, minutiaes, end, fork, 0); // tmp => end, binary => fork
	write_ppm("minutiae0.ppm", REGION_WIDTH, REGION_HEIGHT, expand, end, NULL, fork);
#endif

    // step 11: remove false minutiae
    ret = remove_false_minutiae(REGION_WIDTH, REGION_HEIGHT, thinning, dir, size, minutiaes);
#ifdef DEBUG
	memset(end, 0, REGION_WIDTH*REGION_HEIGHT);
	memset(fork, 0, REGION_WIDTH*REGION_HEIGHT);
	expand_minutiae(REGION_WIDTH, REGION_HEIGHT, size, minutiaes, end, fork, 1); // tmp => end, binary => fork
	write_ppm("minutiae1.ppm", REGION_WIDTH, REGION_HEIGHT, expand, end, NULL, fork);
	write_ppm("minutiae2.ppm", REGION_WIDTH, REGION_HEIGHT, source, end, NULL, fork);
#endif

	/*ret = remove_mask_minutiae(REGION_WIDTH, FINGER_QUALITY_THRESHOLD, quality, size, minutiaes);
#ifdef DEBUG
	for (i=1;i<REGION_HEIGHT/WIN16;i++) {
		for (j=1;j<REGION_WIDTH/WIN16;j++) {
			if (quality[i*REGION_WIDTH/WIN16+j]>FINGER_QUALITY_THRESHOLD) 
				quality[i*REGION_WIDTH/WIN16+j] = 1;
			else 
				quality[i*REGION_WIDTH/WIN16+j] = 0;
		}
	}
	memset(expand, 0, REGION_WIDTH*REGION_HEIGHT);
	ret = expand_quality(REGION_WIDTH, REGION_HEIGHT, quality, expand);
	memset(end, 0, REGION_WIDTH*REGION_HEIGHT);
	memset(fork, 0, REGION_WIDTH*REGION_HEIGHT);
	expand_minutiae(REGION_WIDTH, REGION_HEIGHT, size, minutiaes, end, fork, 1); // tmp => end, binary => fork
	write_ppm("minutiae3.ppm", REGION_WIDTH, REGION_HEIGHT, source, end, expand, fork);
#endif
*/
	//dir
	for (i=0;i<(REGION_WIDTH/WIN8)*(REGION_HEIGHT/WIN8);i++) 
		fingerInfo->fingerDir[i] = dir[i];

    //minutiaes
    for (i = activeNimutiae = 0; i < size && activeNimutiae < MINUTIAE_MAX_NO; i++)
    {
        if (minutiaes[i].type > 0)
        {
          fingerInfo->minutiae[activeNimutiae].type = minutiaes[i].type;
          fingerInfo->minutiae[activeNimutiae].x = minutiaes[i].x;
          fingerInfo->minutiae[activeNimutiae].y = minutiaes[i].y;
          fingerInfo->minutiae[activeNimutiae].dir = minutiaes[i].dir; // for DIR32
          activeNimutiae++;
        }
    }
    fingerInfo->minutiae_no = activeNimutiae;
	fingerInfo->period = period;

    printf("\tBase finger has %d minutiae\n", activeNimutiae);

	// step10: generate neighbor relative nimutiaes
	neighborMinutiae(fingerInfo);

	free(quality);
    free(image0);
    free(image1);
	free(block0);
    free(block1);
    free(block2);
    free(block3);
#ifdef DEBUG
	free(source);
	free(expand);
	free(fork);
	free(end);
#endif
	return 1;
}

int updateFingerDB(int nTrainFingers, FingerInfo *fingerInfoArr)
{
	//if (finger_recognizer->fingerInfoArr)
	//	free (finger_recognizer->fingerInfoArr);
	finger_recognizer->fingerInfoArr = fingerInfoArr;
	finger_recognizer->nTrainFingers = nTrainFingers;

	return 0;
}

//=============================================================================
/**
 * predict finger info from database
 * @param fingerInfo   test finger minutiaes info
 * @param index match index (-1 means match all the database)
 * @return              identified index (-1 means no one matched)
 */
//=============================================================================
int
predictFingerInfo(FingerInfo *testFingerInfo, int index, int threshold)
{
	int iNearest = -1;
	int i, value, score_max = 0;
	MatchInfo matchInfo;
	if (index<0) {
		for (i=0; i<finger_recognizer->nTrainFingers; i++) {
			value = compareFinger(&finger_recognizer->fingerInfoArr[i], testFingerInfo, &matchInfo) ;
			//printf("%d match score = %d, num = %d\n", i, value, matchInfo.match_no);
			//for (j=0;j<matchInfo.match_no;j++)
			//	printf("GOLD: (%d, %d, %d), TEST: (%d, %d, %d)\n", 
			//	matchInfo.minutiae0[j].x, matchInfo.minutiae0[j].y, matchInfo.minutiae0[j].dir,
			//	matchInfo.minutiae1[j].x, matchInfo.minutiae1[j].y, matchInfo.minutiae1[j].dir);
			if (value>score_max) {
				score_max = value;
			}
			
			if (score_max>threshold) {
				iNearest = i;
				break;
			}
		}
	} else {
		score_max = compareFinger(&finger_recognizer->fingerInfoArr[index], testFingerInfo, &matchInfo) ;
		if (score_max>threshold) {
			iNearest = index;
		}
	}
	finger_recognizer->fingerId = iNearest;
	finger_recognizer->score = score_max;
	
	return iNearest;
}

//=============================================================================
/**
 * get match result.
 * @return               identified index (-1 means no one matched)
 */
//=============================================================================
int
getFingerMatchResult(int *score)
{
    *score = finger_recognizer->score;
    return finger_recognizer->fingerId;
}

//=============================================================================
/**
 * merge Finger match minutiaes
 * matchInfo[3] = (f0, f1), (f0, f2), (f1, f2)
 * @return               identified index (-1 means no one matched)
 */
//=============================================================================
int
mergeFingerInfo(FingerInfo *dstFingerInfo, FingerInfo *srcFingerInfo0,  FingerInfo *srcFingerInfo1, int threshold)
{
	int value;
	int i, j,match_num;
	int quality1 = 0, quality2 = 0, quality3 = 0;
	MatchInfo matchInfo;

	if (srcFingerInfo0->minutiae_no==0 || srcFingerInfo1->minutiae_no==0)
		return -1;

	match_num = dstFingerInfo->minutiae_no;

	if (match_num==0) {
		value = compareFinger(srcFingerInfo0, srcFingerInfo1, &matchInfo);
		if (value<threshold)
			return -2;

		dstFingerInfo->period = srcFingerInfo1->period;
		for(i=0;i<matchInfo.match_no;i++) {
			dstFingerInfo->minutiae[match_num].x = matchInfo.minutiae0[i].x;
			dstFingerInfo->minutiae[match_num].y = matchInfo.minutiae0[i].y;
			dstFingerInfo->minutiae[match_num].dir = matchInfo.minutiae0[i].dir;
			dstFingerInfo->minutiae[match_num].type = matchInfo.minutiae0[i].type;
			dstFingerInfo->minutiae[match_num].quality=1;
			printf("DST: %d: (%d, %d, %d)\n", match_num,  
					dstFingerInfo->minutiae[match_num].x,
					dstFingerInfo->minutiae[match_num].y,
					dstFingerInfo->minutiae[match_num].dir);
			match_num++;
		}

	//copy dir
	for (i=0;i<(REGION_WIDTH/WIN8)*(REGION_HEIGHT/WIN8);i++) 
		dstFingerInfo->fingerDir[i] = srcFingerInfo0->fingerDir[i];

	} else {
		int shift_x, shift_y, rotate;
		int x0, y0;
		value = compareFinger(dstFingerInfo, srcFingerInfo0, &matchInfo);
		if (value<threshold)
			return -3;

		// find shift between dst and f0 and rotate angle;
		shift_x = matchInfo.minutiae0[0].x - matchInfo.minutiae1[0].x;
		shift_y = matchInfo.minutiae0[0].y - matchInfo.minutiae1[0].y;
		rotate = matchInfo.rotate;
		x0 =  matchInfo.minutiae1[0].x;
		y0 = matchInfo.minutiae1[0].y;

	printf("shift(%d, %d), zero(%d, %d), rotate = %d\n", 
		shift_x, shift_y, x0, y0, rotate);

		// find the common minutiae between src0 and src1
		value = compareFinger(srcFingerInfo0, srcFingerInfo1, &matchInfo);
		if (value<threshold)
			return -4;

		// check each rotated src1 with dst to find the common minutiae
		for(i=0;i<matchInfo.match_no;i++) {
			int find = 0;
			MinutiaeType rel;
			int rotate_x, rotate_y;
			rel.x = matchInfo.minutiae0[i].x - x0;
			rel.y = matchInfo.minutiae0[i].y - y0;
			rel.dir = matchInfo.minutiae0[i].dir;
			rel.type = matchInfo.minutiae0[i].type;
			rotate_x = (rel.x * cos5[rotate+1] - rel.y * sin5[rotate+1])>>10;
			rotate_y = (rel.x * sin5[rotate+1] + rel.y * cos5[rotate+1])>>10;
			rel.x = x0 + rotate_x;
			rel.y = y0 + rotate_y;
			for(j=0;j<dstFingerInfo->minutiae_no;j++) {
				if ( ABS(rel.x + shift_x - dstFingerInfo->minutiae[j].x)<FINGER_MATCH_RADIUS &&
					  ABS(rel.y + shift_y - dstFingerInfo->minutiae[j].y)<FINGER_MATCH_RADIUS &&
					  dir_diff[rel.dir][dstFingerInfo->minutiae[j].dir]<2 ) {
					dstFingerInfo->minutiae[j].quality++;
					find = 1;
					break;
				}
			}
			// if the minutiae find in src0 and src1, but not in dst, then add to dst
			if (find==0) {
				dstFingerInfo->minutiae[match_num].x = rel.x + shift_x;
				dstFingerInfo->minutiae[match_num].y = rel.y + shift_y;
				dstFingerInfo->minutiae[match_num].dir = rel.dir;
				dstFingerInfo->minutiae[match_num].type = rel.type;
				dstFingerInfo->minutiae[match_num].quality=1;
				printf("DST: %d: (%d, %d, %d)\n", match_num,  
						dstFingerInfo->minutiae[match_num].x,
						dstFingerInfo->minutiae[match_num].y,
						dstFingerInfo->minutiae[match_num].dir);
				match_num++;
			}
		}
	}

	for (i=0;i<match_num;i++) {
		if (dstFingerInfo->minutiae[i].quality==1)
			quality1++;
		else if (dstFingerInfo->minutiae[i].quality==2)
			quality2++;
		else if (dstFingerInfo->minutiae[i].quality>2)
			quality3++;
	}

	printf("quality1 = %d, quality2 = %d, quality3 = %d\n", quality1, quality2, quality3);
	dstFingerInfo->minutiae_no = match_num;
	neighborMinutiae(dstFingerInfo);

    return (quality2+quality3);
}


