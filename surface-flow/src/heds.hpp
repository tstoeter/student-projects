// heds :: a simple and tiny halfedge data structure for triangle meshes

#ifndef HEDS_HPP
#define HEDS_HPP

/*
	DOCUMENT: docs
	Documentation of the heds class and its member functions.

	Class heds creates a halfedge data structure for a given input triangle mesh
	and provides functions for adjacency queries on this mesh.
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <algorithm>

#include "double3.hpp"

using namespace std;

/*
	TYPEDEF: halfedge
	Halfedge structure composed of array indices.

	VARIABLES:
		int v1, v2				- Indices into vertex array of the starting and ending vertex of a halfedge.
		int next, prev, pair	- Indices into halfedge array of the next, previous and pair halfedges of a halfedge.
*/
struct halfedge
{
	int v1, v2;				
	int next, prev, pair;
};

// comparator for sorting halfedges
bool operator<(halfedge a, halfedge b)
{
	return a.v1 < b.v1;
}

// comparator for binary search to accelerate finding pairs
bool operator<(halfedge a, int v1)
{
	return a.v1 < v1;
}

/*
	CLASS: heds class
	The complete halfedge data structure class.
*/
class heds
{
	private:
		void insert_face(int v1, int v2, int v3);
		int find_pair(int i);
		int connect_pairs();
		
	public:
		const vector<double>& vertices;
		const vector<int>& faces;
		int num_vertices;
		int num_faces;

		// GROUP: Member Variables
		/*
			VARIABLE: halfedges
			List of halfedges.
		*/
		vector<halfedge> halfedges;
		int num_halfedges;

		/*
			VARIABLE: vertex_halfedge
			Lookup table storing an outgoing halfedge for every vertex.
		*/
		vector<int> vertex_halfedge;

		// GROUP: Constructor
		/*
			CONSTRUCTOR: heds
			Constructor of heds class creates the halfedge data structure for a given input triangle mesh.
			The triangle mesh has to be provided as a vertex array together with a list of vertex indices for the triangle faces.

			PARAMETERS:
				v	- Vertex list of triangle mesh, where the vertices and their three coordinates are stored consequetively, e.g., the STL vector contains: ax, ay, az, bx, by, bz, cx, cy, cz, ... for the vertices a, b, c, ...
				f	- Face list of triangle mesh as vertex indices, where the faces with their three vertex indices are also stored consequtively in an STL vector.
		*/

		heds(const vector<double>& v, const vector<int>& f);

		bool is_boundary_halfedge(int h) const;
		int halfedge_face(int h) const;

		vector<int> halfedges_around_vertex(int v) const;
		vector<int> vertices_around_vertex(int v) const;
		vector<int> faces_around_vertex(int v) const;

		vector<int> halfedges_of_face(int f) const;
		vector<int> faces_around_face(int f) const;
};

heds::heds(const vector<double>& v, const vector<int>& f) : vertices(v), faces(f), num_vertices(v.size()/3), num_faces(f.size()/3)
{
	vertex_halfedge = vector<int>(num_vertices, -1);

	// create halfedges for all faces
	for (int i=0; i<num_faces; i++)
		insert_face(faces[3*i+0], faces[3*i+1], faces[3*i+2]);

//	sort(halfedges.begin(), halfedges.end());

	// connect halfedge pairs
	connect_pairs();

	num_halfedges = halfedges.size();
}

// GROUP: Member Functions
/*
	FUNCTION: is_boundary_halfedge
	Determines weather a halfedge resides at the boundary of the mesh.

	PARAMETERS:
		h	- Index of the halfedge.

	RETURNS:
		True, if the halfedge h resides at the boundary of the mesh.
		False, otherwise.
*/
bool heds::is_boundary_halfedge(int h) const
{
	assert(h >= 0 && h < num_halfedges);

	return h >= 3*num_faces;
}

