// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// 3DStar.h
//
// Responsible for rendering 3D stars.
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

#include "Objects/RenderObject.h"

class T3DStar : public RenderObject
{
public:
	T3DStar();
	virtual ~T3DStar();

	virtual bool renderFull(Camera& inCamera, float inAlpha);
	virtual bool isSelfIlluminating() const { return true; };
	virtual bool canRenderFull() { return true; };

	void setAbsoluteMagnitude(float_t inAbsoluteMagnitude) { mAbsoluteMagnitude = inAbsoluteMagnitude; };

protected:
	float_t	mAbsoluteMagnitude;
};