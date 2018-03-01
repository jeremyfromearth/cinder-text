#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Text.h"
#include "text.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicApp : public App {
  public:
	void setup() override;
	void draw() override;
  text::renderer_ref t;
  gl::TextureRef tex;
  TextBox tb;
  gl::TextureRef tb_texture;
};

void BasicApp::setup() {
  std::string str = "It is in the admission of ignorance and the admission of uncertainty that there is a hope for the continuous motion of human beings in some direction that doesn't get confined, permanently blocked, as it has so many times before in various periods in the history of man. –Richard Feynman";
  
  t = text::renderer::create();
  t->set_font("DroidSansMono.ttf", 16);
  t->set_max_width(512);
  t->set_leading(0);
  t->set_word_spacing(8);
  t->set_text(str);
  t->set_color(ci::Color::black());
  tex = t->to_texture();
  
  tb = TextBox();
  tb.setFont(Font(loadAsset("DroidSansMono.ttf"), 16));
  tb.setSize(vec2(512, TextBox::GROW));
  tb.setColor(Color::black());
  tb.setText(str);
  tb_texture = gl::Texture::create(
    tb.render(),
    gl::Texture::Format()
      .minFilter(GL_NEAREST)
      .magFilter(GL_NEAREST));
}

void BasicApp::draw() {
	gl::clear(Color(0, 0, 0));
  gl::color(Color::white());
  gl::drawSolidRect(t->get_bounds());
  t->draw();
  gl::ScopedMatrices m;
  gl::translate(0, t->get_bounds().getHeight());
  gl::color(Color::white());
  gl::drawSolidRect(tex->getBounds());
  gl::draw(tex);
  gl::translate(0, tex->getBounds().getHeight());
  ci::gl::drawSolidRect(tb_texture->getBounds());
  gl::draw(tb_texture);
}

CINDER_APP(BasicApp, RendererGl)
