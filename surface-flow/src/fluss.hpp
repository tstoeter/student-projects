#ifndef FLUSS_HPP
#define FLUSS_HPP

#include <cassert>
#include <cmath>
#include <omp.h>

#include "heds.hpp"
#include "double3.hpp"

struct face_data
{
	// geometrics
	double A;	// area
	double3 c;	// centroid
	double3 n;	// face normal

	// physics
	double m;	// contained fluid mass
	double3 u;	// velocity
	double dye;	// dye for visualization
};

struct edge_data
{
	// geometrics
	double3 e;	// normalized edge vector
	double3 n;	// normal on edge in triangle plane
	double l;	// edge length

	double d;	//
	double s;	// sine of angle
	double h;	//
	double hc;	//

	// physics
	double m;
	double3 u;
	double f;	// pressure force magnitude, directed along edge normal
	double dye;

	// management
	char visited;
};

vector<face_data> init_face_data(const heds& mesh)
{
	int nf = mesh.num_faces;
	vector<face_data> fd(nf);

	for (int i=0; i<nf; i++)
	{
		// get vertices and edges
		int i1 = mesh.faces[3*i+0];
		int i2 = mesh.faces[3*i+1];
		int i3 = mesh.faces[3*i+2];

		double3 v1 = double3(&mesh.vertices[3*i1]);
		double3 v2 = double3(&mesh.vertices[3*i2]);
		double3 v3 = double3(&mesh.vertices[3*i3]);

		double3 e1 = double3(v1, v2);
		double3 e2 = double3(v2, v3);
//		double3 e3 = double3(v3, v1);
		double3 e4 = double3(v1, v3);

		// set triangle area, mass and velocity
		fd[i].A = 0.5 * cross(e1,e4).norm();
		fd[i].m = fd[i].A;								// initial density = 1.0, so m = A
		fd[i].u = zero;									// initial velocity = 0
		fd[i].n = normalize(cross(e1,e4));				// face normal

		assert(fd[i].A > 0);
		assert(fd[i].m > 0);

		// compute triangle centroid
		double3 m = v2 + (0.5 * e2);
		fd[i].c = v1 + ((2.0/3.0) * double3(v1,m));

		// set dye
		fd[i].dye = 0;
	}

	return fd;
}

vector<edge_data> init_edge_data(const heds& mesh, const vector<face_data>& fd)
{
	int nh = mesh.num_halfedges;
	vector<edge_data> ed(nh);

	for (int i=0; i<nh; i++)
	{
		// get vertices and edge
		int i1 = mesh.halfedges[i].v1;
		int i2 = mesh.halfedges[i].v2;

		int j = mesh.halfedges[i].next;
		int i3 = mesh.halfedges[j].v2;

		double3 v1 = double3(&mesh.vertices[3*i1]);
		double3 v2 = double3(&mesh.vertices[3*i2]);
		double3 v3 = double3(&mesh.vertices[3*i3]);

		double3 e = double3(v1, v2);

		// set edge and magnitude
		ed[i].e = normalize(e);
		ed[i].l = e.norm();
		ed[i].m = 0;
		ed[i].u = zero;
		ed[i].dye = 0;
		ed[i].f = 0;
		ed[i].visited = 0;

		// compute face dependent values
		int f = mesh.halfedge_face(i);

		if (f < 0)	// halfedge does not have a face
		{
			// set some default values
			ed[i].n = zero;
			ed[i].d = 0;
			ed[i].s = 0;
			ed[i].h = 0;
			ed[i].hc = 0;
		}
		else
		{
			// edge normal
			ed[i].n = normalize(cross(e, fd[f].n));

			// sine of angle at v1
			double3 g = double3(v1, v3);
			ed[i].s = cross(e,g).norm() / (e.norm()*g.norm());

			// altitude on edge
			ed[i].h = 2*fd[f].A / ed[i].l;

			// altitude from centroid to edge
			ed[i].hc = 2*fd[f].A / (3*ed[i].l);
		}
	}

	return ed;
}

int find_cutting_edge(const heds& mesh, const vector<face_data>& fd, const vector<edge_data>& ed, int f, const double3& d)
{
	double dots[3];
	int maxdot = 0;

	vector<int> h = mesh.halfedges_of_face(f);
	double3 n = fd[f].n;

	for (int i=0; i<3; i++)
	{
		double3 e = ed[h[i]].l * ed[h[i]].e;
		dots[i] = dot(n, cross(e, d));

		if (abs(dots[i]) > abs(dots[maxdot]))
			maxdot = i;
	}

	return h[maxdot];
}

