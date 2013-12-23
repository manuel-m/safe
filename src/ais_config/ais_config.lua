ais_udp_in_port = 9990
ais_tcp_server = { 
                     name            = "ais_in",
                     port            = 9999,
                     max_connections = 32,
                     l2 = {
                         name_l2 = "l2",
                         int_l2  = 2,
                         l3 = {
                             name_l3 = "l3",
                             int_l3 = 3
                         }
                      }
                      
}
