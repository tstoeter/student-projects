#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#include "LatticeBoltzmann.h"
#include "LatticeSite.h"
#include "ColorScale.h"
#include "ParticleAdvection.h"

using namespace std;


#define	LBWIDTH		256
#define	LBHEIGHT	256
#define NUMPARTICLES	32768

int winwidth = 384;
int winheight = 384;


const int dims[2] = {LBWIDTH, LBHEIGHT};
double **rho, ***velocity;

LatticeBoltzmann *lb;
ParticleAdvection *pa;

ColorScale rainbow(6);
ColorScale bluewhite(3);

GLuint texnum;

bool drawBoundary = false;
bool addVelocity = false;
int oldx, oldy;

int drawVelocities = 1;
int drawParticles = 1;

int frames = 0;
int t0 = 0, te;


void draw_densities(int x, int y)
{
	double r, g, b;

	if (lb->getSite(x, y).isBoundary())
		r = g = b = 0;
	else
		rainbow.GetColor(rho[x][y], r, g, b);

	glColor4f(r, g, b, 1.0);
	glVertex2i(x, y);
}

void draw_velocities()
{
	unsigned char bitmap[LBWIDTH*LBHEIGHT*4];	// rgba unsigned bytes

	double m, r, g, b;

	#pragma omp parallel for private(r,g,b)
	for (int y=0; y<LBHEIGHT; y++)
	{
		for (int x=0; x<LBWIDTH; x++)
		{
			if (lb->getSite(x, y).isBoundary())
			{
				r = g = b = 0;
			}
			else
			{
				m = sqrt(velocity[x][y][0]*velocity[x][y][0]+velocity[x][y][1]*velocity[x][y][1]);
				bluewhite.GetColor(m*50, r, g, b);
			}

			bitmap[(x+y*LBWIDTH)*4 + 0] = r*255;
			bitmap[(x+y*LBWIDTH)*4 + 1] = g*255;
			bitmap[(x+y*LBWIDTH)*4 + 2] = b*255;
			bitmap[(x+y*LBWIDTH)*4 + 3] = 255;
		}
	}

	glBindTexture(GL_TEXTURE_2D, texnum);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, LBWIDTH, LBHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);

		glColor3f(1.0, 1.0, 1.0);

		glTexCoord2f(0.0, 0.0);
		glVertex2i(0.0, 0.0);

		glTexCoord2f(1.0, 0.0);
		glVertex2i(winwidth, 0.0);

		glTexCoord2f(1.0, 1.0);
		glVertex2i(winwidth, winheight);

		glTexCoord2f(0.0, 1.0);
		glVertex2i(0.0, winheight);

	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void draw_particles()
{
	double *p;

	for (int i=0; i<NUMPARTICLES; i++)
	{
		p = pa->getParticle(i);

		glColor4f(1.0, 0.5, 0.5, 0.5);
		glVertex2i((int)(p[0]*winwidth/LBWIDTH), (int)(p[1]*winheight/LBHEIGHT));
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();

	// setup 2d pixel plotting camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, (GLfloat) winwidth, 0.0f, (GLfloat) winheight, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, winwidth, winheight);
	glDepthRange(0.0f, 1.0f);

	if (drawVelocities > 0)
		draw_velocities();

	if (drawParticles > 0)
	{
		glBegin(GL_POINTS);
	 		draw_particles();
		glEnd();
	}

	glPopMatrix();

	glutSwapBuffers();
}

void update()
{
	lb->update();
	pa->advect0();

	// redraw
	glutPostRedisplay();

	frames++;

	te = glutGet(GLUT_ELAPSED_TIME);
	
	// every second approximately
	if (te-t0 >= 1000)
	{
		char title[80];
		sprintf(title, "Lattice Boltzmann demo    %.1f fps", (1000.0*frames/(te-t0)));
		glutSetWindowTitle(title);

		frames = 0;
		t0 = te;
	}		
}

void mouse(int button, int state, int x, int y)
{
	double u[2] = {0, 0};

	drawBoundary = false;
	addVelocity = false;

	x *= (double)LBWIDTH/winwidth;
	y *= (double)LBHEIGHT/winheight;

	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			if (lb->getSite(x, y).isFluid() && x >= 0 && x < LBWIDTH && y >= 0 && y < LBHEIGHT)
			{
				addVelocity = true;
				oldx = x;
				oldy = y;
			}
		}

		if (button == GLUT_RIGHT_BUTTON)
		{
			drawBoundary = true;

			if (x >= 0 && x < LBWIDTH && y >= 0 && y < LBHEIGHT)
				lb->setSite(x, 255-y, LatticeSite::Boundary, u);
		}
	}
}

void motion(int x, int y)
{
	double m, u[2] = {0, 0};

	x *= (double)LBWIDTH/winwidth;
	y *= (double)LBHEIGHT/winheight;

	if (drawBoundary && (x >= 0 && x < LBWIDTH && y >= 0 && y < LBHEIGHT))
		lb->setSite(x, 255-y, LatticeSite::Boundary, u);

	if (addVelocity && (x >= 0 && x < LBWIDTH && y >= 0 && y < LBHEIGHT))
	{
		if (lb->getSite(x, y).isFluid())
		{
			u[0] = (x-oldx);
			u[1] = (oldy-y);

			m = sqrt(u[0]*u[0]+u[1]*u[1]);
			u[0] /= (1+2*m);
			u[1] /= (1+2*m);

			lb->setSite(x, 255-y, LatticeSite::Fluid, u);
		}
	}
}

void reshape(int w, int h)
{
	winwidth = w;
	winheight = h;
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
	
	if (key == 'r')
	{
		lb->reset();
		pa->reset();
	}
	else if (key == 'v')
	{
		drawVelocities *= -1;
	}
	else if (key == 'p')
	{
		drawParticles *= -1;
	}
}

void cleanup()
{
	lb->~LatticeBoltzmann();
	pa->~ParticleAdvection();

	delete rho;
	delete velocity;
}

int main(int argc, char *argv[])
{
	lb = new LatticeBoltzmann(dims);
	lb->getDensityAndVelocityField(rho, velocity);

	pa = new ParticleAdvection(NUMPARTICLES, velocity, LBWIDTH, LBHEIGHT);

	// create color scale
	rainbow.AddPoint(5, 0, 0, 0);
	rainbow.AddPoint(10, 0, 0, 1);
	rainbow.AddPoint(15, 0, 1, 1);
	rainbow.AddPoint(20, 0, 1, 0);
	rainbow.AddPoint(25, 1, 1, 0);
	rainbow.AddPoint(30, 1, 0, 0);

	// create color scale
	bluewhite.AddPoint(0, 0, 0, 1);
	bluewhite.AddPoint(1.0, 0, 1, 1);
	bluewhite.AddPoint(2.0, 1, 1, 1);

	atexit(cleanup);
	
	// init glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(winwidth, winheight);
	
	glutCreateWindow("Lattice Boltzmann demo");

	glutDisplayFunc(display);
	glutIdleFunc(update);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

	glClearColor(0, 0, 0, 0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);
	
	glGenTextures(1, &texnum);

	glutMainLoop();

	return 0;
}

