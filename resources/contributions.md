Kevin Taverner
Component: Camera Input and Validation, Usable Marker Values and Virtual Lamp
Files: CameraInput.h, CameraInput.cpp, main.cpp, InteractionMapping.h 
InteractionMapping.cpp, LampRenderer.h, LampRenderer.cpp
Responsible for camera/video source opening, frame capture and validation.
Converting marker angle into usable brightness value between 0-100%. Renders a 
virtual lamp that is controlled by this value and also turns on and off by the 
marker detectiong existence.

Love Skön
Component: Binarization and Connected-region Labeling
Files: RegionAnalyzer.cpp, RegionAnalyzer.h, test_region_analyzer.cpp
Responsible for turning frames into binary images and extracting filtered candidate regions for marker detection.

Joel Seger
Component: Contour Detection, Line Estimation, Quadliteral Validation
Files: CountorDetection.cpp, CountorDetection.h, LineEstimation.cpp, LineEstimation.h, IsValidQuad.cpp, IsValidQuad.h
Responsible for finding countors from the binarized picture, then finding four corners in each countor and drawing lines between them. With the lines in mind we do some math and determine if the object is a valid quadliteral.

Thor Anderberg Nilsson
Component: Marker Normalisation and Recognition, Pose Estimation
Files: MarkerRecognizer.h, MarkerRecognizer.cpp, PoseEstimator.h, PoseEstimator.cpp
Responsible for taking the quads passed to me by Joel's component and warping them into flat images. Once there's a flat image of the quad, my component translates it into a bit-matrix in order to determine whether our specific marker is found in the quad, and if so, which way it is rotated. Screenshot: https://i.imgur.com/qkNIWOj.png. Pose Estimation then uses the marker's corners and its real size (measured on the printed paper) to find where the marker is in 3D relative to the camera and draws its axes. It outputs the marker's rotation angle on screen which in turn will be used to control a lightbulb on the screen.