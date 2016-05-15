#ifndef __MONITOR_H__
#define __MONITOR_H__

#define SHMOBJ_PATH "/shmtpsoipc"

#define SEM_PATH "/semtpsoipc"

typedef struct  {
	int worker;
	int thread;
	int status;
} shared_data_t;

#endif // __MONITOR_H__