#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "VectorField2D.h"


// defines piecewise vector field on a regular xdim by ydim grid with deltax/deltay spacing in x/y direction
VectorField2D::VectorField2D(vector2d field[], int xdim, int ydim, double deltax, double deltay)
{
	dimx = xdim;
	dimy = ydim;
	dx = deltax;
	dy = deltay;

	vfield = new vector2d[dimx*dimy];

	for (int i=0; i<dimx*dimy; i++)
		vfield[i] = field[i];

	double y0[2] = {0, 0};

	InitIntegrator(RungeKutta, 0.0005, 2, y0, 0);
}

// set derivatives of flow from piecewise given vector field (the vectors actually) for integration
void VectorField2D::f(double dydt[], double x[], double t)
{
	vector2d v = GetVectorAt(x[0], x[1]);
	dydt[0] = v.x;
	dydt[1] = v.y;
}

void VectorField2D::SnapToGrid(int& x, int& y)
{
	if (x < 0)
		x = 0;
	else if (x >= dimx)
		x = dimx-1;

	if (y < 0)
		y = 0;
	else if (y >= dimy)
		y = dimy-1;
}

// bi-cosine interpolation to get vectors in between grid cells
vector2d VectorField2D::GetVectorAt(double x, double y)
{
	int ul, ur, ll, lr;	// upper/lower left/right array indices
	double wh, wv;		// weights horizontally and vertically
	double whh, wvv;	// cosine weights
	double xu, xl;
	vector2d v;
	int xx, yy;

	xx = (int)x;
	yy = (int)y;

	SnapToGrid(xx, yy);	// snap coordinates back to grid if they are out of bounds

	// compute neighboring array indices for linear array
	ul = yy*dimx + xx;
	ur = ul + 1;
	ll = ul + dimx;
	lr = ll + 1;

	// weights for x and y in [0,1] range (from their distances)
	wh = x - xx;
	wv = y - yy;

	whh = (1-cos(wh*3.14159265f))/2;
	wvv = (1-cos(wv*3.14159265f))/2;

	// interpolate first vector component
	xu = (1-whh) * vfield[ul].x + whh * vfield[ur].x;	// horizontal
	xl = (1-whh) * vfield[ll].x + whh * vfield[lr].x;		// horizontal
	v.x = (1-wvv) * xu + wvv * xl;					// vertical

	// second vector component
	xu = (1-whh) * vfield[ul].y + whh * vfield[ur].y;	// horizontal
	xl = (1-whh) * vfield[ll].y + whh * vfield[lr].y;		// horizontal
	v.y = (1-wvv) * xu + wvv * xl;					// vertical

	return v;
}

int VectorField2D::GetDimX()
{
	return dimx;
}

int VectorField2D::GetDimY()
{
	return dimy;
}

