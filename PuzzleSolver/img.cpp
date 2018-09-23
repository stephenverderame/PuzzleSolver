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

unsigned int IMG::Img::width() const noexcept
{
	return img->getWidth();
}

unsigned int IMG::Img::height() const noexcept
{
	return img->getHeight();
}

void IMG::Img::setPixel(const pixel p) noexcept
{
	img->setPixel(p.p.x, p.p.y, { p.c.red, p.c.green, p.c.blue });
}

void IMG::Img::setPixel(const channel r, const channel g, const channel b, const int x, const int y) noexcept
{
	img->setPixel(x, y, { r, g, b });
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

void IMG::Img::drawRect(Math::point topLeft, Math::point btmRight, color c)
{
	for (int i = topLeft.x; i <= btmRight.x; ++i) {
		img->setPixel(i, topLeft.y, (Color)c);
		img->setPixel(i, btmRight.y, (Color)c);
	}
	for (int i = topLeft.y; i <= btmRight.y; ++i) {
		img->setPixel(topLeft.x, i, (Color)c);
		img->setPixel(btmRight.x, i, (Color)c);
	}
}

IMG::color IMG::Img::getPixel(const Math::point p) const throw(IMG::ImgOutOfRangeException)
{
	if (p.x < 0 || p.x >= img->getWidth() || p.y < 0 || p.y >= img->getHeight()) throw ImgOutOfRangeException();
	Color col = img->getPixel(p.x, p.y);
	return{ col.r, col.g, col.b };
}

IMG::color IMG::Img::getPixel(const int x, const int y) const throw(IMG::ImgOutOfRangeException)
{
	if (x < 0 || x >= img->getWidth() || y < 0 || y >= img->getHeight()) throw ImgOutOfRangeException();
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

void IMG::Img::trueGrayscale(std::shared_ptr<GrayFunc> function)
{
	for (int i = 0; i < img->getHeight(); ++i) {
		for (int j = 0; j < img->getWidth(); ++j) {
			auto c = img->getPixel(j, i);
			img->setPixel(j, i, static_cast<Color>(function->change({c.r, c.g, c.b})));
		}
	}
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


void IMG::Img::drawLine(Math::point start, Math::point end, color c)
{
	Math::point a = end - start;
	double t = atan2(a.y, a.x);
	double distance = sqrt(a.x * a.x + a.y * a.y);
	Math::matrix m = {
		cos(t),	-sin(t),
		sin(t),	 cos(t)
	};
	for (int i = 0; i < ceil(distance); ++i) {
		Math::point p = { i, 0};
		p = m * p;
		p += start;
		setPixel({ c, p });
	}
}

void IMG::Img::clear() noexcept
{
	img->clearBmp(0);
}

bool IMG::Img::xInBounds(int x) noexcept
{
	return x >= 0 && x < img->getWidth();
}

bool IMG::Img::yInBounds(int y) noexcept
{
	return y >= 0 && y < img->getHeight();
}

int IMG::color::avg()
{
	return ((int)red + blue + green) / 3;
}

int IMG::color::total()
{
	return (int)red + green + blue;
}

IMG::color::operator Color()
{
	return { red, green, blue };
}
IMG::imgState::imgState(const imgState & other) : width(other.width), height(other.height)
{
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

IMG::color IMG::LightFunc::change(const color & original)
{
	int ret = (Math::max(Math::max(original.red, original.green), original.blue) + Math::min(Math::min(original.red, original.green), original.blue)) / 2.0;
	return{ static_cast<channel>(ret), static_cast<channel>(ret), static_cast<channel>(ret) };
}

IMG::color IMG::LumFunc::change(const color & original)
{
	int ret = (0.21*original.red + 0.72*original.green + 0.07*original.blue);
	return{ static_cast<channel>(ret), static_cast<channel>(ret), static_cast<channel>(ret) };
}
