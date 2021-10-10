#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

//#include "fr_type.h"

//#define DEBUG

#define WIN8	8
#define WIN16	16
#define WIN32	32

#define MINUTIAE_MAX		1000
#define DEFAULT_MEAN				128
#define DEFAULT_VAR					60
#define BINARY_THRESHOLD		128

typedef struct {
	int	type;
	int	x;
	int	y;
	int vx[4];
	int vy[4];
	int dir;
	int quality;
} minutiae;

typedef struct {
	int			size;
	int			idx;
	minutiae	item[100];
} minutiae_list;

extern minutiae_list mlist;
extern minutiae minutiaes[MINUTIAE_MAX];

void clipping_window(int width, int height, unsigned char *src, unsigned char *dst, int region_x, int region_y, int region_width, int region_height);

int imageBlur(unsigned char *dst, unsigned char *src, int width, int height);

int imageBilinearScale(unsigned char *dst, unsigned char *src, int dst_width, int dst_height, int src_width, int src_height);

int localShadingCorrection(unsigned char *image, int width, int height, int *integralValue);

void bilinearScale(int dst_width, int dst_height, unsigned char *dst, unsigned char *src, int src_left, int src_top, int src_right, int src_bottom);

void globalNormalize(unsigned char *image, int width, int height);

unsigned char median(unsigned char *line0, unsigned char *line1, unsigned char *line2);

int find_mean(int width, int height, unsigned char *din, unsigned char *mean);

int find_variance(int width, int height, unsigned char *din, unsigned char *mean, unsigned char *variance);

int local_normalize(int width, int height, unsigned char *din, 
				  unsigned char *mean, unsigned char *variance, unsigned char *dout);

int expand_quality(int width, int height, unsigned short *din, unsigned char *dout);

int median_filter(int width, int height, unsigned char *din, unsigned char *dout);

int mean_filter(int width, int height, signed char *din, signed char *dout);

int gradient_filter(int sx, int sy, int width, int height, int pitch, unsigned char *din, short *gx, short *gy) ;

int find_dir(int width, int height, unsigned char *din, unsigned char *dir);

int expand_dir(int width, int height, unsigned char *dir, unsigned char *data);

int find_period(int width, int height, unsigned char *din, unsigned char *dir, unsigned short *quality) ;

int dir8_filter(int width, int height, unsigned char *din, unsigned char *dir, unsigned char *mean, unsigned char *variance, unsigned char *dout);

int dir5_filter(int width, int height, unsigned char *din, unsigned char *dir, unsigned char *mean, unsigned char *variance, unsigned char *dout);

int dir6_filter(int width, int height, unsigned char *din, unsigned char *dir, unsigned char *mean, unsigned char *variance, unsigned char *dout);

int binary8_filter(int width, int height, unsigned char *din, unsigned char *mean, unsigned char *dout);

int binary5_filter(int width, int height, unsigned char *din, unsigned char *mean, unsigned char *dout);

int binary6_filter(int width, int height, unsigned char *din, unsigned char *mean, unsigned char *dout);

int smooth_filter(int width, int height, unsigned char *din, unsigned char *dout);

int thinning_teration(int width, int height, unsigned char *din, unsigned char *dout, int iter);

int post_thinning(int width, int height, unsigned char *data);

int thinning_filter(int width, int height, unsigned char *data, unsigned char *tmp);

int find_minutiae(int width, int height, unsigned char *din, unsigned char *mask, minutiae *minutiaes);

int check_minutiae(int size, minutiae *minutiaes, minutiae check);

int search_next(int width, int height, unsigned char *thinning, int x, int y, int prev, minutiae_list *mlist, int cnt, int iter);

int search_end(int width, int height, unsigned char *thinning, minutiae_list *mlist);

int search_fork(int width, int height, unsigned char *thinning, minutiae_list *mlist, int iter);

int direction(int vx, int vy);

int expand_minutiae(int width, int height, int size, minutiae *minutiaes,
					unsigned char *end, unsigned char *fork, int en_dir);

int remove_false_minutiae(int width, int height, unsigned char *thinning, unsigned char *dir, int size, minutiae *minutiaes);

int remove_mask_minutiae(int width, int threshold, unsigned short *quality, int size, minutiae *minutiaes);