/*
	FUNCTION: halfedge_face
	Determines the face a halfedge belongs to.

	PARAMETERS:
		h	- Index of the halfedge.

	RETURNS:
		Index into face list, storing the first vertex index of the face.
*/
int heds::halfedge_face(int h) const
{
	assert(h >= 0 && h < num_halfedges);

	if (is_boundary_halfedge(h))
		return -1;

	// returns index on first vertex faces list
	return h/3;
}

/*
	FUNCTION: halfedges_around_vertex
	Finds all outgoing halfedges for a specified vertex.

	PARAMETERS:
		v	- Index of the specified vertex.

	RETURNS:
		Vector containing the indices of the outgoing halfedges of vertex v.
*/
vector<int> heds::halfedges_around_vertex(int v) const
{
	assert(v >= 0 && v < num_vertices);

	vector<int> hes;

	// set starting halfedge going out of vertex v
	int start = vertex_halfedge[v];
	int pair, curr = start;

	// walk around v using directed halfedges and pairs v
	// until starting halfedge is reached again
	do
	{
		// store vertices
		hes.push_back(curr);

		// go to next outgoing halfedge
		pair = halfedges[curr].pair;
		curr = halfedges[pair].next;
	} while (curr != start);

	return hes;
}

/*
	FUNCTION: vertices_around_vertex
	Finds the one-ring neighborhood vertices around a specified vertex.

	PARAMETERS:
		v	- Index of the specified vertex.

	RETURNS:
		Vector containing the indices of the one-ring neighborhood vertices around v.
*/
vector<int> heds::vertices_around_vertex(int v) const
{
	assert(v >= 0 && v < num_vertices);

	vector<int> hes = halfedges_around_vertex(v);
	vector<int> verts;

	for (int i=0; i<hes.size(); i++)
	{
		int j = hes[i];
		verts.push_back(halfedges[j].v2);
	}

	return verts;
}

/*
	FUNCTION: faces_around_vertex
	Finds the one-ring neighborhood faces around a specified vertex.

	PARAMETERS:
		v	- Index of the specified vertex.

	RETURNS:
		Vector containing the indices of the one-ring neighborhood faces around v.
*/
vector<int> heds::faces_around_vertex(int v) const
{
	assert(v >= 0 && v < num_vertices);

	vector<int> hes = halfedges_around_vertex(v);
	vector<int> fcs;

	for (int i=0; i<hes.size(); i++)
	{
		int j = hes[i];
		fcs.push_back(halfedge_face(j));
	}

	return fcs;
}

/*
	FUNCTION: halfedges_of_face
	Finds all three inner halfedges bordering a specified face.

	PARAMETERS:
		f	- Index of the specified face.

	RETURNS:
		Vector containing the indices of the halfedges bordering face f.
*/
vector<int> heds::halfedges_of_face(int f) const
{
	vector<int> hes(3);

	hes[0] = 3*f+0;
	hes[1] = 3*f+1;
	hes[2] = 3*f+2;

	return hes;
}

/*
	FUNCTION: faces_around_face
	Finds all three neighboring faces bordering a specified face.

	PARAMETERS:
		f	- Index of the specified face.

	RETURNS:
		Vector containing the indices of the neighboring faces bordering face f.
*/
vector<int> heds::faces_around_face(int f) const
{
	vector<int> hes = halfedges_of_face(f);
	vector<int> fcs;

	// get outter halfedges around face
	hes[0] = halfedges[hes[0]].pair;
	hes[1] = halfedges[hes[1]].pair;
	hes[2] = halfedges[hes[2]].pair;

	fcs.push_back(halfedge_face(hes[0]));
	fcs.push_back(halfedge_face(hes[1]));
	fcs.push_back(halfedge_face(hes[2]));

	return fcs;
}

// create and append new halfedges to the list for a given input face
void heds::insert_face(int v1, int v2, int v3)
{
	assert(v1 >= 0); assert(v2 >= 0); assert(v3 >= 0);

	int size = halfedges.size();
			
	halfedge he12 = {v1, v2, size+1, size+2, -1};	// i=size+0
	halfedge he23 = {v2, v3, size+2, size+0, -1};	// i=size+1
	halfedge he31 = {v3, v1, size+0, size+1, -1};	// i=size+2

	halfedges.push_back(he12);
	halfedges.push_back(he23);
	halfedges.push_back(he31);

	vertex_halfedge[v1] = size+0;
	vertex_halfedge[v2] = size+1;
	vertex_halfedge[v3] = size+2;
}

