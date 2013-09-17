#ifndef VIHERMITEINTERPOLATOR_H
#define VIHERMITEINTERPOLATOR_H

#include <viinterpolator.h>

// http://cubic.org/docs/hermite.htm
// http://mathworld.wolfram.com/HermitesInterpolatingPolynomial.html
// http://paulbourke.net/miscellaneous/interpolation/
// http://mcs.cankaya.edu.tr/proje/2009/yaz/zeki_onur_urhan/sunum.pdf
// http://astro.temple.edu/~dhill001/course/numanalspring2010/Hermite%20Interpolation%20Section%205_7.pdf
// http://caig.cs.nctu.edu.tw/course/NM07S/slides/chap3_3.pdf

class ViHermiteInterpolator : public ViDegreeInterpolator
{

	public:

		ViHermiteInterpolator();

		ViHermiteInterpolator* clone();

	protected:

		bool interpolateSamples(const qreal *leftSamples, const int &leftSize, const qreal *rightSamples, const int &rightSize, qreal *outputSamples, const int &outputSize);

};

#endif
