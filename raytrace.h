#ifndef _RAYTRACE_H_
#define _RAYTRACE_H_

/******************************************************************/
/*         Raytracer declarations                                 */
/******************************************************************/


/* constants */
#define TRUE 1
#define FALSE 0

#define PI 3.14159265358979323846264338327

#define EPSILON 0.0001

#define RAYDEPTH 3

/* data structures */

typedef struct point {
  GLfloat x;
  GLfloat y;
  GLfloat z;
  GLfloat w;
} point;

/* a vector is just a point */
typedef point vector;

/* a ray is a start point and a direction */
typedef struct ray {
  point* start;
  vector* dir;
} ray;

typedef struct material {
  /* color */
  GLfloat r;
  GLfloat g;
  GLfloat b;
  GLfloat a;
  /* ambient reflectivity */
  GLfloat amb;
  GLfloat spec;
  GLfloat diff;
} material;

typedef struct color {
  GLfloat r;
  GLfloat g;
  GLfloat b; 
  /* these should be between 0 and 1 */
} color;

typedef struct sphere {
  point* c;  /* center */
  GLfloat r;  /* radius */
  material* m;
} sphere;

typedef struct plane {
  vector* n; /* normal */
  point* c; /* center */
  GLfloat r; /* radius */
  material* m;
} plane;

typedef struct lighting {
  point* p; /* position */
  color* c; /* color */
  GLfloat i; /* intensity */
} lighting;

/* functions in raytrace.cpp */
void traceRay(ray*, color*, int);
void firstHit(ray*,point*,vector*,material**);

/* functions in geometry.cpp */
sphere* makeSphere(GLfloat, GLfloat, GLfloat, GLfloat);
point* makePoint(GLfloat, GLfloat, GLfloat);
point* copyPoint(point *);
GLfloat dotProduct(vector*, vector*);
void normalize(vector *);
void freePoint(point *);
void calculateDirection(point*,point*,point*);
void findPointOnRay(ray*,double,point*);
int raySphereIntersect(ray*,sphere*,double*);
void findSphereNormal(sphere*,point*,vector*);
int rayPlaneIntersect(ray*, plane*, double*);
plane* makePlane(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);


/* functions in light.cpp */
void addLight(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
material* makeMaterial(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
void shade(point*,vector*,material*,vector*,color*,int);

/* global variables */
extern int width;
extern int height;

#endif	/* _RAYTRACE_H_ */
