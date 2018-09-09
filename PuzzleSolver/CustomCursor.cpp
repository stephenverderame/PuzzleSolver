#include "CustomCursor.h"
std::shared_ptr<Cursor::CustomCursor> Cursor::CustomCursor::instance(new Cursor::CustomCursor());

std::shared_ptr<Cursor::CustomCursor> Cursor::CustomCursor::getInstance()
{
	return instance;
}

Cursor::CustomCursor::CustomCursor() : currentCursor(CURSOR_NORMAL)
{
	cursorArray[CURSOR_NORMAL] = LoadCursor(NULL, IDC_ARROW);
	cursorArray[CURSOR_WAIT] = LoadCursor(NULL, IDC_WAIT);
	cursorArray[CURSOR_DRAW] = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_CURSOR_PEN));
	cursorArray[CURSOR_SELECT] = LoadCursor(NULL, IDC_CROSS);
}

void Cursor::CustomCursor::setCursor(cursors c)
{
	getInstance()->currentCursor = c;
}

void Cursor::CustomCursor::drawCursor()
{
	SetCursor(getInstance()->cursorArray[getInstance()->currentCursor]);
}