bool compute_areas(const heds& mesh, const vector<face_data>& fd, const vector<edge_data>& ed, int f, const double3& d, int ce, double A[3], int hi[3], int ho[3])
{
	// halfedge indices, vertex order has to be clock-wise!
	int i = mesh.halfedges[ce].next;
	int j = mesh.halfedges[ce].prev;
	int k = ce;

	double di = abs(dot(d, ed[i].n));
	double dj = abs(dot(d, ed[j].n));
	double dk = abs(dot(d, ed[k].n));

	assert(dk < ed[k].h);

	// trapezoid formulas
	A[0] = di/2 * (2*ed[i].l - dk/ed[i].s);		// next edge
	A[1] = dj/2 * (2*ed[j].l - dk/ed[k].s);		// prev edge
	A[2] = A[0] + A[1];							// cutting edge

	assert(A[0] >= 0);
	assert(A[1] >= 0);
	assert(A[2] >= 0);
	assert(A[2] <= fd[f].A);

	// store inner halfedge indices
	hi[0] = i;
	hi[1] = j;
	hi[2] = k;

	// store outter halfedge indices
	ho[0] = mesh.halfedges[i].pair;
	ho[1] = mesh.halfedges[j].pair;
	ho[2] = mesh.halfedges[k].pair;

	// areas A[i], A[j] outside or inside of triangle?
	return (dot(d, ed[k].n) < 0);
}

void advect_mass_momentum(const heds& mesh, vector<face_data>& fd, vector<edge_data>& ed, const double dt)
{
	int nf = mesh.num_faces;

	#pragma omp parallel for
	for (int i=0; i<nf; i++)
	{
		int hi[3], ho[3];	// inner and outter halfedges of face
		double A[3];

		double m = fd[i].m;
		double Ar = fd[i].A;
		double3 u = fd[i].u;
		double3 d = u * dt;

		if (d.norm2() <= 0)
			continue;

		int ce = find_cutting_edge(mesh, fd, ed, i, d);
		bool out = compute_areas(mesh, fd, ed, i, d, ce, A, hi, ho);

		if (out)
		{
			ed[ho[0]].m = A[0]/Ar * m;
			ed[ho[1]].m = A[1]/Ar * m;

			ed[ho[0]].u = decompose(u, ed[hi[0]].n, ed[hi[0]].e);
			ed[ho[1]].u = decompose(u, ed[hi[1]].n, ed[hi[1]].e);
		}
		else
		{
			ed[ho[2]].m = A[2]/Ar * m;
			ed[ho[2]].u = decompose(u, ed[hi[2]].n, ed[hi[2]].e);
		}

		// decrease face mass
		fd[i].m = (Ar-A[2])/Ar * m;
	}
}

double update_mass_momentum(const heds& mesh, vector<face_data>& fd, vector<edge_data>& ed, const double rho0)
{
	int nf = mesh.num_faces;
	double rmax = 0;

	#pragma omp parallel for reduction(max:rmax)
	for (int i=0; i<nf; i++)
	{
		vector<int> h = mesh.halfedges_of_face(i);

		assert(h[0] >= 0);
		assert(h[1] >= 0);
		assert(h[2] >= 0);

		double3 p = fd[i].m * fd[i].u;

		for (int j=0; j<3; j++)
		{
			double m = ed[h[j]].m;
			double3 u = compose(ed[h[j]].u, ed[h[j]].n, ed[h[j]].e);

			if (m > 0)
				p += m * u;

			fd[i].m += m;
			assert(fd[i].m >= 0);

			// reset values
			ed[h[j]].m = 0;
			ed[h[j]].u = zero;

			ed[h[j]].visited = 0;
		}

		fd[i].u = (1.0/fd[i].m) * p;

		double rho = fd[i].m / fd[i].A;
		double r = abs(rho-rho0);

		if (r > rmax)
			rmax = r;
	}

	return rmax;
}

void apply_gravity(const heds& mesh, vector<face_data>& fd, const double3& down, const double& g, const double& dt)
{
	int nf = mesh.num_faces;

	#pragma omp parallel for
	for (int i=0; i<nf; i++)
	{
		vector<int> h = mesh.halfedges_of_face(i);

		assert(h[0] >= 0);
		assert(h[1] >= 0);
		assert(h[2] >= 0);

		if (fd[i].dye <= 0)
			continue;

		// compute gravitational force acting on triangle
		double rho = fd[i].dye / fd[i].A;
		double3 fg = (rho*g) * plane_project(down, fd[i].n);

		// a = f/m; u = a*dt => u = dt/m * f
		double3 ug = (dt/(fd[i].m+fd[i].dye)) * fg;
		fd[i].u += ug;
	}
}

