#ifndef COLOR_SCALE_H
#define COLOR_SCALE_H

class ColorScale
{
	private:
		double (*scale)[4];
		int n, c;

	public:
		ColorScale(int num);
		void AddPoint(double v, double r, double g, double b);
		void GetColor(double v, double& r, double& g, double& b) const;
};

ColorScale::ColorScale(int num)
{
	scale = new double[num][4];
	c = 0;
	n = num;
}

// note: points should be added in ascending order
void ColorScale::AddPoint(double v, double r, double g, double b)
{
	if (c < n)
	{
		scale[c][0] = v;
		scale[c][1] = r;
		scale[c][2] = g;
		scale[c][3] = b;
		c++;
	}
}

// note: expects scale array to be sorted in ascending order
void ColorScale::GetColor(double v, double& r, double& g, double& b) const
{
	int i = 0;
	double w;

	if (v < scale[0][0])
	{
		r = scale[0][1];
		g = scale[0][2];
		b = scale[0][3];
	}
	else if (v > scale[c-1][0])
	{
		r = scale[c-1][1];
		g = scale[c-1][2];
		b = scale[c-1][3];
	}
	else
	{
		for (i=1; i<=c; i++)
			if (scale[i-1][0] <= v && v < scale[i][0])
				break;

		// linear interpolation
		w = (v - scale[i-1][0]) / (scale[i][0] - scale[i-1][0]);

		r = (1-w)*scale[i-1][1] + w*scale[i][1];
		g = (1-w)*scale[i-1][2] + w*scale[i][2];
		b = (1-w)*scale[i-1][3] + w*scale[i][3];
	}
}

#endif

