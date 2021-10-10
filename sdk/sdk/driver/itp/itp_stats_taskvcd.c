/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Task Statistics VCD Dump.
 *
 * To view the VCD file, please download Wave VCD Viewer from
 * http://www.iss-us.com/wavevcd/index.htm.
 *
 * Insert following function call to start trace the task context
 * switch event.
 *
 *   itpTaskVcdOpen();
 *
 * And insert following code to the main task to dump the VCD file
 * when the trace complete.
 *
 *   itpTaskVcdWrite();
 *
 * Insert tag
 *
 *   itpTaskVcdSetTag(int);
 *
 * @author Kuoping Hsu
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "itp_cfg.h"

#if defined(CFG_DBG_TRACE_ANALYZER) && defined(CFG_DBG_VCD)

// Timer defines
#define VCD_TIMER           (ITH_TIMER3)

// Define 1 to measure memory bandwidth usage.
#define MEMORY_BANDWIDTH_ANALYSIS (1)

// Define 1 to save VCD file to starage devices, define 0 to store the data on memory buffer
#define VCD_DUMP_TO_FILE    (0)

// Define the store mode, 1 for ring buffer, 0 will stop record when the buffer is full.
#define VCD_DUMP_RINGBUF    (1)

// Define the maximun number of event to record on VCD_DUMP_TO_FILE == 0
#define MAX_EVENT_RECORD    (16384)

// Define the maximun task to record, maximun number is 8914
#define MAX_TASK_RECORD     (2048)

/***************************************************************************
 *                              Private Constant
 ***************************************************************************/
// The related parameter for VCD foramt file
#define BEGIN_SYMBOL        (33)  // '!'
#define END_SYMBOL          (126) // '~'
#define VCD_SIG_RANGE       (END_SYMBOL - BEGIN_SYMBOL + 1)
#define MAX_SIG_RANGE       (VCD_SIG_RANGE)*(VCD_SIG_RANGE+1)
#define MODE_MASK           (0xf0000000)
#define MODE_TASK_IN        (0x10000000)
#define MODE_TASK_OUT       (0x20000000)
#define MODE_TASK_DELAY     (0x30000000)
#define MODE_TASK_DELETE    (0x40000000)
#define MODE_MEM_USAGE      (0x50000000)
#define MODE_TAG            (0x60000000)

// disable ring buffer on the file mode.
#if VCD_DUMP_TO_FILE == 1
#  undef  VCD_DUMP_RINGBUF
#  define VCD_DUMP_RINGBUF  (0)
#endif

// check the maximun size of MAX_TASK_RECORD
#if MAX_TASK_RECORD > (MAX_SIG_RANGE - 16)
#  undef  MAX_TASK_RECORD
#  define MAX_TASK_RECORD   (MAX_SIG_RANGE-16)
#endif

/***************************************************************************
 *                              Private Variable
 ***************************************************************************/
#define g_task_name RecorderData.pxTCBName
#define g_last_time RecorderData.last_time
#define g_buf_idx   RecorderData.buf_idx
#define g_buf_full  RecorderData.buf_full
#define g_trace_buf RecorderData.trace_buf
#define g_max_id    RecorderData.max_id
#define g_max_tag   RecorderData.max_tag
struct _RecorderData {
    portCHAR       header[8];   // "KTRACE"
    unsigned int   endian;      // Endian Tag
    unsigned int   tag1;        // MAX_EVENT_RECORD
    unsigned int   tag2;        // configMAX_TASK_NAME_LEN+5
    unsigned int   tag3;        // MEMORY_BANDWIDTH_ANALYSIS
    unsigned int   last_time;   // last event time (OS tick in ms)
    unsigned int   max_id;
    unsigned int   max_tag;
    unsigned int   buf_idx;
    unsigned int   buf_full;
    portCHAR       pxTCBName[MAX_TASK_RECORD][configMAX_TASK_NAME_LEN+5];
    #if VCD_DUMP_TO_FILE == 1
    unsigned int  *trace_buf;
    #else
    unsigned int   trace_buf[MAX_EVENT_RECORD*2];
    #endif
} RecorderData;

static          unsigned int   g_buf_size   = 0;
static          unsigned int * g_buf_ptr    = 0;
static volatile unsigned int   g_enable     = 0;
static volatile unsigned int   g_dump       = 0;
static          const char   * g_fname      = (char*)0;
static          unsigned int * g_last_ptr   = 0;

