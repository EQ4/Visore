#include "vivolumedialwidget.h"
#include "ui_vivolumedialwidget.h"

ViVolumeDialWidget::ViVolumeDialWidget(QWidget *parent)
	: ViWidget(parent)
{
	mUi = new Ui::ViVolumeDialWidget();
	mUi->setupUi(this);
	ViObject::connect(mUi->volumeDial, SIGNAL(valueChanged(int)), mEngine, SLOT(setVolume(int)));
	ViObject::connect(mUi->volumeDial, SIGNAL(valueChanged(int)), this, SLOT(volumeChanged(int)));
	volumeChanged();
	mUi->volumeDial->setSize(80, 80);
	mUi->muteButton->setCheckable(true);
	mUi->muteButton->setIcon(ViThemeManager::image("unmute.png", ViThemeImage::Normal, ViThemeManager::Icon), ViThemeImage::Normal);
	mUi->muteButton->setIcon(ViThemeManager::image("mute.png", ViThemeImage::Normal, ViThemeManager::Icon), ViThemeImage::Selected);
	setMinimumSize(100, 120);
	setMaximumSize(100, 120);
}

ViVolumeDialWidget::~ViVolumeDialWidget()
{
	delete mUi;
}

void ViVolumeDialWidget::volumeChanged(int volume)
{
	volume = mEngine->volume();
	mUi->volumeDial->setValue(volume);
	QString text = QString::number(volume)+"%";
	if(volume < 100)
	{
		text = " " + text;
	}
	if(volume < 10)
	{
		text = " " + text;
	}
	mUi->volumeLabel->setText(text);
}