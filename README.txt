Hey!

== If You Want to Try it ==
There are a few things you need in order for this project
to work on your system:
1) A Kinect with a power supply and usb connector
	(power and connector come with the AC adapter)
2) libusb-1.0
	I used homebrew to install my version (1.0.9)
3) OpenGL and GLUT, came already on my Mac :P
4) You will need to fiddle with the Makefile to
	link things to where YOU have them.

Also, I am only really writing code to be compatible with
my system, although it should really work on any Mac with Lion.

== Background ==

This project was started by Stefan Seritan in Spring 2012.
The goal was to use a Kinect to manipulate a 3D
visualization of molecules using hand gestures, as a
sort of chemistry tool or playground. This project,
while having the definite goal described above, is also
for my education purposes, so I will try to make as much
of this my own code, as well as making this open-source.

This project has several layers:
- libfreenect (using libusb) give me access to the Kinect.
To find more information about this project, look in the licenses folder
or go to openkinect.org.

- I am currently planning to use OpenCV to track the hand motions,
gathered from depth data from the Kinect (hopefully making
the hand tracking much simpler than the more complex
contour methods needed for RGB hand tracking).

-Following the successful hand motion code, I will begin on
generating models of molecules from .pdb files, which are
a standard in the computational chemistry field.

-Once I have both the hand motion tracking and molecule generating
code working, it should be a fairly "simple" matter of
overlaying the hand tracking with the generated molecules
and linking up simple events, such as a hand closing and opening,
the movement of hands, and implementing one and two hand gestures.

If this ever makes it somewhere major, please let me know
or give me credit somewhere. :)

(C) 2012 Stefan Seritan
sseritan@gmail.com
