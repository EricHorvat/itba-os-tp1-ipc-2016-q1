#ifndef __MONITOR_HELPER_H__
#define __MONITOR_HELPER_H__

#include <monitor.h>

int init_monitor(void);
void post_status(int worker, long int thread, int status);
void exit_monitor(void);



#endif // __MONITOR_HELPER_H__