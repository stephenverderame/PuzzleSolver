#include "Painter.h"
using namespace Notification;
using namespace Math;
using namespace Cursor;

Painter::Painter(Wnd * w, IMG::Img * i)
{
	pimpl = std::shared_ptr<impl>(new impl());
	pimpl->window = w;
	pimpl->image = i;
}

void Painter::draw(Notification::notification n)
{
	switch (n.msg) {
	case messages::msg_click:
	if (pimpl->crop) {
		int x = *((int*)n.data1);
		int y = *((int*)n.data2);
		pimpl->lastTime = clock() / CLOCKS_PER_SEC;
		if (!pimpl->clicks) {
			pimpl->clicks++;
			pimpl->position_1 = { x, y };
			pimpl->window->startDrawing();
			pimpl->window->setPixelColor(pimpl->position_1, { 255, 0, 0 });
			pimpl->window->stopDrawing();
		}
		else {
			pimpl->crop = false;
			pimpl->window->customErase(false);
			pimpl->position_2 = { x, y };
//			pimpl->position_1.y -= 20; //acount for size of taskbar
//			pimpl->position_2.y -= 20;
			pimpl->showFinal = true;
			CustomCursor::getInstance()->setCursor(CURSOR_NORMAL);
			pimpl->clicks = 0;
//			addToUndoStack(img);
			IMG::Img image;
			image.createNew(abs(pimpl->position_2.x - pimpl->position_1.x), abs(pimpl->position_2.y - pimpl->position_1.y));
			for (int x = 0; x < image.width(); x++) {
				for (int y = 0; y < image.height(); y++) {
					image.setPixel({ pimpl->image->getPixel({min(pimpl->position_1.x, pimpl->position_2.x) + x, min(pimpl->position_1.y, pimpl->position_2.y) + y}) , {x, y} });
				}
			}
			notification n;
			n.msg = messages::pai_crop;
			IMG::ImgMemento * mem = new IMG::ImgMemento(image.getMemento());
			n.data1 = mem;
			notify(n);
			pimpl->position_1 = { 0, 0 };
			pimpl->position_2 = { 0, 0 };
		}
		delete (int*)n.data1;
		delete (int*)n.data2;
	}
	break;
	case messages::msg_mmove:
		if (pimpl->crop)
			pimpl->window->redraw();
		else if (pimpl->viewRead)
			pimpl->window->redraw();
		pimpl->mouse = { *(int*)n.data1, *(int*)n.data2 };
		delete (int*)n.data1;
		delete (int*)n.data2;
		break;
	case messages::msg_rclick:
		pimpl->crop = false;
		pimpl->clicks = 0;
		pimpl->showFinal = false;
		pimpl->window->redraw();
		pimpl->window->customErase(false);
		CustomCursor::getInstance()->setCursor(CURSOR_NORMAL);
		pimpl->position_1 = { 0, 0 };
		pimpl->position_2 = { 0, 0 };
		break;
	case messages::msg_paint:
	{
		if (pimpl->clicks > 0) {
			pimpl->window->beginDBLPaint();
			pimpl->window->clrScr();
			pimpl->window->canvasDraw(); 
			Stroke s(3, { 255, 0, 0 });
			pimpl->window->setStroke(s);
			pimpl->window->setBrush(null_brush);
			pimpl->window->drawRect({ min(pimpl->position_1.x, pimpl->mouse.x), min(pimpl->position_1.y, pimpl->mouse.y)}, { max(pimpl->position_1.x, pimpl->mouse.x), max(pimpl->position_1.y, pimpl->mouse.y) });
			pimpl->window->endDBLPaint();
		}
		break;
	}
	}
}

void Painter::crop(const bool c)
{
	pimpl->crop = c;
}
