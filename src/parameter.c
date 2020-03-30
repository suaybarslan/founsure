/* *
 * Copyright (c) 2015, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * Part of this function is optional in case the file/parameter instructions
 * do not match the code structure and parameter requirements.
 *
 * parameter.c file for function definitions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

#include "parameter.h"
#include "timing.h"
#include "usage.h"

int AdjustParamNoPrecode(encoder_t* EncoderLT, int filesize){
  // This function only takes into account the constraint 
  // due to file size.
  // int redundantZeros;
  //int blocks = (int)floor(filesize / (EncoderLT->sizek*EncoderLT->sizet) );
  int blocks = filesize/(EncoderLT->sizek*EncoderLT->sizet);
  
  int remainder = filesize - blocks*EncoderLT->sizek*EncoderLT->sizet; // in bytes.
  //printf("blocks is %d\n", blocks);
  if (remainder != 0){
    //remainder = ((int)ceil(remainder/EncoderLT->sizet))*EncoderLT->sizet;
    remainder = ((remainder + EncoderLT->sizet - 1)/EncoderLT->sizet)*EncoderLT->sizet;
  }
  int extra_symbols = remainder/EncoderLT->sizet;
  //int k_add = (int)ceil(extra_symbols/blocks);
  int k_add = (extra_symbols + blocks - 1) / blocks; // ceil for integers.
  //if (extra_symbols == 0)
  //  k_add = 0;
  EncoderLT->sizek = EncoderLT->sizek + k_add;
  int redundantZeros = EncoderLT->sizek*EncoderLT->sizet*blocks - filesize;

  return redundantZeros;
}

int AdjustParamWithPrecode(encoder_t* EncoderPre, int filesize, double target_cr){
  // This function takes into account both constraints due to file size
  // as well as precode block size requirements (these are tighter constraints than
  // the previous function). Our search policy is target at changing source/code block
  // length without changing the symbol size. Other policies might be adapted. 

  int target_K 	= EncoderPre->sizesb;
  int K = 0, N 	= 1;
  int array_k 	= OGIG;
  int array_j 	= OGIG;
  int prim 	= MIN_PRIME;
  int blocks 	= 1;
  int tries 	= 0;
  int totaliter = 0;
  int diff_th 	= DIFF_TH;
  double rate_th = RRATE_TH;
  int redundantZeros;

  while(abs(K - target_K) > diff_th || fabs((double)K/(double)N  - target_cr) > rate_th
                                    || array_k > prim || array_j > prim || array_k < array_j 
                                    || abs(blocks*EncoderPre->sizet*K - filesize) > RED_BYTE_TH){

    N = (int)floor((double)target_K/target_cr) + rand()%(RANDOM_WIN_MAX + RANDOM_WIN_MIN) - RANDOM_WIN_MIN;
    prim = LargestPrimeFactor(N);
    array_k = N / prim;
    array_j = array_k - target_K/prim; 
    if (array_j < ARRAY_MIN_J)
      array_j = 2;
  
    K = (array_k - array_j)*prim;
    if (K > 0){
      blocks = filesize / (EncoderPre->sizet*K) + 1;
      tries++;
      if (tries > TRIES_TH){
        diff_th += DELTA_DIFF_TH;
        rate_th += DELTA_RRATE_TH;
        tries = 0;
      }
    }
    totaliter++;
    if(totaliter > TENM){
        printf(ERRORMSG "Error: Unable to find precode rate for ArrayLDPC. Please choose another 'precode' rate. \n" RESET);
        exit(0);
    }     
  }
  EncoderPre->sizesb = K;
  EncoderPre->sizek = N;
  blocks = filesize / (EncoderPre->sizet*EncoderPre->sizesb) + 1;
  redundantZeros = EncoderPre->sizesb*EncoderPre->sizet*blocks - filesize;
  
  return redundantZeros;
}

int LargestPrimeFactor(int number){ 
// This is an extremely efficient algorithm used only for finding largest prime factor.
// This implementation first removes the powers of two (onle even prime number)
// and later conducts a search on an odd number (all the odd priomes) based on two ideas/observations:
// 1. Let N be an odd number, then the second largest prime factor <= sqrt(N).
// 2. It is redundant to check the primeness of smaller factors.
  int i, result = 0;
  //Remove powers of two:
  while((( number % 2 ) == 0) && number > 1)
    number /= 2;

  if (number == 1)
    return 2;
  else{
    for(i = 3; i*i<= number; i+=2){
      if(number%i == 0){
        result = i;
        while(number%i == 0)
          number /= i;
      }
    }
    if(number>1)
      result = number;
    return result;
  }
}

