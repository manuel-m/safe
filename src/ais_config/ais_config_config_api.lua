-- 'mss_filter' API definition

-- technical definition
mmapi_m = {
  basename = "ais_conf",
  dirname = "src/ais_config",
  version = "0.1",
  define = "__AIS_CONFIG_CONFIG"
}

-- logical definition
    -- singletons
mmapi = {
    udp_in_port = "int",
    tcp_server = {
        name             = "char*",
        port             = "int",
        max_connections  = "int",
        l2 = {
            name_L2 = "char*",
            int_L2  = "int",
            l3      = {
                name_l3 = "char*",
                int_l3  = "int"
            }
        }
    }
}
