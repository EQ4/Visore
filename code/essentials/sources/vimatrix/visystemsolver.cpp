#include <visystemsolver.h>
#include <vilogger.h>


ViVector ViSystemSolver::solve(const ViMatrix &matrix, const ViVector &vector)
{
	ViVector coefficients;
	solve(matrix, vector, coefficients);
	return coefficients;
}

bool ViSystemSolver::solve(const ViMatrix &matrix, const ViVector &vector, ViVector &coefficients)
{
	ViMatrix inverted, transpose = matrix.transpose();
	if(!transpose.scalarMultiply(matrix).invert(inverted))
	{
		STATICLOG("The matrix cannot be inverted.", QtCriticalMsg, "ViSystemSolver");
		return false;
	}
	coefficients = inverted.scalarMultiply(transpose) * vector;
	return true;
}