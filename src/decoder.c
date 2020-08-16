/* *
 * Copyright (c) 2020, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * Decoder.c file for function definitions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "decoder.h"
#include "usage.h"
#include "parameter.h"

void *PrepareDec( decoder_t *codeword, dist_t *Dist, int *erased, int disks){
  // This function prepares the decoder graph, degrees, connections, etc. 
  double temp;
  int i, j, k, cnt, temp0;
  int kk, jj, prim, ind;
  int blocksize = codeword->sizen/disks;
  
  if (codeword->sizesb != codeword->sizek){
    prim = LargestPrimeFactor(codeword->sizek);
    kk = codeword->sizek/prim;
    jj = kk - codeword->sizesb/prim;
    
    for(j=0;j<jj;j++){
      for(k=0;k<prim;k++){
        cnt = 1;
        codeword->srcSymbolArray[codeword->sizesb + k + j*prim].deg = kk - jj + j;
        codeword->srcSymbolArray[codeword->sizesb + k + j*prim].connections = (int *)malloc(sizeof(int *)*(kk-jj+j));

        for(i=1;i<kk-jj+j+1;i++){
          ind = kk*prim - (jj - j + cnt - 1)*prim - (cnt*(jj - j - 1) - k - 1 + prim)%prim - 1;
          codeword->srcSymbolArray[codeword->sizesb + k + j*prim].connections[i-1] = ind;
          cnt ++;
        }
      }
    }
  
  }
  
  //srand(codeword->encSeed);	
  for(j=0; j< codeword->sizen; j++){
    //For future:
    //For fast mode we generate random edge connections with replacement at exp of some perf loss.
    //for slow mode we generate random edge connections without replacement
    // Set the ID of the symbol:
    //codeword->encSymbolArray[j].ID = j*100;
    // fancy random seed initializations -->
    
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
    
    // INSERT the erasure information:
    if(erased[j/blocksize] == 1){
        codeword->encSymbolArray[j].avail = 0;
    }else{
        codeword->encSymbolArray[j].avail = 1;
    }

    
  }

  return codeword;
}

void *runBPNoData(decoder_t *codeword){

    int i, j, cont, temp, NofUnrec;
    // Initialize the availability info for source blocks: We initialize it before each decoding process.
    for(j=0; j< codeword->sizek; j++)
        codeword->srcSymbolArray[j].avail = 0;
    codeword->success = 0;
    int CumRipplesize = 0;
    do{
        cont = 0;
        for(i = 0; i<codeword->sizen; i++){
            if(codeword->encSymbolArray[i].avail == 1){
                NofUnrec = 0;
                for(j=0; j<codeword->encSymbolArray[i].deg; j++){
                    if(codeword->srcSymbolArray[codeword->encSymbolArray[i].connections[j]].avail == 0){
                        NofUnrec++;
                        temp = codeword->encSymbolArray[i].connections[j];   
                    }
                    if(NofUnrec > 1)
                        break;
                }
                if(NofUnrec == 1){
                    codeword->srcSymbolArray[temp].avail = 1; 
                    CumRipplesize++;
                    cont = 1;
                }
            }
        }
        if (CumRipplesize == codeword->sizek){
            codeword->success = 1;
            codeword->unrecovered = 0;
            return codeword;
        }
    }while(cont);

    codeword->unrecovered = codeword->sizek - CumRipplesize;
    return codeword;

}

void *runBP( decoder_t *codeword){

    int cont, temp, i, j;
    int NofUnrec;
    int CumRipplesize = 0;
    sym_t *ssym_t, *esym_t;
    int numOfsubblocks = codeword->sizet/sizeof(sym_t); // how many sym_t's per coding symbol.
    
    // Initialize the availability info for source blocks: We initialize it before each decoding process.
    for(j=0; j< codeword->sizek; j++)
        codeword->srcSymbolArray[j].avail = 0;
    for(j=0; j< codeword->sizek*numOfsubblocks; j++)
        codeword->srcdata[j] = 0; // reset the erased source data.
    codeword->success = 0; // initialize success to 0. 
    
    do{
        cont = 0;
        for(i=0; i<codeword->sizen; i++){
            if(codeword->encSymbolArray[i].avail == 1){ // Use only available coded symbols
                NofUnrec = 0;
                for(j=0; j<codeword->encSymbolArray[i].deg; j++){
                    if(codeword->srcSymbolArray[codeword->encSymbolArray[i].connections[j]].avail == 0){
                        NofUnrec++;
                        temp = codeword->encSymbolArray[i].connections[j];   
                    }
                    if(NofUnrec > 1)
                        break;
                }
                if(NofUnrec == 1 && codeword->srcSymbolArray[temp].avail == 0){
                    codeword->srcSymbolArray[temp].avail = 1; 
                    CumRipplesize++;
                    esym_t = &codeword->srcdata[temp*numOfsubblocks];
                    for(j=0; j< codeword->encSymbolArray[i].deg; j++){
                        if(codeword->encSymbolArray[i].connections[j] != temp){
                            ssym_t = &codeword->srcdata[codeword->encSymbolArray[i].connections[j]*numOfsubblocks];
                            align_XOR(esym_t, ssym_t, numOfsubblocks);
                        }
                    }
                    ssym_t = &codeword->encdata[i*numOfsubblocks]; // Finally add the available encoded data value.
                    align_XOR(esym_t, ssym_t, numOfsubblocks);
                    /* code here the real data decoding using fast XOR */
                    cont = 1;
                }
            }
        }
        //printf("cumsize is %d\n", CumRipplesize);
        if (CumRipplesize == codeword->sizek){
            codeword->success = 1;
            codeword->unrecovered = 0;
            return codeword;
        }
    }while(cont);
    
    codeword->unrecovered = codeword->sizek - CumRipplesize;
    return codeword;
}

