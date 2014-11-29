// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// MiscTemplates.h
//
// Miscellaneous templated functions.
//
// THIS SOFTWARE IS PROVIDED BY CLINT WEISBROD "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL CLINT WEISBROD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------

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
	T minVal = fmin((double)inY1, (double)inY2);
	T maxVal = fmax((double)inY1, (double)inY2);
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