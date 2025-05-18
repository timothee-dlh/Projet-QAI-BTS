CC=clang

SOURCE=main.c
OUT=main.elf
LIB=lib/libmosquitto.a lib/libmariadb.a lib/libmariadbclient.a lib/libssl.a lib/libcrypto.a
OBJ=main.o
INCLUDES=./include
FLAGS=-O2 -static -v

all: main.elf

main.elf: main.o
	$(CC) $(FLAGS) -I $(INCLUDES) -L $(PWD)/lib $(OBJ) $(LIB)   -o $(OUT)

main.o: main.c
		$(CC) -c $(SOURCE) -I $(INCLUDES)
clean:
	rm -rf *.out *.elf *.o