void *prepare_decoding_path_mt(decoder_t *codeword){
	
	int *decoding_path, *d1c, *sdec; 
	int cont, idx, i, j, temp, NofUnrec, total_sum = 0; 
	int CumRipplesize = 0;
	
	for(j=0; j< codeword->sizek; j++)
		codeword->srcSymbolArray[j].avail = 0;  
	
	decoding_path = (int *)malloc(sizeof(int));
	do{
		cont = 0; idx = 0; 
		d1c = (int *)malloc(sizeof(int));
		sdec = (int *)malloc(sizeof(int));

		for(i=0; i<codeword->sizen; i++){
			if(codeword->encSymbolArray[i].avail == 1){ // only available symbols are used. 
				NofUnrec = 0;
				for(j=0; j<codeword->encSymbolArray[i].deg; j++){ // find degree-one symbol that can decode.
				    if(codeword->srcSymbolArray[codeword->encSymbolArray[i].connections[j]].avail == 0){
				        NofUnrec++;
				        if (NofUnrec > 1) break;
				        temp = codeword->encSymbolArray[i].connections[j]; // src symbol to be decoded. 
				    }       
				}
				if(NofUnrec == 1){ 
					NofUnrec = 0; // We reuse this parameter to find multiple same-source-block-index attempted decoding
					for(j=0;j<idx;j++){
						if(sdec[j] == temp){
							NofUnrec = 1;
							if(codeword->encSymbolArray[i].deg < codeword->encSymbolArray[d1c[j]].deg)
								d1c[j] = i;
							break;
						}
					}	
					if(NofUnrec == 0){ // if you do not find anything in sdec, then create new entry. 
						d1c[idx] = i;
						sdec[idx] = temp;
						idx++;
						d1c = (int *)realloc(d1c, (idx+1)*sizeof(int));
						sdec = (int *)realloc(sdec, (idx+1)*sizeof(int));
					}
					cont = 1; // flag to indicate that some symbols are decoded. 
				}
			}
		}
		
		total_sum += (2*idx+1);	
		CumRipplesize += idx;
		decoding_path = (int *)realloc(decoding_path, total_sum*sizeof(int));
		decoding_path[total_sum - 2*idx - 1] = idx;
		for(i=0; i<idx; i++){
			codeword->srcSymbolArray[sdec[i]].avail++;
			decoding_path[total_sum - 2*idx + i] = d1c[i]; // first entry is degree-1 coded symbol
			decoding_path[total_sum - idx + i] = sdec[i]; // second entry is decoded source symbol.
		}
		
		
		free(d1c);
        free(sdec);
        if (CumRipplesize == codeword->sizek){
            codeword->success = 1;
            codeword->unrecovered = 0;
            break;
        }
	
	}while(cont);
	
	if(CumRipplesize != codeword->sizek && codeword->sizesb == codeword->sizek){
		printf(ERRORMSG "Error:  Data cannot be resolved. Not enough coding blocks are found.\n" RESET);
		exit(0);
	}
	if(CumRipplesize < codeword->sizek){
		codeword->success = 0;
        codeword->unrecovered = codeword->sizek-CumRipplesize;
	}

	return decoding_path;
}

