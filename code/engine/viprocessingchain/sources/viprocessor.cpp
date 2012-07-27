#include "viprocessor.h"

ViProcessor::ViProcessor()
	: QObject(), QRunnable(), ViId()
{
	setAutoDelete(false); //Ensures that QThreadPool doesn't automatically delete object
	mWindowSize = 0;
	mIsEnabled = true;
}

void ViProcessor::setWindowSize(int windowSize)
{
	mWindowSize = windowSize;
}

int ViProcessor::windowSize()
{
	return mWindowSize;
}

void ViProcessor::setFormat(ViAudioFormat format)
{
	mFormat = format;
}

ViAudioFormat ViProcessor::format()
{
	return mFormat;
}

void ViProcessor::initialize()
{
}

void ViProcessor::finalize()
{
}

void ViProcessor::enable()
{
	mIsEnabledMutex.lock();
	mIsEnabled = true;
	mIsEnabledMutex.unlock();
}

void ViProcessor::disable()
{
	mIsEnabledMutex.lock();
	mIsEnabled = false;
	mIsEnabledMutex.unlock();
}

ViObserver::ViObserver()
	: ViProcessor()
{
	mData = NULL;
	mBuffer = NULL;
}

void ViObserver::setBuffer(ViAudioBuffer *buffer, ViAudio::Mode mode)
{
	if(mode == ViAudio::AudioInput)
	{
		mBuffer = buffer;
	}
}

void ViObserver::setData(const ViSampleChunk *data)
{
	mData = data;
}

ViModifier::ViModifier()
	: ViProcessor()
{
	mData = NULL;
	mBuffer = NULL;
}

void ViModifier::setBuffer(ViAudioBuffer *buffer, ViAudio::Mode mode)
{
	if(mode == ViAudio::AudioInput)
	{
		mBuffer = buffer;
	}
}

void ViModifier::setData(ViSampleChunk *data)
{
	mData = data;
}

ViDualObserver::ViDualObserver()
	: ViObserver()
{
	mData2 = NULL;
	mBuffer2 = NULL;
}

void ViDualObserver::setBuffer(ViAudioBuffer *buffer, ViAudio::Mode mode)
{
	if(mode == ViAudio::AudioInput)
	{
		mBuffer = buffer;
	}
	else
	{
		mBuffer2 = buffer;
	}
}

void ViDualObserver::setData(const ViSampleChunk *data, const ViSampleChunk *data2)
{
	mData = data;
	mData2 = data2;
}

