
#ifndef _MMTRACE_H_
#define	_MMTRACE_H_

#ifdef	__cplusplus
extern "C" {
#endif
    
#define MM_TRACE_LEVEL_ERR  0
#define MM_TRACE_LEVEL_WARN 1
#define MM_TRACE_LEVEL_INFO 2
    
    

#define MM_ASSERT(COND)                                     \
do {                                                        \
 if(!(COND)){                                               \
     mmtrace(MM_TRACE_LEVEL_ERR,                            \
              __FILE__,                                     \
              __LINE__,                                     \
              #COND);                                       \
      exit(1);                                              \
    }                                                       \
} while(0) 
 
   
#define MM_INFO(b...) mmtrace(MM_TRACE_LEVEL_INFO, __FILE__, __LINE__, b)    
#define MM_WARN(b...) mmtrace(MM_TRACE_LEVEL_WARN, __FILE__, __LINE__, b)    
#define MM_ERR(b...)  mmtrace(MM_TRACE_LEVEL_ERR, __FILE__, __LINE__, b)    
    
void mmtrace_level(int level_);
void mmtrace(int level_, const char *file_, int line_, const char *format_, ...);



#ifdef	__cplusplus
}
#endif

#endif	/* _MMTRACE_H_ */

