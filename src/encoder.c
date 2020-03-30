/* *
 * Copyright (c) 2015, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * Encoder.c file for function definitions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "encoder.h"
#include "usage.h"
#include "parameter.h"
#include "allocate.h"

// RSD parameters to be defined
#define RSD

void *PrepareEnc( encoder_t *codeword, dist_t *Dist, int disks){
  double temp;
  int i, j, k, cnt, temp0;
  int kk, jj, prim,ind;
  int blocksize = codeword->sizen/disks;
  
  if (codeword->sizesb != codeword->sizek){ // If precode needs to apply.
    prim = LargestPrimeFactor(codeword->sizek);
    kk = codeword->sizek/prim;
    jj = kk - codeword->sizesb/prim;
    //printf("%d | %d | %d \n", jj, kk, prim);
    
    for(j=0;j<jj;j++){
      for(k=0;k<prim;k++){
        cnt = 1;
        codeword->srcSymbolArray[codeword->sizesb + k + j*prim].deg = kk - jj + j;
        codeword->srcSymbolArray[codeword->sizesb + k + j*prim].connections = (int *)malloc(sizeof(int *)*(kk-jj+j));
        
        //ind = kk*prim - ((jj-j)*prim-k);
        //codeword->srcSymbolArray[codeword->sizesb + k + j*jj].connections[0] = ind;
        //printf("%d : ",783+k+j*prim);
        for(i=1;i<kk-jj+j+1;i++){
          ind = kk*prim - (jj - j + cnt - 1)*prim - (cnt*(jj - j - 1) - k - 1 + prim)%prim - 1;
          codeword->srcSymbolArray[codeword->sizesb + k + j*prim].connections[i-1] = ind;
          //printf("%d ",ind);
          cnt ++;
        }
        //printf("\n");
      }
    }  
  }
    
  //srand(codeword->encSeed);	
  for(j=0; j< codeword->sizen; j++){
    //For future:
    //For fast mode we generate random edge connections with replacement at exp of some perf loss.
    //for slow mode we generate random edge connections without replacement
    // Set the ID of the symbol in a loop
    
    //codeword->encSymbolArray[j].ID = j*100;
    //srand(codeword->encSeed + codeword->encSymbolArray[j].ID);	
    codeword->encSymbolArray[j].ID = j;
    if (j % blocksize == 0)
    	srand(codeword->encSeed + j/blocksize);
     
    temp = (double)rand() / (double)(RAND_MAX) ;
    for(i=0; i<Dist->maxdeg; i++){
        if(temp>=Dist->cdf[i] && temp<Dist->cdf[i + 1]){
	        codeword->encSymbolArray[j].deg = i + 1;
	        break;
      }
    }
      
    // Set the connections for each symbol:
    codeword->encSymbolArray[j].connections = (int *)malloc(sizeof(int *)*codeword->encSymbolArray[j].deg);

    if(codeword->encSymbolArray[j].connections == NULL){
      printf(ERRORMSG "\nError:  Insufficient Memory. Memory cannot be allocated!\n" RESET);
      free(codeword->encSymbolArray[j].connections);
      exit(0);
    }

    // SELECTION w/o REPLACEMENT: slow. Included for interested users.
    // SWOR(codeword->encSymbolArray[j].deg, codeword->sizek, codeword->encSymbolArray[j].connections);
    // SELECTION w/o REPLACEMENT: brute force - faster.
    
    for(i=0; i<codeword->encSymbolArray[j].deg; i++){
      temp0 = rand() % codeword->sizek;
      cnt = 0;
      while(cnt < i){ // Here we make sure that we do not select the same source symbol twice.
        if (temp0 == codeword->encSymbolArray[j].connections[cnt]){
            temp0 = rand() % codeword->sizek;
            cnt = 0;
   	    }else
            cnt ++;
      }
      codeword->encSymbolArray[j].connections[i] =   temp0; 
    }
  }

  return codeword;
}

void SWOR(int samSize, int popSize, int *connections){
  int totInput = 0; //total number of inputs handled.
  int selItems = 0; //number of selected items upto now.
  double temp;

  while(selItems < samSize){
    temp = (double)rand() / (double)( RAND_MAX );
    if( (popSize-totInput)*temp >= samSize - selItems ){
      totInput++; 
    }else{
      connections[selItems] = totInput;
      totInput++; 
      selItems++;
    }
  }
}

void EncodeCompute(encoder_t *EncoderObj){

  int i,j,k;
  sym_t *ssym_t, *esym_t; 
  esym_t = EncoderObj->encdata;
  
  for(i=0; i<EncoderObj->sizen; i++){
    //printf("%d\n", EncoderObj->sizet/sizeof(sym_t));      
    for(k=0; k< EncoderObj->sizet/sizeof(sym_t); k++){ 
      for(j=0; j< EncoderObj->encSymbolArray[i].deg; j++){
	    ssym_t = &EncoderObj->srcdata[EncoderObj->encSymbolArray[i].connections[j]*EncoderObj->sizet/sizeof(sym_t)]+k;
	    //printf("%" PRIx64 "\n", ssym_t[0]);
	    esym_t[0] ^= ssym_t[0];
      }
      //printf("%" PRIx64 "\n", esym_t[0]);
      esym_t++;
    }
  }
}

void EncodeComputeFast_mt(encoder_t *EncoderObj, int numOfThreads){

  // This particular MT implementation follows the open source standard openMP.
  int i,j,m, ilimit;
  sym_t *ssym_t, *esym_t; 

  //Number of subblocks & symbols per thread:
  int numOfsubblocks = EncoderObj->sizet/sizeof(sym_t);
  int syms_per_thread; // number of different values = number of precoding stages + 1.
  
  
  // Precoding: (first stage)
  if (EncoderObj->sizesb != EncoderObj->sizek){
  	syms_per_thread = (EncoderObj->sizek-EncoderObj->sizesb)/numOfThreads; // Check divisibility.

    #pragma omp parallel for shared(EncoderObj) private(m,i,ilimit,j,esym_t,ssym_t) firstprivate(syms_per_thread, numOfsubblocks, numOfThreads) num_threads(numOfThreads) schedule(dynamic,1)
    for(m=0; m<numOfThreads; m++){
      ilimit = (m+1)*syms_per_thread;
   	  if((m+2)*syms_per_thread > (EncoderObj->sizek-EncoderObj->sizesb))
   	    ilimit = EncoderObj->sizek-EncoderObj->sizesb;
      for(i=EncoderObj->sizesb+m*syms_per_thread; i<EncoderObj->sizesb+ilimit; i++){
        esym_t = &EncoderObj->srcdata[i*numOfsubblocks];
        for(j=0; j< numOfsubblocks; j++) // Initialization is distributed over threads. 
  			esym_t[j] = 0;
        for(j=0; j<EncoderObj->srcSymbolArray[i].deg; j++){
        	ssym_t = &EncoderObj->srcdata[EncoderObj->srcSymbolArray[i].connections[j]*numOfsubblocks];
        	align_XOR(esym_t, ssym_t, numOfsubblocks);
        }
      }
    }      
  }
          
  syms_per_thread = EncoderObj->sizen/numOfThreads;
  // LT encoding here: (second stage)
  #pragma omp parallel for shared(EncoderObj) private(m,i,j,esym_t,ssym_t) firstprivate(syms_per_thread, numOfsubblocks, numOfThreads) num_threads(numOfThreads) schedule(dynamic,1)
  for(m=0; m<numOfThreads; m++){
  	for(i=m*syms_per_thread; i<(m+1)*syms_per_thread; i++){
  		esym_t = EncoderObj->encdata + i*numOfsubblocks;
  		for(j=0; j< numOfsubblocks; j++) // Initialization is distributed over threads. 
  			esym_t[j] = 0;
	  	for(j=0; j< EncoderObj->encSymbolArray[i].deg; j++){
	  		ssym_t = &EncoderObj->srcdata[EncoderObj->encSymbolArray[i].connections[j]*numOfsubblocks];
	  		align_XOR(esym_t, ssym_t, numOfsubblocks);
	  	}
	}
  }
   
}

void EncodeComputeFast(encoder_t *EncoderObj){

  int i,j,m;
  sym_t *ssym_t, *esym_t; 
  
  //Number of subblocks:
  int numOfsubblocks = EncoderObj->sizet/sizeof(sym_t);
  
  // Precoding:
  if (EncoderObj->sizesb != EncoderObj->sizek){
    // Initialize
    for(j=EncoderObj->sizesb*numOfsubblocks; j< EncoderObj->sizek*numOfsubblocks; j++)
      EncoderObj->srcdata[j] = 0; // reset the srcdata tail (for LDPC coding).
    
    esym_t = &EncoderObj->srcdata[EncoderObj->sizesb*numOfsubblocks];
    for(i=EncoderObj->sizesb; i<EncoderObj->sizek; i++){
        for(j=0; j<EncoderObj->srcSymbolArray[i].deg; j++){
            ssym_t = &EncoderObj->srcdata[EncoderObj->srcSymbolArray[i].connections[j]*numOfsubblocks];
            align_XOR(esym_t, ssym_t, numOfsubblocks);
        }
        esym_t +=numOfsubblocks;
    }
  }
  
  for(j=0; j< EncoderObj->sizen*numOfsubblocks; j++)
     EncoderObj->encdata[j] = 0; // reset the encdata.
      
  //memset(EncoderObj->encdata, 0, EncoderObj->sizen*EncoderObj->sizet);
  // LT encoding here:
  esym_t = EncoderObj->encdata;
  for(i=0; i<EncoderObj->sizen; i++){
    //if (EncoderObj->encSymbolArray[i].deg == 1){
    //    ssym_t = &EncoderObj->srcdata[EncoderObj->encSymbolArray[i].connections[0]*numOfsubblocks];
    //    memcpy(esym_t, ssym_t, EncoderObj->sizet);
    //}else{
      for(j=0; j< EncoderObj->encSymbolArray[i].deg; j++){
        ssym_t = &EncoderObj->srcdata[EncoderObj->encSymbolArray[i].connections[j]*numOfsubblocks];
        align_XOR(esym_t, ssym_t, numOfsubblocks);
      }
    //}
    esym_t +=numOfsubblocks;
  }
}

void align_XOR(sym_t * __restrict a, sym_t * __restrict b, int size){
  int i;

  sym_t *x = (sym_t *)__builtin_assume_aligned(a,16);
  sym_t *y = (sym_t *)__builtin_assume_aligned(b,16);

  if(size %8 == 0){
    for (i=0; i< size/8; i++){		
      x[0] ^= y[0];
      x[1] ^= y[1];
      x[2] ^= y[2];
      x[3] ^= y[3];
      x[4] ^= y[4];
      x[5] ^= y[5];
      x[6] ^= y[6];
      x[7] ^= y[7];
      x += 8;
      y += 8;
    }
  }else{
    for (i=0; i< size; i++){
      x[0] ^= y[0];
      x += 1;
      y += 1;
    }
  }
  // other operations that can be parallel:
  //x[i] += ((x[i] > y[i]) ? 0 : y[i]);
  //x[i] = y[i] + y[i+4];
  //z += a[i] + b[i];
}

void EncodeComputeFast2(encoder_t *EncoderObj){
  // encode function for align_XOR2 below.
  int i,j;
  sym_t **ssym_t, *esym_t; 
  esym_t = EncoderObj->encdata;
  ssym_t = (sym_t **)malloc(sizeof(sym_t *)*EncoderObj->sizek);

  int numOfsubblocks = EncoderObj->sizet/sizeof(sym_t);
  for(i=0; i<EncoderObj->sizen; i++){
    for(j=0; j< EncoderObj->encSymbolArray[i].deg; j++)
        ssym_t[j] = &EncoderObj->srcdata[EncoderObj->encSymbolArray[i].connections[j]*numOfsubblocks];

    align_XOR2(esym_t, ssym_t, numOfsubblocks, EncoderObj->encSymbolArray[i].deg);
    esym_t +=numOfsubblocks;
  }
}


void align_XOR2(sym_t * __restrict a, sym_t ** __restrict b, int size, int deg){
  // This is the align_XOR function implemented in a bit fancier way.
  int i,j;

  sym_t *x = (sym_t *)__builtin_assume_aligned(a,16);
  sym_t *y;

  for (i=0; i< deg; i++){
    y = (sym_t *)__builtin_assume_aligned(b[i],16);
    for (j=0; j< size/8; j++){
      x[0] ^= y[0];
      x[1] ^= y[1];
      x[2] ^= y[2];
      x[3] ^= y[3];
      x[4] ^= y[4];
      x[5] ^= y[5];
      x[6] ^= y[6];
      x[7] ^= y[7];
      x +=8;
      y +=8;
    }
    x -= size;
  }

}




void SetDistribution(dist_t *Dist){

  int i;
  char *ch = 0;
  // Allocate memory for dist member cdf:	
  Dist->cdf =  (double *)malloc(sizeof(double)*Dist->maxdeg);
   
  if (strcmp(Dist->name, "FiniteDist") == 0 ){

    Dist->cdf[0] = 0;
    Dist->cdf[1] = Dist->cdf[0] + DIST1; 
    Dist->cdf[2] = Dist->cdf[1] + DIST2;
    Dist->cdf[3] = Dist->cdf[2] + DIST3;
    Dist->cdf[4] = Dist->cdf[3] + DIST4;	
    Dist->cdf[5] = Dist->cdf[4] + DIST5;
    Dist->cdf[6] = Dist->cdf[5];
    Dist->cdf[7] = Dist->cdf[6];
    Dist->cdf[8] = Dist->cdf[7] + DIST8;
    Dist->cdf[9] = Dist->cdf[8] + DIST9;
    for(i=10;i<19; i++) Dist->cdf[i] = Dist->cdf[9];
    Dist->cdf[19] = Dist->cdf[18] + DIST19; 
    for(i=20;i<64; i++) Dist->cdf[i] = Dist->cdf[19];
    Dist->cdf[64] = Dist->cdf[63] + DIST64;
    Dist->cdf[65] = Dist->cdf[64];
    Dist->cdf[66] = Dist->cdf[65] + DIST66;

  }else if (strcmp(Dist->name,"RSD") == 0){
    
    //Initialize RSD parameters
	#ifdef RSD
	    double gamma = RSD_GAMMA;
	    double cc    = RSD_CC;
	    double RR    = cc*log(Dist->maxdeg/gamma)*sqrt(Dist->maxdeg);
	#endif

    Dist->cdf[0] = 0;
    //Generate the Robust Soliton CDF:
    for(i=1; i<Dist->maxdeg; i++){
      if(i < (int)Dist->maxdeg/RR){
	if(i == 1){
	  Dist->cdf[i] = Dist->cdf[i-1] + ((1+RR)/(double)Dist->maxdeg);
	}else{
	  Dist->cdf[i] = Dist->cdf[i-1] + (1/(double)(i*(i-1))) + (RR/(double)(i*Dist->maxdeg));
	}
      }else if(i == (int)Dist->maxdeg/RR){
	Dist->cdf[i] = Dist->cdf[i-1] + (1/(double)(i*(i-1))) + (RR*log(RR/gamma)/(double)(Dist->maxdeg));
      }else{
	Dist->cdf[i] = Dist->cdf[i-1] + (1/(double)(i*(i-1)));
      }
    }
    // finally normalize the cdf:
    for(i=0; i<Dist->maxdeg; i++)
      Dist->cdf[i] /= Dist->cdf[Dist->maxdeg-1];

  }else if (strcmp(Dist->name,"OWN") == 0){
    //do somthing! - this part is yet to be defined. 
  }else{
    printf("No such distribution exists!\n");
    usage( ch );
    exit(0);
  }

}

int numofzeros(temp_conn_t *temp, int n){
    int i, zeros = 0;
    for(i=0; i<n; i++)
        if(temp->conn[i].deg == 0)
            zeros++;
    return zeros;
}

int* setXOR(int* connections_a, int deg_a, int* connections_b, int deg_b){
    int i,j,k,flag, len = deg_a;
    int* xorset = (int*) AllocObject(sizeof(int)*(deg_a+deg_b+1));
    if(xorset == NULL){
    	printf(ERRORMSG "Not enough MEMORY! Exiting... \n" RESET);
    	exit(0);
    }
    
    for(j=0; j<len; j++)
        xorset[j] = connections_a[j];
        
    for(i=0; i<deg_b; i++){
        flag = 1;
        for(j=0; j<len; j++){
            if(connections_b[i] == xorset[j]){
                for(k=j+1; k<len; k++)
                    xorset[k-1] = xorset[k];
                len--;
                flag = 0;
                break;
            }
        }
        if(flag){
            xorset[len] = connections_b[i];
            len++;
        }
    }
    xorset[deg_a+deg_b] = len;
    return xorset;
}

void* uniqueSET(int unique_set[], int* connections_a, int deg_a, int* connections_b, int deg_b){
    int i,j,flag, len = deg_a;
    /*if(sizeof(int)*(deg_a+deg_b+1) > 1000){
    	printf("size is %d\n", deg_a);
    	printf("size is %d\n", deg_b);
    }*/
    //int* unique_set = (int*) AllocObject(sizeof(int)*(deg_a+deg_b+1));


    //int* unique_set = (int*) malloc(sizeof(int)*(deg_a+deg_b+1));
    //int* unique_set = (int*) malloc(sizeof(int)*990);
    //printf("func in\n");
    for(j=0; j<len; j++)
        unique_set[j] = connections_a[j];
        
    for(i=0; i<deg_b; i++){
        flag = 1;
        for(j=0; j<len; j++){
            if(connections_b[i] == unique_set[j]){
                flag = 0;
                break;
            }
        }
        if(flag){
            unique_set[len] = connections_b[i];
            len++;
        }
    }
    unique_set[deg_a+deg_b] = len;
}



