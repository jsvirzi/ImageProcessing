OPENCVLIBS=$(shell pkg-config --libs opencv)
OPENCVINCS=$(shell pkg-config --cflags opencv)
ROOTINCS = -I$(shell root-config --incdir)
ROOTLIBS := $(shell root-config --libs) $(shell root-config --auxlibs)
INCLUDES=$(OPENCVINCS) $(ROOTINCS) -Iinclude

all: bin/test_edge_detection bin/display_video_edges

bin/test_edge_detection: test/test_edge_detection.c 
	g++ $(CFLAGS) $(INCLUDES) -o bin/test_edge_detection test/test_edge_detection.c $(ROOTLIBS) $(OPENCVLIBS) -Wl,-rpath,$(RPATH)

bin/display_video_edges: src/display_video_edges.c 
	g++ $(CFLAGS) $(INCLUDES) -o bin/display_video_edges src/display_video_edges.c $(OPENCVLIBS) -Wl,-rpath,$(RPATH)

