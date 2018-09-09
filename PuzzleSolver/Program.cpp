#include "Program.h"
using namespace Notification;
Program::Main::Main(Wnd & wnd)
{
	wnd.attatch(this);
}

void Program::Main::attatchDialog(std::shared_ptr<DialogSpace::Dialog> d)
{
	d->attatch(this);
}

void Program::Main::update(const Notification::notification m)
{
	msgs.push(m);
}

bool Program::Main::noMessages() const
{
	return msgs.empty();
}

Notification::notification Program::Main::ppopMsg()
{
	Notification::notification msg = msgs.front();
	msgs.pop();
	return msg;
}
