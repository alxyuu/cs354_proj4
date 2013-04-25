/******************************************************************/
/*         Geometry functions                                     */
/*                                                                */
/* Group Members: <FILL IN>                                       */
/******************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "common.h"
#include "raytrace.h"

point* makePoint(GLfloat x, GLfloat y, GLfloat z) {
  point* p;
  /* allocate memory */
  p = (point*) malloc(sizeof(point));
  /* put stuff in it */
  p->x = x; p->y = y; p->z = z; 
  p->w = 1.0;
  return (p);
}

/* makes copy of point (or vector) */
point* copyPoint(point *p0) {
  point* p;
  /* allocate memory */
  p = (point*) malloc(sizeof(point));

  p->x = p0->x;
  p->y = p0->y;
  p->z = p0->z;
  p->w = p0->w;         /* copies over vector or point status */
  return (p);
}

/* unallocates a point */
void freePoint(point *p) {
  if (p != NULL) {
    free(p);
  }
}

/* vector from point p to point q is returned in v */
void calculateDirection(point* p, point* q, point* v) {
  v->x = q->x - p->x;
  v->y = q->y - p->y;
  v->z = q->z - p->z;
  /* a direction is a point at infinity */
  v->w = 0.0;

  normalize(v);
  /* NOTE: v is not unit length currently, but probably should be */
}

/* given a vector, sets its contents to unit length */
void normalize(vector* v) {
	GLfloat len = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
  v->x /= len;
  v->y /= len;
  v->z /= len;
}

/* point on ray r parameterized by t is returned in p */
void findPointOnRay(ray* r,double t,point* p) {
  p->x = r->start->x + t * r->dir->x;
  p->y = r->start->y + t * r->dir->y;
  p->z = r->start->z + t * r->dir->z;
  p->w = 1.0;
}

GLfloat dotProduct(vector* a, vector* b) {
  return a->x * b->x + a->y * b->y + a->z * b->z;
}


/* SPHERES */

sphere* makeSphere(GLfloat x, GLfloat y, GLfloat z, GLfloat r) {
  sphere* s;
  /* allocate memory */
  s = (sphere*) malloc(sizeof(sphere));

  /* put stuff in it */
  s->c = makePoint(x,y,z);   /* center */
  s->r = r;   /* radius */
  s->m = NULL;   /* material */
  return(s);
}

/* returns TRUE if ray r hits sphere s, with parameter value in t */
int raySphereIntersect(ray* r,sphere* s,double* t) {
  point p;   /* start of transformed ray */
  double a,b,c;  /* coefficients of quadratic equation */
  double D;    /* discriminant */
  point* v;
  
  /* transform ray so that sphere center is at origin */
  /* don't use matrix, just translate! */
  p.x = r->start->x - s->c->x;
  p.y = r->start->y - s->c->y;
  p.z = r->start->z - s->c->z;
  v = r->dir; /* point to direction vector */


  a = v->x * v->x  +  v->y * v->y  +  v->z * v->z;
  b = 2*( v->x * p.x  +  v->y * p.y  +  v->z * p.z);
  c = p.x * p.x + p.y * p.y + p.z * p.z - s->r * s->r;

  D = b * b - 4 * a * c;
  
  if (D < 0) {  /* no intersection */
    return (FALSE);
  }
  else {
    D = sqrt(D);
    /* First check the root with the lower value of t: */
    /* this one, since D is positive */
    *t = (-b - D) / (2*a);
    /* ignore roots which are less than zero (behind viewpoint) */
    if (*t < 0) {
      *t = (-b + D) / (2*a);
    }
    if (*t < 0) { return(FALSE); }
    else return(TRUE);
  }
}

plane* makePlane(GLfloat x, GLfloat y, GLfloat z, GLfloat n_x, GLfloat n_y, GLfloat n_z, GLfloat r) {
  plane* p;

  p = (plane*)malloc(sizeof(plane));
  p->n = (vector*)makePoint(n_x,n_y,n_z);
  normalize(p->n);
  p->c = makePoint(x,y,z);
  p->r = r;
  p->m = NULL;

  return p;
}

int rayPlaneIntersect(ray* r, plane* p, double* t) {
  GLfloat dot = dotProduct(r->dir,p->n);
  if(dot == 0) {
    return(FALSE);
  } else {
    point* p1 = makePoint(p->c->x - r->start->x, p->c->y - r->start->y, p->c->z - r->start->z);
    GLfloat dot2 = dotProduct(p1,p->n);
    *t = dot2/dot;
    if(*t < 0) {
      return FALSE;
    }
    point onray;
    findPointOnRay(r,*t,&onray);
    if( !(onray.x >= p->c->x - p->r && onray.x <= p->c->x + p->r&&
          onray.y >= p->c->y - p->r && onray.y <= p->c->y + p->r && 
          onray.z >= p->c->z - p->r && onray.z <= p->c->z + p->r ) ) {
      return FALSE;
    }
   // printf("t: %f\n", *t);
    freePoint(p1);

    return TRUE;
  }

}

int rayCubeIntersect(ray *r, cube* c, double* t, vector* n) {
  double min_t = 999999999999.9;
  int hit = FALSE;
  if(rayPlaneIntersect(r,c->p1,t) && *t < min_t) {
    hit = TRUE;
    min_t = *t;
    memcpy(n,c->p1->n,sizeof(vector));
  }
  if(rayPlaneIntersect(r,c->p2,t) && *t < min_t) {
    hit = TRUE;
    min_t = *t;
    memcpy(n,c->p2->n,sizeof(vector));
  }
  if(rayPlaneIntersect(r,c->p3,t) && *t < min_t) {
    hit = TRUE;
    min_t = *t;
    memcpy(n,c->p3->n,sizeof(vector));
  }
  if(rayPlaneIntersect(r,c->p4,t) && *t < min_t) {
    hit = TRUE;
    min_t = *t;
    memcpy(n,c->p4->n,sizeof(vector));
  }
  if(rayPlaneIntersect(r,c->p5,t) && *t < min_t) {
    hit = TRUE;
    min_t = *t;
    memcpy(n,c->p5->n,sizeof(vector));
  }
  if(rayPlaneIntersect(r,c->p6,t) && *t < min_t) {
    hit = TRUE;
    min_t = *t;
    memcpy(n,c->p6->n,sizeof(vector));
  }
  *t = min_t;
  return hit;
}

cube* makeCube(GLfloat x, GLfloat y, GLfloat z, GLfloat r, material* m) {
  cube* c; 
  c = (cube*)malloc(sizeof(cube));
  c->p1 = makePlane(x-r,y,z,-1,0,0,r);
  c->p1->m = m;
  c->p2 = makePlane(x+r,y,z,1,0,0,r);
  c->p2->m = m;
  c->p3 = makePlane(x,y-r,z,0,-1,0,r);
  c->p3->m = m;
  c->p4 = makePlane(x,y+r,z,0,1,0,r);
  c->p4->m = m;
  c->p5 = makePlane(x,y,z-r,0,0,-1,r);
  c->p5->m = m;
  c->p6 = makePlane(x,y,z+r,0,0,1,r);
  c->p6->m = m;

  return c;
}
/* normal vector of s at p is returned in n */
/* note: dividing by radius normalizes */
void findSphereNormal(sphere* s, point* p, vector* n) {
  n->x = (p->x - s->c->x) / s->r;  
  n->y = (p->y - s->c->y) / s->r;
  n->z = (p->z - s->c->z) / s->r;
  n->w = 0.0;
}


