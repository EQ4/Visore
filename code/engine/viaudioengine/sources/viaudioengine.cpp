#include <viaudioengine.h>
#include <vicorrelatormanager.h>

ViAudioEngine::ViAudioEngine()
{
	//QObject::connect(&mCorrelator, SIGNAL(progressed(short)), this, SIGNAL(correlationProgressed(short)));
	//QObject::connect(&mCorrelator, SIGNAL(finished()), this, SIGNAL(correlationFinished()));

	mFileInput = mConnection.fileInput();
	mFileOutput = mConnection.fileOutput();
	mStreamInput = mConnection.streamInput();
	mStreamOutput = mConnection.streamOutput();

	mStreamOutput->setDevice(QAudioDeviceInfo::defaultOutputDevice());
	mStreamOutput->setFormat(ViAudioFormat::defaultFormat());

	/*ViAudioFormat fileFormat = ViAudioFormat::defaultFormat();
	fileFormat.setCodec(ViAudioManager::codec("WAVE"));
	mFileOutput->setFormat(fileFormat);*/

	mStreamInput->setFormat(ViAudioFormat::defaultFormat());
	mStreamInput->setDevice(QAudioDeviceInfo::defaultInputDevice());

	//Correlators
	//mProcessingChain.attach(ViAudio::AudioInputOutput, &mCrossCorrelator);
	//mProcessingChain.attach(ViAudio::AudioInputOutput, &mSampleCorrelator);


	//Volume
	QObject::connect(mStreamOutput, SIGNAL(volumeChanged(int)), this, SIGNAL(volumeChanged(int)));
	QObject::connect(mStreamOutput, SIGNAL(muted()), this, SIGNAL(muted()));
	QObject::connect(mStreamOutput, SIGNAL(unmuted()), this, SIGNAL(unmuted()));

	//Position
	QObject::connect(mStreamOutput, SIGNAL(positionChanged(ViAudioPosition)), this, SIGNAL(positionChanged(ViAudioPosition)));
	QObject::connect(mStreamOutput, SIGNAL(lengthChanged(ViAudioPosition)), this, SIGNAL(lengthChanged(ViAudioPosition)));
}

ViAudioEngine::~ViAudioEngine()
{
}

