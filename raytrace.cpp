/******************************************************************/
/*         Main raytracer file                                    */
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
#include "lowlevel.h"
#include "raytrace.h"

/* local functions */
void initScene(void);
void initCamera (int, int);
void display(void);
void init(int, int);
void traceRay(ray*,color*);
void drawScene(void);

/* local data */

/* the scene: so far, just one sphere */
sphere* s1;
sphere* s2;
sphere* s3;
plane* p1;

/* the viewing parameters: */
point* viewpoint;
GLfloat pnear;  /* distance from viewpoint to image plane */
GLfloat fovx;  /* x-angle of view frustum */
int width = 500;     /* width of window in pixels */
int height = 350;    /* height of window in pixels */

int main (int argc, char** argv) {
  int win;

  glutInit(&argc,argv);
  glutInitWindowSize(width,height);
  glutInitWindowPosition(100,100);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  win = glutCreateWindow("raytrace");
  glutSetWindow(win);
  init(width,height);
  glutDisplayFunc(display);
  glutMainLoop();
  return 0;
}

void init(int w, int h) {

  /* OpenGL setup */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
  glClearColor(0.0, 0.0, 0.0, 0.0);  

  /* low-level graphics setup */
  initCanvas(w,h);

  /* raytracer setup */
  initCamera(w,h);
  initScene();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  drawScene();  /* draws the picture in the canvas */
  flushCanvas();  /* draw the canvas to the OpenGL window */
  glFlush();
}

void initScene () {
  addLight( 4.0, 4.0, 4.0,
            1.0, 1.0, 1.0,
            1.0 );
  addLight( -4.0, -4.0, 4.0,
             0.1,  0.1, 0.1, 
             1.0 );
  addLight( 1.0, -1.0, 6.0,
            0.3, 0.3, 0.3,
            1.0 );
  s1 = makeSphere(0.0,0.0,-2.5,0.3);
  s1->m = makeMaterial(1.0,0.1,0.15,0.5,0.3,0.7,0.3);
  s2 = makeSphere(0.4,0.2,-3.0,0.25);
  s2->m = makeMaterial(0.1,1.0,0.15,0.5,0.3,0.3,0.7);
  s3 = makeSphere(0.2,-0.2,-1.5,0.1);
  s3->m = makeMaterial(0.15,0.1,1.0,0.5,0.7,0.3,0.3);
  p1 = makePlane(0,0,-4,-3.5,2,3,5);
  p1->m = makeMaterial(0.1,0.1,0.1,0.0,0.5,0.6,0.4);
}

void initCamera (int w, int h) {
  viewpoint = makePoint(0.0,0.0,0.0);
  pnear = 1.0;
  fovx = PI/6;
}

void drawScene () {
  int i,j;
  GLfloat imageWidth;
  /* declare data structures on stack to avoid dynamic allocation */
  point worldPix;  /* current pixel in world coordinates */
  point direction; 
  ray r;
  color c;

  /* initialize */
  worldPix.w = 1.0;
  worldPix.z = -pnear;

  r.start = &worldPix;
  r.dir= &direction;

  imageWidth = 2*pnear*tan(fovx/2);

  /* trace a ray for every pixel */
  for (i=0; i<width; i++) {
    /* Refresh the display */
    /* Comment this line out after debugging */
    flushCanvas();

    for (j=0; j<height; j++) {

      /* find position of pixel in world coordinates */
      /* y position = (pixel height/middle) scaled to world coords */ 
      worldPix.y = (j-(height/2))*imageWidth/width;
      /* x position = (pixel width/middle) scaled to world coords */ 
      worldPix.x = (i-(width/2))*imageWidth/width;

      /* find direction */
      /* note: direction vector is NOT NORMALIZED */
      calculateDirection(viewpoint,&worldPix,&direction);

      /* trace the ray! */
      traceRay(&r,&c,RAYDEPTH);
      /* write the pixel! */
      drawPixel(i,j,c.r,c.g,c.b);
    }
  }
}

/* returns the color seen by ray r in parameter c */
/* d is the recursive depth */
void traceRay(ray* r, color* c, int d) {
  point p;  /* first intersection point */
  vector n;
  material* m;

  p.w = 0.0;  /* inialize to "no intersection" */
  firstHit(r,&p,&n,&m);

  if (p.w != 0.0) {
    shade(&p,&n,m,r->dir,c,d);  /* do the lighting calculations */
  } else {             /* nothing was hit */
    c->r = 0.0;
    c->g = 0.0;
    c->b = 0.0;
  }
}

/* firstHit */
/* If something is hit, returns the finite intersection point p, 
   the normal vector n to the surface at that point, and the surface
   material m. If no hit, returns an infinite point (p->w = 0.0) */
void firstHit(ray* r, point* p, vector* n, material* *m) {
  double t1 = 0;
  double t2 = 0;
  double t3 = 0;
  double t4 = 0;
  int hit1 = FALSE;
  int hit2 = FALSE;
  int hit3 = FALSE;
  int hit4 = FALSE;
  
  ray *r2 = (ray*) malloc(sizeof(ray));
  r2->start = copyPoint(r->start);
  r2->dir = (vector*)copyPoint(r->dir);
  r2->start->x += EPSILON * r2->dir->x;
  r2->start->y += EPSILON * r2->dir->y;
  r2->start->z += EPSILON * r2->dir->z;

  hit1 = raySphereIntersect(r2,s1,&t1) /*&& t1 > EPSILON*/;
  hit2 = raySphereIntersect(r2,s2,&t2) /*&& t2 > EPSILON*/;
  hit3 = raySphereIntersect(r2,s3,&t3) /*&& t3 > EPSILON*/;
  hit4 = rayPlaneIntersect(r2,p1,&t4);

  if( hit1 && (!hit2 || t1 < t2) && (!hit3 || t1 < t3) && (!hit4 || t1 < t4) ) {
    *m = s1->m;
    findPointOnRay(r2,t1,p);
    findSphereNormal(s1,p,n);
  } else if( hit2 && (!hit1 || t2 < t1) && (!hit3 || t2 < t3) && (!hit4 || t2 < t4) ) {
    *m = s2->m;
    findPointOnRay(r2,t2,p);
    findSphereNormal(s2,p,n);
  } else if( hit3 && (!hit2 || t3 < t2) && (!hit1 || t3 < t1) && (!hit4 || t3 < t4) ) {
    *m = s3->m;
    findPointOnRay(r2,t3,p);
    findSphereNormal(s3,p,n);
  } else if( hit4 && (!hit2 || t4 < t2) && (!hit1 || t4 < t1) && (!hit3 || t4 < t3) ) {
    *m = p1->m;
    findPointOnRay(r2,t4,p);
    memcpy(n,p1->n,sizeof(vector));
  } else {
    /* indicates no hit */
    p->w = 0.0;
  }
  freePoint(r2->start);
  freePoint(r2->dir);
  if(r2 != NULL) {
    free(r2);
  }

}

