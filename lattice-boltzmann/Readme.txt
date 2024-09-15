LBdemo
A Lattice Boltzmann fluid simulation
Version 1.0

By Torsten Stoeter, Dec 14, 2008
torsten dot stoeter at st dot ovgu dot de


INTRO
This program shows off my implementation of the basic Lattice Boltzmann Method
in two dimensions (D2Q9) with the BGK collision model and bounce-back boundary
conditions. The Lattice Boltzmann equations are solved on a 256x256 cartesian
grid and the computations are accelerated utilizing multiple processor cores if
available. Using the mouse you can stear a fluid within a rectangular container
adding flow to it. You can also draw additional boundaries to create more
complex geometries. The speed of the flow is colorized with a blue to white
color scale and advected particles indicate the direction of flow.

BUILD
To build the program from source code you'll need GLUT and an OpenMP capable C++
compiler for multi-core support. Makefiles for UNIX and Windows platforms are
provided employing the g++ compiler. Simply run

$ make -f Makefile.unix

to build the program on UNIX systems. On Windows systems use Makefile.win
respectively.

CONTROLS
Keyboard
	esc		exit
	r		reset fluid
	p		toggle particles
	v		toggle velocity color
Mouse
	left button 	add flow
	right button	draw boundary

DISCLAIMER
Source code and binaries are provided free of charge and without any warranty.
Use it at your own risk. You may use my code in your own projects, informing
me about where and how you use it.

