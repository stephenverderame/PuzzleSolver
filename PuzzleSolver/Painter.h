#pragma once
#include "Wnd.h"
#include "img.h"
#include <ctime>
class Painter {
private:
	struct impl {
		Wnd * window;
		IMG::Img * image;
		bool crop, showFinal;
		Math::point position_1, position_2;
		clock_t lastTime;
		int clicks;
		bool viewRead;
		Math::point mouse;
		impl() : crop(false), showFinal(false), lastTime(0),
			clicks(0), viewRead(false) {}
	};
	std::shared_ptr<impl> pimpl;
public:
	Painter(Wnd * w, IMG::Img * i);
	void draw(Notification::notification n);
	void crop(const bool c);
};