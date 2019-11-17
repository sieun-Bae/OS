CC = gcc -g
PROGS = Final_RR_ Final_RR_user_
OBJ1 = Final_RR_.o
OBJ2 = Final_RR_user_.o

all: $(PROGS)

Final_RR_: $(OBJ1) 
	$(CC) -o $@ $^

Final_RR_user_: $(OBJ2) 
	$(CC) -o $@ $^

clean:
	rm -rf *.o *.txt $(PROGS)
