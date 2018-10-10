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
/**
 * RAII class for Win32 HPEN
 * Non-copyable
*/
class Stroke {
	friend class Wnd;
private:
	HPEN pen;
	bool mustDelete;
public:
	Stroke(const Stroke & other) = delete;
	Stroke& operator=(const Stroke & other) = delete;
public:
	Stroke(int size, color c);
	Stroke() : mustDelete(false) { pen = GetStockPen(NULL_PEN); }
	~Stroke();
};
/**
 * RAII class for Win32 HBRUSH
 * Non-copyable
*/
class Brush {
	friend class Wnd;
private:
	HBRUSH brush;
	bool mustDelete;
public:
	Brush(const Brush & other) = delete;
	Brush& operator=(const Brush & other) = delete;
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
	/** 
	 * Class copy of image. Resource is owned by this class
	 * Used to ensure only one image is on the screen at a time
	*/
	Image * img;
	Wnd(const Wnd& other);
public:
	Wnd();
	~Wnd();
	Wnd& operator=(const Wnd& other);

	//* Sends redraw message to window.
	void redraw() const;
	void setPixelColor(const Math::point & p, const color & c);
	HWND createDialog(int resourceId, callback_function func);

	/**
	 * Adds image to canvas. Only 1 visible image at a time.
	 * Once added to canvas, any redraw messages will redraw the added Image
	 * @param i a copy of this image is drawn on the screen.
	*/
	void drawImage(IMG::Img & i);

	/**
	 * Updates canvas. Redraws entire canvas
	 * Does not send actual WM_PAINT msg
	*/
	void canvasDraw();

	/**
	 * Determines if standrad Win32 erase functionality should be used
	*/
	void customErase(bool e);

	/**
	 * Uses GetDC to initialize the device context.
	 * Must be called before any draw commands.
	 * When finished drawing, stopDrawing() must be called
	*/
	void startDrawing();

	//* Wrapper for SwapBuffers. startDrawing() must have been called
	void swapBuffers();

	/**
	 * Releases DC
	 * @see startDrawing()
	*/
	void stopDrawing();

	/**
	 * Initializes DC with BeginPaint
	 * Uses a bitmap for double-buffering
	 * When drawing is done, endDBLPaint() must be called
	*/
	void beginDBLPaint();

	/**
	 * Double buffers internal bitmap onto the DC
	 * @see beginDBLPaint()
	*/
	void endDBLPaint();
	void clrScr();
	void update(Notification::notification n) override;
	/**
	 * Binds stroke for drawing. The set stroke is used for all draw commands until changed
	 * Should be called before any drawing function
	*/
	void setStroke(const Stroke & newStroke);

	/**
	 * Binds brush for drawing. The set brush is used for all draw commands until changed
	 * Should be called before any drawing function.
	*/
	void setBrush(const Brush & newBrush);

	void drawRect(Math::point topLeft, Math::point bottomRight);
	void drawLine(Math::point start, Math::point end);
	Math::point getMousePos();

};
/**
 * Class that allows the Window callback function to be called from Win32 API C code
 * @see Wnd
*/
class WndHelper {
private:
	static Wnd * instance;
public:
	static void setInstance(Wnd * w);
	static LRESULT __stdcall callback(HWND h, UINT u, WPARAM w, LPARAM l);

};

static const Brush  & null_brush = Brush();
static const Stroke & null_stroke = Stroke();
