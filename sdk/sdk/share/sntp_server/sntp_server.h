#ifndef __SNTP_SERVER_H__
#define __SNTP_SERVER_H__

void sntp_server_init(int port);
void sntp_server_exit(void);
void* sntp_server_thread(void* arg);

#endif /* __SNTP_SERVER_H__ */
