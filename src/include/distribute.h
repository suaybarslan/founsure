/* *
 * Copyright (c) 2015, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * Repair.h file for struct and file declerations.
*/

#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "encoder.h"
#include "decoder.h"

#ifndef DISTRIBUTE_H
#define DISTRIBUTE_H

typedef struct vertex{
	int idx;
	int weight;
}vertex_t;

typedef struct repair{
	bool fast_mode;
	int success;
	int sizeCheck2;
	int sizeCheck3;
	symbol_sd *check2;
	symbol_sd *check3; 
}repair_t;


#endif /*DISTRIBUTE_H*/
