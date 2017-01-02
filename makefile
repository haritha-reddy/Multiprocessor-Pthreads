CC = gcc
FLAGS = -pthread -w
SRC_GAUSS = gaussian_par.c

all: gauss
	
gauss: $(SRC_GAUSS)
	$(CC) $(STD) $(FLAGS) -o gauss $(SRC_GAUSS)