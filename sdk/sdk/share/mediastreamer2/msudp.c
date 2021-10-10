#include "mediastreamer2/msudp.h"
#include "mediastreamer2/msticker.h"

#include "libavcodec/avcodec.h"
#include "libavcodec/get_bits.h"
#include "libavcodec/golomb.h"
#include "libavformat/avformat.h"

#define FILE_MODE      0
#define VIDEO_BUF_SIZE 10*1024
#define AUDIO_BUF_SIZE 2048
#define INPUT_BUFFER_SIZE   512000


static char start_code[4] = {0x00, 0x00, 0x01, 0xfc};

struct SenderData{
	msgb_allocator_t allocator;
	MSQueue data_queue;
	udp_config_t udp_conf;
	volatile bool_t run_flag;
	volatile bool_t thread_exit;
};

typedef struct SenderData SenderData;

static int udp_sender_set_para(MSFilter * f, void *arg){
	SenderData *d = (SenderData *)f->data;
	udp_config_t *udp_conf = (udp_config_t*)arg;
	struct sockaddr_in addr;
	memset(&(d->udp_conf),'\0',sizeof(udp_config_t));
	d->udp_conf.cur_filter = f;
	d->udp_conf.c_type = udp_conf->c_type;
	memcpy(d->udp_conf.group_ip,udp_conf->group_ip,16);
	memcpy(d->udp_conf.remote_ip,udp_conf->remote_ip,16);
	d->udp_conf.remote_port= udp_conf->remote_port;
	d->udp_conf.cur_socket = udp_conf->cur_socket;
	
	if(d->udp_conf.cur_socket == -1){		
		d->udp_conf.cur_socket = socket(AF_INET,SOCK_DGRAM,0);
		if(d->udp_conf.cur_socket < 0){
			printf("[udpsend_set_para]sockek create failure\n");
		}
		else
			printf("[udpsend_set_para]udp socket create OK!\n");
	}
}

static int udp_sender_get_socket(MSFilter * f, void *arg){	
	SenderData *d = (SenderData *)f->data;
	((int *)arg)[0] = d->udp_conf.cur_socket;
}


static MSFilterMethod sender_methods[] = {
	{MS_UDP_SEND_SET_PARA  , udp_sender_set_para  },
	{MS_UDP_SEND_GET_SOCKET, udp_sender_get_socket},
	{0, NULL}
};

static void audio_send(unsigned char *data,unsigned int len_size,struct SenderData *send_data,
	 					   struct sockaddr_in *addr,int addr_len)
{

	//printf("audio_send %d\n", len_size);
	sendto(send_data->udp_conf.cur_socket, data, len_size, 0, addr, addr_len);
	usleep(5000);
}

static void video_send(unsigned char *data,unsigned int len_size,struct SenderData *send_data,
	 					   struct sockaddr_in *addr,int addr_len){
	int send_size = 0, remain_size = len_size;
	unsigned char *tmp_buf = malloc(sizeof(char) * VIDEO_BUF_SIZE);
	while(remain_size > 0)
	{
		if(send_size == 0)
		{
			memcpy(&tmp_buf[0], start_code, 4);
			tmp_buf[4] = (remain_size >> 24) & 0xFF;
			tmp_buf[5] = (remain_size >> 16) & 0xFF;
			tmp_buf[6] = (remain_size >> 8) & 0xFF;
			tmp_buf[7] = remain_size & 0xFF;
			//*(uint32_t *)(&tmp_buf[4]) = remain_size;
			if(remain_size > VIDEO_BUF_SIZE - 8)
			{
				memcpy(&tmp_buf[8], &data[send_size], VIDEO_BUF_SIZE - 8);
				sendto(send_data->udp_conf.cur_socket, tmp_buf, VIDEO_BUF_SIZE, 0, addr, addr_len);
				remain_size -= (VIDEO_BUF_SIZE - 8);
				send_size += (VIDEO_BUF_SIZE - 8);
			}
			else
			{
				memcpy(&tmp_buf[8], &data[send_size], remain_size);
				sendto(send_data->udp_conf.cur_socket, tmp_buf, remain_size + 8, 0, addr, addr_len);
				break;
			}
		}
		usleep(2000);
		if(remain_size > VIDEO_BUF_SIZE)
		{
    		sendto(send_data->udp_conf.cur_socket, &data[send_size], VIDEO_BUF_SIZE, 0, addr, addr_len);
			remain_size -= VIDEO_BUF_SIZE;
			send_size += VIDEO_BUF_SIZE;
		}
		else
		{
			sendto(send_data->udp_conf.cur_socket, &data[send_size], remain_size, 0, addr, addr_len);
			remain_size = 0;
		}
	}
	free(tmp_buf);
	usleep(3000);
}

