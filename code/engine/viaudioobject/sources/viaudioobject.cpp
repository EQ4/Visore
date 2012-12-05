#include "viaudioobject.h"

/*******************************************************************************************************************

	CONSTRUCTORS & DESTRUCTORS

*******************************************************************************************************************/

ViAudioObject::ViAudioObject(bool autoDestruct)
	: QObject(), ViId()
{
	mAutoDestruct = autoDestruct;
	mIsFinished = false;
	mIsSong = false;

	mInputType = ViAudioObject::Unknown;
	mOutputType = ViAudioObject::Unknown;

	mTargetBuffer = NULL;
	mCorruptedBuffer = NULL;
	mCorrectedBuffer = NULL;
	mTempBuffer = NULL;

	mTargetFile = "";
	mCorruptedFile = "";
	mCorrectedFile = "";

LOG("created.... "+id());
}

ViAudioObject::~ViAudioObject()
{LOG("deleted.... "+id());
	if(mAutoDestruct)
	{
		clearBuffers();
	}
}

ViAudioObjectPointer ViAudioObject::create(ViAudioObject *object)
{
	return ViAudioObjectPointer(object);
}

ViAudioObjectPointer ViAudioObject::create(bool autoDestruct)
{
	return ViAudioObjectPointer(new ViAudioObject(autoDestruct));
}

ViAudioObjectPointer ViAudioObject::createNull()
{
	return ViAudioObjectPointer(NULL);
}

/*******************************************************************************************************************

	SLOTS

*******************************************************************************************************************/

void ViAudioObject::checkFinished()
{
	QMutexLocker locker(&mMutex);
	if(!mIsFinished)
	{
		locker.unlock();
		if(!isUsed(QIODevice::WriteOnly))
		{
			locker.relock();
			mIsFinished = true;
			locker.unlock();
			emit finished();
		}
	}
}

/*******************************************************************************************************************

	INPUT & OUTPUT

*******************************************************************************************************************/

void ViAudioObject::setType(ViAudioObject::Type input, ViAudioObject::Type output)
{
	setInputType(input);
	setOutputType(output);
}

void ViAudioObject::setInputType(ViAudioObject::Type type)
{
	mInputType = type;
}

void ViAudioObject::setOutputType(ViAudioObject::Type type)
{
	mOutputType = type;
}

/*******************************************************************************************************************

	BUFFERS

*******************************************************************************************************************/

ViBuffer* ViAudioObject::targetBuffer()
{
	QMutexLocker locker(&mMutex);
	if(mTargetBuffer == NULL)
	{
		mTargetBuffer = new ViBuffer();
		QObject::connect(mTargetBuffer, SIGNAL(streamsChanged()), this, SLOT(checkFinished()));
	}
	return mTargetBuffer;
}

ViBuffer* ViAudioObject::corruptedBuffer()
{
	QMutexLocker locker(&mMutex);
	if(mCorruptedBuffer == NULL)
	{
		mCorruptedBuffer = new ViBuffer();
		QObject::connect(mCorruptedBuffer, SIGNAL(streamsChanged()), this, SLOT(checkFinished()));
	}
	return mCorruptedBuffer;
}

ViBuffer* ViAudioObject::correctedBuffer()
{
	QMutexLocker locker(&mMutex);
	if(mCorrectedBuffer == NULL)
	{
		mCorrectedBuffer = new ViBuffer();
		QObject::connect(mCorrectedBuffer, SIGNAL(streamsChanged()), this, SLOT(checkFinished()));
	}
	return mCorrectedBuffer;
}

ViBuffer* ViAudioObject::tempBuffer()
{
	QMutexLocker locker(&mMutex);
	if(mTempBuffer == NULL)
	{
		mTempBuffer = new ViBuffer();
		QObject::connect(mTempBuffer, SIGNAL(streamsChanged()), this, SLOT(checkFinished()));
	}
	return mTempBuffer;
}

ViBuffer* ViAudioObject::inputBuffer()
{
	if(mInputType == ViAudioObject::Target)
	{
		return targetBuffer();
	}
	else if(mInputType == ViAudioObject::Corrupted)
	{
		return corruptedBuffer();
	}
	else if(mInputType == ViAudioObject::Corrected)
	{
		return correctedBuffer();
	}
	else if(mInputType == ViAudioObject::Temporary)
	{
		return tempBuffer();
	}
	return NULL;
}

