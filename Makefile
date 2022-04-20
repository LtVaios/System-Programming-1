# Paths
SRC = src
ODIR = build
	
OBJS = manager.o
OBJS += queue.o
OBJS += set.o
OBJS += worker.o

# Compilers
CC  = gcc

# Compile flags
CFLAGS = -O3 -fPIC -Wall -g

all: $(PROGRAMS)
	make clean
	mkdir -p $(ODIR)
	$(CC) -c $(SRC)/manager/manager.c
	$(CC) -c $(SRC)/queue/queue.c
	$(CC) -c $(SRC)/set/set.c
	$(CC) -c $(SRC)/worker/worker.c
	mv $(OBJS) $(ODIR)
	$(CC) $(CFLAGS) $(ODIR)/queue.o  $(ODIR)/manager.o -o sniffer
	$(CC) $(CFLAGS) $(ODIR)/set.o $(ODIR)/worker.o -o worker

# Delete executable & object files
clean:
	rm -f sniffer
	rm -f worker
	rm -f named_pipes/worker*
	rm -f output_files/*.out
	find . -name '*.o' -print | xargs rm -f
	rm -rf $(ODIR)