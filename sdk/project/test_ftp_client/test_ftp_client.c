#include <unistd.h>
#include "curl/curl.h"
#include "ite/itp.h"

typedef enum TAG_SEARCH_STATE_TAG
{
    SEARCH_TAG_START,
    GET_TAG_PATH,
    GET_TAG_ACCOUNT,
    GET_TAG_PASSWORD,
    GET_TAG_FILE_NAME,
    SEARCH_TAG_END
} TAG_SEARCH_STATE;

static uint32_t write_data(void *ptr, uint32_t size, uint32_t nmemb, FILE *stream) {
    uint32_t written = fwrite(ptr, size, nmemb, stream);
    printf("+++download size %d+++\n", nmemb);
    return written;
}

static uint32_t GetFilesList_response(void *ptr, uint32_t size, uint32_t nmemb, void *data)
{
    FILE *writehere = (FILE *)data;
    printf("#################list##############\n");
    printf("%s\n", ptr);
    printf("###################################\n");
    return fwrite(ptr, size, nmemb, writehere);
}

int Photo_DownloadFromURL(char* url_path)
{
    CURL *curl;
    FILE *fp, *pCfgFile;
    CURLcode res;
    uint32_t r_size, w_size, bufferPos = 0, parsePos = 0;
	uint8_t *pReadFileBuffer, pParse_PathStr[256] = {0}, pParse_FileStr[32]; 
    int errCode = 0;
    //char *url = "http://pgw.udn.com.tw/gw/photo.php?u=https://uc.udn.com.tw/photo/2017/03/22/99/3309883.jpg";
    //char outfilename[256] = "A:download _url.jpg";
	
	pCfgFile = fopen("A:/url.txt", "rb");
    errCode = fseek(pCfgFile, 0L, SEEK_END);
    if (errCode < 0) {
        r_size = 0;
    }
    r_size = ftell(pCfgFile);
    errCode = fseek(pCfgFile, 0L, SEEK_SET);
    if (errCode < 0) {
        r_size = 0;
    }
    pReadFileBuffer = (uint8_t*)malloc(r_size);
	if(pReadFileBuffer != NULL)
    {
        fread(pReadFileBuffer, 1, r_size, pCfgFile);
    }
	fclose(pCfgFile);		    
    if (pReadFileBuffer)
    {   
        TAG_SEARCH_STATE tagState = SEARCH_TAG_START;
        while (bufferPos < r_size)
        {
            switch(tagState)
            {
                case SEARCH_TAG_START:
                {
                    if (pReadFileBuffer[bufferPos] == '<')
                    {
                        tagState = GET_TAG_PATH;
                    }
                    break;
                }
                case GET_TAG_PATH:
                {
                    if (pReadFileBuffer[bufferPos] == '#')
                    {
						pParse_PathStr[parsePos] = '\0';
                        parsePos = 0;
                        tagState = GET_TAG_FILE_NAME;
                    }
                    else
                    {
                        pParse_PathStr[parsePos++] = pReadFileBuffer[bufferPos];
                    }
                    break;
                }
                case GET_TAG_FILE_NAME:
                {
                    if (pReadFileBuffer[bufferPos] == '>')
                    {
						pParse_FileStr[parsePos] = '\0';
                        tagState = SEARCH_TAG_END;
                    }
                    else
                    {
                        pParse_FileStr[parsePos++] = pReadFileBuffer[bufferPos];
                    }
                    break;
                }
			}
			if(tagState == SEARCH_TAG_END)
				break;
			bufferPos++;
		}
	}
	printf("download %s to %s\n", pParse_PathStr, pParse_FileStr);
    curl = curl_easy_init();
    if (curl) {
        fp = fopen((char*)pParse_FileStr,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, pParse_PathStr);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        w_size = ftell(fp);
        fclose(fp);
        if(!w_size)
            remove((char*)pParse_FileStr);
    }
    return 0;
}

