/* *
 * Copyright (c) 2015, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * allocate.c file for memory allocations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "allocate.h"

void *AllocObject(size_t size){
  void* a = (void *)malloc(size);
  return a;
}

char *AllocFileName(char *fn, int a, int b){
  char* temp = (char *)malloc(sizeof(char*)*(a+strlen(fn)+b));
  return temp;
}

void *AllocateMD(decoder_t* codeword){
    // Initialize the internal values of the encoder encoded objects
    codeword->encSymbolArray = (symbol_td *)malloc(sizeof(symbol_td)*codeword->sizen); 
    // Initialize the internal values of the encoder source symbols
    codeword->srcSymbolArray = (symbol_sd *)malloc(sizeof(symbol_sd)*codeword->sizek);
    if(codeword->encSymbolArray == NULL || codeword->srcSymbolArray == NULL){
        printf(ERRORMSG "\nError:  Insufficient Memory. Memory cannot be allocated!\n" RESET);
        free(codeword->encSymbolArray);
        exit(0);
    }
    return codeword;
}

void *AllocateMem(encoder_t* codeword, int BSIZE, int CSIZE){

  // Initialize the internal values of the encoder encoded objects
  codeword->encSymbolArray = (symbol_t *)malloc(sizeof(symbol_t)*codeword->sizen); 
  // Initialize the internal values of the encoder source symbols
  codeword->srcSymbolArray = (symbol_s *)malloc(sizeof(symbol_s)*codeword->sizek);
  // Memory for source data:
  codeword->srcdata = (sym_t *)malloc(BSIZE);
  //codeword->srcdata = (sym_t *)_mm_malloc(BSIZE, 16);
  //codeword->srcdata = (sym_t *)__builtin_alloca_with_align(BSIZE, 128);
  // Memory for coded data:
  codeword->encdata = (sym_t *)malloc(CSIZE);
  // codeword->encdata = (sym_t *)calloc(codeword->sizen*codeword->sizet/sizeof(sym_t), sizeof(sym_t));
  //codeword->encdata = (sym_t *)_mm_malloc(CSIZE, 16);

  if(codeword->encSymbolArray == NULL || codeword->srcSymbolArray == NULL || codeword->srcdata == NULL || codeword->encdata == NULL){
    printf(ERRORMSG "\nError:  Insufficient Memory. Memory cannot be allocated!\n" RESET);
    free(codeword->encSymbolArray);
    exit(0);
  }

  return codeword;
}

void *AllocateMemDec(decoder_t* codeword, int BSIZE, int CSIZE){

  // Initialize the internal values of the encoder encoded objects
  codeword->encSymbolArray = (symbol_td *)malloc(sizeof(symbol_td)*codeword->sizen); 
  // Initialize the internal values of the encoder source symbols
  codeword->srcSymbolArray = (symbol_sd *)malloc(sizeof(symbol_sd)*codeword->sizek);
  // Memory for source data:
  codeword->srcdata = (sym_t *)malloc(BSIZE);
  // Memory for coded data:
  codeword->encdata = (sym_t *)malloc(CSIZE);

  if(codeword->encSymbolArray == NULL || codeword->srcSymbolArray == NULL || codeword->srcdata == NULL || codeword->encdata == NULL){
    printf(ERRORMSG "\nError:  Insufficient Memory. Memory cannot be allocated!\n" RESET);
    free(codeword->encSymbolArray);
    exit(0);
  }

  return codeword;
}


