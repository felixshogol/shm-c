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

#define DFXP_MAX_BUFFERS 1

#define DFXP_SHM_MAX_IP_GTPS 100

#define THREAD_NUM_MAX 64
#define NETIF_PORT_MAX 4
#define PCI_LEN 12
#define SERVER_IPADDR_MAX 16
// gtp-u config
#define GTP_CFG_MAX_TUNNELS 10000
#define MAX_LOCAL_PORTS 1000

typedef enum
{
    DFXP_SHM_STATUS_IDLE = 0,
    DFXP_SHM_STATUS_WRITTEN_BY_SERVER,
    DFXP_SHM_STATUS_WRITTEN_BY_CLIENT,
    DFXP_SHM_STATUS_READ_BY_SERVER,
    DFXP_SHM_STATUS_READ_BY_CLIENT,

} dfxp_shm_status;

typedef enum
{
    DFXP_SHM_CMD_NONE = 0,
    DFXP_SHM_CMD_CONFIG_TRAFFIC,
    DFXP_SHM_CMD_CONFIG_PORTS,
    DFXP_SHM_CMD_START,
    DFXP_SHM_CMD_STOP,
    DFXP_SHM_CMD_SHUTDOWN,
    DFXP_SHM_CMD_ADD_IP_GTP,
    DFXP_SHM_CMD_DEL_IP_GTP,
    DFXP_SHM_CMD_GET_STATS,
    DFXP_SHM_CMD_CLEAR_CONFIG,

} dfxp_shm_cmd;

/*
 * The lower 32 bits represent an IPv6 address.
 * The IPv4 address is in the same position as the lower 32 bits of IPv6.
 * */
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

typedef struct dfxp_port_s
{
    char local_ip[INET6_ADDRSTRLEN];
    char gateway_ip[INET6_ADDRSTRLEN];
    char server_ip[INET6_ADDRSTRLEN];
    char pci[PCI_LEN + 1]; // pci string

} dfxp_port_t;

typedef struct dfxp_ports_s
{
    dfxp_port_t ports[NETIF_PORT_MAX];
    int port_num;

} dfxp_ports_t;

typedef struct dfxp_shm_tunnel_s
{
    uint32_t id;
    uint32_t teid_in;
    uint32_t teid_out;
    uint32_t ue_ipv4;
    uint32_t upf_ipv4;
} dfxp_shm_tunnel_t;

typedef struct dfxp_shm_ip_gtp_s
{
    char address[INET6_ADDRSTRLEN];
    dfxp_shm_tunnel_t tunnel;
} dfxp_shm_ip_gtp_t;

typedef struct dfxp_shm_ip_gtps_s
{
    dfxp_shm_ip_gtp_t ip_gtp[GTP_CFG_MAX_TUNNELS];
    int num;
} dfxp_shm_ip_gtps_t;

typedef struct dfxp_stats_s
{

} dfxp_stats_t;

typedef struct dfxp_traffic_config_s
{
    // required
    bool server;  // mode client | server, true - server
    int duration; // seconds. default 60s
    int cpu[THREAD_NUM_MAX];
    int cpu_num;
    int cps; // total connections per seconds

    int listen;     // default 80
    int listen_num; // default 1

    // not required
    int cc;                 /* current connections not required only client*/
    int keepalive_interval; // useconds
    int keepalive_num;      // number of requests

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

    int lport_min; //  default 1 65535
    int lport_max;
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
    dfxp_traffic_config_t cfgTraffic;
    dfxp_ports_t cfgPorts;
    dfxp_shm_ip_gtps_t cfgIpGtps;
    dfxp_stats_t stats;
} dfxp_shm_t;

int dfxp_shm_main(int argc, char **argv);
int dfxp_shm_thread_join(void);
pthread_t *dfxp_shm_get_threadid(void);

#endif