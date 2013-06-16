#ifndef VIAUDIOOBJECT_H
#define VIAUDIOOBJECT_H

#include <viaudio.h>
#include <vifunctor.h>
#include <vibuffer.h>
#include <vielement.h>
#include <viaudiocoder.h>
#include <vilogger.h>
#include <vialigner.h>
#include <visongidentifier.h>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>

class ViWaveForm;
class ViWaveFormer;
class ViMetadataer;
class ViAudioObject;
class ViModifyProcessor;
class ViDualProcessor;

typedef ViPointer<ViAudioObject> ViAudioObjectPointer;
typedef QList<QList<ViAudioObjectPointer> > ViAudioObjectMatrix;
typedef QList<ViAudioObjectPointer> ViAudioObjectList;
typedef QQueue<ViAudioObjectPointer> ViAudioObjectQueue;

class ViAudioObject : public QObject, public ViFunctorParameter, public ViId
{

    Q_OBJECT

	public:

		/*******************************************************************************************************************

			ENUMERATIONS

		*******************************************************************************************************************/

		//Values are important
		enum Type
		{
			Undefined = 0x1,
			Target = 0x2,
			Corrupted = 0x4,
			Corrected = 0x8,
			Temporary = 0x10,
			Correlated = Corrected,
			Temp = Temporary,
			All = Target | Corrupted | Corrected | Temporary
		};

		//Values are important
		enum Resource
		{
			None = 0x1,
			File = 0x2,
			Buffer = 0x4,
			Both = File | Buffer
		};

	signals:

		void finished();

		void progressed(qreal percentage);
		void statused(QString status);

		void decoded();
		void encoded();

		void aligned();

		void waved();

		void corrected();

        void correlated();

		void infoed(bool success);

	public slots:

		/*******************************************************************************************************************

			FORMATS

		*******************************************************************************************************************/

		void setOutputFormat(ViAudioFormat format);

        void setFormat(ViAudioObject::Type type, ViAudioFormat format);
        void setTargetFormat(ViAudioFormat format);
        void setCorruptedFormat(ViAudioFormat format);
        void setCorrectedFormat(ViAudioFormat format);

	private slots:

		/*******************************************************************************************************************

			LOGGING

		*******************************************************************************************************************/

		void log(QString message, QtMsgType type = QtDebugMsg);
		void logStatus(QString message, QtMsgType type = QtDebugMsg); //Logs and emits status

		/*******************************************************************************************************************

			PROGRESS

		*******************************************************************************************************************/

		void progress(qreal progress);

		/*******************************************************************************************************************

			ENCODE & DECODE SLOTS

		*******************************************************************************************************************/

		void encodeNext();
		void decodeNext();

		/*******************************************************************************************************************

			ALIGN

		*******************************************************************************************************************/

		void alignNext();

		/*******************************************************************************************************************

			WAVEFORM

		*******************************************************************************************************************/

		void initializeWaveForm();
		void generateNextWaveForm();

		/*******************************************************************************************************************

			CORRECTION

		*******************************************************************************************************************/

		void endCorrect();

        /*******************************************************************************************************************

            CORRELATE

        *******************************************************************************************************************/

        void correlateNext();

		/*******************************************************************************************************************

			SONG INFO

		*******************************************************************************************************************/

		void finishDetection(bool success);

	public:

		/*******************************************************************************************************************

			CONSTRUCTORS, DESTRUCTORS & GENERAL

		*******************************************************************************************************************/

		static ViAudioObjectPointer create(ViAudioObject *object);
		static ViAudioObjectPointer create(bool autoDestruct = true);
		static ViAudioObjectPointer createNull();
		~ViAudioObject();

		static QQueue<ViAudioObject::Type> decomposeTypes(ViAudioObject::Type type, ViAudioObject::Type exclude = ViAudioObject::Undefined);
		static ViAudioObject::Type composeTypes(QQueue<ViAudioObject::Type> &types, ViAudioObject::Type exclude = ViAudioObject::Undefined);

		bool hasResource(ViAudioObject::Type type);
		ViAudioObject::Resource resourceAvailable(ViAudioObject::Type type);
		ViAudioObject::Type availableResources(ViAudioObject::Resource resource = ViAudioObject::Both);

		qreal length(ViAudioPosition::Unit unit = ViAudioPosition::Bytes);
		qreal length(ViAudioObject::Type type, ViAudioPosition::Unit unit = ViAudioPosition::Bytes);
		
		/*******************************************************************************************************************

			AUTO DESTRUCT

		*******************************************************************************************************************/

