#include <comm.send.api.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
// https://gist.github.com/jbenet/1087739
#include <time.h>
#include <sys/time.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <glib.h>
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

#include <pthread.h>
#include <utils.h>
#include <comm.fifo.h>

/**

	Typedefs

**/

typedef struct {
	comm_callback_t cb;
	comm_addr_t* origin;
	comm_addr_t* endpoint;
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
static void *pthread_start_with_timeout(pthread_t thread, pthread_func_t func, void *data, unsigned int t);
static void *data_writer(void *data);
static void write_one_by_one(int fd, void *data, size_t size);
static int create_fifo(char *fifo);

/**

	Typedefs

**/

/**
 * writes data one byte at a time
 * @param fd   fd to write to
 * @param data data to write
 * @param size size of data
 */
static void write_one_by_one(int fd, void *data, size_t size) {
	size_t written = 0;
	while (written < size) {
		write(fd, data+written, 1);
		written++;
	}
}

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
static void * pthread_start_with_timeout(pthread_t thread, pthread_func_t func, void *data, unsigned int t) {

	struct timespec ts;
	int thread_err;
	void *ret;

	current_utc_time(&ts);
	ts.tv_sec += t;

	#ifdef __MACH__
	pthread_mutex_lock(&calculating);
#endif

	pthread_create(&thread, NULL, func, data);

#ifdef __MACH__
	thread_err = pthread_cond_timedwait(&done, &calculating, &ts);

	if (thread_err == ETIMEDOUT) {
		fprintf(stderr, "%s: calculation timed out\n", __func__);
		return null;
	}

	if (!thread_err) {
		pthread_mutex_unlock(&calculating);
		pthread_join(thread, &ret);
	}

#else
	thread_err = pthread_timedjoin_np(thread, &ret, &ts);

	if (writer_thread_err != 0) {
		
		return null;
	}
#endif

	return ret;
}

static void *data_listener(void *data) {

	comm_error_t *err;
	char *fifo;
	size_t read_bytes = 0;
	int fd;
	char* buffer;

	buffer = (char*)malloc(2048);

	comm_thread_info_t *info = (comm_thread_info_t*)data;

	buffer[0] = 'c';
	err = NEW(comm_error_t);

	fifo = (char*)malloc(strlen(FIFO_PATH_PREFIX)+strlen(info->origin->host)+strlen(FIFO_RESPONSE_EXTENSION)+strlen(FIFO_EXTENSION));

	if (!fifo) {
		fprintf(stderr, "MemoryError: FIFO [%s] could not be allocated\n", fifo);
		abort();
	}

	printf("[thread] Writing FIFO\n");

	sprintf(fifo, "%s%s%s%s", FIFO_PATH_PREFIX, info->origin->host, FIFO_RESPONSE_EXTENSION, FIFO_EXTENSION);

	while (access(fifo, F_OK) == -1) {
		usleep(500);
	}

	// if () {

	// 	printf("cannot access %s\n", fifo);

	// 	err->code = 2;
	// 	err->msg = "[thread] Response FIFO not found";

	// 	info->cb(err, info->endpoint, NULL);

	// 	pthread_exit(NULL);

	// 	return nil;

	// }

	if ( (fd = open(fifo, O_RDONLY)) == -1) {

		printf("cannot open %s\n", fifo);

		unlink(fifo);
		err->code = 2;
		err->msg = "[thread] FIFO could not be written";
		info->cb(err, info->endpoint, NULL);

		pthread_exit(NULL);

		return nil;

	}
	

	while (*(buffer+read_bytes) != '\0') {
		read(fd, (buffer+(int)read_bytes), 1);
		read_bytes++;
	}

	printf("%s\n", buffer);

	close(fd);

	unlink(fifo);

	err->code = 0;
	err->msg = "Operacion Exitosa";

	info->cb(err, info->endpoint, buffer);

	pthread_exit(NULL);

	return nil;

}

void comm_send_data(void *data, size_t size, comm_addr_t *origin, comm_addr_t *endpoint, comm_error_t *error) {

}

static void *data_writer(void *data) {

	comm_error_t *err;
	comm_thread_info_t *info = (comm_thread_info_t*)data;
	comm_data_writer_ret_t *ret_value;

	int fd;
	char *fifo;
	int len;

	int oldtype;

#ifdef __MACH__
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
#endif

	err = NEW(comm_error_t);

	len = strlen(FIFO_PATH_PREFIX)+strlen(info->origin->host)+strlen(FIFO_REQUEST_EXTENSION)+strlen(FIFO_EXTENSION)+1;
	fifo = (char*)malloc(len);

	memset(fifo, '\0', len);

	if (!fifo) {
		fprintf(stderr, "MemoryError: FIFO [%s] could not be allocated\n", fifo);
		abort();
	}

	ret_value = NEW(comm_data_writer_ret_t);
	ret_value->success = yes;

	sprintf(fifo, "%s%s%s%s", FIFO_PATH_PREFIX, info->origin->host, FIFO_REQUEST_EXTENSION, FIFO_EXTENSION);

	printf("> Opening fifo: %s\n", fifo);

	if (create_fifo(fifo) == -1) {
		err->code = 2;
		err->msg = "Request FIFO could not be created";
		info->cb(err, info->endpoint, NULL);
		ret_value->success = no;
	}

	if (ret_value->success && (fd = open(fifo, O_WRONLY)) == -1) {
		unlink(fifo);
		err->code = 2;
		err->msg = "Request FIFO could not be opened";
		info->cb(err, info->endpoint, NULL);
		ret_value->success = no;
	}
	
	if (ret_value->success) {
		ret_value->fd = fd;
		ret_value->fifo = fifo;
	}
#ifdef __MACH__
	pthread_cond_signal(&done);
#endif

	pthread_exit((void*)ret_value);
	return (void*)ret_value;

}



static void *announcement_writer(void *data) {

	comm_error_t *err;
	comm_thread_info_t *info = (comm_thread_info_t*)data;
	comm_data_writer_ret_t *ret_value;

	int fd;
	char *fifo;
	int len;

	int oldtype;

#ifdef __MACH__
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
#endif

	err = NEW(comm_error_t);

	len = strlen(FIFO_PATH_PREFIX)+strlen(info->endpoint->host)+strlen(FIFO_INPUT_EXTENSION)+strlen(FIFO_EXTENSION)+1;
	fifo = (char*)malloc(len);

	memset(fifo, ZERO, len);

	if (!fifo) {
		fprintf(stderr, "MemoryError: FIFO [%s] could not be allocated\n", fifo);
		abort();
	}

	ret_value = NEW(comm_data_writer_ret_t);
	ret_value->success = yes;

	sprintf(fifo, "%s%s%s%s", FIFO_PATH_PREFIX, info->endpoint->host, FIFO_INPUT_EXTENSION, FIFO_EXTENSION);

	if (create_fifo(fifo) == -1) {
		err->code = 2;
		err->msg = "Announce FIFO could not be created";
		info->cb(err, info->endpoint, NULL);
		ret_value->success = no;
	}

	if (ret_value->success && (fd = open(fifo, O_WRONLY)) == -1) {
		unlink(fifo);
		err->code = 2;
		err->msg = "Announce FIFO could not be opened";
		info->cb(err, info->endpoint, NULL);
		ret_value->success = no;
	}

	if (ret_value->success) {
		ret_value->fd = fd;
		ret_value->fifo = fifo;
	}
#ifdef __MACH__
	pthread_cond_signal(&done);
#endif

	pthread_exit((void*)ret_value);

	return (void*)ret_value;

}

static int announce_to_server(comm_thread_info_t *thread_info) {

	// variable declaration
	comm_error_t *err;
	comm_data_writer_ret_t *data_writer_ret;

	char *fifo_address;

	pthread_t announcer_thread;

	int fd;
	int len;

	printf("> Announcing %s request to server %s ", thread_info->origin->host, thread_info->endpoint->host);

	// mem allocation
	err = NEW(comm_error_t);

	data_writer_ret = (comm_data_writer_ret_t*)pthread_start_with_timeout(announcer_thread, announcement_writer, (void*)thread_info, TIMEOUT);

	if (!data_writer_ret) {
		err->code = 7;
		err->msg = "Timed Out";
		thread_info->cb(err, thread_info->endpoint, nil);
		return 1;
	}

	if (!data_writer_ret->success) {
		return 2;
	}

	fd = data_writer_ret->fd;

	len = strlen(FIFO_PATH_PREFIX)+strlen(thread_info->origin->host)+strlen(FIFO_REQUEST_EXTENSION)+strlen(FIFO_EXTENSION)+1;
	fifo_address = (char*)malloc(len);

	memset(fifo_address, ZERO, len);

	sprintf(fifo_address, "%s%s%s%s", FIFO_PATH_PREFIX, thread_info->origin->host, FIFO_REQUEST_EXTENSION, FIFO_EXTENSION);

	printf("through fifo %s\n", data_writer_ret->fifo);

	printf("> Announcement content: %s\n", fifo_address);
	write_one_by_one(fd, fifo_address, strlen(fifo_address));

	close(fd);

	unlink(fifo_address);

	return 0;

}

void comm_send_data_async(void * data, size_t size, comm_addr_t *origin, comm_addr_t *endpoint, comm_callback_t cb) {

	// variable declaration
	comm_error_t *err;
	comm_thread_info_t *thread_arg;
	comm_data_writer_ret_t *data_writer_ret;

	pthread_t writer_thread;
	int writer_thread_err;

	int fd;
	char *res_fifo;
	size_t written = 0;

	pthread_t listen_thread;
	int iret;

	printf("> Sending %s to %s from %s\n", (char*)data, endpoint->host, origin->host);

	// mem allocation
	err = NEW(comm_error_t);
	thread_arg = NEW(comm_thread_info_t);
	// data_writer_ret = NEW(comm_data_writer_ret_t);

	// fill in thread arguments
	thread_arg->cb = cb;
	thread_arg->origin = origin;
	thread_arg->endpoint = endpoint;

	if (announce_to_server(thread_arg) != 0) {
		return;
	}

	data_writer_ret = (comm_data_writer_ret_t*)pthread_start_with_timeout(writer_thread, data_writer, (void*)thread_arg, TIMEOUT);

	if (!data_writer_ret) {
		// timed out
		err->code = 7;
		err->msg = "Timed Out";
		return cb(err, endpoint, nil);
	}

	if (!data_writer_ret->success) {
		return;
	}

	fd = data_writer_ret->fd;

	write_one_by_one(fd, data, size);	

	close(fd);
	// unlink(data_writer_ret->fifo);

	res_fifo = (char*)malloc(strlen(FIFO_PATH_PREFIX)+strlen(origin->host)+strlen(FIFO_RESPONSE_EXTENSION)+strlen(FIFO_EXTENSION)+1);

	memset(res_fifo, '\0', strlen(FIFO_PATH_PREFIX)+strlen(origin->host)+strlen(FIFO_RESPONSE_EXTENSION)+strlen(FIFO_EXTENSION)+1);

	sprintf(res_fifo, "%s%s%s%s", FIFO_PATH_PREFIX, origin->host, FIFO_RESPONSE_EXTENSION, FIFO_EXTENSION);

	printf("Response fifo: %s\n", res_fifo);

	// if (create_fifo(res_fifo) == -1) {
		// err->code = 4;
		// err->msg = "Response FIFO could not be created";
		// return cb(err, endpoint, NULL);
	// }

	if ( (iret = pthread_create(&listen_thread, NULL, data_listener, (void*)thread_arg)) ) {
		fprintf(stderr, "pthread create returned %d\n", iret);
	}

	// cb(err, addr, (char*)data);
}

