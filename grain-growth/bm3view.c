// bm2view version 0.9
// only 24bit .bm3 files supported
// usage: $ ./bm3view random64.bm3
// keyboard:
// +/-		increase/decrease number of slices
// ./,		increase/decrease translucency
// j/k		previous/next bm3 file in directory
// r		autorotate volume
// mouse:
// left button	manually rotate volume
// compile: $ gcc bm3view.c bmp.c -o bm3view -lglut -lGLEW

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <sys/stat.h>
#include <dirent.h>

#include "bmp.h"


// glut parameters
char name[] = "bm3view";		// window title
int width = 512, height = 512;		// window size
int oldx, oldy;				// old mouse coordinates
int frames = 0;				// frame counter
int t0 = 0, t1;				// timer to compute fps
int dims, step = 0;

// volume vis parameters
int nslices = 250;
double alpha = 0.05;
int rotateVolume = 0;
double rx=0, ry=0;
int autorot = 0;

GLuint texid;

char **files = NULL;
int cur = 0, num = 0;

void load_bm3(char *filename)
{
	bm3_info info;

	char *data24 = loadbm3(filename, &info);

	int w = info.width;
	int h = info.height;
	int d = info.depth;

	if (data24 == NULL || info.bpp != 24)
	{
		free(data24);
		printf("bad bitmap file\n");
		exit(1);
	}

	// convert to 32bit data
	unsigned int *data = (unsigned int *)malloc(w*h*d*sizeof(unsigned int));

	int i;
	for (i=0; i<w*h*d; i++)
			data[i] = (((int)data24[3*i+0] & 0x000000ff) << 16) + (((int)data24[3*i+1] & 0x000000ff) << 8) + (((int)data24[3*i+2] & 0x000000ff) << 0) + 0xff000000;

	// create 3d textures for display
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_3D, texid);
	// clamp to border so we get a black border around the volume and no parts of the volume repeated
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// enable the border

	// make texture
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, w, h, d, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_3D, 0);

	// configure texture coordinate generation
	// for view-aligned slices
	float plane_x[] = {1.0f, 0.0f, 0.0f, 0.0f};
	float plane_y[] = {0.0f, 1.0f, 0.0f, 0.0f};

	glTexGenfv(GL_S, GL_OBJECT_PLANE, plane_x);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, plane_y);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);

	free(data);

        glBindTexture(GL_TEXTURE_3D, texid);
}

void destroy_bm3()
{
	glDeleteTextures(1, &texid);
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
//        glBindTexture(GL_TEXTURE_3D, texid);
//        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, w, h, d, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glEnable(GL_TEXTURE_3D);

	int i;
	for (i=1; i<=nslices; i++)
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

			glColor4f(1, 1, 1, alpha);

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
	else if (key == '+')
	{
		nslices += 2;
	}
	else if (key == '-')
	{
		nslices -= 2;

		if (nslices < 1)
			nslices = 1;
	}
	else if (key == '.')
	{
		alpha *= 2;

		if (alpha > 1)
			alpha = 1;
	}
	else if (key == ',')
	{
		alpha /= 2;

		if (alpha < 0.0001)
			alpha = 0.0001;
	}
	else if (key == 'j')
	{
		cur = (num+cur-1) % num;

		destroy_bm3();
		load_bm3(files[cur]);
	}
	else if (key == 'k')
	{
		cur = (num+cur+1) % num;

		destroy_bm3();
		load_bm3(files[cur]);
	}
}

void cleanup()
{
	destroy_bm3();
}

int is_file(char *path)
{
	if (path == NULL)
		return 0;

	int status;
	struct stat st_buf;

	status = stat(path, &st_buf);

	if (status != 0)
		return 0;

	return S_ISREG(st_buf.st_mode);
}

int is_dir(char *path)
{
	if (path == NULL)
		return 0;

	int status;
	struct stat st_buf;

	status = stat(path, &st_buf);

	if (status != 0)
		return 0;

	return S_ISDIR(st_buf.st_mode);
}

void file_dir(char *filename, char *dirname)
{
	strncpy(dirname, filename, 512);

	int i = strlen(dirname)-1;

	while (i >= 0 && dirname[i] != '/')
		dirname[i--] = 0;

	if (i <= 0)
		strncpy(dirname, "./", 512);
}

int list_files(char ***files, const char *path, const char *ext)
{
	struct dirent *entry;
	DIR *dir;
	int n = 0;
	int lenx = strlen(ext);

	// count number of entries
	dir = opendir(path);

	while (entry = readdir(dir))
	{
		if (is_file(entry->d_name))
		{
			int len = strlen(entry->d_name);

			if (len > lenx)
			{
				if (strcmp(&entry->d_name[len-lenx], ext) == 0)
					n++;
			}
		}
	}

	closedir(dir);

	*files = (char **)malloc(n * sizeof(char *));

	int i;
	for (i=0; i<n; i++)
	{
		(*files)[i] = (char *)malloc(512 * sizeof(char));
		memset((*files)[i], 0, 512 * sizeof(char));
	}

	i = 0;

	dir = opendir(path);

	while (entry = readdir(dir))
	{
		if (is_file(entry->d_name))
		{
			int len = strlen(entry->d_name);

			if (len > lenx)
			{
				if (strcmp(&entry->d_name[len-lenx], ext) == 0)
				{
					strncpy((*files)[i], entry->d_name, 512);
					i++;
				}
			}
		}
	}

	closedir(dir);

	return n;
}

int cstring_cmp(const void *a, const void *b) 
{ 
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;

    return strcmp(*ia, *ib);
}

void sort_files(char **files, int n)
{
	qsort(files, n, sizeof(char *), cstring_cmp);
}

int find_file(char **files, int n, char *filename)
{
	int i;
	for (i=0; i<n; i++)
		if (strncmp(files[i], filename, 512) == 0)
			return i;

	return -1;
}

int main(int argc, char *argv[])
{
	char *param = NULL;
	char dirname[512] = "./";

	if (argc > 1)
	{
		param = argv[1];

		if (!is_dir(param) && !is_file(param))
		{
			printf("bad path");
			exit(0);
		}

		file_dir(param, dirname);
	}

	num = list_files(&files, dirname, ".bm3");
	sort_files(files, num);

	if (is_file(param))
		cur = find_file(files, num, param);
/*
	printf("%d\n", n);

	int i;
	for (i=0; i<n; i++)
		printf("%s\n", files[i]);
*/
	atexit(cleanup);

	// init glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);

	glutInitWindowSize(width, height);
	glutCreateWindow(name);

	glewInit();

	glClearColor(1,1,1,1);

	load_bm3(files[cur]);

	glutDisplayFunc(display3d);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutIdleFunc(update);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

	glutMainLoop();
}

