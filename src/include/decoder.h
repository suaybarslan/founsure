/* *
 * Copyright (c) 2015, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * Decoder.h file for struct and file declerations.
*/
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "encoder.h"

#ifndef DECODER_H
#define DECODER_H

typedef struct symbolSD{ // Source symbol
  int deg;
  int avail;
  int *connections;
} symbol_sd;

typedef struct symbolTD{ // Coded symbol
  unsigned ID;
  int deg;
  int avail;
  int *connections;
} symbol_td;

typedef struct decoder{
  int sizesb;
  int sizek;
  int sizen;
  int sizet;
  int success;
  int unrecovered;
  unsigned encSeed;
  sym_t *srcdata;
  sym_t *encdata;
  symbol_sd *srcSymbolArray; 
  symbol_td *encSymbolArray;
}decoder_t;


void *PrepareDec( decoder_t *codeword, dist_t *Dist, int *erased, int disks);

void *runBPNoData( decoder_t *codeword);

void *runBP( decoder_t *codeword);

void *prepare_decoding_path_mt(decoder_t *codeword);

void *runBP_mt_advance(decoder_t *codeword, int numOfThreads, int *decoding_path);

void *runBP_mt( decoder_t *codeword, int numOfThreads);

void *runBP4PrecodeNoData( decoder_t *codeword);

void *runBP4Precode( decoder_t *codeword);

#endif /*DECODER_H*/
