#include "Maze.h"
#include <thread>
#include <atomic>
struct CV::Maze::impl {
	std::atomic<bool> solving, invalidate;
	impl() : solving(false), invalidate(false) {}
};

CV::Maze::Maze(IMG::Img & i) : img(i) {
	pimpl = std::make_unique<impl>();
}

//* default behavior is fine, just had to be implemented in .cpp file after impl definition otherwise 
//* the deletion of pimpl would cause an error bc impl wasn't defined yet
CV::Maze::~Maze() = default;

void CV::Maze::loadMaze(IMG::Img & im)
{
	img = im;
}

void CV::Maze::choosePoint()
{
	if (!pimpl->solving) {
		buffer.reserve(img.height() * img.width());
		for (int i = 0; i < img.height() * img.width(); ++i) {
			int x = i % img.width();
			int y = i / img.width();
			IMG::color c = img.getPixel({ x, y });
			buffer[i] = (255 * 255 * 3) - ((int)c.red * c.red + (int)c.green * c.green + (int)c.blue * c.blue);
		}
		Notification::notification n;
		n.msg = Notification::messages::mze_selecting;
		notify(n);
		Cursor::CustomCursor::getInstance()->setCursor(Cursor::CURSOR_DRAW);
	}
}

void CV::Maze::calculatePath(Math::point start, Math::point end)
{
	std::thread t([this, start, end]() {calculatePathThread(start, end); });
	t.detach();
}

void CV::Maze::calculatePathThread(Math::point start, Math::point end)
{
	pimpl->solving = true;
	Grid g(img.width(), img.height(), buffer.data());
//	if (crop_1.x != crop_2.x && crop_1.y != crop_2.y)
//		g.setBound({ crop_1.x, crop_1.y }, { crop_2.x, crop_2.y });
	auto path = g.search({ start.x, start.y }, { end.x, end.y });
	while (!path.empty()) {
		auto color = path.top();
		IMG::pixel p;
		p.c = { 0, 0, 255 };
		p.p = { color.first, color.second };
		img.setPixel(p);
		path.pop();
	}
	pimpl->solving = false;
	pimpl->invalidate = true;
	Notification::notification n;
	n.msg = Notification::messages::mze_finish;
	notify(n);
}

bool CV::Maze::isSolving()
{
	bool s = pimpl->solving;
	return s;
}