void advect_dye(const heds& mesh, vector<face_data>& fd, vector<edge_data>& ed, double dt)
{
	int nf = mesh.num_faces;

	#pragma omp parallel for
	for (int i=0; i<nf; i++)
	{
		int hi[3], ho[3];
		double A[3];

		double Ar = fd[i].A;
		double3 u = fd[i].u;
		double3 d = u * dt;
		double dye = fd[i].dye;

		if (d.norm2() <= 0)
			continue;

		int ce = find_cutting_edge(mesh, fd, ed, i, d);
		bool out = compute_areas(mesh, fd, ed, i, d, ce, A, hi, ho);

		if (out)
		{
			ed[ho[0]].dye = A[0]/Ar * dye;
			ed[ho[1]].dye = A[1]/Ar * dye;
		}
		else
		{
			ed[ho[2]].dye = A[2]/Ar * dye;
		}

		// decrease face dye
		fd[i].dye = (Ar-A[2])/Ar * dye;
	}
}

void update_dye(const heds& mesh, vector<face_data>& fd, vector<edge_data>& ed)
{
	int nf = mesh.num_faces;

	#pragma omp parallel for
	for (int i=0; i<nf; i++)
	{
		vector<int> h = mesh.halfedges_of_face(i);

		assert(h[0] >= 0);
		assert(h[1] >= 0);
		assert(h[2] >= 0);

		for (int j=0; j<3; j++)
		{
			double dye = ed[h[j]].dye;
			fd[i].dye += dye;

			// reset values
			ed[h[j]].dye = 0;
		}
	}
}

void relax_pressure(const heds& mesh, vector<face_data>& fd, vector<edge_data>& ed, const double kp, const double dt)
{
	int nf = mesh.num_faces;
	int nh = mesh.num_halfedges;

	// compute internal pressure force inside triangle acting outward on triangle edges
	#pragma omp parallel for
	for (int i=0; i<nf; i++)
	{
		vector<int> h = mesh.halfedges_of_face(i);

		assert(h[0] >= 0);
		assert(h[1] >= 0);
		assert(h[2] >= 0);

		// compute pressure force acting on triangle edges
		double rho = fd[i].m / fd[i].A;
		double p = kp * rho;

		for (int j=0; j<3; j++)
			ed[h[j]].f = p * ed[h[j]].l;
	}

	// compute pressure force difference for all edges
	//#pragma omp parallel for
	for (int i=0; i<nh; i++)
	{
		if (ed[i].visited)
			continue;

		int j = mesh.halfedges[i].pair;

		// compute resulting force between adjacent triangles
		// two pressure forces + viscosity
		double fi = ed[i].f;
		double fj = ed[j].f;

		// pay attention to boundary conditions
		if (mesh.is_boundary_halfedge(i) || mesh.is_boundary_halfedge(j))
		{
			ed[i].f = 0;
			ed[j].f = 0;
		}
		else
		{
			ed[i].f = fi - fj;
			ed[j].f = fj - fi;
		}

		ed[i].visited = 1;
		ed[j].visited = 1;
	}

	// do expansion according to pressure difference on edges
	#pragma omp parallel for
	for (int i=0; i<nf; i++)
	{
		vector<int> h = mesh.halfedges_of_face(i);

		assert(h[0] >= 0);
		assert(h[1] >= 0);
		assert(h[2] >= 0);

		// split triangle into three sub-triangles
		double A[3];
		double Ar = fd[i].A / 3;
		double m = fd[i].m / 3;
		double3 u[3];

		// compute areas after edge displacement
		for (int j=0; j<3; j++)
		{
			double la = ed[h[j]].l;						// original edge length
			u[j] = (ed[h[j]].f/m * dt) * ed[h[j]].n;	// displacement vector for edge
			double ds = dot(u[j], ed[h[j]].n) * dt;		// signed displacement segment
			double lb = la * (1.0 + ds/ed[h[j]].hc);	// length of displaced edge

			A[j] = 0.5*(la+lb)*ds;						// signed area change

			if (A[j] <= 0)								// we consider outgoing mass, so clamp negative area (incoming mass) to zero
				A[j] = 0;
		}

		// total remaining mass and momentum
		double m_tot = 0;
		double3 p_tot = zero;

		// compute mass and momentum per area for inelastic collision
		for (int j=0; j<3; j++)
		{
			int k = mesh.halfedges[h[j]].pair;	// pair halfedge for assigning outgoing mass and momentum

			// outside area, some mass outside, some still inside
			double3 uu = fd[i].u + u[j];
			double mm = m * A[j]/Ar;	// outside mass

			ed[k].m = mm;
			ed[k].u = decompose(uu, ed[h[j]].n, ed[h[j]].e);

//			if (m <= mm)
//				printf("m=%lf Aj=%lf Ar=%lf Aj/Ar=%lf\n", m, A[j], Ar, A[j]/Ar);
			assert(m >= mm);

			// inelastic collision for inside mass
			p_tot += (m-mm) * uu;
			m_tot += (m-mm);
		}

		fd[i].u = (1.0/m_tot) * p_tot;
		fd[i].m = m_tot;
	}
}

