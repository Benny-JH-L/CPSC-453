

---ImGui Controls

-- Bezier and Polynomial B-spline (Chaikin) Curve options
Moving Control Point: Hold left mouse button and drag desired point then let go to stop dragging and confirm position.
NOTE: Has a "detect feature" where if you hold and drag (without a point selected already) and move onto a control point, it will be selected

Deleting Control Points: Click the box next to a label "Delete curve points" in the ImGui to enable this option. (cannot place/move points when enabled)
NOTE: adding/moving points is enabled by default.

"Show curve points": When enabled, shows the points that make up the selected curve (These points cannot be moved)
	- this feature carries over to the 3D curve viewer as well.
"Reset Window": Clears all the active control points placed by the user.

-- Orbit Viewer Controls
Rotate Camera: Hold right mouse button down and move mouse to start rotating the camera.
Zoom in/out: Using the mouse scroll wheel, scroll up to zoom in, and scroll down to zoom out. 



--Other 'cool' feature
First place a set of control points and enable "show curve points".
Next switch to "Orbit viewer - Surface of Revolution".
Then switch to "Orbit viewer - curve".
This will show you the control points, and the points that make up the surface.
(Note: if you swap back to "surface of revolution" the control points and surface points won't be shown)


-- Warnings
I don't recommend using the highest iterations for b-spline generation, or high number of 'slices' for 
surface of revolution generation. But looking at smooth stuff is nice :)


-- Bonus 1
Able to modify perspective camera FOV, near clip distance, and far clip distance, also has reset funcionality.
