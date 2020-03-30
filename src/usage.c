/* *
 * Copyright (c) 2015, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * Part of the contents of this file is inspired by IBM folks Mr. Blair
 * and Dr. Walker at Austin, TX. I hereby do acknowledge their 
 * contribution. 
 *
 * usage.c file for function definitions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

#include "usage.h"
#include "parameter.h"

#define _next_char(string) (char)(*(string+1))

char *optarg = NULL;
int optind = 1;
int opterr = 1;

void usage( const char *programme ){

    printf(PRINTMENU);
	printf("+=========================================================================+\n");
	printf("|Usage information: founsureEnc -f <filename> 				  |\n");
	printf("|Alternative 	  : founsureEnc -f <filename> <options> 		  |\n");
	printf("+-------------------------------------------------------------------------+\n");
	printf("|Example          : founsureEnc -f filename -k 10000 -n 12000 -t 128	  |\n");
	printf("+-------------------------------------------------------------------------+\n");
	printf("| General Options: 							  |\n");
	printf("| -f <filename>			: input file name			  |\n");
	printf("| -d <distribution name>        : the name of the degree distribution     |\n");
	printf("|" KWHT "				  Currently Supported: 			  " KGRN "|\n");
	printf("|" KWHT "			  	  " CSD1 "  " CSD2 " 		  	  " KGRN "|\n");
    printf("| -p				: the name of the precode	          |\n");
    printf("|" KWHT "				  Currently Supported: 			  " KGRN "|\n");
	printf("|" KWHT "				  " CSP1 "  " CSP2 " 		          " KGRN "|\n");
	printf("| -k				: number of source blocks	          |\n");
	printf("| -n				: number of coding blocks 	   	  |\n");
	printf("| -t 				: symbol size (in bytes) 		  |\n");
	printf("| -g				: op.mode (0=slow mode, 1=fast mode)  	  |\n");
    printf("| -v				: verbose printing		 	  |\n");
    printf("| -h 				: help option (prints this menu)          |\n");
	printf("+-------------------------------------------------------------------------+\n");
    printf("| Advanced Options: 							  |\n");
    printf("| -c	 		 	: precode target code rate (default: 0.97)|\n");
    printf("| -m	 		 	: number of threads (MT option)	 	  |\n");
    printf("| -s	 		 	: number of drives			  |\n");
    printf("| -i	 		 	: data distribution policy		  |\n");
	printf("+=========================================================================+\n");
    printf(RESET);
}

void usage_dec( const char *programme ){

    printf(PRINTMENU);
	printf("+=========================================================================+\n");
	printf("|Usage information: founsureDec -f <filename> 				  |\n");
	printf("|Alternative 	  : founsureDec -f <filename> <options> 		  |\n");
	printf("+-------------------------------------------------------------------------+\n");
	printf("|Example          : founsureDec -f filename                         	  |\n");
	printf("+-------------------------------------------------------------------------+\n");    
	printf("| General Options: 							  |\n");
	printf("| -f <filename>			: input file name			  |\n");
	printf("| -g				: op.mode (0=slow mode, 1=fast mode)  	  |\n");
    printf("| -v				: verbose printing		 	  |\n");
    printf("| -h 				: help option (prints this menu)          |\n");
	printf("+-------------------------------------------------------------------------+\n");
    printf("| Advanced Options: 							  |\n");
    printf("| -a	 		 	: advanced decoding enabled               |\n");
    printf("| -m	 		 	: number of threads (MT option)	 	  |\n");
	printf("+=========================================================================+\n");
    printf(RESET);
}

void usage_rep( const char *programme ){

    printf(PRINTMENU);
	printf("+=========================================================================+\n");
	printf("|Usage information: founsureRep -f <filename> 				  |\n");
	printf("|Alternative 	  : founsureRep -f <filename> <options> 		  |\n");
	printf("+-------------------------------------------------------------------------+\n");
	printf("|Example          : founsureRep -f filename                         	  |\n");
	printf("+-------------------------------------------------------------------------+\n");    
	printf("| General Options: 							  |\n");
	printf("| -f <filename>			: input file name			  |\n");
    printf("| -v				: verbose printing		 	  |\n");
    printf("| -h 				: help option (prints this menu)          |\n");
	printf("+-------------------------------------------------------------------------+\n");
    printf("| Advanced Options: 							  |\n");
    printf("| -m	 		 	: number of threads (MT option)	 	  |\n");
	printf("+=========================================================================+\n");
    printf(RESET);
}

void usage_disksim( const char *programme ){

    printf(PRINTMENU);
	printf("+=========================================================================+\n");
	printf("|Usage information: simDisk -f <failures> 				  |\n");
	printf("|Alternative 	  : simDisk -f <failures> <options> 	               	  |\n");
	printf("+-------------------------------------------------------------------------+\n");
	printf("|Example          : simDisk -f failures (a decimal number)             	  |\n");
	printf("+-------------------------------------------------------------------------+\n");    
	printf("| General Options: 							  |\n");
	printf("| -f <failures>			: number of disk failures	          |\n");
	printf("| -x <filesize>			: File size in bytes	                  |\n");
	printf("| -s	 		 	: number of drives			  |\n");
    printf("| -v				: verbose printing		 	  |\n");
    printf("| -h 				: help option (prints this menu)          |\n");
	printf("| -d <distribution name>        : the name of the degree distribution     |\n");
	printf("|" KWHT "				  Currently Supported: 			  " KGRN "|\n");
	printf("|" KWHT "			  	  " CSD1 "  " CSD2 " 		    	  " KGRN "|\n");
    printf("| -p				: the name of the precode	          |\n");
    printf("|" KWHT "				  Currently Supported: 			  " KGRN "|\n");
	printf("|" KWHT "				  " CSP1 "  " CSP2 " 		          " KGRN "|\n");
	printf("| -k				: number of source blocks	          |\n");
	printf("| -n				: number of coding blocks 	   	  |\n");
	printf("+=========================================================================+\n");
    printf(RESET);
}

void usage_genpcm( const char *programme ){

    printf(PRINTMENU);
	printf("+=========================================================================+\n");
	printf("|Usage information: genChecks -f <filename> 				  |\n");
	printf("|Alternative 	  : genChecks -f <filename> <options> 		  	  |\n");
	printf("+-------------------------------------------------------------------------+\n");
	printf("|Example          : genChecks -f filename                         	  |\n");
	printf("+-------------------------------------------------------------------------+\n");    
	printf("| General Options: 							  |\n");
	printf("| -f <filename>			: input file name			  |\n");  
    printf("| -h 				: help option (prints this menu)          |\n");
	printf("| -m				: modify meta data file.(1:True, 0:False) |\n");
	printf("| -e				: number of extra drives for update	  |\n");
	printf("| -v				: verbose printing		 	  |\n");
	printf("+=========================================================================+\n");
    printf(RESET);
}

int getopt(int argc, char *const argv[], const char *opstring){

	static char *pIndexPosition = NULL; 
  	char *pArgString = NULL;
	const char *pOptString;

	if (pIndexPosition != NULL) {
		if(*(++pIndexPosition))
			pArgString = pIndexPosition;
	}
	if(pArgString == NULL){
		if(optind >= argc){
			pIndexPosition = NULL;
			return EOF;
		}

		pArgString = argv[optind++];

		if(('/' != *pArgString) && ('-' != *pArgString)){
			--optind;
			optarg = NULL;
			pIndexPosition = NULL;
			return EOF;
		}

		if(( strcmp(pArgString, "-") == 0) || (strcmp(pArgString, "--") == 0)){
			optarg = NULL;
			pIndexPosition = NULL;
			return EOF;
		}
		pArgString++;	
	}

	if(':' == *pArgString){
		return (opterr ? (int)'?' : (int)':');
	}else if((pOptString = strchr(opstring, *pArgString)) == 0){
		optarg = NULL;
		pIndexPosition = NULL;
		return (opterr ? (int)'?' : (int)*pArgString);
	}else{
		if(':' == _next_char(pOptString)){
			if('\0' != _next_char(pArgString)){
				optarg = &pArgString[1];
			}else{
				if(optind < argc)
					optarg = argv[optind++];
				else{
					optarg = NULL;
					return (opterr ? (int)'?' : (int)*pArgString);
				}
			}
			pIndexPosition = NULL;	
		}else{
			optarg = NULL;
			pIndexPosition = pArgString;
		}
		return (int)*pArgString;
	}
}


