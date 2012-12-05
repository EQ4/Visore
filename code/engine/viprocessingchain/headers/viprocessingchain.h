#ifndef VIPROCESSINGCHAIN_H
#define VIPROCESSINGCHAIN_H

#include <QQueue>
#include <QTime>
#include <QTimer>
#include "viaudioconnection.h"
#include "vimultiexecutor.h"
#include "viaudioobjectqueue.h"
#include "viproject.h"

class ViHandler;
class ViUnderrunHandler;
class ViSectionHandler;
class ViPlaybackHandler;
class ViProjectHandler;

class ViProcessingChain : public QObject
{
    Q_OBJECT

	friend class ViHandler;
	friend class ViUnderrunHandler;
	friend class ViSectionHandler;
	friend class ViPlaybackHandler;
	friend class ViProjectHandler;

	signals:

		void changed();
		void progressStarted();
		void progress(short progress);
		void progressFinished();
		void statusChanged(QString status);

		void inputChanged();
		void streamOutputChanged(ViStreamOutput *output);
		void fileOutputChanged(ViFileOutput *output);
		void attached(ViProcessor *processor);

		void songStarted();
		void songEnded();
		void recordStarted();
		void recordEnded();

	public:

		ViProcessingChain();
		~ViProcessingChain();

		void setWindowSize(int windowSize);
		void setTransmission(ViAudioTransmission *transmission);
		void startProject(ViProject *project, ViAudio::Type type, bool existingProject);
		bool attach(ViAudio::Mode mode, ViProcessor *processor);
		bool detach(ViProcessor *processor);

		ViAudioObjectPointer audioObject();

		ViProcessor* processor(QString type);

	protected:

		bool isSongRunning();
		bool wasSongRunning();

	private:

		QMutex mMutex;

		ViMultiExecutor mMultiExecutor;
		ViAudioInput *mInput;
		ViStreamOutput *mStreamOutput;
		ViFileOutput *mFileOutput;

		QList<ViHandler*> mHandlers;
		ViUnderrunHandler *mUnderrunHandler;
		ViSectionHandler *mSectionHandler;
		ViPlaybackHandler *mPlaybackHandler;
		ViProjectHandler *mProjectHandler;


		ViAudioObjectQueue mAudioObjects;

};

#endif
