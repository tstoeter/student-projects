#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <GL/freeglut.h>

#include "heds.hpp"
#include "fluss.hpp"
#include "render.hpp"

using namespace std;

// glut variables
char name[] = "Fluid Simulation on Curved Triangle Mesh Surfaces";	// window title
int width = 600, height = 600;		// window size

double zoom = 1.0;
double rx = 0, ry = 0, mx = 0, my = 0;
int oldx, oldy;
bool zoomMesh, rotateMesh, moveMesh;
int re = 0, ce = 0;
int paused = 0;
int normed = 0;

// mesh variables
vector<double> vertices;
vector<int> faces;
vector<double> colors;
vector<double> normals;
heds* mesh;
ColorScale* cs;
vector<int> edges;
vector<int> boundary_edges;

// simulation variables
params p;
vector<face_data> fd;
vector<edge_data> ed;

////////////////////////////////////////////////////////////////////////////////

void normalize_mesh(vector<double>& vertices)
{
	double max = 0;

	for (int i=0; i<vertices.size(); i+=3)
	{
		double len = double3(&vertices[i]).norm2();

		if (len > max)
			max = len;
	}

	for (int i=0; i<vertices.size(); i++)
		vertices[i] /= sqrt(max);
}

vector<double> compute_vertex_normals(const heds& he, const vector<face_data>& fd)
{
	int size = he.num_vertices;

	vector<double> vertex_normals(3*size);

	#pragma omp parallel for schedule(dynamic)
	for (int i=0; i<size; i++)
	{
		vector<int> fi = he.faces_around_vertex(i);

		double3 n = zero;

		for (int j=0; j<fi.size(); j++)
		{
			int k = fi[j];

			if (k >= 0)
				n += fd[k].n;
		}

		n = normalize(n);

		vertex_normals[3*i+0] = n.x;
		vertex_normals[3*i+1] = n.y;
		vertex_normals[3*i+2] = n.z;
	}

	return vertex_normals;
}

////////////////////////////////////////////////////////////////////////////////

void init(int argc, char *argv[])
{
	// read data from stdin
	char filename[256];
	cin.getline(filename, 256);

	cin >> p.adt;

	cin >> p.g;
	cin >> p.fdt;

	cin >> p.k;
	cin >> p.pdt;
	cin >> p.rho0;
	cin >> p.eps;

	cin >> p.ddt;
	cin >> p.nda;

	cin >> p.tri;

	cs = new ColorScale(5);
	cs->AddPoint(0.1, 0, 0, 1);
	cs->AddPoint(0.3, 0, 1, 1);
	cs->AddPoint(0.5, 0, 1, 0);
	cs->AddPoint(0.7, 1, 1, 0);
	cs->AddPoint(0.9, 1, 0, 0);

	srand(time(NULL));

	load_off(filename, vertices, faces);
	normalize_mesh(vertices);

	vector<double> tmp(vertices.size(), 0);
	colors = tmp;

	mesh = new heds(vertices, faces);

	fd = init_face_data(*mesh);
	ed = init_edge_data(*mesh, fd);

	normals = compute_vertex_normals(*mesh, fd);

	edges = trimesh_edges(faces);
	boundary_edges = mesh_boundary_edges(*mesh);

//	fd[0].u = plane_project(double3(0.1,0.1,0.1), fd[0].n);
//	fd[0].m *= 1.5;
//	fd[0].dye = 0.1;

	GLfloat LightDiffuse[] = {0.5f, 0.5f, 0.5f, 0.0f};
	GLfloat LightPosition[] = {0.0f, 0.0f, -10.0f, 0.0f};

	glClearColor(1,1,1,1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	glPushMatrix();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
//	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	glEnable(GL_MULTISAMPLE);

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0*zoom, (double)width/height, 0.1, 10.0);
	gluLookAt(0, 0, 4, 0, 0, 0, 0, 1, 0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	p.down = update_down_vector();
}

void display()
{
	int t0 = glutGet(GLUT_ELAPSED_TIME);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0*zoom, (double)width/height, 0.1, 10.0);
	gluLookAt(0, 0, 4, 0, 0, 0, 0, 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushMatrix();
	glLoadIdentity();

	glTranslatef(mx, my, 0);
	glRotatef(rx, 1, 0, 0);
	glRotatef(ry, 0, 1, 0);

	// update view-dependent down vector
	p.down = update_down_vector();

	color_dye(colors, *mesh, fd, *cs, 0, 1);
	render_trimesh(vertices, faces, colors, normals);

	glLineWidth(1.0); glColor3f(0,0,0);
	render_lines(vertices, edges);

	glLineWidth(2.0); glColor3f(1,0,1);
	render_lines(vertices, boundary_edges);

	draw_centers(fd, 3.0);

	if (normed)
		draw_uvec_n(fd, 0.05);
	else
		draw_uvec_s(fd, 0.1);

	glPopMatrix();

	glutSwapBuffers();

	int t1 = glutGet(GLUT_ELAPSED_TIME);

	printf("render: %d ms\n", t1-t0);
}

void update()
{
	if (paused == 0)
	{
		int t0 = glutGet(GLUT_ELAPSED_TIME);

		int n = update_fluss(*mesh, fd, ed, p);

		int t1 = glutGet(GLUT_ELAPSED_TIME);

		printf("compute: %d ms, %d relax steps\n", t1-t0, n);
	}

	// redraw
	glutPostRedisplay();
}

void trimesh_flood_fill(const heds& mesh, vector<face_data>& fd)
{
	int nf = mesh.num_faces;

	for (int i=0; i<nf; i++)
	{
		if (fd[i].dye > 0)
		{
			vector<int> fs = mesh.faces_around_face(i);

			for (int j=0; j<3; j++)
				fd[fs[j]].dye = 1;
		}
	}
}

void keyboard(unsigned char key, int px, int py)
{
	if (key == 27 || key == 'q' || key == 'Q')
		exit(0);

	if (key == ' ')
		paused = 1-paused;

	if (key == '+')
		zoom -= 0.1;

	if (key == '-')
		zoom += 0.1;

	if (key == 'f' || key == 'F')
		fd[p.tri].dye += 0.01;

	if (key == 'n' || key == 'N')
		normed = 1-normed;
}

void mouse(int button, int state, int x, int y)
{
	rotateMesh = false;
	zoomMesh = false;

	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			rotateMesh = true;
			oldx = x;
			oldy = y;
		}

		if (button == GLUT_RIGHT_BUTTON)
		{
			moveMesh = true;
			oldx = x;
			oldy = y;
		}
	}
}

void motion(int x, int y)
{
	if (rotateMesh)
	{
		rx += oldy-y;
		ry += oldx-x;
		oldx = x;
		oldy = y;
	}

	if (moveMesh)
	{
		mx -= 0.01*(oldx-x);
		my += 0.01*(oldy-y);
		oldx = x;
		oldy = y;
	}
}

void wheel(int wheel, int dir, int x, int y)
{
	zoom += 0.1*dir;
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(width, height);

	glutCreateWindow(name);

	glutDisplayFunc(display);
	glutIdleFunc(update);
//	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMouseWheelFunc(wheel);

	init(argc, argv);

	glutMainLoop();

	return 0;
}

