OPENGL_LIBS=-framework OpenGL -framework GLUT
FREENECT_LIBS=-L /Users/Stefan/Documents/Projects/VirtualChem/lib -l freenect -l usb-1.0
HTOBJECTS=handTracking.o imageProcessing.o depthStream.o
TIPOBJECTS=testIP.o imageProcessing.o
THTOBJECTS=testHT.o handTracking.o imageProcessing.o
LDFLAGS=$(OPENGL_LIBS) $(FREENECT_LIBS)
CFLAGS=-g -Wall -I/Users/Stefan/Documents/Projects/VirtualChem/include

all: fakeHT testIP testHT

fakeHT: $(HTOBJECTS) fakeController.o
	gcc $(LDFLAGS) -l ht $(HTOBJECTS) fakeController.o -o $@

testIP: $(TIPOBJECTS)
	gcc $(TIPOBJECTS) -o $@

testHT: $(THTOBJECTS)
	gcc $(THTOBJECTS) -o $@

lib: compileLib
	mv libht.a ../lib/

compileLib: $(HTOBJECTS)
	ar rcs libht.a $(HTOBJECTS)

clean:
	rm -rf *.o fakeHT testIP testHT

cleanLib:
	rm -rf ../lib/libht.a