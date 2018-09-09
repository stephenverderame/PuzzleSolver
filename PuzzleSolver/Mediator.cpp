#include "Mediator.h"

void Control::GuiWndMediator::attachGui(Gui * gui)
{
	this->gui = gui;
}

void Control::GuiWndMediator::attachWnd(Wnd * wnd)
{
	this->wnd = wnd;
}

void Control::GuiWndMediator::sendToGui(events e)
{
	gui->update(e);
}
void Control::GuiWndMediator::update(const Notification::notification n)
{
	if (n.msg == Notification::msg_click) return;
	notify(n);
}
