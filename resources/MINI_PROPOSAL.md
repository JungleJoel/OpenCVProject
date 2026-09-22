Group Members: Joel Seger, Thor Anderberg Nilsson, Love Skön, Kevin Taverner


## Proposal:


We propose using the marker as a light controller.To begin we will just have
a circle on screen that will represent a lightbulb. With the marker you can
then interact with the light bulb in the follow ways:


When the marker is shown for the camera the light will "turn on" to begin
with this will just be the circle turning a yellowish color. If the marker
then disappears the light "turns off" showing a black circle again.


If the camera can see the marker and the marker is turned clockwise the hue or 
gamma will increase to simulate the "light bulb" increasing in light level.
If you instead turn the marker anti-clockwise the light level (hue and gamma) 
will decrease on the artificial light bulb.


For an optional extension we aim to have a physical ledlamp on an arduino chip 
that we could control with the marker through OpenCV. This should be possible 
by making the program runnable on the chip itself and connecting a camera to it.


