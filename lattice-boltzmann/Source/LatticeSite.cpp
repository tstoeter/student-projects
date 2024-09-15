#include "LatticeSite.h"

const int LatticeSite::e[9][2] = {{0,0}, {1,0}, {0,1}, {-1,0}, {0,-1}, {1,1}, {-1,1}, {-1,-1}, {1,-1}};
const double LatticeSite::w[9] = {4.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/9.0, 1.0/36.0, 1.0/36.0, 1.0/36.0, 1.0/36.0};
const double LatticeSite::omega = 0.75;

LatticeSite::LatticeSite()
{

}

void LatticeSite::init(SiteType t, double rho, double u[2])
{
	setType(t);
	
	for (int k=0; k<9; k++)
		f[k] = fEq(k, rho, u);
}

void LatticeSite::computeRhoAndU(double& rho, double u[2])
{
	rho = 0;

	for (int k=0; k<9; k++)
		rho += f[k];

	u[0] = 0;
	u[1] = 0;

	for (int k=0; k<9; k++)
	{
		u[0] += f[k]*e[k][0];
		u[1] += f[k]*e[k][1];
	}

	u[0] /= rho;
	u[1] /= rho;
}

double LatticeSite::fEq(int k, double rho, double u[2])
{
	double u2 = u[0]*u[0] + u[1]*u[1];
	double eu = e[k][0]*u[0] + e[k][1]*u[1];

	return rho * w[k] * (1.0 + 3.0*eu + 4.5*eu*eu - 1.5*u2);
}

void LatticeSite::collide(double& rho, double u[2])
{
	if (type == Boundary)
	{
		int op[9] = {0, 3, 4, 1, 2, 7, 8, 5, 6};
		double tmp[9];

		for (int k=0; k<9; k++)
				tmp[k] = f[k];

		for (int k=0; k<9; k++)
			f[k] = tmp[op[k]];

		rho = 0;
		u[0] = u[1] = 0;
	}
	else if (type == Fluid)
	{
		computeRhoAndU(rho, u);

		for (int k=0; k<9; k++)
		{
			f[k] *= (1.0-omega);
			f[k] += omega*fEq(k, rho, u);
		}
	}
}

bool LatticeSite::isBoundary()
{
	return type == Boundary;
}

bool LatticeSite::isFluid()
{
	return type == Fluid;
}

void LatticeSite::setType(SiteType t)
{
	type = t;
}

void LatticeSite::setU(double u[2])
{
	double rho, uu[2];

	computeRhoAndU(rho, uu);

	uu[0] += u[0];
	uu[1] += u[1];

	for (int k=0; k<9; k++)
		f[k] = fEq(k, rho, u);
}

