// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// VertexBufferStructs.h
//
// Structure definitions for vertex buffer arrangements.
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

#define BUFFER_OFFSET(offset) ((void*)(offset))

typedef struct
{
	float x, y;			// position
} v2f;

typedef struct
{
	float x, y;			// position
	float r, g, b, a;	// color
} v2f_c4f;

typedef struct
{
	float x, y, z;
	float n1, n2, n3;
} v3f_n3f;

typedef struct
{
	float x, y, z;
	float n1, n2, n3;
	float s, t;
} v3f_n3f_t2f;

typedef struct
{
	float x, y, z;		// position
	float s, t;			// texture
	float r, g, b, a;	// color
} v3f_t2f_c4f;
