#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <pthread.h>

#include "src_lib/dfxp_shm_common.h"
#include "dfxp_shm_config.h"
#include "src_lib/dfxp_shm_client.h"

static void error(char *msg);

static dfxp_shm_t *shared_mem_ptr;
static sem_t *mutex_sem, *buffer_count_sem, *spool_signal_sem;
static int fd_shm;
static dfxp_shm_t shm;

dfxp_traffic_config_t g_config;

static void usage(void)
{
    printf("#######################\n");
    printf("start: start traffic   \n");
    printf("stop : stop  traffic   \n");
    printf("shutdown: shutdown dfxp\n");
    printf("config: config dfxp    \n");
    printf("#######################\n");
}

int main(int argc, char **argv)
{

    char *cmd = NULL;
    int ret;

    printf("DEBUG: %s:%d:%s start %d %s ...\n", __FILE__, __LINE__, __func__, argc, argv[0]);

    if (argc < 2)
    {
        printf("Wrong arguments[%d]\n", argc);
        usage();
        exit(1);
    }
    cmd = argv[1];

    if (strcmp(cmd, "stop") == 0)
    {
        shm.cmd = DFXP_SHM_CMD_STOP;
    }
    else if (strcmp(cmd, "start") == 0)
    {
        shm.cmd = DFXP_SHM_CMD_START;
    }
    else if (strcmp(cmd, "shutdown") == 0)
    {
        shm.cmd = DFXP_SHM_CMD_SHUTDOWN;
    }
    else if (strcmp(cmd, "config") == 0)
    {
        shm.cmd = DFXP_SHM_CMD_CONFIG_TRAFFIC;
        if (argc != 4)
        {
            printf("DFPX_SHM_CMD_CONFIG. wrong argv arguments %d\n", argc);
        }

        printf("config shm. [%s:%s]\n", argv[2], argv[3]);
        if (config_parse(argc, argv, &g_config) < 0)
        {
            return -1;
        }
    }
    else if (strcmp(cmd, "stats") == 0)
    {
        shm.cmd = DFXP_SHM_CMD_GET_STATS;
    }
    else
    {
        printf("Wrong cmd[%s]\n", cmd);
        usage();
        exit(1);
    }

    printf("Send command (%s)\n", ShmGetCmdName(shm.cmd));
    shm.status = DFXP_SHM_STATUS_WRITTEN_BY_CLIENT;

    ret = ShmInit(SHARED_MEM_NAME, O_RDWR, 0);
    if (ret != 0)
    {
        printf("ERR: %s:%d:%s -> shm init failed\n", __FILE__, __LINE__, __func__);
        exit(-1);
    }
    ret = ShmWrite(&shm);
    if (ret != 0)
    {
        printf("ERR: %s:%d:%s -> shm write failed\n", __FILE__, __LINE__, __func__);
        exit(-1);
    }

    
}

// Print system error and exit
void error(char *msg)
{
    perror(msg);
    exit(1);
}
