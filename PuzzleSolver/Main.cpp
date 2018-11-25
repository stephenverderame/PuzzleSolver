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
#include "NeuralNetwork.h"
#include <iterator>
#undef min
using namespace Program;
using namespace Notification;
using namespace Control;
using namespace Math;
using namespace DialogSpace;
using namespace CV;
using namespace Undo;

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

#pragma region test
/*	ML::Matrix test1(100, 1);
	test1.function([](double x) -> double {return ML::Random::getInt(0, 255); });
	ML::Matrix test2(36, 100);
	test2.function([](double x) -> double {return ML::Random::getNormal(10000); });
	ML::Matrix test3(26, 36);
	test3.function([](double x) -> double {return ML::Random::getNormal(10000); });
	(test3 * (test2 * test1).apply(ML::sigmoid)).apply(ML::sigmoid).print();
	getchar();*/
	int count = 0, correct = 0;
	double w1 = ML::Random::getNormal(10000), w2 = ML::Random::getNormal(10000), b1 = ML::Random::getNormal(10000), b2 = ML::Random::getNormal(10000);
	while (count++ < 10000) {
		/*
		 Z = a0 * w + b
		 A1 = sigmoid(Z)
		 C = (y - A1)^2

		 dC/dw = (dZ/dw)(dA/dZ)(dC/dA)
		 dC/db = (dZ/db)(dA/dZ)(dC/dA)

		 derivative of C with respect to w = derivative Z with respect to w * derivitive of A with respect to Z * derivative * derivative of C with respect to A
		 treat the variable you are taking the derivative of with respect to as x, and all others as constants or coefficients

		 dC/dA = 2(y - A1)
		 dA/dZ = sigmoid'(Z)
		 dZ/dw = a0

		 dZ/db = 1

		 Multiple Layers

		 dC/da0 = (dZ/da0)(dA/dZ)(dC/dA)
		 da0/dw0 = (dZ0/dw0)(dA0/dZ0)(da0/dA0)
		 da0/db0 = (dZ0/db0)(dA0/dZ0)(da0/dA0)
		 */
		double x = ML::Random::getNormal(10000);
		double a = ML::sigmoid(x * w1 + b1);
		double y = ML::sigmoid(a * w2 + b2);
		double real = ML::sigmoid(ML::sigmoid(x * 0.55 + 0.2) * 0.01 + 0.6);
		printf("%f --> %f ", y, real);
		if (abs(y - real) < 0.04) {
			printf("Correct!\n");
			++correct;
		}
		else printf("Wrong!\n");

//		double cost = (y - real) * (y - real);
		double dCdW2 = 2 * (y - real) * ML::sigmoidDerivitive(a * w2 + b2) * a;
		double dCdB2 = 2 * (y - real) * ML::sigmoidDerivitive(a * w2 + b2);

		double dCda0 = 2 * (y - real) * ML::sigmoidDerivitive(a * w2 + b2) * w2;
		double dCdW1 = dCda0 * ML::sigmoidDerivitive(x * w1 + b1) * x;
		double dCdB1 = dCda0 * ML::sigmoidDerivitive(x * w1 + b1);

		w1 -= dCdW1;
		b1 -= dCdB1;
		w2 -= dCdW2;
		b2 -= dCdB2;
	}
	printf("%d / %d\n", correct, count - 1);
	printf("Weight 1: %f Bias 1: %f  Weight 2: %f Bias 2: %f \n", w1, b1, w2, b2);
	getchar();
	ML::Matrix W1(2, 3), W2(1, 2), B1(2, 1), B2(1, 1);
	W1.randomize();
	W2.randomize();
	B1.randomize();
	B2.randomize();

	ML::Matrix wr1(2, 3), wr2(1, 2), br1(2, 1), br2(1, 1);
	wr1 = {0.4, 0.11, 0.99,
		   0.01, 0.78, 0.88};
	wr2 = { 0.067, 0.4 };
	br1 = { 0.978,
		   0.21 };
	br2 = { 0.23 };
	int cor = 0, trials = 100;
	for (int i = 0; i < trials; ++i) {
		ML::Matrix input(3, 1);
		input.randomize();
		ML::Matrix a = (W1 * input + B1).apply(ML::sigmoid);
		ML::Matrix y = (W2 * a + B2).apply(ML::sigmoid);
		ML::Matrix real(1, 1);
		real = (wr2 * (wr1 * input + br1).apply(ML::sigmoid) + br2).apply(ML::sigmoid);
		printf("#%d: %f --> %f ", i + 1, y.get(0), real.get(0));
		if (abs((y - real).get(0)) < 0.05) {
			++cor;
			printf("Correct!\n");
		}
		else {
			printf("Wrong!\n");
//			char buffer[10];
//			fgets(buffer, 10, stdin);
		}

		//Cost: Summation of all Elements((y - real) ^2)
		ML::Matrix dCdB2 = ML::summation(2 * (y - real)) * (ML::sigDerivative(W2 * a + B2));
		ML::Matrix dCdW2 = dCdB2 * a.transpose();

		ML::Matrix dCda0 = W2.transpose() * dCdB2;
		ML::Matrix dCdB1 = ML::summation(dCda0) * ML::sigDerivative(W1 * input + B1);
		ML::Matrix dCdW1 = dCdB1 * input.transpose();

		W1 -= dCdW1;
		W2 -= dCdW2;
		B1 -= dCdB1;
		B2 -= dCdB2;
	}
	printf("%d / %d \n", cor, trials);
	printf("W1\n");
	W1.print();
	printf("\nW2\n");
	W2.print();
	printf("\nB1\n");
	B1.print();
	printf("\nB2\n");
	B2.print();

	char buffer[10];
	fgets(buffer, 10, stdin);
	ML::NeuralNetwork seedNetwork({ 3, 5, 1 });
	seedNetwork.populate();
	count = 0;
	while (count++ < 30) {
		int iterations = 0, correctIterations = 0;
		std::ifstream in("seedTestData.txt");
		std::string inLine;
		while (std::getline(in, inLine)) {
			std::istringstream iss(inLine);
			std::vector<std::string> data(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
			ML::Matrix input(3, 1);
			input.set(0, std::stod(data[1]));
			input.set(1, std::stod(data[3]));
			input.set(2, std::stod(data[4]));
			ML::Matrix output(1, 1);
			output.set(0, std::stod(data[2]));
			ML::Matrix calc = seedNetwork.calculate(input);
			if (abs((calc - output).get(0)) / output.get(0) < 0.02) {
				++correctIterations;
			}
			++iterations;
			seedNetwork.learn(calc, output);

		}
		printf("#%d: %d / %d\n", count, correctIterations, iterations);
	}
	getchar();
	ML::NeuralNetwork net({ 100, 16, 26 });
	net.populate();
	net.train();

#pragma endregion test
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
			case messages::wnd_tranpose:
			{
				if (!image.isLoaded()) break;
				UndoStack::getInstance()->saveState(image.getMemento());
				image.transpose();
				window.drawImage(image);
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
					if (image.width() * image.height() > 1500000) {
						image.scaleByFactor(1500000.0 / (image.width() * image.height()));
						printf("Scale done\n");
					}
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
				printf("Maze solved!\n");
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