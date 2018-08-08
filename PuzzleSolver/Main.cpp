#define WINVER 0x501 
#define GUI_VISUAL_STYLES_6
#include <thread>
#include <mutex>
#include <Page.h>
#include <window.h>
#include <Dialog.h>
#include "res.h"
#include <ObjIdl.h>
#include <time.h>
#include "Grid.h"
#include "OCR.h"
#include <guiMainEx.h>
#include <stack>
using namespace gui;
MainPage * mainPage;
Image * img;
Window display("Puzzle Solver");
bool crop = false, showFinal = false;
bool pOrigin = false;
bool selecting = false;
int sClicks = 0;
int clicks = 0;
POINT start, finish;
POINT position_1 = { 0, 0 }, position_2 = { 0, 0 };
clock_t lastTime = 0;
int * buffer;
std::mutex sMu;
bool solving = false;
bool invalidate = false;
std::mutex iMu;
HCURSOR cursors[4];
int currentCursor = 0;
POINT origin;
HWND ldModelessDialog;
SearchGrid read;
std::vector<Square> readLocations;
POINT mouse;
bool viewRead = false;
#define CURSOR_NORMAL 0
#define CURSOR_WAIT 1
#define CURSOR_DRAW 2
#define CURSOR_SELECT 3
std::stack<Image *> undoStack;
void calculatePath() {
	sMu.lock();
	solving = true;
	sMu.unlock();
	Grid g(img->getWidth(), img->getHeight(), buffer);
	if (position_1.x != position_2.x && position_1.y != position_2.y)
		g.setBound({ position_1.x, position_1.y }, { position_2.x, position_2.y });
	auto path = g.search({ start.x, start.y }, { finish.x, finish.y });
	while (!path.empty()) {
		pos color = path.top();
		img->setPixel(color.x, color.y, Color{ 0, 0, 255 });
		path.pop();
	}
	sMu.lock();
	solving = false;
	sMu.unlock();
	std::lock_guard<std::mutex> lock(iMu);
	invalidate = true;
}
float getWndVal(HWND parent, int control) {
	int size = GetWindowTextLength(GetDlgItem(parent, control));
	if (size == 0) return 0;
	char * txt = new char[size + 1];
	float f;
	GetWindowText(GetDlgItem(parent, control), txt, size + 1);
	printf("%s \n", txt);
//	txt[size] = '\0';
	f = atoi(txt);
	delete[] txt;
	return f;
}
void addToUndoStack(Image * img) {
	Image * oldImage = new Image(img->getWidth(), img->getHeight());
	oldImage->setPosition(img->getX(), img->getY());
	for (int i = 0; i < img->getWidth() * img->getHeight(); i++) {
		int x = i % img->getWidth();
		int y = i / img->getWidth();
		oldImage->setPixel(x, y, img->getPixel(x, y));
	}
	undoStack.push(oldImage);
}
std::vector<POINT> minLine;
std::vector<POINT> maxLine;
BOOL CALLBACK ldDialogProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
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
			float min, max;
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
			else if(SendMessage(GetDlgItem(hwnd, IDC_CHECK3), BM_GETCHECK, 0, 0) == BST_UNCHECKED){
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
			currentCursor = CURSOR_NORMAL;
			Event e(WM_COMMAND, EventParams((LPARAM)data, w));
			display.fireEvent(e);
//			EndDialog(hwnd, IDC_BUTTON1);
			DestroyWindow(hwnd);
		}
		else if (HIWORD(w) == EN_CHANGE) {
			int control;
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
			std::vector<POINT> line;
			for (int i = origin.x; i < img->getWidth(); i++) {
				line.push_back({ i, origin.y });
			}
			float rotMat[] = {
				cos(radians(theta)),	-sin(radians(theta)),
				sin(radians(theta)),	 cos(radians(theta))
			};
			for (int i = 0; i < line.size(); i++) {
				line[i] = { line[i].x - origin.x, line[i].y - origin.y };
				line[i] = matrixMultiply(rotMat, line[i]);
				line[i] = { line[i].x + origin.x, line[i].y + origin.y };
			}
			if (control == IDC_EDIT1)
				minLine = line;
			else if (control == IDC_EDIT2)
				maxLine = line;
			HDC dc = GetDC(display);
			display.getCanvas()->draw(dc);
			if (control == IDC_EDIT1 || control == IDC_EDIT2) {
				for (int i = 0; i < minLine.size(); i++) {
					SetPixel(dc, minLine[i].x, minLine[i].y, RGB(128, 0, 128));
					
				}
				for (int i = 0; i < maxLine.size(); i++) {
					SetPixel(dc, maxLine[i].x, maxLine[i].y, RGB(128, 128, 0));
				}
			}
			else {
				for (int i = 0; i < line.size(); i++) {
					SetPixel(dc, line[i].x, line[i].y, RGB(255, 0, 0));
				}
			}
			ReleaseDC(hwnd, dc);
		}
		else if (HIWORD(w) == BN_CLICKED) {
			if ((HWND)l == GetDlgItem(hwnd, IDC_CHECK1)) {
				BOOL enable = SendMessage((HWND)l, BM_GETCHECK, 0, 0);
				EnableWindow(GetDlgItem(hwnd, IDC_EDIT3), !enable);
				EnableWindow(GetDlgItem(hwnd, IDC_EDIT4), !enable);
				for (int i = IDC_CHECK2; i < IDC_EDIT3; i++)
					EnableWindow(GetDlgItem(hwnd, i), enable);
				pOrigin = !enable;
				if (pOrigin)
					currentCursor = CURSOR_DRAW;
				else
					currentCursor = CURSOR_NORMAL;
			}
			else if ((HWND)l == GetDlgItem(hwnd, IDC_CHECK3)) {
				BOOL enable = SendMessage((HWND)l, BM_GETCHECK, 0, 0);
				for (int i = IDC_CHECK1; i <= IDC_EDIT4; i++) {
					if (i == IDC_BUTTON1 || i == IDC_CHECK3) continue;
					EnableWindow(GetDlgItem(hwnd, i), !enable);
				}
				if (enable) {
					minLine.resize(0);
					maxLine.resize(0);
					RECT r;
					GetClientRect(display, &r);
					InvalidateRect(display, &r, TRUE);
				}
			}
			else if ((HWND)l == GetDlgItem(hwnd, IDC_CHECK2)) {
				if (SendMessage((HWND)l, BM_GETCHECK, 0, 0)) {
					pOrigin = true;
					currentCursor = CURSOR_DRAW;
					MessageBox(hwnd, "Click the point on the image where you want the center of rotation", "Pick Origin", MB_OK | MB_ICONINFORMATION);
				}
				else {
					currentCursor = CURSOR_NORMAL;
					origin = getOrigin(img);
					pOrigin = false;
				}
			}
		}
		break;
	}
	default:
		return FALSE; //not handled
	return TRUE;
	}
}
BOOL CALLBACK wordDialogProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
	switch (msg) {
	case WM_INITDIALOG:
	{
		HICON icon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON), IMAGE_ICON, 16, 16, 0);
		if (icon)
			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
	}
	case WM_COMMAND: 
		if (LOWORD(w) == IDC_FIND_WORDS) {
			HWND handle = GetDlgItem(hwnd, IDC_WORD_LIST);
			int txtSize = GetWindowTextLength(handle) + 1;
			char * buffer = new char[txtSize + 1];
			GetWindowText(handle, buffer, txtSize);
			buffer[txtSize] = '\0';
			display.fireEvent(Event(WM_COMMAND, EventParams((LPARAM)buffer, w)));
			EndDialog(hwnd, IDC_FIND_WORDS);
		}
	default:
		return FALSE;
	}
}
LRESULT CALLBACK customProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
	std::vector<Image *> * images = Window::boundWindow->getCanvas()->getImages();
	HDC hdc;
	PAINTSTRUCT paint;
	if (msg == WM_LBUTTONDOWN && crop) {
		lastTime = clock() / CLOCKS_PER_SEC;
		if (!clicks) {
			clicks++;
			position_1 = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
			hdc = GetDC(hwnd);
			SetPixel(hdc, position_1.x, position_1.y, RGB(255, 0, 0));
			ReleaseDC(hwnd, hdc);
		}
		else {
			crop = false;
			position_2 = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
			showFinal = true;
			currentCursor = CURSOR_NORMAL;
			clicks = 0;
			addToUndoStack(img);
			Image * buffer = new Image(abs(position_2.x - position_1.x), abs(position_2.y - position_1.y));
			for (int x = 0; x < buffer->getWidth(); x++) {
				for (int y = 0; y < buffer->getHeight(); y++) {
					buffer->setPixel(x, y, img->getPixel(min(position_1.x, position_2.x) + x, min(position_1.y, position_2.y) + y));
				}
			}
			display.getCanvas()->removeImage(img);
			delete img;
			img = buffer;
			display.getCanvas()->addImage(img);
			position_1 = { 0, 0 };
			position_2 = { 0, 0 };
		}
	}
	else if (msg == WM_KEYDOWN && w == 0x55) {
		if (GetAsyncKeyState(VK_CONTROL) < 0)
			display.fireEvent(Event(WM_COMMAND, EventParams(0, IDT_UNDO)));
	}
	else if (msg == WM_RBUTTONDOWN && (crop || showFinal)) {
		crop = false;
		clicks = 0;
		showFinal = false;
		RECT r;
		GetClientRect(hwnd, &r);
		InvalidateRect(hwnd, &r, TRUE);
		currentCursor = CURSOR_NORMAL;
		position_1 = { 0, 0 };
		position_2 = { 0, 0 };
	}
	else if (msg == WM_LBUTTONDOWN && selecting) {
		sMu.lock();
		bool s = solving;
		sMu.unlock();
		if (!s) {
			if (!sClicks) {
				start = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
				for (int x = -1; x <= 1; x++) {
					for (int y = -1; y <= 1; y++) {
						img->setPixel(start.x + x, start.y + y, Color{ 0, 255, 0 });
					}
				}
				sClicks++;
			}
			else {
				finish = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
				for (int x = -1; x <= 1; x++) {
					for (int y = -1; y <= 1; y++) {
						img->setPixel(finish.x + x, finish.y + y, Color{ 255, 0, 0 });
					}
				}
				Progressbar * p = (Progressbar*)mainPage->getCurrentPage()->getControl("progress");
				p->showComponent();
				sClicks = 0;
				selecting = false;
				std::thread t(calculatePath);
				t.detach();
				currentCursor = CURSOR_WAIT;

			}
			RECT r;
			GetClientRect(hwnd, &r);
			InvalidateRect(hwnd, &r, TRUE);
		}
	}
	else if (msg == WM_LBUTTONDOWN && pOrigin) {
		origin = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };

	}
	if (msg == WM_ERASEBKGND && crop) return TRUE;
	if (msg == WM_PAINT && clicks > 0) {
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		RECT r;
		GetClientRect(hwnd, &r);

		hdc = BeginPaint(hwnd, &paint);

		HDC hdcMem = CreateCompatibleDC(paint.hdc);
		HBITMAP hbmMem = CreateCompatibleBitmap(paint.hdc, r.right - r.left, r.bottom - r.top);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

		FillRect(hdcMem, &r, (HBRUSH)COLOR_WINDOW);
		if (Window::boundWindow != nullptr)
			Window::boundWindow->getCanvas()->draw(hdcMem);
		HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
		SelectObject(hdcMem, GetStockBrush(NULL_BRUSH));
		SelectObject(hdcMem, pen);
		Rectangle(hdcMem, min(position_1.x, p.x), min(position_1.y, p.y), max(position_1.x, p.x), max(position_1.y, p.y));
		DeletePen(pen);

		BitBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, hbmOld);
		DeleteObject(hbmMem);
		DeleteDC(hdcMem);

		EndPaint(hwnd, &paint);
		return TRUE;
	}
	if (msg == WM_PAINT && viewRead) {
		std::pair<int, int> dims = read.getDimensions();
		double minDistance = DBL_MAX;
		int loc = 0;
		for (int i = 0; i < (dims.first + 1) * (dims.second + 1); i++) {
			double dist = sqrt(pow(readLocations[i].x + (readLocations[i].width / 2) - mouse.x, 2) + pow(readLocations[i].y + (readLocations[i].height / 2) - mouse.y, 2));
			if (dist < minDistance) {
				minDistance = dist;
				loc = i;
			}
		}
		PAINTSTRUCT paint;
		HDC dc = BeginPaint(display, &paint);
		display.getCanvas()->draw(dc);
		HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
		HPEN oldPen = (HPEN)SelectObject(GetDC(display), pen);
		HBRUSH oldBrush = (HBRUSH)SelectObject(GetDC(display), GetStockBrush(NULL_BRUSH));
		int y = readLocations[loc].y + readLocations[loc].height;
		MoveToEx(dc, readLocations[loc].x, y, NULL);
		LineTo(dc, readLocations[loc].x, y + readLocations[loc].height);
		LineTo(dc, readLocations[loc].x + readLocations[loc].width, y + readLocations[loc].height);
		LineTo(dc, readLocations[loc].x + readLocations[loc].width, y);
		LineTo(dc, readLocations[loc].x, y);
		SelectObject(dc, oldBrush);
		SelectObject(dc, oldPen);
		DeleteObject(pen);
		SwapBuffers(dc);
		EndPaint(display, &paint);
		Statusbar * status = (Statusbar*)mainPage->getCurrentPage()->getControl("letterBar");
		std::string text = "";
		text += read.getLetter(loc)->letter;
		status->setText(text.c_str(), 0);
	}
	if (msg == WM_MOUSEMOVE && crop) {
		RECT r;
		GetClientRect(display, &r);
		InvalidateRect(display, &r, FALSE);
	}
	else if (msg == WM_MOUSEMOVE && viewRead) {
		mouse = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
		RECT r;
		GetClientRect(display, &r);
		InvalidateRect(display, &r, TRUE);
	}
	return 0;
}
int main(){
//int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	display.use();
	WNDCLASSEX update;
	update.lpszMenuName = MAKEINTRESOURCE(ID_MENU);
	update.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON));
	update.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON), IMAGE_ICON, 16, 16, 0);
	display.setWindowProperty(menuName, update);
	display.setWindowProperty(icon, update);
	display.createWindow(300, 300, 800, 800);
	GUI::bindWindow(display);
	int sections[] = { 100, -1 };
	Page * p1 = new Page("Page 1", {
		new Scrollbar("testScroll", 10, display.getDimensions().height - 20, display.getDimensions().width - 20, 10, 100, scrollHorz, SBS_HORZ),
		new Scrollbar("scrollVert", display.getDimensions().width - 10, 0, 10, display.getDimensions().height, 100, scrollVert, SBS_VERT),
		new Progressbar("progress", 10, display.getDimensions().height - 20, display.getDimensions().width - 20, 20, 0, 100),
		new Toolbar("toolbar", {new ToolbarControls{IDT_UNDO, gui::tbm_undo}}),
		new Statusbar("letterBar", 1, sections)
	});
	p1->setLayout(new AbsoluteLayout());
	mainPage = new MainPage({
		p1
	});
	mainPage->navigateTo("Page 1");
	Progressbar * p = (Progressbar*)mainPage->getCurrentPage()->getControl("progress");
	p->setMarquee(true);
	p->hideComponent();
	Toolbar * t = (Toolbar*)p1->getControl("toolbar");
	p1->getControl("letterBar")->hideComponent();
