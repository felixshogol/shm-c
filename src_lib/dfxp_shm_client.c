#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>

#include "dfxp_shm_common.h"
#include "dfxp_shm_client.h"

static int error(const char *msg);

static dfxp_shm_t *shared_mem_ptr;
static sem_t *mutex_sem, *buffer_count_sem, *spool_signal_sem;
static int fd_shm;

static void dump_shm_ports(char *title, dfxp_ports_t *ports);

const char *shm_cmd_name[] = {

    "DFXP_SHM_CMD_NONE",
    "DFXP_SHM_CMD_CONFIG_TRAFFIC",
    "DFXP_SHM_CMD_CONFIG_PORTS",
    "DFXP_SHM_CMD_START",
    "DFXP_SHM_CMD_STOP",
    "DFXP_SHM_CMD_SHUTDOWN",
    "DFXP_SHM_CMD_ADD_IP_GTP",
    "DFXP_SHM_CMD_DEL_IP_GTP",
    "DFXP_SHM_CMD_GET_STATS"};

int ShmInit(const char *name, int oflag, int mode)
{

    printf("DEBUG: %s:%d:%s \n", __FILE__, __LINE__, __func__);

    // Get shared memory
    if ((fd_shm = shm_open(SHARED_MEM_NAME, O_RDWR, 0)) == -1)
        error("shm_open");

    if ((shared_mem_ptr = (dfxp_shm_t *)mmap(NULL, sizeof(dfxp_shm_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                             fd_shm, 0)) == MAP_FAILED)
        error("mmap");

    //  mutual exclusion semaphore, mutex_sem
    if ((mutex_sem = sem_open(SEM_MUTEX_NAME, 0, 0, 0)) == SEM_FAILED)
        error("sem_open");

    // counting semaphore, indicating the number of available buffers.
    if ((buffer_count_sem = sem_open(SEM_BUFFER_COUNT_NAME, 0, 0, 0)) == SEM_FAILED)
        error("sem_open");

    // counting semaphore, indicating the number of strings to be printed. Initial value = 0
    if ((spool_signal_sem = sem_open(SEM_SPOOL_SIGNAL_NAME, 0, 0, 0)) == SEM_FAILED)
        error("sem_open");

    return 0;
}

int ShmSizeofCfg()
{
    return sizeof(dfxp_shm_t);
}

int ShmSizeofTraffic()
{
    return sizeof(dfxp_traffic_config_t);
}

int ShmSizeofPorts()
{
    return sizeof(dfxp_ports_t);
}

int ShmSizeofIpGtps()
{
    return sizeof(dfxp_shm_ip_gtps_t);
}

int ShmWrite(dfxp_shm_t *shm)
{
    printf("DEBUG: %s:%d:%s \n", __FILE__, __LINE__, __func__);

    // get a buffer: P (buffer_count_sem);
    if (sem_wait(buffer_count_sem) == -1)
        error("sem_wait: buffer_count_sem");

    printf("DEBUG: %s:%d:%s -> wait mutex_sem\n", __FILE__, __LINE__, __func__);

    /* There might be multiple producers. Only one producer uses buffer_index at a time.  */
    if (sem_wait(mutex_sem) == -1)
        error("mutex_sem");

    printf("DEBUG: %s:%d:%s -> writing shn\n", __FILE__, __LINE__, __func__);

    memcpy(shared_mem_ptr, shm, sizeof(dfxp_shm_t));

    printf("DEBUG: %s:%d:%s -> writing cmd %d\n", __FILE__, __LINE__, __func__, shared_mem_ptr->cmd);
    if (shared_mem_ptr->cmd == DFXP_SHM_CMD_CONFIG_PORTS)
    {

        dump_shm_ports("shm-c", &shared_mem_ptr->cfgPorts);
    }

    sleep(1);

    // Release mutex sem: V (mutex_sem)
    if (sem_post(mutex_sem) == -1)
        error("sem_post: mutex_sem");

    // Tell spooler that there is a string to print: V (spool_signal_sem);
    if (sem_post(spool_signal_sem) == -1)
        error("sem_post: spool_signal_sem");

    printf("DEBUG: %s:%d:%s -> exit ...\n", __FILE__, __LINE__, __func__);

    return 0;
}

const char *ShmGetCmdName(dfxp_shm_cmd cmd)
{
    switch (cmd)
    {
    case DFXP_SHM_CMD_START:
        return shm_cmd_name[DFXP_SHM_CMD_START];
        break;
    case DFXP_SHM_CMD_STOP:
        return shm_cmd_name[DFXP_SHM_CMD_STOP];
        break;
    case DFXP_SHM_CMD_CONFIG_TRAFFIC:
        return shm_cmd_name[DFXP_SHM_CMD_CONFIG_TRAFFIC];
        break;
    case DFXP_SHM_CMD_CONFIG_PORTS:
        return shm_cmd_name[DFXP_SHM_CMD_CONFIG_PORTS];
        break;
    case DFXP_SHM_CMD_GET_STATS:
        return shm_cmd_name[DFXP_SHM_CMD_GET_STATS];
        break;
    case DFXP_SHM_CMD_SHUTDOWN:
        return shm_cmd_name[DFXP_SHM_CMD_SHUTDOWN];
        break;
    case DFXP_SHM_CMD_DEL_IP_GTP:
        return shm_cmd_name[DFXP_SHM_CMD_DEL_IP_GTP];
        break;
    case DFXP_SHM_CMD_ADD_IP_GTP:
        return shm_cmd_name[DFXP_SHM_CMD_ADD_IP_GTP];
        break;
    default:
        return shm_cmd_name[DFXP_SHM_CMD_NONE];
    }
}

// Print system error and exit
int error(const char *msg)
{
    perror(msg);
    return -1;
}

static void dump_shm_ports(char *title, dfxp_ports_t *ports)
{
    printf("##### Config SHM ports dump %s\n", title);
    printf("port_num:           %d\n", ports->port_num);
    printf("local_ip:           %s\n", ports->ports[0].local_ip);
    printf("gateway_ip          %s\n", ports->ports[0].gateway_ip);
    printf("server_ip           %s\n", ports->ports[0].server_ip);
}