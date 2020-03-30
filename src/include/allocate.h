/* *
 * Copyright (c) 2015, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * allocate.h file for struct and file declerations on mamey allocations.
*/
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <emmintrin.h>

#include "encoder.h"
#include "decoder.h"
#include "parameter.h"
#include "usage.h"

#ifndef ALLOCATE_H
#define ALLOCATE_H

void *AllocObject(size_t size);

char *AllocFileName(char *fn, int a, int b);

void *AllocateMD(decoder_t* codeword);

void *AllocateMem(encoder_t* codeword, int BSIZE, int CSIZE);

void *AllocateMemDec(decoder_t* codeword, int BSIZE, int CSIZE);

#endif /*ALLOCATE_H*/
