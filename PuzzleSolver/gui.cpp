#include "gui.h"
using namespace gui;
using namespace Size;
using namespace Control;
using namespace Math;
Gui::Gui()
{
	int sections[] = { 100, -1 };
	page = new Page("Page 1", {
		new Scrollbar("testScroll", 10, WndDimensions::height - 20, WndDimensions::width - 20, 10, 100, scrollHorz, SBS_HORZ),
		new Scrollbar("scrollVert", WndDimensions::width - 10, 0, 10, WndDimensions::height, 100, scrollVert, SBS_VERT),
		new Progressbar("progress", 10, WndDimensions::height - 20, WndDimensions::width - 20, 20, 0, 100),
//		new Toolbar("toolbar",{ new ToolbarControls{ IDT_UNDO, gui::tbm_undo } }),
		new Statusbar("letterBar", 1, sections)
	});
	page->setLayout(new AbsoluteLayout());
	main = std::unique_ptr<MainPage>(new MainPage({ page }));
	main->navigateTo("Page 1");
	Progressbar * p = (Progressbar*)main->getCurrentPage()->getControl("progress");
	p->setMarquee(true);
	p->hideComponent();
	page->getControl("letterBar")->hideComponent();


	Scrollbar * scroll = (Scrollbar*)main->getCurrentPage()->getControl("testScroll");
	scroll->setScrollMessages([this](int changePos) {
		Notification::notification n;
		n.msg = Notification::messages::gui_hscroll;
		n.data1 = &changePos;
		notify(n);
	});
	scroll = (Scrollbar*)main->getCurrentPage()->getControl("scrollVert");
	scroll->setScrollMessages(nullptr, [this](int changePos) {
		Notification::notification n;
		n.msg = Notification::messages::gui_vscroll;
		n.data1 = &changePos;
		notify(n);
	});
}

void Gui::setStatusText(const char * str)
{
	((Statusbar*)main->getCurrentPage()->getControl("letterBar"))->setText(str, 0);
}

void Gui::updateScrollBar(int imgWidth, int imgHeight)
{
	Scrollbar * scroll = (Scrollbar*)main->getCurrentPage()->getControl("testScroll");
	scroll->update(max(imgWidth - WndDimensions::width, 2));
	scroll = (Scrollbar*)main->getCurrentPage()->getControl("scrollVert");
	scroll->update(max(imgHeight - WndDimensions::height, 2));
}

void Gui::update(events e)
{
	switch (e.c) {
	case wm_std:
		main->handleMessage((MSG*)e.data);
		break;
	}
}

void Gui::update(Notification::notification n)
{
	if (n.msg == Notification::messages::wnd_hscroll || n.msg == Notification::messages::wnd_vscroll) {
		Event * ev = reinterpret_cast<Event*>(n.data1);
		MSG m = ev->toMsg();
		main->handleMessage(&m);
		delete ev;
	}

}

void Gui::showLetterBar(const bool b)
{
	if (b)
		main->getCurrentPage()->getControl("letterBar")->showComponent();
	else
		main->getCurrentPage()->getControl("letterBar")->hideComponent();
}

void Gui::showProgressBar(const bool s)
{
	if (s)
		main->getCurrentPage()->getControl("progress")->showComponent();
	else
		main->getCurrentPage()->getControl("progress")->hideComponent();
}

int Gui::handleGUIEvents()
{
	MSG m;
	while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&m);
		if (m.message == WM_QUIT) return gui_msg_quit;
		DispatchMessage(&m);
		main->handleMessage(&m);
	}
	return gui_msg_ok;
}
