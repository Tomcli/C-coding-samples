// $Id: interp.cpp,v 1.2 2015-07-16 16:57:30-07 - - $

#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "debug.h"
#include "interp.h"
#include "shape.h"
#include "util.h"

static unordered_map<void*, string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

static unordered_map<string, void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

unordered_map<string, interpreter::interpreterfn>
interpreter::interp_map {
   {"define" , &interpreter::do_define },
   {"draw"   , &interpreter::do_draw   },
   {"border" , &interpreter::do_border },
   {"moveby" , &interpreter::do_moveby },
};

unordered_map<string, interpreter::factoryfn>
interpreter::factory_map {
   {"text"     , &interpreter::make_text     },
   {"ellipse"  , &interpreter::make_ellipse  },
   {"circle"   , &interpreter::make_circle   },
   {"polygon"  , &interpreter::make_polygon  },
   {"rectangle", &interpreter::make_rectangle},
   {"square"   , &interpreter::make_square   },
   {"diamond"  , &interpreter::make_diamond  },
   {"triangle" , &interpreter::make_triangle },
   {"equilateral", &interpreter::make_equilateral}
};

interpreter::shape_map interpreter::objmap;

interpreter::~interpreter() {
   for (const auto& itor : objmap) {
      cout << "objmap[" << itor.first << "] = "
           << *itor.second << endl;
   }
}

void interpreter::interpret (const parameters& params) {
   DEBUGF ('i', params);
   param begin = params.cbegin();
   string command = *begin;
   auto itor = interp_map.find (command);
   if (itor == interp_map.end()) throw runtime_error ("syntax error");
   interpreterfn func = itor->second;
   func (++begin, params.cend());
}

void interpreter::do_define (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string name = *begin;
   objmap.emplace (name, make_shape (++begin, end));
}


void interpreter::do_draw (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 4) throw runtime_error ("syntax error");
   string name = begin[1];
   shape_map::const_iterator itor = objmap.find (name);
   if (itor == objmap.end()) {
      throw runtime_error (name + ": no such shape");
   }
   rgbcolor color {begin[0]};
   vertex where {from_string<GLfloat> (begin[2]),
                 from_string<GLfloat> (begin[3])};
   //push back the draw instruction until glutInit
   //is called.
   object obj(itor->second, where, color);
   window::push_back(obj);
}

void interpreter::do_border (param begin, param end) {
   if ((end - begin) != 2) throw runtime_error("syntax error");
   //set border
   rgbcolor color{*begin++};
   window::setborder(color);
   window::setthickness(int(stod(*begin)));
}

void interpreter::do_moveby (param begin, param end) {
   if ((end - begin) != 1) throw runtime_error("syntax error");
   //set move_by space
   window::setmove_by(int(stod(*begin)));
}

shape_ptr interpreter::make_shape (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string type = *begin++;
   auto itor = factory_map.find(type);
   if (itor == factory_map.end()) {
      throw runtime_error (type + ": no such shape");
   }
   factoryfn func = itor->second;
   return func (begin, end);
}

shape_ptr interpreter::make_text (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   auto itor = fontcode.find (*begin);
   if (itor == fontcode.end()) throw runtime_error ("font error");
   string textData = "";
   for (auto i = begin + 1; i != end; ++i) {
      textData += *i + " ";
   }
   return make_shared<text> (itor->second, textData);
}

shape_ptr interpreter::make_ellipse (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if ((end - begin) != 2) throw runtime_error("error: ellipse");
   GLfloat h = stod(*begin);
   GLfloat w = stod(*(++begin));
   return make_shared<ellipse> (h, w);
}

shape_ptr interpreter::make_circle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if ((end - begin) != 1) throw runtime_error("error: circle");
   return make_shared<circle> (GLfloat(stod(*begin)));
}

shape_ptr interpreter::make_polygon (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (((end - begin) % 2) != 0) throw runtime_error ("error: polygon");
   if ((end - begin) < 6) throw runtime_error ("error: polygon not enough vertices");
   vector<vertex> vertices;
   float x = 0;
   float y = 0;
   int points = 0;
   //store all the vertices and sum up the x,y coordinate
   for (auto i = begin; i != end; i++) {
      vertex v{GLfloat(stod(*i++)), GLfloat(stod(*i))};
      x += v.xpos;
      y += v.ypos;
      vertices.push_back(v);
      points++;
   }
   //get the average of x,y coordinate and normallize it
   x /= points;
   y /= points;
   for (auto i : vertices) {
      i.xpos -= x;
      i.ypos -= y;
   }
   return make_shared<polygon> (vertex_list(vertices));
}

shape_ptr interpreter::make_rectangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if ((end - begin) != 2) throw runtime_error("error: rectangle");
   GLfloat h = stod(*begin);
   GLfloat w = stod(*(++begin));
   return make_shared<rectangle> (h, w);
}

shape_ptr interpreter::make_square (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if ((end - begin) != 1) throw runtime_error("error: square");
   return make_shared<square> (GLfloat(stod(*begin)));
}

shape_ptr interpreter::make_diamond (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if ((end - begin) != 2) throw runtime_error("error: diamond");
   GLfloat h = stod(*begin);
   GLfloat w = stod(*(++begin));
   return make_shared<diamond> (h, w);
}

shape_ptr interpreter::make_triangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if ((end - begin) != 6) throw runtime_error("error: triangle");
   vertex x{GLfloat(stod(*begin)), GLfloat(stod(*(++begin)))};
   vertex y{GLfloat(stod(*(++begin))), GLfloat(stod(*(++begin)))};
   vertex z{GLfloat(stod(*(++begin))), GLfloat(stod(*(++begin)))};
   return make_shared<triangle>(x, y, z);
}

shape_ptr interpreter::make_equilateral (param begin, param end) {
   if ((end - begin) != 1) throw runtime_error("error: equilateral");
   return make_shared<equilateral> (GLfloat(stod(*begin)));
}


