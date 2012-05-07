#include "viaudioengine.h"
#include "viqtconnection.h"

#include <QTimer>

ViAudioEngine::ViAudioEngine()
	: QObject()
{
	mStreamInput = NULL;
	mFileInput = NULL;
	mStreamOutput = NULL;
	mFileOutput = NULL;
	mInputType = ViAudioEngine::None;

	mAudioConnection = new ViQtConnection();
	resetMetaData();

	mProcessingChain = new ViAudioProcessingChain();
	ViObject::connectDirect(mProcessingChain->originalBuffer(), SIGNAL(changed(int)), this, SIGNAL(originalBufferChanged(int)));
	ViObject::connectDirect(mProcessingChain->correctedBuffer(), SIGNAL(changed(int)), this, SIGNAL(correctedBufferChanged(int)));

	mFileInput = mAudioConnection->fileInput(mFormat, mProcessingChain->originalBuffer());
	mFileOutput = mAudioConnection->fileOutput(mFormat, mProcessingChain->originalBuffer());
	mStreamInput = mAudioConnection->streamInput(mFormat, mProcessingChain->originalBuffer(), QAudioDeviceInfo::defaultInputDevice());
	mStreamOutput = mAudioConnection->streamOutput(mFormat, mProcessingChain->originalBuffer(), QAudioDeviceInfo::defaultOutputDevice());

	mProcessingChain->attachInput(mFileInput);
	mProcessingChain->attachStreamOutput(mStreamOutput);

	ViObject::connectDirect(mFileInput, SIGNAL(formatChanged(ViAudioFormat)), mStreamOutput, SLOT(changeFormat(ViAudioFormat)));
	ViObject::connectDirect(mStreamOutput, SIGNAL(positionChanged(ViAudioPosition)), this, SIGNAL(positionChanged(ViAudioPosition)));
	ViObject::connectDirect(mProcessingChain->originalBuffer(), SIGNAL(changed(int)), this, SIGNAL(originalBufferChanged(int)));
	ViObject::connectDirect(mProcessingChain->correctedBuffer(), SIGNAL(changed(int)), this, SIGNAL(correctedBufferChanged(int)));
	
	mProcessingChain->attachOriginalProcessor(&mOriginalWaveSummarizer, ViProcessorList::Parallel);
	mProcessingChain->attachCorrectedProcessor(&mCorrectedWaveSummarizer, ViProcessorList::Parallel);
	ViObject::connect(&mOriginalWaveSummarizer, SIGNAL(changed()), this, SIGNAL(originalWaveChanged()));
	ViObject::connect(&mCorrectedWaveSummarizer, SIGNAL(changed()), this, SIGNAL(correctedWaveChanged()));

	/*mSongDetector = new ViSongDetector(mStreamOutput);
	mSongDetector->setProxy(QNetworkProxy::HttpProxy, "137.215.6.53", 8080, "p04416376", "Rd28jRX");
	mSongDetector->setKey("G1TZBE4IHJAYUSNCN");
	ViObject::connect(mSongDetector, SIGNAL(songFound(ViSongInfo)), this, SIGNAL(songInfoChanged(ViSongInfo)));*/

	ViAudioBuffer *b1 = new ViAudioBuffer();
	ViAudioBuffer *b2 = new ViAudioBuffer();
	ViSignalManipulator::createDefaultSignal(b1, 500, ViSignalManipulator::Tooth);
	ViSignalManipulator::createDefaultSignal(b2, 500, ViSignalManipulator::Tooth);

	ViSignalManipulator::createNoise(b1, b2, 0.01);

	ViMatcher *m = new ViMatcher();
m->match(b1, b2);
}

ViAudioEngine::~ViAudioEngine()
{
	/*if(mAudioConnectionLoader != NULL)
	{
		mAudioConnectionLoader->deleteObject("deleteConnection", mAudioConnection);
		delete mAudioConnectionLoader;
		mAudioConnectionLoader = NULL;
	}
	if(mAudioInput != NULL)
	{
		delete mAudioInput;
		mAudioInput = NULL;
	}
	if(mAudioOutputs.size() != 0)
	{
		for(int i = 0; i < mAudioOutputs.size(); ++i)
		{
			ViObject::disconnect(mAudioOutputs[i], SIGNAL(changed(int, int)), this, SLOT(changeReceived(int, int)));
			mAudioOutputs[i] = NULL;
		}
	}
	if(mBuffer != NULL)
	{
		delete mBuffer;
		mBuffer = NULL;
	}*/
}

ViAudioProcessingChain* ViAudioEngine::processingChain()
{
	return mProcessingChain;
}



