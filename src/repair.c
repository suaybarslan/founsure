/* *
 * Copyright (c) 2020, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * Repair.c file for function definitions.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
 
#include "usage.h"
#include "parameter.h"
#include "encoder.h"
#include "repair.h"
#include "allocate.h"


int CmpFunc(const void *elem1, const void *elem2){
	sort_t *i1, *i2;
	i1 = (sort_t*)elem1; i2 = (sort_t*)elem2;
	return i1->val - i2->val;
}

void *AllocAndPrepareRep4Enc( repair_t *repairObj, encoder_t *encoderObj, char *checkfname, int IntArraySize){
	
	int i, j, k, cnt3;
	uint32_t* content2read = (uint32_t *) AllocObject(sizeof(uint32_t)*IntArraySize);
	FILE *ifp = fopen(checkfname, "rb"); 
	
	if (ifp == NULL){
		printf(WARNINGMSG "Warning: The file does not exist. \n" RESET);
		printf(COMMENT "Comment: Reverting back to conventional repair. \n" RESET);
		repairObj->fast_mode = false;
	}else{
		if(fread(content2read, sizeof(int), IntArraySize, ifp) != IntArraySize){  	
    		printf(WARNINGMSG "Warning: Either The file does NOT have enough information. \n" RESET);
    		printf(COMMENT "Comment: Reverting back to conventional repair. \n" RESET);
			repairObj->fast_mode = false;
   		}
		fclose(ifp);
	}
    
    
    if(repairObj->fast_mode == true){
    	
		// Initialize the internal values of the encoder encoded objects
		repairObj->check2 = (symbol_sd *)malloc(sizeof(symbol_sd)*repairObj->sizeCheck2); 
		// Initialize the internal values of the encoder source symbols
		repairObj->check3 = (symbol_sd *)malloc(sizeof(symbol_sd)*repairObj->sizeCheck3);	
		
		sort_t temp[encoderObj->sizen - encoderObj->sizek];
		if(ORDER_CHECK_2){
			i = 0; cnt3 = 0;
			while (i < IntArraySize){
				if(content2read[i] == 0){
					i++;
					temp[cnt3].val = content2read[i];
					temp[cnt3].ind = i;
					for (j=i+1; j< temp[cnt3].val+i+1; j++);
					i = j;	
					cnt3++;			
				}else{
					i++;
				}
			}	
			
			qsort(temp, encoderObj->sizen - encoderObj->sizek, sizeof(sort_t), CmpFunc);
			
			/*for(i=0; i<encoderObj->sizen - encoderObj->sizek; i++)
				printf(" %d", temp[i].val);
			printf("\n");*/
		}else{
			for(i=0; i<encoderObj->sizen - encoderObj->sizek; i++)
				temp[i].ind = i;
		}
						
		int *xorset;
		int ind, deg, cnt2;
		i = 0; k = 0; cnt2 = 0; cnt3 = 0; 
		while (i < IntArraySize && k <  encoderObj->sizen){
			if (content2read[i] == 0){ // check 3
				//printf("%d: ", cnt++);
				i++;k++;
				deg = content2read[temp[cnt3].ind];
				repairObj->check3[cnt3].connections = (int *) malloc(sizeof(int)*deg);
				repairObj->check3[cnt3].deg = deg;
				//printf("%d ", deg);
				for (j=temp[cnt3].ind+1; j<deg+temp[cnt3].ind+1;j++){
					//printf("%d ", content2read[j]);
					repairObj->check3[cnt3].connections[j-temp[cnt3].ind-1] = content2read[j];
				}
				
				for (j=i+1; j<content2read[i]+i+1;j++);
				i = j;
				//printf("\n");
				cnt3++;
				
			}else if(content2read[i] == 1){ // check 2
				//printf("%d %d ", content2read[i], content2read[i+1]);
				i++;
				cnt2 = content2read[i];
				i++;k++;
				deg = content2read[i];
				repairObj->check2[cnt2].connections = (int *) malloc(sizeof(int )*deg);
				repairObj->check2[cnt2].deg = deg;
				//printf("%d ", deg);
				for (j=i+1; j<deg+i+1;j++){
					//printf("%d ", content2read[j]);
					repairObj->check2[cnt2].connections[j-i-1] = content2read[j];
				}
				i = j;	
				//printf("\n");	
			}else{
				i++;k++;
				deg = content2read[i];
				//printf("%d ", deg);
				for (j=i+1; j< deg+i+1; j++);
					//printf("%d ", content2read[j]);
				i = j;
			}	
			//printf("i:%d\n", i);	
		}
		//printf("cnt3: %d\n", repairObj->sizeCheck3);
		
		//for(i=0; i<10; i++)
		//	printf(" %d \n", repairObj->check3[i].deg);
		
		
		// compute extra check3 type nodes.
		for (i=cnt3;i<repairObj->sizeCheck3;i++){
			ind = i-cnt3+encoderObj->sizesb; //index for srcSymbolArray.
			//repairObj->check3[cnt3].connections = (int *) malloc(sizeof(int)*);
			
			// Initialize:
			repairObj->check3[i].deg = repairObj->check2[ind].deg;
			repairObj->check3[i].connections = (int *) malloc(sizeof(int)*repairObj->check3[i].deg);
		
			for(k=0; k<repairObj->check3[i].deg; k++){
				repairObj->check3[i].connections[k] = repairObj->check2[ind].connections[k];
				//printf(" %d ", repairObj->check2[ind].connections[k]);
			}
			//printf("\n");
			//memcpy(repairObj->check3[i].connections, 
			//		repairObj->check2[encoderObj->srcSymbolArray[ind].connections[0]].connections,
			//		repairObj->check3[i].deg * sizeof(int));
			for(j=0; j<encoderObj->srcSymbolArray[ind].deg; j++){
				deg = repairObj->check3[i].deg + 
						repairObj->check2[encoderObj->srcSymbolArray[ind].connections[j]].deg;
				xorset = setXOR(repairObj->check3[i].connections,
		        				repairObj->check3[i].deg,
		        				repairObj->check2[encoderObj->srcSymbolArray[ind].connections[j]].connections,
		        				repairObj->check2[encoderObj->srcSymbolArray[ind].connections[j]].deg);
				repairObj->check3[i].deg = xorset[deg];
				// reallocate memory:
				repairObj->check3[i].connections = (int*)realloc(repairObj->check3[i].connections, sizeof(int)*repairObj->check3[i].deg);
				for(k=0; k<repairObj->check3[i].deg; k++) // compy new result.
					repairObj->check3[i].connections[k] = xorset[k];
				free(xorset);
				//printf(" %d ", encoderObj->srcSymbolArray[ind].connections[j]);
			}	
		}
		
		free(content2read);
	}	

	return repairObj;
 
}
 