// find halfedge pair for edge i from the list of halfedges to connect halfedge pairs
int heds::find_pair(int i)
{
	assert(i >= 0);

	int v1 = halfedges[i].v1;
	int v2 = halfedges[i].v2;

	// slow linear search for pair
	for (int j=i+1; j<halfedges.size(); j++)
		if (halfedges[j].v1 == v2 && halfedges[j].v2 == v1)
			return j;

	return -1;

/*	// fast binary search for pair
	vector<halfedge>::iterator it = lower_bound(halfedges.begin(), halfedges.end(), v2);
	int j = it - halfedges.begin();

	while (halfedges[j].v1 == v2)
	{
		if (halfedges[j].v2 == v1)
			return j;

		j++;
	}

	return -1;
*/
}

int heds::connect_pairs()
{
	int size = halfedges.size();
	vector<int> bedge;	// list of boundary edges, which have no halfedge pair

	// find edge pairs and connect them in halfedge structure
	for (int i=0; i<size; i++)
	{
		if (halfedges[i].pair == -1)	// halfedge pair not yet set?
		{
			int j = find_pair(i);		// try to find pair from halfedge list

			if (j >= 0)					// pair found? then connect both halfedges
			{
				halfedges[i].pair = j;
				halfedges[j].pair = i;
			}
			else	// no pair found? then append to boundary edge list
				bedge.push_back(i);
		}
	}

	// boundary edges at the mesh boundary have no neighboring face
	// so there neither is a corresponding halfedge pair in the list
	// thus create halfedge pairs for boundary edges now
	for (int i=0; i<bedge.size(); i++)
	{
		int j = bedge[i];
		int v1 = halfedges[j].v1;
		int v2 = halfedges[j].v2;

		halfedges[j].pair = size+i;
		
		halfedge he = {v2, v1, -1, -1, j};
		
		halfedges.push_back(he);
	}

	// connect newly created boundary edge pairs with one another
	size = halfedges.size();
	int bedge_index = size - bedge.size();

	// slow O(n^2) for connecting boundary halfedges
	for (int i=bedge_index; i<size; i++)
	for (int j=bedge_index; j<size; j++)
	{
		if (halfedges[i].v2 == halfedges[j].v1)
		{
			halfedges[i].next = j;
			halfedges[j].prev = i;
		}
	}

	return bedge.size();
}

void check_halfedges(const vector<halfedge>& he)
{
	int n = he.size();

	for (int i=0; i<n; i++)
	{
		assert(he[i].v1 >= 0);
		assert(he[i].v2 >= 0);
		assert(he[i].next >= 0 && he[i].next < n);
		assert(he[i].prev >= 0 && he[i].prev < n);
		assert(he[i].pair >= 0 && he[i].pair < n);
	}
}

// load an off triangle mesh
int load_off(const char *filename, vector<double>& vertices, vector<int>& faces)
{
	ifstream file(filename);

	// cannot open file
	if (!file.is_open())
		return 1;

	string line;
	getline(file, line);

	// file is not an off mesh
	if (line.compare("OFF") != 0)
		return 2;

	int num_vertices, num_faces, num_edges;

	file >> num_vertices;
	file >> num_faces;
	file >> num_edges;

	// read vertices
	for (int i=0; i<num_vertices; i++)
	{
		double x, y, z;

		file >> x >> y >> z;

		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
	}

	// read faces
	for (int i=0; i<num_faces; i++)
	{
		int n, v1, v2, v3;

		file >> n;

		// mesh is no triangle mesh
		if (n != 3)
			return 3;

		file >> v1 >> v2 >> v3;

		faces.push_back(v1);
		faces.push_back(v2);
		faces.push_back(v3);
	}

	file.close();

	return 0;
}

#endif

