CC=gcc

SOURCE=main.c
OUT=main.elf
LIB=libmosquitto.so libmariadb.so 
OBJ=main.o
INCLUDES=/usr/include/mysql

all: main.elf

main.elf: main.o
	gcc $(LIB) $(OBJ) -I $(INCLUDES) -o $(OUT)

main.o: main.c
		gcc -c $(SOURCE) -I $(INCLUDES)
clean:
	rm -rf *.out *.elf *.o