void *AllocAndPrepareRep4Dec( repair_t *repairObj, decoder_t *decoderObj, char *checkfname, int IntArraySize){
	
	int i, j, k, cnt3;
	uint32_t* content2read = (uint32_t *) AllocObject(sizeof(uint32_t)*IntArraySize);
	FILE *ifp = fopen(checkfname, "rb"); 
	
	if (ifp == NULL){
		printf(WARNINGMSG "Warning: The file does not exist. \n" RESET);
		printf(COMMENT "Comment: Reverting back to conventional repair. \n" RESET);
		repairObj->fast_mode = false;
	}else{
		if(fread(content2read, sizeof(int), IntArraySize, ifp) != IntArraySize){  	
    		printf(WARNINGMSG "Warning: Either The file does NOT have enough information. \n" RESET);
    		printf(COMMENT "Comment: Reverting back to conventional repair. \n" RESET);
			repairObj->fast_mode = false;
   		}
		fclose(ifp);
	}  
    
    if(repairObj->fast_mode == true){
    	
		// Initialize the internal values of the encoder encoded objects
		repairObj->check2 = (symbol_sd *)malloc(sizeof(symbol_sd)*repairObj->sizeCheck2); 
		// Initialize the internal values of the encoder source symbols
		repairObj->check3 = (symbol_sd *)malloc(sizeof(symbol_sd)*repairObj->sizeCheck3);	
		
		sort_t temp[decoderObj->sizen - decoderObj->sizek];
		i = 0; cnt3 = 0;
		while (i < IntArraySize){
			if(content2read[i] == 0){
				i++;
				temp[cnt3].val = content2read[i];
				temp[cnt3].ind = i;
				for (j=i+1; j< temp[cnt3].val+i+1; j++);
				i = j;	
				cnt3++;		
			}else if(content2read[i] == 1){
				i++;i++;
				for (j=i+1; j< content2read[i]+i+1; j++);
				i = j;
			}else{
				i++;
				for (j=i+1; j< content2read[i]+i+1; j++);
				i = j;
			}
		}


		if(ORDER_CHECK_3)
			qsort(temp, decoderObj->sizen - decoderObj->sizek, sizeof(sort_t), CmpFunc);
		
					
		int *xorset;
		int ind, deg, cnt2;
		i = 0; k = 0; cnt2 = 0; cnt3 = 0; 
		while (i < IntArraySize && k <  decoderObj->sizen){
			 
			if (content2read[i] == 0){ // check 3	
				i++;k++;
				//printf(" - - %d \n", temp[cnt3].ind);
				deg = content2read[temp[cnt3].ind];
				
				repairObj->check3[cnt3].connections = (int *) malloc(sizeof(int)*deg);
				repairObj->check3[cnt3].deg = deg;
				//printf("%d ", deg);
				for (j=temp[cnt3].ind+1; j<deg+temp[cnt3].ind+1;j++){
					//printf("%d ", content2read[j]);
					repairObj->check3[cnt3].connections[j-temp[cnt3].ind-1] = content2read[j];
				}
			
				for (j=i+1; j<content2read[i]+i+1;j++);
				i = j;
				cnt3++;	
				
			}else if(content2read[i] == 1){ // check 2
				//printf("%d %d ", content2read[i], content2read[i+1]);
				i++;
				cnt2 = content2read[i];
				i++;k++;
				deg = content2read[i];
				repairObj->check2[cnt2].connections = (int *) malloc(sizeof(int )*deg);
				repairObj->check2[cnt2].deg = deg;
				//printf("%d ", deg);
				for (j=i+1; j<deg+i+1;j++){
					//printf("%d ", content2read[j]);
					repairObj->check2[cnt2].connections[j-i-1] = content2read[j];
				}
				i = j;	
				//printf("\n");	
			}else{
				i++;k++;
				deg = content2read[i];
				//printf("%d ", deg);
				for (j=i+1; j< deg+i+1; j++);
					//printf("%d ", content2read[j]);
				i = j;
			}	
			//printf("i:%d\n", i);	
		}
		//printf("cnt3: %d\n", repairObj->sizeCheck3);
		
		
		// compute extra check3 type nodes.
		for (i=cnt3;i<repairObj->sizeCheck3;i++){
			ind = i-cnt3+decoderObj->sizesb; //index for srcSymbolArray.
			//repairObj->check3[cnt3].connections = (int *) malloc(sizeof(int)*);
			
			// Initialize:
			repairObj->check3[i].deg = repairObj->check2[ind].deg;
			repairObj->check3[i].connections = (int *) malloc(sizeof(int)*repairObj->check3[i].deg);
		
			for(k=0; k<repairObj->check3[i].deg; k++){
				repairObj->check3[i].connections[k] = repairObj->check2[ind].connections[k];
				//printf(" %d ", repairObj->check2[ind].connections[k]);
			}
			//printf("\n");
			//memcpy(repairObj->check3[i].connections, 
			//		repairObj->check2[decoderObj->srcSymbolArray[ind].connections[0]].connections,
			//		repairObj->check3[i].deg * sizeof(int));
			for(j=0; j<decoderObj->srcSymbolArray[ind].deg; j++){
				deg = repairObj->check3[i].deg + 
						repairObj->check2[decoderObj->srcSymbolArray[ind].connections[j]].deg;
				xorset = setXOR(repairObj->check3[i].connections,
		        				repairObj->check3[i].deg,
		        				repairObj->check2[decoderObj->srcSymbolArray[ind].connections[j]].connections,
		        				repairObj->check2[decoderObj->srcSymbolArray[ind].connections[j]].deg);
				repairObj->check3[i].deg = xorset[deg];
				// reallocate memory:
				repairObj->check3[i].connections = (int*)realloc(repairObj->check3[i].connections, sizeof(int)*repairObj->check3[i].deg);
				for(k=0; k<repairObj->check3[i].deg; k++) // compy new result.
					repairObj->check3[i].connections[k] = xorset[k];
				free(xorset);
				//printf(" %d ", decoderObj->srcSymbolArray[ind].connections[j]);
			}	
		}
		
		free(content2read);
	}	

	return repairObj;
 
}

