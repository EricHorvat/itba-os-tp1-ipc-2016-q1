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

static sem_t * sem_id;

typedef struct {
	int id;
	int status;
} thread_t;

typedef struct {
	int id;
	int total_threads;
	thread_t ** threads;
} worker_t;

#define REFRESH_INTERVAL 1

void signal_callback_handler(int signum) {

	/**
	 * Semaphore unlink: Remove a named semaphore  from the system.
	 */
	if ( shm_unlink(SEM_PATH) < 0 ) {
		perror("shm_unlink");
	}
	/**
	 * Semaphore Close: Close a named semaphore
	 */
	if ( sem_close(sem_id) < 0 ) {
		perror("sem_close");
	}

	/**
	 * Semaphore unlink: Remove a named semaphore  from the system.
	 */
	if ( sem_unlink(SEM_PATH) < 0 ) {
		perror("sem_unlink");
	}

		endwin();

   // Terminate program
   exit(signum);
}


int main(int argc, char *argv[]) {
	int shmfd;
	int vol, cur;
	int shared_seg_size = (1 * sizeof(shared_data_t));   /* want shared segment capable of storing 1 message */
	shared_data_t *shared_msg;      /* the shared segment, and head of the messages list */

	worker_t** workers = (worker_t**)malloc( 100 * sizeof(worker_t*) );

	// memset(workers, NULL, 20 * sizeof(worker_t*));

	signal(SIGINT, signal_callback_handler);

	/* creating the shared memory object    --  shm_open()  */
	shmfd = shm_open(SHMOBJ_PATH, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
	if (shmfd < 0) {
		perror("In shm_open()");
		exit(1);
	}

	fprintf(stderr, "Created shared memory object %s\n", SHMOBJ_PATH);

	/* adjusting mapped file size (make room for the whole segment to map)      --  ftruncate() */
	ftruncate(shmfd, shared_seg_size);

	/**
	 * Semaphore open
	 */
	sem_id = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);

	/* requesting the shared segment    --  mmap() */
	shared_msg = (shared_data_t *)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	if (shared_msg == NULL) {
		perror("In mmap()");
		exit(1);
	}

	fprintf(stderr, "Shared memory segment allocated correctly (%d bytes).\n", shared_seg_size);

	vol = 10;
	cur = 0;

	int current_worker = -1;
	int current_thread = -1;

	int total_workers = 0;

	worker_t *curr_worker;
	thread_t *curr_thread;

	int current_status = -1;

	WINDOW *win = initscr();
	noecho();
	cbreak();
	start_color();
	init_pair(STATUS_ERROR, COLOR_RED, COLOR_BLACK);
	init_pair(STATUS_OK, COLOR_GREEN, COLOR_BLACK);
	init_pair(STATUS_WARN, COLOR_YELLOW, COLOR_BLACK);
	init_pair(STATUS_IDLE, COLOR_CYAN, COLOR_BLACK);
	init_pair(5, COLOR_WHITE, COLOR_BLACK);

	int c;

	int k = 1;

	shared_msg->worker = -1;
	shared_msg->thread = -1;
	shared_msg->status = -1;

	while ( 1 ) {

		if (shared_msg->worker == -1)
			continue;

		
		current_worker = shared_msg->worker;
		current_thread = shared_msg->thread;
		current_status = shared_msg->status;
			
		clear();
		move(0,0);
		sleep(1);

		mvprintw( 0, 0, "%d %d %d", current_worker, current_thread, current_status);
		refresh();

		int i = 0;
		int j = 0;

		for (i = 0; i < total_workers; i++) {
			if (workers[i]->id == current_worker) {
				for (j = 0; j < workers[i]->total_threads; j++) {
					if (workers[i]->threads[j]->id == current_thread) {
						workers[i]->threads[j]->status = current_status;
						break;
					}
				}
				if (j == workers[i]->total_threads) {
					workers[i]->threads[j] = (thread_t*)malloc(sizeof(thread_t));
					workers[i]->threads[j]->id = current_thread;
					workers[i]->threads[j]->status = current_status;
					workers[i]->threads[j+1] = NULL;
					workers[i]->total_threads++;
				}
				break;
			}
		}

		if (i == total_workers) {
			mvprintw(0,20,"new: %d", i);
			refresh();
			workers[i] = (worker_t*)malloc(sizeof(worker_t));
			workers[i]->total_threads = 0;
			workers[i]->id = current_worker;
			workers[i]->threads = (thread_t**)malloc(100*sizeof(thread_t*));
			workers[i]->threads[0] = (thread_t*)malloc(sizeof(thread_t));
			workers[i]->threads[0]->id = current_thread;
			workers[i]->threads[0]->status = current_status;
			workers[i]->threads[1] = NULL;
			workers[i]->total_threads = 1;
			workers[i+1] = NULL;
			total_workers++;
		}

		j = 1;


		for (i = 0; i < total_workers; i++) {

			mvprintw(j, 0, "%d worker %d", j, workers[i]->id);

			j++;
			for (k = 0; k < workers[i]->total_threads; k++) {

				attron(COLOR_PAIR(workers[i]->threads[k]->status));
				mvprintw(j, 2, "thread %d:", j, workers[i]->threads[k]->id);
				attroff(COLOR_PAIR(workers[i]->threads[k]->status)); 
				refresh();
				j++;
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
	if ( sem_close(sem_id) < 0 )
	{
		perror("sem_close");
	}

	/**
	 * Semaphore unlink: Remove a named semaphore  from the system.
	 */
	if ( sem_unlink(SEM_PATH) < 0 )
	{
		perror("sem_unlink");
	}

	return 0;
}
