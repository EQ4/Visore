#include "viaudiotransmission.h"

ViAudioTransmission::ViAudioTransmission()
{
	mBuffer = NULL;
	mState = QAudio::IdleState;
}

ViAudioTransmission::~ViAudioTransmission()
{
}

void ViAudioTransmission::setBuffer(ViBuffer *buffer)
{
	mBuffer = buffer;
	emit bufferChanged();
}

ViBuffer* ViAudioTransmission::buffer()
{
	return mBuffer;
}

ViAudioFormat ViAudioTransmission::format()
{
}

void ViAudioTransmission::setFormat(ViAudioFormat format)
{
}

void ViAudioTransmission::setState(QAudio::State state)
{
	if((mState == QAudio::ActiveState || mState == QAudio::SuspendedState) && (state == QAudio::IdleState || state == QAudio::StoppedState))
	{
		emit finished();
	}
	else if((mState == QAudio::IdleState || mState == QAudio::StoppedState) && state == QAudio::ActiveState)
	{
		emit started();
	}
	mState = state;
	emit stateChanged(mState);
}

QAudio::State ViAudioTransmission::state()
{
	return mState;
}