ViBuffer* ViAudioObject::outputBuffer()
{
	if(mOutputType == ViAudioObject::Target)
	{
		return targetBuffer();
	}
	else if(mOutputType == ViAudioObject::Corrupted)
	{
		return corruptedBuffer();
	}
	else if(mOutputType == ViAudioObject::Corrected)
	{
		return correctedBuffer();
	}
	else if(mOutputType == ViAudioObject::Temporary)
	{
		return tempBuffer();
	}
	return NULL;
}

void ViAudioObject::setTargetBuffer(ViBuffer *buffer)
{
	QMutexLocker locker(&mMutex);
	mTargetBuffer = buffer;
}

void ViAudioObject::setCorruptedBuffer(ViBuffer *buffer)
{
	QMutexLocker locker(&mMutex);
	mCorruptedBuffer = buffer;
}

void ViAudioObject::setCorrectedBuffer(ViBuffer *buffer)
{
	QMutexLocker locker(&mMutex);
	mCorrectedBuffer = buffer;
}

void ViAudioObject::clearBuffers()
{LOG("++++1");
	clearTargetBuffer();
	clearCorruptedBuffer();LOG("++++2");
	clearCorrectedBuffer();LOG("++++3");
	clearTempBuffer();LOG("++++4");
}

void ViAudioObject::clearTargetBuffer()
{
	QMutexLocker locker(&mMutex);
	if(mTargetBuffer != NULL)
	{
		QObject::disconnect(mTargetBuffer, SIGNAL(streamsChanged()), this, SLOT(checkFinished()));
		delete mTargetBuffer;
		mTargetBuffer = NULL;
	}
}

void ViAudioObject::clearCorruptedBuffer()
{
	QMutexLocker locker(&mMutex);
	if(mCorruptedBuffer != NULL)
	{
		QObject::disconnect(mCorruptedBuffer, SIGNAL(streamsChanged()), this, SLOT(checkFinished()));
		delete mCorruptedBuffer;
		mCorruptedBuffer = NULL;
	}
}

void ViAudioObject::clearCorrectedBuffer()
{
	QMutexLocker locker(&mMutex);
	if(mCorrectedBuffer != NULL)
	{
		QObject::disconnect(mCorrectedBuffer, SIGNAL(streamsChanged()), this, SLOT(checkFinished()));
		delete mCorrectedBuffer;
		mCorrectedBuffer = NULL;
	}
}

void ViAudioObject::clearTempBuffer()
{
	QMutexLocker locker(&mMutex);
	if(mTempBuffer != NULL)
	{
		QObject::disconnect(mTempBuffer, SIGNAL(streamsChanged()), this, SLOT(checkFinished()));LOG("rrrrrr1");
		delete mTempBuffer;LOG("rrrrrr2");
		mTempBuffer = NULL;
	}
}

/*******************************************************************************************************************

	FILES

*******************************************************************************************************************/

QString ViAudioObject::targetFile()
{
	return mTargetFile;
}

QString ViAudioObject::corruptedFile()
{
	return mCorruptedFile;
}

QString ViAudioObject::correctedFile()
{
	return mCorrectedFile;
}

void ViAudioObject::setTargetFile(QString path)
{
	mTargetFile = path;
}

void ViAudioObject::setCorruptedFile(QString path)
{
	mCorruptedFile = path;
}

void ViAudioObject::setCorrectedFile(QString path)
{
	mCorrectedFile = path;
}

/*******************************************************************************************************************

	SONG INFO

*******************************************************************************************************************/

ViSongInfo& ViAudioObject::songInfo()
{
	return mSongInfo;
}

void ViAudioObject::setSongInfo(ViSongInfo info)
{
	mSongInfo = info;
}












void ViAudioObject::setSong(bool song)
{
	QMutexLocker locker(&mMutex);
	mIsSong = song;
}

bool ViAudioObject::isSong()
{
	QMutexLocker locker(&mMutex);
	return mIsSong;
}

bool ViAudioObject::isFinished()
{
	QMutexLocker locker(&mMutex);
	return mIsFinished;
}

bool ViAudioObject::isUsed(QIODevice::OpenMode mode)
{
	QMutexLocker locker(&mMutex);
	return	(mTargetBuffer != NULL && mTargetBuffer->streamCount(mode) > 0) ||
			(mCorruptedBuffer != NULL && mCorruptedBuffer->streamCount(mode) > 0) ||
			(mCorrectedBuffer != NULL && mCorrectedBuffer->streamCount(mode) > 0) ||
			(mTempBuffer != NULL && mTempBuffer->streamCount(mode) > 0);
}

void ViAudioObject::addCorrelation(const ViElement &correlation)
{
	mCorrelations.append(correlation);
}

ViElementList& ViAudioObject::correlations()
{
	return mCorrelations;
}
