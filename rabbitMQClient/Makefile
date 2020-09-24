

CC=gcc

FLAGS= -I . -I ./include
LINKS= -L ./lib64 -lrabbitmq -lrt

all: rabbitSend rabbitRecv

rabbitSend:
	gcc -g rabbitSend.c rabbitmq.c $(FLAGS) -o $@ $(LINKS)

rabbitRecv:
	gcc -g rabbitRecv.c rabbitmq.c $(FLAGS) -o $@ $(LINKS)

.PHONE:clean
clean:
	-rm -rf *.o send recv rabbitSend rabbitRecv
