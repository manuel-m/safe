#ifndef MMTEST_H
#define	MMTEST_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define MMTEST_DECL static int has_failed = 0;    
#define MMTEST_END                                                             \
do {                                                                           \
  if (0 == has_failed)                                                         \
    printf("[PASS] %s\n",argv[0]);                                             \
  return has_failed;                                                           \
} while(0);  

#define MMTEST(CHECK)                                                          \
do {                                                                           \
  if((!CHECK)) {                                                               \
   printf("[FAIL] " #CHECK "%s:%d\n", __FILE__, __LINE__);                     \
   has_failed = 1;                                                             \
  }                                                                            \
} while(0); 


#ifdef	__cplusplus
}
#endif

#endif	/* MMTEST_H */

