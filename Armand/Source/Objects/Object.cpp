// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// Object.cpp
//
// Base class for all objects in Armand that have some position and orientation
// in Universal coordinates.
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

#include "Object.h"
#include "Math/constants.h"

Object::Object()
{
	mBoundingRadiusAU = (float_t)kAuPerMetre;
}

Object::~Object()
{
}

void Object::setUniveralPositionAU(const Vec3Big& inPositionAU)
{
	mUniversalPositionAU = inPositionAU;
}

void Object::setUniveralPositionLY(const Vec3d& inPositionLY)
{
	mUniversalPositionAU = Vec3Big(inPositionLY * kAuPerLightYear);
}

void Object::setUniveralPositionAU(const Vec3d& inPositionAU)
{
	mUniversalPositionAU = Vec3Big(inPositionAU);
}

void Object::setUniveralPositionKm(const Vec3d& inPositionKm)
{
	mUniversalPositionAU = Vec3Big(inPositionKm * kAuPerKilometer);
}

void Object::setUniveralPositionMetres(const Vec3d& inPositionMetres)
{
	mUniversalPositionAU = Vec3Big(inPositionMetres * kAuPerMetre);
}