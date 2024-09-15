CC = nvcc
CCFLAGS = -O2 --ptxas-options="-v"
LDFLAGS = -lglut -lGLEW -lGLU -lGL
FILES = bmp.cpp cuggs.cpp grains.cu visual.cpp

cuggs: $(FILES)
	$(CC) $(CCFLAGS) -o cuggs $(FILES) $(LDFLAGS)

clean:
	rm cuggs

