#pragma once
#include "CustomCursor.h"
#include <guiMainEx.h>
#include <Page.h>
#include <memory>
#include "Dimensions.h"
#include "events.h"
#include "Observer.h"
#undef max
#include "Math.h"
constexpr int gui_msg_quit = -1;
constexpr int gui_msg_ok = 0;
class Gui : public Notification::Subject, public Notification::Observer {
private:
	std::unique_ptr<gui::MainPage> main;
	gui::Page * page;
public:
	Gui();
	void setStatusText(const char * str);
	void updateScrollBar(const int imgWidth, const int imgHeight);
	void update(const ::Control::events e);
	void update(Notification::notification n) override;
	void showLetterBar(const bool b);
	void showProgressBar(const bool s);
	int handleGUIEvents();
};
