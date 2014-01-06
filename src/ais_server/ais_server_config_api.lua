-- 'ais_server' API definition

-- technical definition
mmapi_m = {
  basename = "ais_server_config",
  dirname = "src/ais_server",
  version = "0.1",
  define = "__AIS_SERVER_CONFIG"
}

-- logical definition
-- singletons
mmapi = {
         min_ships = "int",
         max_ships = "int",
         step_ships = "int",
         admin_http_port = "int",
         ais_udp_in_port = "int",
         ais_tcp_server = {
                             name             = "char*",
                             port             = "int",
                             max_connections  = "int"
         },
}
--  lists 
mmapi_list = {

}
