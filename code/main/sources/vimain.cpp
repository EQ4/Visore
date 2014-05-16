#include "vimainwindow.h"
#include "viaudioengine.h"
#include "vicoder.h"
#include <QApplication>

#include<vibenchmarker2.h>
#include<vibenchmarker3.h>
#include<vibenchmarker4.h>

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	QIcon logo("logo.png");
	application.setWindowIcon(logo);
	application.setOrganizationName(ViManager::name());
	application.setOrganizationDomain(ViManager::url().toString());
	application.setApplicationName(ViManager::name());
	application.setApplicationVersion(ViManager::version().toString());

	//Register SIGNAL/SLOT parameter types
	qRegisterMetaType<ViAudioPosition>("ViAudioPosition");
	qRegisterMetaType<ViRealSpectrum>("ViRealSpectrum");

    bool BENCH = true;
    //bool BENCH = false;

	ViBenchMarker3 bench;
	if(BENCH)
	{
		bench.benchmark();
	}
	else
	{
		ViMainWindow *window = ViMainWindow::instance();
		window->setWindowIcon(logo);
		window->show();
	}
	
	application.exec();

	return 0;
}

