/* *
 * Copyright (c) 2016, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * timing.h file for measuring time.
*/
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>

#ifndef TIMING_H
#define TIMING_H

typedef struct timeval timeval_t;

void time_diff(timeval_t *result, timeval_t *time1, timeval_t *time2);


#endif /*TIMING_H*/
