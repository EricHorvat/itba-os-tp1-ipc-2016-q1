#include <comm.send.api.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
// https://gist.github.com/jbenet/1087739
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
// http://cursuri.cs.pub.ro/~apc/2003/resources/pthreads/uguide/users-77.htm
pthread_cond_t done  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t calculating = PTHREAD_MUTEX_INITIALIZER;
#endif

void current_utc_time(struct timespec *ts) {

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	ts->tv_sec = mts.tv_sec;
	ts->tv_nsec = mts.tv_nsec;
#else
	clock_gettime(CLOCK_REALTIME, ts);
#endif

}

#include <file_utils.h>
#include <utils.h>
#include <comm.fifo.h>

/**

	Typedefs

**/

typedef struct {
	comm_callback_t cb;
	connection_t *conn;
	comm_sense_t sense;
} comm_thread_info_t;

typedef struct {
	int fd;
	char *fifo;
	bool success;
} comm_data_writer_ret_t;

typedef void* (*pthread_func_t)(void* data);

static unsigned int TIMEOUT = 10;

/**
 * [set_timeout description]
 * @param t timeout to set
 */
void set_timeout(unsigned int t) {
	TIMEOUT = t;
}

/**

	Prototypes

**/

static void *data_listener(void *);
// static void *pthread_start_with_timeout(pthread_t thread, pthread_func_t func, void *data, unsigned int t);
static void *data_writer(void *data);
// static void write_one_by_one(int fd, void *data, size_t size);
static int create_fifo(char *fifo);

/**

	Typedefs

**/



/**
 * [create_fifo description]
 * @param  fifo the fifo path
 * @return      0 if fifo was already created or successfully created -1 otherwise
 */
static int create_fifo(char *fifo) {

	if (access(fifo, F_OK) == -1) {
		// fifo does not exist
		if (access(fifo, F_OK) == -1 && mkfifo(fifo, FIFO_PERMS) == -1) {
			fprintf(stderr, "mkfifo failed with errno:%d\tmsg%s\n", errno, strerror(errno));
			return -1;
		}
	}

	return 0;
}

/**
 * [pthread_start_with_timeout description]
 * @param  thread the thread to start
 * @param  func   function to start a new thread with
 * @param  data   void* to pass to func
 * @param  t      timeout
 * @return        null if timeout was reached otherwise the result of func
 */
// static void * pthread_start_with_timeout(pthread_t thread, pthread_func_t func, void *data, unsigned int t) {

// 	struct timespec ts;
// 	int thread_err;
// 	void *ret;

// 	current_utc_time(&ts);
// 	ts.tv_sec += t;

// 	#ifdef __MACH__
// 	pthread_mutex_lock(&calculating);
// #endif

// 	pthread_create(&thread, NULL, func, data);

// #ifdef __MACH__
// 	thread_err = pthread_cond_timedwait(&done, &calculating, &ts);

// 	if (thread_err == ETIMEDOUT) {
// 		fprintf(stderr, "%s: calculation timed out\n", __func__);
// 		return null;
// 	}

// 	if (!thread_err) {
// 		pthread_mutex_unlock(&calculating);
// 		pthread_join(thread, &ret);
// 	}

// #else
// 	thread_err = pthread_timedjoin_np(thread, &ret, &ts);

// 	if (thread_err != 0) {
		
// 		return null;
// 	}
// #endif

// 	return ret;
// }

pthread_mutex_t lock;
bool mutex_init = no;

