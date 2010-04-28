# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles testLocks testConds exittest exectest forktest lockTest1 lockTest2 AirportLC AirportLiaison Passenger PassC

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

testLocks.o: testLocks.c
	$(CC) $(CFLAGS) -c testLocks.c
testLocks: testLocks.o start.o
	$(LD) $(LDFLAGS) start.o testLocks.o -o testLocks.coff
	../bin/coff2noff testLocks.coff testLocks

lockTest1.o: lockTest1.c
	$(CC) $(CFLAGS) -c lockTest1.c
lockTest1: lockTest1.o start.o
	$(LD) $(LDFLAGS) start.o lockTest1.o -o lockTest1.coff
	../bin/coff2noff lockTest1.coff lockTest1

lockTest2.o: lockTest2.c
	$(CC) $(CFLAGS) -c lockTest2.c
lockTest2: lockTest2.o start.o
	$(LD) $(LDFLAGS) start.o lockTest2.o -o lockTest2.coff
	../bin/coff2noff lockTest2.coff lockTest2

testConds.o: testConds.c
	$(CC) $(CFLAGS) -c testConds.c
testConds: testConds.o start.o
	$(LD) $(LDFLAGS) start.o testConds.o -o testConds.coff
	../bin/coff2noff testConds.coff testConds

exittest.o: exittest.c
	$(CC) $(CFLAGS) -c exittest.c
exittest: exittest.o start.o
	$(LD) $(LDFLAGS) start.o exittest.o -o exittest.coff
	../bin/coff2noff exittest.coff exittest

exectest.o: exectest.c
	$(CC) $(CFLAGS) -c exectest.c
exectest: exectest.o start.o
	$(LD) $(LDFLAGS) start.o exectest.o -o exectest.coff
	../bin/coff2noff exectest.coff exectest

forktest.o: forktest.c
	$(CC) $(CFLAGS) -c forktest.c
forktest: forktest.o start.o
	$(LD) $(LDFLAGS) start.o forktest.o -o forktest.coff
	../bin/coff2noff forktest.coff forktest

AirportLiaison.o: AirportLiaison.c
	$(CC) $(CFLAGS) -c AirportLiaison.c
AirportLiaison: AirportLiaison.o start.o
	$(LD) $(LDFLAGS) start.o AirportLiaison.o -o AirportLiaison.coff
	../bin/coff2noff AirportLiaison.coff AirportLiaison

AirportLC.o: AirportLC.c
	$(CC) $(CFLAGS) -c AirportLC.c
AirportLC: AirportLC.o start.o
	$(LD) $(LDFLAGS) start.o AirportLC.o -o AirportLC.coff
	../bin/coff2noff AirportLC.coff AirportLC

Passenger.o: Passenger.c
	$(CC) $(CFLAGS) -c Passenger.c
Passenger: Passenger.o start.o
	$(LD) $(LDFLAGS) start.o Passenger.o -o Passenger.coff
	../bin/coff2noff Passenger.coff Passenger

PassC.o: PassC.c
	$(CC) $(CFLAGS) -c PassC.c
PassC: PassC.o start.o
	$(LD) $(LDFLAGS) start.o PassC.o -o PassC.coff
	../bin/coff2noff PassC.coff PassC

clean:
	rm -f *.o *.coff
