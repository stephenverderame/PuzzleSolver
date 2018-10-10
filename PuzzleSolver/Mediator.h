#pragma once
#include "gui.h"
#include "Wnd.h"
#include <memory>
#include <vector>
#include "events.h"
namespace Control {
	/**
	 * Provides indirection so there isn't a direct Gui-Wnd link
	 * Gui can notify Mediator which then notifies Wnd and visa versa
	 * @see Gui
	 * @see Wnd
	*/
	class GuiWndMediator : public Notification::Observer , public Notification::Subject {
	private:
		/**
		 * These resources are NOT owned by this class - acquantinceship
		*/
		Gui * gui;
		/**
		* These resources are NOT owned by this class - acquantinceship
		*/
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