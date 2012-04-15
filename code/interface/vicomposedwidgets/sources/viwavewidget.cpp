#include "viwavewidget.h"

ViWaveWidgetThread::ViWaveWidgetThread(ViWaveWidget *widget)
	: QThread()
{
	mWidget = widget;
	mPosition = -1;
}

void ViWaveWidgetThread::run()
{
	mMutex.lock();
	bool isEmpty = mSizes.isEmpty();
	mMutex.unlock();
	while(!isEmpty)
	{

		mMutex.lock();
//cout<<"p1: "<<mChunks.size();
		int size = mSizes.takeFirst();
mMutex.unlock();

ViAudioBufferChunk chunk;
s->read(&chunk, size);
ViWaveFormChunk wave;
former.pcmToReal(&chunk, &wave);



//cout<<"   p2: "<<mChunks.size()<<endl;
	

		qreal *data = wave.data();
		int chunkSize = wave.size();

		for(int i = 0; i < chunkSize; ++i)
		{
			//mFormMutex.lock();
			mForm.append(data[i]);
			//mFormMutex.unlock();
			/*mFormMutex.lock();
			for(int j = 0; j < mForms.size(); ++j)
			{
				mForms[j].append(data[i]);
			}
			mFormMutex.unlock();*/
		}


		emit tileAvailable();
		mMutex.lock();
		isEmpty = mSizes.isEmpty();
		mMutex.unlock();
	}
	//emit tileAvailable();
}

void ViWaveWidgetThread::analyze(int size)
{
	mMutex.lock();
	mSizes.append(size);
	mMutex.unlock();
	if(!isRunning())
	{
		start();
	}
}

void ViWaveWidgetThread::changed(QSharedPointer<ViWaveFormChunk> chunk)
{

}

void ViWaveWidgetThread::positionChanged(ViAudioPosition position)
{
	mPosition = position.sample();
	emit tileAvailable();
}

ViWaveWidget::ViWaveWidget(ViAudioEngine *engine, ViAudioBuffer::ViAudioBufferType type, QWidget *parent)
	: ViWidget(parent)
{
	setEngine(engine);
	mToolbar = new ViWidgetToolbar(ViWidgetToolbar::Right, parent);
	mToolbar->setEngine(engine);
	mToolbar->addButton("Zoom In", ViThemeManager::image("zoomin.png", ViThemeImage::Normal, ViThemeManager::Icon).icon(), this, SLOT(zoomIn()));
	mToolbar->addButton("Zoom Out", ViThemeManager::image("zoomout.png", ViThemeImage::Normal, ViThemeManager::Icon).icon(), this, SLOT(zoomOut()));
	mThread = new ViWaveWidgetThread(this);
	mThread->mBufferType = type;

mThread->s = engine->mProcessingChain->originalBuffer()->createReadStream();

	if(type == ViAudioBuffer::Original)
	{
		ViObject::connect(mEngine, SIGNAL(originalBufferChanged(int)), mThread, SLOT(analyze(int)));
	}
	else
	{
		ViObject::connect(mEngine, SIGNAL(correctedBufferChanged(int)), mThread, SLOT(analyze(int)));
	}

	ViObject::connect(mEngine, SIGNAL(positionChanged(ViAudioPosition)), mThread, SLOT(positionChanged(ViAudioPosition)));
	ViObject::connectQueued(mThread, SIGNAL(tileAvailable()), this, SLOT(repaint()));
	mZoomLevel = 15;
}

ViWaveWidget::~ViWaveWidget()
{
	mThread->quit();
	delete mThread;
	delete mToolbar;
}

void ViWaveWidget::paintEvent(QPaintEvent *event)
{
	resize(mParent->width(), mParent->height());
	QPainter painter(this);

	static QPen penNormal(ViThemeManager::color(15));
	static QPen penAverage(ViThemeManager::color(14));
	static QPen penPosition(ViThemeManager::color(12));

	qreal ratio = UNSIGNED_CHAR_HALF_VALUE / (height() / 2.0);

	int halfHeight = height() / 2;
	int halfWidth = width() / 2;
	int position = mThread->mPosition / (FIRST_ZOOM_LEVEL * qPow(ZOOM_LEVEL_INCREASE, mZoomLevel)); //(FIRST_ZOOM_LEVEL * (mZoomLevel + 1));
	int start = position - halfWidth;
	int end = position + halfWidth;

	//mThread->mFormMutex.lock();
	int zoomSize = mThread->mForm.size(mZoomLevel);
	bool underCutOff = mThread->mForm.isUnderCutoff(mZoomLevel);
	//mThread->mFormMutex.unlock();

	if(start < 0)
	{
		start = 0;
	}
	if(end > zoomSize)
	{
		end = zoomSize;
	}
	int drawStart = halfWidth + (start - position);

	if(underCutOff)
	{
		int previous = halfHeight;
		for(int i = start; i < end; ++i)
		{
			painter.setPen(penNormal);
			//mThread->mFormMutex.lock();
			int now = mThread->mForm.maximum(i, mZoomLevel) / ratio;
			//mThread->mFormMutex.unlock();
			painter.drawLine(drawStart, previous, drawStart + 1, now);
			previous = now;
			drawStart++;
		}
	}
	else
	{
		for(int i = start; i < end; ++i)
		{
			painter.setPen(penNormal);
			//mThread->mFormMutex.lock();
			painter.drawLine(drawStart, halfHeight, drawStart, mThread->mForm.maximum(i, mZoomLevel) / ratio);
			painter.drawLine(drawStart, halfHeight, drawStart, mThread->mForm.minimum(i, mZoomLevel) / ratio);
			//mThread->mFormMutex.unlock();

			painter.setPen(penAverage);
			//mThread->mFormMutex.lock();
			painter.drawLine(drawStart, halfHeight, drawStart, mThread->mForm.maximumAverage(i, mZoomLevel) / ratio);
			painter.drawLine(drawStart, halfHeight, drawStart, mThread->mForm.minimumAverage(i, mZoomLevel) / ratio);
			//mThread->mFormMutex.unlock();
			drawStart++;
		}
	}

	painter.setPen(penPosition);
	painter.drawLine(0, halfHeight, width(), halfHeight);

	QRect rectangle(0, 0, halfWidth, height());
	QColor color = ViThemeManager::color(11);
	color = QColor(color.red(), color.green(), color.blue(), 100);
	painter.fillRect(rectangle, color);

	painter.drawLine(halfWidth, 0, halfWidth, height());
}

void ViWaveWidget::resizeEvent(QResizeEvent *event)
{
	mToolbar->refresh();
}

void ViWaveWidget::enterEvent(QEvent *event)
{
	mToolbar->show();
}

void ViWaveWidget::leaveEvent(QEvent *event)
{
	QPoint mouse = QCursor::pos();
	QPoint position = mapToGlobal(pos());
	if(mouse.x() < position.x() || mouse.x() > position.x() + width() || mouse.y() < position.y() || mouse.y() > position.y() + height())
	{
		mToolbar->hide();
	}
}

void ViWaveWidget::zoomIn()
{
	qint8 level = mZoomLevel - 1;
	if(level >= 0)
	{
		mZoomLevel = level;
	}
	repaint();
}

void ViWaveWidget::zoomOut()
{
	qint8 level = mZoomLevel + 1;
	if(level < MAXIMUM_ZOOM_LEVELS)
	{
		mZoomLevel = level;
	}
	repaint();
}
