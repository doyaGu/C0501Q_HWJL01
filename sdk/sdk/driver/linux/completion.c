
#include <linux/completion.h>
#include <linux/spinlock.h>

/**
 * complete: - signals a single thread waiting on this completion
 * @x:  holds the state of this particular completion
 *
 * This will wake up a single thread waiting on this completion. Threads will be
 * awakened in the same order in which they were queued.
 *
 * It may be assumed that this function implies a write memory barrier before
 * changing the task state if and only if any tasks are woken up.
 */
void complete(struct completion *x)
{
    ithEnterCritical();
	x->done++;
    ithExitCritical();
	sem_post(&x->wait);
}

/**
 * wait_for_completion_timeout: - waits for completion of a task (w/timeout)
 * @x:  holds the state of this particular completion
 * @timeout:  timeout value in jiffies
 *
 * This waits for either a completion of a specific task to be signaled or for a
 * specified timeout to expire. The timeout is in jiffies. It is not
 * interruptible.
 *
 * The return value is 0 if timed out, and positive (at least 1, or number of
 * jiffies left till timeout) if completed.
 */
unsigned long
wait_for_completion_timeout(struct completion *x, unsigned long timeout)
{
    unsigned long ret;

    /* itpSemWaitTimeout() return -1 when timeout */
    ret = itpSemWaitTimeout(&x->wait, timeout);
    return ret ? 0 : 1;
}

