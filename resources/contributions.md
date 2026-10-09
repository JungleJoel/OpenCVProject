Kevin Taverner
Component: Camera Input and Validation
Files: CameraInput.h, CameraInput.cpp, main.cpp
Responsible for camera/video source opening, frame capture and validation.

Love Skön
Component: Binarization and Connected-region Labeling
Files: RegionAnalyzer.cpp, RegionAnalyzer.h, test_region_analyzer.cpp
Responsible for turning frames into binary images and extracting filtered candidate regions for marker detection.

Joel Seger
Component: Contour Detection, Line Estimation, Quadliteral Validation
Files: CountorDetection.cpp, CountorDetection.h, LineEstimation.cpp, LineEstimation.h, IsValidQuad.cpp, IsValidQuad.h
Responsible for finding countors from the binarized picture, then finding four corners in each countor and drawing lines between them. With the lines in mind we do some math and determine if the object is a valid quadliteral.

Kevin Taverner
Component: Camera Input and Validation
Files: CameraInput.h, CameraInput.cpp, main.cpp
Responsible for camera/video source opening, frame capture and validation.