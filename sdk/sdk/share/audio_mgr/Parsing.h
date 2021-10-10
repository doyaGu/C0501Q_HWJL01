#ifdef CFG_AUDIOLINK_UPNP_PARSING_DATA_ENABLE

#ifndef PARSING_H
#define PARSING_H

#ifdef __cplusplus
extern "C" {
#endif

// init parsing buffer
int parsing_data_init();

// pBuf is pointer to buf, nBufSize is buffer size,nType is audio type
// nType 1:mp3, 
// return audio frame count 
int parsing_data(char* pBuf,int nBufSize,int nType);

// return current parsing time
int parsing_data_get_current_time();

#ifdef __cplusplus
}
#endif

#endif /* PARSING_H */


#endif