void *runBP_mt_advance(decoder_t *codeword, int numOfThreads, int *decoding_path){
	
	int i, j, idx, iter,d1c, sdec, total = 0;
	int numOfsubblocks = codeword->sizet/sizeof(sym_t); // how many sym_t's per coding symbol.
	sym_t *ssym_t, *esym_t;
	
	
	for(j=0; j< codeword->sizek; j++)
		codeword->srcSymbolArray[j].avail = 0; 
	for(j=0; j< codeword->sizek*numOfsubblocks; j++)
		codeword->srcdata[j] = 0; // reset the erased source data.
	omp_set_dynamic(0);
    omp_set_num_threads(numOfThreads);
	iter = 0;
	
    while(total < codeword->sizek-codeword->unrecovered){
    	idx = decoding_path[total*2+iter];
		#pragma omp parallel for shared(codeword, numOfsubblocks, decoding_path) private(j,esym_t, ssym_t, d1c, sdec) schedule(dynamic,1)
		for(i=0; i<idx; i++){	 
			d1c = decoding_path[(total*2+iter)+1+i];
			sdec = decoding_path[(total*2+iter)+1+idx+i];
			codeword->srcSymbolArray[sdec].avail++;
			esym_t = &codeword->srcdata[sdec*numOfsubblocks];
			for(j=0; j< codeword->encSymbolArray[d1c].deg; j++){			    	
				if(codeword->encSymbolArray[d1c].connections[j] != sdec){
				    ssym_t = &codeword->srcdata[codeword->encSymbolArray[d1c].connections[j]*numOfsubblocks];
				    align_XOR(esym_t, ssym_t, numOfsubblocks);
				}
			}
			ssym_t = &codeword->encdata[d1c*numOfsubblocks]; // Finally add the available encoded data value.
			align_XOR(esym_t, ssym_t, numOfsubblocks);
		}
		total += idx;
		iter++;
	}
	
	if(total != codeword->sizek){
		codeword->success = 0;
	}else{
		codeword->success = 1;
	}
		
	//codeword->success = 1;
	//codeword->unrecovered = 0;
    return codeword;

}

