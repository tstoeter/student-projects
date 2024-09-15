Cuggs :: CUDA Grain Growth Simulation
based on the Monte-Carlo-Potts model
Version 1.0

By Torsten Stoeter, Sep 20, 2010
torsten dot stoeter at st dot ovgu dot de


TODO
	- improve pseudo randum number generator
	- adopt kernels for compute capability 2.x
	- instead of copying output texture, do texture "flipping"

CHANGELOG

INTRO
Cuggs is the CUDA Grain Growth Simulation, a tool for simulating normal grain
growth in the recrystallization process of fused metals. It is based on the
Monte-Carlo-Potts model and has been implemented using CUDA for parallel
execution on the GPU to increase performance. Cuggs allows to simulate and
visualize both 2D and 3D grain structures, loading and storing of grain
structures and performing statistical analysis on the grain distribution.
The program was developed in collaboration with Dr. Dana Zoellner and Stefan
Schaefer of the Department of Material Physics / Institute for Experimental
Physics at the University of Magdeburg.

REQUIREMENTS
A CUDA capable graphics adapter with drivers supporting CUDA 3.x is required to
run this program.

BUILD
To build the program from source code you'll need the CUDA 3.x Toolkit and the
GLUT and GLEW libraries installed. Then simply run make in the source code
subdirectory for building. For convenience a pre-compiled executable is provided
for Windows systems.

USAGE
Cuggs provides a few commandline options for configuring the simulation run and
its output. But you can start cuggs also without any commandline options

$ ./cuggs

which simply simulates a random 2D grain distribution in visual mode.
To get a list of valid commandline options run

$ ./cuggs -help

usage: ./cuggs [options]
default action without any options is to simulate
a random 2d grain distribution in visual mode

options:
  -help           print this help screen
  -load <file>    load <file> as initial grain distribution
  -kt <t>         set simulation temperature <t> as decimal value
  -term <x>       terminate simulation after <x> steps
  -save <file>    save final grain distribution to <file> when terminating
  -stat <x>       print statistics for grain distribution every <x> steps
  -verb           print verbose data on grain distribution every <x> steps
  -dump <x>       dump grain distribution to disk every <x> steps
  -visual         enable visualization

The input data for cuggs has a few constraints you should know. Valid input data
for the different options are:
  -load <file>          24bit .bmp/.bm3 files
  -kt <t>               a positive decimal > 0
  -term/stat/dump <x>   positive integers

As an example you could simulate the grain growth on the Lena picture in visual
mode with a simulation temperature of 2.5 by executing

$ ./cuggs -load lena.bmp -visual -kt 2.5

If you would also like to get statistical output on the grain distribution for
every 100 steps and stop the simulation after 1000 steps, storing the final
distribution you can extend the command above to:

$ ./cuggs -load lena.bmp -visual -kt 2.5 -stat 100 -term 1000 -save lena1k.bmp

CONTROLS
Keyboard
	esc		exit
	space		in visual: start/pause simulation
	r		in 3d: toggle auto rotation
Mouse
	left button	in 3d: rotate volume

DISCLAIMER
Source code and binaries are provided free of charge and without any warranty.
Use it at your own risk. You may use my code in your own projects, informing
me about where and how you use it.

