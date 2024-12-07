
CPSC 453 A4 Virtual Orrery and Real Time Rendering | Benny Liang 30192142

---General Info.---
This assignment was built in Microsoft Visual Studio 2022 Preview (newest version) on Windows 11 (newest public version). 
Note, the download and set up of this program, C++, and skeleton code was followed step by step from the D2L video.

Additional files included are the Panel.cpp and Panel.h for the ImGUI,
and associated files for them in 'thirdparty' folder (From assignment 3).
Orbits and axial rotation are 'realistic' to a certain degree, with respect to Earths axial rotation and orbit rates.

--Bonus Implemented--
Implmented Bonus Category 3 1. (Include all planets of the solar system + Saturns rings). I assumed no dwarf planets like Pluto.
Implmented Bonus Category 2 3. (animate cloud texture for earth).

--Controls--
Use the ImGUI to pause/play, and control the simulation speed.
Note: No command line, or key input controls.

To control the spherical camera:
	Scroll wheel zooms in and out on the cube
	Holding the right mouse button and dragging allows you to rotate the camera around the cube

To add textures to the project, place them in the textures folder and refresh CMakeLists.txt. 
The textures will be copied to the output directory in a directory also called textures.
To path the textures in the program, do: "./textures/<File>" or "textures/<File>".

-- Other Notes --
I rotated the texture 'saturn_ring.png' by 90-degrees such that its longest along the vertical axis, and shortest along the horizonal axis.
If the rings look weird, please rotate the texture .png by 90-degrees.
