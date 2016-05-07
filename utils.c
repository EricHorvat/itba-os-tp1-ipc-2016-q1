#include <utils.h>
#include <time.h>
#include <math.h>

#define BOUNDARY_PREFIX "--boundaryTPSO1IPC"

static bool seeded = false;

static int rand_max_digits = -1;

int getrnd(int min, int max) {
	if (!seeded) {
		srand(time(NULL));
		seeded = true;
	}
	return rand() % (max-min) + min;
}

char *gen_boundary() {

	char *boundary;
	size_t length;

	if (rand_max_digits == -1) {
		rand_max_digits = ceilf(log10(RAND_MAX));
	}

	length = strlen(BOUNDARY_PREFIX)+rand_max_digits;
	boundary = malloc(length+1);

	length = sprintf(boundary, "%s%d", BOUNDARY_PREFIX, getrnd((int)pow(10,rand_max_digits-1), RAND_MAX ) );
	boundary[length] = ZERO;

	return boundary;

}