		void setAutoDestruct(bool destruct);
		void addDestructRule(ViAudioObject::Type type, bool destruct);

		/*******************************************************************************************************************

			ENCODE & DECODE

		*******************************************************************************************************************/

		void setEncoder(ViAudioCoder *coder); //Takes ownership
		bool hasEncoder();
        Q_INVOKABLE bool encode(int type);
		Q_INVOKABLE bool encode(ViAudioFormat format, bool clearWhenFinished = false);
		Q_INVOKABLE bool encode(ViAudioObject::Type type, ViAudioFormat format, bool clearWhenFinished = false);
		Q_INVOKABLE bool encode(ViAudioObject::Type type = ViAudioObject::All, bool clearWhenFinished = false);
		
		void setDecoder(ViAudioCoder *coder); //Takes ownership
		bool hasDecoder();
        Q_INVOKABLE bool decode(int type);
        Q_INVOKABLE bool decode(ViAudioObject::Type type = ViAudioObject::All);

		/*******************************************************************************************************************

			ALIGN

		*******************************************************************************************************************/

		void setAligner(ViAligner *aligner); //Takes ownership
		bool hasAligner();
		Q_INVOKABLE bool align(ViAligner *aligner = NULL); //Takes ownership

		/*******************************************************************************************************************

			INPUT & OUTPUT

		*******************************************************************************************************************/

		void setType(ViAudioObject::Type input, ViAudioObject::Type output);
		void setInputType(ViAudioObject::Type type);
		void setOutputType(ViAudioObject::Type type);

		ViAudioObject::Type inputType();
		ViAudioObject::Type outputType();
		
		/*******************************************************************************************************************

			BUFFERS

		*******************************************************************************************************************/

		// Transfer a certain buffer from object to this object.
		// If type is UnknownType, the output buffer will be used.
		// The ownership of the buffer will be transfered to this object, hence object will not delete it.
		// The file path is also transfered.
		void transferBuffer(ViAudioObjectPointer object, ViAudioObject::Type type = ViAudioObject::Undefined);

		ViBuffer* buffer(ViAudioObject::Type type, bool dontCreate = false); // dontCreate will not create buffers if they are NULL
		ViBuffer* targetBuffer(bool dontCreate = false);
		ViBuffer* corruptedBuffer(bool dontCreate = false);
		ViBuffer* correctedBuffer(bool dontCreate = false);
		ViBuffer* temporaryBuffer(bool dontCreate = false);

		ViBuffer* inputBuffer(bool dontCreate = false); // returns the buffer that will be used as input for the processing chain
		ViBuffer* outputBuffer(bool dontCreate = false); // returns the buffer that will be used as output for the processing chain

		void setBuffer(ViAudioObject::Type type, ViBuffer *buffer);
		void setTargetBuffer(ViBuffer *buffer);
		void setCorruptedBuffer(ViBuffer *buffer);
		void setCorrectedBuffer(ViBuffer *buffer);

		Q_INVOKABLE void clearBuffers(ViAudioObject::Type type = ViAudioObject::All);
		Q_INVOKABLE void clearBuffer(ViAudioObject::Type type);
		Q_INVOKABLE void clearTargetBuffer();
		Q_INVOKABLE void clearCorruptedBuffer();
		Q_INVOKABLE void clearCorrectedBuffer();
		Q_INVOKABLE void clearTemporaryBuffer();

		bool hasBuffer(ViAudioObject::Type type);
		bool hasInputBuffer();
		bool hasOutputBuffer();

		/*******************************************************************************************************************

			FILES

		*******************************************************************************************************************/

		QString filePath(ViAudioObject::Type type);
		QString targetFilePath();
		QString corruptedFilePath();
		QString correctedFilePath();

		void setFilePath(ViAudioObject::Type type, QString path);
		void setTargetFilePath(QString path);
		void setCorruptedFilePath(QString path);
		void setCorrectedFilePath(QString path);

		bool hasFile(ViAudioObject::Type type);

		QString fileName(bool track = true, bool side = false);
		QString temporaryFilePath(ViAudioObject::Type type);

		/*******************************************************************************************************************

			FORMATS

		*******************************************************************************************************************/

		ViAudioFormat format(ViAudioObject::Type type);
		ViAudioFormat targetFormat();
		ViAudioFormat corruptedFormat();
		ViAudioFormat correctedFormat();
		ViAudioFormat inputFormat();
		ViAudioFormat outputFormat();

		/*******************************************************************************************************************

			WAVEFORM

		*******************************************************************************************************************/

		bool generateWaveForm(ViAudioObject::Type types);
		void setWaveForm(ViAudioObject::Type type, ViWaveForm *form);
		ViWaveForm* waveForm(ViAudioObject::Type type);