void ViAudioEngine::changeInput(ViAudio::Input input)
{
	if(input == ViAudio::File)
	{
		//mProcessingChain.setTransmission(mFileInput);
	}
	else if(input == ViAudio::Line)
	{
		//mProcessingChain.setTransmission(mStreamInput);
	}
	emit inputChanged(input);
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

void ViAudioEngine::setPosition(int seconds)
{
	mStreamOutput->setPosition(seconds);
}

void ViAudioEngine::startRecording()
{
	mStreamInput->start();
}

void ViAudioEngine::stopRecording()
{
	mStreamInput->stop();
}

void ViAudioEngine::pauseRecording()
{
	mStreamInput->pause();
}

void ViAudioEngine::openFile(QString filePath)
{
	mFileInput->setFile(filePath);
	mFileInput->start();
}

void ViAudioEngine::saveFile(QString filePath)
{

}

void ViAudioEngine::calculateSpectrum(qint32 size, QString windowFunction)
{
	/*mExecutor.setWindowSize(size);
	mSpectrumAnalyzer.setWindowFunction(windowFunction);
	if(!mExecutor.execute(mProcessingChain.buffer(ViAudio::AudioInput), &mSpectrumAnalyzer))
	{
		emit spectrumFinished();
	}*/
}

void ViAudioEngine::calculateCorrelation(ViAudioObjectPointer object)
{
	/*ViSingleExecutor *executor = singleExecutor();
	executor->setMessage("Correlating tracks");
	mCrossCorrelator.setWindowSize(4096);
	executor->execute(object, &mCrossCorrelator, ViAudio::Target, ViAudio::Corrupted);
*/

	
	QObject::connect(object.data(), SIGNAL(progressed(qreal)), this, SIGNAL(progressed(qreal)));
	QObject::connect(object.data(), SIGNAL(statused(QString)), this, SIGNAL(statusChanged(QString)));
	object->align();


	/*mExecutor.setWindowSize(ViExecutor::defaultWindowSize());
	if(!mExecutor.execute(mProcessingChain.buffer(ViAudio::AudioInput), mProcessingChain.buffer(ViAudio::AudioOutput), &mCorrelator))
	{
		emit correlationFinished();
	}*/
}

void ViAudioEngine::correct(ViAudioObjectQueue objects, ViModifyProcessor *corrector)
{
	mObjectChain.clear();
	mObjectChain.add(objects);
	QObject::connect(&mObjectChain, SIGNAL(progressed(qreal)), this, SIGNAL(progressed(qreal)), Qt::UniqueConnection);
	QObject::connect(&mObjectChain, SIGNAL(statused(QString)), this, SIGNAL(statusChanged(QString)), Qt::UniqueConnection);
	QObject::connect(&mObjectChain, SIGNAL(finished()), this, SIGNAL(progressFinished()), Qt::UniqueConnection);
	mObjectChain.addFunction(ViFunctionCall("decode", QVariant::fromValue(ViAudio::Target | ViAudio::Corrupted)), 0.01);
	mObjectChain.addFunction(ViFunctionCall("correct", QVariant::fromValue(corrector)), 0.91);
	mObjectChain.addFunction(ViFunctionCall("encode", QVariant(ViAudio::Corrected)), 0.01);
	mObjectChain.addFunction(ViFunctionCall("align"), 0.01);
	mObjectChain.addFunction(ViFunctionCall("correlate", QVariant::fromValue(ViCorrelatorManager::libraries())), 0.05);
	mObjectChain.addFunction(ViFunctionCall("clearBuffers"), 0.01, false);
	mObjectChain.execute();
}

void ViAudioEngine::recordProject(ViProject *project, ViAudio::Type type, ViAudioFormat format, int sides, bool detectInfo)
{
	QObject::connect(&mRecorder, SIGNAL(statused(QString)), this, SIGNAL(statusChanged(QString)), Qt::UniqueConnection);
	QObject::connect(&mRecorder, SIGNAL(progressed(qreal)), this, SIGNAL(progressed(qreal)), Qt::UniqueConnection);
	QObject::connect(&mRecorder, SIGNAL(finished()), this, SIGNAL(progressFinished()), Qt::UniqueConnection);
	mRecorder.record(project, type, format, sides, detectInfo);
}

void ViAudioEngine::updateMetadata(ViProject *project)
{
	mObjectChain.clear();
	mObjectChain.add(*project);
	QObject::connect(&mObjectChain, SIGNAL(progressed(qreal)), this, SIGNAL(progressed(qreal)), Qt::UniqueConnection);
	QObject::connect(&mObjectChain, SIGNAL(finished()), this, SIGNAL(progressFinished()), Qt::UniqueConnection);
	QObject::connect(&mObjectChain, SIGNAL(statused(QString)), this, SIGNAL(statusChanged(QString)), Qt::UniqueConnection);
	mObjectChain.addFunction(ViFunctionCall("decode", QVariant::fromValue(ViAudio::Target | ViAudio::Corrupted | ViAudio::Corrected)), 0.40);
	mObjectChain.addFunction(ViFunctionCall("encode", QVariant::fromValue(ViAudio::Target | ViAudio::Corrupted | ViAudio::Corrected)), 0.59);
	mObjectChain.addFunction(ViFunctionCall("clearBuffers"), 0.01, false);
	mObjectChain.execute("Updating Metadata");
}

void ViAudioEngine::generateWave(ViAudioObjectPointer object, ViAudio::Type type, const bool &align)
{
	mObjectChain.clear();
	mObjectChain.add(object);
	QObject::connect(&mObjectChain, SIGNAL(progressed(qreal)), this, SIGNAL(progressed(qreal)), Qt::UniqueConnection);
	QObject::connect(&mObjectChain, SIGNAL(finished()), this, SIGNAL(progressFinished()), Qt::UniqueConnection);
	QObject::connect(&mObjectChain, SIGNAL(statused(QString)), this, SIGNAL(statusChanged(QString)), Qt::UniqueConnection);
	mObjectChain.addFunction(ViFunctionCall("decode", QVariant::fromValue(type)), 0.19);

	qfloat percentage = 0.8;
	if(align)
	{
		percentage = 0.75;
		mObjectChain.addFunction(ViFunctionCall("align"), 0.05);
	}

	mObjectChain.addFunction(ViFunctionCall("generateWave", {QVariant::fromValue(type), true}), percentage);
	mObjectChain.addFunction(ViFunctionCall("clearBuffers"), 0.01, false);
	mObjectChain.execute("Generating Waves");
}

void ViAudioEngine::align(ViProject &project)
{
	mObjectChain.clear();
	mObjectChain.add(project);
	QObject::connect(&mObjectChain, SIGNAL(progressed(qreal)), this, SIGNAL(progressed(qreal)));
	QObject::connect(&mObjectChain, SIGNAL(statused(QString)), this, SIGNAL(statusChanged(QString)));
	mObjectChain.addFunction("align");
	mObjectChain.execute();
}

void ViAudioEngine::align(ViAudioObjectPointer object)
{
	mObjectChain.clear();
	mObjectChain.add(object);
	QObject::connect(&mObjectChain, SIGNAL(progressed(qreal)), this, SIGNAL(progressed(qreal)));
	QObject::connect(&mObjectChain, SIGNAL(statused(QString)), this, SIGNAL(statusChanged(QString)));
	mObjectChain.addFunction("align");
	mObjectChain.execute();
}

void ViAudioEngine::disconnectObject()
{
	sender()->disconnect();
}


//Volume

void ViAudioEngine::setVolume(int volume)
{
	mStreamOutput->setVolume(volume);
}

void ViAudioEngine::mute(bool value)
{
	mStreamOutput->mute(value);
}

void ViAudioEngine::unmute()
{
	mStreamOutput->unmute();
}

int ViAudioEngine::volume()
{
	return mStreamOutput->volume();
}
