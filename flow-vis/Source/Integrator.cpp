#include "Integrator.h"

void Integrator::InitIntegrator(IntegrationMethod method, double h, int dim, double y0[], double t)
{
	SetIntegrationMethod(method);
	SetStepSize(h);
	SetDimension(dim);
	SetInitialValues(y0, t);
}

void Integrator::SetDimension(int dim)
{
	mdim = dim;
	my = new double[mdim];
}

int Integrator::GetDimension()
{
	return mdim;
}

void Integrator::SetInitialValues(double y0[], double t)
{
	mt = t;

	for (int i=0; i<mdim; i++)
		my[i] = y0[i];
}

void Integrator::SetStepSize(double h)
{
	mh = h;
}

double Integrator::GetStepSize()
{
	return mh;
}

void Integrator::SetIntegrationMethod(IntegrationMethod method)
{
	mmethod = method;
}

void Integrator::SingleStep(double ys[], double& ts)
{
	switch (mmethod)
	{
		case Euler:
			ComputeEuler(ys, ts);
		break;

		case Midpoint:
			ComputeMidpoint(ys, ts);
		break;

		case Heun:
			ComputeHeun(ys, ts);
		break;

		case RungeKutta:
			ComputeRungeKutta(ys, ts);
		break;

		default:
			ComputeRungeKutta(ys, ts);
		break;
	}
}

void Integrator::StepForward(double ys[], double& ts)
{
	this->SingleStep(ys, ts);
}

void Integrator::StepBackward(double ys[], double& ts)
{
	// do a single step with reverse step size
	mh *= -1;
	this->SingleStep(ys, ts);

	// change step size back to original
	mh *= -1;
}

void Integrator::ComputeEuler(double ys[], double& ts)
{
	double A[mdim];

	f(A, my, mt);

	for (int i=0; i<mdim; i++)
	{
		my[i] = my[i] + A[i]*mh;
		ys[i] = my[i];
	}

	mt += mh;	
	ts = mt;
}

void Integrator::ComputeMidpoint(double ys[], double& ts)
{
	double A[mdim];
	double ymid[mdim];
	double h2 = mh/2;

	f(A, my, mt);

	for (int i=0; i<mdim; i++)
		ymid[i] = my[i] + A[i]*h2;

	f(A, ymid, mt+h2);

	for (int i=0; i<mdim; i++)
	{
		my[i] = my[i] + A[i]*mh;
		ys[i] = my[i];
	}

	mt += mh;	
	ts = mt;
}

void Integrator::ComputeHeun(double ys[], double& ts)
{
	double A[mdim], B[mdim];
	double ytmp[mdim];

	f(A, my, mt);

	for (int i=0; i<mdim; i++)
		ytmp[i] = my[i] + A[i]*mh;

	mt += mh;

	f(B, ytmp, mt);

	for (int i=0; i<mdim; i++)
	{
		my[i] = my[i] + (A[i]+B[i])/2 * mh;
		ys[i] = my[i];
	}

	ts = mt;
}

void Integrator::ComputeRungeKutta(double ys[], double& ts)
{
	double A[mdim], B[mdim], C[mdim], D[mdim];
	double ytmp[mdim];
	double h2 = mh/2;

	f(A, my, mt);

	for (int i=0; i<mdim; i++)
		ytmp[i] = my[i] + A[i]*h2;

	f(B, ytmp, mt+h2);

	for (int i=0; i<mdim; i++)
		ytmp[i] = my[i] + B[i]*h2;

	f(C, ytmp, mt+h2);

	for (int i=0; i<mdim; i++)
		ytmp[i] = my[i] + C[i]*mh;

	f(D, ytmp, mt);

	for (int i=0; i<mdim; i++)
	{
		my[i] = my[i] + (A[i]+2*B[i]+2*C[i]+D[i])/6 * mh;
		ys[i] = my[i];
	}

	mt += mh;
	ts = mt;
}

