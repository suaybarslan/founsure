/* *
 * Copyright (c) 2016, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, ISTANBUL/TURKEY.
 * All rights reserved.
 *
 * "SOPHISTICATED IDEAS are WORTH EXPLORING!"
 * 
 * Founsure - A C/C++ package for basic fountain erasure coding techniques.
 * 
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

#include "encoder.h"
#include "decoder.h"
#include "usage.h"
#include "parameter.h"
#include "allocate.h"
#include "timing.h"

enum Distribution_Type{FiniteDist, RSD, OWN};
char *Methods[3] = {"FiniteDist", "RSD", "OWN"}; 

int main(int argc, char *argv[]){
    FILE *fp;				// File pointer
    
    int i, ch, verbose = 0, advanced = 0, readinBytes, writeinBytes, numOfThreads, fp2;
    char *curdir, *cs1, *cs2, *extension, *fname;
    char *srcfname;
    char *precodename;
    int filesize, readins, disks, IntArraySize;
    int blocksize;
    int total;				// used to write data, not padding to file
    int *erased, *decoding_path;
    sym_t **blocks;
    
    int md, BSIZE, CSIZE;
    char *filename = NULL;
    double timeMB, timeMB2, bytesMB;
    
    timeval_t tic, toc, tic2, toc2, diff, diff2;
    
    diff2.tv_sec = 0; diff2.tv_usec = 0;
    gettimeofday(&tic2, NULL);
    
    // Allocate struct-memory for various objects:
    dist_t *Dist 		= (dist_t *)AllocObject(sizeof(dist_t));
    decoder_t *DecoderObj = (decoder_t *)AllocObject(sizeof(decoder_t));
    
    
    // Check if memory allocation is successful. 
    if(DecoderObj == NULL || Dist == NULL){
        printf(ERRORMSG "\nError:  Insufficient Memory. Memory cannot be allocated!\n" RESET);
        free(DecoderObj);
        free(Dist);
        exit(0);
    }
    
    numOfThreads          = DEFAULT_NUMBER_OF_THREADS;
    
    while( (ch = getopt( argc, argv, "f:m:a:v:h" ) ) !=-1 ){
        switch( ch ){
        case 'f':
            filename = optarg;
            break;
        case 'm':
            numOfThreads = atoi(optarg);
            break;
        case 'a':
            advanced = atoi(optarg);
            break;
		case 'v':
            verbose = 1;
            break;
        case 'h':
            usage_dec( argv[0] );
            exit(0);
            break;
        default:
            usage_dec( argv[0] );
            exit(0);
            break;
        }
    }

    /* Error checking parameters */
    if (filename == NULL){
        printf(WARNINGMSG "Warning: " "No data file/path is specified.\n");
    }
    if (access(filename, F_OK) != 0){
        printf(ERRORMSG "\rError  : No such file exists.                    \t"
        COMMENT "\nComment: Type founsureDec -h for help		    \n"      RESET);
        exit(0);
    }else{
        printf(COMMENT "\rComment: File is found.                          \n" RESET);
    }

    /* Get current working directory for construction of file names */
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
    
    //fname = (char *)malloc(sizeof(char*)*(100+strlen(argv[1])+20));
    fname = AllocFileName(filename, 100, 20);
    Dist->name = AllocFileName(filename, 100, 20);
	/* Read in parameters from metadata file */
	sprintf(fname, "%s/Coding/%s_meta.txt", curdir, cs1);
    fp = fopen(fname, "rb");
    if (fp == NULL){
        printf(ERRORMSG "Error: No metadata file at %s is found.\n" RESET, fname);
        printf(WARNINGMSG "Warning: Decoder cannot continue without a metadata file.\n" RESET);
        exit(1);
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
    if (fscanf(fp, "%d", &DecoderObj->encSeed) != 1) {
		printf(ERRORMSG "Error: Seed Number is not valid\n" RESET);
		exit(0);
	}
	if (fscanf(fp, "%d %d %d %d", &DecoderObj->sizesb, &DecoderObj->sizek, &DecoderObj->sizen, &DecoderObj->sizet) != 4) {
		printf(ERRORMSG "Error: Parameters are not correct\n" RESET);
		exit(0);
	}
    if (fscanf(fp, "%d", &disks) != 1) {
		printf(ERRORMSG "Error: Metadata file - bad format 1\n" RESET);
		exit(0);
	}
	if (fscanf(fp, "%d", &readins) != 1) {
		printf(ERRORMSG "Error: Metadata file - bad format 2\n" RESET);
		exit(0);
	}
	if (fscanf(fp, "%d", &IntArraySize) != 1) {
		if (advanced == 1 )
			printf(WARNINGMSG "Warning: Metadata file DOES NOT contain info for advanced decoding/repair.\n" RESET);
	}
	fclose(fp);	
	
	BSIZE = DecoderObj->sizesb*DecoderObj->sizet;
	CSIZE = DecoderObj->sizen*DecoderObj->sizet;
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
        printf("Buffersize	     : %d (bytes) \n", BSIZE);
        if ( advanced == 1 )
        	printf("Advanced Decoding    : ON\n");
       	else
       		printf("Advanced Decoding    : OFF\n");
       	printf("Number Of Threads    : %d \n" RESET, numOfThreads);
        printf(RESET BOLD "-----------------------------------------------------------\n");
    }
	
	/*************************************************** DISTRIBUTION PARAMETERS SET - BEGIN *************************************/
	/* Set Distribution Parameters */
	blocks = (sym_t **)malloc(sizeof(sym_t*)*disks);
    if (strcmp(Dist->name, Methods[FiniteDist]) == 0 ){
        Dist->maxdeg = MAXMU;
    }else if (strcmp(Dist->name, Methods[RSD]) == 0 ){
        Dist->maxdeg = DecoderObj->sizek + 1;
    }else if (strcmp(Dist->name, "OWN") == 0 ){
    //Not yet defined. 
    }

	/*************************************************** DISTRIBUTION PARAMETERS SET - END ***************************************/

	/*************************************************** DECODER RELATED PARAMETERS SET - BEGIN **********************************/
	

	blocksize = DecoderObj->sizen/disks;
    md = DISK_INDX_STRNG_LEN;
	
	//erased = (int *)malloc(sizeof(int)*(disks));
	erased = (int *)AllocObject(sizeof(int)*(disks));
	for (i = 0; i < disks; i++)
		erased[i] = 0;
	// Determine the missing files and erasures.
	for (i = 1; i <= disks; i++) {
	    sprintf(fname, "%s/Coding/%s_disk%0*d%s", curdir, cs1, md, i, extension);
	    fp = fopen(fname, "rb");
	    if (fp == NULL) {
	        erased[i-1] = 1;
	        printf(WARNINGMSG "Erased file detected: %s\n" RESET, fname);
	    }else{
	        fclose(fp);
	    }
	}
	
	
	/*************************************************** DECODER RELATED PARAMETERS SET - END ************************************/
    
    
    diff.tv_sec = 0; diff.tv_usec = 0;
    gettimeofday(&tic, NULL);
    /*************************************************** DECODER PARAMETERS SET - BEGIN ******************************************/
   	/* Set the distribution values: maxdeg and name are already set , cdf is not. */
    SetDistribution(Dist); // set CDF 
    /*Allocate memory for Decoder Object*/
    DecoderObj = (decoder_t *)AllocateMemDec(DecoderObj, DecoderObj->sizek*DecoderObj->sizet, CSIZE);
	/*Prepare decoder for decoding computation*/
    DecoderObj = (decoder_t *)PrepareDec(DecoderObj, Dist, erased, disks);
    if (numOfThreads > 1)/*If multi-threaded, find the best decoding path*/ 
    	decoding_path = (int *)prepare_decoding_path_mt(DecoderObj);
	/*************************************************** DECODER PARAMETERS SET - END ********************************************/
    gettimeofday(&toc, NULL);
    time_diff(&diff, &tic, &toc); 
   
    timeMB = (double)(diff.tv_sec*ONEM + diff.tv_usec)/(ONEM);
    printf(BOLD KMAG "Prep. Time: %lf GB/sec\n" RESET, bytesMB/timeMB/1024 );
	
    diff.tv_sec = 0; diff.tv_usec = 0;
	int n = 1;
	total = 0;
	int unrecovered_symbols = DecoderObj->unrecovered;
	while(n <= readins){ // blocksize: number of coding symbols per disk.
	    for(i = 0; i < disks; i++) 
	        blocks[i] = DecoderObj->encdata+(i*blocksize*DecoderObj->sizet/(int)sizeof(sym_t)); 
		
	    for(i = 1; i <= disks; i++) {
	        if(erased[i-1] == 0){ // If unerased. 
	            sprintf(fname, "%s/Coding/%s_disk%0*d%s", curdir, cs1, md, i, extension);
	            fp = fopen(fname, "rb");
	            //printf("file offset is %d\n", blocksize*DecoderObj->sizet*(n-1));
	            fseek(fp, blocksize*DecoderObj->sizet*(n-1), SEEK_SET); 
	            //fp2 = open(fname, O_RDONLY, 0644);
	            if (fp != NULL){
	                //readinBytes = read(fp2, DecoderObj->encdata+((i-1)*blocksize*DecoderObj->sizet/(int)sizeof(sym_t)), blocksize*DecoderObj->sizet);           	                       
	                readinBytes = fread(blocks[i-1], sizeof(char), blocksize*DecoderObj->sizet, fp);    
	                //printf("%d -- %d\n", readinBytes, blocksize*DecoderObj->sizet);
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
	    gettimeofday(&tic, NULL);
	    //printf("unrecovered: %d\n",DecoderObj->unrecovered);
	    
	    // Run main decoder:
	    if (numOfThreads == 1){
	    	DecoderObj = (decoder_t *)runBP(DecoderObj);
	    }else{
	    	DecoderObj->unrecovered = unrecovered_symbols;
	    	//DecoderObj = (decoder_t *)runBP_mt(DecoderObj, numOfThreads);
	    	DecoderObj = (decoder_t *)runBP_mt_advance(DecoderObj, numOfThreads, decoding_path);
	    }
	    gettimeofday(&toc, NULL);
	    time_diff(&diff, &tic, &toc);
	    //printf("%d...\n",DecoderObj->success);
	    // If unsuccesful, run decoder for precode:
	    if (DecoderObj->success == 0){
	        if (n==1)
	            printf(WARNINGMSG "Warning: Precode decoding is taking place...\n" RESET);
	        gettimeofday(&tic, NULL);
	        DecoderObj = (decoder_t *)runBP4Precode(DecoderObj);
	        gettimeofday(&toc, NULL);time_diff(&diff, &tic, &toc);
	    }
	    
	    //printf("%d...\n",DecoderObj->success);
	    /* Create decoded file */
		sprintf(fname, "%s/Coding/%s_decoded%s", curdir, cs1, extension);
		if(n==1){
	        fp2 = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	    }else{
	        fp2 = open(fname, O_WRONLY | O_APPEND | O_EXCL, 0644);
	    }
	    
	    if (n < readins){
	        writeinBytes = write(fp2, DecoderObj->srcdata , DecoderObj->sizesb*DecoderObj->sizet);
	    }else{
	        writeinBytes = write(fp2, DecoderObj->srcdata , filesize - total);
	    }
        if(writeinBytes != DecoderObj->sizesb*DecoderObj->sizet && n <readins){
            printf(ERRORMSG "Error:  An unresolved write error is detected. The file might be corrupted.\n" RESET);
            exit(0);
        }
        total += DecoderObj->sizesb*DecoderObj->sizet;
        close(fp2);
	    n++;
	}

    //for (i = 0; i < disks; i++)
    //    DecoderObj->encdata+(i*blocksize*DecoderObj->sizet/(int)sizeof(sym_t)) = blocks[i];
	

	if(DecoderObj->success == 1)
	    printf(COMMENT "Comment: Decoding is Successful!\n" RESET);
	if(DecoderObj->success == 0)
	    printf(WARNINGMSG "Warning: Decoding is Unsuccessful! | %d unrecovered symbols.\n" RESET,DecoderObj->unrecovered);    
	    
    gettimeofday(&toc2, NULL);  
    time_diff(&diff2, &tic2, &toc2); 

    timeMB = (double)(diff.tv_sec*ONEM + diff.tv_usec)/ONEM;
    timeMB2 = (double)(diff2.tv_sec*ONEM + diff2.tv_usec)/ONEM;
    printf(BOLD KMAG "Decode Time: %lf MB/sec\n" RESET, bytesMB/timeMB );
    printf(BOLD KMAG "Total Time: %lf MB/sec\n" RESET, bytesMB/timeMB2 );
	
	free(Dist);
	free(blocks);
	free(DecoderObj->encdata);
	//free(DecoderObj->srcSymbolArray);
	free(DecoderObj->encSymbolArray);
	//free(DecoderObj->srcdata);
    //free(DecoderObj);
	
    return 0;
}
