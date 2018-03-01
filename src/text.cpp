//
//  text.cpp
//  Basic
//
//  Created by jeremy on 2/28/18.
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
  !`-=~!@#$%^&*()_+[]\{}|;':\",./<>?`¡ \
  ™£¢∞§¶•ªº–≠`⁄€‹›ﬁﬂ‡°·‚—±Œ„´‰ˇÁ¨ˆØ∏”’ \
  »ÅÍÎÏ˝ÓÔÒÚÆ¸˛Ç◊ı˜Â¯˘¿œ∑´†¥¨π“‘«åß∂ƒ© \
  ˙∆˚¬…æΩ≈ç√∫˜µ≤≥÷";

renderer_ref renderer::create() {
  return std::make_shared<renderer>();
}

renderer_ref renderer::create(std::string str, ci::Font f) {
  auto r = std::make_shared<renderer>();
  r->set_font(f);
  r->set_text(str);
  return r;
}

renderer::renderer() {
  str = "";
  leading = 0;
  max_width = 512;
  word_spacing = 8;
  alignment = Left;
  invalidated = true;
  color = ci::Color::black();
  options.clipVertical(false).clipHorizontal(false).pixelSnap(true);
}

void renderer::set_font(std::string path, int font_size, std::string charset) {
  ci::Font f = ci::Font(ci::app::loadAsset(path), font_size);
  renderer::set_font(f, charset);
  invalidated = true;
}

void renderer::set_font(ci::Font f, std::string charset) {
  font = ci::gl::TextureFont::create(f, ci::gl::TextureFont::Format(), charset);
  invalidated = true;
}

void renderer::clear() {
  str = "";
  words.clear();
  invalidated = true;
}

void renderer::draw() {
  layout();
  ci::gl::ScopedMatrices m1;
  ci::gl::ScopedColor c;
  ci::gl::ScopedBlendAlpha a;
  ci::gl::enableAlphaBlendingPremult();
  for (int i = 0; i < words.size(); i++) {
    ci::gl::color(color);
    font->drawString(words[i].text, words[i].bounds, ci::vec2(), options);
  }
  ci::gl::disableAlphaBlending();
}

void renderer::layout() {
  // exit early if we can
  if(!invalidated) return;
  invalidated = false;
  std::vector<std::string> parts;
  boost::split(parts, str, boost::is_any_of(" "));

  ci::vec2 coords(0, 0);
  std::vector<std::vector<word>> lines;
  lines.push_back(std::vector<word>());
  bounds.set(0.0f, 0.0f, 0.0f, 0.0f);

  // iterate through each word in the string and create a bounding box for it
  for (int i = 0; i < parts.size(); i++) {
    ci::Rectf word_bounds;
    ci::vec2 size = font->measureString(parts[i]);
    
    if(coords.x + size.x > max_width) {
      coords.x = 0;
      coords.y += size.y + leading;
      lines.push_back(std::vector<word>());
    }
    
    word w(parts[i],
      ci::Rectf(
        coords.x, coords.y, coords.x + size.x,
        coords.y + size.y + leading
      ));
    
    coords.x += size.x + word_spacing;
    bounds.include(ci::vec2(coords.x - word_spacing, 0));
    
    lines[lines.size() - 1].push_back(w);
    words.push_back(w);
  }

   // remove leading from the bounding boxes of the last line of words
  for(auto & word : lines[lines.size()-1]) {
    word.bounds -= ci::vec2(0, leading);
    bounds.include(word.bounds);
  }

  // offset lines to be right aligned
  if (alignment == Right) {
    words.clear();
    for (int i = 0; i < lines.size(); i++) {
      float width = 0;
      for (int j = 0; j < lines[i].size(); j++) {
        width += lines[i][j].bounds.getWidth();
      }
      
      width += word_spacing * lines[i].size();
      width -= word_spacing;
      
      float offset = std::max(0.0f, (float)max_width - width);
      for (int j = 0; j < lines[i].size(); j++) {
        lines[i][j].bounds.offset(ci::vec2(offset, 0.0f));
        words.push_back(lines[i][j]);
      }
    }
  }
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
  
  ci::gl::Texture::Format tex_format;
  tex_format.setMagFilter(GL_NEAREST);
  tex_format.setMinFilter(GL_NEAREST);
  
  ci::gl::Fbo::Format fbo_format;
  fbo_format.setSamples(16);
  fbo_format.setColorTextureFormat(tex_format);
  
  
  ci::gl::FboRef fbo = ci::gl::Fbo::create(w, h, fbo_format);
  ci::gl::ScopedMatrices sm;
  ci::gl::ScopedFramebuffer sf(fbo);
  ci::gl::ScopedViewport sv(ci::ivec2(0), fbo->getSize());
  ci::gl::setMatricesWindow(fbo->getSize());
  ci::gl::ScopedBlendAlpha sa;
  ci::gl::clear(ci::ColorA(0, 0, 0, 0));
  draw();
  return fbo->getColorTexture();
}
