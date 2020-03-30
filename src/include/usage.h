/* *
 * Copyright (c) 2015, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * Usage.h file for main routine declerations.
*/

#ifndef USAGE_H
#define USAGE_H

//Color Codes:
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#define RESET "\033[0m"
#define BOLD  "\033[1m"
#define CLEAR "\033[2J"
#define UNDERLINE "\033[4m"

// Manual Color Codes:
#define PRINTMENU KGRN BOLD
#define ERRORMSG  KRED BOLD
#define WARNINGMSG KYEL BOLD
#define COMMENT KGRN BOLD

extern char* optarg;
extern int optind;

void usage(const char *programme);

void usage_dec( const char *programme );

void usage_rep( const char *programme );

void usage_disksim( const char *programme );

void usage_genpcm( const char *programme );

int getopt(int argc, char *const *argv, const char *opstring);


#endif /*USAGE_H*/
