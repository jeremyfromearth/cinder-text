#pragma once

#include <algorithm>
#include <string>

// boost
#include "boost/algorithm/string.hpp"

// cinder
#include "cinder/Color.h"
#include "cinder/Font.h"
#include "cinder/Json.h"
#include "cinder/Text.h"
#include "cinder/Timeline.h"
#include "cinder/Vector.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/Utilities.h"
#include "cinder/Unicode.h"


class TextFormat {
public:
  
  std::map<std::string, ci::JsonTree> styles;
  
  static TextFormat & getInstance() {
    static TextFormat instance;
    return instance;
  }
  
  TextFormat() {
    charset =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!`1234567890-=~!@#$%^&*()_+[]\{}|;':\",./<>?";
  }
  
  void addToCharset(std::string s) {
    charset += s;
    std::string upper;
    std::transform(s.begin(), s.end(), upper.begin(), toupper);
    charset += upper;
  }
  
  
  void trimCharset() {
    std::u32string str = ci::toUtf32(charset);
    std::stable_sort(str.begin(), str.end());
    auto unique_end = std::unique(str.begin(), str.end());
    str.erase(unique_end, str.end());
    unique = ci::toUtf8(str);
  }
  
  std::string getCharset() {
    return charset;
  }
  
  void init(ci::JsonTree cfg) {
    std::string styleName;
    ci::JsonTree::Iter iter;
    for (iter = cfg.begin(); iter != cfg.end(); iter++) {
      styleName = iter->getKey();
      styles.insert(std::pair<std::string, ci::JsonTree>(styleName, *iter));
    }
  }
  
protected:
  std::string charset;
  std::string unique;
  std::map<char, bool> exists;
};

class Word {
public:
  std::string text;
  ci::Rectf bounds;
  Word(std::string t, ci::Rectf b) : text(t), bounds(b) {}
  ~Word() {
    text = "";
  }
};

class Text {
public:
  enum Alignment { LEFT, RIGHT };
  
  ci::vec2 coords;
  
  Text() {
    text = "";
    leading = 0;
    maxWidth = 512;
    wordSpacing = 8;
    alignment = LEFT;
    color = ci::Color::black();
    format = &TextFormat::getInstance();
    options.clipVertical(false).clipHorizontal(false);
  }
  
  ~Text() {
    clear();
  }
  
  Alignment getAlignment() { return alignment; }
  
  ci::Rectf getBounds() { return bounds; }
  
  int getMaxWidth() { return maxWidth; }
  
  void setAlignment(Alignment a) { alignment = a; }
  
  void setColor(ci::Color c) {
    color = ci::ColorA(c, 1.0);
  }
  
  void setColor(ci::ColorA c) {
    color = c;
  }
  
  void setFont(std::string fontName, int fontSize) {
    ci::Font f = ci::Font(ci::app::loadAsset(fontName), fontSize);
    font = ci::gl::TextureFont::create(f, ci::gl::TextureFont::Format(), format->getCharset());
  }
  
  void setMaxWidth(int w) { maxWidth = w; }
  
  void setLeading(int l) { leading = l; }
  
  void setLineOffset(ci::vec2 offset) {
    lineOffset = offset;
  }
  
  void setStyle(std::string styleName) {
    ci::JsonTree styles = format->styles[styleName];
    leading = styles.getChild("leading").getValue<int>();
    wordSpacing = styles.getChild("word_spacing").getValue<int>();
    setFont(styles.getChild("font").getValue<std::string>(), styles.getChild("size").getValue<int>());
  }
  
  void setText(std::string s) {
    clear();
    text = s;
    std::vector<std::string> parts;
    boost::split(parts, text, boost::is_any_of(" "));
    
    ci::vec2 coords(0, 0);
    std::vector<std::vector<Word>> lines;
    lines.push_back(std::vector<Word>());
    bounds.set(0.0f, 0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < parts.size(); i++) {
      ci::Rectf wordBounds;
      ci::vec2 size = font->measureString(parts[i]) + wordPadding;
      if (coords.x + size.x < maxWidth) {
        wordBounds.set(coords.x, coords.y, coords.x + size.x, coords.y + size.y);
        coords.x += size.x + wordSpacing;
      }
      else{
        coords.y += leading;
        wordBounds.set(0, coords.y, size.x, coords.y + leading);
        coords.x = size.x + wordSpacing;
        lines.push_back(std::vector<Word>());
      }
      
      Word word = Word(parts[i], wordBounds);
      lines[lines.size() - 1].push_back(word);
      words.push_back(word);
      bounds.include(word.bounds);
    }
    
     // remove leading from the bounding boxes of the last line of words
    for(auto & word : lines[lines.size()-1]) {
      std::cout << "pre: " << word.bounds << std::endl;
      //word.bounds -= ci::vec2(0, leading);
      std::cout << "post: " << word.bounds << std::endl;
    }
    
    if (alignment == RIGHT) {
      words.clear();
      for (int i = 0; i < lines.size(); i++) {
        float width = 0;
        for (int j = 0; j < lines[i].size(); j++) {
          width += lines[i][j].bounds.getWidth();
        }
        
        width += wordSpacing * lines[i].size();
        width -= wordSpacing;
        
        float offset = std::max(0.0f, (float)maxWidth - width);
        for (int j = 0; j < lines[i].size(); j++) {
          lines[i][j].bounds.offset(ci::vec2(offset, 0.0f));
          words.push_back(lines[i][j]);
        }
      }
    }
  }
  
  void setWordPadding(int x, int y) { wordPadding = ci::vec2(x, y); }
  
  void setWordSpacing(int n) { wordSpacing = n; }
  
  void clear() {
    text = "";
    words.clear();
  }
  
  void draw() {
    ci::gl::enableAlphaBlending();
    ci::gl::pushMatrices();
    ci::gl::translate(coords);
    for (int i = 0; i < words.size(); i++) {
      ci::gl::color(color);
      font->drawString(words[i].text, words[i].bounds, lineOffset, options);
    }
    ci::gl::popMatrices();
  }

protected:
  
  std::string text;
  std::vector<Word> words;
  
  int leading;
  int maxWidth;
  int wordSpacing;
  
  ci::ColorA color;
  ci::vec2 lineOffset;
  ci::vec2 wordPadding;
  
  ci::Rectf bounds;
  ci::gl::TextureFontRef font;
  ci::Anim<ci::vec2> highlightAlpha;
  
  TextFormat * format;
  
  Alignment alignment;
  ci::gl::TextureFont::DrawOptions options;
};
