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
/**
 * Handles all gui interactions
*/
class Gui : public Notification::Subject, public Notification::Observer {
private:
	std::unique_ptr<gui::MainPage> main;
	gui::Page * page;
public:
	Gui();
	/**
	 * Sets text of status bar
	 * Used to show nearest letter
	 * @see viewLetters
	*/
	void setStatusText(const char * str);
	/**
	 * Changes scrollbar step based upon new image dimensions
	 * @param image dimentions, width and height respectivly
	*/
	void updateScrollBar(const int imgWidth, const int imgHeight);
	/**
	 * @param e delegates event to internal controls
	 */
	void update(const ::Control::events e);
	/**
	 * Handles scroll notifications
	 * @param n notification to handle
	*/
	void update(Notification::notification n) override;
	/**
	 * Shows status bar (nearest letter)
	 */
	void showLetterBar(const bool b);
	/**
	 * Shows progress bar in marquee form
	*/
	void showProgressBar(const bool s);
	/**
	 * Performs Win32 message loop and delegates messages to internal controls
	 * @return gui_msg_quit (-1) if WM_QUIT is encountered otherwise gui_msg_ok (0)
	 */
	int handleGUIEvents();
};
