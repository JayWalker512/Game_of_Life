Game of Life
============

This is an implementation of Conway's Game of Life, written in C using SDL2 and OpenGL for graphics. Threading is implemented with POSIX threads.

The aim of this program is to be able to simulate many generations of life very quickly.  

This iteration makes use of only two threads. Main thread renders the world to the screen and handles input, the other thread runs the simulation. 
