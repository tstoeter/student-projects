CC = g++
CCFLAGS = -O3 -Wall -fopenmp -DNDEBUG
LDFLAGS = -lglut -lGLU -lGL
HEADERS = $(wildcard src/*.hpp)
SOURCES = $(wildcard src/*.cpp)

fluss: $(HEADERS) $(SOURCES)
	$(CC) $(CCFLAGS) -o fluss $(SOURCES) $(LDFLAGS)

clean:
	rm fluss
