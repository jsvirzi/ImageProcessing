OPENCVLIBS=$(shell pkg-config --libs opencv)
OPENCVINCS=$(shell pkg-config --cflags opencv)
INCLUDES=$(OPENCVINCS) -Iinclude

all: bin/test_entropy

lib/libcamera.so: src/camera.c include/camera.h src/debayer.c
	mkdir -p lib
	g++ $(CFLAGS) -pthread -shared -fPIC $(OPENCVINCS) $(INCLUDES) -o lib/libcamera.so src/camera.c src/colorspaces.c $(OPENCVLIBS) -DFWVERSION='"$(FWVERSION)"' -DGITHASH='"$(GITHASH)"'

bin/test_entropy: test/test_entropy.c src/entropy.c
	g++ $(CFLAGS) $(INCLUDES) -o bin/test_entropy test/test_entropy.c src/entropy.c $(OPENCVLIBS) -Wl,-rpath,$(RPATH)