static void sender_init(MSFilter *f){
	SenderData *d =(SenderData *)ms_new0(SenderData,1);	
	f->data = d;
}

static void sender_process(MSFilter *f){
	SenderData *d = (SenderData *)f->data;
	mblk_t *m = NULL;	
	struct sockaddr_in addr;
	int addr_len = sizeof(struct sockaddr_in);
	void (*data_send)(unsigned char *data,unsigned int len_size,struct SenderData *send_data, struct sockaddr_in *addr,int addr_len);
	switch(d->udp_conf.c_type){
		case AUDIO_OUTPUT:
			data_send = audio_send;
			break;
		case VIDEO_OUTPUT:
			data_send = video_send;
			break;
	}
	//printf("++++++++ %s, %d\n", d->udp_conf.remote_ip, d->udp_conf.remote_port);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(d->udp_conf.remote_port);
	addr.sin_addr.s_addr = inet_addr(d->udp_conf.remote_ip);
	ms_filter_lock(f);
	while(m = ms_queue_get(f->inputs[0])){
		data_send(m->b_rptr,m->b_wptr - m->b_rptr,d,&addr,addr_len);
		freemsg(m);
		m = NULL;
		//usleep(1000);
	}
	ms_filter_unlock(f);
}

static void sender_uninit(MSFilter *f){
	SenderData *d = (SenderData *)(f->data);
	ms_filter_lock(f);
	close(d->udp_conf.cur_socket);
	ms_filter_unlock(f);
	printf("[sender_uninit]socket close OK!\n");
	ms_free(d);
}

#ifdef _MSC_VER

MSFilterDesc ms_udp_send_desc = {
	MS_UDP_SEND_ID,
	"MSUDPSend",
	N_("UDP output filter"),
	MS_FILTER_OTHER,
	NULL,
	1,
	0,
	sender_init,
	NULL,
	sender_process,
	NULL,
	sender_uninit,
	sender_methods,
	MS_FILTER_IS_PUMP
};

#else

MSFilterDesc ms_udp_send_desc = {
	.id = MS_UDP_SEND_ID,
	.name = "MSUDPSend",
	.text = N_("UDP output filter"),
	.category = MS_FILTER_OTHER,
	.ninputs = 1,
	.noutputs = 0,
	.init = sender_init,
	.process = sender_process,
	.uninit = sender_uninit,
	.methods = sender_methods,
	.flags=MS_FILTER_IS_PUMP
};

#endif

struct ReceiverData{
	msgb_allocator_t allocator;
	MSQueue data_queue;
	ms_thread_t recv_thread;
	udp_config_t udp_conf;
	volatile bool_t run_flag;
	volatile bool_t thread_exit;
};

typedef struct ReceiverData ReceiverData;

