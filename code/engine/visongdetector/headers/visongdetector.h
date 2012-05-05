#ifndef VISONGDETECTOR_H
#define VISONGDETECTOR_H

#include <QUrl>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include "viaudiobuffer.h"
#include "viaudioprocessingchain.h"
#include "viaudioposition.h"
#include "visonginfo.h"
#include "viobject.h"
#include "vipcmconverter.h"
#include "viechonestresponse.h"
#include "vicoder.h"

//Interval (milliseconds) use to request info
#define REQUEST_SAMPLES_1 20000000
#define REQUEST_SAMPLES_2 100000000
#define REQUEST_SAMPLES_3 200000000

class ViSongCodeGeneratorThread : public QThread
{
	Q_OBJECT

	signals:
		void finished(QString code, QString version, int codeLength);

	public:
		ViSongCodeGeneratorThread(ViAudioBuffer *buffer, ViAudioFormat format, QObject *parent = 0);
		void setOffset(int offset);

	protected:
		void run();

	private:
		ViAudioBuffer *mBuffer;
		ViAudioFormat mFormat;
		int mOffset;
};

class ViSongDetector : public QObject
{
	Q_OBJECT

	public:
		enum State
		{
			Idle = 0,
			Busy = 1,
			Finished = 3
		};
		enum Result
		{
			None = 0,
			Timeout = 1,
			NetworkProblem = 2,
			NotFound = 3,
			Found = 4,
			KeyProblem = 5
		};

	signals:
		void stateChanged(ViSongDetector::State state);

	private slots:
		void bufferChanged(int size);
		void codeFinished(QString code, QString version, int codeLength);
		void replyFinished(QNetworkReply *reply);
		void downloadFinished(QNetworkReply *reply);
		void encodingStateChanged(ViCoder::State state);

	public:
		ViSongDetector(ViAudioOutput *output);
		~ViSongDetector();
		void setProxy(QNetworkProxy::ProxyType type, QString host, quint16 port, QString username, QString password);
		void setKey(QString key);
		ViSongDetector::State state();
		ViSongDetector::Result result();
		ViSongInfo songInfo();

	private:
		QByteArray mOutput;
		ViCoder mCoder;

		ViEchoNestResponse mResponse;
		ViSongCodeGeneratorThread *mThread;
		ViAudioOutput *mAudioOutput;

		QNetworkAccessManager *mNetworkManager;

		QString mKey;
		qint64 mBufferSize;
		ViSongDetector::State mState;
		ViSongDetector::Result mResult;
		ViSongInfo mSongInfo;
		qint8 mRequestsSent;
};

#endif
