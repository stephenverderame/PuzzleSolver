#pragma once
#include "img.h"
#include <stack>
#include <memory>
namespace Undo {
	class UndoStack {
	private:
		std::stack<IMG::ImgMemento> undoStack;
		static std::shared_ptr<UndoStack> instance;
	public:
		void saveState(IMG::ImgMemento & memento);
		bool isEmpty();
		IMG::ImgMemento undo();
		static std::shared_ptr<UndoStack> getInstance() { return instance; }
	};
}