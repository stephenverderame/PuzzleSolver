#pragma once
#include "res.h"
#include <string>
#include <window.h>
#include <guiMain.h>
#include <memory>
#include "Observer.h"
#include "Wnd.h"
#include "Dialog.h"
#include <queue>
namespace Program {
	class Main : public Notification::Observer {
	private:
		std::queue<Notification::notification> msgs;
	public:
		Main(Wnd & wnd);
		void attatchDialog(std::shared_ptr<DialogSpace::Dialog> d);
		void update(const Notification::notification m) override;
		bool noMessages() const;
		Notification::notification ppopMsg();
	};
}