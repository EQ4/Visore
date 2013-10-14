#ifndef VIPOLYNOMIALINTERPOLATOR_H
#define VIPOLYNOMIALINTERPOLATOR_H

#include <viinterpolator.h>

class ViPolynomialInterpolator : public ViAutoDegreeInterpolator
{

	public:

		ViPolynomialInterpolator();
		ViPolynomialInterpolator(const int &degree);

		virtual ViPolynomialInterpolator* clone();

	protected:

		virtual bool interpolateSamples(const qreal *leftSamples, const int &leftSize, const qreal *rightSamples, const int &rightSize, qreal *outputSamples, const int &outputSize);

};

#endif