#pragma region eventHandling
	Scrollbar * scroll = (Scrollbar*)mainPage->getCurrentPage()->getControl("testScroll");
	scroll->setScrollMessages([](int changePos) {
		for (auto it = display.getCanvas()->getImages()->begin(); it != display.getCanvas()->getImages()->end(); it++) {
			(*it)->setPosition((*it)->getX() - changePos, (*it)->getY());
		}
	});
	scroll = (Scrollbar*)mainPage->getCurrentPage()->getControl("scrollVert");
	scroll->setScrollMessages(nullptr, [](int changePos) {
		for (auto it = display.getCanvas()->getImages()->begin(); it != display.getCanvas()->getImages()->end(); it++) {
			(*it)->setPosition((*it)->getX(), (*it)->getY() - changePos);
		}
	});
	display.addEventListener(new EventListener([](EventParams ep) { 
		MSG m = Event(WM_SIZE, ep).toMsg();
		mainPage->handleMessage(&m);
	}, WM_SIZE));
	display.addEventListener(new EventListener([](EventParams ep) {
		if (LOWORD(ep.getParam3()) == IDM_LOAD) {
			fileFilter f("Images", { ".bmp", ".jpg", ".png" });
			std::string path = openFileDialog(&f);
			if (path.size() > 1) {
				if (img != nullptr) {
					addToUndoStack(img);
					display.deleteImage(img);
					delete img;
				}
				img = new Image(path.c_str(), 0, 20);
				display.drawImage(img);
				//				img->toMonochrome();
				//				img->toGreyscale();
				Scrollbar * scroll = (Scrollbar*)mainPage->getCurrentPage()->getControl("testScroll");
				scroll->update(max(img->getWidth() - display.getDimensions().width, 2));
				scroll = (Scrollbar*)mainPage->getCurrentPage()->getControl("scrollVert");
				scroll->update(max(img->getHeight() - display.getDimensions().height, 2));
			}
		}
		else if (LOWORD(ep.getParam3()) == IDM_MONOCHROME) {
			if (img != nullptr) {
				img->toMonochrome();
				RECT r;
				GetClientRect(display, &r);
				InvalidateRect(display, &r, TRUE);
			}
		}
		else if (LOWORD(ep.getParam3()) == IDM_SAVE) {
			if (img != nullptr) {
				std::string path = saveFileDialog();
				if (path.size() > 1) {
					img->saveBmp(path.c_str());
				}
			}
		}
		else if (LOWORD(ep.getParam3()) == IDM_CROP) {
			if (img != nullptr) {
				crop = true;
				showFinal = false;
				currentCursor = CURSOR_SELECT;
			}
		}
		else if (LOWORD(ep.getParam3()) == IDM_ROTATE) {
			if (img != nullptr) {
				printf("Rotate! \n");
				addToUndoStack(img);
				origin = getOrigin(img);
				//				int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LOAD), display, ldDialogProc);
				ldModelessDialog = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LOAD), display, ldDialogProc);
				ShowWindow(ldModelessDialog, SW_SHOW);
			}
		}
		else if (LOWORD(ep.getParam3()) == IDM_SOLVE_MAZE) {
			if (img != nullptr) {
				sMu.lock();
				bool s = solving;
				sMu.unlock();
				if (!s) {
					if (buffer != nullptr) delete buffer;
					buffer = new int[img->getHeight() * img->getWidth()];
					for (int i = 0; i < img->getHeight() * img->getWidth(); i++) {
						int x = i % img->getWidth();
						int y = i / img->getWidth();
						Color c = img->getPixel(x, y);
						buffer[i] = (255 * 255 * 3) - ((int)c.r * c.r + (int)c.g * c.g + (int)c.b * c.b);
					}
					selecting = true;
					currentCursor = CURSOR_DRAW;
				}
			}
		}
		else if (LOWORD(ep.getParam3()) == IDC_BUTTON1) {
			float * bounds = (float*)ep.getParaml();
			Bounds b{ bounds[0], bounds[1] };
			float f;
			if (bounds[0] != bounds[1] && bounds[0] != -1 && bounds[1] != -7)
				f = findSkewAngle(img, &origin, &b);
			else
				f = bounds[0];
			if (bounds[0] != -1 && bounds[1] != -7) {
				printf("Angle: %f \n", f);
				rotateImage(img, -f, origin);
				Scrollbar * scroll = (Scrollbar*)mainPage->getCurrentPage()->getControl("testScroll");
				scroll->update(max(img->getWidth() - display.getDimensions().width, 2));
				scroll = (Scrollbar*)mainPage->getCurrentPage()->getControl("scrollVert");
				scroll->update(max(img->getHeight() - display.getDimensions().height, 2));
			}
			delete[] bounds;
		}
		else if (LOWORD(ep.getParam3()) == IDT_UNDO) {
			if (!undoStack.empty()) {
				display.getCanvas()->removeImage(img);
				delete img;
				img = undoStack.top();
				undoStack.pop();
				display.getCanvas()->addImage(img);
				RECT r;
				GetClientRect(display, &r);
				InvalidateRect(display, &r, TRUE);
			}
		}
		else if (LOWORD(ep.getParam3()) == IDM_SOLVE_SEARCH) {
			if (img != nullptr) {
				addToUndoStack(img);
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_FIND), display, wordDialogProc);
			}
		}
		else if (LOWORD(ep.getParam3()) == IDM_VIEW_READ) {
			if (img != nullptr) {
				viewRead = !viewRead;
				if (viewRead)
					mainPage->getCurrentPage()->getControl("letterBar")->showComponent();
				else
					mainPage->getCurrentPage()->getControl("letterBar")->hideComponent();
			}
		}
		else if (LOWORD(ep.getParam3()) == IDC_FIND_WORDS) {
			readLocations = getCharacterLocations(img);
			//augmentDataSet(readLocations, { 'C', 'A', 'T', 'S', 'L', 'B', 'H', 'O', 'L', 'R', 'P', 'R', 'E', 'A', 'L', 'L', 'L', 'B', 'U', 'N', 'C', 'K', 'S', 'I' }, img);
			//				augmentDataSet(list, {'T', 'N', 'E', 'R', 'R', 'U', 'C', 'M', 'M', 'A', 'G', 'N', 'E', 'T', 'I', 'S', 'M', 'P'}, img);
			//				augmentDataSet(list, { 'D', 'N', 'T', 'I', 'G', 'C', 'A', 'J', 'C', 'A', 'O', 'J', 'O', 'H', 'S', 'I', 'E', 'I', 'B', 'A' }, img);
			//				augmentDataSet(list, { 'H', 'M', 'E', 'G', 'Q', 'T', 'Z', 'M', 'R', 'H', 'Y', 'L', 'S', 'R' }, img);
#ifndef DEBUGGING_SPACE
			read.copyFrom(identifyLetters(img, readLocations));
			char * buffer = (char*)ep.getParaml();
			std::string word;
			std::vector<std::string> words;
			printf("Words: \n");
			char c = buffer[0];
			int i = 0;
			int lastDelim = -1;
			for (i = 0; true; i++) {
				if (buffer[i] == '\r' || buffer[i] == '\0' || buffer[i] == ';') {
					char * wd = new char[i - lastDelim];
					memcpy_s(wd, i - lastDelim - 1, buffer + lastDelim + 1, i - lastDelim - 1);
					wd[i - lastDelim - 1] = '\0';
					words.push_back(std::string(wd));
					printf("%s ", wd);
					lastDelim = i;
					while (buffer[lastDelim + 1] == '\r' || buffer[lastDelim + 1] == '\n' || buffer[lastDelim + 1] == ';' || buffer[lastDelim + 1] == ' ') lastDelim++;
					i = lastDelim;
					delete[] wd;
				}
				if (buffer[i] == '\0') break;
			}
			printf("\n");
			delete[] buffer;
			read.search(img, readLocations, words);
#endif
		}
	}, WM_COMMAND));
	display.addEventListener(new EventListener([](EventParams ep) {
		MSG m = Event(WM_HSCROLL, ep).toMsg();
		m.hwnd = GUI::useWindow();
		mainPage->handleMessage(&m);
	}, WM_HSCROLL));
	display.addEventListener(new EventListener([](EventParams ep) {
		MSG m = Event(WM_VSCROLL, ep).toMsg();
		m.hwnd = GUI::useWindow();
		mainPage->handleMessage(&m);
	}, WM_VSCROLL));
	display.setWindowProc(customProc);
	cursors[CURSOR_NORMAL] = LoadCursor(NULL, IDC_ARROW);
	cursors[CURSOR_WAIT] = LoadCursor(NULL, IDC_WAIT);
	cursors[CURSOR_DRAW] = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_CURSOR_PEN));
	cursors[CURSOR_SELECT] = LoadCursor(NULL, IDC_CROSS);
	display.addEventListener(new EventListener([](EventParams ep) {
		SetCursor(cursors[currentCursor]);
	}, WM_SETCURSOR));
#pragma endregion
	MSG msg;
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) break;
			mainPage->handleMessage(&msg);
		}
		std::lock_guard<std::mutex> lock(iMu);
		if (invalidate) {
			currentCursor = CURSOR_NORMAL;
			Progressbar * pr = (Progressbar*)mainPage->getCurrentPage()->getControl("progress");
			pr->hideComponent();
			RECT r;
			GetClientRect(display, &r);
			InvalidateRect(display, &r, TRUE);
			invalidate = false;
			position_1 = { 0, 0 };
			position_2 = { 0, 0 };
		}
	}
	GUI_CLEANUP(mainPage);
	return 0;
}