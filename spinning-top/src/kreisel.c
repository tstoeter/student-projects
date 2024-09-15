#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>

#include "model.h"
#include "texture.h"

#define PI		3.141592654
#define DEG2RAD(x)	((x)*PI/180.0)
#define RAD2DEG(x)	((x)*180.0/PI)


// glut variables
char name[] = "Brummkreisel";		// window title
int width = 512, height = 384;		// window size
int frames = 0;				// frame counter
int t0 = 0, t1;				// timer to compute fps
float fps = 0;

// time globals
double	time = 0.0,
	time_step = 0.01;

// constants
double 	i = 0.001,
	i_ = 0.04,
	w = 0.8,
	d = 0.2;

// initial values
double y[6] = 	{0.0,	// thed
		 0.0,	// psid
		 500.0,	// phid
		 0.01,	// theta
		 0.0,	// psi
		 0.0};  // phi

double 	nudge = 1.5,
	nudge_time = -1;

// values for graphs recorded in ringbuffer
int num_vals=500, first=0;
double	theta_vals[500],
	thetad_vals[500],
	psid_vals[500],
	x_vals[500], y_vals[500];

// derivatives
void derivs(double y[6], double t, double dydx[6])
{
	double 	thed = y[0],
		psid = y[1],
		phid = y[2],
		the  = y[3],
		psi  = y[4],
		phi  = y[5],
		psi2d;

	dydx[0] = (i_*psid*psid*sin(the)*cos(the) - i*(phid+psid*cos(the))*psid*sin(the) + w*d*sin(the)) / i_;
	dydx[1] = psi2d = (-2*i_*thed*psid*cos(the) + i*(phid+psid*cos(the))*thed) / (i_*sin(the));
	dydx[2] = psid*thed*sin(the) - psi2d*cos(the);
	dydx[3] = thed;
	dydx[4] = psid;
	dydx[5] = phid;
}

// integration steps
void step(double y[6], double dydx[6], double h, double yout[6])
{
	// take one integration step by h
	yout[0] = y[0] + h * dydx[0];
	yout[1] = y[1] + h * dydx[1];
	yout[2] = y[2] + h * dydx[2];
	yout[3] = y[3] + h * dydx[3];
	yout[4] = y[4] + h * dydx[4];
	yout[5] = y[5] + h * dydx[5];
}

// ode solver: 4th-order runge-kutta method
void rk4(double y[6], double h)
{
	double yout[6];
	double y14[6], y12[6], y34[6];
	double dy[6], dy14[6], dy12[6], dy34[6];
	
	derivs(y, 0, dy);

	step(y, dy, h/2, y14);
	derivs(y14, 0, dy14);

	step(y, dy14, h/2, y12);
	derivs(y12, 0, dy12);

	step(y, dy12, h, y34);
	derivs(y34, 0, dy34);

	yout[0] = y[0] + h/6 * (dy[0] + 2*dy14[0] + 2*dy12[0] + dy34[0]);
	yout[1] = y[1] + h/6 * (dy[1] + 2*dy14[1] + 2*dy12[1] + dy34[1]);
	yout[2] = y[2] + h/6 * (dy[2] + 2*dy14[2] + 2*dy12[2] + dy34[2]);

	yout[3] = y[3] + h/6 * (y[0] + 2*y14[0] + 2*y12[0] + y34[0]);
	yout[4] = y[4] + h/6 * (y[1] + 2*y14[1] + 2*y12[1] + y34[1]);
	yout[5] = y[5] + h/6 * (y[2] + 2*y14[2] + 2*y12[2] + y34[2]);

	memcpy(y, yout, sizeof(yout));
}

void draw_string(float x, float y, float z, void *font, char *string)
{
	char *c;

	glRasterPos3f(x, y,z);

	for (c=string; *c != '\0'; c++)
		glutBitmapCharacter(font, *c);
}

