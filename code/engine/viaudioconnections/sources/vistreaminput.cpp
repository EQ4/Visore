#include "vistreaminput.h"

ViStreamBuffer::ViStreamBuffer()
	: QBuffer()
{
}

void ViStreamBuffer::setBuffer(ViBuffer *buffer)
{
	mStream = buffer->createWriteStream();
}

qint64 ViStreamBuffer::write(const char *data, qint64 maxSize)
{
	return mStream->write((char*) data, maxSize);
}

qint64 ViStreamBuffer::write(const char *data)
{
	return mStream->write((char*) data, qstrlen(data));
}

qint64 ViStreamBuffer::write(const QByteArray &byteArray)
{
	return mStream->write((char*) byteArray.data(), byteArray.size());
}

qint64 ViStreamBuffer::writeData(const char *data, qint64 length)
{
	return mStream->write((char*) data, length);
}

ViStreamInput::ViStreamInput()
	: ViAudioInput()
{
	mDevice = QAudioDeviceInfo::defaultInputDevice();
	mAudioInput = NULL;
	setState(QAudio::IdleState);
timer = new QTimer(this);
 connect(timer, SIGNAL(timeout()), this, SLOT(tu()));
     
}

ViStreamInput::~ViStreamInput()
{
	if(mAudioInput != NULL)
	{
		delete mAudioInput;
	}
}

void ViStreamInput::setDevice(QAudioDeviceInfo device)
{
	mDevice = device;
}

void ViStreamInput::setBuffer(ViBuffer *buffer)
{
	ViAudioInput::setBuffer(buffer);
	mBuffer->setFormat(mFormat);
	mBufferDevice.close();
	mBufferDevice.setBuffer(mBuffer);
	mBufferDevice.open(QIODevice::WriteOnly);
}

ViAudioFormat ViStreamInput::format()
{
	return mFormat;
}

void ViStreamInput::setFormat(ViAudioFormat format)
{
	mFormat = format;
	if(mBuffer != NULL)
	{
		mBuffer->setFormat(mFormat);
	}
	if(mAudioInput != NULL)
	{
		delete mAudioInput;
	}
	mAudioInput = new QAudioInput(mDevice, mFormat.toQAudioFormat(), this);
}

void ViStreamInput::start()
{
	if(mAudioInput->state() == QAudio::SuspendedState)
	{
		LOG("Recording resumed.");
		mAudioInput->resume();
	}
	else
	{
		LOG("Recording started.");
		mAudioInput->start(&mBufferDevice);
	}
	setState(QAudio::ActiveState);
timer->start(3000);
}

void ViStreamInput::stop()
{
	LOG("Recording stopped.");
	mBufferDevice.seek(0);
	mAudioInput->stop();
	setState(QAudio::StoppedState);
}

void ViStreamInput::pause()
{
	LOG("Recording paused.");
	mAudioInput->suspend();
	setState(QAudio::SuspendedState);
}

qreal ViStreamInput::volume()
{
	return mAudioInput->volume();
}

void ViStreamInput::setVolume(qreal volumeValue)
{
	if(mIsMute)
	{
		mMuteVolume = volume();
	}
	else
	{
		mAudioInput->setVolume(volumeValue);
	}
}

void ViStreamInput::mute(bool value)
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
