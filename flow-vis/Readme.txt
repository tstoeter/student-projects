FlowVis2D
An image-based flow visualization demo
Version 1.1

By Torsten Stoeter, Aug 25, 2008
torsten dot stoeter at st dot ovgu dot de

Changelog
Version 1.1
	- added texture advection

This program demonstrates my implementation of five different techniques for
visualizing steady 2D vector fields, I read about in the flow visualization
class I took. Namely these techniques are: Arrow Plots, Integrate & Draw, Line
Integral Convolution (LIC), Spot Noise and Texture Advection. The program
creates a random vector field on a regular 5x5 grid, visualizing it using one of
the above methods. The magnitude of the vectors can be color coded with either
color scale and critical points will be shown if desired. Running the demo,
you'll see a menu to make your choices of visualization technique, color scale
and if you want to display critical points or not. Please note, that
computations may take some time, especially for Spot Noise images.

To build the program from source code you'll need to install the Allegro
library. You can download Allegro at http://alleg.sourceforge.net/. Then simply
call make -f on the appropriate Makefile (.unix or .win respectively):

$ make -f Makefile.unix

Source code and binaries are provided free of charge and without any warranty.
Use it at your own risk. You may use my code in your own projects, informing
me about where and how you use it.

