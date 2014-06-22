CC=g++
CFLAGS=-O2 -W -Wall -Wextra -pedantic
PROJ=pcvc

ALL: $(PROJ)

$(PROJ): main.o
	$(CC) -o $@ $^

main.o: main.cc
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o

