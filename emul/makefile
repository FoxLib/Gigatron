CC=g++
SDL=`sdl-config --cflags --libs`
LIBS=-lSDL
WARN=-Wall -Wno-strict-overflow -Wno-unused-result -Wno-unused-variable -Wno-unused-but-set-variable
CFLAGS=-Icore -I. $(SDL) -O3 $(WARN)
OBJ=window.o disasm.o audio.o gamepad.o vga.o cpu.o gigatron.o

all: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LIBS) -o gigatron
	./gigatron

%.o: %.cc gigatron.h
	$(CC) $(CFLAGS) $(LIBS) $< -c -o $@

clean:
	rm -f *.o gigatron
