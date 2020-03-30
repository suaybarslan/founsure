# what flags you want to pass to the C compiler & linker

AM_CPPFLAGS = -I$(top_srcdir)/src/include
AM_CFLAGS = -Wall -Wstrict-prototypes -O3 -fopenmp -g -I..
AM_LDFLAGS = -O3 -g -fopenmp
LDLIBS = -lm -lmetis
#LFLAGS = -lm
#LDADD = -lm

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS =  founsureEnc \
                founsureDec \
                founsureRep \
                genChecks \
                genAlloc \
                simDisk
                
founsureEnc_SOURCES = main_enc.c 
founsureDec_SOURCES = main_dec.c
founsureRep_SOURCES = main_rep.c
genChecks_SOURCES 	= genChecks.c
genAlloc_SOURCES 	= genAlloc.c
simDisk_SOURCES		= simDisk.c
                  
founsureEnc_CFLAGS = $(AM_CFLAGS)
founsureDec_CFLAGS = $(AM_CFLAGS)
founsureRep_CFLAGS = $(AM_CFLAGS)
genChecks_CFLAGS = $(AM_CFLAGS)
genAlloc_CFLAGS = $(AM_CFLAGS)
simDisk_CFLAGS = $(AM_CFLAGS)
                     
lib_LTLIBRARIES = libfounsure.la

libfounsure_la_SOURCES = allocate.c \
                         decoder.c \
                         encoder.c \
                         fastrand.c \
                         parameter.c \
                         repair.c \
                         timing.c \
                         usage.c
    

#founsureincludedir = $(includedir)/main                     
include_HEADERS = include/allocate.h \
                  include/decoder.h \
                  include/encoder.h \
                  include/fastrand.h \
                  include/parameter.h \
                  include/repair.h  \
                  include/timing.h \
                  include/usage.h \
                  include/metis.h  

libfounsure_la_LDFLAGS = -version-info 2:0:0

noinst_HEADERS = include/timing.h
noinst_LIBRARIES = libtiming.a
libtiming_a_SOURCES = timing.c
libtiming_a_CFLAGS = $(AM_CFLAGS)
                     
LDADD = ../src/libfounsure.la
founsureEnc_LDADD = $(LDADD) ../src/libtiming.a
founsureDec_LDADD = $(LDADD) ../src/libtiming.a
founsureRep_LDADD = $(LDADD) ../src/libtiming.a
genChecks_LDADD = $(LDADD) ../src/libtiming.a
simDisk_LDADD = $(LDADD) ../src/libtiming.a

