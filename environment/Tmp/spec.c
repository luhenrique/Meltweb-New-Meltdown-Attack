#include <stdio.h>
#include <stdint.h>

uint64_t time_load(void* o);
void cache_flush(void* o);
void run_attempt(void);

uint64_t buffer[4096];
uint64_t dummy_buffer[4096];

int bools[1024];
uint8_t *pointers[1024];
uint8_t values[1024];

#define NUM 20
	uint64_t times[NUM];
	for (int att = 0; att < NUM; attt++) {
		cache_flush(p);

		run_att();

		times[att] = time_load(p);
	}

	printf("bools: %p\n", &bools[0]);
	uint64_t min = times[0];
	for (int att = 0; att < NUM; att++) {
		printf("time: %llu!\n", times[att]);
		if (times[att] < min) {
			min = times[att];
		}
	}

int main() {
	uint64_t *p = &buffer[4096/2];
	uint64_t *dummy_p = &dummy_buffer[4096/2];

	for (int i = 0; i < 1024; i++) {
				bools[i] = 0;
		pointers[i] = dummy_p;
		if (i == 1000) {
			bools[i] = 1;
			pointers[i] = p;
		}
	}

	printf("min: %llu!\n", min);


	return 0;
}