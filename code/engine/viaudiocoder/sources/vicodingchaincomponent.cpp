#include <vicodingchaincomponent.h>
#include <QFileInfo>
#include <vimanager.h>

#define CHUNK_SIZE 8192

/**********************************************************
ViCodingChainComponent
**********************************************************/

ViCodingChainComponent::ViCodingChainComponent()
	: QObject()
{
	mNext = NULL;
	mError = ViCoder::NoError;
}

ViCoder::Error ViCodingChainComponent::error()
{
	return mError;
}

void ViCodingChainComponent::setError(ViCoder::Error error)
{
	mError = error;
	emit failed(mError);
}

void ViCodingChainComponent::setHeader(QByteArray header)
{
	mHeader = header;
}

void ViCodingChainComponent::setNext(ViCodingChainComponent *next)
{
	mNext = next;
}

int ViCodingChainComponent::chunkSize()
{
	return CHUNK_SIZE;
}

void ViCodingChainComponent::changeFormat(ViAudioFormat format)
{
}

void ViCodingChainComponent::addData(ViSampleArray *data)
{
	mData.enqueue(data);
	execute();
}

/**********************************************************
ViCodingChainInput
**********************************************************/

ViCodingChainInput::ViCodingChainInput()
	: ViCodingChainComponent()
{
	mSampleSize = -1;
}

void ViCodingChainInput::setSampleSize(int size)
{
	mSampleSize = size / 8;
}

/**********************************************************
ViCodingChainFileInput
**********************************************************/

ViCodingChainFileInput::ViCodingChainFileInput()
	: ViCodingChainInput()
{
	mFilePath = "";
}

void ViCodingChainFileInput::setFilePath(QString filePath)
{
	mFilePath = filePath;
}

bool ViCodingChainFileInput::hasData()
{
	return !mFile.atEnd();
}

int ViCodingChainFileInput::size()
{
	return mFile.size();
}

void ViCodingChainFileInput::initialize()
{
	mData.clear();
	if(mFilePath == "")
	{
		setError(ViCoder::InputFileError);
		return;
	}
	mFile.setFileName(mFilePath);
	if(!mFile.open(QIODevice::ReadOnly))
	{
		setError(ViCoder::InputFileError);
	}
}

void ViCodingChainFileInput::execute()
{
	char *data = new char[CHUNK_SIZE];
	int size = mFile.read(data, CHUNK_SIZE);
	mNext->addData(new ViSampleArray(data, size));
}

void ViCodingChainFileInput::finalize()
{
	mFile.close();
}

/**********************************************************
ViCodingChainDataInput
**********************************************************/

ViCodingChainDataInput::ViCodingChainDataInput()
	: ViCodingChainInput()
{
	mByteArray = NULL;
	mStream = NULL;
}

ViCodingChainDataInput::~ViCodingChainDataInput()
{
	if(mStream != NULL)
	{
		delete mStream;
		mStream = NULL;
	}
	mByteArray = NULL;
}

void ViCodingChainDataInput::setData(QByteArray &data)
{
	mByteArray = &data;
}

bool ViCodingChainDataInput::hasData()
{
	return !mStream->atEnd();
}

int ViCodingChainDataInput::size()
{
	return mByteArray->size();
}

void ViCodingChainDataInput::initialize()
{
	mData.clear();
	if(mStream != NULL)
	{
		delete mStream;
		mStream = NULL;
	}
	mStream = new QDataStream(mByteArray, QIODevice::ReadOnly);
}

void ViCodingChainDataInput::execute()
{
	char *data = new char[CHUNK_SIZE];
	int size = mStream->readRawData(data, CHUNK_SIZE);
	mNext->addData(new ViSampleArray(data, size, size / mSampleSize));
}

void ViCodingChainDataInput::finalize()
{
	if(mStream != NULL)
	{
		delete mStream;
		mStream = NULL;
	}
	mByteArray = NULL;
}

/**********************************************************
ViCodingChainBufferInput
**********************************************************/

ViCodingChainBufferInput::ViCodingChainBufferInput()
	: ViCodingChainInput()
{
	mBuffer = NULL;
}

ViCodingChainBufferInput::~ViCodingChainBufferInput()
{
	mBuffer= NULL;
}

void ViCodingChainBufferInput::setBuffer(ViBuffer *buffer)
{
	mBuffer = buffer;
}

bool ViCodingChainBufferInput::hasData()
{
	return !mStream->atEnd();
}

