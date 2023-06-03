#ifndef DFXP_SHM_CONFIG_H
#define DFXP_SHM_CONFIG_H


struct config_keyword {
    const char *name;
    int (*handler)(int argc, char *argv[], void *data);
    const char *help;  
};

#define CONFIG_ARG_NUM_MAX    32
#define CONFIG_LINE_MAX       2048
#define TX_BURST_MAX        1024
#define TX_BURST_DEFAULT    8
#define SLOW_START_MIN 10
#define SLOW_START_MAX 600


int config_parse(int argc, char **argv, dfxp_shm_config_t *cfg);
const char * get_shm_cmd_name (dfpx_shm_cmd cmd);

#endif