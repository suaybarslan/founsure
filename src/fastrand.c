/* *
 * Copyright (c) 2015, Suayb S. Arslan
 * MEF University, Ayazaga, Maslak, IST/TURKEY.
 * All rights reserved.
 *
 * fastrand.c file for function definitions.
 */


#include "fastrand.h"

static int32_t g_seed;
//Used to seed the generator.
static inline void fast_srand( unsigned int seed ){
    g_seed = seed;
}
//fastrand routine returns one integer, similar output value range as C lib.
static inline unsigned int fastrand(){
    //g_seed = ((214013*g_seed + 2531011)) & 0x7fffffff;
    g_seed = ((1103515245*g_seed + 12345)) & 0x7fffffff;
    return (unsigned int) g_seed;
}
