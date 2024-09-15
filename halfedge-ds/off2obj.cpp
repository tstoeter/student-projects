#include <iostream>
#include <algorithm>
#include "double3.hpp"
#include "heds.hpp"

using namespace std;

// mesh variables
vector<double> vertices;
vector<int> faces;
vector<double> colors;
vector<double> face_normals;
vector<double> vertex_normals;

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

// load an off triangle mesh
int save_obj(const char *filename, vector<double>& vertices, vector<int>& faces, vector<double>& normals)
{
	ofstream file(filename);

	// cannot open file
	if (!file.is_open())
		return 1;

	file << "g" << endl;

	// store vertices
	for (int i=0; i<vertices.size(); i+=3)
		file << "v" << " " << vertices[i+0] << " " << vertices[i+1] << " " << vertices[i+2] << endl;

	// store normals
	for (int i=0; i<normals.size(); i+=3)
		file << "vn" << " " << normals[i+0] << " " << normals[i+1] << " " << normals[i+2] << endl;

	// store faces
	for (int i=0; i<faces.size(); i+=3)
		file << "f" << " " << faces[i+0]+1 << "//" << faces[i+0]+1 << " " << faces[i+1]+1 << "//" << faces[i+1]+1 << " " << faces[i+2]+1 << "//" << faces[i+2]+1 << endl;

	file.close();

	return 0;
}

vector<double> compute_face_normals(const vector<double>& vertices, const vector<int>& faces)
{
	int size = faces.size();
	vector<double> face_normals(size);

	#pragma omp parallel for
	for (int i=0; i<size; i+=3)
	{
		int v1 = 3*faces[i+0];
		int v2 = 3*faces[i+1];
		int v3 = 3*faces[i+2];

		double3 a = double3(&vertices[v2]) - double3(&vertices[v1]);
		double3 b = double3(&vertices[v3]) - double3(&vertices[v1]);

		double3 n = cross(a, b);

		face_normals[i+0] = -n.x;
		face_normals[i+1] = -n.y;
		face_normals[i+2] = -n.z;
	}

	return face_normals;
}

vector<double> compute_vertex_normals(heds& he, const vector<double>& face_normals)
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
				n += double3(&face_normals[3*k]);
		}

		n = normalize(n);

		vertex_normals[3*i+0] = n.x;
		vertex_normals[3*i+1] = n.y;
		vertex_normals[3*i+2] = n.z;
	}

	return vertex_normals;
}

int main(int argc, char *argv[])
{
	printf("off2obj :: basic off to obj mesh file converter\n");
	printf("converts to obj mesh file and computes vertex normals\n");
	printf("supports triangle meshes only\n");
	printf("usage: ./off2obj [-f] <off file> <obj file>\n");
	printf("options: -f flips vertex order for faces\n\n");

	if (argc < 3 || (argc == 4 && string(argv[1]).compare("-f") != 0))
	{
		printf("bad command line options\n");
		exit(1);
	}

	printf("reading %s...\n", argv[argc-2]);
	load_off(argv[argc-2], vertices, faces);

	// flip vertex order if desired
	if (string(argv[1]).compare("-f") == 0)
		for (int i=0; i<faces.size(); i+=3)
			swap(faces[i+1], faces[i+2]);

	printf("computing face normals...\n");
	face_normals = compute_face_normals(vertices, faces);

	printf("creating halfedge data structure...\n");
	heds he = heds(vertices, faces);

	printf("computing vertex normals...\n");
	vertex_normals = compute_vertex_normals(he, face_normals);

	printf("saving %s...\n", argv[argc-1]);
	save_obj(argv[argc-1], vertices, faces, vertex_normals);

	return 0;
}