static void *data_listener(void *data) {

	comm_error_t *err;
	//char *fifo;
	//size_t fifo_len;
	size_t read_bytes = 0;
	//int fd;
	char* buffer;

	long int self = (long int)pthread_self();

	buffer = (char*)malloc(2048);
	memset(buffer, '\0', 2048);

	comm_thread_info_t *info = (comm_thread_info_t*)data;

	err = NEW(comm_error_t);

	
	// fifo = (char*)malloc(strlen(FIFO_PATH_PREFIX)+strlen(info->origin->host)+strlen(FIFO_RESPONSE_EXTENSION)+strlen(FIFO_EXTENSION));
	//fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(info->conn->client_addr->host)+strlen(info->conn->server_addr->host)+strlen(FIFO_EXTENSION);
	//fifo = (char*)malloc(fifo_len+2);

	//if (!fifo) {
	//	fprintf(stderr, ANSI_COLOR_RED"MemoryError: FIFO [%s] could not be allocated\n"ANSI_COLOR_RESET, fifo);
	//	abort();
	//}

	//printf("[thread] Writing FIFO\n");

	// sprintf(fifo, "%s%s%s%s", FIFO_PATH_PREFIX, info->origin->host, FIFO_RESPONSE_EXTENSION, FIFO_EXTENSION);
	// sprintf(fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, info->conn->endpoint->host, info->origin->host, FIFO_EXTENSION);
	//if (info->sense != COMMUNICATION_CLIENT_SERVER)
	//	fifo_len = sprintf(fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, info->conn->client_addr->host, info->conn->server_addr->host, FIFO_EXTENSION);
	//else
	//	fifo_len = sprintf(fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, info->conn->server_addr->host, info->conn->client_addr->host, FIFO_EXTENSION);
	//fifo[fifo_len] = '\0';

	// OJOOOOOO
	//if (!exists(fifo)) // OJOOOOOO
	//	while (!exists(fifo)); // OJOOOOOO
	// OJOOOOOO

	//if ( (fd = open(fifo, O_RDONLY)) == -1) {

	//	printf("cannot open %s with error:%d msg:%s\n", fifo, errno, strerror(errno));

	//	unlink(fifo);
	//	err->code = 2;
	//	err->msg = "[thread] FIFO could not be opened";
	//	info->cb(err, info->conn, NULL);

	//	pthread_exit(NULL);

	//	return nil;

	//}

	pthread_mutex_lock(&lock);

	printf("[thread %ld] about to read %d\n", self, info->conn->res_fd);
	//flock(fd, LOCK_SH);
	do {
		read(info->conn->res_fd, buffer+read_bytes, 1);
	} while (*(buffer+read_bytes++) != '\0');
	//flock(fd, LOCK_UN);
	pthread_mutex_unlock(&lock);
	//close(fd);


	err->code = 0;
	err->msg = "Operacion Exitosa";

	info->cb(err, info->conn, buffer);

	pthread_exit(NULL);

	return nil;

}

void comm_send_data(void *data, size_t size, connection_t *conn, comm_sense_t sense, comm_error_t *error) {


	if (!error)
		error = NEW(comm_error_t);

	printf(ANSI_COLOR_CYAN"locking fd(%d)\n"ANSI_COLOR_RESET, conn->req_fd);
	flock(conn->req_fd, LOCK_EX);
	write_one_by_one(conn->req_fd, data, size);
	flock(conn->req_fd, LOCK_UN);
	printf(ANSI_COLOR_CYAN"unlocking fd(%d)\n"ANSI_COLOR_RESET, conn->req_fd);

	error->code = 0;
	error->msg = "Todo OK";

}

void comm_send_data_async(void * data, size_t size, connection_t *conn, comm_sense_t sense, comm_callback_t cb) {

	int pthread_ret;
	pthread_t listener;
	comm_error_t *err;
	comm_thread_info_t *thread_arg;

	printf(ANSI_COLOR_CYAN"locking fd(%d)\n"ANSI_COLOR_RESET, conn->req_fd);
	flock(conn->req_fd, LOCK_EX);
	write_one_by_one(conn->req_fd, data, size);
	flock(conn->req_fd, LOCK_UN);
	printf(ANSI_COLOR_CYAN"unlocking fd(%d)\n"ANSI_COLOR_RESET, conn->req_fd);

	thread_arg = NEW(comm_thread_info_t);

	// fill in thread arguments
	thread_arg->cb = cb;
	thread_arg->conn = conn;
	thread_arg->sense = sense;



	if (!mutex_init) {
		if (pthread_mutex_init(&lock, NULL) != 0) {
        	printf("\n mutex init failed\n");
        	err = NEW(comm_error_t);
        	err->code = 3;
        	err->msg = "mutex init failed";
        	return cb(err, conn, null);
        }
        mutex_init = yes;
    }

	if ( (pthread_ret = pthread_create(&listener, NULL, data_listener, (void*)thread_arg)) ) {
		fprintf(stderr, ANSI_COLOR_RED"pthread create returned %d\n"ANSI_COLOR_RED, pthread_ret);
	}

}
