#include "Undo.h"
std::shared_ptr<Undo::UndoStack> Undo::UndoStack::instance(new Undo::UndoStack());

void Undo::UndoStack::saveState(IMG::ImgMemento & m)
{
	undoStack.push(m);
}

bool Undo::UndoStack::isEmpty()
{
	return undoStack.empty();
}

IMG::ImgMemento Undo::UndoStack::undo()
{
	if (!undoStack.empty()) {
		IMG::ImgMemento temp = undoStack.top();
		undoStack.pop();
		return temp;
	}
}