/*
void ViAudioEngine::initializeInputStream(QAudioDeviceInfo deviceInfo, QAudioFormat format)
{
	if(mAudioInput != NULL)
	{
		ViObject::disconnect(mAudioInput, SIGNAL(changed(int, int)), this, SLOT(changeReceived(int, int)));
		delete mAudioInput;
	}
	mInputAudioType = ViAudioEngine::Stream;
	//mAudioInput = new ViStreamInput(mBuffer, deviceInfo, format);
	ViObject::connect(mAudioInput, SIGNAL(changed(int, int)), this, SLOT(changeReceived(int, int)));
}

void ViAudioEngine::initializeInputFile(QString filePath)
{
	if(mAudioInput != NULL)
	{
		ViObject::disconnect(mAudioInput, SIGNAL(changed(int, int)), this, SLOT(changeReceived(int, int)));
		delete mAudioInput;
	}
	mInputAudioType = ViAudioEngine::File;
	//mAudioInput = new ViFileInput(mBuffer, filePath);
	ViObject::connect(mAudioInput, SIGNAL(changed(int, int)), this, SLOT(changeReceived(int, int)));
}

void ViAudioEngine::initializeOutputStream()
{
	if(mOutputAudioType == ViAudioEngine::File)
	{
		mOutputAudioType = ViAudioEngine::FileAndStream;
	}
	else if(mOutputAudioType == ViAudioEngine::None)
	{
		mOutputAudioType = ViAudioEngine::Stream;
	}
	//mAudioOutputs.append(new ViStreamOutput(mBuffer));
}

void ViAudioEngine::initializeOutputFile()
{
	if(mOutputAudioType == ViAudioEngine::Stream)
	{
		mOutputAudioType = ViAudioEngine::FileAndStream;
	}
	else if(mOutputAudioType == ViAudioEngine::None)
	{
		mOutputAudioType = ViAudioEngine::File;
	}
	//mAudioOutputs.append(new ViStreamOutput(mBuffer));
}*/

void ViAudioEngine::setInput(ViAudioEngine::ViAudioType type)
{
	mInputType = type;
	emit inputChanged(mInputType);
}

void ViAudioEngine::setInputFilePath(QString filePath)
{
	resetMetaData();
	mFileInput->setFile(filePath);
	mProcessingChain->attachInput(mFileInput);
	mFileInput->start();
}

void ViAudioEngine::createOutputFile(QString filePath, ViAudioFormat format)
{
	mFileOutput->setFile(filePath);
	mFileOutput->setFormat(format);
	mFileOutput->start();
}

void ViAudioEngine::reset()
{
	mProcessingChain->reset();
}

int ViAudioEngine::volume()
{
	return mStreamOutput->volume() * 100;
}

void ViAudioEngine::setVolume(int volume)
{
	mStreamOutput->setVolume(volume / 100.0);
}

void ViAudioEngine::mute(bool value)
{
	mStreamOutput->mute(value);
}

void ViAudioEngine::startPlayback()
{
	mStreamOutput->start();
}

void ViAudioEngine::stopPlayback()
{
	mStreamOutput->stop();
}

void ViAudioEngine::pausePlayback()
{
	mStreamOutput->pause();
}

void ViAudioEngine::startRecording()
{
	mProcessingChain->reset();
	resetMetaData();
	mProcessingChain->attachInput(mStreamInput);
	mStreamInput->start();
}

void ViAudioEngine::stopRecording()
{
	mStreamInput->stop();
}

void ViAudioEngine::setStreamPosition(ViAudioPosition position)
{
	mStreamOutput->setPosition(position);
}

void ViAudioEngine::startOutputFile()
{
	mFileOutput->start();
}

void ViAudioEngine::stopOutputFile()
{
	mFileOutput->stop();
}

void ViAudioEngine::resetMetaData()
{
	mFormat.setSampleRate(44100);
	mFormat.setChannelCount(2);
	mFormat.setSampleSize(16);
	mFormat.setCodec(ViCodecManager::selected("PCM"));
	mFormat.setSampleType(QAudioFormat::SignedInt);
	mFormat.setByteOrder(QAudioFormat::LittleEndian);
}

ViWaveForm* ViAudioEngine::waveSummary(ViAudioBuffer::ViAudioBufferType type)
{
	if(type == ViAudioBuffer::Original)
	{
		return mOriginalWaveSummarizer.waveSummary();
	}
	else if(type == ViAudioBuffer::Corrected)
	{
		return mCorrectedWaveSummarizer.waveSummary();
	}
}
