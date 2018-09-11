#pragma once
#include "gui.h"
#include "Wnd.h"
#include <memory>
#include <vector>
#include "events.h"
namespace Control {
	class GuiWndMediator : public Notification::Observer , public Notification::Subject {
	private:
		Gui * gui;
		Wnd * wnd;
	public:
		GuiWndMediator() {};
		GuiWndMediator(Gui * gui, Wnd * wnd) : gui(gui), wnd(wnd) {};
		void attachGui(Gui * gui);
		void attachWnd(Wnd * wnd);
		void sendToGui(events e);
		void update(const Notification::notification n) override;

	};
}