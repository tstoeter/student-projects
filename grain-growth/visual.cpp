#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <cuda_runtime.h>

#include "visual.h"
#include "grains.h"
#include "bmp.h"


// glut parameters
char name[] = "cuggs";			// window title
int width = 256, height = 256;		// window size
int oldx, oldy;				// old mouse coordinates
int frames = 0;				// frame counter
int t0 = 0, t1;				// timer to compute fps
int dims, step = 0;

// volume vis parameters
int nslices = 250;
int rotateVolume = 0;
double rx=0, ry=0;
int autorot = 0;
int pause = 1;

// control parameters
int visual=0;
int stat=0, verb=0, dump=0, term=-1;
const char *savefilename;

void display2d()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();

	// setup 2d pixel plotting camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, (GLfloat)width, 0.0f, (GLfloat)height, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, width, height);
	glDepthRange(0.0f, 1.0f);

        // load texture from pbo
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
        glBindTexture(GL_TEXTURE_2D, texid);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_ABGR_EXT, GL_UNSIGNED_BYTE, 0);
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);

		glColor3f(1.0, 1.0, 1.0);

		glTexCoord2f(0.0, 0.0);
		glVertex2i(0.0, 0.0);

		glTexCoord2f(1.0, 0.0);
		glVertex2i(width, 0.0);

		glTexCoord2f(1.0, 1.0);
		glVertex2i(width, height);

		glTexCoord2f(0.0, 1.0);
		glVertex2i(0.0, height);

	glEnd();

	glDisable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

	glPopMatrix();

	glutSwapBuffers();
}

void display3d()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (float)width/height, 0.1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, width, height);
	gluLookAt(0.0, 0.0, 1.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// x-ray
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();

	// load texture from pbo
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
        glBindTexture(GL_TEXTURE_3D, texid);
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, w, h, d, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	glEnable(GL_TEXTURE_3D);

	for (int i=1; i<=nslices; i++)
	{
		double z = (i*1.0/(nslices+1)-0.5);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();

		glTranslatef(0.5, 0.5, 0.5);
		glScalef(1.75, 1.75, 1.75);
		glRotatef(rx, 1, 0, 0);
		glRotatef(ry, 0, 1, 0);
		glTranslatef(0, 0, z);

		glMatrixMode(GL_MODELVIEW);

		glBegin(GL_QUADS);

			glColor4f(1, 1, 1, 0.04);

		   	glVertex3f(-0.5, -0.5, z);
			glVertex3f(-0.5, 0.5, z);
			glVertex3f(0.5, 0.5, z);
			glVertex3f(0.5, -0.5, z);
	
		glEnd();
	}

	glDisable(GL_TEXTURE_3D);

	glPopMatrix();

	glutSwapBuffers();
}

void update()
{
	// output only if not paused
	if (!pause || !visual)
	{
		// do statistics output
		if (stat)
		{
			download_grains();

			if (step == 0)
			{
				init_stats();
			
				// print stats
				if (verb)
					print_grains(step);
				else
					print_stats(step);
			}
			// every n simulation steps
			else if ((step % stat) == 0)
			{
				// compute statistics
				update_stats();

				// print stats
				if (verb)
					print_grains(step);
				else
					print_stats(step);
			}
		}

		// store grains after termination time was reached
		if ((term >= 0) && (step >= term))
		{
			download_grains();
			store_grains(savefilename);
			exit(0);
		}

		if (dump && (step % dump == 0))
		{
			char dumpfile[80];

			if (dims == 2)
				sprintf(dumpfile, "dump%d.bmp\0", step);
			else
				sprintf(dumpfile, "dump%d.bm3\0", step);

			download_grains();
			store_grains(dumpfile);
		}

		// do one monte carlo step in 2d or 3d
		if (dims == 2)
			run_mcs2d(w, h, kT);
		else
			run_mcs3d(w, h, d, kT);

		// increment simulation step
		step++;
	}

	// redraw
	glutPostRedisplay();

	// count frame per second
	frames++;

	t1 = glutGet(GLUT_ELAPSED_TIME);
	
	// every second approximately
	if (t1-t0 >= 1000)
	{
		char title[80];
		sprintf(title, "%s    %.1f fps", name, (1000.0*frames/(t1-t0)));
		glutSetWindowTitle(title);

		frames = 0;
		t0 = t1;
	}

	// autorotate feature
	rx += autorot*0.4;
	ry += autorot*0.4;
}

void mouse(int button, int state, int x, int y)
{
	rotateVolume = 0;

	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			rotateVolume = 1;
			oldx = x;
			oldy = y;
			
		}

		if (button == GLUT_RIGHT_BUTTON)
		{
			
		}
	}
}

void motion(int x, int y)
{
	if (rotateVolume)
	{
		rx += oldy-y;
		ry += oldx-x;

		oldx = x;
		oldy = y;
	}
}

void reshape(int w, int h)
{
	width = w;
	height = h;
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 27)
	{
		exit(0);
	}
	else if (key == 'r')
	{
		autorot = 1-autorot;
	}
	else if (key == ' ')
		pause = 1-pause;
}

void cleanup()
{
	destroy_grains();
}

void run_cuggs(int argc, char *argv[], const char *loadfile, const char *savefile)
{
	atexit(cleanup);

	savefilename = savefile;

	// init glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	glutInitWindowSize(width, height);
	glutCreateWindow(name);

	glewInit();

	dims = init_grains(loadfile);

	if (visual & dims == 2)
	{
		glutDisplayFunc(display2d);
	}
	else if (visual & dims == 3)
	{
		glutDisplayFunc(display3d);
		glutMouseFunc(mouse);
		glutMotionFunc(motion);
	}

	glutIdleFunc(update);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

	glutMainLoop();
}

