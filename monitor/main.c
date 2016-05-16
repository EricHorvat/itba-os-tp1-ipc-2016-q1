// https://github.com/Jeshwanth/Linux_code_examples/tree/master/POSIX_shm_sem
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <ncurses.h>
#include <string.h>

#include <monitor.h>
// #include <utils.h>

static sem_t* sem_id;

typedef struct {
	int id;
	int status;
} thread_t;

typedef struct {
	int        id;
	int        total_threads;
	thread_t** threads;
} worker_t;

static void signal_callback_handler(int signum);

static void signal_callback_handler(int signum) {

	shm_unlink(SEM_PATH);
	sem_close(sem_id);
	sem_unlink(SEM_PATH);

	endwin();
	exit(signum);
}

int main(void) {
	int            shmfd;
	int            shared_seg_size = (1 * sizeof(shared_data_t));
	shared_data_t* shared_msg;
	worker_t**     workers;

	int current_worker = -1;
	int current_thread = -1;
	int total_workers  = 0;
	int current_status = -1;
	int k              = 1;
	int i              = 0;
	int j              = 0;

	workers = (worker_t**)malloc(100 * sizeof(worker_t*));

	signal(SIGINT, signal_callback_handler);

	shmfd = shm_open(SHMOBJ_PATH, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
	if (shmfd < 0) {
		perror("shm_open()");
		exit(1);
	}

	ftruncate(shmfd, shared_seg_size);

	sem_id = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);

	shared_msg = (shared_data_t*)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	if (shared_msg == NULL) {
		perror("mmap()");
		exit(1);
	}

	initscr();
	noecho();
	cbreak();
	start_color();
	init_pair(STATUS_ERROR, COLOR_RED, COLOR_BLACK);
	init_pair(STATUS_OK, COLOR_GREEN, COLOR_BLACK);
	init_pair(STATUS_WARN, COLOR_YELLOW, COLOR_BLACK);
	init_pair(STATUS_IDLE, COLOR_CYAN, COLOR_BLACK);
	init_pair(5, COLOR_WHITE, COLOR_BLACK);

	shared_msg->worker = -1;
	shared_msg->thread = -1;
	shared_msg->status = -1;

	while (1) {

		if (shared_msg->worker == -1) {
			usleep(500);
			continue;
		}

		current_worker = shared_msg->worker;
		current_thread = shared_msg->thread;
		current_status = shared_msg->status;

		clear();
		move(0, 0);
		usleep(1500);

		for (i = 0; i < total_workers; i++) {
			if (workers[i]->id == current_worker) {
				for (j = 0; j < workers[i]->total_threads; j++) {
					if (workers[i]->threads[j]->id == current_thread) {
						workers[i]->threads[j]->status = current_status;
						break;
					}
				}
				if (j == workers[i]->total_threads) {
					workers[i]->threads[j]         = (thread_t*)malloc(sizeof(thread_t));
					workers[i]->threads[j]->id     = current_thread;
					workers[i]->threads[j]->status = current_status;
					workers[i]->threads[j + 1]     = NULL;
					workers[i]->total_threads++;
				}
				break;
			}
		}

		if (i == total_workers) {
			workers[i]                     = (worker_t*)malloc(sizeof(worker_t));
			workers[i]->total_threads      = 0;
			workers[i]->id                 = current_worker;
			workers[i]->threads            = (thread_t**)malloc(100 * sizeof(thread_t*));
			workers[i]->threads[0]         = (thread_t*)malloc(sizeof(thread_t));
			workers[i]->threads[0]->id     = current_thread;
			workers[i]->threads[0]->status = current_status;
			workers[i]->threads[1]         = NULL;
			workers[i]->total_threads      = 1;
			workers[i + 1]                 = NULL;
			total_workers++;
		}

		j = 0;

		for (i = 0; i < total_workers; i++) {

			mvprintw(j, 0, "worker %d", workers[i]->id);

			j++;
			for (k = 0; k < workers[i]->total_threads; k++) {

				if (workers[i]->threads[k]->status != STATUS_DOWN) {

					attron(COLOR_PAIR(workers[i]->threads[k]->status));
					mvprintw(j, 2, "thread %d:", workers[i]->threads[k]->id);
					attroff(COLOR_PAIR(workers[i]->threads[k]->status));
					refresh();
					j++;
				}
			}
		}
	}

	nocbreak();
	// keypad(0);
	echo();

	endwin();

	if (shm_unlink(SHMOBJ_PATH) != 0) {
		perror("In shm_unlink()");
		exit(1);
	}
	/**
	 * Semaphore Close: Close a named semaphore
	 */
	if (sem_close(sem_id) < 0) {
		perror("sem_close");
	}

	/**
	 * Semaphore unlink: Remove a named semaphore  from the system.
	 */
	if (sem_unlink(SEM_PATH) < 0) {
		perror("sem_unlink");
	}

	return 0;
}
