#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glew.h>
#include <cuda_gl_interop.h>
#include <iostream>
#include <list>
#include <vector>

#include "cutil.h"

#include "grains.h"
#include "visual.h"
#include "bmp.h"

using namespace std;


// simulation parameters
unsigned int w, h, d;
float kT = 1.0f;
int *rands;

// statistics variables
unsigned int *domain, *stats, ngrains;
list<int> grain_list;

// opengl buffers
struct cudaGraphicsResource *pbores;
GLuint pbo, texid;

texture<unsigned int, 2, cudaReadModeElementType> tex;
texture<unsigned int, 3, cudaReadModeElementType> tex3d;
cudaArray* d_array;

/////// mcp simulation code ////////////////////////////////

// relative nearest neighbor indices
__constant__ int nn2d[8*2] = {1,0, -1,0, 0,1, 0,-1, 1,1, -1,1, -1,-1, 1,-1};
__constant__ int nn3d[26*3] = {1,0,0, -1,0,0, 0,1,0, 0,-1,0, 1,1,0, -1,1,0, -1,-1,0, 1,-1,0,
			0,0,1, 1,0,1, -1,0,1, 0,1,1, 0,-1,1, 1,1,1, -1,1,1, -1,-1,1, 1,-1,1,
			0,0,-1, 1,0,-1, -1,0,-1, 0,1,-1, 0,-1,-1, 1,1,-1, -1,1,-1, -1,-1,-1, 1,-1,-1};

// delta function
__device__ int delta(int qi, int qj)
{
	return (qi == qj);
}

// convert absolute texture coords to (0,1) - normalized texture coords
__device__ float abs2norm(float x, float w)
{
	return (x+0.5)/w;
}

// computes energy difference of a cell (x,y) based on a new orientation qn and nearest neighbor orientations
__device__ int energy_diff2d_tex(int x, int y, int w, int h, int qo, int qn)
{
	int j, e=0, qj;

	for (j=0; j<16; j+=2)
	{
		qj = tex2D(tex, abs2norm(x+nn2d[j+0],w), abs2norm(y+nn2d[j+1],h));
		e += delta(qj, qo) - delta(qj, qn);
	}

	return e;
}

// computes energy difference of a cell (x,y) based on a new orientation qn and nearest neighbor orientations
__device__ int energy_diff3d_tex(int x, int y, int z, int w, int h, int d, int qo, int qn)
{
	int j, e=0, qj;

	for (j=0; j<78; j+=3)
	{
		qj = tex3D(tex3d, abs2norm(x+nn3d[j+0],w), abs2norm(y+nn3d[j+1],h), abs2norm(z+nn3d[j+2],d));
		e += delta(qj, qo) - delta(qj, qn);
	}

	return e;
}

// re-orientation probability depending on energy difference
__device__ float prob(int e, float kT)
{
	return __expf(-e/kT);
}

// 16-bit linear congruentional generator for pseudo random numbers
__device__ int lcg_rand(int x)
{
	return (__umul24(19961, x) + 13) % 0xFFFF;
}

// do one monte-carlo-potts iteration
__global__ void mcs2d(int *domain, int w, int h, float kT, int *rands)
{
	int x = threadIdx.x;
	int y = blockIdx.x;
	int i = __umul24(y,w) + x;

	int rand1 = rands[i];
	int rand2 = lcg_rand(rand1);

	// store prng state
	rands[i] = lcg_rand(rand2);

	int j = (rand1 & 7) << 1;
	int xx = x + nn2d[j+0];
	int yy = y + nn2d[j+1];

	int qo = tex2D(tex, abs2norm(x,w), abs2norm(y,h));
	int qn = tex2D(tex, abs2norm(xx,w), abs2norm(yy,h));

	if (qo != qn)
	{
		int e = energy_diff2d_tex(x, y, w, h, qo, qn);

		// probabilistic re-orientation
		if ((float)rand2/0xFFFF < prob(e,kT))
			domain[i] = qn;
	}
}

