#ifndef MMCONFIG_H
#define	MMCONFIG_H

#include "libconfig.h"
#include "mmtrace.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MM_INT    0
#define MM_FLOAT  1
#define MM_DOUBLE 2
#define MM_STRING 3  
    
    
#define MM_CONFIG_INIT(PCFG, CFG_FILE_PATH)                                    \
do {                                                                           \
    memset(&values, 0, sizeof (values));                                       \
    config_init((PCFG));                                                       \
    if (!config_read_file((PCFG), (CFG_FILE_PATH))) {                          \
        MM_GERR("%s:%d - %s\n", config_error_file((PCFG)),                     \
                config_error_line((PCFG)), config_error_text((PCFG)));         \
    }                                                                          \
    if (0 < load_config(&cfg)) {                                               \
        MM_GERR("Error in configuration file %s", (CFG_FILE_PATH));            \
    }                                                                          \
}                                                                              \
while(0);
    

#define MM_CFG_GET_INT(CFG,PATH,VALUE)                                         \
do {                                                                           \
  if(CONFIG_TRUE!=config_lookup_int((CFG),#PATH, &(VALUE.PATH) ))              \
  {                                                                            \
    MM_ERR("invalid %s in configuration file",#PATH );                         \
    goto err;                                                                  \
  }                                                                            \
}                                                                              \
while(0);
    
#define MM_CFG_GET_UNSIGNED(CFG,PATH,VALUE)                                    \
do {                                                                           \
  int val;                                                                     \
  if(CONFIG_TRUE!=config_lookup_int((CFG),#PATH, &val ))                       \
  {                                                                            \
    if(0 > val ) {                                                             \
      MM_ERR("invalid %s in configuration file",#PATH );                       \
      goto err;                                                                \
      }                                                                        \
  }                                                                            \
  (VALUE.PATH)=val;                                                            \
}                                                                              \
while(0);    

#define MM_CFG_GET_STR(CFG,PATH,VALUE)                                         \
do {                                                                           \
  if(CONFIG_TRUE!=config_lookup_string((CFG),#PATH, &(VALUE.PATH) ))           \
  {                                                                            \
    MM_ERR("invalid %s in configuration file",#PATH );                         \
    goto err;                                                                  \
  }                                                                            \
}                                                                              \
while(0);

#define MM_CFGNODE_GET_STR(NODE,PATH,VALUE)                                    \
do {                                                                           \
  if(CONFIG_TRUE!=config_setting_lookup_string((NODE),#PATH, &((VALUE)->PATH)))\
  {                                                                            \
    MM_ERR("invalid %s in configuration file",#PATH );                         \
    goto err;                                                                  \
  }                                                                            \
}                                                                              \
while(0);

#define MM_CFGNODE_GET_DOUBLE(NODE,PATH,VALUE)                                 \
do {                                                                           \
  if(CONFIG_TRUE!=config_setting_lookup_float((NODE),#PATH, &((VALUE)->PATH))) \
  {                                                                            \
    MM_ERR("invalid %s in configuration file",#PATH );                         \
    goto err;                                                                  \
  }                                                                            \
}                                                                              \
while(0);

#define MM_CFGNODE_GET_INT(NODE,PATH,VALUE)                                    \
do {                                                                           \
  if(CONFIG_TRUE!=config_setting_lookup_int((NODE),#PATH, &((VALUE)->PATH)))   \
  {                                                                            \
    MM_ERR("invalid %s in configuration file",#PATH );                         \
    goto err;                                                                  \
  }                                                                            \
}                                                                              \
while(0);




#ifdef __cplusplus
}
#endif

#endif	/* MMCONFIG_H */

