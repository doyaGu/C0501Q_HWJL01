#include <sys/stat.h>
#include <assert.h>
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ctrlboard.h"

static bool photoInited, photoQuit, photoLoaded;
static char photoPath[PATH_MAX];
static sem_t photoSem;
static pthread_t photoTask;
static PhotoLoadCallback photoLoadCallback;

static void* PhotoLoadTask(void* arg);

void PhotoInit(void)
{
    photoInited = photoQuit = photoLoaded = false;

    sem_init(&photoSem, 0, 0);

    pthread_create(&photoTask, NULL, PhotoLoadTask, NULL);

    photoInited = true;
}

void PhotoExit(void)
{
    if (!photoInited)
        return;

    photoQuit = true;
    sem_post(&photoSem);

    pthread_join(photoTask, NULL);
}

static void* PhotoLoadTask(void* arg)
{
    for (;;)
    {
        int size = 0;
        uint8_t* data = NULL;
        FILE* f;
        struct stat sb;

        sem_wait(&photoSem);

        if (photoQuit)
            break;

        assert(photoLoadCallback);

        printf("Load %s\n", photoPath);
    
        // try to load screensaver jpeg file if exists
        f = fopen(photoPath, "rb");
        if (f)
        {
            if (fstat(fileno(f), &sb) != -1)
            {
                size = sb.st_size;
                data = malloc(size);
                if (data)
                {
                    size = fread(data, 1, size, f);
                }
            }
            fclose(f);
        }

        // return result
        photoLoadCallback(data, size);
        photoLoadCallback = NULL;

        if (photoQuit)
            break;
    }
    return NULL;
}

int PhotoLoad(char* filename, PhotoLoadCallback func)
{
    if (photoLoadCallback || photoQuit)
        return -1;

    strncpy(photoPath, filename, PATH_MAX);

    photoLoadCallback    = func;
    sem_post(&photoSem);

    return 0;
}
