OPENGL_LIBS=-framework OpenGL -framework GLUT
FREENECT_LIBS=-L /Users/Stefan/Documents/Projects/VirtualChem/lib -l freenect -l usb-1.0
HTOBJECTS=handTracking.o imageProcessing.o depthStream.o
TIPOBJECTS=testIP.o imageProcessing.o
LDFLAGS=$(OPENGL_LIBS) $(FREENECT_LIBS)
CFLAGS=-g -Wall -I/Users/Stefan/Documents/Projects/VirtualChem/include/libfreenect

all: placeHTLib fakeHT testIP

fakeHT: $(HTOBJECTS) fakeController.o
	gcc $(LDFLAGS) -l ht $(HTOBJECTS) fakeController.o -o $@

handTrackingLib: $(HTOBJECTS)
	ar rcs libht.a $(HTOBJECTS)

placeHTLib: handTrackingLib
	mv libht.a ../lib/

testIP: $(TIPOBJECTS)
	gcc $(TIPOBJECTS) -o $@

clean:
	rm -rf *.o *.a fakeHT testIP