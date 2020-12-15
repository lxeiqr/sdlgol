CFLAGS := -Iinclude -Wall -fstack-protector-strong -O2 -pipe \
    -Werror=format-security -I/usr/local/include `sdl2-config --cflags` -Iext
LDFLAGS := -L/usr/local/lib `sdl2-config --libs` -lm

BIN := sdlgol 
OBJ := sdlgol.o

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) 

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -r $(BIN) $(OBJ)

.PHONY: clean
.SUFFIXES: .c .o
