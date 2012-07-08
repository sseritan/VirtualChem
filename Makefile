OPENGL_LIBS=-framework OpenGL -framework GLUT
FREENECT_LIBS=-L /Users/Stefan/Documents/Projects/VirtualChem/lib -l freenect -l usb-1.0
LDFLAGS=$(OPENGL_LIBS) $(FREENECT_LIBS)
CFLAGS=-g -I/Users/Stefan/Documents/Projects/VirtualChem/include/libfreenect

handTracking: handTracking.o
	gcc $(LDFLAGS) -o $@ $<

clean:
	rm -rf handTracking.o handTracking