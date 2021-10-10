#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <alloca.h>

#include "config.h"
#include "tslib-private.h"

struct ad7879_ts_event { /* Used in the IBM Arctic II */
	signed short pressure;
	signed int x;
	signed int y;
	int millisecs;
	int flags;
};

static int ad7879_read(struct tslib_module_info *inf, struct ts_sample *samp, int nr)
{
	struct tsdev *ts = inf->dev;
	struct ad7879_ts_event *ad7879_evt;
	int ret;
	int total = 0;
	ad7879_evt = alloca(sizeof(*ad7879_evt) * nr);
    ret = read(ts->fd, ad7879_evt, sizeof(*ad7879_evt) * nr);
	if(ret > 0) {
		int nr = ret / sizeof(*ad7879_evt);
		while(ret >= (int)sizeof(*ad7879_evt)) {
			samp->x = (short)ad7879_evt->x;
			samp->y = (short)ad7879_evt->y;
			samp->pressure = ad7879_evt->pressure;
#ifdef DEBUG
        fprintf(stderr,"RAW---------------------------> %d %d %d\n",samp->x,samp->y,samp->pressure);
#endif /*DEBUG*/
			gettimeofday(&samp->tv,NULL);
			samp++;
			ad7879_evt++;
			ret -= sizeof(*ad7879_evt);
		}
	} else {
		return -1;
	}

	ret = nr;
	return ret;
}

static const struct tslib_ops ad7879_ops =
{
	ad7879_read,
};

TSAPI struct tslib_module_info *ad7879_mod_init(struct tsdev *dev, const char *params)
{
	struct tslib_module_info *m;

	m = malloc(sizeof(struct tslib_module_info));
	if (m == NULL)
		return NULL;

	m->ops = &ad7879_ops;
	return m;
}

#ifndef TSLIB_STATIC_CASTOR3_MODULE
	TSLIB_MODULE_INIT(ad7879_mod_init);
#endif
