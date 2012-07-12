OPENGL_LIBS=-framework OpenGL -framework GLUT
FREENECT_LIBS=-L /Users/Stefan/Documents/Projects/VirtualChem/lib -l freenect -l usb-1.0
HTOBJECTS=handTracking.o imageProcessing.o
TIPOBJECTS=testIP.o imageProcessing.o
LDFLAGS=$(OPENGL_LIBS) $(FREENECT_LIBS)
CFLAGS=-g -I/Users/Stefan/Documents/Projects/VirtualChem/include/libfreenect

all: handTracking testIP

handTracking: $(HTOBJECTS)
	gcc $(LDFLAGS) $(HTOBJECTS) -o $@ 

testIP: $(TIPOBJECTS)
	gcc $(TIPOBJECTS) -o $@

clean:
	rm -rf *.o handTracking testIP