void check_repair(repair_t *repairword, decoder_t *codeword){

	int i,j, k, codedsym;
	int *xorset;
	int deg;
	int *temp;
	
	for(i=0; i<repairword->sizeCheck3; i++){
		codedsym = repairword->check3[i].connections[0];
		
		temp = (int *)malloc(codeword->encSymbolArray[codedsym].deg*sizeof(int));
		deg = codeword->encSymbolArray[codedsym].deg;
		for(j=0; j<codeword->encSymbolArray[codedsym].deg; j++)
			temp[j] = codeword->encSymbolArray[codedsym].connections[j];
		
		for(j=1; j<repairword->check3[i].deg; j++){
		
			codedsym = repairword->check3[i].connections[j];
			xorset = setXOR(temp, deg,
            				codeword->encSymbolArray[codedsym].connections,
            				codeword->encSymbolArray[codedsym].deg);
            				
            deg = xorset[deg+codeword->encSymbolArray[codedsym].deg];
            temp = (int *)realloc(temp, sizeof(int)*deg);
            for(k=0;k<deg; k++)
           		temp[k] = xorset[k];
           	free(xorset);
		}		
	}
}

void *need2AccessDisks(repair_t *repairword, decoder_t *codeword, int numErased, int bs, int *access_freq){

	int i, j, cont, temp, NofUnrec = 0; 
	repairword->success = 0;
	int repairedsyms = 0, deg = 0;
	int *xorset = 0;
	//int disks = codeword->sizen/bs;
	//printf("numErased: %d, %d\n", numErased, disks);

	if(repairword->fast_mode == true){
		do{
			cont = 0;
			for(i = 0; i<repairword->sizeCheck3; i++){
				NofUnrec = 0; temp = 0;
				for(j = 0; j<repairword->check3[i].deg; j++){
					if(codeword->encSymbolArray[repairword->check3[i].connections[j]].avail == 0){
		        		NofUnrec++;
		        		temp = repairword->check3[i].connections[j];   
		        	}
		        	if(NofUnrec > 1)
		        		break;
				}
		        if(NofUnrec == 1){
		            codeword->encSymbolArray[temp].avail = 1; 
		            repairedsyms++;
		            cont = 1;
		            
					xorset = setXOR(xorset, deg,
		        					repairword->check3[i].connections,
		        					repairword->check3[i].deg);
		        	deg = xorset[deg + repairword->check3[i].deg];
		        	                
		        	if (repairedsyms == numErased){
		        		repairword->success = 1;
		        		
		        		for(i=0; i<deg; i++){ // BW of all transfered data (no computations at nodes).
							access_freq[xorset[i]/bs]++; //bs: number of syms per disk.
						}
							        		
		        		return codeword;
		    		}
		    		goto exitloop;
		        }
			}
		exitloop: ;
		}while(cont);	
	}
	
	if(repairword->fast_mode == false || repairword->success == 0){
		for(j=0; j< codeword->sizek; j++)
        	codeword->srcSymbolArray[j].avail = 0;
		codeword->success = 0;
		int CumRipplesize = 0;
		int *xorset2 = (int *)AllocObject(sizeof(int));
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
		                xorset2[0] = i;
						xorset = setXOR(xorset, deg, xorset2, 1);
				    	deg = xorset[deg + 1];  
		            }
		        }
		    }
		    if (CumRipplesize == codeword->sizek){
		        codeword->success = 1;
		        codeword->unrecovered = 0;
		        
		        for(i=0; i<deg; i++)
					access_freq[xorset[i]/bs]++;
		        
		        return codeword;
		    }
		}while(cont);
		free(xorset2);
		codeword->unrecovered = codeword->sizek - CumRipplesize;
		if(codeword->success == 0)
			codeword = (decoder_t *)runBP4Precode(codeword);
		if(codeword->success == 1){
			for(i=0; i<deg; i++)
				access_freq[xorset[i]/bs]++;
			return codeword;
		}else{
			printf(ERRORMSG "Error: Decoding/Repair is Unsuccessful! | %d unrecovered symbols.\n" RESET,codeword->unrecovered); 
			exit(0);
		}	
	}	
	return codeword;
}

