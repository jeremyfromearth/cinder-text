#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "text.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicApp : public App {
  public:
	void setup() override;
	void draw() override;
  text::renderer t;
};

void BasicApp::setup() {
  t.set_font("DroidSansMono.ttf", 16);
  t.set_max_width(512);
  t.set_leading(0);
  t.set_word_spacing(8);
  t.set_text("It is in the admission of ignorance and the admission of uncertainty that there is a hope for the continuous motion of human beings in some direction that doesn't get confined, permanently blocked, as it has so many times before in various periods in the history of man. –Richard Feynman");
  t.set_color(ci::Color(0.2, 0.2, 0.2));
}

void BasicApp::draw() {
	gl::clear(Color(0, 0, 0));
  gl::color(Color::white());
  gl::drawSolidRect(t.get_bounds());
  t.draw();
}

CINDER_APP(BasicApp, RendererGl)
