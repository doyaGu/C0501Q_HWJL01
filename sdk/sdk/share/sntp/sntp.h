#ifndef __SNTP_H__
#define __SNTP_H__

void sntp_init(void);
void sntp_deinit(void);
void sntp_add_server_address(char* addr);
void sntp_clear_server_addresses(void);
void sntp_set_update_delay(int sec);
void sntp_set_port(int port);
void sntp_update(void);

#endif /* __SNTP_H__ */
