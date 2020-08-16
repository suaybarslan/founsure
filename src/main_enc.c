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

#include "timing.h"
#include "encoder.h"
#include "usage.h"
#include "parameter.h"
#include "allocate.h"

enum Distribution_Type{FiniteDist, RSD, OWN};
char *Methods[3] = {"FiniteDist", "RSD", "OWN"}; 

int main(int argc, char *argv[]){

  int i, verbose = 0;
  int BSIZE = 0;
  int CSIZE;
  int filesize;
  char *curdir;
  
  // get parameters of the system: 
  int ch, g = 1, disks, numOfThreads, readinBytes, writeinBytes, fileHandler, fh2, fflag = 1;
  FILE *fp2;
  char *s1, *s2, *extension;
  char *fname = 0;
  int md;
  int redundantZeros;
  char *filename = NULL;
  char *precodename = NULL;
  double target_cr, bytesMB, timeMB, timeMB2; // for time measurement.
  stat_t st;
  timeval_t tic, toc, tic2, toc2, diff, diff2;
  
  //Founsure specific arguments:
  sym_t **blocks;
  
  diff2.tv_sec = 0;
  diff2.tv_usec = 0;
  gettimeofday(&tic2, NULL);
   
  // Allocate struct-memory for various objects:
  dist_t *Dist 		= (dist_t *)   AllocObject(sizeof(dist_t));
  encoder_t *EncoderObj = (encoder_t *)AllocObject(sizeof(encoder_t));

  // Check if memory allocation is successful. 
  if(EncoderObj == NULL || Dist == NULL){
    printf(ERRORMSG "\nError:  Insufficient Memory. Memory cannot be allocated!\n" RESET);
    free(EncoderObj);
    free(Dist);
    exit(0);
  }	
  // Set the default values:
  int WRITE_ENABLE      = 1;
  disks                 = DEFAULT_NUMBER_OF_DISKS;
  numOfThreads          = DEFAULT_NUMBER_OF_THREADS;
  Dist->name            = DEFAULT_DIST_NAME;	
  precodename           = DEFAULT_PC_NAME; 
  target_cr             = DEFAULT_PRECODE_RATE;
  EncoderObj->sizesb    = SIZE_K;
  EncoderObj->sizek     = SIZE_K;
  EncoderObj->sizen     = SIZE_N;
  EncoderObj->sizet     = SIZE_T;
  EncoderObj->encSeed   = DEFAULT_SEED; 
  // Next version will compute this value based on the HASH(file ID, name)
  // using off-the-shelf hash functions.   
  
  while( (ch = getopt( argc, argv, "f:d:k:n:t:p:c:g:s:m:h:v" ) ) !=-1 ){
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
    case 'm':
      numOfThreads = atoi(optarg);
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
  // Allocate memory for the pointer array:
  blocks = (sym_t **)malloc(sizeof(sym_t*)*disks);
  // Error check on file
  if (filename == NULL){
    printf(WARNINGMSG "Warning: " "No data file/path is specified.\n");
  }else{
    printf(KWHT "File %s is now being checked..." RESET,filename);
  }
  if (access(filename, F_OK) != 0){
    printf(ERRORMSG "\rError  : No such file exists.                    \t"
	   COMMENT "\nComment: Type founsureEnc -h for help		    \t"
	   "\nComment: 1G Random data shall be generated for testing...       \n" RESET);
    fflag = 0; // No file is found.
    WRITE_ENABLE = 0;
  }else{
    fileHandler = open(filename, O_RDONLY);
    stat(filename, &st);
    printf(COMMENT "\rComment: File is found.                          \n" RESET);
  }
  
  // Error check on symbol size
  if (EncoderObj->sizet % ((int)sizeof(sym_t)*8) != 0){
    printf(ERRORMSG "Error:   Symbol size (-t) is supposed to use increments of %d bytes.\n" RESET, (int)sizeof(sym_t)*8);
    exit(0);
  }

  // Determine the file size:
  filesize = fflag ? (int)st.st_size : OGIG; // 1 gigabyte is the default size.

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
  // Adjust parameter n according to the number of disks and threads:
  while(EncoderObj->sizen%disks != 0 || EncoderObj->sizen%numOfThreads != 0) 
    EncoderObj->sizen++;
    
  int blocksize = EncoderObj->sizen/disks; //How many symbols per disk.
  // Finally check error on k and n
  if (EncoderObj->sizek >= EncoderObj->sizen){
    printf(WARNINGMSG "Warning: " "Parameter n cannot be less than %d: No protection is assumed.\n" RESET, EncoderObj->sizek);
    EncoderObj->sizen = EncoderObj->sizek;
    printf(COMMENT "Comment: " "Parameter n is set to minimum possible.\n" RESET);
    while(EncoderObj->sizen%disks != 0 || EncoderObj->sizen%numOfThreads != 0)
      EncoderObj->sizen++;
    blocksize = EncoderObj->sizen/disks;
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
    if (WRITE_ENABLE != 0)
      printf("WRITE ENABLE         : ENABLED \n");
    printf("Parameter s          : %d Disks \n", disks);
    if (strcmp(precodename, "None") != 0 ){
      printf("Target Precode Rate  : %.3f\n", target_cr);
      printf("Adjusted Precode Rate: %.3f\n", (double)EncoderObj->sizesb/EncoderObj->sizek);
    }  
    printf("Buffersize	     : %d (bytes) \n", BSIZE);
    if ( g == 0 ){
      printf("Parameter g 	     : SLOW MODE\n");
    }else{
      printf("Parameter g 	     : FAST MODE\n");
    }
    printf("Number Of Threads    : %d \n" RESET, numOfThreads);
    printf(BOLD "-----------------------------------------------------------\n" RESET);
  }

  
  // Set the distribution parameter:
  if (strcmp(Dist->name, "FiniteDist") == 0 ){
    Dist->maxdeg = MAXMU;
  }else if (strcmp(Dist->name, "RSD") == 0 ){
    Dist->maxdeg = EncoderObj->sizek + 1;
  }else if (strcmp(Dist->name, "OWN") == 0 ){
    //Not yet defined. 
  }

  diff.tv_sec = 0;
  diff.tv_usec = 0;
  gettimeofday(&tic, NULL);
  //Set the distribution values: maxdeg and name are already set , cdf is not.
  SetDistribution(Dist);
  //allocate memory for encoder and initialize it:
  EncoderObj = (encoder_t *)AllocateMem(EncoderObj, EncoderObj->sizek*EncoderObj->sizet, CSIZE); 
  //prepare encoder for compute:
  EncoderObj = (encoder_t *)PrepareEnc(EncoderObj, Dist, disks);
  
  gettimeofday(&toc, NULL);
  time_diff(&diff, &tic, &toc);

  bytesMB = (double)filesize/OMEG; 

  timeMB = (double)(diff.tv_sec*ONEM + diff.tv_usec)/(ONEM);
  printf(BOLD KMAG "Prep.  Time: %lf GB/sec\n" RESET, bytesMB/timeMB/1024 );

  //create coding folder and write data files:
  // Get current working directory for construction of file names
  curdir = (char*)malloc(sizeof(char)*1000);	
  assert(curdir == getcwd(curdir, 1000));
  //printf("cur dir is %s\n", curdir);
  if(WRITE_ENABLE != 0){
    if(mkdir("Coding", ACCESSPERMS) != 0){
      printf(WARNINGMSG "Warning:  Coding directory exists. Overwriting the data ...\n" RESET);
      //fprintf(stderr, "Founsure is unable to create the directory. Check permissions.\n");
    }
  }
  
  // Break inputfile name into the filename and extension -- this part is borrowed from Jerasure 2.0 encoder implementation. 	
  if(WRITE_ENABLE != 0){
    s1 = (char*)malloc(sizeof(char)*(strlen(filename)+20));
    s2 = strrchr(filename, '/');
    if (s2 != NULL) {
	  s2++;
	  strcpy(s1, s2);
    }
    else {
	  strcpy(s1, filename);
    }
    s2 = strchr(s1, '.');
    if (s2 != NULL) {
      extension = strdup(s2);
      *s2 = '\0';
    }else{
      extension = strdup("");
    }

    md = DISK_INDX_STRNG_LEN;
    fname = (char*)malloc(sizeof(char)*(strlen(filename)+strlen(curdir)+20));
  }
  
  int n = 1;
  // Reset the cumulative time-sum:
  diff.tv_sec = 0;
  diff.tv_usec = 0;
  
  
  //printf("%d :" "%" PRIx64 "\n",i, EncoderObj->srcdata[7924*(EncoderObj->sizet/sizeof(sym_t))]);
  //printf("BSIZE is %d\n", BSIZE);
  if(fflag){
    while((readinBytes = read(fileHandler, EncoderObj->srcdata, BSIZE)) > 0){
      //sym_t = EncoderObj->encdata;
      //printf("%d :" "%" PRIx64 "\n",i, EncoderObj->srcdata[7924*(EncoderObj->sizet/sizeof(sym_t))]);
      gettimeofday(&tic, NULL);
      //Compute the encoded symbols: 
      if (numOfThreads == 1){
      	EncodeComputeFast(EncoderObj);
      }else{
        EncodeComputeFast_mt(EncoderObj, numOfThreads);
      }
      //EncodeCompute(EncoderObj);
      gettimeofday(&toc, NULL);  
      time_diff(&diff, &tic, &toc); 
      /* Set pointers to point to file data */
      if(WRITE_ENABLE !=0){
	    for (i = 0; i < disks; i++)
          blocks[i] = EncoderObj->encdata+(i*blocksize*EncoderObj->sizet/(int)sizeof(sym_t));
            
        for(i = 1; i <= disks; i++) {
          sprintf(fname, "%s/Coding/%s_disk%0*d%s", curdir, s1, md, i, extension);
          if(n==1){
	        fh2 = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	      }else{
	        fh2 = open(fname, O_WRONLY | O_APPEND | O_EXCL, 0644);
	      }
	      writeinBytes = write(fh2, blocks[i-1] , blocksize*EncoderObj->sizet);
	      //printf("write bytes is %d\n", writeinBytes);
	      if(writeinBytes != blocksize*EncoderObj->sizet){
	        printf(ERRORMSG "Error:  An unresolved write error is detected. The file might be corrupted.\n" RESET);
	        exit(0);
	      }
	      close(fh2);
        }
      }
      
      n++;
    }
  }else{
    for(i=0;i<OGIG/BSIZE; i++){
      memset(EncoderObj->srcdata, i, BSIZE);
      gettimeofday(&tic, NULL);
      //Compute the encoded symbols: 
      EncodeComputeFast(EncoderObj);
      gettimeofday(&toc, NULL);
      time_diff(&diff, &tic, &toc);  
    } 
  }
  // write metadata:
  if(fflag){
	  sprintf(fname, "%s/Coding/%s_meta.txt", curdir, s1);
	  fp2 = fopen(fname, "wb");
	  fprintf(fp2, "%s\n", argv[2]);
	  fprintf(fp2, "%s\n", Dist->name);
	  fprintf(fp2, "%s\n", precodename);
	  fprintf(fp2, "%d\n", filesize);
	  fprintf(fp2, "%d\n", EncoderObj->encSeed);
	  fprintf(fp2, "%d %d %d %d\n", EncoderObj->sizesb, EncoderObj->sizek, EncoderObj->sizen, EncoderObj->sizet);
	  fprintf(fp2, "%0*d\n", md, disks); 
	  fprintf(fp2, "%d\n", n-1); // How many readins you have done.
	  fclose(fp2);
  }
  
  //printf("%d\n", );
  gettimeofday(&toc2, NULL);  
  time_diff(&diff2, &tic2, &toc2); 
  
  timeMB = (double)(diff.tv_sec*ONEM + diff.tv_usec)/ONEM;
  timeMB2 = (double)(diff2.tv_sec*ONEM + diff2.tv_usec)/ONEM;
  printf(BOLD KMAG "Encode Time: %lf MB/sec\n" RESET, bytesMB/timeMB );
  printf(BOLD KMAG "Total Time: %lf MB/sec\n" RESET, bytesMB/timeMB2 );


  //for(i=0; i<15; i++)
  //    printf("%d :" "%" PRIx64 "\n",i, EncoderObj->encdata[i]); //print hexadecimal of uint64_t 

  //for(i=7921; i<7927; i++)
  //    printf("%d :" "%" PRIx64 "\n",i, EncoderObj->srcdata[i]); //print hexadecimal of uint64_t

  /*for(i=0; i<1; i++){
    printf("degree is %d\n",EncoderObj->encSymbolArray[i].deg);
    for(j=0; j<EncoderObj->encSymbolArray[i].deg;j++){
      printf("%d ", EncoderObj->encSymbolArray[i].connections[j]);
    }
    printf("\n");
  }*/

  // Deallocate memory "buttom up":
  //free(Dist->cdf);
  //free(Dist);
  free(blocks);
  free(EncoderObj->srcdata);
  free(EncoderObj->encdata);
  free(EncoderObj->srcSymbolArray);
  //free(EncoderObj->encSymbolArray);
  free(EncoderObj);
  return(0);
}
