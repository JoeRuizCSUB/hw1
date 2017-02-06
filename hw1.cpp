//modified by:	Joe Ruiz
//date:			January 26, 2017
//purpose:		Demonstrates use of git and repository.
//
//cs3350 Spring 2017 Lab-1
//author: Gordon Griesel
//date: 2014 to present
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <math.h>
#include "fonts.h"
#define _USE_MATH_DEFINES
#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 360

#define MAX_PARTICLES 100000
#define GRAVITY 0.8 
#define rnd() (float)rand() / (float)RAND_MAX
//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	 float x, y, z;
};

struct Shape {
	 float width, height;
	 float radius;
	 Vec center;
};

struct Particle {
	 Shape s;
	 Vec velocity;
};

struct Game {
	 Shape box[6];
	 Particle particle[MAX_PARTICLES];
	 int n;
	 int bubbler;
	 int mouse[2];
	 Game() { n=0; bubbler=0;}
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
	 int done=0;
	 srand(time(NULL));
	 initXWindows();
	 init_opengl();
	 //declare game object
	 Game game;
	 game.n=0;

	 //declare a box shape
	 game.box[0].width = 75;
	 game.box[0].height = 13;
	 game.box[0].center.x = 350;
	 game.box[0].center.y = 100;

	 game.box[1].width = 75;
	 game.box[1].height = 13;
	 game.box[1].center.x = 275;
	 game.box[1].center.y = 150;

	 game.box[2].width = 75;
	 game.box[2].height = 13;
	 game.box[2].center.x = 225;
	 game.box[2].center.y = 200;

	 game.box[3].width = 75;
	 game.box[3].height = 13;
	 game.box[3].center.x = 175;
	 game.box[3].center.y = 250;

	 game.box[4].width = 75;
	 game.box[4].height = 13;
	 game.box[4].center.x = 125;
	 game.box[4].center.y = 300;

	 //declare a hill on bottom right
	 game.box[5].center.x = 480;
	 game.box[5].center.y = -70;
	 game.box[5].radius = 125;

	 //start animation
	 while (!done) {
		  while (XPending(dpy)) {
				XEvent e;
				XNextEvent(dpy, &e);
				check_mouse(&e, &game);
				done = check_keys(&e, &game);
		  }
		  movement(&game);
		  render(&game);
		  glXSwapBuffers(dpy, win);
	 }
	 cleanupXWindows();
	 cleanup_fonts();
	 return 0;
}

void set_title(void)
{
	 //Set the window title bar.
	 XMapWindow(dpy, win);
	 XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void)
{
	 //do not change
	 XDestroyWindow(dpy, win);
	 XCloseDisplay(dpy);
}

void initXWindows(void)
{
	 //do not change
	 GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	 int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	 dpy = XOpenDisplay(NULL);
	 if (dpy == NULL) {
		  std::cout << "\n\tcannot connect to X server\n" << std::endl;
		  exit(EXIT_FAILURE);
	 }
	 Window root = DefaultRootWindow(dpy);
	 XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	 if (vi == NULL) {
		  std::cout << "\n\tno appropriate visual found\n" << std::endl;
		  exit(EXIT_FAILURE);
	 } 
	 Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	 XSetWindowAttributes swa;
	 swa.colormap = cmap;
	 swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		  ButtonPress | ButtonReleaseMask | PointerMotionMask |
		  StructureNotifyMask | SubstructureNotifyMask;
	 win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
				InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	 set_title();
	 glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	 glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	 //OpenGL initialization
	 glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	 //Initialize matrices
	 glMatrixMode(GL_PROJECTION); glLoadIdentity();
	 glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	 //Set 2D mode (no perspective)
	 glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	 //Set the screen background color
	 glClearColor(0.1, 0.1, 0.1, 1.0);
	 glEnable(GL_TEXTURE_2D);
	 initialize_fonts();
}

void makeParticle(Game *game, int x, int y)
{
	 if (game->n >= MAX_PARTICLES)
		  return;
	 //position of particle
	 Particle *p = &game->particle[game->n];
	 p->s.center.x = x;
	 p->s.center.y = y;
	 p->velocity.y = rnd() * 0.5;
	 p->velocity.x = rnd() * 1.0 - 0.2;
	 game->n++;
}


void check_mouse(XEvent *e, Game *game)
{
	 static int savex = 0;
	 static int savey = 0;

	 if (e->type == ButtonRelease) {
		  return;
	 }
	 if (e->type == ButtonPress) {
		  if (e->xbutton.button==1) {
				//Left button was pressed
				int y = WINDOW_HEIGHT - e->xbutton.y;
				makeParticle(game, e->xbutton.x, y);
				return;
		  }
		  if (e->xbutton.button==3) {
				//Right button was pressed
				return;
		  }
	 }
	 //Did the mouse move?
	 if (savex != e->xbutton.x || savey != e->xbutton.y) {
		  savex = e->xbutton.x;
		  savey = e->xbutton.y;
		  int y = WINDOW_HEIGHT - e->xbutton.y;
		  game->mouse[0] = savex;
		  game->mouse[1] = y;
	 }

}


