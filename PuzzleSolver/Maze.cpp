#include "Maze.h"

CV::Maze::Maze(IMG::Img & i) : img(i) {}

void CV::Maze::loadMaze(IMG::Img & im)
{
	img = im;
}

void CV::Maze::choosePoint()
{
	sMu.lock();
	bool s = solving;
	sMu.unlock();
	if (!s) {
		buffer.reserve(img.height() * img.width());
		for (int i = 0; i < img.height() * img.width(); i++) {
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

void CV::Maze::calculatePath(Math::point start, Math::point end, Math::point crop_1, Math::point crop_2)
{
	std::thread t([this, start, end, crop_1, crop_2]() {calculatePathThread(start, end, crop_1, crop_2); });
	t.detach();
}

void CV::Maze::calculatePathThread(Math::point start, Math::point end, Math::point crop_1, Math::point crop_2)
{
	sMu.lock();
	solving = true;
	sMu.unlock();
	Grid g(img.width(), img.height(), buffer.data());
	if (crop_1.x != crop_2.x && crop_1.y != crop_2.y)
		g.setBound({ crop_1.x, crop_1.y }, { crop_2.x, crop_2.y });
	auto path = g.search({ start.x, start.y }, { end.x, end.y });
	while (!path.empty()) {
		pos color = path.top();
		IMG::pixel p;
		p.c = { 0, 0, 255 };
		p.p = { color.x, color.y };
		img.setPixel(p);
		path.pop();
	}
	sMu.lock();
	solving = false;
	sMu.unlock();
	iMu.lock();
	invalidate = true;
	iMu.unlock();
	Notification::notification n;
	n.msg = Notification::messages::mze_finish;
	notify(n);
}

bool CV::Maze::isSolving()
{
	std::lock_guard<std::mutex> guard(sMu);
	bool s = solving;
	return s;
}
