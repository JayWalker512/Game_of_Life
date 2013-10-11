Game of Life
============

This is an implementation of Conway's Game of Life, written in C using SDL for graphics. Threading is implemented with POSIX threads.

The aim of this program is to be able to simulate many generations of life very quickly.  

This iteration only makes use of two threads. One thread renders the world to the screen, the other thread runs the simulation. 
