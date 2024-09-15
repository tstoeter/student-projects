#ifndef PARTICLE_ADVECTION_H
#define PARTICLE_ADVECTION_H

class ParticleAdvection
{
	private:
		double ***vf;
		double **particles;
		int n;
		int dimx, dimy;

	public:
		ParticleAdvection(int num, double ***f, int dx, int dy);
		~ParticleAdvection();
		void reset();
		double* getParticle(int i);
		double** getParticleArray();
		void advect0();
};

inline double* ParticleAdvection::getParticle(int i)
{
	return particles[i];
}

inline double** ParticleAdvection::getParticleArray()
{
	return particles;
}

#endif

