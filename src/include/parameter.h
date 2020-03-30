/* *
 * Copyright (c) 2015, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * parameter.h file for parameter adjustment.
*/

#include "encoder.h"

#ifndef PARAMETER_H
#define PARAMETER_H

//Precode Definitions:
#define DIFF_TH 		          5 	        // Threshold for allowed error in determining the value of source block k.
#define RRATE_TH 		          0.001 	    // Threshold for allowed error in the value of precode rate.
#define TRIES_TH 		          500 	    	// Threshold on the number of tries before incrementing DIFF_TH and RRATE_TH
#define SECS_TH                   1           	// Threshold on the number of seconds within which a precode rate must be found.
#define DELTA_DIFF_TH	 	      10 	        // Increment for DIFF_TH
#define DELTA_RRATE_TH 		      0.0002	    // Increment for RRATE_TH
#define RED_BYTE_TH 		      200000 	    // Threshold for Maximum number of redundant/zero bytes (default value)
#define TARGET_CR	 	          0.97	    	// Target Precode Code rate (default value)
#define ARRAY_MIN_J               2           	// Minimum Array LDPC "j" parameter value.
#define ARRAY_MIN_K               2            	// Minimum Array LDPC "k" parameter value.
#define RANDOM_WIN_MAX 		      60          	// Random number search window max value.
#define RANDOM_WIN_MIN 		      60          	// Random number search window min value. 
#define MIN_PRIME		          2           	// Minimum prime number.
#define DEFAULT_PRECODE_RATE 	  0.97 	    	// Default precode rate.
#define DEFAULT_DIST_NAME 	      "FiniteDist"	// Default Distribution Name.
#define DEFAULT_PC_NAME 	      "None" 	    // Default Precode Name.
#define DEFAULT_NUMBER_OF_DISKS   10          	// Default number of disks to which we write data.
#define DEFAULT_NUMBER_OF_THREADS 1				// Default number of threads Encoder/Decoder use. 
#define DEFAULT_SEED 		      1389488782  	// Default seed number for any file (this will be a function of file properties).

//I/O parameters/file names
#define DISK_INDX_STRNG_LEN		  4				// This is the the number of digits used after *disk for encoded files. 
//Currently supported parameters:
#define CSD1 					  "'FiniteDist'"
#define CSD2 					  "'RSD'"
#define CSP1 					  "'None'"
#define CSP2 					  "'ArrayLDPC'"

// Default encoder/decoder values:
#define SIZE_K 					  8192			// Default SOURCE/PRECODED-SOURCE block size.
#define	SIZE_N 					  10000			// Default CODED block size.
#define SIZE_T 					  128			// Default symbol size (in bytes).

// Default repair parameters:
#define ORDER_CHECK_3			  1				// Default order for check 3 is True  (1)
#define ORDER_CHECK_2			  0				// Default order for check 2 is False (0)

//Distribution Variables:
#define MAXMU 					  67			// Maximum degree of the degree distribution.

#define DIST1 					  0.007969		// Probability of 1. 
#define DIST2 					  0.493570		// Probability of 2.
#define DIST3 					  0.166220		// Probability of 3.
#define DIST4 					  0.072646    	// Probability of 4.
#define DIST5 					  0.082558   	// Probability of 5.
#define DIST8 					  0.056058    	// Probability of 8.
#define DIST9 					  0.037229    	// Probability of 9.
#define DIST19 					  0.055590   	// Probability of 19.
#define DIST64 					  0.025023    	// Probability of 64.
#define DIST66 					  0.003135    	// Probability of 66.

#define RSD_GAMMA 				  0.01        	// One of the RSD parameters.
#define RSD_CC    				  0.05       	// One of the RSD parameters.     

//Miscellaneous Variables:
#define ONEM  					  1000000    	// ONE MILLION
#define TENM  					  10000000   	// TEN MILLION
#define HUNM					  100000000		// HUNDRED MILLION
#define OMEG  					  1048576     	// ONE MEGA = 2^20
#define OGIG  					  1073741824  	// ONE GIGA = 2^30
#define OTER  					  1099511627776	// ONE TERA = 2^40
 
int LargestPrimeFactor(int number);

int AdjustParamNoPrecode(encoder_t* codeword, int filesize);

int AdjustParamWithPrecode(encoder_t* EncoderPre, int filesize, double target_cr);


#endif /*PARAMETER_H*/