void *runRepairNoData(repair_t *repairword, decoder_t *codeword, int NumErased){
	
	int i, j, cont, temp, NofUnrec = 0; 
	//int srcsym, k, deg1n = 0;
	//printf("check3: %d\n", repairword->sizeCheck3);
	repairword->success = 0;
	int repairedsyms = 0;
	//int *degree1s = (int *)AllocObject(sizeof(int));
	
	/*for(k = 0; k<codeword->sizen; k++){
		if(codeword->encSymbolArray[k].deg == 1){
			degree1s[deg1n] = k; deg1n++;
			degree1s = (int*)realloc(degree1s,sizeof(int)*deg1n);	
		}
	}
	for(k=0; k<deg1n; k++)
		printf("%d ", degree1s[k]);
	printf("\n");*/

	do{
		cont = 0;
		for(i = 0; i<repairword->sizeCheck3; i++){
			NofUnrec = 0; temp = 0;
			//printf(" %d ", repairword->check3[i].deg);
			for(j = 0; j<repairword->check3[i].deg; j++){
				if(codeword->encSymbolArray[repairword->check3[i].connections[j]].avail == 0){
            		NofUnrec++;
            		temp = repairword->check3[i].connections[j];   
            	}
            	if(NofUnrec > 1)
            		break;
			}
            if(NofUnrec == 1){
                codeword->encSymbolArray[temp].avail = 1; 
                repairedsyms++;
                cont = 1;
            	if (repairedsyms == NumErased){
            		repairword->success = 1;
            		return codeword;
        		}
        		goto exitloop;
            }
		}	
		/*if (cont == 0){
			printf("here\n");
			for(k=0; k<deg1n; k++){
				(codeword->encSymbolArray[degree1s[k]].avail == 0) ? (NofUnrec = 1) : (NofUnrec = 0);
				if (NofUnrec == 1)
					temp = degree1s[k];
				srcsym = codeword->encSymbolArray[degree1s[k]].connections[0];
				for(j = 0; j < repairword->check2[srcsym].deg; j++){
					if(codeword->encSymbolArray[repairword->check2[srcsym].connections[j]].avail == 0){
						NofUnrec++;
						temp = repairword->check2[srcsym].connections[j];
					}
            		if(NofUnrec > 1)
            			break;
				}
				if(NofUnrec == 1){
					codeword->encSymbolArray[temp].avail = 1; 
					repairedsyms++;
                	cont = 1;
				}
			}
		} */
		//printf("repaired syms: %d\n", repairedsyms);
	exitloop: ;
	}while(cont);
	return codeword;
}



