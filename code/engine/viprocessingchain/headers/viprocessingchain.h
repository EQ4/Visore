#ifndef VIPROCESSINGCHAIN_H
#define VIPROCESSINGCHAIN_H

#include <QQueue>
#include <QTime>
#include <QTimer>
#include "viaudioconnection.h"
#include "vimultiexecutor.h"
#include "viprojectmanager.h"
#include "viaudioobject.h"

class ViHandler;
class ViUnderrunHandler;
class ViProjectHandler;
class ViSectionHandler;

class ViProcessingChain : public QObject
{
    Q_OBJECT

	friend class ViHandler;
	friend class ViUnderrunHandler;
	friend class ViProjectHandler;
	friend class ViSectionHandler;

	signals:

		void changed();
		void progress(short progress);
		void statusChanged(QString status);

		void inputChanged();
		void outputChanged();
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
		void setProject(ViProject *project, ViAudioFormat format);
		bool attach(ViAudio::Mode mode, ViProcessor *processor);
		bool detach(ViProcessor *processor);
		
		ViAudioObject* dequeueObject();

	protected:

		void enqueueObject(ViAudioObject *object);
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
		ViProjectHandler *mProjectHandler;
		ViSectionHandler *mSectionHandler;

		QQueue<ViAudioObject*> mAudioObjects;
};

#endif
