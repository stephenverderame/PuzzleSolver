#include "Dialog.h"
using namespace Cursor;
using namespace Notification;
namespace DialogSpace {

	template <typename T>
	std::shared_ptr<T> DialogHelper<T>::instance;

	inline float getWndVal(HWND parent, int control) {
		int size = GetWindowTextLength(GetDlgItem(parent, control));
		if (size == 0) return 0;
		char * txt = new char[size + 1];
		GetWindowText(GetDlgItem(parent, control), txt, size + 1);
		float f = atoi(txt);
		delete[] txt;
		return f;
	}
	int __stdcall RotateDialog::callback(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
	{
		switch (msg) {
		case WM_INITDIALOG:
		{
			HWND autoCheck = GetDlgItem(hwnd, IDC_CHECK1);
			HWND min = GetDlgItem(hwnd, IDC_EDIT1);
			HWND max = GetDlgItem(hwnd, IDC_EDIT2);
			SendMessage(autoCheck, BM_SETCHECK, BST_CHECKED, 1);
			SetWindowText(min, "0");
			SetWindowText(max, "360");
			EnableWindow(GetDlgItem(hwnd, IDC_EDIT3), 0);
			EnableWindow(GetDlgItem(hwnd, IDC_EDIT4), 0);
			HICON icon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON), IMAGE_ICON, 16, 16, 0);
			if (icon)
				SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
			break;
		}
		case WM_COMMAND:
		{
			if (LOWORD(w) == IDC_BUTTON1) {
				float * data = new float[2];
				float min = 0, max = 0;
				if (SendMessage(GetDlgItem(hwnd, IDC_CHECK1), BM_GETCHECK, 0, 0) == BST_CHECKED) {
					//auto rotate checked
					int size = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT1));
					char * entryText = new char[size + 1];
					GetWindowText(GetDlgItem(hwnd, IDC_EDIT1), entryText, size + 1);
					min = atof(entryText);
					delete[] entryText;
					size = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT2));
					entryText = new char[size + 1];
					GetWindowText(GetDlgItem(hwnd, IDC_EDIT2), entryText, size + 1);
					max = atof(entryText);
					delete[] entryText;

				}
				else if (SendMessage(GetDlgItem(hwnd, IDC_CHECK3), BM_GETCHECK, 0, 0) == BST_UNCHECKED) {
					//no rotation needed unchecked
					int size = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT3));
					char * entryText = new char[size + 1];
					GetWindowText(GetDlgItem(hwnd, IDC_EDIT3), entryText, size + 1);
					min = atof(entryText);
					max = min;
					delete[] entryText;

				}
				else {
					data[0] = -1;
					data[1] = -7;
				}
				if (SendMessage(GetDlgItem(hwnd, IDC_CHECK3), BM_GETCHECK, 0, 0) == BST_CHECKED) {
					data[0] = -1;
					data[1] = -7;
				}
				data[0] = min;
				data[1] = max;
				CustomCursor::getInstance()->setCursor(CURSOR_NORMAL);
				notification n = { nullptr, nullptr, messages::dlg_set_limits };
				n.data1 = data;
				notify(n);
				DestroyWindow(hwnd);
			}
			else if (HIWORD(w) == EN_CHANGE) {
				int control = IDC_EDIT3;
				if ((HWND)l == GetDlgItem(hwnd, IDC_EDIT1)) {
					control = IDC_EDIT1;
				}
				else if ((HWND)l == GetDlgItem(hwnd, IDC_EDIT2)) {
					control = IDC_EDIT2;
				}
				else {
					control = IDC_EDIT3;
				}
				float theta = getWndVal(hwnd, control);
				if (control == IDC_EDIT3) {
					float decimal = getWndVal(hwnd, IDC_EDIT4);
					int places = 0;
					int dec = (int)floor(decimal);
					while (dec > 0) {
						dec /= 10;
						places++;
					}
					printf("Places %d \n", places);
					if (places != 0)
						theta += decimal / (pow(10, places));
				}

				printf("Theta: %f \n", theta);
				notification n = { nullptr, nullptr, messages::dlg_draw_lines };
				n.data1 = new Math::point{ origin.x, origin.y };
				n.data2 = new double[2];
				((double*)n.data2)[0] = theta;
				((double*)n.data2)[1] = control;
				notify(n);
			}
			else if (HIWORD(w) == BN_CLICKED) {
				if ((HWND)l == GetDlgItem(hwnd, IDC_CHECK1)) { //auto rotate
					BOOL enable = SendMessage((HWND)l, BM_GETCHECK, 0, 0);
					EnableWindow(GetDlgItem(hwnd, IDC_EDIT3), !enable);
					EnableWindow(GetDlgItem(hwnd, IDC_EDIT4), !enable);
					for (int i = IDC_CHECK2; i < IDC_EDIT3; i++)
						EnableWindow(GetDlgItem(hwnd, i), enable);
					pOrigin = !enable;
					if (pOrigin)
						CustomCursor::getInstance()->setCursor(CURSOR_DRAW);
					else
						CustomCursor::getInstance()->setCursor(CURSOR_NORMAL);
				}
				else if ((HWND)l == GetDlgItem(hwnd, IDC_CHECK3)) { //no rotation
					BOOL enable = SendMessage((HWND)l, BM_GETCHECK, 0, 0);
					for (int i = IDC_CHECK1; i <= IDC_EDIT4; i++) {
						if (i == IDC_BUTTON1 || i == IDC_CHECK3) continue;
						EnableWindow(GetDlgItem(hwnd, i), !enable);
					}
					if (enable) {
						minLine.resize(0);
						maxLine.resize(0);
						window.redraw();
					}
				}
				else if ((HWND)l == GetDlgItem(hwnd, IDC_CHECK2)) { //Manual Set Origin
					if (SendMessage((HWND)l, BM_GETCHECK, 0, 0)) {
						pOrigin = true;
						CustomCursor::getInstance()->setCursor(CURSOR_DRAW);
						MessageBox(hwnd, "Click the point on the image where you want the center of rotation", "Pick Origin", MB_OK | MB_ICONINFORMATION);
						notification n = { nullptr, nullptr, messages::dlg_pick_origin };
						notify(n);
					}
					else {
						CustomCursor::getInstance()->setCursor(CURSOR_NORMAL);
						notification n = { nullptr, nullptr, messages::dlg_geta_origin };
						notify(n);
					}
				}
			}
			break;
		}
		case WM_CLOSE:
			CustomCursor::getInstance()->setCursor(CURSOR_NORMAL);
			DestroyWindow(hwnd);
			break;
		default:
			return FALSE; //not handled
			return TRUE;
		}
	}
	RotateDialog::RotateDialog(Math::point origin, Wnd & wnd) : origin(origin), pOrigin(false), window(wnd) {}
	void RotateDialog::openDialog()
	{
		modelessDialog = window.createDialog(IDD_LOAD, DialogHelper<RotateDialog>::callback);
		ShowWindow(modelessDialog, SW_SHOW);
	}
	void RotateDialog::recv(DialogNotification n)
	{
		if (n.m == dlgm_update_origin) {
			Math::point * p = (Math::point*)n.data;
			origin.x = p->x;
			origin.y = p->y;
			//p is address of local
		}
	}
	void RotateDialog::setOrigin(const Math::point p)
	{
		origin = p;
	}
	int __stdcall SearchDialog::callback(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
	{
		switch (msg) {
		case WM_INITDIALOG:
		{
			HICON icon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON), IMAGE_ICON, 16, 16, 0);
			if (icon)
				SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
			break;
		}
		case WM_COMMAND:
			if (LOWORD(w) == IDC_FIND_WORDS) {
				HWND handle = GetDlgItem(hwnd, IDC_WORD_LIST);
				int txtSize = GetWindowTextLength(handle) + 1;
				char * buffer = new char[txtSize + 1];
				GetWindowText(handle, buffer, txtSize);
				buffer[txtSize] = '\0';
				notification n = { buffer, nullptr, messages::dlg_find_words };
				notify(n);
				EndDialog(hwnd, IDC_FIND_WORDS);
			}
			break;
		case WM_CLOSE:
			EndDialog(hwnd, 0);
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	void SearchDialog::openDialog()
	{
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_FIND), gui::GUI::useWindow(), DialogHelper<SearchDialog>::callback);
	}
}
