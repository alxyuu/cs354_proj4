/******************************************************************/
/*         Lighting functions                                     */
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
#include <vector>

std::vector<lighting*> lights;

void addLight(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat i) {
  lighting* l;
  l = (lighting*) malloc(sizeof(lighting));
  l->p = (point*) malloc(sizeof(point));
  l->c = (color*) malloc(sizeof(color));
  l->p->x = x;
  l->p->y = y;
  l->p->z = z;
  l->c->r = r;
  l->c->g = g;
  l->c->b = b;
  lights.push_back(l);
}

material* makeMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat amb, GLfloat spec, GLfloat diff) {
  material* m;
  
  /* allocate memory */
  m = (material*) malloc(sizeof(material));
  /* put stuff in it */
  m->r = r;
  m->g = g;
  m->b = b;
  m->a = a;
  m->amb = amb;
  m->spec = spec;
  m->diff = diff;
  return(m);
}

/* LIGHTING CALCULATIONS */

/* shade */
/* color of point p with normal vector n and material m returned in c */
/* in is the direction of the incoming ray and d is the recusive depth */
void shade(point* p, vector* n, material* m, vector* in, color* c, int d) {

  /* so far, just finds ambient component of color */
  c->r = m->amb * m->r * (1-m->a);
  c->g = m->amb * m->g * (1-m->a);
  c->b = m->amb * m->b * (1-m->a);

  for(unsigned i = 0; i < lights.size(); i++) {
    lighting* light = lights[i];
    vector* li = (vector*)makePoint(light->p->x - p->x, light->p->y - p->y, light->p->z - p->z);
    normalize(li);
    GLfloat dot = dotProduct(li,n);
    vector* rm = (vector*)makePoint(2*dot*n->x - li->x, 2*dot*n->y - li->y, 2*dot*n->z - li->z);
    vector* v = (vector*)makePoint(-p->x, -p->y, -p->z);
    normalize(v);
    GLfloat specDot = dotProduct(rm, v);
    

    point n_p;  /* next intersection point */
    vector n_n;
    material* n_m;
    ray r;
    r.start = p;
    r.dir = li;
    n_p.w = 0.0;  /* inialize to "no intersection" */
    firstHit(&r,&n_p,&n_n,&n_m);

    if(n_p.w == 0.0) {
      if(dot > 0) {
        c->r += m->diff * light->c->r * dot;
        c->g += m->diff * light->c->g * dot;
        c->b += m->diff * light->c->b * dot;
      }
      if(specDot > 0) {
        specDot = pow(specDot, 16);
        c->r += m->spec * light->c->r * specDot;
        c->g += m->spec * light->c->g * specDot;
        c->b += m->spec * light->c->b * specDot;
      }
    }
    freePoint(li);
    freePoint(rm);
    freePoint(v);
  }

  if(d > 0) {
    color n_c;
    vector* in_r = (vector*)makePoint(-in->x, -in->y, -in->z);
    normalize(in_r);
    GLfloat dot = dotProduct(n,in_r);
    vector* ref = (vector*)makePoint(2*dot*n->x - in_r->x, 2*dot*n->y - in_r->y, 2*dot*n->z - in_r->z);

    point n_p;  /* next intersection point */
    vector n_n;
    material* n_m;
    ray r;
    r.start = p;
    r.dir = ref;
    n_p.w = 0.0;  /* inialize to "no intersection" */
    firstHit(&r,&n_p,&n_n,&n_m);

    //reflection
    if (n_p.w != 0.0) {
      shade(&n_p,&n_n,n_m,ref,&n_c,d-1);
      c->r += (d/(double)RAYDEPTH) * m->spec * n_c.r;
      c->g += (d/(double)RAYDEPTH) * m->spec * n_c.g;
      c->b += (d/(double)RAYDEPTH) * m->spec * n_c.b;
    }

    //refraction
    n_p.w = 0.0;
    r.dir = in;
    firstHit(&r, &n_p, &n_n, &n_m);

    if (n_p.w != 0.0) {
      shade(&n_p,&n_n,n_m,in,&n_c,d-1); 
      c->r += (d/(double)RAYDEPTH) * m->a * n_c.r;
      c->g += (d/(double)RAYDEPTH) * m->a * n_c.g;
      c->b += (d/(double)RAYDEPTH) * m->a * n_c.b;
    }

    freePoint(in_r);
    freePoint(ref);
  }

  /* clamp color values to 1.0 */
  if (c->r > 1.0) c->r = 1.0;
  if (c->g > 1.0) c->g = 1.0;
  if (c->b > 1.0) c->b = 1.0;

}

