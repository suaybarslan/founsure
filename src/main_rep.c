/* *
 * Copyright (c) 2016, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, ISTANBUL/TURKEY.
 * All rights reserved.
 *
 * "SOPHISTICATED IDEAS are WORTH EXPLORING!"
 * 
 * Founsure - A C/C++ package for basic fountain erasure coding techniques.
 *
 * Main function for repair functionality.
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
#include <stdlib.h>
#include <string.h>

#include "repair.h"
#include "usage.h"
#include "parameter.h"
#include "allocate.h"
#include "timing.h"


enum Distribution_Type{FiniteDist, RSD, OWN};
char *Methods[3] = {"FiniteDist", "RSD", "OWN"}; 

int main(int argc, char *argv[]){

	FILE *fp;
	int i, ch, readinBytes, writeinBytes, verbose = 0, filesize, readins, blocksize, disks, IntArraySize, md;
	char *curdir, *cs1, *cs2, *extension, *metafname, *checkfname, *srcfname;
	char *filename = NULL, *precodename;
	sym_t **blocks;
	int *erased, *access_freq, numErased = 0, fp2, numOfsubblocks;
	
	timeval_t tic, toc, tic2, toc2, diff, diff2;
	double timeMB, timeMB2, bytesMB;
	
	diff2.tv_sec = 0; diff2.tv_usec = 0;
    gettimeofday(&tic2, NULL);

    // Allocate struct-memory for various objects:
    dist_t *Dist 			= (dist_t *)AllocObject(sizeof(dist_t));
    decoder_t *DecoderObj 	= (decoder_t *)AllocObject(sizeof(decoder_t));
    repair_t *RepairObj 	= (repair_t *)AllocObject(sizeof(repair_t));

	// Set default mode for repair:
	RepairObj->fast_mode = true;
	
    // Check if memory allocation is successful. 
    if(DecoderObj == NULL || Dist == NULL || RepairObj == NULL ){
        printf(ERRORMSG "\nError:  Insufficient Memory. Memory cannot be allocated!\n" RESET);
        free(DecoderObj); free(Dist); free(RepairObj);
        exit(0);
    }
	
    while( (ch = getopt( argc, argv, "f:h:v" ) ) !=-1 ){
        switch( ch ){
        case 'f':
            filename = optarg;
            break;
		case 'v':
            verbose = 1;
            break;
        case 'h':
            usage_rep( argv[0] );
            exit(0);
            break;
        default:
            usage_rep( argv[0] );
            exit(0);
            break;
        }
    }
    
    /* get the current directory */
    curdir = (char*)malloc(sizeof(char)*1000);	
	assert(curdir == getcwd(curdir, 1000));
	
	/* Begin recreation of file names */
	cs1 = AllocFileName(filename, 0, 0); //(char*)malloc(sizeof(char)*strlen(argv[1]));
	cs2 = strrchr(filename, '/');
	if (cs2 != NULL) {
		cs2++;
		strcpy(cs1, cs2);
	}else{
		strcpy(cs1, filename);
	}
	cs2 = strchr(cs1, '.');
	if (cs2 != NULL) {
        extension = strdup(cs2);
		*cs2 = '\0';
	}else{
        extension = strdup("");
    }
    
    metafname = AllocFileName(filename, 100, 20);
    checkfname = AllocFileName(filename, 100, 20);
    Dist->name = AllocFileName(filename, 100, 20);
    /* Read in parameters from metadata file */
	sprintf(metafname, "%s/Coding/%s_meta.txt", curdir, cs1);
	sprintf(checkfname, "%s/Coding/%s_check.data", curdir, cs1);
	
	
	fp = fopen(metafname, "rb");
    if (fp == NULL){
        printf(ERRORMSG "Error: No metadata file at %s is found.\n" RESET, metafname);
        printf(WARNINGMSG "Warning: Repair cannot continue without a metadata file.\n" RESET);
        exit(1);
    }
    srcfname = AllocFileName(filename, 0, 20);
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
    if (fscanf(fp, "%d", &DecoderObj->encSeed) != 1) {
		printf(ERRORMSG "Error: Seed Number is not valid\n" RESET);
		exit(0);
	}
	if (fscanf(fp, "%d %d %d %d", &DecoderObj->sizesb, &DecoderObj->sizek, &DecoderObj->sizen, &DecoderObj->sizet) != 4) {
		printf(ERRORMSG "Error: Code Parameters are not correct\n" RESET);
		exit(0);
	}
    if (fscanf(fp, "%d", &disks) != 1) {
		printf(ERRORMSG "Error: Metadata file - bad format 0xDisk\n" RESET);
		exit(0);
	}
	if (fscanf(fp, "%d", &readins) != 1) {
		printf(ERRORMSG "Error: Metadata file - bad format 0xReadin\n" RESET);
		exit(0);
	}
	if (fscanf(fp, "%d", &IntArraySize) != 1) {
		printf(WARNINGMSG "Warning: Metadata file DOES NOT contain info for fast repair.\n" RESET);
		printf(COMMENT "Comment: Reverting to conventional repair\n" RESET);
		RepairObj->fast_mode = false;
	}
	fclose(fp);	
	
	blocksize = DecoderObj->sizen/disks;
	numOfsubblocks = DecoderObj->sizet/(int)sizeof(sym_t);
	bytesMB = (double)filesize/OMEG; 
	
	if (verbose){
        printf(BOLD "-----------------------------------------------------------\n" RESET);
        printf(KWHT "Distribution Type    : %s\n",Dist->name);
        printf("Precode Type	     : %s\n",precodename);
        if (strcmp(precodename, "None") == 0 )
            printf("Parameter k 	     : %d\n", DecoderObj->sizek);
        else{
            printf("Interblock k	     : %d\n", DecoderObj->sizesb);
            printf("Parameter k  	     : %d\n", DecoderObj->sizek);
            printf("Adjusted Precode Rate: %.3f\n", (double) DecoderObj->sizesb/DecoderObj->sizek);
        }
        printf("Parameter n 	     : %d\n", DecoderObj->sizen);
        printf("Parameter t 	     : %d (bytes) \n", DecoderObj->sizet);
        printf("Parameter s          : %d Disks \n", disks);
        printf("Buffersize	     : %d (bytes) \n", DecoderObj->sizesb*DecoderObj->sizet);
        if ( RepairObj->fast_mode == true )
        	printf("Advanced Repair      : ON\n");
       	else
       		printf("Advanced Repair      : OFF\n");
        printf(RESET BOLD "-----------------------------------------------------------\n");
    }
	
	
	/* Set Distribution Parameters */
	blocks = (sym_t **)malloc(sizeof(sym_t*)*disks);

    if (strcmp(Dist->name, Methods[FiniteDist]) == 0 ){
        Dist->maxdeg = MAXMU;
    }else if (strcmp(Dist->name, Methods[RSD]) == 0 ){
        Dist->maxdeg = DecoderObj->sizek + 1;
    }else if (strcmp(Dist->name, Methods[OWN]) == 0 ){
    //Not yet defined. 
    }
    
    md = DISK_INDX_STRNG_LEN; 
	//erased = (int *)malloc(sizeof(int)*(disks));
	erased = (int *)AllocObject(sizeof(int)*(disks));
	access_freq = (int *)calloc(disks, sizeof(int));
	for (i = 0; i < disks; i++)
		erased[i] = 0;
	// Determine the missing files and erasures.
	for (i = 1; i <= disks; i++) {
	    sprintf(metafname, "%s/Coding/%s_disk%0*d%s", curdir, cs1, md, i, extension);
	    fp = fopen(metafname, "rb");
	    if (fp == NULL) {
	        erased[i-1] = 1;
	        numErased++;
	        printf(COMMENT "Comment: Repairing... " WARNINGMSG "%s\n" RESET, metafname);
	        //printf("Comment: Repairing... %s\n", metafname);
	    }else{
	        fclose(fp);
	    }
	}
	
	if (numErased == 0){
		printf(WARNINGMSG "Warning: There is nothing to be repaired. Exiting... \n" RESET);
		exit(0);
	}

	diff.tv_sec = 0; diff.tv_usec = 0;
    gettimeofday(&tic, NULL);
    /*********************************************** REPAIR PARAMS SET ***********************************/
	SetDistribution(Dist); // set CDF 
	
    /*Allocate memory for Decoder Object*/
    DecoderObj = (decoder_t *)AllocateMemDec(DecoderObj, DecoderObj->sizek*DecoderObj->sizet, DecoderObj->sizen*DecoderObj->sizet);
    
	/*Prepare decoder for decoding computation*/
    DecoderObj = (decoder_t *)PrepareDec(DecoderObj, Dist, erased, disks); 
    //printf("here\n");
    if(RepairObj->fast_mode == true){
    	/* Set Fast Repair Parameters */
		RepairObj->sizeCheck2 = DecoderObj->sizek;
		RepairObj->sizeCheck3 = DecoderObj->sizen - DecoderObj->sizesb;
    	/*Prepare RepairObj for Regeneration*/
		RepairObj = (repair_t*)AllocAndPrepareRep4Dec(RepairObj, DecoderObj, checkfname, IntArraySize);	
				
	}
	
	gettimeofday(&toc, NULL);
    time_diff(&diff, &tic, &toc); 
	//printf("Repair is %d\n", RepairObj->success);
	timeMB = (double)(diff.tv_sec*ONEM + diff.tv_usec)/(ONEM);
    printf(BOLD KMAG "Prep. Time: %lf GB/sec\n" RESET, bytesMB/timeMB/1024 );
	DecoderObj = need2AccessDisks(RepairObj, DecoderObj, numErased*blocksize, blocksize, access_freq);	
	// Check repair check #2 & #3: For Debugging Purposes
	// check_repair(RepairObj, DecoderObj);

	
	
	/*Run repair process w/o data: For debugging */ 
	/* runRepairNoData(RepairObj, DecoderObj, numErased*blocksize);
	if (RepairObj->success == 1)
		printf("Repair is successfull!\n"); */
		
	/* Start repair process */
	diff.tv_sec = 0; diff.tv_usec = 0;
	int n = 1;	
	while(n <= readins){        
		
		for(i=0; i< DecoderObj->sizen*numOfsubblocks; i++)
        	DecoderObj->encdata[i] = 0; // reset encoded data.
	
	
		for(i = 1; i <= disks; i++) {
			blocks[i-1] = DecoderObj->encdata+((i-1)*blocksize*numOfsubblocks);
			
	        //if(erased[i-1] == 0 && access_freq[i-1] > 0){ // If unerased and useful at the same time.
	        if(erased[i-1] == 0){
	            sprintf(metafname, "%s/Coding/%s_disk%0*d%s", curdir, cs1, md, i, extension);
	            
	            fp = fopen(metafname, "rb");
	            fseek(fp, blocksize*DecoderObj->sizet*(n-1), SEEK_SET); 
	            if (fp != NULL){         	                       
	                readinBytes = fread(blocks[i-1], sizeof(char), blocksize*DecoderObj->sizet, fp);    
	                if(readinBytes != blocksize*DecoderObj->sizet){
                        printf(ERRORMSG "Error:  An unresolved write error is detected. The file might be corrupted.\n" RESET);
                        exit(0);
                    }
                    
                    fclose(fp);
                }else{
                    printf(ERRORMSG "Error:  Unable to open the unerased file!.\n" RESET);
                    exit(0);
                }
	        }
	    }
	    
	    // Run actual repair for the data blocks:
	    gettimeofday(&tic, NULL);
	    DecoderObj = (decoder_t *)runRepair(RepairObj, DecoderObj, numErased*blocksize, erased, disks);
	    gettimeofday(&toc, NULL);time_diff(&diff, &tic, &toc);
	   
		// Write only the repaired blocks:
        for(i = 1; i <= disks; i++) {  	
        	if(erased[i-1] == 1){ // If  erased, fix it.
          		sprintf(metafname, "%s/Coding/%s_disk%0*d%s", curdir, cs1, md, i, extension);
        		if(n==1){
	        		fp2 = open(metafname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	      		}else{
	        		fp2 = open(metafname, O_WRONLY | O_APPEND | O_EXCL, 0644);
	      		}
	      		writeinBytes = write(fp2, blocks[i-1] , blocksize*DecoderObj->sizet);
	      
	      		if(writeinBytes != blocksize*DecoderObj->sizet){
	        		printf(ERRORMSG "Error:  An unresolved write error is detected. The file might be corrupted.\n" RESET);
	        		exit(0);
	      		}
	      		close(fp2);
        	}	
        }     
		n++;
	}
	
	gettimeofday(&toc2, NULL);  
    time_diff(&diff2, &tic2, &toc2);     
    fp2 = 0;
    ch = 0; 
    for(i=0; i<disks; i++) 
    	if(erased[i] == 0 && access_freq[i] != 0){
    		fp2 += access_freq[i];
		ch++;
	}
    printf(COMMENT "Comment:  A total of %d bytes are transferred/read from %d disks (min possible:%d). \n" RESET, fp2*readins*DecoderObj->sizet, ch, (fp2/blocksize)+1);
    

    timeMB = (double)(diff.tv_sec*ONEM + diff.tv_usec)/ONEM;
    timeMB2 = (double)(diff2.tv_sec*ONEM + diff2.tv_usec)/ONEM;
    printf(BOLD KMAG "Repair Time: %lf MB/sec\n" RESET, bytesMB/timeMB );
    printf(BOLD KMAG "Total Time: %lf MB/sec\n" RESET, bytesMB/timeMB2 );
	
	//free(Dist);
	//free(RepairObj->check2);
	//free(RepairObj->check3);
	//free(DecoderObj->srcdata);
	//free(DecoderObj->encdata);
	//free(DecoderObj->srcSymbolArray);
	//free(DecoderObj->encSymbolArray);
	//free(DecoderObj);

	return(0);
    
}
