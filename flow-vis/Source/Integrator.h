#ifndef INTEGRATOR_H
#define INTEGRATOR_H

class Integrator
{
	public:
		enum IntegrationMethod {Euler, Midpoint, Heun, RungeKutta};

	private:
		int mdim;
		double *my;
		double mt, mh;
		IntegrationMethod mmethod;

		void ComputeEuler(double ys[], double& ts);
		void ComputeMidpoint(double ys[], double& ts);
		void ComputeHeun(double ys[], double& ts);
		void ComputeRungeKutta(double ys[], double& ts);

	public:
		void InitIntegrator(IntegrationMethod method, double h, int dim, double y0[], double t);

		virtual void f(double dydt[], double x[], double t) = 0;

		void SetDimension(int dim);
		void SetInitialValues(double y0[], double t);
		void SetStepSize(double h);
		void SetIntegrationMethod(IntegrationMethod method);

		int GetDimension();
		double GetStepSize();

		void SingleStep(double ys[], double& ts);
		void StepForward(double ys[], double& ts);
		void StepBackward(double ys[], double& ts);
		// void MultiStep(...);
};

#endif

