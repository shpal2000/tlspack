#ifndef __TLS_CLIENT__H

#include "app.hpp"

#define DEFAULT_WRITE_CHUNK 1200
#define DEFAULT_WRITE_BUFF_LEN 1000000
#define DEFAULT_READ_BUFF_LEN 1000000

class tls_client_cs_grp : public ev_app_cs_grp 
{
public:
    tls_client_cs_grp (json jcfg, std::vector<ev_sockstats*> *stats_arr) 
                                        : ev_app_cs_grp (jcfg, stats_arr) 
    {
        m_cs_data_len = jcfg["cs_data_len"].get<int>();
        m_sc_data_len = jcfg["sc_data_len"].get<int>();
        m_cs_start_tls_len = jcfg["cs_start_tls_len"].get<int>();
        m_sc_start_tls_len = jcfg["sc_start_tls_len"].get<int>();
        m_cipher = jcfg["cipher"].get<std::string>().c_str();

        const char* tls_version = jcfg["tls_version"].get<std::string>().c_str();

        const char* close_type = jcfg["close_type"].get<std::string>().c_str();

        const char* close_notify = jcfg["close_notify"].get<std::string>().c_str();

        m_write_chunk = jcfg["write_chunk"].get<int>();
        if (m_write_chunk == 0){
            m_write_chunk = DEFAULT_WRITE_CHUNK;
        }

        m_write_buffer_len = jcfg["app_snd_buff"].get<uint32_t>();
        if (m_write_buffer_len == 0) {
            m_write_buffer_len = DEFAULT_WRITE_BUFF_LEN;
        }
        m_write_buffer = (char*) malloc (m_write_buffer_len);

        m_read_buffer_len = jcfg["app_rcv_buff"].get<uint32_t>();
        if (m_read_buffer_len == 0) {
            m_read_buffer_len = DEFAULT_READ_BUFF_LEN;
        }
        m_read_buffer = (char*) malloc (m_read_buffer_len);
        
        if (strcmp(close_type, "fin") == 0) {
            m_close = close_fin;
        } else if (strcmp(close_type, "reset") == 0) {
            m_close = close_reset;
        }else {
            m_close = close_fin;
        }


        if (strcmp(close_notify, "send") == 0) {
            m_close_notify = close_notify_send;
        } else if (strcmp(close_notify, "send_recv") == 0) {
            m_close_notify = close_notify_send_recv;
        } else if (strcmp(close_notify, "no_send") == 0)  {
            m_close_notify = close_notify_no_send;
        } else {
            m_close_notify = close_notify_send_recv;
        }

        if (strcmp(tls_version, "sslv3") == 0){
            m_version = sslv3;
        } else if (strcmp(tls_version, "tls1") == 0){
            m_version = tls1;
        } else if (strcmp(tls_version, "tls1_1") == 0){
            m_version = tls1_1;
        } else if (strcmp(tls_version, "tls1_2") == 0){
            m_version = tls1_2;
        } else if (strcmp(tls_version, "tls1_3") == 0){
            m_version = tls1_3;
        } else if (strcmp(tls_version, "tls_all") == 0){
            m_version = tls_all;
        } else {
            m_version = tls_all;
        }
    }

    int m_cs_data_len;
    int m_sc_data_len;
    int m_cs_start_tls_len;
    int m_sc_start_tls_len;

    int m_write_chunk;
    char* m_read_buffer;
    char* m_write_buffer;
    int m_read_buffer_len;
    int m_write_buffer_len;

    std::string m_cipher;
    std::string m_cipher2;
    enum_close_type m_close;
    enum_close_notify m_close_notify;
    enum_tls_version m_version;

    SSL_CTX* m_ssl_ctx;
};

struct tls_client_stats_data : app_stats
{
    uint64_t tls_client_stats_1;
    uint64_t tls_client_stats_100;

    virtual void dump_json (json &j)
    {
        app_stats::dump_json (j);
        
        j["tls_client_stats_1"] = tls_client_stats_1;
        j["tls_client_stats_100"] = tls_client_stats_100;
    }

    virtual ~tls_client_stats_data() {};
};

struct tls_client_stats : tls_client_stats_data
{
    tls_client_stats () : tls_client_stats_data () {}
};

class tls_client_app : public app
{
public:
    tls_client_app(json app_json
                    , tls_client_stats* all_app_stats
                    , ev_sockstats* all_ev_app_stats);

    ~tls_client_app();

    void run_iter(bool tick_sec);
    
    ev_socket* alloc_socket();
    void free_socket(ev_socket* ev_sock);

public:
    std::vector<tls_client_cs_grp*> m_cs_groups;
    int m_cs_group_index;
    int m_cs_group_count;
};

class tls_client_socket : public ev_socket
{
public:
    tls_client_socket()
    {
        m_bytes_written = 0;
        m_bytes_read = 0;
        m_ssl = nullptr;
    };

    virtual ~tls_client_socket()
    {

    };
    
    void ssl_init ();

    void on_establish ();
    void on_write ();
    void on_wstatus (int bytes_written, int write_status);
    void on_read ();
    void on_rstatus (int bytes_read, int read_status);
    void on_finish ();

public:
    tls_client_cs_grp* m_cs_grp;
    tls_client_app* m_app;
    SSL* m_ssl;
    int m_bytes_written;
    int m_bytes_read;
};

#endif