/* *
 * Copyright (c) 2020, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * This file is used to measure encode/decode/total time passed for
 * encoding/decoding operations. It is not essential to core
 * inner workings of founsure.
 *
 * timing.c file for function definitions.
*/

#include "timing.h"
#include "parameter.h"

void time_diff(timeval_t *result, timeval_t *time1, timeval_t *time2){
  
  long int diff = (result->tv_usec + ONEM * result->tv_sec) 
    + (time2->tv_usec + ONEM * time2->tv_sec) 
    - (time1->tv_usec + ONEM * time1->tv_sec);

   
  result->tv_sec = diff / ONEM;
  result->tv_usec = diff % ONEM;

}

