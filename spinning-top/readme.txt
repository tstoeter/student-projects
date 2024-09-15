Brummkreisel
A Real-Time Simulation of a Spinning Top
Version 1.0

By Torsten Stoeter, Feb 5, 2010
torsten dot stoeter at st dot ovgu dot de


INTRO
Brummkreisel (German for spinning toy top) is a small demo simulating the
motions of a spinning top, after it has been nudged. The underlying physically-
based model describing the top's behavior consists of a system of non-linear
second-order differential equations and is solved numerically using the
classical Runge-Kutta method. A first prototype was implemented in MATLAB,
ultimately to develop this three-dimensional real-time simulation, which is
visualized by a 3D model of a "Brummkreisel". Plots of a few time-dependent
physical quantities give additional information about the oscillation, its
amplitude and the rotational speed.

BUILD
To build the program from source code under Linux you'll need the GLUT library
installed. Simply run

$ make -f makefile.unix

in the source code subdirectory to build the program on UNIX systems. On Windows
systems use makefile.win respectively or the Visual C++ project files. For
convenience a pre-built executable is provided for Windows systems.
The MATLAB script is contained in the files kreisel.m and kreiself.m, which
outputs a small animation of a spinning top.

CONTROLS
Keyboard
	esc		exit
	space		nudge the top

DISCLAIMER
Source code and binaries are provided free of charge and without any warranty.
Use it at your own risk. You may use my code in your own projects, informing
me about where and how you use it.

