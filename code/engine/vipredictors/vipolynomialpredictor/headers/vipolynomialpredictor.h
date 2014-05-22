#ifndef VIPOLYNOMIALPREDICTOR_H
#define VIPOLYNOMIALPREDICTOR_H

#include <vipredictor.h>

class ViPolynomialPredictor : public ViModelPredictor
{

	public:

		ViPolynomialPredictor(const Estimation &estimation = Fixed);
		ViPolynomialPredictor(const ViPolynomialPredictor &other);
		~ViPolynomialPredictor();

		void setParameter(const int &number, const qreal &value);

	protected:

		bool estimateModel(const int &degree, ViVector &coefficients, const qreal *samples, const int &size);
		void predictModel(const int &degree, const ViVector &coefficients, qreal *prediction, const int &size, const int &start);

};

#endif
