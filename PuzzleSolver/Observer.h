#pragma once
#include <vector>
#include <memory>
namespace Notification {
	enum class messages {
		msg_click,
		msg_undo,
		msg_rclick,
		msg_paint,
		msg_mmove,
		wnd_size,
		wnd_load,
		wnd_crop,
		wnd_tranpose,
		wnd_save,
		wnd_rotate,
		wnd_undo,
		wnd_solve_maze,
		wnd_solve_srch,
		wnd_view_read,
		wnd_hscroll,
		wnd_vscroll,
		dlg_set_limits,
		dlg_draw_lines,
		dlg_pick_origin,
		dlg_geta_origin,
		dlg_find_words,
		gui_hscroll,
		gui_vscroll,
		mze_selecting,
		mze_finish,
		pai_crop
	};
	struct notification {
		void * data1;
		void * data2;
		messages msg;
	};
	class Observer {
	public:
		virtual void update(const notification m) = 0;
	};
	class Subject {
	protected:
		std::vector<Observer *> observers;
	public:
		void notify(const notification m);
		void attatch(Observer * o);

	};
}
