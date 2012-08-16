#include "viprocessingchain.h"
#include "viaudiocodec.h"

#include "viunderrunhandler.h"

#define MINIMUM_SONG_LENGTH 1500

ViProcessingChain::ViProcessingChain()
	: QObject()
{
	mInput = NULL;
	mStreamOutput = NULL;
	mFileOutput = NULL;
	mInputBuffer = NULL;
	mOutputBuffer = NULL;
	mProject = NULL;
	mMultiExecutor.setNotify(true);
	QObject::connect(&mMultiExecutor, SIGNAL(progressed(short)), this, SIGNAL(changed()));

	mHandlers.append(new ViUnderrunHandler(this));
}

ViProcessingChain::~ViProcessingChain()
{
	if(mInputBuffer != NULL)
	{
		delete mInputBuffer;
		mInputBuffer = NULL;
	}
	qDeleteAll(mInputBuffers);
	mInputBuffers.clear();
	if(mOutputBuffer != NULL)
	{
		delete mOutputBuffer;
		mOutputBuffer = NULL;
	}
	qDeleteAll(mOutputBuffers);
	mOutputBuffers.clear();

	if(mProject != NULL)
	{
		delete mProject;
		mProject = NULL;
	}

	qDeleteAll(mHandlers);
	mHandlers.clear();
}

void ViProcessingChain::changeInput(ViAudioPosition position)
{
	QObject::connect(&mMultiExecutor, SIGNAL(finished()), this, SLOT(finalize()));
	mInput->setBuffer(allocateBuffer(ViAudio::AudioInput));
}

void ViProcessingChain::startInput(ViAudioPosition position)
{
	allocateBuffer(ViAudio::AudioInput);
	allocateBuffer(ViAudio::AudioOutput);
	nextBuffer(ViAudio::AudioInput);
	nextBuffer(ViAudio::AudioOutput);
	mInput->setBuffer(mInputBuffer);
	mMultiExecutor.setBuffer(ViAudio::AudioInput, mInputBuffer);
	mMultiExecutor.setBuffer(ViAudio::AudioOutput, mOutputBuffer);
	mStreamOutput->setBuffer(mOutputBuffer);
	mMultiExecutor.initialize();
}

void ViProcessingChain::finalize()
{
	mMultiExecutor.finalize();
	QObject::disconnect(&mMultiExecutor, SIGNAL(finished()), this, SLOT(finalize()));
	nextBuffer(ViAudio::AudioInput);
	mMultiExecutor.setBuffer(ViAudio::AudioInput, mInputBuffer);
	mMultiExecutor.setBuffer(ViAudio::AudioOutput, allocateBuffer(ViAudio::AudioOutput));
	qreal songLength = ViAudioPosition::convertPosition(mOutputBuffer->size(), ViAudioPosition::Samples, ViAudioPosition::Milliseconds, mOutputBuffer->format());
	if(mProject != NULL && songLength >= MINIMUM_SONG_LENGTH)
	{
		QObject::connect(mFileOutput, SIGNAL(finished()), this, SLOT(finishWriting()));
		mFileOutput->setBuffer(mOutputBuffer);
		mFileOutput->setFile(mProject->originalPath(), mProject->nextOriginalSongNumber(), mFileOutput->format().codec()->extensions()[0]);
		mFileOutput->start();
	}
	else
	{
		nextBuffer(ViAudio::AudioOutput);
		mStreamOutput->setBuffer(mOutputBuffer);
	}
}

void ViProcessingChain::finishWriting()
{
	ViFileSongInfo info = ViFileSongInfo(mFileOutput->songInfo());
	info.setOriginalFilePath(mFileOutput->filePath());
	mProject->addSong(info);
	mProject->save();
	QObject::disconnect(mFileOutput, SIGNAL(finished()), this, SLOT(finishWriting()));
	nextBuffer(ViAudio::AudioOutput);

	QObject::connect(mStreamOutput, SIGNAL(finished()), this, SLOT(finishPlaying()));
	if(mStreamOutput->state() != QAudio::ActiveState)
	{
		finishPlaying();
	}
}

