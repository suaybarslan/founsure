/* *
 * Copyright (c) 2020, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, ISTANBUL/TURKEY.
 * All rights reserved.
 *
 * "SOPHISTICATED IDEAS are WORTH EXPLORING!"
 * 
 * Founsure - A C/C++ package for basic fountain erasure coding techniques.
 * 
 * Revision 1.0: First version includes basic LT and basic concatenated fountain coding.
 * See the doc: "Incremental Redundancy and Fountain codes" at Arxiv.
 * 
 * History: 8/11/2017 --> This document is prepared for optimal sym allocation. 
 *                        Not complete yet. Ignore compile time warnings. 
 * 
 * Redistributions of the source code is ought to retain the above copyright notice, the
 * list of conditions and the disclaimer thereof. The author's affiliation may not be used
 * to endorse or promote products dervied from the following software package without the
 * permission of any sort.
 * 
 * THIS SOFTWARE COMES WITH NO WARRANTY OF ANY KIND AND "AS IS" WITHOUT COMMITING ANY IMPLIED 
 * GUARANTEE OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES HOWEVER CAUSED AND ON ANY THEORY OR LIABILITY, WHETHER
 * IN CONTRACT, STRICT WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH AFROMENTIONED UNPREDICTED DAMAGE. 
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "repair.h"
#include "timing.h"
#include "encoder.h"
#include "usage.h"
#include "parameter.h"
#include "allocate.h"

enum Distribution_Type{FiniteDist, RSD, OWN};
char *Methods[3] = {"FiniteDist", "RSD", "OWN"}; 


int main(int argc, char *argv[]){

  int i, j, k, cnt3, verbose = 0;
  int BSIZE = 0;
  int CSIZE;
  int filesize;
  int* xorset;
  
  // get parameters of the system: 
  int ch, g = 1, disks, changed, fileHandler;
  int xorsetlen;
  int redundantZeros;
  char *filename = NULL;
  char *precodename = NULL;
  double target_cr; // for time measurement.
  stat_t st;
  
  // Allocate struct-memory for various objects:
  dist_t 		*Dist 		= (dist_t *)   AllocObject(sizeof(dist_t));
  encoder_t 	*EncoderObj = (encoder_t *)AllocObject(sizeof(encoder_t));
  temp_conn_t 	*temp     	= (temp_conn_t *)AllocObject(sizeof(temp_conn_t));
  temp_conn_t 	*localset 	= (temp_conn_t *)AllocObject(sizeof(temp_conn_t));
  repair_t 		*RepairObj 	= (repair_t *)AllocObject(sizeof(repair_t));
  
  // Set default mode for repair:
  RepairObj->fast_mode = true;

  // Check if memory allocation is successful. 
  if(EncoderObj == NULL || Dist == NULL || RepairObj == NULL){
    printf(ERRORMSG "\nError:  Insufficient Memory. Memory cannot be allocated!\n" RESET);
    free(EncoderObj); free(Dist); free(RepairObj);
    exit(0);
  }	

  disks                 = DEFAULT_NUMBER_OF_DISKS;
  Dist->name            = DEFAULT_DIST_NAME;	
  precodename           = DEFAULT_PC_NAME; 
  target_cr             = DEFAULT_PRECODE_RATE;
  EncoderObj->sizesb    = SIZE_K;
  EncoderObj->sizek     = SIZE_K;
  EncoderObj->sizen     = SIZE_N;
  EncoderObj->sizet     = SIZE_T;
  EncoderObj->encSeed   = DEFAULT_SEED;  
  
  while( (ch = getopt( argc, argv, "f:d:k:n:t:p:c:g:s:h:v" ) ) !=-1 ){
    switch( ch ){
    case 'f':
      filename = optarg;
      break;
    case 'd':
      Dist->name = optarg;
      break;
    case 'k':
      EncoderObj->sizek = atoi(optarg);
      break;
    case 'n':
      EncoderObj->sizen = atoi(optarg);
      break;
    case 't':
      EncoderObj->sizet = atoi(optarg);
      break;
    case 'p':
      precodename = optarg;
      break;
    case 'c':
      target_cr = atof(optarg);
      break;
    case 'g':
      g = atoi(optarg);
      break;
    case 's':
      disks = atoi(optarg);
      break;
    case 'h':
      usage( argv[0] );
      exit(0);
      break;
    case 'v':
      verbose = 1;
      break;
    default:
      usage( argv[0] );
      exit(0);
      break;
    }
  }

  // Error check on file
  if (filename == NULL){
    printf(WARNINGMSG "Warning: " "No data file/path is specified.\n");
  }else{
    printf(KWHT "File %s is now being checked..." RESET,filename);
  }
  if (access(filename, F_OK) != 0){
    printf(ERRORMSG "\rError  : No such file exists.                    \t"
	   COMMENT "\nComment: Type founsureEnc -h for help		    \n" RESET);
	exit(0);
  }else{
    fileHandler = open(filename, O_RDONLY);
    stat(filename, &st);
    filesize = (int)st.st_size;
    printf(COMMENT "\rComment: File is found.                          \n" RESET);
  }
  
  // Error check on symbol size
  if (EncoderObj->sizet % ((int)sizeof(sym_t)*8) != 0){
    printf(ERRORMSG "Error:   Symbol size (-t) is supposed to use increments of %d bytes.\n" RESET, (int)sizeof(sym_t)*8);
    exit(0);
  }

  // Precode dependent adjustments
  if (strcmp(precodename, "None") == 0 ){
    redundantZeros = AdjustParamNoPrecode(EncoderObj, filesize);
    EncoderObj->sizesb = EncoderObj->sizek;
    BSIZE = EncoderObj->sizek*EncoderObj->sizet;
  }else if (strcmp(precodename, "ArrayLDPC") == 0 ){
    EncoderObj->sizesb = EncoderObj->sizek;
    redundantZeros = AdjustParamWithPrecode(EncoderObj, filesize, target_cr);
    BSIZE = EncoderObj->sizesb*EncoderObj->sizet;
  }else{
    printf(WARNINGMSG "Warning: " "Precode" KCYN " %s " KYEL "is not recognized. Switching to default (No precode)\n" RESET, precodename);
    precodename = "None"; //Default is "None" when the selection is unrecognized.
    redundantZeros = AdjustParamNoPrecode(EncoderObj, filesize);
    EncoderObj->sizesb = EncoderObj->sizek;
    BSIZE = EncoderObj->sizek*EncoderObj->sizet;
  }
  if (redundantZeros == 0)
    printf(COMMENT "Comment: " "File and parameter selections perfectly match\n" RESET);
  else{
    printf(WARNINGMSG "Warning: " "File and parameter selections do not match!\n");
    printf(WARNINGMSG "Warning: " "Number of redundant/paded bytes : %d\n" RESET, redundantZeros);
  }
  // Adjust parameter n according to the number of disks:
  while(EncoderObj->sizen%disks != 0)
    EncoderObj->sizen++;
  //int blocksize = EncoderObj->sizen/disks; //How many symbols per disk.
  // Finally check error on k and n
  if (EncoderObj->sizek >= EncoderObj->sizen){
    printf(WARNINGMSG "Warning: " "Parameter n cannot be less than %d: No protection is assumed.\n" RESET, EncoderObj->sizek);
    EncoderObj->sizen = EncoderObj->sizek;
    printf(COMMENT "Comment: " "Parameter n is set to minimum possible.\n" RESET);
    while(EncoderObj->sizen%disks != 0)
      EncoderObj->sizen++;
    //blocksize = EncoderObj->sizen/disks;
  }

  CSIZE = EncoderObj->sizen*EncoderObj->sizet;

  if (verbose){
    printf(BOLD "-----------------------------------------------------------\n" RESET);
    printf(KWHT "Distribution Type    : %s\n",Dist->name);
    printf("Precode Type	     : %s\n",precodename);
    if (strcmp(precodename, "None") == 0 )
      printf("Parameter k 	     : %d\n", EncoderObj->sizek);  
    else{
      printf("Interblock k	     : %d\n", EncoderObj->sizesb);
      printf("Parameter k  	     : %d\n", EncoderObj->sizek);
    }
    printf("Parameter n 	     : %d\n", EncoderObj->sizen);
    printf("Parameter t 	     : %d (bytes) \n", EncoderObj->sizet);
    printf("Parameter s          : %d Disks \n", disks);
    if (strcmp(precodename, "None") != 0 ){
      printf("Target Precode Rate  : %.3f\n", target_cr);
      printf("Adjusted Precode Rate: %.3f\n", (double)EncoderObj->sizesb/EncoderObj->sizek);
    }  
    printf("Buffersize	     : %d (bytes) \n", BSIZE);
    if ( g == 0 ){
      printf("Parameter g 	     : SLOW MODE\n");
    }else{
      printf("Parameter g 	     : FAST MODE\n" RESET);
    }
    printf(BOLD "-----------------------------------------------------------\n" RESET);
  }

  
  // Set the distribution parameter:
  if (strcmp(Dist->name, Methods[0]) == 0 ){
    Dist->maxdeg = MAXMU;
  }else if (strcmp(Dist->name, Methods[1]) == 0 ){
    Dist->maxdeg = EncoderObj->sizek + 1;
  }else if (strcmp(Dist->name, Methods[2]) == 0 ){
    //Not yet defined. 
  }

  //Set the distribution values: maxdeg and name are already set , cdf is not.
  SetDistribution(Dist);
  // Introduce a temporary source symbol array with deg/connections:
  temp->conn = (symbol_s *) AllocObject(sizeof(symbol_s)*EncoderObj->sizen);
  // Introduce also local set construct:
  localset->conn = (symbol_s *) AllocObject(sizeof(symbol_s)*EncoderObj->sizen);
  //allocate memory for encoder and initialize it:
  EncoderObj = (encoder_t *)AllocateMem(EncoderObj, EncoderObj->sizek*EncoderObj->sizet, CSIZE); 
  //prepare encoder for compute:
  EncoderObj = (encoder_t *)PrepareEnc(EncoderObj, Dist, disks);
  
  printf(COMMENT "Comment: " "Bipartite Graph for Check #2 and #3 is being generated...\n" RESET);
  // Initialize temp and localset:
  for(i=0; i<EncoderObj->sizen; i++){
    temp->conn[i].deg = EncoderObj->encSymbolArray[i].deg;
    temp->conn[i].connections = (int *)AllocObject(sizeof(int)*temp->conn[i].deg);
    for(j=0; j<temp->conn[i].deg; j++)
        temp->conn[i].connections[j] = EncoderObj->encSymbolArray[i].connections[j];
    localset->conn[i].connections = (int *)AllocObject(sizeof(int));
    localset->conn[i].connections[0] = i;
    localset->conn[i].deg = 1;
  }
  
  while(numofzeros(temp, EncoderObj->sizen) < EncoderObj->sizen - EncoderObj->sizek){   
	changed = 0;
	for(j=0;j<EncoderObj->sizen;j+=1){
	for(i=0;i<EncoderObj->sizen;i+=1){
		if(j != i && 2*temp->conn[j].deg > temp->conn[i].deg){
		    xorset = setXOR(temp->conn[j].connections,
		                    temp->conn[j].deg,
		                    temp->conn[i].connections,
		                    temp->conn[i].deg);
		    xorsetlen = xorset[temp->conn[i].deg + temp->conn[j].deg];
		    if(xorsetlen < temp->conn[j].deg){ // Copy result and update.
		        temp->conn[j].deg = xorsetlen;
		        for(k=0; k<xorsetlen; k++)
		            temp->conn[j].connections[k] = xorset[k];
		        free(xorset); // free the allocated memory.
		        xorset = setXOR(localset->conn[j].connections,
		                        localset->conn[j].deg,
		                        localset->conn[i].connections,
		                        localset->conn[i].deg);
		        xorsetlen = xorset[localset->conn[i].deg + localset->conn[j].deg];
		        localset->conn[j].deg = xorsetlen;
		        // reallocate memory.
		        localset->conn[j].connections = (int*)realloc(localset->conn[j].connections,sizeof(int)*localset->conn[j].deg);
		        for(k=0; k<xorsetlen; k++)
		            localset->conn[j].connections[k] = xorset[k];
		        free(xorset);
		        changed = 1;
		    }else{
		        free(xorset);
		    }
		}
	}
	}
	if(changed == 0)
	break;
  }
  
  // Fill in the repair Object appropriately:
  RepairObj->sizeCheck2 = EncoderObj->sizek;
  RepairObj->sizeCheck3 = EncoderObj->sizen - EncoderObj->sizesb;
  // Allocate memory space:
  RepairObj->check2 = (symbol_sd *)malloc(sizeof(symbol_sd)*RepairObj->sizeCheck2); 
  RepairObj->check3 = (symbol_sd *)malloc(sizeof(symbol_sd)*RepairObj->sizeCheck3);
  
  sort_t temp2[EncoderObj->sizen - EncoderObj->sizek];
  cnt3 = 0;
  for(i=0; i<EncoderObj->sizen; i++){
	if(temp->conn[i].deg == 0){
		temp2[cnt3].val = localset->conn[i].deg;
		temp2[cnt3].ind = i;
		cnt3++;
	}
  }	

  if(ORDER_CHECK_3)
    qsort(temp2, EncoderObj->sizen - EncoderObj->sizek, sizeof(sort_t), CmpFunc);

 
  // Regular Check #3 nodes:
  cnt3 = 0;
  for(i=0; i<EncoderObj->sizen; i++){
  	if(temp->conn[i].deg == 0){
  		RepairObj->check3[cnt3].connections = (int *) malloc(sizeof(int)*localset->conn[temp2[cnt3].ind].deg);
		RepairObj->check3[cnt3].deg = localset->conn[temp2[cnt3].ind].deg;
		for(j=0; j<localset->conn[temp2[cnt3].ind].deg; j++)
			RepairObj->check3[cnt3].connections[j] = localset->conn[temp2[cnt3].ind].connections[j];
		cnt3++;
  	}else if(temp->conn[i].deg == 1){
  		RepairObj->check2[temp->conn[i].connections[0]].connections = (int *) malloc(sizeof(int)*localset->conn[i].deg);
		RepairObj->check2[temp->conn[i].connections[0]].deg = localset->conn[i].deg;
		for(j=0; j<localset->conn[i].deg; j++)
			RepairObj->check2[temp->conn[i].connections[0]].connections[j] = localset->conn[i].connections[j];
  	}else{
  		continue;
  	}
  }
  
  // Extra Check #3 nodes due to Check #2.
  int ind, deg;
  for (i=cnt3;i<RepairObj->sizeCheck3;i++){
	ind = i-cnt3+EncoderObj->sizesb; //index for srcSymbolArray.
	// Initialize:
	RepairObj->check3[i].deg = RepairObj->check2[ind].deg;
	RepairObj->check3[i].connections = (int *) malloc(sizeof(int)*RepairObj->check3[i].deg);

	for(k=0; k<RepairObj->check3[i].deg; k++)
		RepairObj->check3[i].connections[k] = RepairObj->check2[ind].connections[k];
	
	for(j=0; j<EncoderObj->srcSymbolArray[ind].deg; j++){
		deg = RepairObj->check3[i].deg + 
				RepairObj->check2[EncoderObj->srcSymbolArray[ind].connections[j]].deg;
		xorset = setXOR(RepairObj->check3[i].connections,
        				RepairObj->check3[i].deg,
        				RepairObj->check2[EncoderObj->srcSymbolArray[ind].connections[j]].connections,
        				RepairObj->check2[EncoderObj->srcSymbolArray[ind].connections[j]].deg);
		RepairObj->check3[i].deg = xorset[deg];
		// reallocate memory:
		RepairObj->check3[i].connections = (int*)realloc(RepairObj->check3[i].connections, sizeof(int)*RepairObj->check3[i].deg);
		for(k=0; k<RepairObj->check3[i].deg; k++) // compy new result.
			RepairObj->check3[i].connections[k] = xorset[k];
		free(xorset);
	}	
  }
  
  for(i=0;i<EncoderObj->sizen; i++){
  	free(temp->conn[i].connections);
  	temp->conn[i].deg = 0;
  	free(localset->conn[i].connections);
  }
  	
  // Calculate the undirected Graph...	
  int* temp3, USET[1000];
  //for(i=0; i<RepairObj->sizeCheck3; i++){
  for(i=0; i<300; i++){
  	for(j=0; j<RepairObj->check3[i].deg-1; j++){
  		//printf("begin\n");
  		temp3 = RepairObj->check3[i].connections+j+1;
  		deg = temp->conn[RepairObj->check3[i].connections[j]].deg + RepairObj->check3[i].deg - (j+1);
  		//printf("begin2 %d, %d\n", RepairObj->check3[i].deg, j);
  		uniqueSET(USET, temp3, 
  						RepairObj->check3[i].deg - (j+1),
  						temp->conn[RepairObj->check3[i].connections[j]].connections, 
  						temp->conn[RepairObj->check3[i].connections[j]].deg);
  						
  		temp->conn[RepairObj->check3[i].connections[j]].deg = USET[deg];
  		//printf("%d\n",USET[deg]);
  		//printf("complete %d - %d\n", sizeof(int)*USET[deg], temp->conn[RepairObj->check3[i].connections[j]].connections[0]);
  		temp->conn[RepairObj->check3[i].connections[j]].connections = (int*)realloc(temp->conn[RepairObj->check3[i].connections[j]].connections, sizeof(int)*USET[deg]);
  		//printf("--\n");
  		for(k=0; k<USET[deg]; k++)
  			temp->conn[RepairObj->check3[i].connections[j]].connections[k] = USET[k];
  		//free(xorset);
  		
  	}
  }
  
  /*for(i=0;i<20;i++){
  	for(j=0;j<RepairObj->check3[i].deg;j++){
  		printf("%d ", RepairObj->check3[i].connections[j]);
  	}
  	printf("\n");
  }*/
  
  /*for(i=0;i<EncoderObj->sizen;i++){
  	if(temp->conn[i].deg != 0){
  		printf("%d: %d:  ", i, temp->conn[i].deg);
  		for(j=0;j<temp->conn[i].deg; j++)
  			printf(" %d", temp->conn[i].connections[j]);
  		printf("\n");
  	}
  }*/

  
 
  
  
  
  
  
  
  free(EncoderObj->srcdata);
  free(EncoderObj->encdata);
  free(EncoderObj->srcSymbolArray);
  //free(EncoderObj->encSymbolArray);
  free(EncoderObj);
  return(0);
}
