#include <ctype.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src_lib/dfxp_shm_common.h"
#include "dfxp_shm_config.h"

static void config_help(void);
static int config_keyword_parse(const char *file_path, const struct config_keyword *keywords, void *data);
static void config_keyword_help(const struct config_keyword *keywords);

static int config_parse_mode(int argc, char *argv[], void *data);
static int config_parse_protocol(int argc, char *argv[], void *data);
static int config_parse_gtpu_enable(int argc, char *argv[], void *data);
static int config_parse_duration(int argc, char *argv[], void *data);
static int config_parse_cps(int argc, char *argv[], void *data);
static int config_parse_cc(int argc, char *argv[], void *data);
static int config_parse_launch_num(int argc, char *argv[], void *data);
static int config_parse_tx_burst(int argc, char *argv[], void *data);
static int config_parse_slow_start(int argc, char *argv[], void *data);
static int config_parse_wait(int argc, char *argv[], void *data);


const char *config_shm_cmd_name[] = {

    "DFPX_SHM_CMD_NONE",
    "DFPX_SHM_CMD_CONFIG",
    "DFPX_SHM_CMD_START",
    "DFPX_SHM_CMD_STOP",
    "DFPX_SHM_CMD_SHUTDOWN",
    "DFPX_SHM_CMD_ADD_IP_GTP",
    "DFPX_SHM_CMD_DEL_IP_GTP",
    "DFPX_SHM_CMD_GET_STATS",
};

static struct config_keyword g_config_keywords[] = {
    // {"keepalive", config_parse_keepalive, "Interval(Timeout) [Number[0-" DEFAULT_STR(KEEPALIVE_REQ_NUM) "]], "
    //                                                                                                     "eg 1ms/10us/1s"},
    // {"pipeline", config_parse_pipeline, "Number[" DEFAULT_STR(PIPELINE_MIN) "-" DEFAULT_STR(PIPELINE_MAX) "], default " DEFAULT_STR(PIPELINE_DEFAULT)},
    {"server", config_parse_mode, "client/server"},
    // {"cpu", config_parse_cpu, "n0 n1 n2-n3..., eg 0-4 7 8 9 10"},
    // //{"socket_mem", config_parse_socket_mem, "n0,n1,n2..."},
    // {"port", config_parse_port, "PCI/bondMode:Policy(PCI0,PCI1,...) IPAddress Gateway [Gateway-Mac], eg 0000:13:00.0 192.168.1.3 192.168.1.1"},
    // {"duration", config_parse_duration, "Time, eg 1.5d, 2h, 3.5m, 100s, 100"},
    // {"cps", config_parse_cps, "Number, eg 1m, 1.5m, 2k, 100"},
    // {"cc", config_parse_cc, "Number, eg 100m, 1.5m, 2k, 100"},
    // {"launch_num", config_parse_launch_num, "Number, default " DEFAULT_STR(DEFAULT_LAUNCH)},
    // {"client", config_parse_client, "IPAddress Number"},
    // {"server", config_parse_server, "IPAddress Number"},
    // {"listen", config_parse_listen, "Port Number, default 80 1"},
    // {"payload_random", config_parse_payload_random, ""},
    // {"payload_size", config_parse_payload_size, "Number"},
    // {"packet_size", config_parse_packet_size, "Number"},
    // {"mss", config_parse_mss, "Number, default 1460"},
    {"protocol", config_parse_protocol, "http/tcp/udp, default tcp"},
    // {"tx_burst", config_parse_tx_burst, "Number[1-1024]"},
    // {"slow_start", config_parse_slow_start,
    //  "Number[" DEFAULT_STR(SLOW_START_MIN) "-" DEFAULT_STR(SLOW_START_MAX) "],"
    //                                                                        " default " DEFAULT_STR(SLOW_START_DEFAULT)},
    // {"wait", config_parse_wait, "Number, default " DEFAULT_STR(WAIT_DEFAULT)},
    // {"tos", config_parse_tos, "Number[0x00-0xff], default 0, eg 0x01 or 1"},
    // {"jumbo", config_parse_jumbo, ""},
    // {"quiet", config_parse_quiet, ""},
    // {"tcp_rst", config_parse_tcp_rst, "Number[0-1], default 1"},
    // {"http_host", config_parse_http_host, "String, default " HTTP_HOST_DEFAULT},
    // {"http_path", config_parse_http_path, "String, default " HTTP_PATH_DEFAULT},
    // {"lport_range", config_parse_lport_range, "Number [Number], default 1 65535"},
    {"gtpu_enable", config_parse_gtpu_enable, "enable/disable, default disable"},
    //{"gtpu_teid", config_parse_gtpu_teid, "teid_in teid_out"},

