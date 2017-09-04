SRC = libprovenance.c provenanceProvJSON.c provenanceutils.c provenancefilter.c relay.c
OBJ = $(SRC:.c=.o)
OUT = libprovenance.so
INCLUDES = -I../threadpool -I../include -I../uthash/uthash/src
CCFLAGS = -g -O2 -fpic
CCC = gcc
LDFLAGS = -Wl,--whole-archive ../threadpool/thpool.a -Wl,--no-whole-archive -shared

.SUFFIXES: .c

all: dynamic

dynamic: $(OUT)

.c.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CCC) $(OBJ) -o libprovenance.so $(LDFLAGS)

clean:
	rm -f $(OBJ) $(OUT)
	rm -f *.a

install:
	cp --force ./libprovenance.so /usr/lib/libprovenance.so
	sudo ldconfig
