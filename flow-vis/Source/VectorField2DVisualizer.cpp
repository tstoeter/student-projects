#include <stdio.h>
#include <math.h>
#include <allegro.h>

#include "VectorField2DVisualizer.h"
#include "ColorScale.h"

const double VectorField2DVisualizer::eps = 0.00001;

VectorField2DVisualizer::VectorField2DVisualizer(VectorField2D& field, BITMAP *bmp) : vfield(field)
{
	image = bmp;
	clear_to_color(image, makecol(0,0,0));
}

int VectorField2DVisualizer::sign(double x)
{
	return x < 0 ? -1 : 1;
}

void VectorField2DVisualizer::PermutatePixels(int *p)
{
	int c, tmp;

	for (int i=0; i<image->w*image->h; i++)
		p[i] = i;

	for (int j=0; j<image->w*image->h; j++)
	{
		c = rand() % (image->w*image->h);

		tmp = p[j];
		p[j] = p[c];
		p[c] = tmp;
	}
}

void VectorField2DVisualizer::DrawTangents(int spx, int spy, double length)
{
	double ys[2], ts = 0;
	int dimx, dimy;

	dimx = vfield.GetDimX();
	dimy = vfield.GetDimY();

	vector2d v;

	for (int y=0; y<image->h; y+=spy)
	{
		for (int x=0; x<image->w; x+=spx)
		{
			ys[0] = ((double)x*(dimx-1) / image->w);
			ys[1] = ((double)y*(dimy-1) / image->h);

			v = vfield.GetVectorAt(ys[0], ys[1]);
			ys[0] = v.x;
			ys[1] = v.y;

			circlefill(image, x, y, 1, makecol(255,255,255));

			line(image, x, y, x+length*ys[0], y+length*ys[1], makecol(255,255,255));
		}
	}
}

void VectorField2DVisualizer::IntegrateAndDraw()
{
	double ys[2], ts = 0;
	int x, y, oldx, oldy;
	int sx, sy, c;
	int percent;

	int permut[image->w*image->h];

	vector2d v;
	int dimx, dimy;

	dimx = vfield.GetDimX();
	dimy = vfield.GetDimY();

	// generate random permutation of pixels to visit
	PermutatePixels(permut);

	// traverse every pixel
	for (int k=0; k<image->w*image->h; k++)
	{
		percent = (int)round((100.0f*k)/(image->w*image->h));
		textprintf_ex(screen, font, 2, 2, makecol(127,127,255), makecol(0,0,0), "%d%%", percent);

		// get x,y coordinates from linear pixel array
		x = permut[k] % image->w;
		y = permut[k] / image->w;

		// initialize pixel values
		oldx = sx = x;
		oldy = sy = y;
		c = 1 + rand() % 255;

		// or if this pixel has already been drawn
		if (getpixel(image, sx, sy) != makecol(0, 0, 0))
			continue;

		// draw starting pixel
		putpixel(image, sx, sy, makecol(c, c, c));

		// set initial values
		ys[0] = ((double)sx*(dimx-1) / image->w);
		ys[1] = ((double)sy*(dimy-1) / image->h);
		vfield.SetInitialValues(ys, ts);

		// while no break condition was met
		while (1)
		{
			// integrate until a new pixel has been reached
			while (sx == oldx && sy == oldy)
			{
				// integrate one step forward
				vfield.StepForward(ys, ts);

				// and check for sinks
				v = vfield.GetVectorAt(ys[0], ys[1]);

				if (fabs(v.x) < eps && fabs(v.y) < eps)
					break;

				// compute pixel coordinates
				sx = round(ys[0] * image->w / (dimx-1));
				sy = round(ys[1] * image->h / (dimy-1));
			}

			// break if we move out of the image
			if (sx < 0 || sx >= image->w)
				break;

			if (sy < 0 || sy >= image->h)
				break;

			// or if this pixel has already been drawn
			if (getpixel(image, sx, sy) != makecol(0,0,0))
				break;

			// draw new pixel
			putpixel(image, sx, sy, makecol(c, c, c));

			// update last pixel's position
			oldx = sx;
			oldy = sy;
		}

		// reset initial values for backward integration
		ys[0] = ((double)sx*(dimx-1) / image->w);
		ys[1] = ((double)sy*(dimy-1) / image->h);
		vfield.SetInitialValues(ys, ts);

		// same as above for backward integration
		while (1)
		{
			// integrate until a new pixel has been reached
			while (sx == oldx && sy == oldy)
			{
				// integrate one step backward
				vfield.StepBackward(ys, ts);

				// and check for sinks
				v = vfield.GetVectorAt(ys[0], ys[1]);

				if (fabs(v.x) < eps && fabs(v.y) < eps)
					break;

				// compute pixel coordinates
				sx = round(ys[0] * image->w / (dimx-1));
				sy = round(ys[1] * image->h / (dimy-1));
			}

			// break if we move out of the image
			if (sx < 0 || sx >= image->w)
				break;

			if (sy < 0 || sy >= image->h)
				break;

			// or if this pixel has already been drawn
			if (getpixel(image, sx, sy) != makecol(0,0,0))
				break;

			// draw new pixel
			putpixel(image, sx, sy, makecol(c, c, c));

			// update last pixel's position
			oldx = sx;
			oldy = sy;
		}
	}
}

