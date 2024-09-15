#ifndef GRAINS_H
#define GRAINS_H

#include <GL/glut.h>


extern unsigned int w, h, d;
extern float kT;
extern struct cudaGraphicsResource *pbores;
extern int *rands;

extern GLuint pbo, texid;

void run_mcs2d(int w, int h, float kT);
void run_mcs3d(int w, int h, int d, float kT);
int init_grains(const char *filename);
void store_grains(const char *filename);
void destroy_grains();
void download_grains();
void init_stats();
void update_stats();
void print_stats(int step);
void print_grains(int step);

#endif

