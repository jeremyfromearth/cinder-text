//
//  text.cpp
//  Basic
//
//  Created by jeremy on 2/28/18
//

// std
#include <stdio.h>

// cinder
#include "cinder/Log.h"

// text
#include "text.h"

using namespace text;

const std::string renderer::default_charset =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ \
  abcdefghijklmnopqrstuvwxyz0123456789 \
  !$%&()-+=;:'\"[],./?–—”“…↑↓’";

std::map<std::pair<std::string, int>, ci::gl::TextureFontRef> renderer::font_cache;

renderer_ref renderer::create() {
  return std::make_shared<renderer>();
}

renderer::renderer() {
  debug = false;
  leading = 0;
  max_width = 512;
  word_spacing = 8;
  paragraph_spacing = 16;
  alignment = Left;
  invalidated = true;
  color = ci::Color::black();
  options.clipVertical(false).clipHorizontal(false).pixelSnap(true);
}

void renderer::append(std::string s) {
  std::vector<std::string> parts;
  boost::split(parts, s, boost::is_any_of("\n\r"));
  blocks.insert(blocks.end(), parts.begin(), parts.end());
}

void renderer::set_font(std::string path, int font_size, std::string charset) {
  auto p = std::make_pair(path, font_size);
  if(renderer::font_cache.count(p) != 0) {
    font = renderer::font_cache.at(p);
  } else {
    ci::Font f = ci::Font(ci::app::loadAsset(path), font_size);
    font = ci::gl::TextureFont::create(f, ci::gl::TextureFont::Format().enableMipmapping(true), charset);
    renderer::font_cache.emplace(std::make_pair(p, font));
  }
  
  invalidated = true;
}

void renderer::set_style(const ci::JsonTree & style) {
  invalidated = true;
  
  if(style.hasChild("font") && style.hasChild("size")) {
    set_font(style.getChild("font").getValue(), style.getChild("size").getValue<int>());
  }
  
  if(style.hasChild("color")) {
    ci::JsonTree c = style.getChild("color");
    if(c.getNumChildren() >= 3) {
      float r = c.getValueAtIndex<float>(0) / 255.0f;
      float g = c.getValueAtIndex<float>(1) / 255.0f;
      float b = c.getValueAtIndex<float>(2) / 255.0f;
      if(c.getNumChildren() == 4) {
        float a = c.getValueAtIndex<float>(3);
        set_color(ci::ColorA(r, g, b, a));
      } else {
        set_color(ci::Color(r, g, b));
      }
    }
  }
  
  if(style.hasChild("leading")) {
    set_leading(style.getChild("leading").getValue<int>());
  }
  
  if(style.hasChild("word-spacing")) {
    set_word_spacing(style.getChild("word-spacing").getValue<int>());
  }
  
  if(style.hasChild("max-width")) {
    set_max_width(style.getChild("max-width").getValue<int>());
  }
  
  if(style.hasChild("align")) {
    std::string a = style.getChild("align").getValue();
    if(a == "left") alignment = renderer::align::Left;
    if(a == "right") alignment = renderer::align::Right;
  }
  
  if(style.hasChild("paragraph-spacing")) {
    set_paragraph_spacing(style.getValueForKey<int>("paragraph-spacing"));
  }
}

void renderer::clear() {
  words.clear();
  blocks.clear();
  invalidated = true;
}

void renderer::draw() {
  layout();
  ci::gl::ScopedColor c(color);
  ci::gl::ScopedBlendPremult pre;
  for (int i = 0; i < words.size(); i++) {
    font->drawString(words[i].text, words[i].bounds, ci::vec2(), options);
  }
}

std::vector<word> renderer::layout() {
  // exit early if we can
  if(!invalidated) return words;
  invalidated = false;
  
  ci::vec2 size;
  ci::vec2 coords(0, 0);
  bounds.set(0.0f, 0.0f, 0.0f, 0.0f);
  std::vector<std::vector<word>> lines;
  
  for(auto & b: blocks) {
    std::vector<std::string> parts;
    lines.push_back(std::vector<word>());
    boost::split(parts, b, boost::is_any_of(" "));
    
    // iterate through each word in the string and create a bounding box for it
    for (int i = 0; i < parts.size(); i++) {
      ci::Rectf word_bounds;
      ci::vec2 new_size = font->measureString(parts[i]);
      size.x = new_size.x;
      size.y = fmax(new_size.y, size.y);
      
      if(coords.x + size.x > max_width) {
        coords.x = 0;
        coords.y += size.y + leading;
        lines.push_back(std::vector<word>());
      }
      
      word w(parts[i], ci::Rectf(
        coords.x, coords.y, coords.x + size.x,
        coords.y + size.y + leading
      ));
      
      coords.x += size.x + word_spacing;
      bounds.include(ci::vec2(coords.x - word_spacing, 0));
      
      lines[lines.size() - 1].push_back(w);
      words.push_back(w);
    }
    
    // offset each word by the difference of the max-width
    // and the end of the last word in a line
    if(alignment == Right) {
      words.clear();
      for(auto & l: lines) {
        if(l.size()) {
          auto & last = l[l.size() -1];
          float diff = max_width - last.bounds.x2;
          for(auto & w : l) {
            w.bounds.offset({diff, 0});
            words.push_back(w);
          }
        }
      }
    }
    
    coords.x = 0;
    coords.y += size.y + paragraph_spacing;
  }

   // remove leading from the bounding boxes of the last line of words
  for(auto & word : lines[lines.size()-1]) {
    word.bounds -= ci::vec2(0, leading);
    bounds.include(word.bounds);
  }
  
  return words;
}

ci::gl::TextureRef renderer::to_texture() {
  layout();
  // Not sure if this is a good idea or not
  // If the size is invalid, the creation of the fbo will crash
  // Return an empty texture instead?
  int w = get_bounds().getWidth();
  int h = get_bounds().getHeight();
  
  if(w == 0 || h == 0) {
    CI_LOG_W("Warning: Invalid size after call to layout");
    return ci::gl::Texture::create(1, 1);
  }
  
  // TODO: Make these format objects static, no need to re-create them every time
  ci::gl::Texture::Format tex_format;
  tex_format.setMagFilter(GL_NEAREST);
  tex_format.setMinFilter(GL_NEAREST);
  
  ci::gl::Fbo::Format fbo_format;
  fbo_format.setSamples(2);
  fbo_format.setColorTextureFormat(tex_format);
  
  ci::gl::FboRef fbo = ci::gl::Fbo::create(w, h, fbo_format);
  ci::gl::ScopedMatrices sm;
  ci::gl::ScopedFramebuffer sf(fbo);
  ci::gl::ScopedViewport sv(ci::ivec2(0), fbo->getSize());
  ci::gl::setMatricesWindow(fbo->getSize());
  ci::gl::clear(ci::ColorA(0, 0, 0, 0));
  draw();
  return fbo->getColorTexture();
  
}
