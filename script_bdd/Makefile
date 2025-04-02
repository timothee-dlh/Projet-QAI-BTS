CC=gcc

SOURCE=main.c
OUT=main.elf
LIB=libmosquitto.so
OBJ=main.o

all: main.elf

main.elf: main.o
	gcc $(LIB) $(OBJ) -o $(OUT)

main.o: main.c
		gcc -c $(SOURCE) 
clean:
	rm -rf *.out *.elf *.o
