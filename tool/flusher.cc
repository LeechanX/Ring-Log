#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "rlog.h"

#define SHM_KEY_ID_SEQ 12356

void help()
{
    printf("./flusher -f exefilepath [-d]\n");
}

int main(int argc, char** argv)
{
    if (argc != 3 && argc != 4)
    {
        help();
        return -1;
    }
    char* exec_path = NULL;
    bool if_del = false;
    int i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "-f") == 0)
        {
            exec_path = realpath(argv[i + 1], NULL);
            i += 2;
        }
        else if (strcmp(argv[i], "-d") == 0)
        {
            if_del = true;
            i += 1;
        }
        else
        {
            help();
            return -1;
        }
    }
    if (!exec_path)
    {
        help();
        return -1;
    }

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
    int next_cf_shmid;
    FILE* fp = fopen("leaveLog.log", "w");
    do
    {
        cell_buffer* cf = (cell_buffer*)shmat(curr_cf_shmid, 0, 0);
        cf->_data = (char*)cf + sizeof(cell_buffer);
        if (cf->empty())
            break;
        cf->persist(fp);
        next_cf_shmid = cf->next_shmid;
        shmdt((void*)cf);
        shmctl(curr_cf_shmid, IPC_RMID, NULL);
        curr_cf_shmid = next_cf_shmid;
    }
    while (head_cf_shmid != curr_cf_shmid);
    fclose(fp);
    shmdt((void*)p_cf_shmid);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
