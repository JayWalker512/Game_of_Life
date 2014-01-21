Game of Life
============

This is an implementation of Conway's Game of Life, written in C. It uses SDL2 for cross-platform threading functionality (among other things) and OpenGL for graphics. 

The aim of this program is to be able to simulate many generations of life very quickly.  

This iteration makes use of only two threads. Main thread renders the world to the screen and handles input, the other thread runs the simulation. 

User Input
----------

Keys:

R: Re-loads the file that was loaded using the -l flag. If nothing was loaded, it clears the world to all cells dead.
Z: Randomizes the world state.
Space: Pauses or un-pauses the simulation.
Esc: Quits the simulation

Mouse:

Left-clicking and moving the mouse allows you to translate the world according to mouse movements so that you may view different parts of the world. Using the scroll wheel zooms in and out of the world, to magnify different features of the simulation.

Command Line Arguments
----------------------
-f            Enables fullscreen.
-x <number>   Sets the window width in pixels.
-y <number>   Sets the window height in pixels.
-w <number>   Sets the world width in cells.
-h <number>   Sets the world height in cells.
-r <number>   Sets the region size. This is for performance tuning, numbers less than 8 are generally best. NOTE: The world width and height are automatically scaled up to become multiples of this number. This will change the simulation result when playing in a toroidal world. DEFAULT value is 4, so world dimensions will be scaled up to become divisible by 4 if this parameter is not changed.
-l <path>     Load a .life pattern file in to the simulator.
-z <number>   Sets the number of times per second the world state is drawn to the screen.
