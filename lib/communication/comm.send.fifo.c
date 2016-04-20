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

typedef struct {
	comm_callback_t cb;
	comm_addr_t* origin;
	comm_addr_t* endpoint;
} comm_thread_info_t;

typedef struct {
	int fd;
	char *fifo;
} comm_data_writer_ret_t;

static void *data_listener(void *);

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

	fifo = (char*)malloc(sizeof(FIFO_PATH_PREFIX)+sizeof(info->origin->host)+sizeof(FIFO_RESPONSE_EXTENSION));

	if (!fifo) {
		err->code = 1;
		err->msg = "[thread] FIFO could not be allocated";
		info->cb(err, info->endpoint, NULL);

		pthread_exit(NULL);

		return nil;
	}

	printf("[thread] Writing FIFO\n");

	sprintf(fifo, "%s%s%s", FIFO_PATH_PREFIX, info->origin->host, FIFO_RESPONSE_EXTENSION);

	if (access(fifo, F_OK) == -1) {

		err->code = 2;
		err->msg = "[thread] Response FIFO not found";

		info->cb(err, info->endpoint, NULL);

		pthread_exit(NULL);

		return nil;

	}

	if ( (fd = open(fifo, O_RDONLY)) == -1) {

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

	int oldtype;

	printf("data writer\n");

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);

	err = NEW(comm_error_t);

	fifo = (char*)malloc(sizeof(FIFO_PATH_PREFIX)+sizeof(info->origin->host)+sizeof(FIFO_REQUEST_EXTENSION));

	if (!fifo) {
		err->code = 1;
		err->msg = "FIFO could not be allocated";
		info->cb(err, info->endpoint, NULL);
		pthread_exit(NULL);
		return nil;
	}


	printf("Writing FIFO\n");

	sprintf(fifo, "%s%s%s", FIFO_PATH_PREFIX, info->origin->host, FIFO_REQUEST_EXTENSION);

	if (access(fifo, F_OK) == -1) {
		// fifo does not exist

		if (mkfifo(fifo, FIFO_PERMS) == -1) {
			err->code = 2;
			err->msg = "FIFO could not be created";
			info->cb(err, info->endpoint, NULL);
			pthread_exit(NULL);
			return nil;
		}

	}

	printf("Opening FIFO: %s\n", fifo);
	if ( (fd = open(fifo, O_WRONLY)) == -1) {
		unlink(fifo);
		err->code = 2;
		err->msg = "FIFO could not be opened";
		info->cb(err, info->endpoint, NULL);
		pthread_exit(NULL);
		return nil;
	}

	ret_value = NEW(comm_data_writer_ret_t);

	ret_value->fd = fd;
	ret_value->fifo = fifo;

	pthread_cond_signal(&done);

	pthread_exit((void*)ret_value);

	return (void*)ret_value;

}

 void comm_send_data_async(void * data, size_t size, comm_addr_t *origin, comm_addr_t *endpoint, comm_callback_t cb) {

 	comm_error_t *err;
 	comm_thread_info_t *thread_arg;
 	comm_data_writer_ret_t *data_writer_ret;

 	struct timespec ts;
 	pthread_t writer_thread;
 	int writer_thread_err;

 	int fd;
 	char *res_fifo;
 	size_t written = 0;

 	pthread_t listen_thread;
 	int iret;

 	err = NEW(comm_error_t);
 	thread_arg = NEW(comm_thread_info_t);
 	data_writer_ret = NEW(comm_data_writer_ret_t);

 	thread_arg->cb = cb;
 	thread_arg->origin = origin;
 	thread_arg->endpoint = endpoint;

 	current_utc_time(&ts);

 	ts.tv_sec += 20;

#ifdef __MACH__
 	pthread_mutex_lock(&calculating);
#endif

 	pthread_create(&writer_thread, NULL, data_writer, (void*)thread_arg);

 	printf("Before wait\n");
#ifdef __MACH__
 	printf("Mach\n");
 	writer_thread_err = pthread_cond_timedwait(&done, &calculating, &ts);

 	// pthread_join(writer_thread, (void**)&(data_writer_ret));

 	printf("Writer err: %d\n", writer_thread_err);

 	if (writer_thread_err == ETIMEDOUT) {
		fprintf(stderr, "%s: calculation timed out\n", __func__);
		// timed out
 		err->code = 7;
 		err->msg = "Timed Out";
 		return cb(err, endpoint, nil);
 	}

 	if (!writer_thread_err) {
 		printf("no error\n");
		pthread_mutex_unlock(&calculating);
		pthread_join(writer_thread, (void**)&(data_writer_ret));
		printf("fd: %d\n", data_writer_ret->fd);
		printf("fifo: %s\n", data_writer_ret->fifo);
 	}

#else
 	printf("linux\n");
 	writer_thread_err = pthread_timedjoin_np(writer_thread, (void**)&(data_writer_ret), &ts);

 	printf("Writer err: %d\n", writer_thread_err);

 	if (writer_thread_err != 0) {
		// timed out
 		err->code = 7;
 		err->msg = "Timed Out";
 		return cb(err, endpoint, nil);
 	}
#endif

 	fd = data_writer_ret->fd;

 	while (written < size) {
 		write(fd, data+written, 1);
 		written++;
 	}

 	close(fd);
 	unlink(data_writer_ret->fifo);

 	return;

 	res_fifo = (char*)malloc(sizeof(FIFO_PATH_PREFIX)+sizeof(origin->host)+sizeof(FIFO_RESPONSE_EXTENSION));

 	sprintf(res_fifo, "%s%s%s", FIFO_PATH_PREFIX, origin->host, FIFO_RESPONSE_EXTENSION);

 	if (access(res_fifo, F_OK) == -1) {
		// fifo does not exist

 		if (mkfifo(res_fifo, FIFO_PERMS) == -1) {
 			err->code = 4;
 			err->msg = "Response FIFO could not be created";
 			return cb(err, endpoint, NULL);
 		}

 	}

 	if ( (iret = pthread_create(&listen_thread, NULL, data_listener, (void*)thread_arg)) ) {
 		fprintf(stderr, "pthread create returned %d\n", iret);
 	}

	// cb(err, addr, (char*)data);
 }

