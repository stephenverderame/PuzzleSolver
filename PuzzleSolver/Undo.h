#pragma once
#include "img.h"
#include <stack>
#include <memory>
namespace Undo {
	/**
	 * Singleton handling saved Memento states
	*/
	class UndoStack {
	private:
		std::stack<IMG::ImgMemento> undoStack;
		static std::shared_ptr<UndoStack> instance;
	public:
		/**
		 * Pushes memento onto the Undo stack
		*/
		void saveState(IMG::ImgMemento & memento);
		bool isEmpty();
		/**
		 * Pops a memento off the undo stack. Does nothing if stack is empty
		 * @return memento to be loaded by image
		*/
		IMG::ImgMemento undo();
		static std::shared_ptr<UndoStack> getInstance() { return instance; }
	};
}