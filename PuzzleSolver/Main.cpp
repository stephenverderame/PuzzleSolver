#define WINVER 0x501 
#define GUI_VISUAL_STYLES_6
#include <thread>
#include "Grid.h"
#include "OCR.h"
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
#undef min
using namespace Program;
using namespace Notification;
using namespace Control;
using namespace Math;
using namespace DialogSpace;
using namespace CV;
using namespace Undo;

bool inline notSeen(std::vector<point> & vec, const int x, const int y) {
	return std::find(vec.cbegin(), vec.cend(), point{ x, y }) == vec.cend();
}
void inline printVector(std::vector<int> & v) {
	for (auto e : v)
		printf("%d ", e);
	printf("\n");
}
int main() {
	//int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) { 
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
	painter.attatch(&mainProgram);
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
			if (note.msg == messages::msg_click) {
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
			case messages::dlg_draw_lines:
			{
				double theta = ((double*)note.data2)[0];
				int control = ((double*)note.data2)[1];
				point origin = *((point*)note.data1);
				for (int i = origin.x; i < Size::WndDimensions::width; i++) {
					line.push_back({ i, origin.y });
				}
				matrix rotMat = rotationMatrix(theta);
				for (int i = 0; i < line.size(); i++) {
					line[i] = line[i] - origin;
					line[i] = rotMat * line[i];
					line[i] = line[i] + origin;
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
			case messages::dlg_set_limits:
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
			case messages::dlg_pick_origin:
				pickOrigin = true;
				break;
			case messages::dlg_geta_origin:
			{
				Math::point p = getOrigin(image);
				rotationOrigin = p;
				DialogNotification n = { dlgm_update_origin, &p };
				DialogHelper<RotateDialog>::send(n);
				break;
			}
			case messages::wnd_hscroll:
			{
				events e;
				e.c = wm_std;
				e.data = note.data1;
				mediator.sendToGui(e);
				break;
			}
			case messages::wnd_vscroll:
			{
				events e;
				e.c = wm_std;
				e.data = note.data1;
				mediator.sendToGui(e);
				break;
			}
			case messages::wnd_view_read:
			{
				if (!image.isLoaded()) break;
				viewRead = !viewRead;
				gui.showLetterBar(viewRead);
				break;
			}
			case messages::wnd_solve_srch:
			{
				if (!image.isLoaded()) break;
				UndoStack::getInstance()->saveState(image.getMemento());
				std::shared_ptr<SearchDialog> dialog(new SearchDialog());
				DialogHelper<SearchDialog>::setInstance(dialog);
				mainProgram.attatchDialog(dialog);
				dialog->openDialog();
				break;
			}
			case messages::msg_undo:
				if (!UndoStack::getInstance()->isEmpty()) {
					image.loadFrom(UndoStack::getInstance()->undo());
					window.drawImage(image);
					window.redraw();
				}
				break;
			case messages::wnd_rotate:
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
			case messages::wnd_save:
			{
				if (!image.isLoaded()) break;
				std::string path = gui::saveFileDialog();
				if (path.size() > 1) {
					image.saveAsBmp(path.c_str());
				}
				break;
			}
			case messages::wnd_crop:
				if (!image.isLoaded()) break;
				UndoStack::getInstance()->saveState(image.getMemento());
				painter.crop(true);
				window.customErase(true);
				Cursor::CustomCursor::getInstance()->setCursor(Cursor::CURSOR_SELECT);
				break;
			case messages::pai_crop:
			{
				IMG::ImgMemento * mem = (IMG::ImgMemento*)note.data1;
				image.loadFrom(*mem);
				window.drawImage(image);
				window.redraw();
				delete mem;
				break;
			}
			case messages::wnd_monochrome:
			{
				if (!image.isLoaded()) break;
				//testing 
				UndoStack::getInstance()->saveState(image.getMemento());
				Kernel guassian({
					1,  4,  6,  4, 1,
					4, 16, 24, 16, 4,
					6, 24, 36, 24, 6,
					4, 16, 24, 16, 4,
					1,  4,  6,  4, 1
				});
				guassian.scale(1 / 256.0);
				image.trueGrayscale(std::make_unique<IMG::LumFunc>());
//				guassian.kernelConvolution(image);
				int avg = (image.integralImageValue(image.width() - 1, image.height() - 1) - image.integralImageValue(image.width() - 1, 0) - image.integralImageValue(0, image.height() - 1) + image.integralImageValue(0, 0)) / (image.width() * image.height());
				avg = 255 - avg;
				auto img = cannyEdgeDetection(image);
				Hough hough;
				hough.transform(*img);
				avg *= 1.7;
				printf("AVG: %d \n", avg);
				auto list = hough.getLines(avg);
				printf("%d lines found!\n", list.size());
				decltype(list) newList;
				for (decltype(auto) line : list) {
					auto d = line.second - line.first;
					if ((abs(d.y) < img->height() * 0.05 && abs(d.x) > img->width() * 0.95) || (abs(d.x) < img->width() * 0.05 && abs(d.y) > img->height() * 0.95)) {
						bool add = true;
//						for (int i = 0; i < newList.size(); ++i)
//							if (isCloseTo(Line{ newList[i].first, newList[i].second }, Line{ line.first, line.second }, 10)) { add = false; break; }
//						if (add) {
//							newList.push_back(line);
							img->drawLine(line.first, line.second, { 255, 0, 0 });
//						}
					}
				}
/*				image.drawRect({ 200, 200 }, { 400, 400 }, { 255, 0, 0 });
				image.drawLine({ 300, 0 }, { 500, 500 }, { 255, 0, 0 });
				printf("Collision: %d\n", Math::lineRectIntersection({ 300, 0 }, { 500, 500 }, { 200, 200 }, { 400, 400 }));*/
				window.drawImage(*img);
				image = *img;
				window.redraw();
				break;
			}
			case messages::wnd_load:
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
			case messages::dlg_find_words:
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
			case messages::wnd_solve_maze:
			{
				if (!image.isLoaded()) break;
				UndoStack::getInstance()->saveState(image.getMemento());
				maze.attatch(&window);
				maze.choosePoint();
				break;
			}
			case messages::mze_finish:
				Cursor::CustomCursor::getInstance()->setCursor(Cursor::CURSOR_NORMAL);
				gui.showProgressBar(false);
				window.drawImage(image);
				window.redraw();
				break;
			case messages::mze_selecting:
				maze.loadMaze(image);
				mazePointSelection = true;
				break;
			case messages::msg_paint:
			{
				if (viewRead) {
					point mouse = window.getMousePos();
					window.startDrawing();
					window.canvasDraw();
					Stroke s(1, { 0, 255, 0 });
					window.setStroke(s);
					window.setBrush(null_brush);
					auto letter = wordSearch.getLetterNearest(mouse);
					window.drawRect({ letter.second.x, letter.second.y }, { letter.second.x + letter.second.width, letter.second.y + letter.second.height });
					std::string str;
					str += letter.first;
					gui.setStatusText(str.c_str());
					window.swapBuffers();
					window.stopDrawing();
					break;
				}
			}
			case messages::msg_mmove:
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