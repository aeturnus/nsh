CC=gcc
CFLAGS=-Wall -Wextra
SOURCES=*.c
OBJECTS=$(SOURCES:.c:.o)
EXECUTABLE=nsh

build: $(SOURCES)
	$(CC) -o $(EXECUTABLE) $(SOURCES)

clean:
	rm *.o $(EXECUTABLE)

test:
	./$(EXECUTABLE)
