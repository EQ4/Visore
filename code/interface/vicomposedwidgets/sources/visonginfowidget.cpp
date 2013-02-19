#include "visonginfowidget.h"
#include "ui_visonginfowidget.h"

#include "vithememanager.h"
#include "viratingwidget.h"
#include <QtCore/qmath.h>
#include <QPixmap>

#define INFO_CHANGE_TIMEOUT 5000

ViSongInfoWidget::ViSongInfoWidget(QWidget *parent)
	: ViWidget(parent)
{
	mUi = new Ui::ViSongInfoWidget();
	mUi->setupUi(this);

	setBorder(ViThemeManager::color(ViThemeColors::BorderColor1), 5, 10);

	//mUi->button->setIcon(ViThemeManager::image("", ViThemeImage::Selected, ViThemeManager::Icon), ViThemeImage::Selected);
	mUi->button->setCheckable(false);
	mUi->button->setGlow(ViThemeManager::color(ViThemeColors::ButtonGlowColor1));
	mUi->button->setSize(20, 20);
	QObject::connect(mUi->button, SIGNAL(clicked()), this, SLOT(switchInfo()));
	mTimer = new QTimer(this);
	QObject::connect(mTimer, SIGNAL(timeout()), this, SLOT(switchInfo()));
	mTimer->start(INFO_CHANGE_TIMEOUT);

	QObject::connect(engine().data(), SIGNAL(songDetected(ViSongInfo)), this, SLOT(changeSongInfo(ViSongInfo)));
	QObject::connect(engine().data(), SIGNAL(songEnded()), this, SLOT(clearSongInfo()));
}

ViSongInfoWidget::~ViSongInfoWidget()
{
	delete mUi;
	delete mTimer;
}

void ViSongInfoWidget::changeSongInfo(ViSongInfo info)
{
	mUi->artistLabel->setText(info.artistName());
	mUi->titleLabel->setText(info.songTitle());

	mUi->artistHotnessWidget->setRating(info.artistHotness());
	mUi->artistFamiliarityWidget->setRating(info.artistFamiliarity());
	mUi->songHotnessWidget->setRating(info.songHotness());
	mUi->songDanceabilityWidget->setRating(info.songDanceability());
	mUi->songEnergyWidget->setRating(info.songEnergy());

	QString duration = QString::number(qFloor(info.songDuration() / 60)) + ":" + QString::number(int(info.songDuration()) % 60);
	mUi->durationLabel->setText(duration);
	mUi->loudnessLabel->setText(QString::number(info.songLoudness(), 'f', 1) + " dB");
	mUi->tempoLabel->setText(QString::number(info.songTempo(), 'f', 1) + " bpm");

	QPixmap cover(info.imagePath());
	if(!cover.isNull())
	{
		cover = cover.scaled(mUi->coverPicture->width(), mUi->coverPicture->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		mUi->coverPicture->setPixmap(cover);
	}
}

void ViSongInfoWidget::clearSongInfo()
{
	mUi->artistLabel->setText("");
	mUi->titleLabel->setText("");

	mUi->artistHotnessWidget->setRating(0);
	mUi->artistFamiliarityWidget->setRating(0);
	mUi->songHotnessWidget->setRating(0);
	mUi->songDanceabilityWidget->setRating(0);
	mUi->songEnergyWidget->setRating(0);

	mUi->durationLabel->setText("");
	mUi->loudnessLabel->setText("");
	mUi->tempoLabel->setText("");

	mUi->coverPicture->setPixmap(QPixmap());
}

void ViSongInfoWidget::switchInfo()
{
	mTimer->start(INFO_CHANGE_TIMEOUT);
	int currentInfo = mUi->stackedWidget->currentIndex() + 1;
	if(currentInfo >= mUi->stackedWidget->count())
	{
		currentInfo = 0;
	}
	mUi->stackedWidget->setCurrentIndex(currentInfo);
}
