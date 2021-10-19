.PHONY : clean
OBJ = gbsh.o

all: gbsh
gbsh: gbsh.o
	g++ -Wall -g -o gbsh $(OBJ)
%.o: %.c
	g++ -Wall -g -c $<
clean:
	-rm  $(objects)  gbsh.o
	
