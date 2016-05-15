#include <helpers/monitor_helpers.h>
#include <utils.h>
#include <monitor.h>

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>

static sem_t * sem_id;
static shared_data_t* shared_msg = NULL;

static void signal_callback_handler(int signum);

static void signal_callback_handler(int signum) {

	shm_unlink(SEM_PATH);
	sem_close(sem_id);
	sem_unlink(SEM_PATH);
	exit(signum);
}

int init_monitor(void) {

	int shmfd;
	int shared_seg_size = (1 * sizeof(shared_data_t));


	signal(SIGINT, signal_callback_handler);

	shmfd = shm_open(SHMOBJ_PATH, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
	if (shmfd < 0) {
		ERROR("shm_open()");
		return 2;
	}

	/* adjusting mapped file size (make room for the whole segment to map)      --  ftruncate() */
	ftruncate(shmfd, shared_seg_size);

	sem_id = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);

	shared_msg = (shared_data_t *)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	if (shared_msg == NULL) {
			ERROR("mmap()");
			return 1;
	}

	return 0;
}

void post_status(int worker, long int thread, int status) {
	if (!shared_msg)
		return;
	sem_wait(sem_id);
	shared_msg->worker = worker;
	shared_msg->thread = thread;
	shared_msg->status = status;
	sem_post(sem_id);
}

void exit_monitor(void) {

	signal_callback_handler(0);

} 
