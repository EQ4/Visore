#include "viexecutor.h"

#define DEFAULT_WINDOW_SIZE 2048

ViExecutor::ViExecutor()
	: QThread()
{
	mWindowSize = DEFAULT_WINDOW_SIZE;
	mNotify = false;
	mWasInitialized = false;
	mInputChunk = NULL;
	mInputSamples = NULL;
	mOutputSamples = NULL;
	mOutputChunk = NULL;
	mObject = ViAudioObject::createNull();
	QObject::connect(&mTimer, SIGNAL(timeout()), this, SLOT(updateProcessingRate()));
}

ViExecutor::~ViExecutor()
{
	if(mInputChunk != NULL)
	{
		delete mInputChunk;
		mInputChunk = NULL;
	}
	if(mInputSamples != NULL)
	{
		delete mInputSamples;
		mInputSamples = NULL;
	}
	if(mOutputSamples != NULL)
	{
		delete mOutputSamples;
		mOutputSamples = NULL;
	}
	if(mOutputChunk != NULL)
	{
		delete mOutputChunk;
		mOutputChunk = NULL;
	}
}

qreal ViExecutor::processingRate()
{
	return mProcessingRate;
}

void ViExecutor::setWindowSize(int windowSize)
{
	mWindowSize = windowSize;
}

void ViExecutor::setNotify(bool notify)
{
	mNotify = notify;
}

bool ViExecutor::attach(ViAudio::Mode mode, ViProcessor *processor)
{
	return mProcessors.add(mode, processor);
}

bool ViExecutor::detach(ViProcessor *processor)
{
	disconnect(processor);
	return mProcessors.remove(processor);
}

void ViExecutor::setObject(ViAudioObjectPointer object)
{
	mObject = object;
	if(mInputChunk == NULL)
	{
		mInputChunk = new ViRawChunk();
	}
	if(mInputSamples == NULL)
	{
		mInputSamples = new ViSampleChunk();
	}
	mReadStream = mObject->inputBuffer()->createReadStream();
	
	if(mOutputChunk == NULL)
	{
		mOutputChunk = new ViRawChunk();
	}
	if(mOutputSamples == NULL)
	{
		mOutputSamples = new ViSampleChunk();
	}
	mWriteStream = mObject->outputBuffer()->createWriteStream();
}

int ViExecutor::defaultWindowSize()
{
	return DEFAULT_WINDOW_SIZE;
}

ViProcessor* ViExecutor::processor(QString type)
{
	return mProcessors.processor(type);
}

void ViExecutor::setFormat(ViAudioFormat format)
{
	QObject::disconnect(mObject->inputBuffer(), SIGNAL(formatChanged(ViAudioFormat)), this, SLOT(setFormat(ViAudioFormat)));
	initialize();
}

void ViExecutor::execute()
{
	if(!mWasInitialized)
	{
		initialize();
	}
	if(!isRunning())
	{
		start();
	}
}

void ViExecutor::initialize()
{
	mWasInitialized = true;
	if(mReadStream != NULL)
	{
		mInputFormat = mObject->inputBuffer()->format();
		if(!mInputFormat.isValid())
		{
			mWasInitialized = false;
			QObject::connect(mObject->inputBuffer(), SIGNAL(formatChanged(ViAudioFormat)), this, SLOT(setFormat(ViAudioFormat)), Qt::UniqueConnection);
			return;
		}
		mInputConverter.setSize(mInputFormat.sampleSize());
	}
	if(mWriteStream != NULL)
	{
		mOutputFormat = mInputFormat;
		mObject->outputBuffer()->setFormat(mOutputFormat);
		mOutputConverter.setSize(mOutputFormat.sampleSize());
	}

	QList<ViProcessor*> processors = mProcessors.processors();
	for(int i = 0; i < processors.size(); ++i)
	{
		processors[i]->setObject(mObject);
		processors[i]->setWindowSize(mWindowSize);
		processors[i]->setFormat(mInputFormat);
		processors[i]->initialize();
		if(mNotify)
		{
			connect(processors[i]);
		}
		else
		{
			disconnect(processors[i]);
		}
	}

	if(mInputChunk != NULL)
	{
		mInputChunk->resize(mWindowSize * (mInputFormat.sampleSize() / 8));
	}
	if(mInputSamples != NULL)
	{
		mInputSamples->resize(mWindowSize);
	}
	if(mOutputSamples != NULL)
	{
		mOutputSamples->resize(mWindowSize);
	}
	if(mOutputChunk != NULL)
	{
		mOutputChunk->resize(mWindowSize * (mOutputFormat.sampleSize() / 8));
	}

	mProcessedSamples = 0;
	mProcessingRate = 0;
	mRateCounter = 0;
	mTimer.start(1000);

	QObject::connect(mObject->inputBuffer(), SIGNAL(changed()), this, SLOT(execute()), Qt::UniqueConnection);
}

void ViExecutor::finalize()
{
	if(mWasInitialized)
	{
		while(isRunning()); // TODO: busy waiting here?

		if(!mReadStream.isNull() && !mObject.isNull())
		{
			QObject::disconnect(mObject->inputBuffer(), SIGNAL(changed()), this, SLOT(execute()));
		}

		QList<ViProcessor*> processors = mProcessors.processors();
		for(int i = 0; i < processors.size(); ++i)
		{
			processors[i]->finalize();
LOG("8888***1");
			processors[i]->setObject(NULL);LOG("8888***2");
		}
LOG("8888***3: "+QString::number(mObject.referenceCount()));

LOG("8888***4.5");
		mObject = ViAudioObject::createNull(); // delete object
LOG("8888***5");
		// "Delete" the streams, so that the audio object will emit finished
		mReadStream = NULL;LOG("8888***5.5");
		mWriteStream = NULL;LOG("8888***5.6");

		mWasInitialized = false;
		mTimer.stop();
	}
}

void ViExecutor::updateProcessingRate()
{
	++mRateCounter;
	mProcessingRate = ((mProcessingRate / mRateCounter) * (mRateCounter - 1)) + ((mProcessedSamples / mInputFormat.channelCount()) / mRateCounter);
	mProcessedSamples = 0;
	emit processingRateChanged(mProcessingRate);
}

void ViExecutor::connect(ViProcessor *processor)
{
	QObject::connect(this, SIGNAL(progressed(short)), processor, SIGNAL(progressed(short)), Qt::UniqueConnection);
	QObject::connect(this, SIGNAL(finished()), processor, SIGNAL(finished()), Qt::UniqueConnection);
}

void ViExecutor::disconnect(ViProcessor *processor)
{
	QObject::disconnect(this, SIGNAL(progressed(short)), processor, SIGNAL(progressed(short)));
	QObject::disconnect(this, SIGNAL(finished()), processor, SIGNAL(finished()));
}

void ViExecutor::start()
{
	QThread::start();
}

void ViExecutor::run()
{
	if(mNotify)
	{
		runNotify();
	}
	else
	{
		runNormal();
	}
}