void *runBP_mt(decoder_t *codeword, int numOfThreads){

    int cont, idx, temp, i, j;
    int NofUnrec;
    int *d1c, *sdec; // d1c: degree-1 coded symbols, sdec: source symbols to be decoded. 
    int CumRipplesize = 0;
    sym_t *ssym_t, *esym_t;
    int numOfsubblocks = codeword->sizet/sizeof(sym_t); // how many sym_t's per coding symbol.
    
    // Initialize the availability info for source blocks: We initialize it before each decoding process.
    for(j=0; j< codeword->sizek; j++)
        codeword->srcSymbolArray[j].avail = 0;
    for(j=0; j< codeword->sizek*numOfsubblocks; j++)
        codeword->srcdata[j] = 0; // reset the erased source data.
    codeword->success = 0; // initialize success to 0. 
    
    //printf("++++++++++++++++++++++++++++++++++++++\n");
    omp_set_dynamic(0);
    omp_set_num_threads(numOfThreads);
    
    do{
    	cont = 0; // continue flag. If = 1, continue, stop otherwise. 
        idx = 0;
        d1c = (int *)malloc(sizeof(int));
    	sdec = (int *)malloc(sizeof(int));
        
        for(i=0; i<codeword->sizen; i++){
        	if(codeword->encSymbolArray[i].avail == 1){ // only available symbols are used. 
		        NofUnrec = 0;
		        for(j=0; j<codeword->encSymbolArray[i].deg; j++){ // find degree-one symbol that can decode.
		            if(codeword->srcSymbolArray[codeword->encSymbolArray[i].connections[j]].avail == 0){
		                NofUnrec++;
		                if (NofUnrec > 1) break;
		                temp = codeword->encSymbolArray[i].connections[j]; // src symbol to be decoded. 
		            }       
		        }
		    	//if(NofUnrec == 1 && codeword->srcSymbolArray[temp].avail == 0){ 
		    	if(NofUnrec == 1){ 
		    		NofUnrec = 0; // We reuse this parameter to find multiple same-source-block-index attempted decoding
		    		for(j=0;j<idx;j++){
		    			if(sdec[j] == temp){
		    				NofUnrec = 1;
		    				if(codeword->encSymbolArray[i].deg < codeword->encSymbolArray[d1c[j]].deg)
		    					d1c[j] = i;
		    				break;
		    			}
		    		}	
		    		if(NofUnrec == 0){ // if you do not find anything in sdec, then create new entry. 
		    			d1c[idx] = i;
		    			sdec[idx] = temp;
						idx++;
						d1c = (int *)realloc(d1c, (idx+1)*sizeof(int));
						sdec = (int *)realloc(sdec, (idx+1)*sizeof(int));
					}
		    		cont = 1; // flag to indicate that some symbols are decoded. 
		    	}
        	}
        }
        
        
        for(i=0; i<idx; i++)
        	codeword->srcSymbolArray[sdec[i]].avail++;
        
        if(cont == 1){ // Only execute if there is some decoded symbols for a given iteration. 
		    #pragma omp parallel for shared(codeword, numOfsubblocks, sdec, d1c) private(j,esym_t, ssym_t) reduction(+:CumRipplesize) schedule(dynamic,1)
		    for(i=0; i<idx; i++){	 
		    	//#pragma omp critical	
		    	//codeword->srcSymbolArray[sdec[i]].avail++;
		    	//if(codeword->srcSymbolArray[sdec[i]].avail == 1){
	    		esym_t = &codeword->srcdata[sdec[i]*numOfsubblocks];
				for(j=0; j< codeword->encSymbolArray[d1c[i]].deg; j++){			    	
				    if(codeword->encSymbolArray[d1c[i]].connections[j] != sdec[i]){
				        ssym_t = &codeword->srcdata[codeword->encSymbolArray[d1c[i]].connections[j]*numOfsubblocks];
				        align_XOR(esym_t, ssym_t, numOfsubblocks);
				    }
				}
				ssym_t = &codeword->encdata[d1c[i]*numOfsubblocks]; // Finally add the available encoded data value.
				align_XOR(esym_t, ssym_t, numOfsubblocks);
				CumRipplesize++;
		    	//}
		    }
        }
        //exit(0);
        //for(i=0; i<idx; i++)
        //	codeword->srcSymbolArray[sdec[i]].avail++;
        	
		//printf("cumRipplesize: %d\n", CumRipplesize);
        free(d1c);
        free(sdec);
        if (CumRipplesize == codeword->sizek){
            codeword->success = 1;
            codeword->unrecovered = 0;
            return codeword;
        }
    }while(cont);
    
    codeword->unrecovered = codeword->sizek - CumRipplesize;
    return codeword;
}