static void *video_receive_thread(void *arg)
{
	ReceiverData *d = (ReceiverData *)arg;
	struct sockaddr_in remote_addr;
	mblk_t *blk_tmp = NULL,*blk_cont = NULL,*blk_cont_tmp = NULL;
	int len = sizeof(struct sockaddr_in);
	int ret = 0;
	unsigned char *buf_ptr = NULL;
	unsigned int frame_size = 0,frame_size_count = 0,blk_len_tmp = 0;
	unsigned char *buf = NULL;
	bool_t start_recv = FALSE;
	fd_set readfds;
	struct timeval timeout = {0};

	buf = malloc(sizeof(char) * VIDEO_BUF_SIZE);
	d->run_flag = TRUE;
	d->thread_exit = FALSE;
	while(d->run_flag){
		FD_ZERO(&readfds);
		FD_SET(d->udp_conf.cur_socket,&readfds);
		timeout.tv_sec = 0;
		timeout.tv_usec = 5000;
		select(d->udp_conf.cur_socket+1,&readfds,NULL,NULL,&timeout); 

		memset(buf,'\0',VIDEO_BUF_SIZE);
		if(FD_ISSET(d->udp_conf.cur_socket,&readfds))
		{
			ret = recvfrom(d->udp_conf.cur_socket,buf,VIDEO_BUF_SIZE,0,(struct sockaddr*)&remote_addr,&len);
			buf_ptr = buf;
			//printf("receiver=%d\n", ret);
			while(ret > 0){
			if(memcmp(buf_ptr, start_code, 4) == 0)
			{
				if(blk_tmp != NULL){
					freemsg(blk_tmp);
				}
				frame_size = frame_size_count = blk_len_tmp = 0;					
				blk_tmp = blk_cont = blk_cont_tmp = NULL;
				frame_size = (buf_ptr[4] << 24) | (buf_ptr[5] << 16) | (buf_ptr[6] << 8) | buf_ptr[7];   
				//printf("get frame=%d\n", frame_size);
				ret -= 8;
				buf_ptr += 8;
				if (ret < 0)
				{
				    start_recv = FALSE;
				}
				else
				{
				    start_recv = TRUE;
				}
			}
			if(start_recv){
				blk_len_tmp = (frame_size_count + ret) > frame_size ? (frame_size - frame_size_count):ret;
				if(blk_tmp == NULL){
					blk_tmp = msgb_allocator_alloc(&(d->allocator),blk_len_tmp);
					memcpy(blk_tmp->b_wptr,buf_ptr,blk_len_tmp);
					blk_tmp->b_wptr += blk_len_tmp;
					frame_size_count += blk_len_tmp;
					mblk_set_timestamp_info(blk_tmp, (uint32_t)(d->udp_conf.cur_filter->ticker->time*90));
					//printf("time=0x%x\n", blk_tmp->reserved1);
				}else{
					blk_cont = msgb_allocator_alloc(&(d->allocator),blk_len_tmp);
					memcpy(blk_cont->b_wptr,buf_ptr,blk_len_tmp);
					blk_cont->b_wptr += blk_len_tmp;
					frame_size_count += blk_len_tmp;
					if(NULL == blk_cont_tmp){
						blk_tmp->b_cont = blk_cont;
					}else{
						blk_cont_tmp->b_cont = blk_cont;
					}
					blk_cont_tmp = blk_cont;
				}				
				ret -= blk_len_tmp;
				buf_ptr += blk_len_tmp;
				if(frame_size != 0 && frame_size == frame_size_count){
					start_recv = FALSE;
					if(blk_tmp != NULL)
						ms_queue_put(&(d->data_queue),blk_tmp);	
					frame_size = frame_size_count = blk_len_tmp = 0;					
					blk_tmp = blk_cont = blk_cont_tmp = NULL;
				}
			}else{
				printf("unknow data:ret = %d\n",ret);
				break;
			}
			}
			usleep(1000);
		}
		//usleep(1000);
	}
	if(buf)
		free(buf);
	d->thread_exit = TRUE;
}

static void *audio_receive_thread(void *arg)
{
	ReceiverData *d = (ReceiverData *)arg;
	mblk_t *blk_tmp = NULL;
	unsigned char *buf = NULL;
	int recv_size = 0;
	struct sockaddr_in remote_addr;
	int len = sizeof(struct sockaddr_in);
	fd_set readfds;
	struct timeval timeout = {0};

	buf = malloc(sizeof(char) * AUDIO_BUF_SIZE);
	d->run_flag = TRUE;
	d->thread_exit = FALSE;
	while(d->run_flag){
		FD_ZERO(&readfds);
		FD_SET(d->udp_conf.cur_socket,&readfds);
		timeout.tv_sec = 0;
		timeout.tv_usec = 5000;
		select(d->udp_conf.cur_socket+1,&readfds,NULL,NULL,&timeout); 

		memset(buf,'\0',AUDIO_BUF_SIZE);
		if(FD_ISSET(d->udp_conf.cur_socket,&readfds))
		{
			recv_size = recvfrom(d->udp_conf.cur_socket,buf,AUDIO_BUF_SIZE,0,(struct sockaddr*)&remote_addr,&len);
			//printf("audio recv %d\n", recv_size);
			if(blk_tmp == NULL){
				blk_tmp = msgb_allocator_alloc(&(d->allocator),recv_size);
				memcpy(blk_tmp->b_wptr,buf,recv_size);
				blk_tmp->b_wptr += recv_size;
				mblk_set_timestamp_info(blk_tmp, (uint32_t)(d->udp_conf.cur_filter->ticker->time*8));
				//printf("time=0x%x\n", blk_tmp->reserved1);
			}
			if(blk_tmp != NULL)
			{
				ms_queue_put(&(d->data_queue),blk_tmp);
				blk_tmp = NULL;
			}
			usleep(5000);
		}
		usleep(1000);
	}
	if(buf)
		free(buf);
	d->thread_exit = TRUE;
}

