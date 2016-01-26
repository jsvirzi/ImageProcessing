OPENCVLIBS=$(shell pkg-config --libs opencv)
OPENCVINCS=$(shell pkg-config --cflags opencv)
INCLUDES=$(OPENCVINCS) -Iinclude

all: bin/test_edge_detection

bin/test_edge_detection: test/test_edge_detection.c 
	g++ $(CFLAGS) $(INCLUDES) -o bin/test_edge_detection test/test_edge_detection.c $(OPENCVLIBS) -Wl,-rpath,$(RPATH)

