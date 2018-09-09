#pragma once
#include "gui.h"
#include "res.h"
#include <window.h>
#include "Observer.h"
#include "Math.h"
#include "events.h"
#include "CustomCursor.h"
#include "Dimensions.h"
#include "img.h"
#include <sstream>
#include <iostream>
struct color {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};
class Wnd;
class Stroke {
	friend class Wnd;
private:
	HPEN pen;
	bool mustDelete;
	Stroke(const Stroke & other) {};
	Stroke& operator=(const Stroke & other) {};
public:
	Stroke(int size, color c);
	Stroke() : mustDelete(false) { pen = GetStockPen(NULL_PEN); }
	~Stroke();
};
class Brush {
	friend class Wnd;
private:
	HBRUSH brush;
	bool mustDelete;
	Brush(const Brush & other) {};
	Brush& operator=(const Brush & other) {};
public:
	Brush(color c);
	Brush() : mustDelete(false) { brush = GetStockBrush(NULL_BRUSH); }
	~Brush();
};
using callback_function = int(__stdcall*)(HWND, UINT, WPARAM, LPARAM);
class WndHelper;
class Wnd : public Notification::Subject, public Notification::Observer {
	friend class WndHelper;
private:
	HDC dc, dco;
	HBITMAP hbm, hbmOld;
	HBRUSH oldBrush;
	HPEN oldPen;
	bool firstPen, firstBrush;
	PAINTSTRUCT paint;
	std::shared_ptr<Window> display;
	int __stdcall callback(HWND hwnd, UINT msg, WPARAM w, LPARAM l);
	bool overrideErase = false;
	bool dcIsValid = false;
	Image * img;
	Wnd(const Wnd& other);
public:
	Wnd();
	~Wnd();
	Wnd& operator=(const Wnd& other);
	void redraw() const;
	void setPixelColor(const Math::point & p, const color & c);
	HWND createDialog(int resourceId, callback_function func);
	void drawImage(IMG::Img & i);
	void canvasDraw();
	void customErase(bool e);
	void startDrawing();
	void swapBuffers();
	void stopDrawing();
	void beginDBLPaint();
	void endDBLPaint();
	void clrScr();
	void update(Notification::notification n);
	void setStroke(Stroke & newStroke);
	void setBrush(Brush & newBrush);
	void drawRect(Math::point topLeft, Math::point bottomRight);
	Math::point getMousePos();

};
class WndHelper {
private:
	static Wnd * instance;
public:
	static void setInstance(Wnd * w);
	static LRESULT __stdcall callback(HWND h, UINT u, WPARAM w, LPARAM l);

};