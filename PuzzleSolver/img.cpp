#include "img.h"

void IMG::Img::loadFromPath(const char * s)
{
	img = std::shared_ptr<Image>(new Image(s));
	loaded = true;
}

void IMG::Img::createNew(int width, int height)
{
	img = std::shared_ptr<Image>(new Image(width, height));
	loaded = true;
}

unsigned int IMG::Img::width() const
{
	return img->getWidth();
}

unsigned int IMG::Img::height() const
{
	return img->getHeight();
}

void IMG::Img::setPixel(const pixel p)
{
	img->setPixel(p.p.x, p.p.y, { p.c.red, p.c.green, p.c.blue });
}

void IMG::Img::rect(Math::point topLeft, Math::point btmRight, color c)
{
	for (int x = topLeft.x; x <= btmRight.x; ++x) {
		for (int y = topLeft.y; y <= btmRight.y; ++y) {
			pixel p;
			p.c = c;
			p.p = { x, y };
			setPixel(p);
		}
	}
}

IMG::color IMG::Img::getPixel(const Math::point p) const
{
	Color col = img->getPixel(p.x, p.y);
	return{ col.r, col.g, col.b };
}

IMG::color IMG::Img::getPixel(const int x, const int y) const
{
	Color col = img->getPixel(x, y);
	return{ col.r, col.g, col.b };
}

void IMG::Img::saveAsBmp(const char * path) const
{
	img->saveBmp(path);
}

void IMG::Img::greyscale()
{
	img->toGreyscale();
}

void IMG::Img::resize(int width, int height)
{
	img->resize(width, height);
}

int IMG::Img::integralImageValue(int x, int y)
{
	return img->integralImageValue(x, y);
}

IMG::ImgMemento IMG::Img::getMemento()
{
	return ImgMemento(this);
}

void IMG::Img::loadFrom(ImgMemento memento)
{
	imgState state = memento.getState();
	if (img->getWidth() != state.width || img->getHeight() != state.height)
		img->resize(state.width, state.height);

	for (int i = 0; i < state.width * state.height; ++i) {
		int x = i % state.width;
		int y = i / state.width;
		color col = state.imgData[i];
		img->setPixel(x, y, (Color)col);
	}
}


IMG::Img::operator Image*()
{
	assert(!"THIS CODE SHOULD NOT BE USED - JUST HERE TO STOP ERRORS");
	return img.get();
}

int IMG::color::avg()
{
	return ((int)red + blue + green) / 3;
}

IMG::color::operator Color()
{
	return { red, green, blue };
}
IMG::imgState::imgState(const imgState & other) : width(other.width), height(other.height)
{
	printf("state copy constructor\n");
	if (other.width * other.height > 0) {
		imgData.reserve(other.width * other.height);
		for (int i = 0; i < other.width * other.height; ++i)
			imgData[i] = other.imgData[i];
	}
}

IMG::imgState & IMG::imgState::operator=(const imgState & other)
{
	width = other.width;
	height = other.height;
	if (imgData.size() > 0)
		imgData.clear();
	if (other.width * other.height > 0) {
		imgData.reserve(width * height);
		for (int i = 0; i < width * height; ++i)
			imgData[i] = other.imgData[i];
	}
	return *this;
}
IMG::ImgMemento::ImgMemento(IMG::Img * img)
{
	state.width = img->width();
	state.height = img->height();
	if (img->width() * img->height() > 0) {
		state.imgData.reserve(state.width * state.height);
		for (int i = 0; i < img->width() * img->height(); ++i) {
			int x = i % img->width();
			int y = i / img->width();
			state.imgData[i] = img->getPixel(x, y);
		}
	}
}

