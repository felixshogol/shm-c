#ifndef DFXP_SHM_SERVER_H
#define DFXP_SHM_SERVER_H

#include <stdint.h>
#include <stdbool.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>

#define SEM_MUTEX_NAME "/sem-mutex-dfxp-shm"
#define SEM_BUFFER_COUNT_NAME "/sem-count-dfxp-shm"
#define SEM_SPOOL_SIGNAL_NAME "/sem-spool-dfxp-shm"
#define SHARED_MEM_NAME "/dfxp-shm"


typedef enum
{
    DFXP_SHM_STATUS_IDLE = 0,
    DFXP_SHM_STATUS_WRITTEN,
    DFXP_SHM_STATUS_READ,

} dfxp_shm_status;

typedef enum
{
    DFXP_SHM_CMD_NONE = 0,
    DFXP_SHM_CMD_CONFIG,
    DFXP_SHM_CMD_START,
    DFXP_SHM_CMD_STOP,
    DFXP_SHM_CMD_SHUTDOWN,
    DFXP_SHM_CMD_ADD_IP_GTP,
    DFXP_SHM_CMD_DEL_IP_GTP,
    DFXP_SHM_CMD_GET_STATS,

} dfxp_shm_cmd;

#define DFXP_THREAD_NUM_MAX 64
#define DFXP_NETIF_PORT_MAX 4
#define DFXP_PCI_LEN 12
#define DFXP_SHM_MAX_IP_GTPS 100
#define SERVER_IPADDR_MAX   16

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

typedef struct server_ipaddr_range_s
{
    ipaddr_t addresses[SERVER_IPADDR_MAX];
    int num;
} server_ipaddr_range_t;

typedef struct dfxp_port_s
{
    server_ipaddr_range_t server_ipaddr_range;
    ipaddr_t gateway_ip;
    char pci[DFXP_PCI_LEN + 1]; // pci string
    int lport_min;          //  default 1 65535
    int lport_max;

} dfxp_port_t;

typedef struct dfxp_ports_s
{
    dfxp_port_t ports[DFXP_NETIF_PORT_MAX];
    int port_num;

} dfxp_ports_t;

typedef struct dfxp_shm_tunnel_s
{
    uint8_t id;
    uint32_t teid_in;
    uint32_t teid_out;
    uint32_t ue_ipv4;
    uint32_t upf_ipv4;
} dfxp_shm_tunnel_t;

typedef struct dfxp_shm_ip_gtp_s
{
    ipaddr_t address;
    dfxp_shm_tunnel_t tunnel;
} dfxp_shm_ip_gtp_t;

typedef struct dfxp_shm_ip_gtps_s
{
    dfxp_shm_ip_gtp_t ip_gtp[DFXP_SHM_MAX_IP_GTPS];
    int num;
} dfxp_shm_ip_gtps_t;

typedef struct dfxp_stats_s
{

} dfxp_stats_t;

typedef struct dfxp_traffic_config_s
{
    // required
    bool server;  // mode client | server
    int duration; // default 60s
    int cpu[DFXP_THREAD_NUM_MAX];
    int cpu_num;
    int cps; // total connections per seconds

    int listen;     // default 80
    int listen_num; // default 1

    // not required
    int cc; /* current connections not required only client*/
    bool keepalive;
    uint32_t launch_num; // connections are initiated by the client at a time. default = 4 only clinet
    bool payload_random; // not required
    int payload_size;    // not required  max 1514 default 0
    int packet_size;     // not required (0-1514)
    bool jumbo;
    uint8_t protocol; /* TCP/UDP not required  default TCP*/
    uint8_t tx_burst; // Number (1-1024)  default 8
    int wait;         // client waits seconds after startup before entering the slow-start phase. default 3 seconds
    int slow_start;   // only client in seconds  default 30s
    uint8_t tos;      // not required default 0
    bool tcp_rst;     // Set whether replies rst to SYN packets requesting unopened TCP ports. dafault true

    bool gtpu_enable;

    bool ipv6;
    bool quiet; // Turn off output statistics per second
    bool http;
    bool stats_http; // payload size >= HTTP_DATA_MIN_SIZE
    uint8_t pipeline;
    int ticks_per_sec; // default   (10 * 1000) accorsing to keepalive

} dfxp_traffic_config_t;

typedef struct dfxp_shm_s
{
    dfxp_shm_cmd cmd;
    dfxp_shm_status status;
    union
    {
        dfxp_traffic_config_t cfg;
        //  Set the port ranges that the server listens to,
        dfxp_ports_t ports;
        dfxp_shm_ip_gtps_t ip_gtps;
        dfxp_stats_t stats;
    };
} dfxp_shm_t;

int dfxp_shm_main(int argc, char **argv);
int dfxp_shm_thread_join(void);
pthread_t *dfxp_shm_get_threadid(void);

#endif