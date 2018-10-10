#pragma once
#include "res.h"
#include <Windows.h>
#include <memory>
namespace Cursor {
	enum cursors {
		CURSOR_NORMAL,
		CURSOR_WAIT,
		CURSOR_DRAW,
		CURSOR_SELECT
	};
	/**
	 * Singleton handling different cursors
	*/
	class CustomCursor {
	private:
		HCURSOR cursorArray[4];
		cursors currentCursor;
		static std::shared_ptr<CustomCursor> instance;
	public:
		static std::shared_ptr<CustomCursor> getInstance();
	public:
		CustomCursor();
		void setCursor(cursors c);
		/**
		 * Wrapper for SetCursor()
		 * Must be called WM_SETCURSOR message (not a permanent draw)
		*/
		void drawCursor();
	};
}