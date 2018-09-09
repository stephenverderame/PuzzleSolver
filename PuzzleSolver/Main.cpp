#define WINVER 0x501 
#define GUI_VISUAL_STYLES_6
#include <thread>
#include <mutex>
#include <ObjIdl.h>
#include <ctime>
#include "Grid.h"
#include "OCR.h"
#include <stack>
#include "Wnd.h"
#include "Program.h"
#include "Mediator.h"
#include "Math.h"
#include "img.h"
#include "Dialog.h"
#include <Dialog.h>
#include "Painter.h"
#include "Maze.h"
#include "Undo.h"
using namespace Program;
using namespace Notification;
using namespace Control;
using namespace Math;
using namespace DialogSpace;
using namespace CV;
using namespace Undo;

int main() {
	//int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) { 

#pragma region eventHandling

/*		else if (LOWORD(ep.getParam3()) == IDM_CROP) {
			if (img != nullptr) {
				crop = true;
				showFinal = false;
				currentCursor = CURSOR_SELECT;
			}
		}*/
		/*		else if (LOWORD(ep.getParam3()) == IDM_SOLVE_MAZE) {
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
		/*		else if (LOWORD(ep.getParam3()) == IDT_UNDO) {
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
				}*/
#pragma endregion
	MSG msg;
	Wnd window;
	Gui gui;
	GuiWndMediator mediator;
	Main mainProgram(window); //window.attach(&mainProgram)
	IMG::Img image;
	mediator.attachGui(&gui);
	mediator.attachWnd(&window);
	mediator.attatch(&window);
	mediator.attatch(&gui);
	window.attatch(&mediator);
	gui.attatch(&mediator);
	std::vector<point> line;
	std::vector<point> minLine;
	std::vector<point> maxLine;
	Painter painter(&window, &image);
	Maze maze(image);
	maze.attatch(&mainProgram);
	bool mazePointSelection = false;
	bool mazeSelectionClick = false;
	bool viewRead = false;
	bool crop = false;
	bool pickOrigin = false;
	struct {
		point start;
		point end;
	}mazeData{ {0, 0}, {0, 0} };
	point rotationOrigin;
	SearchGrid wordSearch(image);
	while (true) {
		if (gui.handleGUIEvents() == gui_msg_quit) break;
		while (!mainProgram.noMessages()) {
			notification note = mainProgram.ppopMsg();
			if (note.msg == msg_click) {
				if (mazePointSelection) {
					int x = *((int*)note.data1);
					int y = *((int*)note.data2);
					if (!maze.isSolving()) {
						if (!mazeSelectionClick) {
							image.rect({ x - 1, y - 1 }, { x + 1, y + 1 }, { 0, 255, 0 });
							mazeData.start = { x, y };
							window.drawImage(image);
							window.redraw();
							mazeSelectionClick = true;
						}
						else {
							mazeData.end = { x, y };
							image.rect({ x - 1, y - 1 }, { x + 1, y + 1 }, { 255, 0, 0 });
							window.drawImage(image);
							window.redraw();
							mazeSelectionClick = false;
							gui.showProgressBar(true);
							maze.calculatePath(mazeData.start, mazeData.end);
							Cursor::CustomCursor::getInstance()->setCursor(Cursor::CURSOR_WAIT);
						}
					}
				}
				else if (pickOrigin) {
					int x = *((int*)note.data1);
					int y = *((int*)note.data2);
					pickOrigin = false;
					Math::point p = { x, y };
					rotationOrigin = p;
					DialogNotification n = { dlgm_update_origin, &p };
					DialogHelper<RotateDialog>::send(n);
				}
			}
			switch (note.msg) {
			case dlg_draw_lines:
			{
				double theta = ((double*)note.data2)[0];
				int control = ((double*)note.data2)[1];
				point origin = *((point*)note.data1);
				for (int i = origin.x; i < Size::WndDimensions::width; i++) {
					line.push_back({ i, origin.y });
				}
				matrix rotMat = rotationMatrix(theta);
				for (int i = 0; i < line.size(); i++) {
					line[i] = { line[i].x - origin.x, line[i].y - origin.y };
					line[i] = rotMat * line[i];
					line[i] = { line[i].x + origin.x, line[i].y + origin.y };
				}
				if (control == IDC_EDIT1)
					minLine = line;
				else if (control == IDC_EDIT2)
					maxLine = line;
				window.startDrawing();
				window.canvasDraw();
				if (control == IDC_EDIT1 || control == IDC_EDIT2) {
					for (int i = 0; i < minLine.size(); i++) {
						window.setPixelColor(minLine[i], { 128, 0, 128 });

					}
					for (int i = 0; i < maxLine.size(); i++) {
						window.setPixelColor(maxLine[i], { 128, 128, 0 });
					}
				}
				else {
					for (int i = 0; i < line.size(); i++) {
						window.setPixelColor(line[i], { 255, 0, 0 });
					}
				}
				window.stopDrawing();
				line.clear();
				minLine.clear();
				maxLine.clear();
				delete (point*)note.data1;
				delete[](double*)note.data2;
				break;
			}
			case dlg_set_limits:
			{
				float * bounds = (float*)note.data1;
				Bounds b{ bounds[0], bounds[1] };
				float f;
				if (bounds[0] != bounds[1] && bounds[0] != -1 && bounds[1] != -7)
					f = findSkewAngle(image, &rotationOrigin, &b);
				else
					f = bounds[0];
				if (bounds[0] != -1 && bounds[1] != -7) {
					printf("Angle: %f \n", f);
					rotateImage(image, -f, rotationOrigin);
					window.drawImage(image);
					window.redraw();
					gui.updateScrollBar(image.width(), image.height());
				}
				delete[] bounds;
				break;
			}
			case dlg_pick_origin:
				pickOrigin = true;
				break;
			case dlg_geta_origin:
			{
				Math::point p = getOrigin(image);
				rotationOrigin = p;
				DialogNotification n = { dlgm_update_origin, &p };
				DialogHelper<RotateDialog>::send(n);
				break;
			}
			case wnd_hscroll:
			{
				events e;
				e.c = wm_std;
				e.data = note.data1;
				mediator.sendToGui(e);
				break;
			}
			case wnd_vscroll:
			{
				events e;
				e.c = wm_std;
				e.data = note.data1;
				mediator.sendToGui(e);
				break;
			}
			case wnd_view_read:
			{
				if (!image.isLoaded()) break;
				viewRead = !viewRead;
				gui.showLetterBar(viewRead);
				break;
			}
			case wnd_solve_srch:
			{
				if (!image.isLoaded()) break;
				UndoStack::getInstance()->saveState(image.getMemento());
				std::shared_ptr<SearchDialog> dialog(new SearchDialog());
				DialogHelper<SearchDialog>::setInstance(dialog);
				mainProgram.attatchDialog(dialog);
				dialog->openDialog();
				break;
			}
			case msg_undo:
				if (!UndoStack::getInstance()->isEmpty()) {
					image.loadFrom(UndoStack::getInstance()->undo());
					window.drawImage(image);
					window.redraw();
				}
				break;
			case wnd_rotate:
			{
				if (!image.isLoaded()) break;
				UndoStack::getInstance()->saveState(image.getMemento());
				point origin = getOrigin(image);
				std::shared_ptr<RotateDialog> rotate(new RotateDialog(origin, window));
				DialogHelper<RotateDialog>::setInstance(rotate);
				mainProgram.attatchDialog(rotate);
				rotate->openDialog();
				rotate->setOrigin(origin);
				rotationOrigin = origin;
				break;
			}
			case wnd_save:
			{
				if (!image.isLoaded()) break;
				std::string path = gui::saveFileDialog();
				if (path.size() > 1) {
					image.saveAsBmp(path.c_str());
				}
				break;
			}
			case wnd_crop:
				if (!image.isLoaded()) break;
				UndoStack::getInstance()->saveState(image.getMemento());
				painter.crop(true);
				window.customErase(true);
				Cursor::CustomCursor::getInstance()->setCursor(Cursor::CURSOR_SELECT);
				break;
			case wnd_monochrome:
				if (!image.isLoaded()) break;
				UndoStack::getInstance()->saveState(image.getMemento());
				image.greyscale();
				window.drawImage(image);
				window.redraw();
				break;
			case wnd_load:
			{
				gui::fileFilter f("Images", { ".bmp", ".png", ".jpg" });
				std::string path = gui::openFileDialog(&f);
				if (path.size() > 1) {
					if(image.isLoaded()) UndoStack::getInstance()->saveState(image.getMemento());
					image.loadFromPath(path.c_str());
					gui.updateScrollBar(image.width(), image.height());
					window.drawImage(image);
					window.redraw();
				}
				break;
			}
			case dlg_find_words:
			{	
				if (!image.isLoaded()) break;
				UndoStack::getInstance()->saveState(image.getMemento());
#ifndef DEBUGGING_SPACE
				wordSearch.load(image);
				char * buffer = (char*)note.data1;
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
				wordSearch.search(words);
				window.drawImage(wordSearch.getEditedImage());
				window.redraw();
#endif
				break;
			}
			case wnd_solve_maze:
			{
				if (!image.isLoaded()) break;
				UndoStack::getInstance()->saveState(image.getMemento());
				maze.attatch(&window);
				maze.choosePoint();
				break;
			}
			case mze_finish:
				Cursor::CustomCursor::getInstance()->setCursor(Cursor::CURSOR_NORMAL);
				gui.showProgressBar(false);
				window.drawImage(image);
				window.redraw();
				break;
			case mze_selecting:
				maze.loadMaze(image);
				mazePointSelection = true;
				break;
			case msg_paint:
			{
				if (viewRead) {
					point mouse = window.getMousePos();
					double minDistance = std::numeric_limits<double>::max();
					int loc = 0;
					for (int i = 0; i < wordSearch.getLocationsSize(); ++i) {
						double dist = sqrt(pow(wordSearch.getLocation(i).x + (wordSearch.getLocation(i).width / 2) - mouse.x, 2) + pow(wordSearch.getLocation(i).y + (wordSearch.getLocation(i).height / 2) - mouse.y, 2));
						if (dist < minDistance) {
							minDistance = dist;
							loc = i;						}

					}
					window.startDrawing();
					window.canvasDraw();
					Stroke s(1, { 0, 255, 0 });
					Brush b;
					window.setStroke(s);
					window.setBrush(b);
					window.drawRect({ wordSearch.getLocation(loc).x, wordSearch.getLocation(loc).y }, { wordSearch.getLocation(loc).x + wordSearch.getLocation(loc).width, wordSearch.getLocation(loc).y + wordSearch.getLocation(loc).height });
					std::string str;
					str += wordSearch.getLetter(loc)->letter;
					gui.setStatusText(str.c_str());
					window.swapBuffers();
					window.stopDrawing();
					break;
				}
			}
			case msg_mmove:
				if (viewRead)
					window.redraw();
			default:
				painter.draw(note);
				break;
			}
		}
	}
	return 0;
}