/***************************************************************************
 *                              Private Functions
 ***************************************************************************/
static unsigned int
getDiffTime(
    void)
{
    uint32_t diff, time = ithTimerGetCounter(VCD_TIMER);
    static uint32_t clock = 0;

    if (time >= clock)
    {
        diff = time - clock;
    }
    else
    {
        diff = (0xFFFFFFFF - clock) + 1 + time;
    }

    diff = (uint64_t)diff * 1000000 / ithGetBusClock();

    g_last_time = itpGetTickCount();
    clock = ithTimerGetCounter(VCD_TIMER);

    return diff;
}

static char *
dec2bin(int n)
{
    static char bin[33];
    int i;
    for(i=0; i<32; i++)
    {
        bin[i] = (n&0x80000000) ? '1' : '0';
        n <<= 1;
    }
    bin[32] = 0;
    return bin;
}

static char *
sigsymbol(int n)
{
    int  a, b;
    static char str[4];

    a = n / VCD_SIG_RANGE;
    b = n % VCD_SIG_RANGE;

    if (a == 0) str[1] = (char)0;
    else        str[1] = (char)(BEGIN_SYMBOL + a - 1);
    str[0] = (char)(BEGIN_SYMBOL + b);
    str[2] = (char)0;

    return (char*)&str;
}