void VectorField2DVisualizer::LIC(int l)
{
	double ys[2], ts = 0;
	int x, y, oldx, oldy;
	int sx, sy, c;
	long avg, n;
	int percent;

	int permut[image->w*image->h];

	vector2d v;
	int dimx, dimy;

	dimx = vfield.GetDimX();
	dimy = vfield.GetDimY();

	// generate random permutation of pixels to visit
	PermutatePixels(permut);

	// create white noise
	for (y=0; y<image->h; y++)
	{
		for (x=0; x<image->w; x++)
		{
			c = rand() % 256;
			putpixel(image, x, y, makecol(c, c, c));
		}
	}

	// traverse every pixel
	for (int k=0; k<image->w*image->h; k++)
	{
		percent = (int)round((100.0f*k)/(image->w*image->h));
		textprintf_ex(screen, font, 2, 2, makecol(127,127,255), makecol(0,0,0), "%d%%", percent);

		// get x,y coordinates from linear pixel array
		x = permut[k] % image->w;
		y = permut[k] / image->w;

		// initialize pixel values
		oldx = sx = x;
		oldy = sy = y;

		// set initial values
		ys[0] = ((double)sx*(dimx-1) / image->w);
		ys[1] = ((double)sy*(dimy-1) / image->h);
		vfield.SetInitialValues(ys, ts);

		// get current pixels color
		avg = getr(getpixel(image, x, y));
		n = 1;

		// integrate l pixels forward
		for (int i=0; i<l; i++)
		{
			// integrate until a new pixel has been reached
			while (sx == oldx && sy == oldy)
			{
				// integrate one step forward
				vfield.StepForward(ys, ts);

				// and check for sinks
				v = vfield.GetVectorAt(ys[0], ys[1]);

				if (fabs(v.x) < eps && fabs(v.y) < eps)
					break;

				// compute pixel coordinates
				sx = round(ys[0] * image->w / (dimx-1));
				sy = round(ys[1] * image->h / (dimy-1));
			}

			// break if we move out of the image
			if (sx < 0 || sx >= image->w)
				break;

			if (sy < 0 || sy >= image->h)
				break;

			// get new pixel's color
			avg += getr(getpixel(image, sx, sy));
			n++;

			// update last pixel's position
			oldx = sx;
			oldy = sy;
		}

		// reset initial values for backward integration
		ys[0] = ((double)sx*(dimx-1) / image->w);
		ys[1] = ((double)sy*(dimy-1) / image->h);
		vfield.SetInitialValues(ys, ts);

		// same as above, just s pixel steps backward
		for (int i=0; i<l; i++)
		{
			// integrate until a new pixel has been reached
			while (sx == oldx && sy == oldy)
			{
				// integrate one step forward
				vfield.StepForward(ys, ts);

				// and check for sinks
				v = vfield.GetVectorAt(ys[0], ys[1]);

				if (fabs(v.x) < eps && fabs(v.y) < eps)
					break;

				// compute pixel coordinates
				sx = round(ys[0] * image->w / (dimx-1));
				sy = round(ys[1] * image->h / (dimy-1));
			}

			// break if we move out of the image
			if (sx < 0 || sx >= image->w)
				break;

			if (sy < 0 || sy >= image->h)
				break;

			// get new pixel's color
			avg += getr(getpixel(image, sx, sy));
			n++;

			// update last pixel's position
			oldx = sx;
			oldy = sy;
		}

		// now average the color
		avg /= n;

		// and redraw original pixel with new color
		putpixel(image, x, y, makecol(avg, avg, avg));
	}
}

