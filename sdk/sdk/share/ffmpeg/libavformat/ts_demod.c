#include "libavutil/avstring.h"
#include "avformat.h"
#include "ts_source.h"


#define CAMERA_MODE			1
#define INPUT_SOURCE_NOLOCK	1

typedef struct TSDemodContext {
	TS_READER *tsReader;
	int fd;
} TSDemodContext;

static int tsdemod_read(URLContext *h, unsigned char *buf, int size)
{
	TSDemodContext *s = h->priv_data;
	TS_READER *tsReader = s->tsReader;
	int fd = s->fd;
	int rets = size;
	bool retv = false;
	int act_size = 0;

	while (1)
	{
#if INPUT_SOURCE_NOLOCK
		rets = size;
		retv = DvrTsReaderRead(tsReader, buf, &rets);

		if (rets != 0)
		{
			act_size = rets;
			break;
		}
#else
		rets = size - act_size;
		retv = DvrTsReaderRead(tsReader, buf + act_size, &rets);
		act_size += rets;

		if (act_size == size)
			break;
#endif
	}

	av_log(h, AV_LOG_DEBUG, "tsdemod%d_read req:buf=%p size=%d, ret:size=%d\n", fd, buf, size, rets);
	return act_size;
}

static int tsdemod_write(URLContext *h, const unsigned char *buf, int size)
{
	TSDemodContext *s = h->priv_data;
	int fd = s->fd;

	av_log(h, AV_LOG_INFO, "tsdemod%d_write no support!! req info: buf=%p size=%d\n", fd, buf, size);

	return -1;
}

static int tsdemod_get_handle(URLContext *h)
{
	TSDemodContext *s = h->priv_data;
	int fd = s->fd;

	av_log(h, AV_LOG_INFO, "tsdemod%d_get_handle\n", fd);

	return fd;
}

static int tsdemod_check(URLContext *h, int mask)
{
	TSDemodContext *s = h->priv_data;
	int fd = s->fd;
	av_log(h, AV_LOG_INFO, "tsdemod%d_check mask=%x\n", fd, mask);
	return -1;
}

static int tsdemod_open(URLContext *h, const char *filename, int flags)
{
	TSDemodContext *s = h->priv_data;
	int fd;
	char *final;
	int port = -1, service = -1;

	av_strstart(filename, "tsdemod:", &filename);

	fd = strtol(filename, &final, 10);
	if ((filename == final) || *final)
	{
		/* No digits found, or something like 10ab */
		fd = 0;
	}
#if 0 // Only exist D1 service
	s->fd = fd;

#if CAMERA_MODE /* case in camera mode */
	s->tsReader = DvrTsReaderCreate(TS_SRC_CUSTOMER_PLAYBACK, fd, TS_SRC_SERVICE_D1);
#else /* case in TS Generator mode */
	s->tsReader = DvrTsReaderCreate(TS_SRC_CUSTOMER_PLAYBACK, fd, 0);
#endif
#else
	s->fd = port = (fd % 4);
	service = (fd / 4);

	s->tsReader = DvrTsReaderCreate(TS_SRC_CUSTOMER_PLAYBACK, port, service);
#endif
	h->is_streamed = 1;

	if (s->tsReader)
		av_log(h, AV_LOG_INFO, "tsdemod create fd=%d tsReader %p\n", s->fd, s->tsReader);

	return 0;
}

static int64_t tsdemod_seek(URLContext *h, int64_t pos, int whence)
{
	TSDemodContext *s = h->priv_data;
	int fd = s->fd;

	av_log(h, AV_LOG_INFO, "tsdemod%d_seek pos=%lld whence=%x\n", fd, pos, whence);

	return -1;
}

static int tsdemod_close(URLContext *h)
{
	TSDemodContext *s = h->priv_data;
	int fd = s->fd;
	TS_READER *tsReader = s->tsReader;

	av_log(h, AV_LOG_DEBUG, "tsdemod%d_close\n", fd);

	DvrTsReaderDestroy(tsReader);
	fd = -1;
	tsReader = NULL;

	return -1;
}

#if defined(_MSC_VER)
URLProtocol ff_tsdemod_protocol = {
	"tsdemod",
	tsdemod_open,
	tsdemod_read,
	NULL,
	NULL,
	tsdemod_close,
	NULL,
	NULL,
	NULL,
	tsdemod_get_handle,
	sizeof(TSDemodContext),
	NULL,
	0,
	tsdemod_check
};
#else // no defined (_MSC_VER)
URLProtocol ff_tsdemod_protocol = {
	.name			= "tsdemod",
	.url_open		= tsdemod_open,
	.url_read		= tsdemod_read,
	.url_write		= NULL, //tsdemod_write,
	.url_seek		= NULL, //tsdemod_seek,
	.url_close		= tsdemod_close,
	.url_read_pause		= NULL,
	.url_read_seek		= NULL,
	.url_get_file_handle	= tsdemod_get_handle,
	.url_check		= tsdemod_check,
	.priv_data_size		= sizeof(TSDemodContext),
};
#endif // define _MSC_VER