static
vcd_dump(void)
{
#if VCD_DUMP_TO_FILE == 1
    #define BUFSIZE   (10 << 10)
    #define TIMESCALE 1
    int n, i, j;
    unsigned int t;
    FILE* fp;
    char *buf = NULL, *outbuf = NULL;
    unsigned int event_size;
    unsigned int total_time;
    unsigned long long current_time;

    if (g_buf_full)
        event_size = g_buf_size;
    else
        event_size = g_buf_idx;

    g_enable = 0;
    if (!g_fname)
    {
        printf("[RTOS][DUMP] No dump name is specified!!\n");
        goto end;
    }

    LOG_DBG "dump to %s\n", g_fname LOG_END
    if ((fp = fopen(g_fname, "wb")) == NULL)
    {
        printf("[RTOS][DUMP] Can not create dump file!!\n");
        goto end;
    }

    if (!g_buf_idx && !g_buf_full)
    {
        printf("[RTOS][DUMP] No data to dump!!\n");
        goto end;
    }

    //vTaskSuspendAll();
    printf("[RTOS][DUMP] Starting dump....\n");

    if (!(buf = malloc(BUFSIZE+80)))
    {
        printf("[RTOS][DUMP] Out of memory!!\n");
        goto end;
    }

    if ((g_max_id + 1) + (g_max_tag + 1 ) >= MAX_SIG_RANGE)
    {
        g_max_id = MAX_SIG_RANGE - g_max_tag - 2;
        printf("[RTOS][DUMP] Out of signal to dump!!\n");
    }

    g_buf_ptr = g_trace_buf;

    snprintf(buf, BUFSIZE+80, "$version\n");                          fwrite(buf, 1, strlen(buf), fp);
    snprintf(buf, BUFSIZE+80, "   OpenRTOS Context Switch dump.\n");  fwrite(buf, 1, strlen(buf), fp);
    snprintf(buf, BUFSIZE+80, "$end\n");                              fwrite(buf, 1, strlen(buf), fp);
    snprintf(buf, BUFSIZE+80, "$comment\n");                          fwrite(buf, 1, strlen(buf), fp);
    snprintf(buf, BUFSIZE+80, "   ITE Tech. Corp. Written by Kuoping Hsu, Dec. 2010\n"); fwrite(buf, 1, strlen(buf), fp);
    snprintf(buf, BUFSIZE+80, "$end\n");                              fwrite(buf, 1, strlen(buf), fp);
    snprintf(buf, BUFSIZE+80, "$timescale %dus $end\n", TIMESCALE);   fwrite(buf, 1, strlen(buf), fp);
    snprintf(buf, BUFSIZE+80, "$scope module task $end\n");           fwrite(buf, 1, strlen(buf), fp);

    for(i=1; i<=g_max_id; i++) // ignore task ID 0
    {
        snprintf(buf, BUFSIZE+80, "$var wire 1 %s %s $end\n", sigsymbol(i), g_task_name[i]);
        fwrite(buf, 1, strlen(buf), fp);
    }

    for(j=0; j<=g_max_tag; j++)
    {
        snprintf(buf, BUFSIZE+80, "$var wire 32 %s tag%02d $end\n", sigsymbol(i+j), j);
        fwrite(buf, 1, strlen(buf), fp);
    }

    #if MEMORY_BANDWIDTH_ANALYSIS == 1
    snprintf(buf, BUFSIZE+80, "$var wire 32 %s %s $end\n", sigsymbol(i+j), "mem_usage");
    fwrite(buf, 1, strlen(buf), fp);
    #endif // MEMORY_BANDWIDTH_ANALYSIS == 1

    snprintf(buf, BUFSIZE+80, "$upscope $end\n");         fwrite(buf, 1, strlen(buf), fp);
    snprintf(buf, BUFSIZE+80, "$enddefinitions $end\n");  fwrite(buf, 1, strlen(buf), fp);
    snprintf(buf, BUFSIZE+80, "$dumpvars\n");             fwrite(buf, 1, strlen(buf), fp);

    for(i=1; i<=g_max_id; i++) // ignore task ID 0
    {
        snprintf(buf, BUFSIZE+80, "x%s\n", sigsymbol(i));
        fwrite(buf, 1, strlen(buf), fp);
    }

    for(j=0; j<=g_max_tag; j++)
    {
        snprintf(buf, BUFSIZE+80, "bxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx %s\n", sigsymbol(i+j));
        fwrite(buf, 1, strlen(buf), fp);
    }

    #if MEMORY_BANDWIDTH_ANALYSIS == 1
    snprintf(buf, BUFSIZE+80, "bxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx %s\n", sigsymbol(i+j));
    fwrite(buf, 1, strlen(buf), fp);
    #endif // MEMORY_BANDWIDTH_ANALYSIS == 1

    snprintf(buf, BUFSIZE+80, "$end\n");
    fwrite(buf, 1, strlen(buf), fp);

    outbuf = buf;
    n = 0;
    i = g_buf_full ? g_buf_idx*2 : 0;
    j = 0;
    t = 0;
    while(n++ < event_size)
    {
        if (((g_buf_ptr[i] & MODE_MASK) != MODE_TASK_IN)     &&
            ((g_buf_ptr[i] & MODE_MASK) != MODE_TASK_OUT)    &&
            ((g_buf_ptr[i] & MODE_MASK) != MODE_TASK_DELAY)  &&
            ((g_buf_ptr[i] & MODE_MASK) != MODE_TASK_DELETE) &&
            ((g_buf_ptr[i] & MODE_MASK) != MODE_TAG)         &&
            ((g_buf_ptr[i] & MODE_MASK) != MODE_MEM_USAGE))
        {
            printf("[RTOS][DUMP] unknown task stat 0x%08x.\n", g_buf_ptr[i]);
        } else {
            total_time += g_trace_buf[i+1];
            if (total_time < t) {
                printf("[RTOS][DUMP] Error: time stamp overflow!!\n");
                return;
            }
            t = total_time;
        }

        i += 2;

        if (i >= g_buf_size * 2)
            i = 0;
    }
    printf("[RTOS][DUMP] Totally %u us is recorded.\n", total_time);

    n = 0;
    i = g_buf_full ? g_buf_idx*2 : 0;
    j = 0;
    current_time = (unsigned long long)g_last_time * 1000 - total_time;

    while(n++ < event_size)
    {
        int task_id = g_buf_ptr[i] & ~MODE_MASK;
        unsigned int diff_time = g_trace_buf[i+1];

        current_time += (unsigned long long)diff_time;

        if (task_id == 0) goto next; // ignore task ID 0

        snprintf(outbuf, (BUFSIZE+80-j), "#%llu\n", current_time / TIMESCALE);
        j += strlen(outbuf);
        outbuf = &buf[j];

        if ((g_buf_ptr[i] & MODE_MASK) == MODE_TASK_IN)
        {
            snprintf(outbuf, (BUFSIZE+80-j), "1%s\n", sigsymbol(task_id));
            j += strlen(outbuf);
        }
        else if ((g_buf_ptr[i] & MODE_MASK) == MODE_TASK_OUT)
        {
            snprintf(outbuf, (BUFSIZE+80-j), "0%s\n", sigsymbol(task_id));
            j += strlen(outbuf);
        }
        else if ((g_buf_ptr[i] & MODE_MASK) == MODE_TASK_DELAY)
        {
            snprintf(outbuf, (BUFSIZE+80-j), "x%s\n", sigsymbol(task_id));
            j += strlen(outbuf);
        }
        else if ((g_buf_ptr[i] & MODE_MASK) == MODE_TASK_DELETE)
        {
            snprintf(outbuf, (BUFSIZE+80-j), "x%s\n", sigsymbol(task_id));
            j += strlen(outbuf);
        }
        else if ((g_buf_ptr[i] & MODE_MASK) == MODE_TAG)
        {
            int tag_id = (g_buf_ptr[i] >> 24) & 0xf;
            int value  = g_buf_ptr[i] & ~(MODE_MASK | 0x0f000000);
            snprintf(outbuf, (BUFSIZE+80-j), "b%s %s\n", dec2bin(value), sigsymbol((g_max_id+1)+tag_id));
            j += strlen(outbuf);
        }
        #if MEMORY_BANDWIDTH_ANALYSIS == 1
        else if ((g_buf_ptr[i] & MODE_MASK) == MODE_MEM_USAGE)
        {
            snprintf(outbuf, (BUFSIZE+80-j), "b%s %s\n", dec2bin(g_buf_ptr[i] & ~MODE_MASK), sigsymbol((g_max_id+1)+(g_max_tag+1)));
            j += strlen(outbuf);
        }
        #endif // MEMORY_BANDWIDTH_ANALYSIS == 1

        if (j >= BUFSIZE)
        {
            fwrite(buf, 1, j, fp);
            outbuf = buf;
            j = 0;
            printf("."); fflush(stdout);
        }
        else
        {
            outbuf = &buf[j];
        }

next:
        i += 2;

        if (i >= g_buf_size * 2)
            i = 0;
    }

    if (j != 0)
    {
        fwrite(buf, 1, j, fp);
        printf("."); fflush(stdout);
        printf("\n");
    }

    //xTaskResumeAll();
    printf("[RTOS][DUMP] Dump complete %d event....\n", event_size);

    /* stop trace after dump the trace */
end:
    if (fp)
    {
        fclose(fp);
    }

    if (buf)
    {
        free(buf);
    }
#else
    printf("[RTOS][DUMP] Warnning: do not support vcd dump to file.\n");
#endif // VCD_DUMP_TO_FILE == 1

    itpTaskVcdClose();
}

