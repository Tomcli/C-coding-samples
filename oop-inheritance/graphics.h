// $Id: graphics.h,v 1.1 2015-07-16 16:47:51-07 - - $

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <memory>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "rgbcolor.h"
#include "shape.h"

class object {
private:
   shared_ptr<shape> pshape;
   vertex center;
   rgbcolor color;
public:
   //Define object so we can push it to the vector and draw it
   //after glutInit.
   object(const shared_ptr<shape> &shape_ptr, const vertex &c,
          const rgbcolor &colour) {
      pshape = shape_ptr;
      center = c;
      color = colour;
   }
   void drawborder(rgbcolor colour){pshape->draw (center, colour);}
   void draw() { pshape->draw (center, color); }
   void move (GLfloat delta_x, GLfloat delta_y) {
      center.xpos += delta_x;
      center.ypos += delta_y;
   }
};

class mouse {
   friend class window;
private:
   int xpos {0};
   int ypos {0};
   int entered {GLUT_LEFT};
   int left_state {GLUT_UP};
   int middle_state {GLUT_UP};
   int right_state {GLUT_UP};
private:
   void set (int x, int y) { xpos = x; ypos = y; }
   void state (int button, int state);
   void draw();
};


class window {
   friend class mouse;
private:
   static int width;         // in pixels
   static int height;        // in pixels
   static vector<object> objects;
   static size_t selected_obj;
   static mouse mus;
   static int move_by;
   static int thickness;
   static rgbcolor border;
private:
   static void close();
   static void entry (int mouse_entered);
   static void display();
   static void reshape (int width, int height);
   static void keyboard (GLubyte key, int, int);
   static void special (int key, int, int);
   static void motion (int x, int y);
   static void passivemotion (int x, int y);
   static void mousefn (int button, int state, int x, int y);
   static void select_object_ (size_t obj);
   static void move_selected_object (float x, float y);
public:
   static void push_back (const object& obj) {
      objects.push_back (obj);
   }
   static void setwidth (int width_) { width = width_; }
   static void setheight (int height_) { height = height_; }
   static void setmove_by(int move_by_) { move_by = move_by_; }
   static void setthickness(int thickness_) {thickness = thickness_; }
   static void setborder(rgbcolor color_) {border = color_; }
   static void main();
};

#endif

