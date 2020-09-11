
CC=gcc

FLAGS=-g -Wall

SRC= src
OBJ= $(SRC)/cJSON.o
OBJ += $(SRC)/msgPacket.o
OBJ += $(SRC)/httpInterface.o

INC= -I ./inc

LINK= -lcurl -lm

$(SRC)/%.o:$(SRC)/%.c
	$(CC) $(FLAGS) $(INC) -c $< -o $@

all: a.out

a.out: $(OBJ)
	$(CC) $(FLAGS) $(INC) main.c $(OBJ) -o $@ $(LINK)

.PHONE:clean
clean:
	rm  -f a.out src/*.o