static void
mem_trace(void)
{
#if MEMORY_BANDWIDTH_ANALYSIS == 1
    #if VCD_DUMP_RINGBUF == 0
    if (g_buf_idx >= g_buf_size)
    {
        g_dump   = 1;
        g_enable = 0;
    }
    else
    #endif
    {
        uint32_t servCount, cycle;

        ithMemStatServCounterDisable();

        servCount = ithMemStatGetAllServCount();
        cycle = ithMemStatGetServCount();

        //*g_last_ptr  = (int)((servCount * 8) / (cycle / (float)ithGetMemClock()) / 1000000) | MODE_MEM_USAGE;
        *g_last_ptr  = (int)((servCount * 100) / cycle) | MODE_MEM_USAGE;
        g_last_ptr   = g_buf_ptr;

        if (g_buf_idx >= g_buf_size) {
            g_buf_full = 1;
            g_buf_idx  = 0;
            g_buf_ptr  = g_trace_buf;
        } else {
            g_buf_idx++;
        }
        *g_buf_ptr++ = MODE_MEM_USAGE;
        *g_buf_ptr++ = getDiffTime();

        ithMemStatServCounterEnable();
    }
#endif // MEMORY_BANDWIDTH_ANALYSIS == 1
}

static int add_tasklist(int task_id)
{
    char* pch;
    int i;

    if (task_id == 0)
        task_id = ++g_max_id;

    snprintf(g_task_name[task_id], configMAX_TASK_NAME_LEN+5, "%04d_%s", task_id, pcTaskGetTaskName(xTaskGetCurrentTaskHandle()));
    pch = g_task_name[task_id];

    for(i=0; pch[i] != 0 && i < configMAX_TASK_NAME_LEN+5; i++)
    {
        if (pch[i] == ' ')
            pch[i] = '_';
    }

    if (g_max_id >= MAX_TASK_RECORD)
    {
        g_dump   = 1;
        g_enable = 0;
        g_max_id = MAX_TASK_RECORD-1;
        printf("[RTOS][DUMP] Out of task to record\n");
    }

    return task_id;
}

