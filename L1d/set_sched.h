#ifndef   SET_SCHED_H
#define   SET_SCHED_H

#define _GNU_SOURCE
#include <sched.h>


int pin_cpu(int cpu);


int set_real_time_sched_priority(int policy, int priority);

#endif