int check_keys(XEvent *e, Game *game)
{
	 //Was there input from the keyboard?
	 if (e->type == KeyPress) {
		  int key = XLookupKeysym(&e->xkey, 0);
		  if (key == XK_Escape) {
				return 1;
		  }
		  //You may check other keys here.
		  if (key == XK_b) {
				game->bubbler ^= 1;
		  }

	 }
	 return 0;
}

void movement(Game *game)
{
	 Particle *p;
	 if (game->n <= 0)
		  return;
	 if (game->bubbler != 0) {
		  //the bubbler is on!
		  for (int i=0; i<100; i++) 
				makeParticle(game, 125, 350);
	 }

	 for (int i=0; i<game->n; i++) {
		  p = &game->particle[i];
		  p->velocity.y -= GRAVITY;
		  p->s.center.x += p->velocity.x;
		  p->s.center.y += p->velocity.y;

		  //check for collision with shapes...
		  Shape *s;
		  for (int j=0; j<5; j++) {
				s = &game->box[j];
				if (p->s.center.y < s->center.y + s->height &&
						p->s.center.y > s->center.y - s->height &&
						  p->s.center.x >= s->center.x - s->width &&
						  p->s.center.x <= s->center.x + s->width) {
					 p->s.center.y = s->center.y + s->height;
					 p->velocity.y = -p->velocity.y * rnd() * 0.04f/1.5;
					 p->velocity.x += rnd()/10.0f;
				}
				//check for off-screen
				if (p->s.center.y < 0.0) {
					 game->particle[i] = game->particle[--game->n];
				}
		  }
		  s = &game->box[5];
		  float dist0 = p->s.center.x - s->center.x;
		  float dist1 = p->s.center.y - s->center.y;
		  dist0 = dist0 * dist0;
		  dist1 = dist1 * dist1;
		  float dist = sqrt(dist0 + dist1); 

			//check for collision with hill
		  if (dist < s->radius) {
				p->velocity.x = -s->center.x/dist;
				p->velocity.y = s->center.y/dist;
				p->velocity.x = p->velocity.x;
				p->velocity.y = p->velocity.y/0.8;		
		  }

	 }

}

void render(Game *game)
{
	 Rect r;
	 glClear(GL_COLOR_BUFFER_BIT);
	 r.bot = WINDOW_HEIGHT - 20;
	 r.left = 10;
	 r.center = 0;
	 ggprint8b(&r, 16, 0xffffff, "Waterfall model");
	 glPopAttrib();


	 float w, h;
	 //Draw shapes...

	 //draw boxes
	 Shape *s;
	 glColor3ub(90,140,90);
	 for (int i=0; i<5; i++) {
		  s = &game->box[i];
		  glPushMatrix();
		  glTranslatef(s->center.x, s->center.y, s->center.z);
		  w = s->width;
		  h = s->height;
		  glBegin(GL_QUADS);
		  glVertex2i(-w,-h);
		  glVertex2i(-w, h);
		  glVertex2i( w, h);
		  glVertex2i( w,-h);
		  glEnd();
		  glPopMatrix();
	 }

	 r.bot = game->box[0].center.y - game->box[0].height/2;
	 r.left = game->box[0].center.x - game->box[0].width/2;
	 r.center = 0;
	 ggprint8b(&r, 16, 0xffffff, "Maintenance");
	 glPopAttrib();

	 r.bot = game->box[1].center.y - game->box[1].height/2;
	 r.left = game->box[1].center.x - 20;
	 r.center = 0;
	 ggprint8b(&r, 16, 0xffffff, "Testing");
	 glPopAttrib();

	 r.bot = game->box[2].center.y - game->box[2].height/2;
	 r.left = game->box[2].center.x - 20;
	 r.center = 0;
	 ggprint8b(&r, 16, 0xffffff, "Coding");
	 glPopAttrib();

	 r.bot = game->box[3].center.y - game->box[3].height/2;
	 r.left = game->box[3].center.x - 15;
	 r.center = 0;
	 ggprint8b(&r, 16, 0xffffff, "Design");
	 glPopAttrib();

	 r.bot = game->box[4].center.y - game->box[4].height/2;
	 r.left = game->box[4].center.x - game->box[4].width/2;
	 r.center = 0;
	 ggprint8b(&r, 16, 0xffffff, "Requirements");
	 glPopAttrib();


	 //draw hill
	 s = &game->box[5];
	 glColor3ub(51,102,0);
	 glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	 glBegin(GL_TRIANGLE_FAN);
	 for (int i=0; i< 180; i++) {
		  int x = s->radius * cos(i * 2 * M_PI / 180);
		  int y = s->radius * sin(i * 2 * M_PI / 180);
		  glVertex3f(x, y, 0);
	 }
	 glEnd();
	 glPopMatrix();

	 //draw all particles here
	 for (int i=0; i<game->n; i++) {
		  glPushMatrix();
		  int blue = rand() % 2;
		  if (blue == 0)
				glColor3ub(51, 153, 220);
		  else
				glColor3ub(153, 204, 255);

		  Vec *c = &game->particle[i].s.center;
		  w = 4;
		  h = 4;
		  glBegin(GL_QUADS);
		  glVertex2i(c->x-w, c->y-h);
		  glVertex2i(c->x-w, c->y+h);
		  glVertex2i(c->x+w, c->y+h);
		  glVertex2i(c->x+w, c->y-h);
		  glEnd();
		  glPopMatrix();
	 }

}



