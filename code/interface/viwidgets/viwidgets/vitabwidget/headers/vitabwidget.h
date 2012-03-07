#ifndef VITABWIDGET_H
#define VITABWIDGET_H

#include <QPainter>
#include <QWidget>
#include <QList>
#include "vithememanager.h"
#include "vitabbutton.h"

namespace Ui
{
    class ViTabWidget;
}

class ViTabWidget : public QWidget
{
	Q_OBJECT

	private slots:
		void tabSelected(qint8 id);

	public:
		ViTabWidget(QWidget *parent = 0);
		~ViTabWidget();
		void addTab(QString title, QWidget *widget);
		void setSize(int width, int height);
		void setWidth(int width);
		void setHeight(int height);
		void selectTab(qint8 index);

	protected:
		void paintEvent(QPaintEvent *event);

	private:
		Ui::ViTabWidget *mUi;
		QList<ViTabButton*> mButtons;
		qint8 mCurrentId;
};

#endif
