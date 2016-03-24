CC = gcc
LD = ld
CFLAGS =-Wall -Wextra -g
SOURCES = *.c
EXECUTABLE = nsh

build: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCES) 

clean:
	rm *.o $(EXECUTABLE)

test:
	./$(EXECUTABLE)

mem:
	valgrind --leak-check=yes ./$(EXECUTABLE)

debug:
	gdb ./$(EXECUTABLE)
