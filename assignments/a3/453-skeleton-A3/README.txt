
CPSC 453 A3 Bezier, B-Spline Curves, and Surfaces | Benny Liang 30192142

---General Info.---
This assignment was built in Microsoft Visual Studio 2022 Preview on Windows 11 (newest public version). 
Note, the download and set up of this program, C++, and skeleton code was followed step by step from the D2L video.

There are no additional files that were created for this assignment. Everything was done in and built off of the 
"main.cpp" and other related files in the skeleton code, so it should compile and run with no issues.

---ImGui Controls---

---Bezier and Open Polynomial B-spline (Chaikin) Curve options
Moving Control Point: Hold left mouse button and drag desired point then let go to stop dragging and confirm position.
NOTE: Has a "detect feature" where if you hold and drag (without a point selected already) and move onto a control point, it will be selected

Deleting Control Points: Click the box next to a label "Delete curve points" in the ImGui to enable this option. (cannot place/move points when enabled)
NOTE: adding/moving points is enabled by default.

"Show curve points": When enabled, shows the points that make up the selected curve (These points cannot be moved)
	- this feature carries over to the 3D curve viewer as well.
"Reset Window": Clears all the active control points placed by the user.

---Orbit Viewer Controls
"Show Wireframe": Toggle for wireframe/solid surface.
Rotate Camera: Hold right mouse button down and move mouse to start rotating the camera.
Zoom in/out: Using the mouse scroll wheel, scroll up to zoom in, and scroll down to zoom out. 
(Surface of revolution) "Show mesh points & control points": Shows the points that make up the triangle mesh, and control points that make up the b-spline
(Surface of revolution) "Number of Slices": The higher the number, the smoother the surface.
See "Bonus 1" for other implmented camera controls.

---End of ImGui Controls---

---Warnings---
I don't recommend using the highest iterations for b-spline generation, or high number of 'slices' for 
surface of revolution generation. But looking at smooth stuff is nice :)


---Bonus 1---
Able to modify perspective camera FOV, near clip distance, and far clip distance, also has reset funcionality.
