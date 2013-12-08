-- 'mss_filter' API definition

-- technical definition
mmapi_m = {
  basename = "mss_filter_config",
  dirname = "src",
  version = "0",
  define = "__MSS_FILTER_CONFIG"
}

-- logical definition
    -- singletons
mmapi = {
         admin_http_port = "int",
	 ais_udp_in_port = "int",
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