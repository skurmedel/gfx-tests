SOURCES=spheretrace.c
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-Wall -std=c99 -O2
EXE=spheretrace.exe

all: $(SOURCES) $(EXE)

$(EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	@rm -rf *.o
	@rm -rf $(EXE)