void draw_graph(char *title, double *yvals, double sy, double yoff, int i, int n, int x, int y, int w, int h, int gsx, int gsy)
{
	int c;
	double sx = (double)w/n;

	// setup orthographic projection for 2d display
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	// save old projection matrix
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// disable lighting and depth test
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	// make lines look nice
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// draw bg
	glColor4f(0.25f, 0.25f, 0.25f, 0.6f);

	glBegin(GL_QUADS);
		glVertex2i(x, y);
		glVertex2i(x, y+h-1);
		glVertex2i(x+w-1, y+h-1);
		glVertex2i(x+w-1, y);
	glEnd();

	// draw grid
	glColor4f(0.1f, 0.1f, 0.1f, 0.6f);

	if (gsx > 0)
	{
		glBegin(GL_LINES);
			for (c=x+gsx; c<=x+w-gsx; c+=gsx)
			{
				glVertex2i(c, y);
				glVertex2i(c, y+h-1);
			}
		glEnd();
	}

	if (gsy > 0)
	{
		glBegin(GL_LINES);
			for (c=y+gsy; c<=y+h-gsy; c+=gsy)
			{
				glVertex2i(x, c);
				glVertex2i(x+w-1, c);
			}
		glEnd();
	}

	// print graph title
	glColor4f(0, 0, 0, 1);
	draw_string(x+1, y+13, 0, GLUT_BITMAP_8_BY_13, title);

	// draw graph
	glColor4f(0.25, 1, 0.25, 0.6);

	glBegin(GL_LINE_STRIP);
		for (c=0; c<n; c++)
			glVertex2f(x+c*sx, yoff+y+h-1-yvals[(i+c)%n]*sy);
	glEnd();

	// restore 3d projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}

void draw_phase_plot(char *title, double *xvals, double *yvals, double sx, double sy, int i, int n, int x, int y, int w, int h, int gsx, int gsy)
{
	int c;

	// setup orthographic projection for 2d display
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	// save old projection matrix
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// disable lighting and depth test
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	// make lines look nice
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// draw bg
	glColor4f(0.25f, 0.25f, 0.25f, 0.6f);

	glBegin(GL_QUADS);
		glVertex2i(x, y);
		glVertex2i(x, y+h-1);
		glVertex2i(x+w-1, y+h-1);
		glVertex2i(x+w-1, y);
	glEnd();

	// draw grid
	glColor4f(0.1f, 0.1f, 0.1f, 0.6f);

	if (gsx > 0)
	{
		glBegin(GL_LINES);
			for (c=x+gsx/2; c<=x+w-gsx/2; c+=gsx)
			{
				glVertex2i(c, y);
				glVertex2i(c, y+h-1);
			}
		glEnd();
	}

	if (gsy > 0)
	{
		glBegin(GL_LINES);
			for (c=y+gsy/2; c<=y+h-gsy/2; c+=gsy)
			{
				glVertex2i(x, c);
				glVertex2i(x+w-1, c);
			}
		glEnd();
	}

	// print graph title
	glColor4f(0, 0, 0, 1);
	draw_string(x+1, y+13, 0, GLUT_BITMAP_8_BY_13, title);

	// draw graph
	glColor4f(0.4f, 0.4f, 1, 0.6f);

	glBegin(GL_LINE_STRIP);
		for (c=0; c<n; c++)
			glVertex2f(x+w/2-xvals[(i+c)%n]*sx, y+h/2+yvals[(i+c)%n]*sy);
	glEnd();

	// restore 3d projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}

void draw_top(int mirror)
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	if (mirror)
	{
		glFrontFace(GL_CW);

		glScalef(0.06f, -0.06f, 0.06f);

		glEnable(GL_LIGHT1);

		Group2_handle();
		Group2_top();
		Group2_body();
		Group2_bottom();

		glFrontFace(GL_CCW);
	}
	else
	{
		glScalef(0.06f, 0.06f, 0.06f);

		glEnable(GL_LIGHT0);

		Group2_bottom();
		Group2_body();
		Group2_top();
		Group2_handle();
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT0);
}

void draw_info()
{
	char buf[80];

	// setup orthographic projection for 2d display
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	// save old projection matrix
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// disable lighting and depth testing
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	sprintf(buf, "%.2f fps", fps);

	glColor3f(240/255.0, 83/255.0, 12/255.0);
	draw_string(8, 8+13, 0, GLUT_BITMAP_8_BY_13, "Brummkreisel");
	glColor3f(255/255.0, 132/255.0, 2/255.0);
	draw_string(8, 8+13+15, 0, GLUT_BITMAP_8_BY_13, "press space to nudge");
	glColor3f(251/255.0, 228/255.0, 0/255.0);
	draw_string(8, 8+13+30, 0, GLUT_BITMAP_8_BY_13, buf);

	// restore 3d projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void init()
{
	GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 0.0f};
	GLfloat LightDiffuse[] = {0.5f, 0.5f, 0.5f, 0.0f};
	GLfloat LightSpecular[] = {1.0f, 1.0f, 1.0f, 0.0f};
	GLfloat LightPosition[] = {-20.0f, 10.0f, 20.0f, 0.0f};

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	// setup mirrored light
	LightPosition[1] = -10.0f;
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpecular);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);

	glPushMatrix();

	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)width/height, 0.1, 100);
	gluLookAt(0, 2, 2, 0, 0, -0.5, 0, 1, 0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void display()
{
	double 	theta = RAD2DEG(y[3]),
		psi = RAD2DEG(y[4]),
		phi = RAD2DEG(y[5]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushMatrix();
	glLoadIdentity();

	// draw mirrored top
	glRotatef(psi, 0, 1, 0);
	glRotatef(theta, 0, 0, 1);
	glRotatef(phi, 0, 1, 0);

	draw_top(1);

	glPopMatrix();

	// draw mirroring floor
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glDisable(GL_LIGHTING);

	glColor4f(0.6, 0.6, 0.6, 0.8);

	glBegin(GL_QUADS);
		glTexCoord2f(388.0/1024, 421.0/1024);
		glVertex3f(-1.5, 0, -1);

		glTexCoord2f(388.0/1024, 600.0/1024);
		glVertex3f(-1.5, 0, 1.5);

		glTexCoord2f(567.0/1024, 600.0/1024);
		glVertex3f(1.5, 0, 1.5);

		glTexCoord2f(567.0/1024, 421.0/1024);
		glVertex3f(1.5, 0, -1);
	glEnd();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);

	// draw spinning top
	glRotatef(psi, 0, 1, 0);
	glRotatef(theta, 0, 0, 1);
	glRotatef(phi, 0, 1, 0);

	draw_top(0);

	glPopMatrix();

	// draw graphs
	draw_graph("theta(t)", theta_vals, 160, 0, first, num_vals, width-148, 8, 140, 120, 28, 24);
	draw_graph("theta_d(t)", thetad_vals, 30, -50, first, num_vals, width-148, 12+120, 140, 120, 28, 24);
	draw_graph("psid_d(t)", psid_vals, 5, -20, first, num_vals, width-148, 16+240, 140, 120, 28, 24);

	draw_phase_plot("x-y phase plot", x_vals, y_vals, 80, 80, first, num_vals, 8, height-148, 140, 140, 28, 28);

	// draw info text
	draw_info();

	// count frames
	frames++;
	glutSwapBuffers();
}

void timer(int t)
{
	// solve ode
	rk4(y, time_step);
	time += time_step;

	// record all the values for graphs
	theta_vals[first] = y[3];
	thetad_vals[first] = y[0];
	psid_vals[first] = y[1];
	x_vals[first] = sin(y[3])*cos(y[4]);
	y_vals[first] = sin(y[3])*sin(y[4]);
	first = (first+1)%num_vals;

	glutTimerFunc(1000*time_step, timer, 0);
}

void update()
{
	// has been nudged?
	if (nudge_time == 0)
	{
		y[0] = nudge;
		nudge_time = time + 0.1;
	}
	else if (time > nudge_time)
	{
		y[0] = 0;
		nudge_time = -1;
	}

	// compute frames per second
	t1 = glutGet(GLUT_ELAPSED_TIME);
	
	// for every second approximately
	if (t1-t0 >= 1000)
	{
		
		fps = (1000.0*frames/(t1-t0));
		frames = 0;
		t0 = t1;
	}

	// redraw
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	glutReshapeWindow(width, height);
}

void keyboard(unsigned char key, int px, int py)
{
	if (key == 27)
		exit(0);

	if (key == ' ')
		nudge_time = 0;
}

int main(int argc, char *argv[])
{
	// init glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(width, height);

	glutCreateWindow(name);

	glutDisplayFunc(display);
	glutIdleFunc(update);
	glutTimerFunc(1000*time_step, timer, 0);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	init();

	glutMainLoop();

	return 0;
}

