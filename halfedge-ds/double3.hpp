#ifndef double3_H
#define double3_H

#include <cmath>

struct double3
{
	// data
	double x, y, z;

	// constructors
	double3(double x1 = 0.0, double x2 = 0.0, double x3 = 0.0) : x(x1), y(x2), z(x3) {}
	double3(const double x[3]) : x(x[0]), y(x[1]), z(x[2]) {}
	double3(double3 u, double3 v) : x(v.x-u.x), y(v.y-u.y), z(v.z-u.z) {}

	// functions and operators
	inline double	norm()				{ return sqrt(x*x + y*y + z*z); }
	inline double	norm2()				{ return x*x + y*y + z*z; }
//	inline double3	normalize()			{ return 1.0f/norm() * *this; }

	inline double 	operator~() 			{ return norm(); }

	inline double3 	operator+=(double3 v) 		{ x+=v.x; y+=v.y; z+=v.z; return *this; }
	inline double3	operator-=(double3 v) 		{ x-=v.x; y-=v.y; z-=v.z; return *this; }

	template <typename T>
	inline double3 	operator*=(T s) 		{ x*=s; y*=s; z*=s; return *this; }

	inline bool	operator==(double3 v)		{ return x==v.x && y==v.y && z==v.z; }

	inline double&	operator()(unsigned i)		{ return *((double*)this+i-1); }
};

static inline double3 	operator+(double3 u, double3 v)		{ return double3(u.x+v.x, u.y+v.y, u.z+v.z); }
static inline double3 	operator-(double3 u, double3 v)		{ return double3(u.x-v.x, u.y-v.y, u.z-v.z); }
static inline double 	operator*(double3 u, double3 v)		{ return u.x*v.x + u.y*v.y + u.z*v.z; }
static inline double3 	operator^(double3 u, double3 v)		{ return double3(u.y*v.z-v.y*u.z, v.x*u.z-u.x*v.z, u.x*v.y-v.x*u.y); }

template <typename T>
static inline double3 	operator*(T s, double3 u) 	{ return double3(s*u.x, s*u.y, s*u.z); }

template <typename T>
static inline double3 	operator*(double3 u, T s) 	{ return double3(s*u.x, s*u.y, s*u.z); }

static inline double 	dot(double3 u, double3 v) 		{ return u*v; }
static inline double3 	cross(double3 u, double3 v)		{ return u^v; }
static	inline double3	normalize(double3 u)			{ return u * (1.0/u.norm()); }

const double3 zero = double3();

#endif

