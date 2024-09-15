#ifndef LATTICE_SITE_H
#define LATTICE_SITE_H

class LatticeSite
{
	public:
		enum SiteType {Fluid = 'f', Boundary = 'b', Null = 'n'};

		static const int e[9][2];
		double f[9];

	private:
		static const double w[9];
		static const double omega;

		SiteType type;

		double fEq(int k, double rho, double u[2]);

	public:
		LatticeSite();
		void init(SiteType t, double rho, double u[2]);
		void computeRhoAndU(double& rho, double u[2]);
		void collide(double& rho, double u[2]);
		bool isBoundary();
		bool isFluid();
		void setType(SiteType t);
		void setU(double u[2]);
};

#endif

