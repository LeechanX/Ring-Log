#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "rlog.h"

#define SHM_KEY_ID_SEQ 12356

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./log_flusher exefilepath\n");
        return -1;
    }
    char* exec_path = realpath(argv[1], NULL);
    key_t shm_key = ftok(exec_path, SHM_KEY_ID_SEQ);
    if (shm_key < 0)
    {
        perror("ftok");
        return -1;
    }
    int shmid = shmget(shm_key, sizeof(int*), 0666);
    if (shmid == -1)
    {
        perror("shmget");
        return -1;
    }
    //clear old cell_buffer shm here
    int* p_cf_shmid = (int*)shmat(shmid, 0, 0);
    int head_cf_shmid = *p_cf_shmid;
    int curr_cf_shmid = head_cf_shmid;
    FILE* fp = fopen("leaveLog.log", "w");
    do
    {
        printf("shmid %d\n", curr_cf_shmid);
        cell_buffer* cf = (cell_buffer*)shmat(curr_cf_shmid, 0, 0);
        cf->_data = (char*)cf + sizeof(cell_buffer);
        if (cf->empty())
            break;
        cf->persist(fp);
        curr_cf_shmid = cf->next_shmid;
    }
    while (head_cf_shmid != curr_cf_shmid);
    fclose(fp);
    return 0;
}