// kernel for bitmaps with w > 512
__global__ void mcs2d512(int *domain, int w, int h, float kT, int *rands)
{
	int i = (blockIdx.x << 9) + threadIdx.x;
	int x = i % w;
	int y = i / w;

	int rand1 = rands[i];
	int rand2 = lcg_rand(rand1);

	// store prng state
	rands[i] = lcg_rand(rand2);

	int j = (rand1 & 7) << 1;
	int xx = x + nn2d[j+0];
	int yy = y + nn2d[j+1];

	int qo = tex2D(tex, abs2norm(x,w), abs2norm(y,h));
	int qn = tex2D(tex, abs2norm(xx,w), abs2norm(yy,h));

	if (qo != qn)
	{
		int e = energy_diff2d_tex(x, y, w, h, qo, qn);

		// probabilistic re-orientation
		if ((float)rand2/0xFFFF < prob(e,kT))
			domain[i] = qn;
	}
}


// do one monte-carlo-potts iteration in 3d
__global__ void mcs3d(int *domain, int w, int h, int d, float kT, int *rands)
{
	int x = threadIdx.x;
	int y = blockIdx.x;
	int z = blockIdx.y;
	int i = __umul24(z, __umul24(w,h)) + __umul24(y,w) + x;

	int rand1 = rands[i];
	int rand2 = lcg_rand(rand1);

	// store prng state
	rands[i] = lcg_rand(rand2);

	int j = __umul24(3, (rand1 % 26));
	int xx = x + nn3d[j+0];
	int yy = y + nn3d[j+1];
	int zz = z + nn3d[j+2];

	int qo = tex3D(tex3d, abs2norm(x,w), abs2norm(y,h), abs2norm(z,d));
	int qn = tex3D(tex3d, abs2norm(xx,w), abs2norm(yy,h), abs2norm(zz,d));

	if (qo != qn)
	{
		int e = energy_diff3d_tex(x, y, z, w, h, d, qo, qn);

		// probabilistic re-orientation
		if ((float)rand2/0xFFFF < prob(e,kT))
			domain[i] = qn;
	}
}

////////////////////////////////////////////////////////////////////

void run_mcs2d(int w, int h, float kT)
{
	int *d_out;
	cudaGraphicsMapResources(1, &pbores, 0);
	size_t num_bytes;
	cudaGraphicsResourceGetMappedPointer((void**)&d_out, &num_bytes, pbores);

	// run cuda kernel
	if (w <= 512)
		mcs2d<<<h,w>>>(d_out, w, h, kT, rands);
	else
		mcs2d512<<<(w*h)/512,512>>>(d_out, w, h, kT, rands);

	CUT_CHECK_ERROR("kernel launch failure");

//	cudaThreadSynchronize();

	// copy output data to texture for next iteration
	cudaUnbindTexture(tex);
	cudaMemcpyToArray( d_array, 0, 0, d_out, w * h * sizeof(int), cudaMemcpyDeviceToDevice);
	cudaBindTextureToArray(tex, d_array);

	cudaGraphicsUnmapResources(1, &pbores, 0);
}

void run_mcs3d(int w, int h, int d, float kT)
{
	int *d_out;
	cudaGraphicsMapResources(1, &pbores, 0);
	size_t num_bytes;
	cudaGraphicsResourceGetMappedPointer((void**)&d_out, &num_bytes, pbores);

	dim3 gridDim(h,d);

	mcs3d<<<gridDim,w>>>(d_out, w, h, d, kT, rands);

	CUT_CHECK_ERROR("kernel launch failure");

//	cudaThreadSynchronize();

	// copy output data to texture for next iteration
	cudaUnbindTexture(tex);
	cudaMemcpy3DParms params = {0};
	params.srcPtr = make_cudaPitchedPtr(d_out, 0, w, h);
	params.extent = make_cudaExtent(w,h,d);
	params.dstArray = d_array;
	params.kind = cudaMemcpyDeviceToDevice;
	cudaMemcpy3D(&params);
	CUT_CHECK_ERROR("resource allocation failure");
	cudaBindTextureToArray(tex, d_array);

	cudaGraphicsUnmapResources(1, &pbores, 0);
}

void init_stats()
{
	// zero stats array
	memset(stats, 0, 0x00ffffff*sizeof(int));
	ngrains = 0;

	// and count occurences
	for (int i=0; i<w*h*d; i++)
		stats[domain[i]>>8]++;

	// create grain list
	for (int i=0; i<0x00ffffff; i++)
		if (stats[i] > 0)
			grain_list.push_back(i);
}