int ViCodingChainBufferInput::size()
{
	return mStream->size();
}

void ViCodingChainBufferInput::initialize()
{
	mStream = mBuffer->createReadStream();
}

void ViCodingChainBufferInput::execute()
{
	char *data = new char[CHUNK_SIZE];
	int size = mStream->read(data, CHUNK_SIZE);
	mNext->addData(new ViSampleArray(data, size, size / mSampleSize));
}

void ViCodingChainBufferInput::finalize()
{
	mBuffer= NULL;
}

/**********************************************************
ViCodingChainCoder
**********************************************************/

ViCodingChainCoder::ViCodingChainCoder()
	: ViCodingChainComponent()
{
	mCoder = NULL;
}

void ViCodingChainCoder::setCoder(ViAbstractCoder *coder)
{
	mCoder = coder;
}

/**********************************************************
ViCodingChainDecoder
**********************************************************/

ViCodingChainDecoder::ViCodingChainDecoder()
	: ViCodingChainCoder()
{
}

void ViCodingChainDecoder::setCoder(ViAbstractCoder *coder)
{
	ViCodingChainCoder::setCoder(coder);
	coder->disconnect(coder, SIGNAL(formatChanged(ViAudioFormat)));
	QObject::connect(coder, SIGNAL(formatChanged(ViAudioFormat)), mNext, SLOT(changeFormat(ViAudioFormat)), Qt::DirectConnection);
}

void ViCodingChainDecoder::initialize()
{
	mData.clear();
	if(mCoder != NULL)
	{
		if(mCoder->initializeDecode())
		{
			mCoder->disconnect(mCoder, SIGNAL(decoded(ViSampleArray*)));
			QObject::connect(mCoder, SIGNAL(decoded(ViSampleArray*)), mNext, SLOT(addData(ViSampleArray*)), Qt::DirectConnection);
		}
		else
		{
			setError(ViCoder::DecoderInitializationError);
		}
	}
}

void ViCodingChainDecoder::finalize()
{
	if(mCoder != NULL)
	{
		if(!mCoder->finalizeDecode())
		{
			setError(ViCoder::DecoderFinalizationError);
		}
		mCoder->disconnect();
	}
}

void ViCodingChainDecoder::execute()
{
	ViSampleArray *array = mData.dequeue();
	mCoder->decode(array->data(), array->size());
	delete array;
}

/**********************************************************
ViCodingChainEncoder
**********************************************************/

ViCodingChainEncoder::ViCodingChainEncoder()
	: ViCodingChainCoder()
{
}

void ViCodingChainEncoder::changeFormat(ViAudioFormat format)
{
	if(mCoder != NULL)
	{
		mCoder->setFormat(ViAudio::AudioInput, format);
		if(mCoder->initializeEncode())
		{
			mCoder->disconnect(mCoder, SIGNAL(encoded(ViSampleArray*)));
			QObject::connect(mCoder, SIGNAL(encoded(ViSampleArray*)), mNext, SLOT(addData(ViSampleArray*)), Qt::DirectConnection);
		}
		else
		{
			setError(ViCoder::EncoderInitializationError);
		}
	}
}

void ViCodingChainEncoder::initialize()
{
	mData.clear();
}

void ViCodingChainEncoder::finalize()
{
	if(mCoder != NULL)
	{
		if(!mCoder->finalizeEncode())
		{
			setError(ViCoder::EncoderFinalizationError);
		}
		mCoder->disconnect();
	}
	mNext->setHeader(mCoder->header());
}

void ViCodingChainEncoder::execute()
{
	ViSampleArray *array = mData.dequeue();
	mCoder->encode(array->data(), array->samples());
	delete array;
}

/**********************************************************
ViCodingChainOutput
**********************************************************/

ViCodingChainOutput::ViCodingChainOutput()
	: ViCodingChainComponent()
{
	mOffset = 0;
	mCurrentOffset = 0;
}

void ViCodingChainOutput::initialize()
{
	mCurrentOffset = 0;
}

void ViCodingChainOutput::setOffset(int offset)
{
	mOffset = offset;
}

/**********************************************************
ViCodingChainFileOutput
**********************************************************/

ViCodingChainFileOutput::ViCodingChainFileOutput()
	: ViCodingChainOutput()
{
	mFilePath = "";
}

void ViCodingChainFileOutput::setFilePath(QString filePath)
{
	mFilePath = filePath;
}

