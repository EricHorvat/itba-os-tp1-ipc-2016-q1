#ifndef __MONITOR_H__
#define __MONITOR_H__

#define SHMOBJ_PATH "/shmtpsoipc"

#define SEM_PATH "/semtpsoipc"

typedef struct  {
	int worker;
	long int thread;
	int status;
} shared_data_t;

#define STATUS_OK 2
#define STATUS_WARN 3
#define STATUS_ERROR 1
#define STATUS_IDLE 4

#endif // __MONITOR_H__