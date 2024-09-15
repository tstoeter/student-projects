#include <stdlib.h>
#include "ParticleAdvection.h"

ParticleAdvection::ParticleAdvection(int num, double ***f, int dx, int dy)
{
	vf = f;
	n = num;
	dimx = dx;
	dimy = dy;

	particles = new double*[n];

	reset();
}

void ParticleAdvection::reset()
{
	#pragma omp parallel for
	for (int i=0; i<n; i++)
	{
		particles[i] = new double[2];

		// dont spawn on the boundary
		particles[i][0] = 1 + (int)(rand() % dimx-1);
		particles[i][1] = 1 + (int)(rand() % dimy-1);
	}
}

ParticleAdvection::~ParticleAdvection()
{
	delete particles;
}

void ParticleAdvection::advect0()
{
	#pragma omp parallel for
	for (int i=0; i<n; i++)
	{
		int x = (int)particles[i][0];
		int y = (int)particles[i][1];

		if (x <= 0 || x >= dimx-1 || y <= 0 || y >= dimy-1)
		{
			particles[i][0] = x = 1 + (int)(rand() % dimx-1);
			particles[i][1] = y = 1 + (int)(rand() % dimy-1);
		}

		particles[i][0] += 50*vf[x][y][0];
		particles[i][1] += 50*vf[x][y][1];
	}
}

