project('tlspack', 'cpp',
  version : '1.0',
  default_options : ['warning_level=3'
                      , 'cpp_std=c++17'])

openssl = dependency('openssl')
nlohmann_json_proj = subproject('json')
nlohmann_json_dep = nlohmann_json_proj.get_variable('nlohmann_json_dep')
incdirs = include_directories(['./ev_sock/'])

executable('tlspack.exe'
            , sources : [ './ev_sock/ev_socket.cpp'
                          , './ev_apps/tls_server/tls_server.cpp'
                          , './ev_apps/tls_client/tls_client.cpp'
                          , './ev_apps/tcp_proxy/tcp_proxy.cpp'
                          , 'main.cpp' ]
            , dependencies: [openssl, nlohmann_json_dep]
            , include_directories : [incdirs]
            , install : true)