        /*******************************************************************************************************************

            CORRECTION

        *******************************************************************************************************************/

        void setCorrector(ViModifyProcessor *corrector); //Takes ownership
        bool hasCorrector();
        Q_INVOKABLE bool correct(ViModifyProcessor *corrector = NULL); //Takes ownership

        /*******************************************************************************************************************

            CORRELATE

        *******************************************************************************************************************/

        void clearCorrelators();
        void addCorrelator(ViDualProcessor *correlator); //Takes ownership
        bool hasCorrelator();
        int correlatorCount();
        Q_INVOKABLE bool correlate(ViDualProcessor *correlator); //Takes ownership
        Q_INVOKABLE bool correlate(QList<ViDualProcessor*> correlators); //Takes ownership
        Q_INVOKABLE bool correlate();

		/*******************************************************************************************************************

			SONG INFO

		*******************************************************************************************************************/

		ViSongInfo& songInfo();
		void setSongInfo(ViSongInfo info);
		void detectSongInfo();
		bool isDetectingSongInfo();

		void setSideNumber(int side);
		int sideNumber();

		void setTrackNumber(int track);
		int trackNumber();
















		void setSong(bool song = true); //If the buffers represent a song, or if they are just intermediate buffers

		bool isSong();

		bool isFinished();
		bool isUsed(QIODevice::OpenMode mode = QIODevice::ReadWrite);

	private:

		/*******************************************************************************************************************

			CONSTRUCTORS, DESTRUCTORS & GENERAL

		*******************************************************************************************************************/

		ViAudioObject(bool autoDestruct); // autoDestruct determines if the buffers will be deleted automatically.

		/*******************************************************************************************************************

			PROGRESS

		*******************************************************************************************************************/

		void setProgress(qreal parts);

	private:

		ViAudioObjectPointer thisPointer; // Keep a ViPointer to own object. Needed for passing it to ViProcessor

		ViAudioObject::Type mInputType;
		ViAudioObject::Type mOutputType;
		ViAudioObject::Type mDestructType;

		ViBuffer *mTargetBuffer;
		ViBuffer *mCorruptedBuffer;
		ViBuffer *mCorrectedBuffer;
		ViBuffer *mTemporaryBuffer;

		QString mTargetFile;
		QString mCorruptedFile;
		QString mCorrectedFile;

		QMutex mMutex;
		bool mIsFinished;
		bool mIsSong;

		/*******************************************************************************************************************

			PROGRESS

		*******************************************************************************************************************/

		qreal mProgressParts;
		qreal mProgress;

		/*******************************************************************************************************************

			FORMATS

		*******************************************************************************************************************/

        ViAudioFormat mOutputFormat;

        ViAudioFormat mTargetFormat;
        ViAudioFormat mCorruptedFormat;
        ViAudioFormat mCorrectedFormat;

		/*******************************************************************************************************************

			ENCODE & DECODE

		*******************************************************************************************************************/

		ViAudioCoder *mEncoder;
		ViAudioCoder *mDecoder;

		QQueue<ViAudioObject::Type> mCodingInstructions;
		bool mClearEncodedBuffer;
		ViAudioObject::Type mPreviousEncodedType;

		/*******************************************************************************************************************

			ALIGN

		*******************************************************************************************************************/

		ViAligner *mAligner;
		QQueue<ViAudioObject::Type> mAlignerInstructions;
		ViAudioObject::Type mAlignerTypes;
		ViAudioObject::Type mMainAligner;

		/*******************************************************************************************************************

			WAVEFORM

		*******************************************************************************************************************/

		ViWaveFormer *mWaveFormer;
		QQueue<ViAudioObject::Type> mWaveInstructions;
		QMap<ViAudioObject::Type, ViWaveForm*> mWaveForms;

		/*******************************************************************************************************************

			CORRECTION

		*******************************************************************************************************************/

		ViModifyProcessor *mCorrector;

        /*******************************************************************************************************************

            CORRELATE

        *******************************************************************************************************************/

        QList<ViDualProcessor*> mCorrelators;
        QQueue<QPair<ViAudioObject::Type, ViAudioObject::Type> > mCorrelations;
        int mCurrentCorrelator;

		/*******************************************************************************************************************

			SONG INFO

		*******************************************************************************************************************/

		ViMetadataer *mMetadataer;
		ViMetadata mMetadata;
		ViSongInfo mSongInfo;
		bool mIsDetectingInfo;
		int mSideNumber;
		int mTrackNumber;

};

#endif
