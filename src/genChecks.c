/* *
 * Copyright (c) 2020, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, ISTANBUL/TURKEY.
 * All rights reserved.
 * 
 * Founsure utility function: genChecks
 * 
 * Definition: This utility function can be used to find appropriate parity check matrices of the base
 * fountain code
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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "timing.h"
#include "encoder.h"
#include "usage.h"
#include "parameter.h"
#include "allocate.h"


int main(int argc, char *argv[]){
    
    int i,j, k, filesize, flag = 0, ch, changed, cnt, xorsetlen, disks, modify = 0;
    int pos, pos2, chunksize;
    int readins, IntArraySize, verbose = 0;
    int* xorset;
    char *precodename = NULL;
    char *s1, *s2;
    char *srcfname;
    char *fname = 0; // For metadata file. 
    char *curdir;
    FILE *fp;
    char *filename = NULL; // fOR FILE IDENTIFICATION.
    int extra = 0;
    int md = DISK_INDX_STRNG_LEN;
    
    dist_t *Dist 		  = (dist_t *)     AllocObject(sizeof(dist_t));
    encoder_t *EncoderObj = (encoder_t *)  AllocObject(sizeof(encoder_t));
    temp_conn_t *temp     = (temp_conn_t *)AllocObject(sizeof(temp_conn_t));
    temp_conn_t *localset = (temp_conn_t *)AllocObject(sizeof(temp_conn_t));
    
    disks                 = DEFAULT_NUMBER_OF_DISKS;
    Dist->name            = DEFAULT_DIST_NAME;	
    precodename           = DEFAULT_PC_NAME; 
    EncoderObj->sizesb    = SIZE_K;
    EncoderObj->sizek     = SIZE_K;
    EncoderObj->sizen     = SIZE_N;
    EncoderObj->sizet     = 1;
    EncoderObj->encSeed   = DEFAULT_SEED; 
    
	while( (ch = getopt( argc, argv, "f:e:m:h:v" ) ) !=-1 ){
		switch( ch ){
			case 'f':
				filename = optarg;
				break;
			case 'e':
				extra = atoi(optarg);
				break;
			case 'm':
				modify = atoi(optarg);
				break;
			case 'h':
				usage_genpcm( argv[0] );
				exit(0);
				break;
			case 'v':
            	verbose = 1;
            	break;
			default:
				usage_genpcm( argv[0] );
				exit(0);
				break;
        }
    }
    
	if (filename == NULL){
		printf(WARNINGMSG "Warning: " "No data file/path is specified.\n");
	}else{
		printf(KWHT "File %s is now being checked..." RESET,filename);
	}
	
	if (access(filename, F_OK) != 0){
		printf(ERRORMSG "\rError  : No such file exists.                    \t"
		COMMENT "\nComment: Type genChecks -h for help\n" RESET);
		exit(0);
	}else{
		curdir = (char*)malloc(sizeof(char)*1000);	
	  	assert(curdir == getcwd(curdir, 1000));
	  	fname = (char*)malloc(sizeof(char)*(strlen(filename)+strlen(curdir)+20)); 
	  	Dist->name = AllocFileName(filename, 100, 20);	
		s1 = (char*)malloc(sizeof(char)*(strlen(filename)+20));
		s2 = strrchr(filename, '/');	
		if (s2 != NULL) {
			s2++;strcpy(s1, s2);
		} else {
			strcpy(s1, filename);
		}

		sprintf(fname, "%s/Coding/%s_meta.txt", curdir, s1);
	  	fp = fopen(fname, "rb+");
	  	if (fp == NULL){
	  		printf(ERRORMSG "\rError: Meta File is not accesible. Run encoder to generate metadata file.                  \n" RESET);
	  		exit(0);
	  	} else {
	  		printf(COMMENT "\rComment: Meta File is found.                          \n" RESET);
	  	}

	  	srcfname = AllocFileName(filename, 0, 20); // (char *)malloc(sizeof(char)*(strlen(argv[1])+20));
		if (fscanf(fp, "%s", srcfname) != 1) {
			printf(ERRORMSG "Error:\tMetadata file - bad format 0\n" RESET);
			exit(0);
		}
		if (fscanf(fp, "%s", Dist->name) != 1) {
			printf(ERRORMSG "Error: Distribution name format is not valid.\n" RESET);
			exit(0);
		}
		precodename = AllocFileName(filename, 0, 20);
		if (fscanf(fp, "%s", precodename) != 1) {
			printf(ERRORMSG "Error: Precode name format is not valid.\n" RESET);
			exit(0);
		}
		if (fscanf(fp, "%d", &filesize) != 1) {
			printf(ERRORMSG "Error: Original size is not valid\n" RESET);
			exit(0);
		}
		if (fscanf(fp, "%d", &EncoderObj->encSeed) != 1) {
			printf(ERRORMSG "Error: Seed Number is not valid\n" RESET);
			exit(0);
		}
		
		pos = ftell(fp);
		if (fscanf(fp, "%d %d %d %d", &EncoderObj->sizesb, &EncoderObj->sizek, &EncoderObj->sizen, &EncoderObj->sizet) != 4) {
			printf(ERRORMSG "Error: Parameters are not correct\n" RESET);
			exit(0);
		}
		if (fscanf(fp, "%d", &disks) != 1) {
			printf(ERRORMSG "Error: Metadata file - bad format 1\n" RESET);
			exit(0);
		}
		if(modify >= 1) {
			pos2 = ftell(fp);
			fseek(fp, pos-pos2+1, SEEK_CUR);
			chunksize = EncoderObj->sizen/disks;	
			EncoderObj->sizen += chunksize*extra;
			fprintf(fp, "%d %d %d %d\n", EncoderObj->sizesb, EncoderObj->sizek, EncoderObj->sizen, EncoderObj->sizet);
			disks += extra;
			fprintf(fp, "%0*d\n", md, disks); 
		}
		pos = ftell(fp);
		if (fscanf(fp, "%d", &readins) != 1) {
			printf(ERRORMSG "Error: Metadata file - bad format 2\n" RESET);
			exit(0);
		}
		if (fscanf(fp, "%d", &IntArraySize) != 1) {
			printf(WARNINGMSG "Warning: Metadata file DOES NOT contain info for advanced decoding/repair.\n" RESET);
			if (modify >= 1)
				printf(COMMENT "Comment: Adding INFO for advanced decoding/repair...\n" RESET);
		}
		pos2 = ftell(fp);
		//ffclose(fp);	
	}
	
	if (verbose){
		printf(BOLD "-----------------------------------------------------------\n" RESET);
		printf(KWHT "Distribution Type	: %s\n",Dist->name);
		printf("Precode Type		: %s\n",precodename);
		if (strcmp(precodename, "None") == 0 )
			printf("Parameter k		: %d\n", EncoderObj->sizek);
		else{
			printf("Interblock k		: %d\n", EncoderObj->sizesb);
            printf("Parameter k		: %d\n", EncoderObj->sizek);
            printf("Adjusted Precode Rate	: %.3f\n", (double) EncoderObj->sizesb/EncoderObj->sizek);
        }
        printf("Parameter n		: %d\n", EncoderObj->sizen);
        printf("Parameter t		: %d (bytes) \n", EncoderObj->sizet);
        printf("Parameter s		: %d Disks \n", disks);
        if (IntArraySize > 2)
        	printf("Curr. Integer Arr. size	: %d \n", IntArraySize);	     
    }
    
    
    // Check the metadata file to overwrite the values: 
    if (strcmp(Dist->name, "FiniteDist") == 0 ){
        Dist->maxdeg = MAXMU;
    }else if (strcmp(Dist->name, "RSD") == 0 ){
        Dist->maxdeg = EncoderObj->sizek + 1;
    }    
    // Set Distribution:
    SetDistribution(Dist);
    // Introduce a temporary source symbol array with deg/connections:
    temp->conn = (symbol_s *) AllocObject(sizeof(symbol_s)*EncoderObj->sizen);
    // Introduce also local set construct:
    localset->conn = (symbol_s *) AllocObject(sizeof(symbol_s)*EncoderObj->sizen);
    // Initialize the encoder:
    EncoderObj = (encoder_t *)AllocateMem(EncoderObj, EncoderObj->sizek*EncoderObj->sizet, EncoderObj->sizen*EncoderObj->sizet); 
    // Prepare the encoder:
    EncoderObj = (encoder_t *)PrepareEnc(EncoderObj, Dist, disks);
    
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
    
    // To aviod an extra variable definition. Same "changed" is used to
    // find the total number of integers in the array thats going to be written. 
	changed = 0;
    for(i=0; i<EncoderObj->sizen; i++){
        changed = changed + localset->conn[i].deg;
    }
    changed = changed + EncoderObj->sizek + 2*EncoderObj->sizen;
    
    uint32_t* content2write = (uint32_t *) AllocObject(sizeof(uint32_t)*changed);

    
    cnt = 0;
    for(i=0; i<EncoderObj->sizen; i++){
    	if(temp->conn[i].deg == 1){ // check #2
    		//printf("1 %d ", temp->conn[i].connections[0]);
    		content2write[cnt] = 1; cnt++;
    		content2write[cnt] = (uint32_t) temp->conn[i].connections[0]; cnt++;
    	}else if(temp->conn[i].deg == 0){ // check #3
    		//printf("0 ");
    		content2write[cnt] = 0; cnt++;
    	}else{
    		//printf("E ");
    		flag = 1;
    		printf(ERRORMSG "Unrecognized symbol is encountered inside the check file! Exiting... \n" RESET);
    		exit(0);
    		//content2write[cnt] = 'E'; 
    		//cnt++;
    	}

    	//printf("%d ", localset->conn[i].deg);
    	content2write[cnt] = (uint32_t) localset->conn[i].deg; cnt++;
        for(k=0; k<localset->conn[i].deg; k++){
            //printf("%d ", localset->conn[i].connections[k]);
            content2write[cnt] = (uint32_t) localset->conn[i].connections[k]; cnt++;
        }
        //printf("\n");
        
    }
    if (cnt != changed)
    	changed = cnt;
   
       
    //for(i=0; i<EncoderObj->sizen; i++){
    //	printf(" %d ", content2write[i]);
    //}
   
   //printf("changed is %d\n", changed);
   //printf("IntArraySize is %d\n", IntArraySize);
    // Append 'changed' to the metadata file. 
    if (modify >= 1 && (IntArraySize < 2 || changed != IntArraySize)){
		//sprintf(fname, "%s/Coding/%s_meta.txt", curdir, s1);
	  	//fp = fopen(fname, "ab");
	  	fseek(fp, pos-pos2, SEEK_CUR);
	  	fprintf(fp, "%d\n", readins);
	  	fprintf(fp, "%d\n", changed);
		fclose(fp);	
	}else{
		fclose(fp);
	}
	
	if (verbose){
    	if (changed != IntArraySize && modify >= 1){
    		printf("New Integer Arr. size	: %d \n", changed);
    		printf(RESET BOLD "-----------------------------------------------------------\n");
    	}else{
    		printf(RESET BOLD "-----------------------------------------------------------\n");
    	}
    }

    // Write local set info to a check.data file:
	if (modify >= 1){
		sprintf(fname, "%s/Coding/%s_check.data", curdir, s1);
    	FILE *fhandler = fopen(fname, "wb");
		fwrite(content2write, sizeof(int), changed, fhandler);
		fclose(fhandler);
	}
   
    if (flag == 1){
    	printf(WARNINGMSG "Warning: The algorithm is unable to generate all checks:\n" RESET);
    	printf(COMMENT "\t + You can increase 'n' to avoid this situation. \n" RESET);
    	printf(COMMENT "\t + You can increase/decrease 'k' to avoid this situation. \n" RESET);
    }
    
    /**************************************************************************************************************/
    /**************************************************************************************************************/	
    /**************************************************************************************************************/
    // read the file:
    /*FILE *ifp = fopen(fname, "rb"); 
    if(fread(content2write, sizeof(int), changed, ifp) != changed){
    	printf(ERRORMSG "The file does not have enough information. Make sure that it is not corrupt! \n" RESET);
    	exit(0);
   	}
    fclose(ifp);
    
    int deg;
    i = 0;
    k = 0;
    while (i < changed && k <  EncoderObj->sizen){
    	if (content2write[i] == 0){
    		printf("%d ", content2write[i]);
    		i++;k++;
    		deg = content2write[i];
    		printf("%d ", deg);
    		for (j=i+1; j<deg+i+1;j++)
    			printf("%d ", content2write[j]);
    		i = j;
    		printf("\n");
		}else if(content2write[i] == 1){
			printf("%d %d ", content2write[i], content2write[i+1]);
			i+=2;k++;
			deg = content2write[i];
			printf("%d ", deg);
    		for (j=i+1; j<deg+i+1;j++)
    			printf("%d ", content2write[j]);
    		i = j;	
    		printf("\n");		
		}else{
			i++;k++;
			deg = content2write[i];
			//printf("%d ", deg);
    		for (j=i+1; j< deg+i+1; j++);
    			//printf("%d ", content2write[j]);
    		i = j;
		}		
    }
    */
    
    /*for(i=0; i< EncoderObj->sizen; i++){
        printf("%d:%d:", i, EncoderObj->encSymbolArray[i].deg);
        for(j=0; j< EncoderObj->encSymbolArray[i].deg; j++)
            printf("%d\t", EncoderObj->encSymbolArray[i].connections[j]);
        printf("\n");
    }*/
      
    /*srand(EncoderObj->encSeed); 
    for(i=0; i<1000000000; i++){
        temp = rand(); 
        if(temp == 456435) 
            printf("%d ", temp);
     }
     printf("\n");
     
    fast_srand(EncoderObj->encSeed);
    for(i=0; i<1000000000; i++){
        temp = fastrand(); 
        if(temp == 456435) 
            printf("%d ", temp);
    }
    printf("\n");*/
    
    free(Dist);
	free(EncoderObj->encdata);
	free(EncoderObj->srcSymbolArray);
	for(j=0; j< EncoderObj->sizen; j++)
	    free(EncoderObj->encSymbolArray[j].connections);
	free(EncoderObj->encSymbolArray);

    return(0);
}
