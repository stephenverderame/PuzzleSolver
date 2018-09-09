#include "Wnd.h"
using namespace Notification;
Wnd * WndHelper::instance = nullptr;
int Wnd::callback(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
{
	notification n;
	switch (msg) {
	case WM_LBUTTONDOWN:
		printf("Mouse click!\n");
		n.msg = msg_click;
		n.data1 = new int(GET_X_LPARAM(l));
		n.data2 = new int(GET_Y_LPARAM(l));
		notify(n);
		break;
	case WM_KEYDOWN:
		if (w == 'Z' && GetAsyncKeyState(VK_CONTROL) < 0) {
			n.msg = msg_undo;
			notify(n);
		}
		break;
	case WM_RBUTTONDOWN:
		n.msg = msg_rclick;
		notify(n);
		break;
	case WM_ERASEBKGND:
		if (overrideErase)
			return TRUE;
		break;
	case WM_PAINT:
		n.msg = msg_paint;
		notify(n);
		break;
	case WM_MOUSEMOVE:
		n.msg = msg_mmove;
		n.data1 = new int(GET_X_LPARAM(l));
		n.data2 = new int(GET_Y_LPARAM(l));
		notify(n);
		break;

	}
/*	else if (msg == WM_LBUTTONDOWN && pOrigin) {
		origin = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };

	} */
//	if (msg == WM_ERASEBKGND && crop) return TRUE;
	return FALSE;
}

Wnd::Wnd() : img(nullptr)
{
	display = std::shared_ptr<Window>(new Window("Puzzle Solver"));
	display->use();
	WNDCLASSEX update;
	update.lpszMenuName = MAKEINTRESOURCE(ID_MENU);
	update.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON));
	update.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON), IMAGE_ICON, 16, 16, 0);
	display->setWindowProperty(menuName, update);
	display->setWindowProperty(icon, update);
	WndHelper::setInstance(this);
	display->setWindowProc(WndHelper::callback);
	display->createWindow(300, 300, 800, 800);
	Size::WndDimensions::width = 800;
	Size::WndDimensions::height = 800;
	gui::GUI::bindWindow(display->getHwnd());

	display->addEventListener(new EventListener([this](EventParams ep) {
		Size::WndDimensions::width = LOWORD(ep.getParaml());
		Size::WndDimensions::height = HIWORD(ep.getParaml());
		notification n;
		n.msg = wnd_size;
		notify(n);
	}, WM_SIZE));
	display->addEventListener(new EventListener([this](EventParams ep) {
		notification n;
		ZeroMemory(&n, sizeof(n));
		switch (LOWORD(ep.getParam3())) {
		case IDM_LOAD:
			n.msg = wnd_load;
			break;
		case IDM_CROP:
			n.msg = wnd_crop;
			break;
		case IDM_MONOCHROME:
			n.msg = wnd_monochrome;
			break;
		case IDM_ROTATE:
			n.msg = wnd_rotate;
			break;
		case IDM_SAVE:
			n.msg = wnd_save;
			break;
		case IDM_SOLVE_MAZE:
			n.msg = wnd_solve_maze;
			break;
		case IDM_SOLVE_SEARCH:
			n.msg = wnd_solve_srch;
			break;
		case IDM_VIEW_READ:
			n.msg = wnd_view_read;
			break;
		case IDT_UNDO:
			n.msg = wnd_undo;
			break;
		}
		if (n.msg != msg_click) //if msg is set
			notify(n);
	}, WM_COMMAND));
	display->addEventListener(new EventListener([this](EventParams ep) {
		notification n;
		n.msg = wnd_hscroll;
		n.data1 = new Event(WM_HSCROLL, ep);
		notify(n);
	}, WM_HSCROLL));
	display->addEventListener(new EventListener([this](EventParams ep) {
		notification n;
		n.msg = wnd_vscroll;
		n.data1 = new Event(WM_VSCROLL, ep);
		notify(n);
	}, WM_VSCROLL));
	display->addEventListener(new EventListener([this](EventParams ep) {
		Cursor::CustomCursor::getInstance()->drawCursor();
	}, WM_SETCURSOR));
}

Wnd::~Wnd()
{
	if (img != nullptr)
		delete img;
}

Wnd & Wnd::operator=(const Wnd & other)
{
	display = other.display;
	if (img != nullptr)
		delete img;
	if (other.img != nullptr) {
		img = new Image(other.img->getWidth(), other.img->getHeight());
		for (int i = 0; i < other.img->getWidth() * other.img->getHeight(); i++) {
			int x = i % other.img->getWidth();
			int y = i / other.img->getWidth();
			img->setPixel(x, y, other.img->getPixel(x, y));
		}
	}
}

void Wnd::redraw() const
{
	RECT r;
	GetClientRect(display->getHwnd(), &r);
	InvalidateRect(display->getHwnd(), &r, TRUE);
}

void Wnd::setPixelColor(const Math::point & p, const color & c)
{
	SetPixel(dc, p.x, p.y, RGB(c.red, c.green, c.blue));
}

