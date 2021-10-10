#include <sys/time.h>
#include <errno.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"

#define BUFFER 40

static mqd_t queue;
static const char* data = "Test Message";

static void* Test_mqueue_receive(void* arg)
{
	char rcvdata[BUFFER];
    struct   timespec tm;

	// Receive the data
	if(mq_receive(queue, rcvdata , strlen(data) , NULL) == -1)
		puts("Error from mq_receive");
	else
		printf("Received Data : %s\n",rcvdata);

    clock_gettime(CLOCK_REALTIME, &tm);
    tm.tv_sec += 2;  // Set for 2 seconds

	// Receive the data
	if(mq_timedreceive(queue, rcvdata , strlen(data) , NULL, &tm) == -1)
		puts("Error from mq_receive");
	else
		printf("Received Data : %s\n",rcvdata);

    return NULL;
}

void Test_mqueue(void)
{
	const char* queuename = "test_mq";

  	struct mq_attr attr;
	 
	// First we need to set up the attribute structure   	  
  	attr.mq_maxmsg = BUFFER;
    attr.mq_msgsize = BUFFER;
  	attr.mq_flags = 0;
	
	// Open a queue with the attribute structure
  	queue = mq_open(queuename, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR , &attr );
  	if (queue != (mqd_t)-1 ) 
 	{
        pthread_t task;
        struct   timespec tm;

        pthread_create(&task, NULL, Test_mqueue_receive, NULL);

		// Send the data
		if(mq_send(queue, data, strlen(data), 1) == -1)
			puts("Error from mq_send");		
		else
			printf("Sent Data : %s\n",data);

		printf("Wait...\n");
		sleep(2);

        clock_gettime(CLOCK_REALTIME, &tm);
        tm.tv_sec += 2;  // Set for 2 seconds

		// Send the data
        if (mq_timedsend(queue, data, strlen(data), 1, &tm) == -1)
			puts("Error from mq_timedsend");		
		else
			printf("Sent Data : %s\n",data);

        pthread_join(task, NULL);
 	}
 	else
        perror("'mq_open' failed for : ");

	// Close open message queue descriptors  	
 	mq_close(queue);
}
    



void* TestFunc(void* arg)
{
  	itpInit();
	
	printf("\nTest: mq_open(),mq_send(),mq_receive(),mq_close()\n-----------------\n");
	Test_mqueue();
    
    printf("\nEnd the test\n");   

    return NULL;
}