void ViProcessingChain::finishPlaying()
{
	QObject::disconnect(mStreamOutput, SIGNAL(finished()), this, SLOT(finishPlaying()));
	mStreamOutput->setBuffer(mOutputBuffer);
	mMultiExecutor.initialize();
}

void ViProcessingChain::setWindowSize(int windowSize)
{
	mMultiExecutor.setWindowSize(windowSize);
}

void ViProcessingChain::setTransmission(ViAudioTransmission *transmission)
{
	ViAudioInput *input;
	ViStreamOutput *streamOutput;
	ViFileOutput *fileOutput;
	if((input = dynamic_cast<ViAudioInput*>(transmission)) != NULL)
	{
		mInput = input;
		allocateBuffer(ViAudio::AudioInput);
		nextBuffer(ViAudio::AudioInput);
		mInput->setBuffer(mInputBuffer);
		mMultiExecutor.setBuffer(ViAudio::AudioInput, mInputBuffer);
	}
	else if((streamOutput = dynamic_cast<ViStreamOutput*>(transmission)) != NULL)
	{
		mStreamOutput = streamOutput;
		emit streamOutputChanged(mStreamOutput);
		allocateBuffer(ViAudio::AudioOutput);
		nextBuffer(ViAudio::AudioOutput);
		mStreamOutput->setBuffer(mOutputBuffer);
		mMultiExecutor.setBuffer(ViAudio::AudioOutput, mOutputBuffer);
	}
	else if((fileOutput = dynamic_cast<ViFileOutput*>(transmission)) != NULL)
	{
		mFileOutput = fileOutput;
	}
}

void ViProcessingChain::setProject(ViProject *project, ViAudioFormat format)
{
	mFileOutput->setFormat(format);
	mProject = project;
	mProject->save();
}

bool ViProcessingChain::attach(ViAudio::Mode mode, ViProcessor *processor)
{
	mMultiExecutor.attach(mode, processor);
}

bool ViProcessingChain::detach(ViProcessor *processor)
{
	mMultiExecutor.detach(processor);
}

ViAudioBuffer* ViProcessingChain::buffer(ViAudio::Mode mode)
{
	if(mode == ViAudio::AudioInput)
	{
		return mInputBuffer;
	}
	else if(mode == ViAudio::AudioOutput)
	{
		return mOutputBuffer;
	}
}

ViStreamOutput* ViProcessingChain::streamOutput()
{
	return mStreamOutput;
}

ViExecutor* ViProcessingChain::executor()
{
	return &mMultiExecutor;
}

ViAudioBuffer* ViProcessingChain::allocateBuffer(ViAudio::Mode mode)
{
	ViAudioBuffer *buffer = new ViAudioBuffer();
	if(mode == ViAudio::AudioInput)
	{
		mInputBuffers.enqueue(buffer);
	}
	else if(mode == ViAudio::AudioOutput)
	{
		mOutputBuffers.enqueue(buffer);
	}
	return buffer;
}

void ViProcessingChain::nextBuffer(ViAudio::Mode mode)
{
	if(mode == ViAudio::AudioInput)
	{
		if(mInputBuffer != NULL)
		{
			delete mInputBuffer;
		}
		if(mInputBuffers.isEmpty())
		{
			mInputBuffer = NULL;
		}
		else
		{
			mInputBuffer = mInputBuffers.dequeue();
		}
	}
	else if(mode == ViAudio::AudioOutput)
	{
		if(mOutputBuffer != NULL)
		{
			delete mOutputBuffer;
		}
		if(mOutputBuffers.isEmpty())
		{
			mOutputBuffer = NULL;
		}
		else
		{
			mOutputBuffer = mOutputBuffers.dequeue();
		}
	}
}