static void init(
    void)
{
    g_buf_idx    = 0;
    g_buf_full   = 0;
    g_buf_ptr    = g_trace_buf;
    g_enable     = 1;
    g_dump       = 0;

    getDiffTime();

    snprintf(g_task_name[0], configMAX_TASK_NAME_LEN+5, "%04d_untrack", 0);

#if MEMORY_BANDWIDTH_ANALYSIS == 1

    g_last_ptr   = g_buf_ptr;
    *g_buf_ptr++ = MODE_MEM_USAGE;
    *g_buf_ptr++ = getDiffTime();
    g_buf_idx++;

    ithMemStatSetServCountPeriod(0xFFFF);

    #ifdef __arm__
    ithMemStatSetServ0Request(ITH_MEMSTAT_ARM);
    #elif defined(__sm32__)
    ithMemStatSetServ0Request(ITH_MEMSTAT_RISC);
    #elif defined(__riscv)
    // TODO: RISCV
    #else
    #error "Unknown CPU type!"
    #endif // __arm__

    ithMemStatServCounterEnable();

#endif // MEMORY_BANDWIDTH_ANALYSIS == 1

    strncpy(RecorderData.header, "KTRACE", sizeof(RecorderData.header));
    RecorderData.endian = 0x12345678;
    RecorderData.tag1   = MAX_EVENT_RECORD;
    RecorderData.tag2   = configMAX_TASK_NAME_LEN+5;
    RecorderData.tag3   = MEMORY_BANDWIDTH_ANALYSIS;

#if VCD_DUMP_TO_FILE == 0
    g_buf_size          = MAX_EVENT_RECORD;
#endif
}

/***************************************************************************
 *                              Public Functions
 ***************************************************************************/
void portTASK_SWITCHED_IN(void)
{
    if (g_trace_buf && g_enable)
    {
        #if VCD_DUMP_RINGBUF == 0
        if (g_buf_idx >= g_buf_size)
        {
            g_dump   = 1;
            g_enable = 0;
        }
        else
        #endif
        {
            int task_id = uxTaskGetTaskNumber(xTaskGetCurrentTaskHandle());
            if (task_id == 0)
                task_id = add_tasklist(task_id);

            if (g_buf_idx >= g_buf_size) {
                g_buf_full = 1;
                g_buf_idx  = 0;
                g_buf_ptr  = g_trace_buf;
            } else {
                g_buf_idx++;
            }
            *g_buf_ptr++ = task_id | MODE_TASK_IN;
            *g_buf_ptr++ = getDiffTime();
        }

        mem_trace();
    }
}

void portTASK_SWITCHED_OUT(void)
{
    if (g_trace_buf && g_enable)
    {
        #if VCD_DUMP_RINGBUF == 0
        if (g_buf_idx >= g_buf_size)
        {
            g_dump   = 1;
            g_enable = 0;
        }
        else
        #endif
        {
            int task_id = uxTaskGetTaskNumber(xTaskGetCurrentTaskHandle());
            if (task_id == 0)
                task_id = add_tasklist(task_id);

            if (g_buf_idx >= g_buf_size) {
                g_buf_full = 1;
                g_buf_idx  = 0;
                g_buf_ptr  = g_trace_buf;
            } else {
                g_buf_idx++;
            }
            *g_buf_ptr++ = task_id | MODE_TASK_OUT;
            *g_buf_ptr++ = getDiffTime();
        }
    }
}

void portTASK_DELAY(void)
{
    if (g_trace_buf && g_enable)
    {
        #if VCD_DUMP_RINGBUF == 0
        if (g_buf_idx >= g_buf_size)
        {
            g_dump   = 1;
            g_enable = 0;
        }
        else
        #endif
        {
            int task_id = uxTaskGetTaskNumber(xTaskGetCurrentTaskHandle());
            if (task_id == 0)
                task_id = add_tasklist(task_id);

            if (g_buf_idx >= g_buf_size) {
                g_buf_full = 1;
                g_buf_idx  = 0;
                g_buf_ptr  = g_trace_buf;
            } else {
                g_buf_idx++;
            }
            *g_buf_ptr++ = task_id | MODE_TASK_DELAY;
            *g_buf_ptr++ = getDiffTime();
        }
    }
}