void *runBP4PrecodeNoData( decoder_t *codeword){

    int cont, temp, i, j;
    int NofUnrec;
    int CumRipplesize = codeword->sizek - codeword->unrecovered;
    codeword->success = 0;
    
    do{
        cont = 0;
        for(i=codeword->sizesb; i< codeword->sizek; i++){
            NofUnrec = 0;
            if(codeword->srcSymbolArray[i].avail == 0){
                NofUnrec++;
                temp = i;
            }
            for(j=0; j<codeword->srcSymbolArray[i].deg; j++){
                if(codeword->srcSymbolArray[codeword->srcSymbolArray[i].connections[j]].avail == 0){
                    NofUnrec++;
                    temp = codeword->srcSymbolArray[i].connections[j];   
                }
                if(NofUnrec > 1)
                    break;
            }
            if(NofUnrec == 1){
                codeword->srcSymbolArray[temp].avail = 1; 
                CumRipplesize++;
                cont = 1;
            }
        }
        if (CumRipplesize == codeword->sizek){
            codeword->success = 1;
            codeword->unrecovered = 0;
            return codeword;
        }
    }while(cont);

    codeword->unrecovered = codeword->sizek - CumRipplesize;
    return codeword;    
}

void *runBP4Precode( decoder_t *codeword){


    int cont, temp, i, j;
    int NofUnrec;
    sym_t *ssym_t, *esym_t;
    int CumRipplesize = codeword->sizek - codeword->unrecovered;
    int numOfsubblocks = codeword->sizet/sizeof(sym_t); // how many sym_t's per coding symbol.
    codeword->success = 0;
    
    do{
        cont = 0;
        //printf("CumReipplesize = %d\n", CumRipplesize);
        for(i=codeword->sizesb; i< codeword->sizek; i++){
            NofUnrec = 0;
            if(codeword->srcSymbolArray[i].avail == 0){
                NofUnrec++;
                temp = i;
            }
            for(j=0; j<codeword->srcSymbolArray[i].deg; j++){
                if(codeword->srcSymbolArray[codeword->srcSymbolArray[i].connections[j]].avail == 0){
                    NofUnrec++;
                    temp = codeword->srcSymbolArray[i].connections[j];   
                }
                if(NofUnrec > 1)
                    break;
            }
            if(NofUnrec == 1){
            	//printf("temp: %d, %d, %d\n", temp, numOfsubblocks, codeword->srcSymbolArray[temp].avail);
                codeword->srcSymbolArray[temp].avail = 1; 
                CumRipplesize++;
                //printf("%d :" "%" PRIx64 "\n",i, codeword->srcdata[temp*numOfsubblocks]);
                esym_t = &codeword->srcdata[temp*numOfsubblocks];
                for(j=0; j< codeword->srcSymbolArray[i].deg; j++){
                    if(codeword->srcSymbolArray[i].connections[j] != temp){
                        ssym_t = &codeword->srcdata[codeword->srcSymbolArray[i].connections[j]*numOfsubblocks];
                        align_XOR(esym_t, ssym_t, numOfsubblocks);
                    }
                }
                if (i != temp){
                    ssym_t = &codeword->srcdata[i*numOfsubblocks];
                    align_XOR(esym_t, ssym_t, numOfsubblocks);
                }
                /* code here the real data decoding using fast XOR */
                cont = 1;
            }
            if (CumRipplesize == codeword->sizek)
            	break;
        }
        //printf("CumRipplesize = %d\n", CumRipplesize);
        if (CumRipplesize == codeword->sizek){
            codeword->success = 1;
            codeword->unrecovered = 0;
            return codeword;
        }
    }while(cont);

    codeword->unrecovered = codeword->sizek - CumRipplesize;
    return codeword;
}
