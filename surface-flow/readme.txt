Fluss :: FLUid Simulation on Surfaces
=====================================
represented by 3D triangle meshes
Version 1.0

By Torsten Stoeter, Aug 17, 2011
torsten dot stoeter at st dot ovgu dot de


TODO
----
* adaptive time stepping using Courant-Friedrichs-Lewy condition
* formulate and solve pressure relaxation using large linear system


INTRO
-----
Fluss implements my final thesis project -- a simple geometric and physically-
based method for simulating fluid flows on curved triangle mesh surfaces.
Previous methods for simulating surface flows require parametrizations of the
surface globally or locally flattening the surface. However, these
parametrizations introduce distortions to the flow. The proposed method operates
directly on the triangle mesh, geometrically computing the fluid flow for
individual triangles and does not need a parametrization of the surface. The
thesis further investigated the numerical and physical properties of the method
and demonstrated its accuracy in resolving fluid flows.


BUILD
-----
To build the program from source code you'll need GLUT and an OpenMP capable C++
compiler for multi-core support. A makefile for UNIX platforms are provided
employing the g++ compiler. Simply run

$ make

to build the program on UNIX systems.


USAGE
-----
Fluss requires several parameters to be set for running simulations, which are
briefly explained in params.txt, while other the txt files offer examples. For
example, the first line defines which surface mesh to load for simulation.

After building, the program can be started from the UNIX command line via, e.g.

$ cat holes.txt | ./fluss > output.log

where the simulation parameters given in holes.txt are passed as input into
fluss and its output is written to output.log.

Next, the provided surface mesh will be rendered in a window for interactive
simulation and visualization of the fluid flow and be be controlled as described
below.


CONTROLS
--------
Keyboard
	esc:	exit
	space:	pause/resume simulation
	f:		fill in dye to be advected
	n:		normalize velocity vectors in visualization
	+/-:	zoom in/out
Mouse
	left button:    rotate mesh
	right button:	translate mesh
	wheel up/down:	zoom in/out


LICENSE
-------
This software is licensed under the 3-Clause BSD license.

