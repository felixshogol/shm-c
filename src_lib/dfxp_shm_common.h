#ifndef DFXP_SHM_COMMON_H
#define DFXP_SHM_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>


#define SEM_MUTEX_NAME "/sem-mutex-dfxp-shm"
#define SEM_BUFFER_COUNT_NAME "/sem-count-dfxp-shm"
#define SEM_SPOOL_SIGNAL_NAME "/sem-spool-dfxp-shm"
#define SHARED_MEM_NAME "/dfxp-shm"
#define MAX_BUFFERS 1

typedef enum
{
    DFPX_SHM_STATUS_IDLE = 0,
    DFPX_SHM_STATUS_WRITTEN,
    DFPX_SHM_STATUS_READ,

} dfpx_shm_status;

typedef enum
{

    DFPX_SHM_CMD_NONE = 0,
    DFPX_SHM_CMD_CONFIG,
    DFPX_SHM_CMD_START,
    DFPX_SHM_CMD_STOP,
    DFPX_SHM_CMD_SHUTDOWN,
    DFPX_SHM_CMD_ADD_IP_GTP,
    DFPX_SHM_CMD_DEL_IP_GTP,
    DFPX_SHM_CMD_GET_STATS,
} dfpx_shm_cmd;

#define DFXP_THREAD_NUM_MAX 64
#define DFXP_NETIF_PORT_MAX 4
#define DFXP_PCI_LEN 12
#define DFXP_SHM_MAX_IP_GTPS 100


typedef struct
{
    union
    {
        struct in6_addr in6;
        struct
        {
            uint32_t pad[3];
            uint32_t ip;
        };
    };
} ipaddr_t;

typedef struct dfxp_shm_config_s
{
    // required
    bool server;    // default false (client)
    int duration; // default 60s
    int cpu[DFXP_THREAD_NUM_MAX];
    int cpu_num;
    int cps; // total connections per seconds
    //dfxp_port_s ports[DFXP_NETIF_PORT_MAX];
    int port_num;

    // Set the port ranges that the server listens to,
    ipaddr_t server_address;
    int listen;     // default 80
    int listen_num; // default 1

    // not required
    int cc; // current connections only client
    bool keepalive;
    uint32_t launch_num; // default = 4 , only clinet

    bool payload_random;
    int payload_size;
    int packet_size;
    bool jumbo;
    uint8_t protocol; // TCP/UDP default TCP
    uint8_t tx_burst; // Number (1-1024)  default 8
    int wait;         // client waits seconds after startup before entering the slow-start phase. default 3 seconds
    int slow_start;   // only client in seconds  default 30s
    uint8_t tos;      // default 0
    bool tcp_rst; // Set whether dfxp replies rst to SYN packets requesting unopened TCP ports. dafault true

    int lport_min; //  default 1 65535
    int lport_max;
    bool gtpu_enable; // default disable


    bool ipv6;
    bool quiet;
    bool http;
    bool stats_http; /* payload size >= HTTP_DATA_MIN_SIZE */
    uint8_t pipeline;
    int ticks_per_sec; // default   (10 * 1000) accorsing to keepalive
} dfxp_shm_config_t;

typedef struct dfxp_shm_tunnel_s
{
    uint8_t id;
    uint32_t teid_in;
    uint32_t teid_out;
    uint32_t nb_ipv4;
    uint32_t upf_ipv4;
} dfxp_shm_tunnel_t;

typedef struct dfxp_shm_s
{
    dfpx_shm_cmd cmd;
    dfpx_shm_status status;
    union
    {
        dfxp_shm_config_t cfg;
        dfxp_shm_tunnel_t tunnel;
    };
} dfxp_shm_t;

#endif