    {NULL, NULL, NULL}};

static struct option g_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"conf", required_argument, NULL, 'c'},
    {NULL, 0, NULL, 0}};

const char * get_shm_cmd_name (dfxp_shm_cmd cmd)
{
    switch (cmd) {
        case DFXP_SHM_CMD_START:
            return config_shm_cmd_name[DFXP_SHM_CMD_START];
        break;
        case DFXP_SHM_CMD_STOP:
            return config_shm_cmd_name[DFXP_SHM_CMD_STOP];
        break;
        case DFXP_SHM_CMD_CONFIG:
            return config_shm_cmd_name[DFXP_SHM_CMD_CONFIG];
        break;
        case DFXP_SHM_CMD_GET_STATS:
            return config_shm_cmd_name[DFXP_SHM_CMD_GET_STATS];
        break;
        case DFXP_SHM_CMD_SHUTDOWN:
            return config_shm_cmd_name[DFXP_SHM_CMD_SHUTDOWN];
        break;
        case DFXP_SHM_CMD_DEL_IP_GTP:
            return config_shm_cmd_name[DFXP_SHM_CMD_DEL_IP_GTP];
        break;
        case DFXP_SHM_CMD_ADD_IP_GTP:
            return config_shm_cmd_name[DFXP_SHM_CMD_ADD_IP_GTP];
        break;
        default:
            return config_shm_cmd_name[DFXP_SHM_CMD_NONE];
    }
}

