Hey!

This project was started by Stefan Seritan in Spring 2012. The goal was to use a Kinect
to manipulate a 3D visualization of molecules using hand gestures, as a sort of 
chemistry tool or playground. This project, while having the definite goal described
above, is also for my education purposes, so I will try to make as much of this my own
code, as well as making this open-source.

This project has several layers:
- libfreenect (using libusb) give me access to the Kinect. To find more information about
this project, navigate to the Kinect folder and view their licenses, contributors, and README.

- I am currently planning to use OpenCV to track the hand motions, gathered from depth data
from the Kinect (hopefully making the hand tracking much simpler than the more complex
contour methods needed for RGB hand tracking).

-Following the successful hand motion code, I will begin on generating models of molecules
from .pdb files, which are a standard in the computational chemistry field, which will
require a small code base there as well.

-Once I have both the hand motion tracking and molecule generating code working, it should
be a fairly "simple" matter of overlaying the hand tracking with the generated molecules
and linking up simple events, such as a hand closing and opening, the movement of hands,
and implementing one and two hand gestures.

If this ever makes it somewhere major, please let me know or give me credit somewhere. :)

(C) 2012 Stefan Seritan
sseritan@gmail.com
