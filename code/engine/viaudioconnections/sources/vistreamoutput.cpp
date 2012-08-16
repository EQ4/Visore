#include "vistreamoutput.h"

ViStreamOutput::ViStreamOutput()
	: ViAudioOutput()
{
	mDevice = QAudioDeviceInfo::defaultOutputDevice();
	mAudioOutput = NULL;
	mVolume = 0;
	mOldLength = 0;
	mIsMute = false;
	setState(QAudio::IdleState);
}

ViStreamOutput::~ViStreamOutput()
{
	if(mAudioOutput != NULL)
	{
		delete mAudioOutput;
	}
}

void ViStreamOutput::setDevice(QAudioDeviceInfo device)
{
	mDevice = device;
}

void ViStreamOutput::setBuffer(ViAudioBuffer *buffer)
{
	if(mBuffer != NULL)
	{
		QObject::disconnect(this, SLOT(checkLength()));
	}
	ViAudioOutput::setBuffer(buffer);
	if(mBuffer != NULL)
	{
		QObject::connect(buffer, SIGNAL(changed(int)), this, SLOT(checkLength()));
	}

}

ViAudioFormat ViStreamOutput::format()
{
	if(mBuffer != NULL)
	{
		return mBuffer->format();
	}
	return ViAudioFormat();
}

void ViStreamOutput::start()
{
	if(mAudioOutput != NULL && mAudioOutput->state() == QAudio::SuspendedState)
	{
		LOG("Playback resumed.");
		mAudioOutput->resume();
	}
	else
	{
		LOG("Playback started.");
		mBufferDevice.close();
		mBufferDevice.setBuffer(mBuffer->data());
		mBufferDevice.open(QIODevice::ReadOnly);
		if(mAudioOutput != NULL)
		{
			mAudioOutput->disconnect();
			delete mAudioOutput;
		}
		mAudioOutput = new QAudioOutput(mDevice, format().toQAudioFormat(), this);
		mAudioOutput->setNotifyInterval(150);
		mOldLength = 0;
		QObject::connect(mAudioOutput, SIGNAL(notify()), this, SLOT(checkPosition()), Qt::DirectConnection);
		QObject::connect(mAudioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(checkUnderrun()));

		mAudioOutput->start(&mBufferDevice);
	}
	setState(QAudio::ActiveState);
}

void ViStreamOutput::stop()
{
	LOG("Playback stopped.");
	mBufferDevice.seek(0);
	mAudioOutput->stop();
	checkPosition();
	setState(QAudio::StoppedState);
}

void ViStreamOutput::pause()
{
	LOG("Playback paused.");
	mAudioOutput->suspend();
	setState(QAudio::SuspendedState);
}

bool ViStreamOutput::setPosition(ViAudioPosition position)
{
	mBufferDevice.seek(position.position(ViAudioPosition::Bytes));
	checkPosition();
}

bool ViStreamOutput::setPosition(int seconds)
{
	mBufferDevice.seek(ViAudioPosition::convertPosition(seconds, ViAudioPosition::Seconds, ViAudioPosition::Bytes, format()));
	checkPosition();
}

ViAudioPosition ViStreamOutput::position()
{
	if(mBuffer != NULL)
	{
		if(mState == QAudio::StoppedState)
		{
			return ViAudioPosition(0, ViAudioPosition::Microseconds, format());
		}
		return ViAudioPosition(mBufferDevice.pos(), ViAudioPosition::Bytes, format());
	}
	return ViAudioPosition();
}

void ViStreamOutput::checkPosition()
{
	ViAudioPosition pos = position();
	if(pos != mOldPosition)
	{
		mOldPosition = pos;
		emit positionChanged(pos);
	}
}

void ViStreamOutput::checkLength()
{
	if(buffer()->size() != mOldLength)
	{
		mOldLength = buffer()->size();
		emit lengthChanged(ViAudioPosition(mOldLength, ViAudioPosition::Bytes, format()));
	}
}

void ViStreamOutput::checkUnderrun()
{
	if(mAudioOutput->error() == QAudio::UnderrunError)
	{
		LOG("Audio output underrun.", QtWarningMsg);
		pause();
		emit underrun();
	}
}

qreal ViStreamOutput::volume()
{
	return mAudioOutput->volume();
}

void ViStreamOutput::setVolume(qreal volumeValue)
{
	if(mIsMute)
	{
		mMuteVolume = volume();
	}
	else
	{
		mAudioOutput->setVolume(volumeValue);
	}
}

void ViStreamOutput::mute(bool value)
{
	if(value)
	{
		mMuteVolume = volume();
		setVolume(0);
		mIsMute = true;
	}
	else
	{
		mIsMute = false;
		setVolume(mMuteVolume);
	}
}