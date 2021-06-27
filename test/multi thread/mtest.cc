#include "rlog.h"
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

int64_t get_current_millis(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void* thdo(void* args)
{
    for (int i = 0;i < 1e7; ++i)
    {
        LOG_ERROR("my number is number my number is my number is my number is my number is my number is my number is %d", i);
    }
}

int main(int argc, char** argv)
{
    int nThreads = 20;
    LOG_INIT("log", "myname", 3);
    uint64_t start_ts = get_current_millis();
    pthread_t tids[nThreads];
    for (int i = 0;i < nThreads; ++i)
    	pthread_create(&tids[i], NULL, thdo, NULL);

    for (int i = 0;i < nThreads; ++i)
	pthread_join(tids[i], NULL);
    uint64_t end_ts = get_current_millis();
    printf("time use %lums\n", end_ts - start_ts);
}
