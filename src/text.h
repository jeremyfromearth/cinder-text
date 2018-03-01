#pragma once

// boost
#include "boost/algorithm/string.hpp"

namespace text {
struct word {
  std::string text;
  ci::Rectf bounds;
  word(std::string t, ci::Rectf b) : text(t), bounds(b) {}
  ~word() { text = ""; }
};

class renderer {
public:
  ////////////////////////////////////////////////
  // static
  ////////////////////////////////////////////////
  enum align { Left, Right };
  static const std::string default_charset;
  
  typedef std::shared_ptr<renderer> renderer_ref;
  
  static renderer_ref create();
  
  static renderer_ref create(std::string str, ci::Font font);
  
  ////////////////////////////////////////////////
  // ctr/dstr
  ////////////////////////////////////////////////
  renderer();
  
  ~renderer() { clear(); }
  
  ////////////////////////////////////////////////
  // getter/setter
  ////////////////////////////////////////////////
  align get_alignment() { return alignment; }
  void set_alignment(align a) {
    alignment = a;
    invalidated = true;
  }
  
  ci::Rectf get_bounds() { return bounds; }
  
  void set_color(ci::Color c) { color = ci::ColorA(c, 1.0); }
  
  void set_color(ci::ColorA c) { color = c; }
  
  void set_font(std::string path, int font_size, std::string charset=renderer::default_charset);
  
  void set_font(ci::Font f, std::string charset=renderer::default_charset);
  
  void set_leading(int l) {
    leading = l;
    invalidated = true;
  }
  
  int get_max_width() { return max_width; }
  void set_max_width(int w) {
    max_width = w;
    invalidated = true;
  }
  
  void set_word_spacing(int n) {
    word_spacing = n;
    invalidated = true;
  }
  
  void set_text(std::string s) {
    clear();
    str = s;
  }
  
  ////////////////////////////////////////////////
  // methods
  ////////////////////////////////////////////////
  
  void clear();
  
  void draw();
  
  void layout();

protected:
  
  std::string str;
  std::vector<word> words;
  
  int leading;
  int max_width;
  int word_spacing;
  bool invalidated;
  
  ci::ColorA color;
  
  ci::Rectf bounds;
  ci::gl::TextureFontRef font;
  
  align alignment;
  ci::gl::TextureFont::DrawOptions options;
};
typedef renderer::renderer_ref renderer_ref;
}

