#include "Observer.h"

void Notification::Subject::notify(const notification m)
{
	for (auto o : observers)
		o->update(m);
}

void Notification::Subject::attatch(Observer * o)
{
	observers.push_back(o);
}
