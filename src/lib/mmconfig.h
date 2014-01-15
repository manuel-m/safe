#ifndef MMCONFIG_H
#define	MMCONFIG_H

#include "libconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MM_INT    0
#define MM_FLOAT  1
#define MM_DOUBLE 2
#define MM_STRING 3  


#define MM_CFG_GET_INT(CFG,PATH,VALUE)                                         \
do {                                                                           \
  if(CONFIG_TRUE!=config_lookup_int(&(CFG),#PATH, &(VALUE.PATH) ))             \
  {                                                                            \
    MM_ERR("invalid %s in configuration file",#PATH );                         \
    goto err;                                                                  \
  }                                                                            \
}                                                                              \
while(0);

#define MM_CFG_GET_STR(CFG,PATH,VALUE)                                         \
do {                                                                           \
  if(CONFIG_TRUE!=config_lookup_string(&(CFG),#PATH, &(VALUE.PATH) ))          \
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

