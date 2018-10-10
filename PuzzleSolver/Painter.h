#pragma once
#include "Wnd.h"
#include "img.h"
#include "Observer.h"
#include <ctime>
/**
 * Basically just a class to keep clutter out of main message handling.
 * Handles most paint messages sent
*/
class Painter : public Notification::Subject {
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
	//*@param n unhandled paint notification
	void draw(Notification::notification n);
	//*@param c bool to determine if crop grid lines should be shown
	void crop(const bool c);
};