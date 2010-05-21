CFLAGS =  `pkg-config --cflags gtk+-2.0` `curl-config --cflags`
LDFLAGS =  `pkg-config --libs gtk+-2.0` `curl-config --libs` -export-dynamic 

SRC=main.c
OBJ=$(SRC:.c=.o)

all: main

main: $(OBJ)
	$(LINK.c) $^ -o $@
