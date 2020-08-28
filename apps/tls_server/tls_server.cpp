#include "tls_server.hpp"


tls_server_app::tls_server_app(json app_json
                                , tls_server_stats* zone_app_stats
                                , ev_sockstats* zone_sock_stats)
{
    server_config_init (app_json);

    m_app_stats = new tls_server_stats();

    auto srv_list = app_json["srv_list"];
    for (auto it = srv_list.begin(); it != srv_list.end(); ++it)
    {
        auto srv_cfg = it.value ();

        const char* srv_label 
            = srv_cfg["srv_label"].get<std::string>().c_str();
        
        int srv_enable = srv_cfg["enable"].get<int>();
        if (srv_enable == 0) {
            continue;
        }

        tls_server_stats* srv_stats = new tls_server_stats();
        set_app_stats (srv_stats, srv_label);

        std::vector<ev_sockstats*> *srv_stats_arr
            = new std::vector<ev_sockstats*> ();

        srv_stats_arr->push_back (srv_stats);
        srv_stats_arr->push_back (m_app_stats);
        srv_stats_arr->push_back (zone_app_stats);
        srv_stats_arr->push_back (zone_sock_stats);


        tls_server_srv_grp* next_srv_grp
            = new tls_server_srv_grp (srv_cfg, srv_stats_arr);

        if (next_srv_grp->m_emulation_id == 0) {
            next_srv_grp->m_ssl_ctx = next_srv_grp->create_ssl_ctx ();
        }

        m_srv_groups.push_back (next_srv_grp);
    }

    for (auto srv_grp : m_srv_groups)
    {
        tls_server_socket* srv_socket 
            = (tls_server_socket*) new_tcp_listen (&srv_grp->m_srvr_addr
                                                    , 10000
                                                    , srv_grp->m_stats_arr
                                                    , &srv_grp->m_sock_opt);
        if (srv_socket) 
        {
            srv_socket->m_app = this;
            srv_socket->m_srv_grp = srv_grp;
        }
        else
        {
            //todo error handling
        }
    }
}

tls_server_app::~tls_server_app()
{
}

void tls_server_app::run_iter(bool tick_sec)
{
    ev_app::run_iter (tick_sec);

    if (tick_sec)
    {
        m_app_stats->tick_sec();
    }
}

ev_socket* tls_server_app::alloc_socket()
{
    return new tls_server_socket();
}

void tls_server_app::free_socket(ev_socket* ev_sock)
{
    delete ev_sock;
}

void tls_server_socket::on_establish ()
{
    // printf ("on_establish\n");
    m_lsock = (tls_server_socket*) get_parent();
    m_app = m_lsock->m_app;
    m_srv_grp = m_lsock->m_srv_grp;

    if (m_srv_grp->m_emulation_id == 0){
        m_ssl = SSL_new (m_srv_grp->m_ssl_ctx);
    }else{
        m_ssl_ctx = m_srv_grp->create_ssl_ctx ();
        if (m_ssl_ctx) {
            m_ssl = SSL_new (m_ssl_ctx);
            if (not m_ssl){
                SSL_CTX_free (m_ssl_ctx);
                m_ssl_ctx = nullptr;
            }
        }
    }

    if (m_ssl){
        set_as_ssl_server (m_ssl);
    } else {
        //stats
        abort ();
    }
}

void tls_server_socket::on_write ()
{
    // printf ("on_write\n");
    if (m_bytes_written < m_srv_grp->m_sc_data_len) {
        int next_chunk 
            = m_srv_grp->m_sc_data_len - m_bytes_written;

        int next_chunk_target = m_srv_grp->m_write_chunk;
        if (next_chunk_target == 0) {
            next_chunk_target = m_app->get_next_chunk_size ();
        }

        if ( next_chunk > next_chunk_target){
            next_chunk = next_chunk_target;
        }

        if ( next_chunk > m_srv_grp->m_write_buffer_len){
            next_chunk = m_srv_grp->m_write_buffer_len;
        }

        write_next_data (m_srv_grp->m_write_buffer, 0, next_chunk, true);
    } else {
        disable_wr_notification ();
    }
}

void tls_server_socket::on_wstatus (int bytes_written, int write_status)
{
    // printf ("on_wstatus\n");
    if (write_status == WRITE_STATUS_NORMAL) {
        m_bytes_written += bytes_written;
        if (m_bytes_written == m_srv_grp->m_sc_data_len) {
            if (m_srv_grp->m_close == close_reset){
                abort ();
            } else {
                switch (m_srv_grp->m_close_notify)
                {
                    case close_notify_no_send:
                        write_close ();
                        break;
                    case close_notify_send:
                        write_close (SSL_SEND_CLOSE_NOTIFY);
                        break;
                    case close_notify_send_recv:
                        write_close (SSL_SEND_RECEIVE_CLOSE_NOTIFY);
                        break;
                }
            }
        }
    } else {
        abort ();
    }
}

void tls_server_socket::on_read ()
{
    // printf ("on_read\n");
    read_next_data (m_srv_grp->m_read_buffer, 0, m_srv_grp->m_read_buffer_len, true);
}

void tls_server_socket::on_rstatus (int bytes_read, int read_status)
{
    // printf ("on_rstatus\n");
    if (bytes_read == 0) 
    {
        if (read_status != READ_STATUS_TCP_CLOSE) {
            abort ();
        }
    } else {
        m_bytes_read += bytes_read;
    }
}

void tls_server_socket::on_finish ()
{
    // printf ("on_free\n");
    if (m_ssl) {
        SSL_free (m_ssl);
        m_ssl = nullptr;
    }
    if (m_ssl_ctx) {
        SSL_CTX_free (m_ssl_ctx);
        m_ssl_ctx = nullptr;
    }
}
