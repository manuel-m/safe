-- 'mss_filter' API definition

-- technical definition
mmapi_m = {
  basename = "ais_filter_config",
  dirname = "src/ais_filter",
  version = "0.1",
  define = "__AIS_FILTER_CONFIG"
}

-- logical definition
    -- singletons
mmapi = {
         admin_http_port = "int",
         ais_udp_in_port = "int",
         ais_tcp_server = {
                             name             = "char*",
                             port             = "int",
                             max_connections  = "int"
         },
         ais_tcp_error = {
                             name             = "char*",
                             port             = "int",
                             max_connections  = "int"
         },
             
	 geofilter = { 
	               x1 = "double",
	               y1 = "double",
	               x2 = "double",
		       y2 = "double"
	 }
}
    --  lists 
mmapi_list = {
         ais_out_udp = "char*",
}