void *runRepair(repair_t *repairword, decoder_t *codeword, int NumErased, int* erased, int disks){

	int i, j, cont, temp, NofUnrec = 0; 
	int numOfsubblocks = codeword->sizet/sizeof(sym_t); // how many sym_t's per coding symbol.
	int repairedsyms = 0;
	int blocksize = codeword->sizen/disks;
	sym_t *ssym_t, *esym_t;
	
	// Initialize success:
	repairword->success = 0;
	
	for(j=0; j< codeword->sizen; j++){ //initialize encoded symbols availability:
    	if(erased[j/blocksize] == 1){
    	    codeword->encSymbolArray[j].avail = 0;
    	}else{
        	codeword->encSymbolArray[j].avail = 1;
    	}
    }
	
	
	if(repairword->fast_mode == true){

		do{
			cont = 0;
			for(i = 0; i<repairword->sizeCheck3; i++){
				NofUnrec = 0; temp = 0;
				//printf(" %d ", repairword->check3[i].deg);
				for(j = 0; j<repairword->check3[i].deg; j++){
					if(codeword->encSymbolArray[repairword->check3[i].connections[j]].avail == 0){
		        		NofUnrec++;
		        		temp = repairword->check3[i].connections[j];   
		        	}
		        	if(NofUnrec > 1)
		        		break;
				}
		        if(NofUnrec == 1){
		            codeword->encSymbolArray[temp].avail = 1; 
		            repairedsyms++;
		            esym_t = &codeword->encdata[temp*numOfsubblocks];
		            for(j=0; j< repairword->check3[i].deg; j++){
		            	if(repairword->check3[i].connections[j] != temp){
		            		ssym_t = &codeword->encdata[repairword->check3[i].connections[j]*numOfsubblocks];
		                    align_XOR(esym_t, ssym_t, numOfsubblocks);
		            	}
		       	    }    
		        	cont = 1;
		        	if (repairedsyms == NumErased){
		        		repairword->success = 1;
		        		return codeword;
		        	}
		        	goto exitloop; //once decoded, go back to the beginning to check solutions from low degree check equations to reduce bandwidth.
		        }

			}
			//printf("repaired syms: %d\n", repairedsyms);
			//printf("numErased: %d\n", NumErased);
		exitloop: ;
		}while(cont);
	
	} // end of fast mode.
	
	// If fast repair does not work, 
	// switch to conventional wisdom: decode and partially encode:
	if (repairword->success == 0){
	
		codeword = (decoder_t *)runBP(codeword);
		if (codeword->success == 0)
			codeword = (decoder_t *)runBP4Precode(codeword);
		if (codeword->success == 0){
	    	printf(ERRORMSG "Error: Repair could NOT be succesfully completed. \n" RESET);
	    	printf(ERRORMSG "Error: Left-over unrecovered: %d\n" RESET, codeword->unrecovered);
	    	exit(0);
	    }else{
	    	repairword->success = 1;
	    }
	    // partially encode:
	    for(i=0; i<codeword->sizen; i++){
			if( codeword->encSymbolArray[i].avail == 0){
				esym_t = codeword->encdata +i*numOfsubblocks;
				for(j=0; j< codeword->encSymbolArray[i].deg; j++){
        			ssym_t = &codeword->srcdata[codeword->encSymbolArray[i].connections[j]*numOfsubblocks];
        			align_XOR(esym_t, ssym_t, numOfsubblocks);
      			}
			}
		}
	}
	
	
	return codeword;
}



