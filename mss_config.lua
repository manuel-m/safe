-- configuration file for program 'mss_filter'

-- http server port
admin_http_port = 9997

-- ais incoming port
ais_udp_in_port = 9998

-- geographical filter
geofilter = { 
               x1 = -16.0, 
               y1 = 45.0, 
               x2 = 9.0, 
               y2 = 36.0 
            }


-- ais out udp
ais_out_udp = {  
                "192.168.0.12:9990",
		"192.168.43.63:9990",
                "192.168.200.140:9990", 
                "192.168.200.150:9990",
                "192.168.200.160:9990"
             }
    
