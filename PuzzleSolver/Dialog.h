#pragma once
#include "CustomCursor.h"
#include "Math.h"
#include "Observer.h"
#include <vector>
#include "Wnd.h"
namespace DialogSpace {
	inline float getWndVal(HWND parent, int control);
	enum DialogMessage {
		dlgm_update_origin,
		dlgm_geta_origin
	};
	struct DialogNotification {
		DialogMessage m;
		void * data;
	};

	class Dialog : public Notification::Subject{
	public:
		virtual int __stdcall callback(HWND, UINT, WPARAM, LPARAM) = 0;
		virtual void openDialog() = 0;
		virtual void recv(DialogNotification n) = 0;
	};
	class RotateDialog : public Dialog {
	private:
		bool pOrigin;
		std::vector<Math::point> minLine;
		std::vector<Math::point> maxLine;
		Math::point origin;
		Wnd & window;
		HWND modelessDialog;
	public:
		int __stdcall callback(HWND hwnd, UINT msg, WPARAM w, LPARAM l);
	public:
		RotateDialog(Math::point origin, Wnd & wnd);
		void openDialog();
		void recv(DialogNotification n);
		void setOrigin(const Math::point p);
	};
	class SearchDialog : public Dialog {
	public:
		int __stdcall callback(HWND hwnd, UINT msg, WPARAM w, LPARAM l);
	public:
		void openDialog();
		void recv(DialogNotification n) {};
	};


	template<typename T, typename = void>
	struct is_dialog : std::false_type {};
	template<typename T>
	struct is_dialog<T, std::void_t<decltype(std::declval<T&>().callback(HWND(), UINT(), WPARAM(), LPARAM()))>> : std::true_type {};

	template <typename T>
	class DialogHelper {
	private:
		static std::shared_ptr<T> instance;
	public:
		static void setInstance(std::shared_ptr<T> i) { instance = i; }
		static int __stdcall callback(HWND h, UINT m, WPARAM w, LPARAM l) {
			assert(is_dialog<T>::value && "Dialog helper must be passed a dialog type!");
			return instance->callback(h, m, w, l);
		}
		static void send(DialogNotification n) {
			assert(is_dialog<T>::value && "Dialog helper must be passed a dialog type!");
			instance->recv(n);
		}
		
	};
}