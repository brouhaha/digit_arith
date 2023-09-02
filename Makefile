CFLAGS = -g -Wall -Wextra
LDFLAGS = -g

digit_arith: digit_arith.o

digit_arith.o: digit_arith.c

clean:
	rm -f *.o digit_arith
