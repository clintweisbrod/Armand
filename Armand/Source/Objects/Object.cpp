#include "stdafx.h"

#include "Object.h"

Object::Object()
{
}

Object::~Object()
{
}

void Object::setUniveralPosition(const Vec3Big& inPosition)
{
	mUniversalPosition = inPosition;
}

void Object::setUniveralPositionLY(const Vec3d& inPosition)
{
	mUniversalPosition = Vec3Big(inPosition * kAuPerLightYear);
}

void Object::setUniveralPositionAU(const Vec3d& inPosition)
{
	mUniversalPosition = Vec3Big(inPosition);
}

void Object::setUniveralPositionKm(const Vec3d& inPosition)
{
	mUniversalPosition = Vec3Big(inPosition * kAuPerKilometer);
}

void Object::setUniveralPositionMetres(const Vec3d& inPosition)
{
	mUniversalPosition = Vec3Big(inPosition * kAuPerMetre);
}