OPENCVLIBS=$(shell pkg-config --libs opencv)
OPENCVINCS=$(shell pkg-config --cflags opencv)
ROOTINCS = -I$(shell root-config --incdir)
ROOTLIBS := $(shell root-config --libs) $(shell root-config --auxlibs)
INCLUDES=$(OPENCVINCS) $(ROOTINCS) -Iinclude

all: bin/test_line_detection bin/display_video

bin/test_line_detection: test/test_line_detection.c 
	g++ $(CFLAGS) $(INCLUDES) -o bin/test_line_detection test/test_line_detection.c $(ROOTLIBS) $(OPENCVLIBS) -Wl,-rpath,$(RPATH)

bin/display_video: src/display_video.c 
	g++ $(CFLAGS) $(INCLUDES) -o bin/display_video src/display_video.c $(ROOTLIBS) $(OPENCVLIBS) -Wl,-rpath,$(RPATH)

