# Paths
SRC = src
ODIR = build
	
OBJS = manager.o
OBJS += queue.o

# Compilers
CC  = gcc

# Compile flags
CFLAGS = -O3 -fPIC -Wall -g

all: $(PROGRAMS)
	mkdir -p $(ODIR)
	$(CC) -c $(SRC)/manager/manager.c
	$(CC) -c $(SRC)/queue/queue.c
	mv $(OBJS) $(ODIR)
	$(CC) $(CFLAGS) $(ODIR)/queue.o  $(ODIR)/manager.o -o sniffer

# Delete executable & object files
clean:
	rm -f sniffer
	rm -f named_pipes/*
	find . -name '*.o' -print | xargs rm -f
	rm -rf $(ODIR)