void ViCodingChainFileOutput::initialize()
{
	ViCodingChainOutput::initialize();
	mData.clear();
	if(mFilePath == "")
	{
		setError(ViCoder::OutputFileError);
		return;
	}
	QFileInfo info(mFilePath);
	QString folder = ViManager::tempPath() + QDir::separator() + "coder";
	QDir dir(folder);
	if(!dir.exists())
	{
		dir.mkpath(folder);
	}
	mTempFile.setFileName(folder + QDir::separator() + info.fileName());
	if(!mTempFile.open(QIODevice::WriteOnly))
	{
		setError(ViCoder::OutputFileError);
	}
}

void ViCodingChainFileOutput::finalize()
{
	mTempFile.close();
	mFile.setFileName(mFilePath);
	if(!mTempFile.open(QIODevice::ReadOnly) || !mFile.open(QIODevice::WriteOnly))
	{
		setError(ViCoder::OutputFileError);
	}

	mFile.write(mHeader);
	mHeader.clear();	

	char data[CHUNK_SIZE];
	qint64 size = 0;
	while(!mTempFile.atEnd())
	{
		size = mTempFile.read(data, CHUNK_SIZE);
		mFile.write(data, size);
	}
	mTempFile.close();
	mTempFile.remove();
	mFile.close();
}

void ViCodingChainFileOutput::execute()
{
	ViSampleArray *array = mData.dequeue();
	if(mCurrentOffset < mOffset)
	{
		int offset = mOffset - mCurrentOffset;
		if(offset <= array->size())
		{
			mTempFile.write(array->charData() + offset, array->size() - offset);
		}
		mCurrentOffset += array->size();
	}
	else
	{
		mTempFile.write(array->charData(), array->size());
	}
	delete array;
}

/**********************************************************
ViCodingChainDataOutput
**********************************************************/

ViCodingChainDataOutput::ViCodingChainDataOutput()
	: ViCodingChainOutput()
{
	mByteArray = NULL;
	mStream = NULL;
}

ViCodingChainDataOutput::~ViCodingChainDataOutput()
{
	if(mStream != NULL)
	{
		delete mStream;
		mStream = NULL;
	}
	mByteArray = NULL;
}

void ViCodingChainDataOutput::setData(QByteArray &data)
{
	mByteArray = &data;
}

void ViCodingChainDataOutput::initialize()
{
	ViCodingChainOutput::initialize();
	mData.clear();
	if(mStream != NULL)
	{
		delete mStream;
		mStream = NULL;
	}
	mStream = new QDataStream(mByteArray, QIODevice::WriteOnly);
}

void ViCodingChainDataOutput::finalize()
{
	mByteArray->insert(0, mHeader);
	mHeader.clear();
	if(mStream != NULL)
	{
		delete mStream;
		mStream = NULL;
	}
	mByteArray = NULL;
}

void ViCodingChainDataOutput::execute()
{
	ViSampleArray *array = mData.dequeue();
	if(mCurrentOffset < mOffset)
	{
		int offset = mOffset - mCurrentOffset;
		if(offset <= array->size())
		{
			mStream->writeRawData(array->charData() + offset, array->size() - offset);
		}
		mCurrentOffset += array->size();
	}
	else
	{
		mStream->writeRawData(array->charData(), array->size());
	}
	delete array;
}

/**********************************************************
ViCodingChainBufferOutput
**********************************************************/

ViCodingChainBufferOutput::ViCodingChainBufferOutput()
	: ViCodingChainOutput()
{
	mBuffer = NULL;
}

ViCodingChainBufferOutput::~ViCodingChainBufferOutput()
{
	mBuffer = NULL;
}

void ViCodingChainBufferOutput::setBuffer(ViBuffer *buffer)
{
	mBuffer = buffer;
}

void ViCodingChainBufferOutput::initialize()
{
	ViCodingChainOutput::initialize();
	mStream = mBuffer->createWriteStream();
}

void ViCodingChainBufferOutput::finalize()
{
	mStream->insert(0, mHeader.data(), mHeader.size());
	mHeader.clear();
	mBuffer = NULL;
}

void ViCodingChainBufferOutput::execute()
{
	ViSampleArray *array = mData.dequeue();
	if(mCurrentOffset < mOffset)
	{
		int offset = mOffset - mCurrentOffset;
		if(offset <= array->size())
		{
			mStream->write(array->charData() + offset, array->size() - offset);
		}
		mCurrentOffset += array->size();
	}
	else
	{
		mStream->write(array->charData(), array->size());
	}
	delete array;
}
