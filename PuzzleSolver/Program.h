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
		//* Initializes Main as observer of window notifcations
		Main(Wnd & wnd);

		//* Sets Main class as observer of dialog notifications
		void attatchDialog(std::shared_ptr<DialogSpace::Dialog> d);

		/**
		 * Adds message to message queue
		 * @param m msg to be added to queue
		*/
		void update(const Notification::notification m) override;

		//* @return true if the message queue is empty
		bool noMessages() const;

		/** 
		 * Peek and pop from the msg queue
		 * @return notification at the front of the queue
		*/
		Notification::notification ppopMsg();
	};
}