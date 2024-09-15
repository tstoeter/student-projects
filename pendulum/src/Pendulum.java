public class Pendulum extends Integrator
{
	double l;
	double gamma;
	double[] p = {0,0,0};
	int i;

	public Pendulum(double _l, double _g, double y[], double t, double h)
	{
		l = _l;
		gamma = _g;
		InitIntegrator(IntegrationMethod.RungeKutta, h, 2, y, t);
	}

	public void update(double _p[], int _i)
	{
		p = _p;
		i = _i;
	}

	// second derivative of profile line
	double p2d()
	{
		double p0 = p[(3+i-1)%3];
		double p1 = p[(3+i)%3];
		double p2 = p[(3+i+1)%3];

		return (p0 - 2.0*p1 + p2) / (mh*mh);
	}

	double omega_sqrd()
	{
		return (9.81+p2d())/l;
	}

	public void f(double dydt[], double y[], double t)
	{

		if (y[1] > Math.PI)
			y[1] -= 2*Math.PI;
		else if (y[1] < -Math.PI)
			y[1] += 2*Math.PI;

		dydt[0] = -2.0*gamma*y[0] - omega_sqrd()*y[1];
		dydt[1] = y[0];
	}
}

