#ifndef VISUAL_H
#define VISUAL_H

// glut parameters
extern int width, height;		// window size


extern int dims, visual;
extern int stat, verb, dump, term;

void run_cuggs(int argc, char *argv[], const char *loadfile, const char *savefile);

#endif