HWND Wnd::createDialog(int resourceId, callback_function func)
{
	return CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceId), display->getHwnd(), func);
}

void Wnd::drawImage(IMG::Img & i)
{
	if (img != nullptr) {
		display->deleteImage(img);
		delete img;
	}
	img = new Image(i.width(), i.height());
	for (int j = 0; j < i.width() * i.height(); j++) {
		int x = j % i.width();
		int y = j / i.width();
		IMG::color c = i.getPixel({ x, y });
		img->setPixel(x, y, {c.red, c.green, c.blue});
	}
	display->drawImage(img);
}

void Wnd::canvasDraw()
{
	if (dcIsValid)
		display->getCanvas()->draw(dc);
	else
		display->getCanvas()->draw();
}

void Wnd::customErase(bool e)
{
	overrideErase = e;
}

void Wnd::startDrawing()
{
	dc = GetDC(display->getHwnd());
	firstBrush = true;
	firstPen = true;
	dcIsValid = true;
}

void Wnd::swapBuffers()
{
	SwapBuffers(dc);
}

void Wnd::stopDrawing()
{
	if (!firstBrush)
		SelectObject(dc, oldBrush);
	if (!firstPen)
		SelectObject(dc, oldPen);
	ReleaseDC(display->getHwnd(), dc);
	dcIsValid = false;
}

void Wnd::beginDBLPaint()
{
	RECT r;
	GetClientRect(display->getHwnd(), &r);
//	BeginPaint(display->getHwnd(), &paint);
	dco = GetDC(display->getHwnd());
	dc = CreateCompatibleDC(dco);
	hbm = CreateCompatibleBitmap(dco, r.right - r.left, r.bottom - r.top);
	hbmOld = (HBITMAP)SelectObject(dc, hbm);
	firstPen = true;
	firstBrush = true;
	dcIsValid = true;

	
}

void Wnd::endDBLPaint()
{
	if (!firstBrush)
		SelectObject(dc, oldBrush);
	if (!firstPen)
		SelectObject(dc, oldPen);
	RECT r;
	GetClientRect(display->getHwnd(), &r);

	BitBlt(dco, r.left, r.top, r.right - r.left, r.bottom - r.top, dc, 0, 0, SRCCOPY);

	SelectObject(dc, hbmOld);
	DeleteObject(hbm);
	DeleteDC(dc);

	dcIsValid = false;

//	EndPaint(display->getHwnd(), &paint);
	ReleaseDC(display->getHwnd(), dco);
}

void Wnd::clrScr()
{
	assert(dcIsValid && "Attempted operation that requires a valid DC!");
	RECT r;
	GetClientRect(display->getHwnd(), &r);
	FillRect(dc, &r, (HBRUSH)COLOR_WINDOW);
}

void Wnd::update(Notification::notification n)
{
	switch (n.msg) {
	case Notification::gui_hscroll:
	{
		int changePos = *((int*)n.data1);
		for (auto it = display->getCanvas()->getImages()->begin(); it != display->getCanvas()->getImages()->end(); it++) {
			(*it)->setPosition((*it)->getX() - changePos, (*it)->getY());
		}
		break;
	}
	case Notification::gui_vscroll:
	{
		int changePos = *((int*)n.data1);
		for (auto it = display->getCanvas()->getImages()->begin(); it != display->getCanvas()->getImages()->end(); it++) {
			(*it)->setPosition((*it)->getX(), (*it)->getY() - changePos);
		}
		break;
	}
	}
}

void Wnd::setStroke(Stroke & newStroke)
{
	HPEN p = (HPEN)SelectObject(dc, newStroke.pen);
	if (firstPen) {
		firstPen = false;
		oldPen = p;
	}
}

void Wnd::setBrush(Brush & newBrush)
{
	HBRUSH b = (HBRUSH)SelectObject(dc, newBrush.brush);
	if (firstBrush) {
		firstBrush = false;
		oldBrush = b;
	}
}

void Wnd::drawRect(Math::point topLeft, Math::point bottomRight)
{
	assert(dcIsValid && "Attempted operation that requires a valid DC!");
	Rectangle(dc, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
}

Math::point Wnd::getMousePos()
{
	POINT mp;
	GetCursorPos(&mp);
	ScreenToClient(display->getHwnd(), &mp);
	return{ mp.x, mp.y };
}

Stroke::Stroke(int size, color c) : mustDelete(true)
{
	pen = CreatePen(PS_SOLID, size, RGB(c.red, c.green, c.blue));
}

Stroke::~Stroke()
{
	if(mustDelete)
		DeleteObject(pen);
}

Brush::Brush(color c) : mustDelete(true)
{
	brush = CreateSolidBrush(RGB(c.red, c.green, c.blue));
}

Brush::~Brush()
{
	if(mustDelete)
		DeleteObject(brush);
}

void WndHelper::setInstance(Wnd * w)
{
	instance = w;
}

LRESULT WndHelper::callback(HWND h, UINT u, WPARAM w, LPARAM l)
{
	return instance->callback(h, u, w, l);
}
