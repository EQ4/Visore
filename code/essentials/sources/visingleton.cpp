#include "visingleton.h"

ViSingleton::ViSingleton()
{
	mName = "";
}

ViSingleton* ViSingleton::instance()
{
	return NULL;
}

QString ViSingleton::name()
{
	return mName;
}
