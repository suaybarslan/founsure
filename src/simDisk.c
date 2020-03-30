/* *
 * Copyright (c) 2015, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, ISTANBUL/TURKEY.
 * All rights reserved.
 * 
 * Founsure utility function: simDisk
 * 
 * Definition: This utility function can be used to find number of disk failure tolerance
 * based on the desirable durability set by system admins. By playing with the appropriate 
 * selections, you can obtain the right set of parameters for your system. 
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

#include <signal.h>

#include "timing.h"
#include "encoder.h"
#include "usage.h"
#include "parameter.h"
#include "allocate.h"

static volatile int keepRunning = 1;
 
void intHandler(int dummy) {
    keepRunning = 0;
}

int main(int argc, char *argv[]){

    signal(SIGINT, intHandler);
    
    int i, j, k ,c, disks, ch, failures=0, verbose = 0;
    int numOfCombs = 0, numOfSuccess = 0;
    int *erased, *allzero;
    char *precodename = NULL;
    double target_cr;
    int filesize, redundantZeros;;
    // Allocate struct-memory for various objects:
    dist_t *Dist 		= (dist_t *)   AllocObject(sizeof(dist_t));
    encoder_t *EncoderObj = (encoder_t *)AllocObject(sizeof(encoder_t));
    decoder_t *DecoderObj = (decoder_t *)AllocObject(sizeof(decoder_t));

    // Check if memory allocation is successful. 
    if(EncoderObj == NULL || DecoderObj == NULL || Dist == NULL){
        printf(ERRORMSG "\nError:  Insufficient Memory. Memory cannot be allocated!\n" RESET);
        free(DecoderObj);
        free(Dist);
        exit(0);
    }	
    
    disks                 = DEFAULT_NUMBER_OF_DISKS;
    Dist->name            = DEFAULT_DIST_NAME;	
    precodename           = DEFAULT_PC_NAME; 
    target_cr             = DEFAULT_PRECODE_RATE;
    EncoderObj->sizesb    = DecoderObj->sizesb    = SIZE_K;
    EncoderObj->sizek     = DecoderObj->sizek     = SIZE_K;
    EncoderObj->sizen     = DecoderObj->sizen     = SIZE_N;
    EncoderObj->sizet     = DecoderObj->sizet     = 1; // This parameter is irrelevant for this utility function.
    EncoderObj->encSeed   = DecoderObj->encSeed   = DEFAULT_SEED; 
    // default file size:
    filesize = OGIG;
    
    while( (ch = getopt( argc, argv, "f:x:d:k:n:p:c:s:h:v" ) ) !=-1 ){
        switch( ch ){
            case 'f':
              failures = atoi(optarg);
              break;
            case 'x':
              filesize = atoi(optarg);
              break;
            case 'd':
              Dist->name = optarg;
              break;
            case 'k':
              DecoderObj->sizek = EncoderObj->sizek = atoi(optarg);
              break;
            case 'n':
              DecoderObj->sizen = EncoderObj->sizen = atoi(optarg);
              break;
            case 'p':
              precodename = optarg;
              break;
            case 'c':
              target_cr = atof(optarg);
              break;
            case 's':
              disks = atoi(optarg);
              break;
            case 'h':
              usage_disksim( argv[0] );
              exit(0);
              break;
            case 'v':
              verbose = 1;
              break;
            default:
              usage_disksim( argv[0] );
              exit(0);
              break;
        }
    }
    
    // Precode dependent adjustments
    if (strcmp(precodename, "None") == 0 ){
        redundantZeros = AdjustParamNoPrecode(EncoderObj, filesize);
        EncoderObj->sizesb = EncoderObj->sizek;
    }else if (strcmp(precodename, "ArrayLDPC") == 0 ){
        EncoderObj->sizesb = EncoderObj->sizek;
        redundantZeros = AdjustParamWithPrecode(EncoderObj, filesize, target_cr);
    }else{
        printf(WARNINGMSG "Warning: " "Precode" KCYN " %s " KYEL "is not recognized. Switching to default (No precode)\n" RESET, precodename);
        precodename = "None"; //Default is "None" when the selection is unrecognized.
        redundantZeros = AdjustParamNoPrecode(EncoderObj, filesize);
        EncoderObj->sizesb = EncoderObj->sizek;
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
    int symsPerDisk = EncoderObj->sizen/disks; //How many symbols per disk.
    // Finally check error on k and n
    if (EncoderObj->sizek >= EncoderObj->sizen){
        printf(WARNINGMSG "Warning: " "Parameter n cannot be less than %d: No protection is assumed.\n" RESET, EncoderObj->sizek);
        EncoderObj->sizen = EncoderObj->sizek;
        printf(COMMENT "Comment: " "Parameter n is set to minimum possible.\n" RESET);
        while(EncoderObj->sizen%disks != 0)
            EncoderObj->sizen++;
        symsPerDisk = EncoderObj->sizen/disks;
    }  
    if (filesize < EncoderObj->sizesb*EncoderObj->sizet){
        printf(ERRORMSG "Error:  Wrong file size for the code parameters selected.\n" RESET);
        free(EncoderObj);
        free(Dist);
        exit(0);
    }
    
    // Copy encoder paraemters to Decoder paraemters:
    DecoderObj->sizesb  = EncoderObj->sizesb;
    DecoderObj->sizek   = EncoderObj->sizek;
    DecoderObj->sizen   = EncoderObj->sizen;

    if (verbose){
        printf(KWHT BOLD "-----------------------------------------------------------\n" RESET);
        printf(KWHT "Distribution Type    : %s\n",Dist->name);
        printf("Precode Type	     : %s\n",precodename);
        printf("Failed Disks	     : %d\n",failures);
        printf("File Size	     : %d\n",filesize);
        if (strcmp(precodename, "None") == 0 )
            printf("Parameter k 	     : %d\n", DecoderObj->sizek);  
        else{
            printf("Interblock k	     : %d\n", DecoderObj->sizesb);
            printf("Parameter k  	     : %d\n", DecoderObj->sizek);
        }
        printf("Parameter n 	     : %d\n", DecoderObj->sizen);
        printf("Parameter t 	     : %d (bytes) -- Irrelevant for this function. \n", DecoderObj->sizet);
        printf("Parameter s          : %d Disks \n", disks);
        if (strcmp(precodename, "None") != 0 ){
          printf("Target Precode Rate  : %.3f\n", target_cr);
          printf("Adjusted Precode Rate: %.3f\n", (double)DecoderObj->sizesb/DecoderObj->sizek);
        }  
        printf(BOLD "-----------------------------------------------------------\n" RESET);
    }
    
    // Set the distribution parameter:
    if (strcmp(Dist->name, "FiniteDist") == 0 ){
        Dist->maxdeg = MAXMU;
    }else if (strcmp(Dist->name, "RSD") == 0 ){
        Dist->maxdeg = DecoderObj->sizek + 1;
    }
    
    allzero = (int *)AllocObject(sizeof(int)*(disks)); for (i = 0; i < disks; i++) allzero[i] = 0;
    //Set the distribution values: maxdeg and name are already set , cdf is not.
    SetDistribution(Dist);
    //allocate memory for encoder and initialize it:
    DecoderObj = (decoder_t *)AllocateMD(DecoderObj); 
    //prepare encoder for compute:
    DecoderObj = (decoder_t *)PrepareDec(DecoderObj, Dist, allzero, disks);
    // Run main decoder:
       
    // Set the erasure info.
    erased = (int *)AllocObject(sizeof(int)*(disks));
    
    if (verbose){
        printf("Combinations:\t\tDecoding Result:\n" RESET);	
        printf(BOLD "-----------------------------------------------------------\n" RESET);	
    }else{
        printf("If this takes more than expected, you can terminate the process by pressing CTRL+C key.\n" RESET);	
    }
	
    for (i=0; i<(1<<disks); i++) {  
        for (j=0,c=0; j<32; j++) if (i & (1<<j)) c++;
        if (c == failures) {
            // Main routin starts here:
            numOfCombs++;
            for (k = 0; k < disks; k++)
		        erased[k] = 0;
            for (j=0;j<32; j++){
                if (i & (1<<j)){
                    erased[j] = 1;
                    if (verbose)
                        printf ("%i ", j+1);
                }
            }       
            for(k=0; k< DecoderObj->sizen; k++){
                // INSERT the erasure information:
                if(erased[k/symsPerDisk] == 1){
                    DecoderObj->encSymbolArray[k].avail = 0;
                }else{
                    DecoderObj->encSymbolArray[k].avail = 1;
                }
            }             
            DecoderObj = (decoder_t *)runBPNoData(DecoderObj);
            if (DecoderObj->success == 0)
	            DecoderObj = (decoder_t *)runBP4PrecodeNoData(DecoderObj);
	        // Print outcome:
            if(DecoderObj->success == 1){
                if (verbose)
                    printf(COMMENT "\t\t\tComment: Decoding is Successful!\n" RESET);
                numOfSuccess++;
            }
            if(DecoderObj->success == 0)
                if (verbose)
                    printf(WARNINGMSG "\t\t\tWarning: Decoding is Unsuccessful! | %d unrecovered symbols.\n" RESET,DecoderObj->unrecovered); 
	        
	            
	        // reset source array:
	        for(k=0; k< DecoderObj->sizen; k++)
	            if (DecoderObj->encSymbolArray[k].avail == 0)
	                DecoderObj->encSymbolArray[k].avail = 1; // initially all available.
	        if(keepRunning == 0)
	            break;
        }
    }

    if (verbose)
        printf(BOLD "-----------------------------------------------------------\n" RESET);	
    printf(BOLD KMAG "\rResult: %d successful recovery out of %d failure combinations.\n" RESET, numOfSuccess, numOfCombs );
 
    
    free(DecoderObj->srcSymbolArray);
    free(DecoderObj->encSymbolArray);
    free(EncoderObj);
    free(DecoderObj);
    free(Dist);
    free(erased);
    return(0);
}