void noslip_boundary_condition(const heds& mesh, vector<edge_data>& ed)
{
	// all boundary edges were appended to the halfedge data structure
	// so they are stored at the end of the halfedge list

	int nhs = 3*mesh.num_faces;		// start of boundary edges
	int nhe = mesh.num_halfedges;	// end of boundary edges

	#pragma omp parallel for
	for (int i=nhs; i<nhe; i++)
	{
		int j = mesh.halfedges[i].pair;

		// return outgoing mass to originating face with zero momentum
		ed[j].m = ed[i].m;
		ed[j].u = zero;
		ed[j].f = 0;

		// nothing is going out through boundary edges
		ed[i].m = 0;
		ed[i].u = zero;
		ed[i].f = 0;
	}
}

void noslip_boundary_condition_dye(const heds& mesh, vector<edge_data>& ed)
{
	// all boundary edges were appended to the halfedge data structure
	// so they are stored at the end of the halfedge list

	int nhs = 3*mesh.num_faces;		// start of boundary edges
	int nhe = mesh.num_halfedges;	// end of boundary edges

	#pragma omp parallel for
	for (int i=nhs; i<nhe; i++)
	{
		int j = mesh.halfedges[i].pair;

		// return outgoing mass to originating face with zero momentum
		ed[j].dye = ed[i].dye;

		// nothing is going out through boundary edges
		ed[i].dye = 0;
	}
}

struct params
{
	double adt;		// advection time step

	double g;		// gravitational constant
	double3 down;	// normalized down vector
	double fdt;		// force apply time step

	double k;		// pressure constant
	double pdt;		// pressure relaxation time step
	double rho0;	// equilibrium pressure
	double eps;		// compressibility constant

	double ddt;		// dye advection time step
	unsigned int nda;// num dye advection iterations

	unsigned int tri;// triangle to fill dye in
};

double total_mass(vector<face_data>& fd)
{
	int nf = fd.size();

	double m = 0.0;

	for (int i=0; i<nf; i++)
		m += fd[i].m;

	return m;
}

double total_dye(vector<face_data>& fd)
{
	int nf = fd.size();

	double m = 0.0;

	for (int i=0; i<nf; i++)
		m += fd[i].dye;

	return m;
}

int update_fluss(const heds& mesh, vector<face_data>& fd, vector<edge_data>& ed, const params& p)
{
	double drho;
	int numsteps = 0;

	apply_gravity(mesh, fd, p.down, p.g, p.fdt);

	advect_mass_momentum(mesh, fd, ed, p.adt);

	noslip_boundary_condition(mesh, ed);

	drho = update_mass_momentum(mesh, fd, ed, p.rho0);

	printf("mtot=%lf drho=%lf dtot=%lf\n", total_mass(fd), drho, total_dye(fd));

	while (drho > p.eps)// && numsteps < 100)
	{
//		printf("step=%d mtot=%lf drho=%lf\n", numsteps, total_mass(fd), drho);
		relax_pressure(mesh, fd, ed, p.k, p.pdt);

		noslip_boundary_condition(mesh, ed);

		drho = update_mass_momentum(mesh, fd, ed, p.rho0);

		numsteps++;
	}

	// perform dye advection for visualization
	for (int i=0; i<p.nda; i++)
	{
//		printf("da.. fd[0].dye=%lf pdt=%lf\n", fd[0].dye, p.ddt);
		advect_dye(mesh, fd, ed, p.ddt);
		noslip_boundary_condition_dye(mesh, ed);
		update_dye(mesh, fd, ed);
	}

	return numsteps;
}

#endif

