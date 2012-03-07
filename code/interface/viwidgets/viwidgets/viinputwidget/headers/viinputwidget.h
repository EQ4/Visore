#ifndef VIINPUTWIDGET_H
#define VIINPUTWIDGET_H

#include "vithememanager.h"
#include "viwidget.h"

namespace Ui
{
    class ViInputWidget;
}

class ViInputWidget : public ViWidget
{
	Q_OBJECT

	private slots:
		void selectLineInput();
		void selectFileInput();

	public:
		ViInputWidget(QWidget *parent = 0);
		~ViInputWidget();
		void setEngine(ViAudioEngine *engine);

	private:
		Ui::ViInputWidget *mUi;
};

#endif
