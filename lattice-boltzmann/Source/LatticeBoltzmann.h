#ifndef LATTICE_BOLTZMANN_H
#define LATTICE_BOLTZMANN_H

#include "LatticeSite.h"
#include "ColorScale.h"

class LatticeBoltzmann
{
	private:
		int dims[2];

		double **rho;
		double ***u;

		LatticeSite **sites;
		LatticeSite **sites_;

		void streamToNeighbors(int x, int y);

	public:
		LatticeBoltzmann(const int d[2]);
		~LatticeBoltzmann();

		void reset();

		LatticeSite getSite(int x, int y);
		void setSite(int x, int y, LatticeSite::SiteType type, double u[2]);

		void getDensityAndVelocityField(double **&rho, double ***&u);
		void stream();
		void update();
};

inline LatticeSite LatticeBoltzmann::getSite(int x, int y)
{
	return sites[x][y];
}

#endif

