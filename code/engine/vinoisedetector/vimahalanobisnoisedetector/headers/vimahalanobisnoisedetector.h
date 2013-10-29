#ifndef VIMAHALANOBISNOISEDETECTOR_H
#define VIMAHALANOBISNOISEDETECTOR_H

#include <vinoisedetector.h>

// http://www.utexas.edu/courses/schwab/sw388r7/SolvingProblems/DetectingOutliers.ppt
// http://mathworld.wolfram.com/CovarianceMatrix.html
// http://feelmare.blogspot.com/2013/02/covariance-matrix-example.html

class ViMahalanobisNoiseDetector : public ViNoiseDetector
{

    public:

		ViMahalanobisNoiseDetector();
		ViMahalanobisNoiseDetector* clone();

	protected:

		void calculateNoise(QQueue<qreal> &samples);

};

#endif
