OPENGL_LIBS=-framework OpenGL -framework GLUT
FREENECT_LIBS=-L /Users/Stefan/Documents/Projects/VirtualChem/lib -l freenect -l usb-1.0
HTOBJECTS=handTracking.o imageProcessing.o
LDFLAGS=$(OPENGL_LIBS) $(FREENECT_LIBS)
CFLAGS=-g -I/Users/Stefan/Documents/Projects/VirtualChem/include/libfreenect

handTracking: $(HTOBJECTS)
	gcc $(LDFLAGS) $(HTOBJECTS) -o $@ 

clean:
	rm -rf *.o handTracking