void VectorField2DVisualizer::SpotNoise(int n, int size)
{
	int x, y, r, c;
	double ys[2], ts = 0;
	double magn, angle;
	int w, h;
	int percent;

	int dimx, dimy;

	dimx = vfield.GetDimX();
	dimy = vfield.GetDimY();

	vector2d v;

	double imap[image->w][image->h][2];

	BITMAP *tex = create_bitmap(size, size);
	BITMAP *tmp = create_bitmap(image->w, image->h);

	for (int i=0; i<n; i++)
	{
		percent = (int)round((100.0f*i)/n);
		textprintf_ex(screen, font, 2, 2, makecol(127,127,255), makecol(0,0,0), "%d%%", percent);

		// choose random pixel
		x = rand() % image->w;
		y = rand() % image->h;
		c = rand() % 256;
		r = rand() % (size/2);

		clear_to_color(image, makecol(255,0,255));
		clear_to_color(tex, makecol(255,0,255));
		clear_to_color(tmp, makecol(255,0,255));

		// create spot texture
		circlefill(tex, size/2, size/2, r, makecol(c,c,c));

		// set initial values
		ys[0] = ((double)x*(dimx-1) / image->w);
		ys[1] = ((double)y*(dimy-1) / image->h);
		
		// get vector at the random position
		v = vfield.GetVectorAt(ys[0], ys[1]);
		ys[0] = v.x;
		ys[1] = v.y;

		// compute length and orientation of vector
		magn = sqrt(ys[0]*ys[0] + ys[1]*ys[1]);
		angle = -1*sign(ys[1])*acos(ys[0]/magn) * 128/3.14159265f;

		// scale and rotate spot texture
		w = round((double)size*(1+magn));
		h = round((double)size/(1+magn));

		// avoid scaling too thin (to be visible)
		if (h <= 1)
			h = 2;

		// take care: center streched sprite correctly and then rotate it
		stretch_sprite(tmp, tex, tmp->w/2-w/2, tmp->h/2-h/2, w, h);

		// rotate and draw final spot onto bitmap centered at initial random position
		pivot_sprite(image, tmp, x, y, tmp->w/2, tmp->h/2, ftofix(angle));

		// convert bitmap to intensity map for later averaging intensity values,
		// but also make benefit of bitmap clipping and blitting
		for (int v=0; v<image->h; v++)
		{
			for (int u=0; u<image->w; u++)
			{
				if (getpixel(image, u, v) != makecol(255, 0, 255))
				{
					imap[u][v][0] += c;
					imap[u][v][1]++;
				}
			}
		}
	}

	// convert intensity map back to bitmap
	for (y=0; y<image->h; y++)
	{
		for (x=0; x<image->w; x++)
		{
			c = imap[x][y][0]/imap[x][y][1];
			putpixel(image, x, y, makecol(c,c,c));
		}
	}

	destroy_bitmap(tex);
	destroy_bitmap(tmp);
}

void VectorField2DVisualizer::TextureAdvection(double impact, int gc)
{
	double ys[2], ts = 0;
	int x, y;
	int sx, sy, c;
	int w, h;
	
	w = image->w;
	h = image->h;

	int dimx, dimy;

	dimx = vfield.GetDimX();
	dimy = vfield.GetDimY();

	vector2d v;
	
	BITMAP *tex = load_bitmap("empire.tga", NULL);
	
	BITMAP *grid = create_bitmap(w, h);
	clear_to_color(grid, makecol(255,255,255));
	
	if (gc > 0)
	{	
		// create grid texture
		for (y=15; y<h; y+=32)
			line(grid, 0, y, w, y, makecol(gc,gc,gc));
		
		for (x=15; x<w; x+=32)
			line(grid, x, 0, x, h, makecol(gc,gc,gc));
	}
	
	// do advection for texture
	for (y=0; y<h; y++)
	{
		for (x=0; x<w; x++)
		{
			// convert pixel to vector field coordinates
			ys[0] = ((double)x*(dimx-1) / w);
			ys[1] = ((double)y*(dimy-1) / h);
		
			// get vector at prior pixel position
			v = vfield.GetVectorAt(ys[0], ys[1]);
			ys[0] = v.x;
			ys[1] = v.y;
			
			// compute source pixel coordinates
			sx = x - round(impact*ys[0]);
			sy = y - round(impact*ys[1]);
			
			c = getpixel(tex, sx, sy);
			putpixel(image, x, y, c);
		}
	}
	
	// do advection for grid	
	for (y=0; y<h; y++)
	{
		for (x=0; x<w; x++)
		{
			// convert pixel to vector field coordinates
			ys[0] = ((double)x*(dimx-1) / w);
			ys[1] = ((double)y*(dimy-1) / h);
		
			// get vector at prior pixel position
			v = vfield.GetVectorAt(ys[0], ys[1]);
			ys[0] = v.x;
			ys[1] = v.y;
			
			// compute source pixel coordinates
			sx = x - round(impact*ys[0]);
			sy = y - round(impact*ys[1]);
			
			c = getpixel(grid, sx, sy);
			
			// translucently draw color code on top of image
			drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
			set_multiply_blender(0, 0, 0, 127);
			
			putpixel(image, x, y, c);
			
			drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
		}
	}
	
	destroy_bitmap(grid);
	destroy_bitmap(tex);
}

