#ifndef VECTOR_FIELD_2D_VISUALIZER_H
#define VECTOR_FIELD_2D_VISUALIZER_H

#include <allegro.h>
#include "VectorField2D.h"
#include "ColorScale.h"


class VectorField2DVisualizer
{
	private:
		VectorField2D& vfield;
		BITMAP *image;
		static const double eps;
		int sign(double x);
		void PermutatePixels(int *p);
		bool IsCritical(vector2d v);

	public:
		VectorField2DVisualizer(VectorField2D& field, BITMAP *bmp);
		void DrawTangents(int spx, int spy, double length);
		void IntegrateAndDraw();
		void LIC(int l);
		void SpotNoise(int n, int size);
		void TextureAdvection(double impact, int gc);
		void ColorizeMagnitude(ColorScale cs);
		void FindCriticalRecursive(int step, int maxsteps, double x1, double y1, double x2, double y2);
		void FindCriticalPoints(int maxsteps);
		bool HasCritical(double x1, double y1, double x2, double y2);
};

#endif

