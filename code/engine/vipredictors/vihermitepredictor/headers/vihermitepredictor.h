#ifndef VIHERMITEPREDICTOR_H
#define VIHERMITEPREDICTOR_H

#include <vipredictor.h>

class ViHermitePredictor : public ViPredictor
{

	public:

		ViHermitePredictor();
		ViHermitePredictor(const ViHermitePredictor &other);
		~ViHermitePredictor();

		void setDegree(const int &degree);

		void setParameter(const int &number, const qreal &value);

	protected:

		void predict(const qreal *samples, const int &size, qreal *predictedSamples, const int &predictionCount);

	private:

		int mDegree;

};

#endif
