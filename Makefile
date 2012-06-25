OPENGL_LIBS=-framework OpenGL -framework GLUT
FREENECT_LIBS=-L /Users/Stefan/Documents/Projects/VirtualChem/lib -l freenect -l usb-1.0
LDFLAGS=$(OPENGL_LIBS) $(FREENECT_LIBS)
CFLAGS=-I/Users/Stefan/Documents/Projects/VirtualChem/include/libfreenect

depthStream: depthStream.o
	gcc $(LDFLAGS) -o $@ $<

clean:
	rm -rf depthStream.o depthStream