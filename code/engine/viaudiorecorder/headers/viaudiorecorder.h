#ifndef VIAUDIORECORDER_H
#define VIAUDIORECORDER_H

#include <vinotifier.h>
#include <viproject.h>
#include <vistreaminput.h>
#include <vifrequencysegmentdetector.h>

class ViAudioRecorder : public ViNotifier
{

	Q_OBJECT

	private slots:

		void startRecord();
		void endRecord();
		void startSong();
		void endSong();
		void serialize();
		void finish();
		void checkSize();

	public:

		ViAudioRecorder();
		~ViAudioRecorder();

		bool record(ViProject *project, ViAudioObject::Type type, bool detectInfo);
		bool record(ViProject *project, ViAudioObject::Type type, bool detectInfo, ViAudioFormat format);

	protected:

		void nextObject(bool startTimer = true);

	private:

		ViAudioObjectPointer mObject;
		ViAudioObjectQueue mQueue;

		ViStreamInput mInput;
		ViFrequencySegmentDetector mSegmentDetector;

		ViProject *mProject;
		ViAudioObject::Type mType;

		QTimer mIdleTimer;

		bool mDetectInfo;

};

#endif