void portTASK_CREATE(TaskHandle_t xTask)
{
    unsigned portBASE_TYPE task_id = ++g_max_id;
    char* pch;
    int i;

    vTaskSetTaskNumber(xTask, task_id);

    snprintf(g_task_name[task_id], configMAX_TASK_NAME_LEN+5, "%04d_%s", task_id, pcTaskGetTaskName(xTask));
    pch = g_task_name[task_id];

    for(i=0; pch[i] != 0 && i < configMAX_TASK_NAME_LEN+5; i++)
    {
        if (pch[i] == ' ')
            pch[i] = '_';
    }

    if (g_max_id >= MAX_TASK_RECORD)
    {
        g_dump   = 1;
        g_enable = 0;
        g_max_id = MAX_TASK_RECORD-1;
        printf("[RTOS][DUMP] Out of task to record\n");
    }
}

void portTASK_DELETE(TaskHandle_t xTask)
{
    if (g_trace_buf && g_enable)
    {
        #if VCD_DUMP_RINGBUF == 0
        if (g_buf_idx >= g_buf_size)
        {
            g_dump   = 1;
            g_enable = 0;
        }
        else
        #endif
        {
            int task_id = uxTaskGetTaskNumber(xTask);
            if (g_buf_idx >= g_buf_size) {
                g_buf_full = 1;
                g_buf_idx  = 0;
                g_buf_ptr  = g_trace_buf;
            } else {
                g_buf_idx++;
            }
            *g_buf_ptr++ = task_id | MODE_TASK_DELETE;
            *g_buf_ptr++ = getDiffTime();
        }
    }
}

void itpTaskVcdInit(void)
{
    ithTimerCtrlEnable(VCD_TIMER, ITH_TIMER_UPCOUNT);
    ithTimerSetCounter(VCD_TIMER, 0);
    ithTimerEnable(VCD_TIMER);

#if VCD_DUMP_TO_FILE == 0
    init();
#endif
}

void itpTaskVcdOpen(const char* filename, int count)
{
#if VCD_DUMP_TO_FILE == 1
    g_buf_size   = count;
    g_fname      = filename;

    // Allocate Trace Buffer
    if (!g_trace_buf)
    {
        g_trace_buf = (unsigned int*)malloc(g_buf_size*sizeof(int)*2);
    }

    if (g_trace_buf == (unsigned int*)0)
    {
        printf("[RTOS][DUMP] Can not creat trace buffer\n");
    } else {
        printf("[RTOS][DUMP] Open VCD %s file.\n", filename);
    }

    init();
#else
    printf("[RTOS][DUMP] Warnning: do not support vcd dump to file.\n");
#endif
}

int itpTaskVcdGetEventCount(void)
{
    return (g_buf_full) ? g_buf_size : g_buf_idx;
}

void itpTaskVcdSetTag(int id, int tag)
{
    ithEnterCritical();

    if (id > 15)
    {
        printf("[RTOS][DUMP] Out of tag id number.\n");
        goto end;
    }

    if (tag & 0xff000000)
    {
        printf("[RTOS][DUMP] tag value can not exceed 24-bits.\n");
    }

    if(id > g_max_tag)
        g_max_tag = id;

    if (g_trace_buf && g_enable)
    {
        #if VCD_DUMP_RINGBUF == 0
        if (g_buf_idx >= g_buf_size)
        {
            g_dump   = 1;
            g_enable = 0;
        }
        else
        #endif
        {
            if (g_buf_idx >= g_buf_size) {
                g_buf_full = 1;
                g_buf_idx  = 0;
                g_buf_ptr  = g_trace_buf;
            } else {
                g_buf_idx++;
            }
            *g_buf_ptr++ = (tag & ~(MODE_MASK | 0x0f000000)) | MODE_TAG | (id << 24);
            *g_buf_ptr++ = getDiffTime();
        }
    }

end:
    ithExitCritical();
}

void itpTaskVcdWrite(void)
{
    vcd_dump();
}

void itpTaskVcdClose(void)
{
#if VCD_DUMP_TO_FILE == 1
    free((void*)g_trace_buf);
    g_trace_buf = (unsigned int*)0;
#endif
    g_buf_idx   = 0;
    g_enable    = 0;
    g_dump      = 0;
    g_max_tag   = 0;
    g_buf_ptr   = g_trace_buf;
}

#endif // CFG_DBG_TRACE_ANALYZER && CFG_DBG_VCD

