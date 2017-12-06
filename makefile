VERSION = 3.02
CC = gcc

OBJ = Peer.o
SRC = Peer.c

  # compiler flags:
  #  -g    fügt debugging information zu ausführender Datei
  #  -Wall fügt die meisten, aber nicht alle, Compiler Warnungen

CFLAGS  = -g -Wall
LDFLAGS = -pthread
BIN = prog
DEPENDFILE = .depend

default: all

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)

Peer.o : Peer.c Peer.h
	$(CC) $(CFLAGS) -c Peer.c

clean:
	rm -rf $(BIN) $(OBJ)

dep: $(SRC)
	$(CC) -MM $(SRC) > $(DEPENDFILE)

-include $(DEPENDFILE)