int config_parse(int argc, char **argv, dfxp_traffic_config_t *cfg)
{

    int opt = 0;

    const char *optstr = "hvtmc:";
    if (argc == 1)
    {
        config_help();
        return -1;
    }

    while ((opt = getopt_long_only(argc, argv, optstr, g_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'c':
            if (config_keyword_parse(optarg, g_config_keywords, cfg) < 0)
            {
                return -1;
            }
            break;
        case 'h':
            config_help();
            return -1;
            break;
        default:
            return -1;
        }
    }

    return 0;
}

static void config_help(void)
{
    printf("-h --help\n");
    printf("-c --conf file  Run with conf file\n");
}

static const struct config_keyword *config_keyword_lookup(const struct config_keyword *keywords, const char *name)
{
    int i = 0;

    while (keywords[i].name != NULL)
    {
        if (strcmp(keywords[i].name, name) == 0)
        {
            return &keywords[i];
        }
        i++;
    }

    return NULL;
}

static char *config_skip_space(char *str)
{
    char *s = str;

    while ((s != NULL) && (*s != 0))
    {
        if (isspace(*s))
        {
            s++;
            continue;
        }
        else
        {
            return s;
        }
    }

    return NULL;
}

static char *config_next_space(char *str)
{
    char *s = str;

    while ((s != NULL) && (*s != 0))
    {
        if (isspace(*s))
        {
            break;
        }
        s++;
    }

    return s;
}

static int config_get_string(char *in, char **out, char **next)
{
    char *start = NULL;
    char *end = NULL;

    start = config_skip_space(in);
    if (start == NULL)
    {
        return 0;
    }

    end = config_next_space(start);
    if (start == end)
    {
        return 0;
    }

    *out = start;
    if (*end != 0)
    {
        *end = 0;
        end++;
    }

    *next = end;
    return end - start;
}

static int config_keyword_check_input(const char *line)
{
    const char *p = line;

    while (p && *p)
    {
        if (!(isascii(*p) && ((isprint(*p) || isspace(*p)))))
        {
            return -1;
        }
        p++;
    }

    return 0;
}

static int config_keyword_parse_line(char *line, char **argv, int argv_size)
{
    int argc = 0;
    char *str = NULL;
    char *arg = NULL;

    if (config_keyword_check_input(line) < 0)
    {
        return -1;
    }

    str = line;
    for (argc = 0; argc < argv_size; argc++)
    {
        if (config_get_string(str, &arg, &str) == 0)
        {
            break;
        }

        argv[argc] = arg;
        if ((argc == 0) && (*arg == '#'))
        {
            return 0;
        }
    }

    return argc;
}

static int config_keyword_call(const struct config_keyword *keywords, int argc, char **argv, void *data, int line_num)
{
    const struct config_keyword *keyword = NULL;

    keyword = config_keyword_lookup(keywords, argv[0]);
    if (keyword == NULL)
    {
        printf("line %d: unknown config keyword(\"%s\")\n", line_num, argv[0]);
        return -1;
    }

    if (keyword->handler)
    {
        if (keyword->handler(argc, argv, data) != 0)
        {
            printf("line %d: error\n", line_num);
            return -1;
        }
    }

    return 0;
}

static int config_keyword_parse(const char *file_path, const struct config_keyword *keywords, void *data)
{
    int ret = 0;
    int argc = 0;
    int line_num = 0;
    FILE *fp = NULL;
    char *argv[CONFIG_ARG_NUM_MAX];
    char config_line[CONFIG_LINE_MAX];

    fp = fopen(file_path, "r");
    if (fp == NULL)
    {
        printf("config file open error: %s\n", file_path);
        return -1;
    }

    while (!feof(fp))
    {
        line_num++;
        if (fgets(config_line, CONFIG_LINE_MAX - 1, fp) == NULL)
        {
            break;
        }

        argc = config_keyword_parse_line(config_line, argv, CONFIG_ARG_NUM_MAX);
        if (argc == 0)
        {
            continue;
        }
        else if (argc == -1)
        {
            ret = -1;
            break;
        }

        ret = config_keyword_call(keywords, argc, argv, data, line_num);
        if (ret < 0)
        {
            break;
        }
    }

    fclose(fp);
    printf("line:%d ret:%d\n", line_num, ret);

    return ret;
}

static void config_keyword_help(const struct config_keyword *keywords)
{
    const struct config_keyword *keyword = keywords;

    while (keyword && (keyword->name != NULL))
    {
        if (keyword->help)
        {
            printf("%s %s\n", keyword->name, keyword->help);
        }
        else
        {
            printf("%s\n", keyword->name);
        }
        keyword++;
    }
}

static char *config_str_find_nondigit(char *s, bool float_enable)
{
    char *p = s;
    int point = 0;

    while (*p)
    {
        if ((*p >= '0') && (*p <= '9'))
        {
            p++;
            continue;
        }
        else if (float_enable && (*p == '.'))
        {
            p++;
            point++;
            if (point > 1)
            {
                return NULL;
            }
        }
        else
        {
            return p;
        }
    }

    return NULL;
}



static int config_parse_number(char *str, bool float_enable, bool rate_enable)
{
    char *p = NULL;
    int rate = 1;
    int val = 0;

    p = config_str_find_nondigit(str, float_enable);
    if (p != NULL)
    {
        if (rate_enable == false)
        {
            return -1;
        }

        if (strlen(p) != 1)
        {
            return -1;
        }

        if ((*p == 'k') || (*p == 'K'))
        {
            rate = 1000;
        }
        else if ((*p == 'm') || (*p == 'M'))
        {
            rate = 1000000;
        }
        else
        {
            return -1;
        }
    }

    if (p == str)
    {
        return -1;
    }

    if (float_enable)
    {
        val = atof(str) * rate;
    }
    else
    {
        val = atoi(str) * rate;
    }

    if (val < 0)
    {
        return -1;
    }

    return val;
}


static int config_parse_mode(int argc, char *argv[], void *data)
{
    dfxp_traffic_config_t *cfg = data;

    if (argc != 2)
    {
        return -1;
    }

    if (strcmp("client", argv[1]) == 0)
    {
        cfg->server = 0;
    }
    else if (strcmp("server", argv[1]) == 0)
    {
        cfg->server = 1;
    }
    else
    {
        printf("unknown server %s\n", argv[1]);
        return -1;
    }

    return 0;
}

static int config_parse_protocol(int argc, char *argv[], void *data)
{
    dfxp_traffic_config_t *cfg = data;

    if (argc != 2)
    {
        return -1;
    }

    if (strcmp(argv[1], "tcp") == 0)
    {
        cfg->protocol = IPPROTO_TCP;
        return 0;
    }
    else if (strcmp(argv[1], "udp") == 0)
    {
        cfg->protocol = IPPROTO_UDP;
        return 0;
#ifdef HTTP_PARSE
    }
    else if (strcmp(argv[1], "http") == 0)
    {
        cfg->protocol = IPPROTO_TCP;
        cfg->http = true;
        return 0;
#endif
    }
    else
    {
        return -1;
    }
}

static int config_parse_gtpu_enable(int argc, char *argv[], void *data)
{
    dfxp_traffic_config_t *cfg = data;

    if (argc != 2)
    {
        return -1;
    }

    if (strcmp(argv[1], "true") == 0)
    {
        cfg->gtpu_enable = true;
        return 0;
    }
    else if (strcmp(argv[1], "false") == 0)
    {
        cfg->gtpu_enable = false;
        return 0;
    }
    else
    {
        return -1;
    }

    return 0;
}

static int config_parse_duration(int argc, char *argv[], void *data)
{
    int c = 0;
    int len = 0;
    int rate = 1;
    int duration = 0;
    double val = 0.0;
    dfxp_traffic_config_t *cfg = data;

    if (argc != 2)
    {
        return -1;
    }

    len = strlen(argv[1]);
    c = argv[1][len - 1];
    if (c == 'm')
    {
        rate = 60;
    }
    else if (c == 'h')
    {
        rate = 60 * 60;
    }
    else if (c == 'd')
    {
        rate = 60 * 60 * 24;
    }

    val = atof(argv[1]);
    if (val < 0)
    {
        return -1;
    }
    duration = val * rate;
    if (duration <= 0)
    {
        return -1;
    }

    cfg->duration = duration;
    return 0;
}

static int config_parse_cps(int argc, char *argv[], void *data)
{
    int cps = 0;
    dfxp_traffic_config_t *cfg = data;

    if (argc != 2)
    {
        return -1;
    }

    cps = config_parse_number(argv[1], true, true);
    if (cps < 0)
    {
        return -1;
    }

    cfg->cps = cps;
    return 0;
}

static int config_parse_cc(int argc, char *argv[], void *data)
{
    int cc = 0;
    dfxp_traffic_config_t *cfg = data;

    if (argc != 2)
    {
        return -1;
    }

    cc = config_parse_number(argv[1], true, true);
    if (cc <= 0)
    {
        return -1;
    }

    cfg->cc = cc;
    return 0;
}

static int config_parse_launch_num(int argc, char *argv[], void *data)
{
    int val = 0;
    dfxp_traffic_config_t *cfg = data;

    if (argc != 2)
    {
        return -1;
    }

    val = config_parse_number(argv[1], false, false);
    if (val < 0)
    {
        return -1;
    }
    cfg->launch_num = val;
    return 0;
}

static inline int config_parse_tx_burst(int argc, char *argv[], void *data)
{
    int val = 0;
    dfxp_traffic_config_t *cfg = data;

    if (argc != 2)
    {
        return -1;
    }

    val = config_parse_number(argv[1], false, false);
    if ((val < 1) || (val > TX_BURST_MAX))
    {
        return -1;
    }
    cfg->tx_burst = val;
    return 0;
}

static int config_parse_slow_start(int argc, char *argv[], void *data)
{
    int val = 0;
    dfxp_traffic_config_t *cfg = data;

    if (argc != 2)
    {
        return -1;
    }

    val = config_parse_number(argv[1], false, false);
    if ((val < SLOW_START_MIN) || (val > SLOW_START_MAX))
    {
        return -1;
    }
    cfg->slow_start = val;
    return 0;
}

static int config_parse_wait(int argc, char *argv[], void *data)
{
    int val = 0;
    dfxp_traffic_config_t *cfg = data;

    if (argc != 2)
    {
        return -1;
    }

    val = config_parse_number(argv[1], false, false);
    if (val <= 0)
    {
        return -1;
    }
    cfg->wait = val;

    return 0;
}

