Vertically Driven Pendulum
==========================

Version 1.0

By Torsten Stöter, Jan 12, 2012
torsten dot stoeter at st dot ovgu dot de


INTRO
-----

This special pendulum is not only swinging due to an initial displacement, but
it is also driven by vertical motions of the pendulum's mount. Two java applets
demonstrate this pendulum's nonlinear dynamics. This Java program shows the
swinging motion for a user defined excitation function, dampening and initial
displacement.

The pendulum's behavior can be described by this non-linear second-order
differential equation

    ẍ + 2γẋ + ω²x = 0,

where ω is an excitation function and γ is the dampening. The excitation
function ω² is give by

    ω² = (g + ḧ) / l,

where g is the gravitational constant, l is the rod length of the pendulum and h
is the height profile driving the pendulum.

To solve the pendulum's equations of motion numerically the classical
Runge-Kutta method is employed in combination with finite differences for the
second-order derivative of the height profile h.


BUILD
-----

To build the program from source code you'll need the Java 8+ JDK and the Ant
build tool. Then simply run ant from the base directory for building. For
convenience a pre-compiled executable JAR is provided, too.


USAGE
-----

With the built artifact, simply execute 

$ java -jar pendulum.jar

to run the program. In the user interface the excitation function h(t), the
initial deflection, and dampening can be set. The function h(t) can be written
using common mathematical expressions and is automatically parsed.

Pressing the run button starts visualizing the pendulum's dynamics.


LICENSE
-------

This software is licensed under the 3-Clause BSD license.

