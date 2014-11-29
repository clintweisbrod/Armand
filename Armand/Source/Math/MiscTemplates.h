#pragma once

template<class T> void constrain(T& ioVal, T inMin, T inMax)
{
	if (ioVal < inMin)
		ioVal = inMin;
	if (ioVal > inMax)
		ioVal = inMax;
}

template<class T> T linearInterpolate(T inX1, T inX2, T inY1, T inY2, T inX)
{
	if (fabs((double)inX2 - (double)inX1) < 1e-6)
		return inY1;

	T result = (T)(((double)(inY2 - inY1) / (double)(inX2 - inX1))*(double)inX + ((double)(inX2*inY1 - inX1*inY2) / (double)(inX2 - inX1)));

	// Constrain result between inY1 and inY2
	T minVal = fmin((double_t)inY1, (double_t)inY2);
	T maxVal = fmax((double_t)inY1, (double_t)inY2);
	if (result < minVal)
		result = minVal;
	if (result > maxVal)
		result = maxVal;

	return result;
}

template<class T> T linearInterpolateNoConstrain(T inX1, T inX2, T inY1, T inY2, T inX)
{
	if (fabs((double)inX2 - (double)inX1) < 1e-6)
		return inY1;

	return (T)(((double)(inY2 - inY1) / (double)(inX2 - inX1))*(double)inX + ((double)(inX2*inY1 - inX1*inY2) / (double)(inX2 - inX1)));
}