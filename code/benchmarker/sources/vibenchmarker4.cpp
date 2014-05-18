#include <vibenchmarker4.h>
#include <viaudiodata.h>
#include <vinoisecreator.h>
#include <vilogger.h>
#include <QTextStream>
#include <iomanip>

#include <vicosineinterpolator.h>
#include <vipolynomialinterpolator.h>
#include <visplineinterpolator.h>
#include <vihermiteinterpolator.h>
#include <vifourierinterpolator.h>
#include <viarmainterpolator.h>

#define WINDOW_SIZE 4096
#define MASK_START 0
#define MASK_END 1
//#define MASK_INTERVAL 0.0001
#define MASK_INTERVAL 0.001
#define NOISE_TYPE Direct

#define NO_CHANGE 50

ViBenchMarker4::ViBenchMarker4()
{
	mCurrentObject = ViAudioObject::create();
	mMainTime.start();

	//mInterpolator = new ViCosineInterpolator();
	//mInterpolator = new ViPolynomialInterpolator();
	//mInterpolator = new ViSplineInterpolator();
	mInterpolator = new ViArmaInterpolator();
	//mInterpolator = new ViHermiteInterpolator();
	//mInterpolator = new ViFourierInterpolator();

	addParam("WINDOW SIZE", 1024, 1024, 1);
	addParam("ARDEGREE", 5, 5, 1);
	addParam("MADEGREE", 0, 0, 1);

	/*addParam("WINDOW SIZE", 128, 128, 1);
	addParam("DEGREE", 9,9, 1);*/
}

ViBenchMarker4::~ViBenchMarker4()
{
}

void ViBenchMarker4::addParam(QString name, qreal start, qreal end, qreal increase)
{
	mParamsName.append(name);
	mParamsStart.append(start);
	mParamsEnd.append(end);
	mParamsIncrease.append(increase);
	mParamsCurrent.append(0);
}

void ViBenchMarker4::initParams()
{
	mDoneParamIterations = 0;
	mTotalParamIterations = 1;
	for(int i = 0; i < mParamsStart.size(); ++i)
	{
		mTotalParamIterations *= (mParamsEnd[i] - mParamsStart[i] + mParamsIncrease[i]) / mParamsIncrease[i];
	}
}

bool ViBenchMarker4::nextParam()
{
	int size = mParamsStart.size();

	bool finished = true;
	for(int i = 0; i < size; ++i)
	{
		if(mParamsCurrent[i] < mParamsEnd[i])
		{
			finished = false;
			break;
		}
	}
	if(finished) return false; //All paramaters were tested

	for(int i = size - 1; i >= 0; --i)
	{
		if(mParamsCurrent[i] < mParamsEnd[i])
		{
			mParamsCurrent[i] += mParamsIncrease[i];
			return true;
		}
		else if(mParamsCurrent[i] >= mParamsEnd[i])
		{
			mParamsCurrent[i] = mParamsStart[i];
			int pre = 1;
			while(mParamsCurrent[i-pre] >= mParamsEnd[i-pre])
			{
				mParamsCurrent[i-pre] = mParamsStart[i-pre];
				++pre;
			}
			mParamsCurrent[i-pre] += mParamsIncrease[i-pre];
			return true;
		}
	}
}

void ViBenchMarker4::benchmark()
{
	QDir dir("/home/visore/Visore Projects/Files/");
	QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	for(int i = 0; i < dirs.size(); ++i)
	{
		QDir dir3("/home/visore/Visore Projects/Results/"+dirs[i]);
		if(!dir3.exists()) dir3.mkpath(dir3.absolutePath());

		QDir dir2(dir.absoluteFilePath(dirs[i]));
		QStringList files2 = dir2.entryList(QDir::Files);
		for(int j = 0; j < files2.size(); ++j)
		{
			mFiles.enqueue(dir2.absoluteFilePath(files2[j]));
			mResults.enqueue(dir3.absoluteFilePath(files2[j])+".txt");
		}
	}
	mTotalFiles = mFiles.size();
	nextFile();
}

void ViBenchMarker4::nextFile()
{
	if(mFiles.isEmpty())
	{
		quit();
	}
	else
	{
		initParams();
		for(int i = 0; i < mParamsStart.size(); ++i)
		{
			mParamsCurrent[i] = mParamsStart[i];
		}

		mCurrentFile = mFiles.dequeue();

		mOutputFile.close();
		mOutputFile.setFileName(mResults.dequeue());
		mOutputFile.open(QIODevice::WriteOnly);
		mOutputStream.setDevice(&mOutputFile);

		mOutputStream<<mInterpolator->name()<<"\n\n";
		mOutputStream << QFileInfo(mCurrentFile).fileName();
		mOutputStream<<"\n\n";

		for(int i = 0; i < mParamsName.size(); ++i)
		{
			mOutputStream << "PARAMETER "<<(i+1) << " (" << mParamsName[i] << ")\t";
		}
		mOutputStream << "ACCURACY" << "\t" << "TIME" << "\n";

		mCurrentObject->clearBuffers();
		mCurrentObject.setNull();
		mCurrentObject = ViAudioObject::create();

		mCurrentObject->setFilePath(ViAudio::Target, mCurrentFile);
		QObject::connect(mCurrentObject.data(), SIGNAL(decoded()), this, SLOT(process()));
		mCurrentObject->decode();
	}
}