static int *parse_264_file_thread(void *arg)
{
    ReceiverData *d = (ReceiverData *)arg;
    mblk_t *m = NULL;
    
    // ffmpeg
    AVPacket       avpkt;

    uint8_t       *inbuf          = NULL;
    unsigned int  inbuf_size      = INPUT_BUFFER_SIZE;

    unsigned char *pBufferStart   = NULL, *pBufferEnd = NULL;
    unsigned char *pCurFrameStart = NULL, *pLastFrameStart = NULL;
    bool_t          bReadFromFile   = TRUE;
    int           ret             = 0;
    bool_t          bEOS            = FALSE;
    // file
    FILE          *f;
    
    inbuf = malloc(sizeof(char) * INPUT_BUFFER_SIZE);
    av_init_packet(&avpkt);
    avpkt.timestamp = 0;
    
    pCurFrameStart = pLastFrameStart = pBufferStart = inbuf;
    f = fopen("E:/test.264", "rb");
    while (!bEOS)
    {
        unsigned int copy_size = 0;
        while (1) // find frame
        {
            if (copy_size >= INPUT_BUFFER_SIZE) // temp solution, need modify
            {
                printf("[%s] Out of allocated parsing buffer size (%d)\n", __FUNCTION__, INPUT_BUFFER_SIZE);
            }
            
            // read pattern
            if (bReadFromFile)
            {
                inbuf_size = fread(pBufferStart + copy_size, 1, INPUT_BUFFER_SIZE - copy_size, f);
                if (inbuf_size == 0) // EOS
                {
                    bEOS = TRUE;
                    pCurFrameStart = pBufferStart + copy_size; // output remain data
                    break;
                }

                pBufferEnd    = pBufferStart + copy_size + inbuf_size;
                bReadFromFile = FALSE;
            }

            for (; pCurFrameStart <= (pBufferEnd - 8); pCurFrameStart++)
            {
                if (pCurFrameStart[0] == 0 && pCurFrameStart[1] == 0 && pCurFrameStart[2] == 0 && pCurFrameStart[3] == 1)
                {
                    if(pCurFrameStart[4] == 0x67 || pCurFrameStart[4] == 0x68 || pCurFrameStart[4] == 0x65 || pCurFrameStart[4] == 0x41 || pCurFrameStart[4] == 0x61 || pCurFrameStart[4] == 0x06) break;
                }
            }

            // Buffer boundary, copy remain data to buffer start point
            if (pCurFrameStart > (pBufferEnd - 8))
            {
                unsigned int parsing_offset = pCurFrameStart - pLastFrameStart;

                copy_size = pBufferEnd - pLastFrameStart;

                memcpy(pBufferStart, pLastFrameStart, copy_size);

                pLastFrameStart = pBufferStart;
                pCurFrameStart  = pBufferStart + parsing_offset;

                bReadFromFile   = TRUE;

                continue;
            }

            break;
        }

        avpkt.data      = pLastFrameStart;
        avpkt.size      = pCurFrameStart - pLastFrameStart;
        if((avpkt.data[4] == 0x67 || avpkt.data[4] == 0x68 || avpkt.data[4] == 0x65 || avpkt.data[4] == 0x41 || avpkt.data[4] == 0x61 || avpkt.data[4] == 0x06) && avpkt.size != 0)
        {
            if(avpkt.data[4] == 0x65 || avpkt.data[4] == 0x41 || avpkt.data[4] == 0x61)
                avpkt.timestamp += 0.033;
            m = allocb(avpkt.size + 10, 0);
            if (m != NULL)
            {
                printf("%x %x %x %x %x\n", avpkt.data[0], avpkt.data[1], avpkt.data[2], avpkt.data[3], avpkt.data[4]);
                memcpy(m->b_wptr, avpkt.data, avpkt.size);
                m->b_wptr += avpkt.size;
                m->b_rptr += 4;
                mblk_set_timestamp_info(m, (uint32_t)(avpkt.timestamp*90000));
                ms_queue_put(&(d->data_queue), m);
            }
            usleep(30000);
        }
        pLastFrameStart = pCurFrameStart;
        pCurFrameStart += 3; // next start code
        usleep(1000);
    }

    if (f)
        fclose(f);
    f              = NULL;
    if (inbuf)
        free(inbuf);
    inbuf          = NULL;
    pCurFrameStart = pLastFrameStart = pBufferStart = inbuf;
    return 0;
}

