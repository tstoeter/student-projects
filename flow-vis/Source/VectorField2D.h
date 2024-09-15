#ifndef VECTOR_FIELD_2D_H
#define VECTOR_FIELD_2D_H

#include "Integrator.h"

typedef struct
{
	double x, y;
} vector2d;

class VectorField2D : public Integrator
{
	private:
		int dimx, dimy;
		double dx, dy;
		vector2d *vfield;

		void f(double dydt[], double x[], double t);
		void SnapToGrid(int& x, int& y);

	public:
		VectorField2D(vector2d field[], int xdim, int ydim, double deltax, double deltay);
		vector2d GetVectorAt(double x, double y);
		int GetDimX();
		int GetDimY();
};

#endif