int Photo_DownloadFromFtp(char* ftp_path)
{
    CURL *curl;
    FILE *fp, *pCfgFile, *ftpfile;
    CURLcode res;
    uint32_t r_size, w_size, bufferPos = 0, parsePos = 0;
	uint8_t *pReadFileBuffer, pParse_PathStr[256] = {0}, pParse_ACTStr[32] = {0}, pParse_PWDStr[16] = {0}, pParse_FileStr[32] = {0}; 
    int errCode = 0;
    char ftpurl[256] = {0}, file_name[128] = {0};
	
	pCfgFile = fopen("A:/ftp.txt", "rb");
    errCode = fseek(pCfgFile, 0L, SEEK_END);
    if (errCode < 0) {
        r_size = 0;
    }
    r_size = ftell(pCfgFile);
    errCode = fseek(pCfgFile, 0L, SEEK_SET);
    if (errCode < 0) {
        r_size = 0;
    }
    pReadFileBuffer = (uint8_t*)malloc(r_size);
	if(pReadFileBuffer != NULL)
    {
        fread(pReadFileBuffer, 1, r_size, pCfgFile);
    }
	fclose(pCfgFile);		    
    if (pReadFileBuffer)
    {   
        TAG_SEARCH_STATE tagState = SEARCH_TAG_START;
        while (bufferPos < r_size)
        {
            switch(tagState)
            {
                case SEARCH_TAG_START:
                {
                    if (pReadFileBuffer[bufferPos] == '<')
                    {
                        tagState = GET_TAG_PATH;
                    }
                    break;
                }
                case GET_TAG_PATH:
                {
                    if (pReadFileBuffer[bufferPos] == '#')
                    {
						pParse_PathStr[parsePos] = '\0';
                        parsePos = 0;
                        tagState = GET_TAG_ACCOUNT;
                    }
                    else
                    {
                        pParse_PathStr[parsePos++] = pReadFileBuffer[bufferPos];
                    }
                    break;
                }
                case GET_TAG_ACCOUNT:
                {
                    if (pReadFileBuffer[bufferPos] == ':')
                    {
						//pParse_ACTStr[parsePos] = '\0';
						pParse_ACTStr[parsePos++] = pReadFileBuffer[bufferPos];
                        tagState = GET_TAG_PASSWORD;
                    }
                    else
                    {
                        pParse_ACTStr[parsePos++] = pReadFileBuffer[bufferPos];
                    }
                    break;
                }
                case GET_TAG_PASSWORD:
                {
                    if (pReadFileBuffer[bufferPos] == '#')
                    {
						pParse_ACTStr[parsePos] = '\0';
                        parsePos = 0;
                        tagState = GET_TAG_FILE_NAME;
                    }
                    else
                    {
                        pParse_ACTStr[parsePos++] = pReadFileBuffer[bufferPos];
                    }
                    break;
                }
                case GET_TAG_FILE_NAME:
                {
                    if (pReadFileBuffer[bufferPos] == '>')
                    {
						pParse_FileStr[parsePos] = '\0';
                        parsePos = 0;
                        tagState = SEARCH_TAG_END;
                    }
                    else
                    {
                        pParse_FileStr[parsePos++] = pReadFileBuffer[bufferPos];
                    }
                    break;
                }
			}
			if(tagState == SEARCH_TAG_END)
				break;
			bufferPos++;
		}
	}
	printf("ftp from %s(%s) to %s\n", pParse_PathStr, pParse_ACTStr, pParse_FileStr);
    fp = fopen("A:/list.txt", "wb");
    curl = curl_easy_init();
    if(curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, pParse_PathStr);
        curl_easy_setopt(curl, CURLOPT_USERPWD, pParse_ACTStr);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GetFilesList_response);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 1);

        /* some servers don't like requests that are made without a user-agent
             field, so we provide one */ 
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, 15L);
#ifndef NDEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
        /* get file list! */
        res = curl_easy_perform(curl);
        printf("######ftp status=%d####\n", res);
        fclose(fp);
        if (CURLE_OK != res)
        {
            printf("curl_easy_perform() fail: %d\n", res);
            goto error;
        }
        
        snprintf(ftpurl, sizeof(ftpurl), "%s/%s", pParse_PathStr, pParse_FileStr);
        snprintf(file_name, sizeof(file_name), "A:/%s", pParse_FileStr);
        printf("ftp download from %s to %s\n", ftpurl, file_name);
            
        ftpfile = fopen(file_name, "wb");
        /* specify URL to get */ 
        curl_easy_setopt(curl, CURLOPT_URL, ftpurl);

        /* send all data to this function  */ 
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        /* we pass our 'chunk' struct to the callback function */ 
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, ftpfile);
        curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 0);

        /* get it! */
        res = curl_easy_perform(curl);
        fclose(ftpfile);
        
        if (CURLE_OK != res)
        {
            printf("curl_easy_perform() fail: %d\n", res);
            goto error;
        }
        curl_easy_cleanup(curl);
    }
    else
    {
        printf("curl_easy_init() fail.\n");
        fclose(fp);
        return 0;
    }
    printf("##########################FTP download success!!!############################\n");
    return 1;
error:
    if (curl)
        curl_easy_cleanup(curl);
    printf("##########################FTP download failed!!!############################\n");
    return 0;
}

void* TestFunc(void *arg)
{        
		itpInit();
    ConfigInit();

#ifdef CFG_NET_ENABLE
    NetworkInit();
#endif // CFG_NET_ENABLE

    while(1)
    {
        if(NetworkIsReady())
            break;
        else
            usleep(1000);
    }
    Photo_DownloadFromFtp(NULL);
    return NULL;
}
