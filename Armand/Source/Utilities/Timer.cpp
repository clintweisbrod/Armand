// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// Timer.cpp
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

#include "stdafx.h"

#include "Timer.h"

LARGE_INTEGER Timer::sTicksPerSecond;

Timer::Timer(void)
{
	reset();
}

void Timer::reset()
{
	mStartMicroseconds = Timer::microseconds();
}

double_t Timer::microseconds()
{
	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);
	double_t rawTime = (double_t)tick.QuadPart;
	double_t seconds = rawTime / sTicksPerSecond.QuadPart;

	return seconds * 1e6;
}

double_t Timer::seconds()
{
	return Timer::microseconds() * 1e-6;
}

double_t Timer::elapsedMicroseconds()
{
	double_t currentMicroseconds = Timer::microseconds();
	return (currentMicroseconds - mStartMicroseconds);
}

double_t Timer::elapsedMilliseconds()
{
	return elapsedMicroseconds() * 1e-3;
}

double_t Timer::elapsedSeconds()
{
	return elapsedMicroseconds() * 1e-6;
}

double_t Timer::elapsedTicks()
{
	return  elapsedSeconds() * 60.0;
}