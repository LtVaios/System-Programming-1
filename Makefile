# Paths
MODULES = src
ODIR = build
	
MAIN_O = main.o
OBJS = worker.o
OBJS += manager.o
OBJS += listener.o

# Compilers
CC  = gcc

# Compile flags
CFLAGS = -O3 -fPIC -Wall -g

all: $(PROGRAMS)
	mkdir -p $(ODIR)
	$(CC) -c $(MODULES)/worker/worker.c
	$(CC) -c $(MODULES)/manager/manager.c
	$(CC) -c $(MODULES)/listener/listener.c
	$(CC) -c $(MODULES)/main.c
	mv $(OBJS) $(MAIN_O) $(ODIR)
	$(CC) $(CFLAGS) $(ODIR)/$(MAIN_O) $(ODIR)/listener.o  $(ODIR)/manager.o $(ODIR)/worker.o -o sniffer

# Delete executable & object files
clean:
	rm -f sniffer
	find . -name '*.o' -print | xargs rm -f
	rm -rf $(ODIR)