static int udp_receiver_set_para(MSFilter * f, void *arg){
	ReceiverData *d = (ReceiverData *)f->data;
	udp_config_t *udp_conf = (udp_config_t*)arg;
	struct sockaddr_in addr;
	pthread_attr_t attr;
	int err = 0;
	memset(&(d->udp_conf),'\0',sizeof(udp_config_t));
	d->udp_conf.cur_filter = f;
	d->udp_conf.c_type = udp_conf->c_type;
	memcpy(d->udp_conf.group_ip,udp_conf->group_ip,16);
	memcpy(d->udp_conf.remote_ip,udp_conf->remote_ip,16);
	d->udp_conf.remote_port= udp_conf->remote_port;
	d->udp_conf.cur_socket = udp_conf->cur_socket;
	
	if(d->udp_conf.cur_socket == -1){		
		d->udp_conf.cur_socket = socket(AF_INET,SOCK_DGRAM,0);
		if(d->udp_conf.cur_socket < 0){
			printf("[udprecv_set_para]sockek create failure\n");
		}else
			printf("[udprecv_set_para]udp socket create OK!\n");
		memset(&addr,'\0',sizeof(struct sockaddr_in));
		addr.sin_family=AF_INET;
		addr.sin_addr.s_addr=htonl(INADDR_ANY);
		addr.sin_port=htons(d->udp_conf.remote_port);
		if( bind (d->udp_conf.cur_socket,(struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0 ){
			printf("[udprecv_set_para]bind udp  error!\n"); 
		}
		else
			printf("[udprecv_set_para]bind udp ok! \n");
	}
	
	if(d->udp_conf.cur_socket != -1){
		if(d->udp_conf.group_ip[0] != '\0' && d->udp_conf.enable_multicast){		
			d->udp_conf.mreq.imr_multiaddr.s_addr = inet_addr(d->udp_conf.group_ip);
			d->udp_conf.mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			
			err = setsockopt(d->udp_conf.cur_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,&(d->udp_conf.mreq), sizeof(d->udp_conf.mreq));
			if (err < 0){
				perror("[udprecv_set_para]setsockopt():IP_ADD_MEMBERSHIP");
			}
		}	
	}
	
	pthread_attr_init(&attr);
	switch(d->udp_conf.c_type){
		case AUDIO_INPUT:
			pthread_create(&(d->recv_thread), &attr, audio_receive_thread, d);
			break;
		case VIDEO_INPUT:
			pthread_create(&(d->recv_thread), &attr, video_receive_thread, d);
			break;
	}
}

static int udp_receiver_get_socket(MSFilter * f, void *arg){	
	ReceiverData *d = (ReceiverData *)f->data;
	((int *)arg)[0] = d->udp_conf.cur_socket;
}

static MSFilterMethod receiver_methods[] = {
	{	MS_UDP_RECV_SET_PARA	, udp_receiver_set_para	},		
	{	MS_UDP_RECV_GET_SOCKET	, udp_receiver_get_socket},
	{	0, NULL}
};

static void receiver_init(MSFilter *f){
	ReceiverData *d =(ReceiverData *) ms_new0(ReceiverData,1);
	ms_queue_init(&(d->data_queue));	
	msgb_allocator_init(&(d->allocator));
#if FILE_MODE	
    pthread_create(&(d->recv_thread), NULL, parse_264_file_thread, d);
#endif
	f->data = d;
}

static void receiver_process(MSFilter *f){
	ReceiverData *d = (ReceiverData *)f->data;
	mblk_t *m = NULL;
	while(m = ms_queue_get(&(d->data_queue))){
		ms_queue_put(f->outputs[0], m);
    }
}

static void receiver_uninit(MSFilter *f){
	ReceiverData *d = (ReceiverData *)(f->data);
	d->run_flag = FALSE;		
	ms_thread_join(d->recv_thread,NULL);
	if(d->udp_conf.group_ip[0] != '\0' && d->udp_conf.enable_multicast){
		setsockopt(d->udp_conf.cur_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,&(d->udp_conf.mreq), sizeof(d->udp_conf.mreq));
	}
	close(d->udp_conf.cur_socket);
	printf("[receiver_uninit]socket close OK!\n");

	ms_queue_flush(&(d->data_queue));	
	msgb_allocator_uninit(&(d->allocator));
	ms_free(d);	
}

#ifdef _MSC_VER

MSFilterDesc ms_udp_recv_desc = {
	MS_UDP_RECV_ID,
	"MSUDPRecv",
	N_("UDP input filter"),
	MS_FILTER_OTHER,
	NULL,
	0,
	1,
	receiver_init,
	NULL,
	receiver_process,
	NULL,
	receiver_uninit,
	receiver_methods
};

#else

MSFilterDesc ms_udp_recv_desc = {
	.id = MS_UDP_RECV_ID,
	.name = "MSUDPRecv",
	.text = N_("UDP input filter"),
	.category = MS_FILTER_OTHER,
	.ninputs = 0,
	.noutputs = 1,
	.init = receiver_init,
	.process = receiver_process,
	.uninit = receiver_uninit,
	.methods = receiver_methods
};

#endif


MS_FILTER_DESC_EXPORT(ms_udp_send_desc)
MS_FILTER_DESC_EXPORT(ms_udp_recv_desc)
