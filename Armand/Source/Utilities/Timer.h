// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// Timer.h
//
// Wrapper for QueryPerformanceCounter().
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

#pragma once;

#include <math.h>

class Timer
{
public:
	static LARGE_INTEGER sTicksPerSecond;

	Timer();
	virtual ~Timer() {};

	
	static double_t 	microseconds(); // public call returns microseconds since startup
	static double_t 	seconds(); // handy public call returns seconds since startup
	
	virtual void		reset(void);
	virtual double_t	elapsedSeconds(void);
	virtual double_t	elapsedTicks(void);
	virtual double_t	elapsedMilliseconds(void);		// thousandths
	virtual double_t	elapsedMicroseconds(void);		// millionths

private:
	double_t	mStartMicroseconds;
};
