#ifndef RENDER_HPP
#define RENDER_HPP

#include <GL/freeglut.h>
#include <vector>

#include "double3.hpp"
#include "ColorScale.hpp"

using namespace std;

void render_trimesh(const vector<double>& vertices, const vector<int>& faces, const vector<double>& colors)
{
	// activate and specify pointer to vertex array
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, &vertices[0]);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_DOUBLE, 0, &colors[0]);

	// draw indexed triangles from vertex arrays
	glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, &faces[0]);

	// deactivate vertex arrays after drawing
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

void render_trimesh(const vector<double>& vertices, const vector<int>& faces, const vector<double>& colors, const vector<double>& normals)
{
	// activate and specify pointer to vertex array
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, &vertices[0]);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_DOUBLE, 0, &colors[0]);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_DOUBLE, 0, &normals[0]);

	// draw indexed triangles from vertex arrays
	glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, &faces[0]);

	// deactivate vertex arrays after drawing
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

vector<int> trimesh_edges(const vector<int>& faces)
{
	vector<int> lines;

	for (int i=0; i<faces.size(); i+=3)
	{
		lines.push_back(faces[i]);
		lines.push_back(faces[i+1]);

		lines.push_back(faces[i+1]);
		lines.push_back(faces[i+2]);

		lines.push_back(faces[i+2]);
		lines.push_back(faces[i]);
	}

	return lines;
}

vector<int> mesh_boundary_edges(const heds& mesh)
{
	vector<int> lines;

	int start = 3*mesh.num_faces;

	for (int i=start; i<mesh.halfedges.size(); i++)
	{
		lines.push_back(mesh.halfedges[i].v1);
		lines.push_back(mesh.halfedges[i].v2);
	}

	return lines;
}

void render_lines(const vector<double>& vertices, const vector<int>& lines)
{
	glDisable(GL_LIGHTING);

	// activate and specify pointer to vertex array
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, &vertices[0]);
	
	// draw indexed lines from vertex arrays
	glDrawElements(GL_LINES, lines.size(), GL_UNSIGNED_INT, &lines[0]);

	// deactivate vertex arrays after drawing
	glDisableClientState(GL_VERTEX_ARRAY);

	glEnable(GL_LIGHTING);
}

void draw_boundary_edges(const vector<double>& vertices, const vector<int>& faces)
{
	glLineWidth(1.0);
	glColor3f(0,0,0);

	vector<int> lines;

	for (int i=0; i<faces.size(); i+=3)
	{
		lines.push_back(faces[i]);
		lines.push_back(faces[i+1]);

		lines.push_back(faces[i+1]);
		lines.push_back(faces[i+2]);

		lines.push_back(faces[i+2]);
		lines.push_back(faces[i]);
	}

	// activate and specify pointer to vertex array
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, &vertices[0]);
	
	// draw indexed lines from vertex arrays
	glDrawElements(GL_LINES, lines.size(), GL_UNSIGNED_INT, &lines[0]);

	// deactivate vertex arrays after drawing
	glDisableClientState(GL_VERTEX_ARRAY);
}

void draw_uvec_s(const vector<face_data>& fd, double s)
{
	glLineWidth(3.0);
	glColor3f(0,0,0);

	int max = 0;

	for (int i=1; i<fd.size(); i++)
		if (fd[i].u.norm2() > fd[max].u.norm2())
			max = i;

	s /= fd[max].u.norm();

	glBegin(GL_LINES);

		for (int i=0; i<fd.size(); i++)
		{
			if (fd[i].u.norm2() > 0)
			{
				double3 c = fd[i].c;
				double3 u = s * fd[i].u;

				glVertex3f(c.x, c.y, c.z);
				glVertex3f(c.x+u.x, c.y+u.y, c.z+u.z);
			}
		}

	glEnd();
}

void draw_uvec_n(const vector<face_data>& fd, const double s)
{
	glLineWidth(3.0);
	glColor3f(0,0,0);

	glBegin(GL_LINES);

		for (int i=0; i<fd.size(); i++)
		{
			if (fd[i].u.norm2() > 0)
			{
				double3 c = fd[i].c;
				double3 u = s * normalize(fd[i].u);

				glVertex3f(c.x, c.y, c.z);
				glVertex3f(c.x+u.x, c.y+u.y, c.z+u.z);
			}
		}

	glEnd();
}

void draw_centers(const vector<face_data>& fd, const double s)
{
	glPointSize(5.0);
	glColor3f(0,0,0);

	glBegin(GL_POINTS);

		for (int i=0; i<fd.size(); i++)
		{
			double3 c = fd[i].c;
			glVertex3f(c.x, c.y, c.z);
		}

	glEnd();
}

void draw_face_normals(const vector<face_data>& fd, const double s)
{
	glLineWidth(1.0);
	glColor3f(0,0,0);

	glBegin(GL_LINES);

		for (int i=0; i<fd.size(); i++)
		{
			double3 c = fd[i].c;
			double3 u = s * fd[i].n;

			glVertex3f(c.x, c.y, c.z);
			glVertex3f(c.x+u.x, c.y+u.y, c.z+u.z);
		}

	glEnd();
}

// get current camera view down vector
double3 update_down_vector()
{
	double3 down;

	double mat[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, mat);

	// matrix layout
	//	m[0]
	//  m[1]
	//  m[2]
	//  m[3]

	down.x = -mat[1];
	down.y = -mat[5];
	down.z = -mat[9];

	return down;
}

void color_dye(vector<double>& colors, const heds& mesh, const vector<face_data>& fd, const ColorScale& cs, double min, double max)
{
	int nv = mesh.num_vertices;

	for (int i=0; i<nv; i++)
	{
		double dye = 0;
		double A = 0;

		vector<int> fs = mesh.faces_around_vertex(i);

		for (int j=0; j<fs.size(); j++)
		{
			if (fs[j] == -1)
				continue;

			dye += fd[fs[j]].dye;
			A += fd[fs[j]].A;
		}

		double rho = dye/A;
		double r,g,b;
		cs.GetColor((rho-min)/(max-min), r, g, b);

		colors[3*i+0] = r;
		colors[3*i+1] = g;
		colors[3*i+2] = b;
	}
}

void color_velocity();

#endif

