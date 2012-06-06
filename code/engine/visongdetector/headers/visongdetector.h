#ifndef VISONGDETECTOR_H
#define VISONGDETECTOR_H

#include <QUrl>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include "viaudiobuffer.h"
#include "viaudiooutput.h"
#include "viaudioposition.h"
#include "visonginfo.h"
#include "viobject.h"
#include "vipcmconverter.h"
#include "viechonestresponse.h"
#include "vicoder.h"

class ViSongCodeGeneratorThread : public QThread
{
	Q_OBJECT

	signals:
		void finished(QString code, QString version, int codeLength);

	public:
		ViSongCodeGeneratorThread(QObject *parent = 0);
		void setData(QByteArray *data);
		void setOffset(int offset);

	protected:
		void run();

	private:
		QByteArray *mData;
		int mOffset;
};

class ViSongDetector : public QObject
{
	Q_OBJECT

	public:
		enum State
		{
			Idle = 0,
			Encoding = 1,
			CodeGeneration = 2,
			SongDetection = 3,
			ImageRetrieval = 4
		};
		enum Error
		{
			NoError = 0,
			NetworkError = 1,
			KeyError = 2
		};

	signals:
		void stateChanged(ViSongDetector::State state);
		void songFound(ViSongInfo info);

	private slots:
		void bufferChanged(int size = -1);
		void codeFinished(QString code, QString version, int codeLength);
		void replyFinished(QNetworkReply *reply);
		void downloadFinished(QNetworkReply *reply);
		void encodingStateChanged(ViCoder::State state);

	public:
		ViSongDetector(ViAudioOutput *output);
		~ViSongDetector();
		void reset();
		void setProxy(QNetworkProxy::ProxyType type, QString host, quint16 port, QString username, QString password);
		void setKey(QString key);
		ViSongDetector::State state();
		ViSongDetector::Error error();
		QNetworkReply::NetworkError networkError();
		QList<ViSongInfo> songInfo();

	private:
		void setState(ViSongDetector::State state);

	private:
		QByteArray *mOutput;
		ViCoder mCoder;

		ViEchoNestResponse mResponse;
		ViSongCodeGeneratorThread mThread;
		ViAudioOutput *mAudioOutput;

		QNetworkAccessManager *mNetworkManager;

		QString mKey;

		bool mFound;
		ViSongDetector::State mState;
		ViSongDetector::Error mError;
		QNetworkReply::NetworkError mNetworkError;

		qint8 mRequestsSent;
};

#endif