void VectorField2DVisualizer::ColorizeMagnitude(ColorScale cs)
{
	double ys[2];
	int x, y, c, tmp;
	double r, g, b, magn;

	int permut[image->w*image->h];

	vector2d v;
	int dimx, dimy;

	dimx = vfield.GetDimX();
	dimy = vfield.GetDimY();

	// generate random permutation of pixels to visit
	PermutatePixels(permut);

	// traverse every pixel
	for (int k=0; k<image->w*image->h; k++)
	{
		// get x,y coordinates from linear pixel array
		x = permut[k] % image->w;
		y = permut[k] / image->w;

		// set initial values
		ys[0] = ((double)x*(dimx-1) / image->w);
		ys[1] = ((double)y*(dimy-1) / image->h);
		
		// get vector at the random position
		v = vfield.GetVectorAt(ys[0], ys[1]);

		// compute length of vector
		magn = sqrt(v.x*v.x + v.y*v.y);

		// translucently draw color code on top of image
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
		set_trans_blender(0, 0, 0, 127);

		cs.GetColor(magn, r, g, b);

		putpixel(image, x, y, makecol(255*r, 255*g, 255*b));

		// set drawing mode back to solid
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	}
}

bool VectorField2DVisualizer::HasCritical(double x1, double y1, double x2, double y2)
{
	vector2d v[4];

	v[0] = vfield.GetVectorAt(x1, y1);
	v[1] = vfield.GetVectorAt(x1, y2);
	v[2] = vfield.GetVectorAt(x2, y1);
	v[3] = vfield.GetVectorAt(x2, y2);

	if ((sign(v[0].x) == sign(v[1].x) && sign(v[1].x) == sign(v[2].x) && sign(v[2].x) == sign(v[3].x)) ||
	    (sign(v[0].y) == sign(v[1].y) && sign(v[1].y) == sign(v[2].y) && sign(v[2].y) == sign(v[3].y)))
		return false;

	return true;
}

void VectorField2DVisualizer::FindCriticalRecursive(int step, int maxsteps, double x1, double y1, double x2, double y2)
{
	if (HasCritical(x1, y1, x2, y2))
	{
		double midx = (x1+x2)/2;
		double midy = (y1+y2)/2;

		if (step > maxsteps)
		{
			int sx, sy;
			int dimx, dimy;

			dimx = vfield.GetDimX();
			dimy = vfield.GetDimY();

			// compute pixel coordinates
			sx = round(midx * image->w / (dimx-1));
			sy = round(midy * image->h / (dimy-1));

			circlefill(image, sx, sy, 5, makecol(255,0,255));
		}
		else
		{
			FindCriticalRecursive(step+1, maxsteps, x1, y1, midx, midy);
			FindCriticalRecursive(step+1, maxsteps, midx, y1, x2, midy);
			FindCriticalRecursive(step+1, maxsteps, x1, midy, midx, y2);
			FindCriticalRecursive(step+1, maxsteps, midx, midy, x2, y2);
		}
	}
}

void VectorField2DVisualizer::FindCriticalPoints(int maxsteps)
{
	int dimx, dimy;

	dimx = vfield.GetDimX();
	dimy = vfield.GetDimY();

	for (int y=0; y<dimy-1; y++)
		for (int x=0; x<dimx-1; x++)
			FindCriticalRecursive(0, maxsteps, x, y, x+1, y+1);
}

