public abstract class Integrator
{
	public enum IntegrationMethod {Euler, Midpoint, Heun, RungeKutta};

	int mdim;
	double my[];
	double mt, mh;
	IntegrationMethod mmethod;

	private	void ComputeEuler(double ys[])
	{
		double A[] = new double[mdim];

		f(A, my, mt);

		for (int i=0; i<mdim; i++)
		{
			my[i] = my[i] + A[i]*mh;
			ys[i] = my[i];
		}

		mt += mh;
	}

	private	void ComputeMidpoint(double ys[])
	{
		double A[] = new double[mdim];
		double ymid[] = new double[mdim];
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
	}

	private	void ComputeHeun(double ys[])
	{
		double A[] = new double[mdim], B[] = new double[mdim];
		double ytmp[] = new double[mdim];

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
	}

	private	void ComputeRungeKutta(double ys[])
	{
		double A[] = new double[mdim], B[] = new double[mdim], C[] = new double[mdim], D[] = new double[mdim];
		double ytmp[] = new double[mdim];
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

		f(D, ytmp, mt+mh);

		for (int i=0; i<mdim; i++)
		{
			my[i] = my[i] + (A[i]+2*B[i]+2*C[i]+D[i])/6 * mh;
			ys[i] = my[i];
		}

		mt += mh;
	}

	public	void InitIntegrator(IntegrationMethod method, double h, int dim, double y0[], double t)
	{
		SetIntegrationMethod(method);
		SetStepSize(h);
		SetDimension(dim);
		SetInitialValues(y0, t);
	}

	// system of first order ode goes in here
	// yet empty, to be defined by user in a derived class
	public abstract	void f(double dydt[], double x[], double t);

	public	void SetDimension(int dim)
	{
		mdim = dim;
		my = new double[mdim];
	}

	public	void SetInitialValues(double y0[], double t)
	{
		mt = t;

		for (int i=0; i<mdim; i++)
			my[i] = y0[i];
	}

	public	void SetStepSize(double h)
	{
		mh = h;
	}

	public	void SetIntegrationMethod(IntegrationMethod method)
	{
		mmethod = method;
	}

	public	int GetDimension()
	{
		return mdim;
	}

	public	double GetStepSize()
	{
		return mh;
	}

	public	double GetTime()
	{
		return mt;
	}

	public	void SingleStep(double ys[])
	{
		switch (mmethod)
		{
			case Euler:
				ComputeEuler(ys);
			break;

			case Midpoint:
				ComputeMidpoint(ys);
			break;

			case Heun:
				ComputeHeun(ys);
			break;

			case RungeKutta:
				ComputeRungeKutta(ys);
			break;

			default:
				ComputeRungeKutta(ys);
			break;
		}
	}

	public	void StepForward(double ys[])
	{
		SingleStep(ys);
	}

	public	void StepBackward(double ys[])
	{
		// do a single step with reverse step size
		mh *= -1;
		SingleStep(ys);

		// change step size back to original
		mh *= -1;
	}
}