void ViBenchMarker4::process()
{
	QObject::disconnect(mCurrentObject.data(), SIGNAL(decoded()), this, SLOT(process()));

	ViNoiseCreator creator;
	creator.createNoise(mCurrentObject->buffer(ViAudio::Target), mCurrentObject->buffer(ViAudio::Corrupted), mCurrentObject->buffer(ViAudio::NoiseMask));

	int i;
	qint64 accuracyCount, time;
	qreal totalAccuracy, acc, percentageDone, remaining, best = 0;

	do
	{

		if(mParamsStart.size() == 1) mInterpolator->setParameters(mParamsCurrent[0]);
		else if(mParamsStart.size() == 2) mInterpolator->setParameters(mParamsCurrent[0], mParamsCurrent[1]);
		else if(mParamsStart.size() == 3) mInterpolator->setParameters(mParamsCurrent[0], mParamsCurrent[1], mParamsCurrent[2]);
		else if(mParamsStart.size() == 4) mInterpolator->setParameters(mParamsCurrent[0], mParamsCurrent[1], mParamsCurrent[2], mParamsCurrent[3]);
		//else { cout << "Invalid parameter count of "<<mParamsStart.size()<<". Min: 1, Max: 4" << endl; quit(); }

		ViAudioReadData target(mCurrentObject->buffer(ViAudio::Target));
		ViAudioReadData corrected(mCurrentObject->buffer(ViAudio::Corrected));
		ViAudioReadData noise(mCurrentObject->buffer(ViAudio::NoiseMask));
		target.setSampleCount(WINDOW_SIZE);
		corrected.setSampleCount(WINDOW_SIZE);
		noise.setSampleCount(WINDOW_SIZE);

		mInterpolator->interpolate(mCurrentObject->buffer(ViAudio::Corrupted), mCurrentObject->buffer(ViAudio::Corrected), mCurrentObject->buffer(ViAudio::NoiseMask));

		totalAccuracy = 0;
		accuracyCount = 0;

		// Write
		/*QObject::connect(mCurrentObject.data(), SIGNAL(encoded()), this, SLOT(quit()));
		mCurrentObject->encode(ViAudio::Corrected);
		return;*/

		mTime.restart();
		while(corrected.hasData())
		{
			target.read();
			ViSampleChunk &t1 = target.splitSamples(0);
			ViSampleChunk &t2 = target.splitSamples(1);

			corrected.read();
			ViSampleChunk &c1 = corrected.splitSamples(0);
			ViSampleChunk &c2 = corrected.splitSamples(1);

			noise.read();
			ViSampleChunk &n1 = noise.splitSamples(0);
			ViSampleChunk &n2 = noise.splitSamples(1);

			for(i = 0; i < n1.size(); ++i)
			{
				if(n1[i])
				{
					totalAccuracy += qAbs(c1[i] - t1[i]);
					++accuracyCount;
				}
			}
			for(i = 0; i < n2.size(); ++i)
			{
				if(n2[i])
				{
					totalAccuracy += qAbs(c2[i] - t2[i]);
					++accuracyCount;
				}
			}
		}
		time = mTime.elapsed();

		++mDoneParamIterations;
		acc = (2 - (totalAccuracy / accuracyCount)) / 2.0;
		if(acc > best) best = acc;
		percentageDone = mDoneParamIterations / double(mTotalParamIterations);
		remaining = mMainTime.elapsed();

		remaining = ((1.0/percentageDone) * remaining) - remaining;
		cout << int(percentageDone * 100.0) << "%\t("<<timeConversion(remaining).toLatin1().data()<<")\tAccuracy: "<<acc<<" ("<<best<<")\tTime: "<<time<<endl;

		for(i = 0; i < mParamsStart.size(); ++i)
		{
			mOutputStream << (int) mParamsCurrent[i] << "\t";
		}
		mOutputStream << acc << "\t" << time << "\n";
		mOutputStream.flush();
	}
	while(nextParam());

	nextFile();
}

QString ViBenchMarker4::timeConversion(int msecs)
{
	QString formattedTime;

	int days = msecs/(1000*60*60*24);
	int hours = (msecs-(days*1000*60*60*24))/(1000*60*60);
	int minutes = (msecs-(days*1000*60*60*24)-(hours*1000*60*60))/(1000*60);
	int seconds = (msecs-(days*1000*60*60*24)-(minutes*1000*60)-(hours*1000*60*60))/1000;

	formattedTime.append(QString("%1").arg(days, 2, 10, QLatin1Char('0')) + ":" +
						 QString("%1").arg(hours, 2, 10, QLatin1Char('0')) + ":" +
						 QString( "%1" ).arg(minutes, 2, 10, QLatin1Char('0')) + ":" +
						 QString( "%1" ).arg(seconds, 2, 10, QLatin1Char('0')));

	return formattedTime;
}


void ViBenchMarker4::quit()
{
	cout << "QUIT!" << endl;
	exit(0);
}