void update_stats()
{
	// zero stats array
	memset(stats, 0, 0x00ffffff*sizeof(int));
	ngrains = 0;

	// and count occurences
	for (int i=0; i<w*h*d; i++)
		stats[domain[i]>>8]++;

	for (list<int>::iterator it=grain_list.begin(); it!=grain_list.end();)
		if (stats[*it] == 0)
			it = grain_list.erase(it);
		else
			it++;
}

void print_stats(int step)
{
	printf("step=%d\t #grains=%d\n", step, grain_list.size());
}

void print_grains(int step)
{
	printf("%d", step);
	for (list<int>::iterator it=grain_list.begin(); it!=grain_list.end(); it++)
		printf(", %X, %d", *it, stats[*it]);

	printf("\n");
}

void init_rands(unsigned long n, int *rands)
{
	int i;
	vector<int> tmp(n);

	printf("creating random numbers\n");

	srand(time(NULL));

	for (i=0; i<n; i++)
		tmp[i] = rand() % RAND_MAX;

	printf("uploading to gpu\n");

	cudaMemcpy((void **)rands, (void **)(&tmp[0]), n*sizeof(int), cudaMemcpyHostToDevice);
	CUT_CHECK_ERROR("resource allocation failure");
}

int init_grains(const char *filename)
{
	unsigned int *data;
	int dims = 2;

	if (filename == NULL)
	{
		w = h = 256; d = 1;

		data = (unsigned int *)malloc(w*h*sizeof(unsigned int));

		for (int i=0; i<w*h; i++)
			data[i] = (rand() & 0x00ffffff) << 8;
	}
	else if (strcmp(&filename[strlen(filename)-4], ".bmp") == 0)
	{
		bmp_header head;
		bmp_info info;

		char *data24 = loadbmp(filename, &head, &info);

		w = info.width;
		h = info.height;
		d = 1;

		if (data24 == NULL || info.bpp != 24)
		{
			free(data24);
			printf("bad bitmap file\n");
			exit(1);
		}

		// convert to 32bit data
		data = (unsigned int *)malloc(w*h*sizeof(unsigned int));
		for (int i=0; i<w*h; i++)
			data[i] = (((int)data24[3*i+2] & 0x000000ff) << 24) + (((int)data24[3*i+1] & 0x000000ff) << 16) + (((int)data24[3*i+0] & 0x000000ff) << 8);
	}
	else if (strcmp(&filename[strlen(filename)-4], ".bm3") == 0)
	{
		dims = 3;

		bm3_info info;

		char *data24 = loadbm3(filename, &info);

		w = info.width;
		h = info.height;
		d = info.depth;

		if (data24 == NULL || info.bpp != 24)
		{
			free(data24);
			printf("bad bitmap file\n");
			exit(1);
		}

		// convert to 32bit data
		data = (unsigned int *)malloc(w*h*d*sizeof(unsigned int));
		for (int i=0; i<w*h*d; i++)
			data[i] = (((int)data24[3*i+0] & 0x000000ff) << 16) + (((int)data24[3*i+1] & 0x000000ff) << 8) + (((int)data24[3*i+2] & 0x000000ff) << 0) + 0xff000000;

	}

	printf("data converted\n");

	cudaGLSetGLDevice(0);
	CUT_CHECK_ERROR("device setting failure");

	// allocate opengl buffer for computing and upload data
	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
	glBufferData(GL_PIXEL_PACK_BUFFER_ARB, w*h*d*sizeof(unsigned int), data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);

	cudaGraphicsGLRegisterBuffer(&pbores, pbo, cudaGraphicsMapFlagsNone);
	CUT_CHECK_ERROR("resource registration failure");

	printf("gpu memory allocated\n");

	if (dims == 2)
	{
		width = w;
		height = h;

		// copy image data to array
		cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(32, 0, 0, 0, cudaChannelFormatKindUnsigned);
		cudaMallocArray( &d_array, &channelDesc, w, h);
		cudaMemcpyToArray(d_array, 0, 0, data, w*h*sizeof(int), cudaMemcpyHostToDevice);

		// set texture parameters
		tex.addressMode[0] = cudaAddressModeWrap;
		tex.addressMode[1] = cudaAddressModeWrap;
		tex.filterMode = cudaFilterModePoint;
		// tex coords need to be normalized in cuda 3.2 for texture wrapping to work
		tex.normalized = true;

		// Bind the array to the texture
		cudaBindTextureToArray(tex, d_array, channelDesc);

		// create 2d textures for display
		glGenTextures(1, &texid);
		glBindTexture(GL_TEXTURE_2D, texid);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (dims == 3)
	{
		width = 512;
		height = 512;

		// copy image data to array
		cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(32, 0, 0, 0, cudaChannelFormatKindUnsigned);
		cudaMalloc3DArray(&d_array, &channelDesc, make_cudaExtent(w,h,d));

		cudaMemcpy3DParms params = {0};
		params.srcPtr = make_cudaPitchedPtr(data, 4*w, w, h);
		params.extent = make_cudaExtent(w,h,d);
		params.dstArray = d_array;
		params.kind = cudaMemcpyHostToDevice;
		cudaMemcpy3D(&params);
		CUT_CHECK_ERROR("resource allocation failure");

		// set texture parameters
		tex3d.addressMode[0] = cudaAddressModeWrap;
		tex3d.addressMode[1] = cudaAddressModeWrap;
		tex3d.addressMode[2] = cudaAddressModeWrap;
		tex3d.filterMode = cudaFilterModePoint;
		// tex coords need to be normalized in cuda 3.2 for texture wrapping to work
		tex3d.normalized = true;

		// Bind the array to the texture
		cudaBindTextureToArray(tex3d, d_array, channelDesc);

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
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, w, h, d, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
	}

	free(data);

	printf("data uploaded to gpu. %d %d %d\n", w, h, d);

	// init random number generator
	cudaMalloc((void**)&rands, w*h*d*sizeof(int));
	printf("gpu mem for rands allocated\n");
	init_rands(w*h*d, rands);
	printf("rands created and transfered\n");

	// allocate download buffer
	domain = new unsigned int[w*h*d];
	stats = new unsigned int[0x00ffffff];

	printf("stats buffers allocated\n");

	// resize window
	glutReshapeWindow(width, height);

	return dims;
}

void download_grains()
{
	int *dptr;

	cudaGraphicsMapResources(1, &pbores, 0);
	size_t num_bytes; 
	cudaGraphicsResourceGetMappedPointer((void **)&dptr, &num_bytes, pbores);

	cudaMemcpy((void **)domain, (void **)dptr, num_bytes, cudaMemcpyDeviceToHost);
	
	cudaGraphicsUnmapResources(1, &pbores, 0);
}

void store_grains(const char *filename)
{
	if (filename == NULL)
		return;

	char *data24 = (char *)malloc(w*h*d*3);

	if (dims == 2)
	{
		// convert to 24bit data
		for (int i=0; i<w*h*d; i++)
		{
			data24[3*i+2] = (domain[i] >> 24) & 0xff;
			data24[3*i+1] = (domain[i] >> 16) & 0xff;
			data24[3*i+0] = (domain[i] >> 8) & 0xff;
		}		

		bmp_header head;
		bmp_info info;
		
		head.filesize = 2 + sizeof(bmp_header) + sizeof(bmp_info) + 3*w*h*d;
		head.dataoffset = 2 + sizeof(bmp_header) + sizeof(bmp_info);

		info.headersize = sizeof(bmp_info);
		info.numplanes = 1;
		info.width = w;
		info.height = h;
		info.bpp = 24;
		info.compression = 0;
		info.datasize = w*h*3;
		info.widthppm = info.heightppm = 3780;
		info.numcolors = info.impcolors = 0;

		savebmp(filename, &head, &info, data24);
	}
	else if (dims == 3)
	{
		// convert to 24bit data
		for (int i=0; i<w*h*d; i++)
		{
			data24[3*i+0] = (domain[i] >> 16) & 0xff;
			data24[3*i+1] = (domain[i] >> 8) & 0xff;
			data24[3*i+2] = (domain[i] >> 0) & 0xff;
		}		

		bm3_info info;

		info.width = w;
		info.height = h;
		info.depth = d;
		info.bpp = 24;
		info.format = 0;

		savebm3(filename, &info, data24);
	}

	free(data24);
}

void destroy_grains()
{
	cudaFree(rands);
	free(domain);

	delete[] stats;

	if (dims == 2)
		cudaUnbindTexture(tex);
	else
		cudaUnbindTexture(tex3d);

	cudaFreeArray(d_array);

	cudaGraphicsUnregisterResource(pbores);

	glDeleteBuffersARB(1, &pbo);
	glDeleteTextures(1, &texid);
}

