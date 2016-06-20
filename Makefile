CC = g++
CS = gcc
CFLAGS= -lm -g #-O3

OUTPUT = convertft
FILES = decomposition.cpp databasereader.cpp convertft.cpp

OUTPUTDB = dbread
FILESDB = decomposition.cpp databasereader.cpp

OUTPUTP = processraw
FILESP = decomposition.cpp databasereader.cpp processraw.cpp


all:: process convert

dbread:: $(FILESDB)
	$(CC) $(CFLAGS) $(FILESDB) -o $(OUTPUTDB)

boxworld:: $(FILESB)
	$(CC) $(CFLAGS) $(FILESB) -o $(OUTPUTB)

convert:: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o $(OUTPUT)
	
process:: $(FILESP)
	$(CC) $(CFLAGS) $(FILESP) -o $(OUTPUTP)
	
chp:: $(FILESC)
	$(CS) $(CFLAGS) $(FILESC) -o $(OUTPUTC)	

clean::
	rm -f $(OUTPUT)
	rm -f $(OUTPUTP)
	rm -f $(OUTPUTC)

ps::
	cat templateh.ps test.ps templatec.ps > circ.ps
	epstopdf circ.ps
