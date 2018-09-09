#pragma once
namespace Control {
	enum codes {
		wm_std,
		wm_vscroll,
		wm_hscroll
	};
	struct events {
		codes c;
		void * data;
	};
}