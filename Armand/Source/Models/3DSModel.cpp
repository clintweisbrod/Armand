// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// 3ds.cpp
//
// Handles loading and rendering 3DS models.
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

#include <assert.h>
#include <math.h>

#include "3DSModel.h"
#include "Math/constants.h"
#include "Utilities/ConfigReader.h"
#include "OpenGL/GLUtils.h"
#include "OpenGL/ShaderFactory.h"
#include "OpenGL/OpenGLWindow.h"
#include "OpenGL/VBOData.h"
#include "Math/mathlib.h"
#include "Math/quaternion.h"

#define INITIAL_BUFFER_SIZE	2000000

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// 3DS object definitions
//----------------------------------------------------------------------
//----------------------------------------------------------------------

T3DSNormalInfo::T3DSNormalInfo() :	mNormal(0.0f, 0.0f, 0.0f),
									mSharedCount(0)
{
}

T3DSVertex::T3DSVertex() :	mVertex(0.0f, 0.0f, 0.0f),
							mSmoothingGroups(0)
{
}

T3DSMaterialInfo::T3DSMaterialInfo() :	mTexture(NULL),
										mShininess(0.0f),
										mUScale(1.0f),
										mVScale(1.0f),
										mUOffset(0.0f),
										mVOffset(0.0f)
{
	mAmbientColor[0] = mAmbientColor[1] = mAmbientColor[2] = 0.0f; mAmbientColor[3] = 1.0f;
	mDiffuseColor[0] = mDiffuseColor[1] = mDiffuseColor[2] = 0.0f; mDiffuseColor[3] = 1.0f;
	mSpecularColor[0] = mSpecularColor[1] = mSpecularColor[2] = 0.0f; mSpecularColor[3] = 1.0f;
}

T3DSFace::T3DSFace()
{
	mVertIndex[0] = mVertIndex[1] = mVertIndex[2] = 0;
	mCoordIndex[0] = mCoordIndex[1] = mCoordIndex[2] = 0;
	mSmoothingGroup = 0;
	mMaterialID = -1;
}

//----------------------------------------------------------------------
//	T3DSFaceComparator::operator()
//
//	This is a function object we need to properly sort the vector of
//	T3DSFace instances based on opacity. In order to properly render
//	translucent objects, we must render the objects that are opaque first,
//	and then render objects by decreasing opacity. Sorting the vector of
//	T3DSFace instances requires access to private data in the T3DSModel
//	instance that owns the vector of T3DSObjects, therefore this function
//	object is absolutely necessary.
//
//	return true if item1 is more opaque than item2.
//----------------------------------------------------------------------
bool T3DSFaceComparator::operator()(const T3DSFace& inItem1, const T3DSFace& inItem2) const
{
	// We're basically comparing transparencies of the materials used in either item.
	// If item has no material, we will draw opaque and assign a value of 1 for the item
	// If item has material, we assign the alpha value of the diffuse material property
	float item1Value, item2Value;
	if (inItem1.mMaterialID == -1)
		item1Value = 1.0f;
	else
		item1Value = m3DSModel->mMaterials[inItem1.mMaterialID].mDiffuseColor[3];
	if (inItem2.mMaterialID == -1)
		item2Value = 1.0f;
	else
		item2Value = m3DSModel->mMaterials[inItem2.mMaterialID].mDiffuseColor[3];

	return item1Value > item2Value;
}

T3DSObject::T3DSObject() :	mNumVertices(0),
							mNumFaces(0),
							mNumTexCoords(0),
							mVertices(NULL),
							mTexCoords(NULL),
							mHasSmoothingInfo(false),
							mPerVertexNormals(false)
{
}

T3DSModel::T3DSModel()
{
	mModelDataLoaded = false;
	mMetaDataLoaded = false;

    try {
        mBuffer = new uint32_t[INITIAL_BUFFER_SIZE];
        mBufferSize = INITIAL_BUFFER_SIZE;
    } catch (bad_alloc) {
        mBuffer = NULL;
        mBufferSize = 0;
    }

	mShaderHandle = 0;
	mRenderAtmosphere = false;
	mRenderCoordinateAxes = false;
	mConvolvedTextureID = 0;
	mExpansionIterations = 5;
	mConvolutionIterations = 10;
	mVAOs[eUntexturedVAO] = mVAOs[eTexturedVAO] = 0;
	mVBOs[eUntexturedVBO] = mVBOs[eTexturedVBO] = 0;

    // 1 meter by default instead of 0 so that's it's a bit more reasonable.
	mPhysicalRadiusInAU = 1 * (float_t)kAuPerMetre;	// Is only possibly set by associated .meta file.
	mModelUpVector = Vec3f(0.0f, 1.0f, 0.0f);	// The default OpenGL up vector
	mRotationRateInRadiansPerCentury = 0.0f;
	mInclinationAngleInDegrees = 0.0f;
}

T3DSModel::~T3DSModel()
{
	// Go through all the objects in the model. At this point, cleanup() should've done this already.
	T3DSObjectVec_t::iterator object;
	for (object = mObjects.begin(); object != mObjects.end(); object++)
	{
		// Free the faces, normals, vertices, and texture coordinates.
		object->mFaces.clear();
		if (object->mVertices)
			delete [] object->mVertices;
		if (object->mTexCoords)
			delete [] object->mTexCoords;
	}
	
	// Free any textures
	TModelTextureMap_t::iterator texture;
	for (texture = mTextureMap.begin(); texture != mTextureMap.end(); texture++)
	{
		if (texture->second != NULL)
		{
			delete texture->second;
			texture->second = NULL;
		}
	}
	
	if (mBuffer)
		delete [] mBuffer;
	
	glDeleteVertexArrays(eNumVAOs, mVAOs);
	glDeleteBuffers(eNumVBOs, mVBOs);
}

//----------------------------------------------------------------------
//	T3DSModel::cleanup
//
//	Deallocates all data needed to load the model.
//----------------------------------------------------------------------
void T3DSModel::cleanUp()
{
	if (mBuffer)
	{
		delete[] mBuffer;
		mBuffer = NULL;
		mBufferSize = 0;
	}

	// This causes a memory leak, but in debug mode, this code is incredibly slow.
#if NDEBUG
	// Go through all the objects in the model.
	T3DSObjectVec_t::iterator object;
	for (object = mObjects.begin(); object != mObjects.end(); object++)
	{
		// Free the faces, normals, vertices, and texture coordinates.
		object->mFaces.clear();
		if (object->mVertices)
			delete[] object->mVertices;
		if (object->mTexCoords)
			delete[] object->mTexCoords;
	}
	mObjects.clear();
#endif
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// 3DS model loading
//----------------------------------------------------------------------
//----------------------------------------------------------------------

bool T3DSModel::load(File& inModelFile, bool inLoadMetaOnly)
{
	bool bResult = true;

	// Store a copy of the supplied file for possible logging events
	mFile = inModelFile;
	
	// Store the file name without extension. This is the texture sub-folder name
	mTextureFolderName = mFile.getFileNameWithoutExtension();
	
	// Don't load the model data if we only want the meta data
	if (!inLoadMetaOnly && !mModelDataLoaded)
	{
		// Now attempt to load the model
		try
		{
			T3DSChunk currentChunk = {0};
			
			// Open the 3DS file
			FILE* theFileHandle = inModelFile.getCRTFileHandle("rb");
			if (!theFileHandle) 
			{
				LOG(ERROR) << "Unable to find the file: " << inModelFile.getFullPath() << ".";
				return false;
			}

			// Once we have the file open, we need to read the very first data chunk
			// to see if it's a 3DS file.  That way we don't read an invalid file.
			// If it is a 3DS file, then the first chunk ID will be equal to PRIMARY (some hex num)

			// Read the first chuck of the file to see if it's a 3DS file
			readChunk(theFileHandle, &currentChunk);

			// Make sure this is a 3DS file
			if (currentChunk.mID != M3D_PRIMARY)
			{
				LOG(ERROR) << "Unable to recognize file as 3DS format: " << inModelFile.getFullPath() << ".";
				return false;
			}

			// Now we actually start reading in the data.  ProcessNextChunk() is recursive

			// Begin loading objects, by calling this recursive function
			processNextChunk(theFileHandle, &currentChunk);
			
			// Close model file
			fclose(theFileHandle);

			// Remove any objects that have no faces defined.
			removeFacelessObjects();
		
			// For each T3DSObject in mObjects, sort the T3DSFace instances in mFaces by descending opacity so that
			// at render time, we minimize state changes and transparent materials are rendered after opaque materials.

			// CLW: Nov 13, 2014: For some reason I haven't been able to track down, this call is fugging up
			// the vertex indices of a face and causing really poor render results. Clearly, this has something
			// to do with T3DSFaceComparator used by the std::sort() call made in SortFaces(), but I'm unable
			// to determine any violations of the strict weak ordering in the comparator.
			// Oddly, this is only an issue on Mac.
			sortFaces();

			// We need to know the bounding radius of the model so we can render it at the correct physical size
			computeBoundingRadius();
			
			// After we have read the whole 3DS file, we want to calculate our own vertex normals.
			computeNormals();
			
			// Try to load any textures that this model uses
			loadTextures();

			// Adjust any texture coordinates by their material scaling and offset params
			adjustTextureCoordinates();

			// Construct arrays that will be used with glDrawArrays(), glMultiDrawArrays() and VBOs
			buildArrays();

			// Release any memory we no longer need for rendering
			cleanUp();
			
			mModelDataLoaded = true;
		}
		catch (...)
		{
			bResult = false;
		}
	}
	
	// Load model meta data, which will be found in same folder as the model.
	if (!mMetaDataLoaded)
		loadMetaData(inModelFile);

	return bResult;
}

void T3DSModel::removeFacelessObjects()
{
	T3DSObjectVec_t::iterator object = mObjects.begin();
	while (object != mObjects.end())
	{
		// Erase any objects that have no faces defined
		if (object->mNumFaces == 0)
			mObjects.erase(object);
		else
		{
			// Also, erase any faces that have material specifications that make them invisible.
			T3DSFaceVec_t::iterator face = object->mFaces.begin();
			while (face != object->mFaces.end())
			{
				if (face->mMaterialID >= 0)
				{
					// If the alpha values for diffuse and specular color are zero, we don't render it.
					// Why? Because the triangle will just appear black and will not respond to any lighting.
					// Our model of Cassini happens to use this material definition and it looks nutty.
					T3DSMaterialInfo* theMaterial = &mMaterials[face->mMaterialID];
					if ((theMaterial->mDiffuseColor[3] == 0.0f) && (theMaterial->mSpecularColor[3] == 0.0f))
						object->mFaces.erase(face);
					else
						face++;
				}
				else
					face++;
			}
			object++;
		}
	}
}

void T3DSModel::buildArrays()
{
	// Locate all the objects that require and don't require textures
	GLuint numUnTexturedVertices = 0, numTexturedVertices = 0;
	vector<T3DSObject> texturedObjects, unTexturedObjects;
	T3DSObjectVec_t::iterator object;
	for (object = mObjects.begin(); object != mObjects.end(); object++)
	{
		// Go through all of the faces (triangles)
		bool hasTexture = false;
		for (int face = 0; face < object->mNumFaces; face++)
		{
			T3DSFace* theFace = &(object->mFaces[face]);
			if (theFace->mMaterialID >= 0)
			{
				T3DSMaterialInfo* theMaterial = &mMaterials[theFace->mMaterialID];
				if (theMaterial->mTexture != NULL)
				{
					hasTexture = (object->mTexCoords != NULL);
					break;
				}
			}
		}
		if (hasTexture)
		{
			texturedObjects.push_back(*object);
			numTexturedVertices += (object->mNumFaces * 3);
		}
		else
		{
			unTexturedObjects.push_back(*object);
			numUnTexturedVertices += (object->mNumFaces * 3);
		}
	}

	// Allocate VAOs
	glGenVertexArrays(eNumVAOs, mVAOs);

	// Allocate VBOs
	glGenBuffers(eNumVBOs, mVBOs);

	GLint firstValue = 0;
	GLsizei countValue = 0;
	for (object = unTexturedObjects.begin(); object != unTexturedObjects.end(); object++)
	{
		mArrayFirstUntextured.push_back(firstValue);
		countValue = 0;

		// Go through all of the faces (triangles) of the object
		T3DSVBOInfo vertex;
		for (int face = 0; face < object->mNumFaces; face++)
		{
			T3DSFace* theFace = &(object->mFaces[face]);

			setVertexMaterial(vertex, theFace->mMaterialID);

			// Visit each corner of the triangle
			for (int whichVertex = 0; whichVertex < 3; whichVertex++)
			{
				// Position and normal
				int index = theFace->mVertIndex[whichVertex];
				setVertexData(vertex, *object, theFace, index);

				// Add the vertex
				mArrayDataUntextured.push_back(vertex);

				countValue++;
				firstValue++;
			}
		}

		mArrayCountUntextured.push_back(countValue);
	}

	// Bind the untextured VAO as the current used object
	glBindVertexArray(mVAOs[eUntexturedVAO]);

	// Bind the untextured VBO as being the active buffer and storing vertex attributes (coordinates)
	glBindBuffer(GL_ARRAY_BUFFER, mVBOs[eUntexturedVBO]);

	// Copy the vertex data
	glBufferData(GL_ARRAY_BUFFER, mArrayDataUntextured.size() * sizeof(T3DSVBOInfo), mArrayDataUntextured.data(), GL_STATIC_DRAW);
	glCheckForError();

	GLuint offset = 0;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(T3DSVBOInfo), BUFFER_OFFSET(offset));	// vPosition
	offset += (3 * sizeof(GLfloat));
	glVertexAttribPointer(1, GL_BGRA, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(T3DSVBOInfo), BUFFER_OFFSET(offset));	// vNormal
	offset += sizeof(GLuint);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(T3DSVBOInfo), BUFFER_OFFSET(offset));	// vMaterialAmbient
	offset += (4 * sizeof(GLubyte));
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(T3DSVBOInfo), BUFFER_OFFSET(offset));	// vMaterialDiffuse
	offset += (4 * sizeof(GLubyte));
	glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(T3DSVBOInfo), BUFFER_OFFSET(offset));	// vMaterialSpecular
	offset += (4 * sizeof(GLubyte));
	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(T3DSVBOInfo), BUFFER_OFFSET(offset));	// vMaterialShininess

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Add the textured objects
	firstValue = 0;
	countValue = 0;
	for (object = texturedObjects.begin(); object != texturedObjects.end(); object++)
	{
		mArrayFirstTextured.push_back(firstValue);
		countValue = 0;

		// Go through all of the faces (triangle) of the object
		GLuint texID = 0;
		T3DSVBOInfoTextured vertex;
		for (int face = 0; face < object->mNumFaces; face++)
		{
			T3DSFace* theFace = &(object->mFaces[face]);

			setVertexMaterial(vertex, theFace->mMaterialID);

			T3DSMaterialInfo* theMaterial = NULL;
			if ((theFace->mMaterialID >= 0) && (texID == 0))
			{
				theMaterial = &mMaterials[theFace->mMaterialID];
				if (theMaterial->mTexture)
					texID = theMaterial->mTexture->getTextureID();
			}

			// Visit each corner of the triangle and draw it.
			for (int whichVertex = 0; whichVertex < 3; whichVertex++)
			{
				// Position and normal
				int index = theFace->mVertIndex[whichVertex];
				setVertexData(vertex, *object, theFace, index);

				// Texcoords
				glTexCoord2fToTexCoord2us(object->mTexCoords[index], vertex.mTexCoords);

				// Add the vertex
				mArrayDataTextured.push_back(vertex);

				countValue++;
				firstValue++;
			}
		}

		mTextureIDs.push_back(texID);
		mArrayCountTextured.push_back(countValue);
	}

	// Bind the untextured VAO as the current used object
	glBindVertexArray(mVAOs[eTexturedVAO]);

	// Bind the untextured VBO as being the active buffer and storing vertex attributes (coordinates)
	glBindBuffer(GL_ARRAY_BUFFER, mVBOs[eTexturedVBO]);

	// Copy the vertex data
	glBufferData(GL_ARRAY_BUFFER, mArrayDataTextured.size() * sizeof(T3DSVBOInfoTextured), mArrayDataTextured.data(), GL_STATIC_DRAW);
	glCheckForError();

	offset = 0;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(T3DSVBOInfoTextured), BUFFER_OFFSET(offset));	// vPosition
	offset += (3 * sizeof(GLfloat));
	glVertexAttribPointer(1, GL_BGRA, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(T3DSVBOInfoTextured), BUFFER_OFFSET(offset));	// vNormal
	offset += sizeof(GLuint);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(T3DSVBOInfoTextured), BUFFER_OFFSET(offset));	// vMaterialAmbient
	offset += (4 * sizeof(GLubyte));
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(T3DSVBOInfoTextured), BUFFER_OFFSET(offset));	// vMaterialDiffuse
	offset += (4 * sizeof(GLubyte));
	glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(T3DSVBOInfoTextured), BUFFER_OFFSET(offset));	// vMaterialSpecular
	offset += (4 * sizeof(GLubyte));
	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(T3DSVBOInfoTextured), BUFFER_OFFSET(offset));	// vMaterialShininess
	offset += (1 * sizeof(GLfloat));
	glVertexAttribPointer(6, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(T3DSVBOInfoTextured), BUFFER_OFFSET(offset));	// vTexture coordinates

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void T3DSModel::setVertexData(T3DSVBOInfo& ioVertex, T3DSObject& inObject, T3DSFace* inFace, int inIndex)
{
	// Normal
	Vec3f* theNormal;
	uint32_t smoothingGroup = inFace->mSmoothingGroup;
	if (inObject.mPerVertexNormals && smoothingGroup)
		theNormal = &inObject.mVertices[inIndex].mNormalMap[smoothingGroup].mNormal;
	else
		theNormal = &inFace->mNormal;
	ioVertex.mNormal = glNorma3fToGL_INT_2_10_10_10_REV(*theNormal);

	// Vertex
	ioVertex.mPosition[0] = inObject.mVertices[inIndex].mVertex.x;
	ioVertex.mPosition[1] = inObject.mVertices[inIndex].mVertex.y;
	ioVertex.mPosition[2] = inObject.mVertices[inIndex].mVertex.z;
}

void T3DSModel::setVertexMaterial(T3DSVBOInfo& ioVertex, int inMaterialID)
{
	const GLfloat kDefaultMaterialShininess = 0.1f * 128.0f;
	const GLfloat kZeroLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	const GLfloat kWhiteLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	// Does this object have a material?
	T3DSMaterialInfo* theMaterial = NULL;
	if (inMaterialID >= 0)
	{
		theMaterial = &mMaterials[inMaterialID];

		// Set ambient material color
		// We're overriding ambient material color so that we have a consistent ambient lighing effect for all models.
		glColor4fToColor4ub(kZeroLight, ioVertex.mMaterial.mAmbient);

		// Set diffuse material color
		glColor4fToColor4ub(theMaterial->mDiffuseColor, ioVertex.mMaterial.mDiffuse);

		// Set specular material color
		if ((theMaterial->mSpecularColor[0] == 0.0f) && (theMaterial->mSpecularColor[1] == 0.0f) && (theMaterial->mSpecularColor[2] == 0.0f))
		{
			glColor4fToColor4ub(kZeroLight, ioVertex.mMaterial.mSpecular);
			ioVertex.mMaterial.mShininess = 0;
		}
		else
		{
			// Make sure the specular color has full alpha
			float matSpecular[4] = { theMaterial->mSpecularColor[0], theMaterial->mSpecularColor[1], theMaterial->mSpecularColor[2], 1.0f };
			glColor4fToColor4ub(matSpecular, ioVertex.mMaterial.mSpecular);
			ioVertex.mMaterial.mShininess = theMaterial->mShininess;
		}
	}
	else
	{
		// YIKES!! No material definition. Bummer. Default to white material with default shininess.

		// Set default material properties
		glColor4fToColor4ub(kZeroLight, ioVertex.mMaterial.mAmbient);
		glColor4fToColor4ub(kZeroLight, ioVertex.mMaterial.mDiffuse);
		glColor4fToColor4ub(kZeroLight, ioVertex.mMaterial.mSpecular);
		ioVertex.mMaterial.mShininess = kDefaultMaterialShininess;
	}
}

//----------------------------------------------------------------------
//	T3DSModel::processNextChunk
//
//	This method reads the main sections of the 3DS file, then dives
//	deeper with recursion.
//----------------------------------------------------------------------
void T3DSModel::processNextChunk(FILE* inFileHandle, T3DSChunk* ioPreviousChunk)
{
	T3DSObject newObject;			// This is used to add to our object list
	T3DSMaterialInfo newTexture;	// This is used to add to our material list

	T3DSChunk currentChunk = { 0 };	// The current chunk to load
	//T3DSChunk tempChunk = {0};		// A temp chunk for holding data

	// Below we check our chunk ID each time we read a new chunk.  Then, if
	// we want to extract the information from that chunk, we do so.
	// If we don't want a chunk, we just read past it.  

	// Continue to read the sub chunks until we have reached the length.
	// After we read ANYTHING we add the bytes read to the chunk and then check
	// check against the length.
	while (ioPreviousChunk->mBytesRead < ioPreviousChunk->mLength)
	{
		// Read next Chunk
		readChunk(inFileHandle, &currentChunk);

		// Check the chunk ID
		switch (currentChunk.mID)
		{
		case M3D_CHUNKTYPE_VERSION: // This holds the version of the file
			// If the file was made in 3D Studio Max, this chunk has an int that 
			// holds the file version. Since there might be new additions to the 3DS file
			// format in 4.0, we give a warning to that problem.
			// However, if the file wasn't made by 3D Studio Max, we don't 100% what the
			// version length will be so we'll simply ignore the value

			// Read the file version and add the bytes read to our mBytesRead variable
			currentChunk.mBytesRead += read(inFileHandle, 1, currentChunk.mLength - currentChunk.mBytesRead);

			// If the file version is over 3, give a warning that there could be a problem
			if ((currentChunk.mLength - currentChunk.mBytesRead == 4) && (mBuffer[0] > 0x03))
				LOG(WARNING) << "3DS file version greater than 3. Not expected.";
			break;

		case M3D_CHUNKTYPE_OBJECTINFO: // This holds the version of the mesh
			// This chunk holds the version of the mesh.  It is also the head of the MATERIAL
			// and OBJECT chunks.  From here on we start reading in the material and object info.
			processNextChunk(inFileHandle, &currentChunk);
			break;

		case M3D_CHUNKTYPE_MATERIAL: // This holds the material information
			// This chunk is the header for the material info chunks

			// Add a empty texture structure to our texture list.
			// If you are unfamiliar with STL's "vector" class, all push_back()
			// does is add a new node onto the list.  I used the vector class
			// so I didn't need to write my own link list functions.  
			mMaterials.push_back(newTexture);

			// Proceed to the material loading function
			processNextMaterialChunk(inFileHandle, &currentChunk);
			break;

		case M3D_CHUNKTYPE_OBJECT:	// This holds the name of the object being read
		{
			// This chunk is the header for the object info chunks.  It also
			// holds the name of the object.

			mObjects.push_back(newObject);

			// Get the name of the object and store it, then add the read bytes to our byte counter.
			currentChunk.mBytesRead += readString(inFileHandle, mObjects[mObjects.size() - 1].mName);

			// Now proceed to read in the rest of the object information
			processNextObjectChunk(inFileHandle, &(mObjects[mObjects.size() - 1]), &currentChunk);

			// Testing smoothing without smoothing groups
			// (CLW) - Oct 5, 2006
			// It appears reasonable and acceptable to assume that all faces composing an object can
			// belong to the same smoothing group. This will obviously introduce some unwanted lighting
			// artifacts, but the visual results are usually better than not forcing per-vertex
			// lighting normal calculations. 
			T3DSObject* theObject = &(mObjects[mObjects.size() - 1]);
			if ((theObject->mNumFaces > 0) && (theObject->mHasSmoothingInfo == false))
			{
				for (int i = 0; i < theObject->mNumFaces; i++)
					theObject->mFaces[i].mSmoothingGroup = 1;

				theObject->mHasSmoothingInfo = true;
			}

			break;
		}

		case M3D_EDITKEYFRAME:
			//ProcessNextKeyFrameChunk(currentChunk);

			// Read past this chunk and add the bytes read to the byte counter
			currentChunk.mBytesRead += read(inFileHandle, 1, currentChunk.mLength - currentChunk.mBytesRead);
			break;

		default:
			// If we didn't care about a chunk, then we get here.  We still need
			// to read past the unknown or ignored chunk and add the bytes read to the byte counter.
			currentChunk.mBytesRead += read(inFileHandle, 1, currentChunk.mLength - currentChunk.mBytesRead);
			if (currentChunk.mBytesRead == 0) {
				// Need to bail out here or else we get caught in a tight loop
				ioPreviousChunk->mBytesRead = ioPreviousChunk->mLength;
			}
			break;
		}

		// Add the bytes read from the last chunk to the previous chunk passed in.
		ioPreviousChunk->mBytesRead += currentChunk.mBytesRead;
	}
}

//----------------------------------------------------------------------
//	T3DSModel::processNextObjectChunk
//
//	This method handles all the information about the objects in the file.
//	It is also recursive.
//----------------------------------------------------------------------
void T3DSModel::processNextObjectChunk(FILE* inFileHandle, T3DSObject* ioObject, T3DSChunk* ioPreviousChunk)
{
	// The current chunk to work with
	T3DSChunk currentChunk = { 0 };

	// Continue to read these chunks until we read the end of this sub chunk
	while (ioPreviousChunk->mBytesRead < ioPreviousChunk->mLength)
	{
		// Read the next chunk
		readChunk(inFileHandle, &currentChunk);

		// Check which chunk we just read
		switch (currentChunk.mID)
		{
		case M3D_OBJECT_MESH:					// This lets us know that we are reading a new object
			// We found a new object, so let's read in it's info using recursion
			processNextObjectChunk(inFileHandle, ioObject, &currentChunk);
			break;

		case M3D_OBJECT_VERTICES:				// This is the objects vertices
			readVertices(inFileHandle, ioObject, &currentChunk);
			break;

		case M3D_OBJECT_FACES:					// This is the objects face information
			readVertexIndices(inFileHandle, ioObject, &currentChunk);
			break;

		case M3D_OBJECT_MATERIAL:				// This holds the material name that the object has
			// This chunk holds the name of the material that the object has assigned to it.
			// This could either be just a color or a texture map.  This chunk also holds
			// the faces that the texture is assigned to (In the case that there is multiple
			// textures assigned to one object, or it just has a texture on a part of the object.
			// Since most of my game objects just have the texture around the whole object, and 
			// they aren't multitextured, I just want the material name.

			// We now will read the name of the material assigned to this object
			readObjectMaterial(inFileHandle, ioObject, &currentChunk);
			break;

		case M3D_OBJECT_UV:						// This holds the UV texture coordinates for the object
			// This chunk holds all of the UV coordinates for our object.  Let's read them in.
			readUVCoordinates(inFileHandle, ioObject, &currentChunk);
			break;

		case M3D_OBJECT_SMOOTHING_GROUP:
			// This chunk contains smoothing group information.
			readSmoothingGroups(inFileHandle, ioObject, &currentChunk);
			break;

		case M3D_OBJECT_TRANSLATION_MATRIX:
			readTranslationMatrix(inFileHandle, ioObject, &currentChunk);
			break;

		default:
			// Read past the ignored or unknown chunks
			currentChunk.mBytesRead += read(inFileHandle, 1, currentChunk.mLength - currentChunk.mBytesRead);
			break;
		}

		// Add the bytes read from the last chunk to the previous chunk passed in.
		ioPreviousChunk->mBytesRead += currentChunk.mBytesRead;

		if (ioPreviousChunk->mBytesRead > ioPreviousChunk->mLength)
			LOG(ERROR) << "3DS Model load: Read PAST the end of a chunk! " << mFile.getFullPath();
	}
}

//----------------------------------------------------------------------
//	T3DSModel::processNextMaterialChunk
//
//	This method handles all the information about the material.
//----------------------------------------------------------------------
void T3DSModel::processNextMaterialChunk(FILE* inFileHandle, T3DSChunk* ioPreviousChunk)
{
	bool specularDefined = false;

	// The current chunk to work with
	T3DSChunk currentChunk = { 0 };

	// Continue to read these chunks until we read the end of this sub chunk
	while (ioPreviousChunk->mBytesRead < ioPreviousChunk->mLength)
	{
		// Read the next chunk
		readChunk(inFileHandle, &currentChunk);

		// Check which chunk we just read in
		T3DSMaterialInfo* currentMaterial = &mMaterials[mMaterials.size() - 1];
		switch (currentChunk.mID)
		{
		case M3D_MATERIAL_NAME:							// This chunk holds the name of the material
			currentChunk.mBytesRead += readString(inFileHandle, currentMaterial->mName);
			break;

		case M3D_MATERIAL_AMBIENT:						// This holds the R G B color of our object
			readColorChunk(inFileHandle, currentMaterial->mAmbientColor, &currentChunk);
			break;

		case M3D_MATERIAL_DIFFUSE:						// This holds the R G B color of our object
			readColorChunk(inFileHandle, currentMaterial->mDiffuseColor, &currentChunk);
			break;

		case M3D_MATERIAL_SPECULAR:
			readColorChunk(inFileHandle, currentMaterial->mSpecularColor, &currentChunk);
			specularDefined = true;
			break;

		case M3D_MATERIAL_SHININESS:
		{
									   float shininessPercent = 0.0;
									   readPercentageChunk(inFileHandle, &shininessPercent, &currentChunk);
									   if (specularDefined && (shininessPercent == 0.0f))
										   shininessPercent = 0.75f;
									   currentMaterial->mShininess = shininessPercent * 128.0f;
									   break;
		}

		case M3D_MATERIAL_TRANSPARENCY:
		{
										  float transparencyPercent = 0.0;
										  readPercentageChunk(inFileHandle, &transparencyPercent, &currentChunk);
										  currentMaterial->mDiffuseColor[3] = 1.0f - transparencyPercent;
										  currentMaterial->mSpecularColor[3] = 1.0f - transparencyPercent;
										  break;
		}

		case M3D_MATERIAL_MAP:							// This is the header for the texture info
			// Proceed to read in the material information
			processNextMaterialChunk(inFileHandle, &currentChunk);
			break;

		case M3D_MATERIAL_FILE:						// This stores the file name of the material
			// Here we read in the material's file name
			currentChunk.mBytesRead += readString(inFileHandle, currentMaterial->mFilename);
			break;

		case M3D_MATERIAL_MAP_USCALE:
			currentChunk.mBytesRead += readFloat(inFileHandle, &currentMaterial->mUScale);
			break;

		case M3D_MATERIAL_MAP_VSCALE:
			currentChunk.mBytesRead += readFloat(inFileHandle, &currentMaterial->mVScale);
			break;

		case M3D_MATERIAL_MAP_UOFFSET:
			currentChunk.mBytesRead += readFloat(inFileHandle, &currentMaterial->mUOffset);
			break;

		case M3D_MATERIAL_MAP_VOFFSET:
			currentChunk.mBytesRead += readFloat(inFileHandle, &currentMaterial->mVOffset);
			break;

		default:
			// Read past the ignored or unknown chunks
			currentChunk.mBytesRead += read(inFileHandle, 1, currentChunk.mLength - currentChunk.mBytesRead);
			if (currentChunk.mBytesRead == 0) {
				// Need to bail out here or else we get caught in a tight loop
				ioPreviousChunk->mBytesRead = ioPreviousChunk->mLength;
			}
			break;
		}

		// Add the bytes read from the last chunk to the previous chunk passed in.
		ioPreviousChunk->mBytesRead += currentChunk.mBytesRead;
	}
}

//----------------------------------------------------------------------
//	T3DSModel::readChunk
//
//	This method reads in a chunk ID and it's length in bytes.
//----------------------------------------------------------------------
void T3DSModel::readChunk(FILE* inFileHandle, T3DSChunk* outChunk)
{
	// This reads the chunk ID which is 2 bytes.
	// The chunk ID is like OBJECT or MATERIAL.  It tells what data is
	// able to be read in within the chunks section.
	outChunk->mBytesRead = read(inFileHandle, sizeof(outChunk->mID), 1, &outChunk->mID);

	if (outChunk->mID == 0)
		LOG(ERROR) << "This is a problem: outChunk->mID = " << outChunk->mID;

	// Then, we read the length of the chunk which is 4 bytes.
	// This is how we know how much to read in, or read past.
	outChunk->mBytesRead += read(inFileHandle, sizeof(outChunk->mLength), 1, &outChunk->mLength);
	if (outChunk->mLength <= 0)
		outChunk->mLength = 0;

	if ((outChunk->mLength <= 0) || (outChunk->mLength > 5000000))
		LOG(ERROR) << "This is a problem: outChunk->mLength = " << outChunk->mLength;

	if (outChunk->mLength == 65536)
		LOG(ERROR) << "VERY Strange chunk that is exactly USHRT_MAX 65536 long outChunk->mLength" << outChunk->mLength;
}

//----------------------------------------------------------------------
//	T3DSModel::read
//
//	Calls fread() but checks if the destination buffer is big enough or not
//	Only for the fread() calls that use the global buffer.
//----------------------------------------------------------------------
size_t T3DSModel::read(FILE* inFileHandle, size_t inSize, size_t inCount)
{
	size_t bytesRead = 0;

	// check if buffer is big enough
	if (inCount > mBufferSize)
	{
		if (mBuffer)
		{
			delete[] mBuffer;
			mBuffer = NULL;
			mBufferSize = 0;
		}

		try
		{
			mBuffer = new uint32_t[inCount];
			mBufferSize = inCount;
		}
		catch (bad_alloc)
		{
			mBuffer = NULL;
			mBufferSize = 0;
		}
	}

	// read the data
	if (mBuffer && inFileHandle)
		bytesRead = fread(mBuffer, inSize, inCount, inFileHandle) * inSize;

	return bytesRead;
}

//----------------------------------------------------------------------
//	T3DSModel::read
//
//	Wraps fread(), returning bytes read.
//----------------------------------------------------------------------
size_t T3DSModel::read(FILE* inFileHandle, size_t inSize, size_t inCount, void* ioBuf)
{
	size_t bytesRead = 0;

	// read the data
	if (inFileHandle)
		bytesRead = fread(ioBuf, inSize, inCount, inFileHandle) * inSize;

	return bytesRead;
}

//----------------------------------------------------------------------
//	T3DSModel::readFloat
//
//	Calls fread() to read a 32-bit floating point value.
//	All 3DS files are written using little endian format.
//----------------------------------------------------------------------
size_t T3DSModel::readFloat(FILE* inFileHandle, GLfloat* ioFloatValue)
{
	size_t bytesRead = 0;
	uint32_t value;

	if (inFileHandle)
	{
		bytesRead = read(inFileHandle, sizeof(value), 1, &value);
		memcpy(ioFloatValue, &value, 4);
	}

	return bytesRead;
}

//----------------------------------------------------------------------
//	T3DSModel::readString
//
//	This method reads in a string of characters using fread().
//----------------------------------------------------------------------
size_t T3DSModel::readString(FILE* inFileHandle, string& outString)
{
	size_t result = 0;
	char buffer[260];

	// Loop until we get NULL
	const int kMaxStringLen = 256;
	int index = -1;
	do
	{
		index++;

		// Read in a character at a time until we hit NULL.
		result += read(inFileHandle, sizeof(char), 1, &buffer[index]);

	} while ((buffer[index] != '\0') && (index < kMaxStringLen));

	if (index > 200)
		LOG(WARNING) << "3DS string may be too long.";

	if (buffer[index] != '\0')
		buffer[index] = '\0';

	outString = buffer;

	// Return the total bytes read
	return result;
}

//----------------------------------------------------------------------
//	T3DSModel::readColorChunk
//
//	This method reads in the RGB color data which could be specified
//	using either 8-bit integers or 32-bit floats per channel.
//----------------------------------------------------------------------
void T3DSModel::readColorChunk(FILE* inFileHandle, float* ioColor, T3DSChunk* ioChunk)
{
	T3DSChunk tempChunk = { 0 };

	// Read the color chunk info
	readChunk(inFileHandle, &tempChunk);

	if (tempChunk.mID == M3D_CHUNKTYPE_COLOR_24)
	{
		// Read in the R G B color (3 bytes - 0 through 255)
		uint8_t color24[3];
		tempChunk.mBytesRead += read(inFileHandle, sizeof(uint8_t), 3, color24);

		// Add the bytes read to our chunk
		ioChunk->mBytesRead += tempChunk.mBytesRead;

		// Convert to OpenGL color
		ioColor[0] = (float)color24[0] / 255.0f;
		ioColor[1] = (float)color24[1] / 255.0f;
		ioColor[2] = (float)color24[2] / 255.0f;
	}
	else if (tempChunk.mID == M3D_CHUNKTYPE_COLOR_FLOAT)
	{
		tempChunk.mBytesRead += readFloat(inFileHandle, &ioColor[0]);
		tempChunk.mBytesRead += readFloat(inFileHandle, &ioColor[1]);
		tempChunk.mBytesRead += readFloat(inFileHandle, &ioColor[2]);

		// Convert to OpenGL color
		ioColor[0] /= 255.0f;
		ioColor[1] /= 255.0f;
		ioColor[2] /= 255.0f;
	}
}

//----------------------------------------------------------------------
//	T3DSModel::readPercentageChunk
//
//	This method reads in the percentage values.
//----------------------------------------------------------------------
void T3DSModel::readPercentageChunk(FILE* inFileHandle, float* ioPercent, T3DSChunk* ioChunk)
{
	T3DSChunk tempChunk = { 0 };

	// Read the shininess chunk info
	readChunk(inFileHandle, &tempChunk);

	if (tempChunk.mID == M3D_CHUNKTYPE_INT_PCT)
	{
		uint16_t percent;
		tempChunk.mBytesRead += read(inFileHandle, sizeof(percent), 1, &percent);
		*ioPercent = (float)percent / 100.0f;
	}
	else if (tempChunk.mID == M3D_CHUNKTYPE_FLOAT_PCT)
		tempChunk.mBytesRead += readFloat(inFileHandle, ioPercent);

	// Add the bytes read to our chunk
	ioChunk->mBytesRead += tempChunk.mBytesRead;
}

//----------------------------------------------------------------------
//	T3DSModel::readVertexIndices
//
//	This method reads in the indices for the vertex array.
//----------------------------------------------------------------------
void T3DSModel::readVertexIndices(FILE* inFileHandle, T3DSObject* ioObject, T3DSChunk* ioPreviousChunk)
{
	unsigned short index = 0;	// This is used to read in the current face index

	// In order to read in the vertex indices for the object, we need to first
	// read in the number of them, then read them in.  Remember,
	// we only want 3 of the 4 values read in for each face.  The fourth is
	// a visibility flag for 3D Studio Max that doesn't mean anything to us.

	// Read in the number of faces that are in this object (int)
	ioPreviousChunk->mBytesRead += read(inFileHandle, sizeof(ioObject->mNumFaces), 1, &ioObject->mNumFaces);
	if (ioObject->mNumFaces > 0)
	{
		ioObject->mFaces.clear();

		// Go through all of the faces in this object
		int i;
		for (i = 0; i < ioObject->mNumFaces; i++)
		{
			T3DSFace newFace;

			// Next, we read in the A then B then C index for the face, but ignore the 4th value.
			// The fourth value is a visibility flag for 3D Studio Max, we don't care about this.
			for (int j = 0; j < 4; j++)
			{
				// Read the first vertex index for the current face 
				ioPreviousChunk->mBytesRead += read(inFileHandle, sizeof(index), 1, &index);

				if (j < 3)
				{
					// Store the index in our face structure.
					newFace.mVertIndex[j] = index;
				}
			}

			// Store the face in the vector
			ioObject->mFaces.push_back(newFace);
		}

		// When calculating per-vertex lighting normals, we need to know which faces a vertex belongs to.
		// For each vertex, we maintain a list of face indices that identify every face the vertex is a member of.
		// We are guaranteed that vertices will have already been loaded according to the 3DS file format specification,
		// however, we should still make sure before we dereference the pointer in the following loop.

		if (ioObject->mVertices)
		{
			for (i = 0; i < ioObject->mNumFaces; i++)
			{
				// Store the face index for each vertex. We need this for super-fast vertex lighting calculations
				ioObject->mVertices[ioObject->mFaces[i].mVertIndex[0]].mFaceList.push_back(i);
				ioObject->mVertices[ioObject->mFaces[i].mVertIndex[1]].mFaceList.push_back(i);
				ioObject->mVertices[ioObject->mFaces[i].mVertIndex[2]].mFaceList.push_back(i);
			}
		}
	}
}

//----------------------------------------------------------------------
//	T3DSModel::readUVCoordinates
//
//	This method reads in the UV coordinates for the object.
//----------------------------------------------------------------------
void T3DSModel::readUVCoordinates(FILE* inFileHandle, T3DSObject* ioObject, T3DSChunk* ioPreviousChunk)
{
	// In order to read in the UV indices for the object, we need to first
	// read in the amount there are, then read them in.

	// Read in the number of UV coordinates there are (int)
	ioPreviousChunk->mBytesRead += read(inFileHandle, sizeof(ioObject->mNumTexCoords), 1, &ioObject->mNumTexCoords);
	if (ioObject->mNumTexCoords > 0)
	{
		// Allocate memory to hold the UV coordinates
		ioObject->mTexCoords = new Vec2f[ioObject->mNumTexCoords];
		if (ioObject->mTexCoords)
		{
			// Read in the texture coodinates (an array 2 float)
			float u, v;
			for (int i = 0; i < ioObject->mNumTexCoords; i++)
			{
				ioPreviousChunk->mBytesRead += readFloat(inFileHandle, &u);
				ioPreviousChunk->mBytesRead += readFloat(inFileHandle, &v);

				// OpenGL textures are upsidedown so we have to flip v
				v = 1 - v;

				ioObject->mTexCoords[i].x = u;
				ioObject->mTexCoords[i].y = v;
			}
		}
	}
}

//----------------------------------------------------------------------
//	T3DSModel::readSmoothingGroups
//
//	This method reads in smoothing group information.
//----------------------------------------------------------------------
void T3DSModel::readSmoothingGroups(FILE* inFileHandle, T3DSObject* ioObject, T3DSChunk* ioPreviousChunk)
{
	uint32_t value;
	for (int i = 0; i < ioObject->mNumFaces; i++)
	{
		ioPreviousChunk->mBytesRead += read(inFileHandle, sizeof(value), 1, &value);
		ioObject->mFaces[i].mSmoothingGroup = value;
	}

	if (ioObject->mNumFaces > 0)
		ioObject->mHasSmoothingInfo = true;
}

//----------------------------------------------------------------------
//	T3DSModel::readTranslationMatrix
//
//	This method reads in the translation matrix for an object.
//----------------------------------------------------------------------
void T3DSModel::readTranslationMatrix(FILE* inFileHandle, T3DSObject* ioObject, T3DSChunk* ioPreviousChunk)
{
	// Read matrix elements
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, (GLfloat*)&ioObject->mTranslation.m00);
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, (GLfloat*)&ioObject->mTranslation.m10);
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, (GLfloat*)&ioObject->mTranslation.m20);
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, (GLfloat*)&ioObject->mTranslation.m01);
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, (GLfloat*)&ioObject->mTranslation.m11);
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, (GLfloat*)&ioObject->mTranslation.m21);
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, (GLfloat*)&ioObject->mTranslation.m02);
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, (GLfloat*)&ioObject->mTranslation.m12);
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, (GLfloat*)&ioObject->mTranslation.m22);

	// Read local center vector
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, &ioObject->mLocalCenter.x);
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, &ioObject->mLocalCenter.y);
	ioPreviousChunk->mBytesRead += readFloat(inFileHandle, &ioObject->mLocalCenter.z);
}

//----------------------------------------------------------------------
//	T3DSModel::readVertices
//
//	This method reads in the vertices for an object.
//----------------------------------------------------------------------
void T3DSModel::readVertices(FILE* inFileHandle, T3DSObject* ioObject, T3DSChunk* ioPreviousChunk)
{
	// Like most chunks, before we read in the actual vertices, we need
	// to find out how many there are to read in.  Once we have that number
	// we then fread() them into our vertice array.

	// Read in the number of vertices (int)
	ioPreviousChunk->mBytesRead += read(inFileHandle, sizeof(ioObject->mNumVertices), 1, &(ioObject->mNumVertices));
	if (ioObject->mNumVertices > 0)
	{
		// Allocate the memory for the verts and initialize the structure
		ioObject->mVertices = new T3DSVertex[ioObject->mNumVertices];
		if (ioObject->mVertices)
		{
			// Read in the array of vertices (an array of 3 floats)
			for (int i = 0; i < ioObject->mNumVertices; i++)
			{
				ioPreviousChunk->mBytesRead += readFloat(inFileHandle, &ioObject->mVertices[i].mVertex.x);
				ioPreviousChunk->mBytesRead += readFloat(inFileHandle, &ioObject->mVertices[i].mVertex.y);
				ioPreviousChunk->mBytesRead += readFloat(inFileHandle, &ioObject->mVertices[i].mVertex.z);
			}

			// Now we should have all of the vertices read in.  Because 3D Studio Max
			// Models with the Z-Axis pointing up (strange and ugly I know!), we need
			// to flip the y values with the z values in our vertices.  That way it
			// will be normal, with Y pointing up.  If you prefer to work with Z pointing
			// up, then just delete this next loop.  Also, because we swap the Y and Z
			// we need to negate the Z to make it come out correctly.
			/*
			// Go through all of the vertices that we just read and swap the Y and Z values
			for (int i = 0; i < ioObject->mNumVertices; i++)
			{
			// Store off the Y value
			float fTempY = ioObject->mVertices[i].mVertex.y;

			// Set the Y value to the Z value
			ioObject->mVertices[i].mVertex.y = ioObject->mVertices[i].mVertex.z;

			// Set the Z value to the Y value,
			// but negative Z because 3D Studio max does the opposite.
			ioObject->mVertices[i].mVertex.z = -fTempY;
			}
			*/
		}
	}
}

//----------------------------------------------------------------------
//	T3DSModel::readObjectMaterial
//
//	This method reads in the material name assigned to the object
//	and sets the mMaterialID. A material is either the color or the
//	texture map of the object. It can also hold other information like
//	the brightness, shine, etc...
//----------------------------------------------------------------------
void T3DSModel::readObjectMaterial(FILE* inFileHandle, T3DSObject* ioObject, T3DSChunk* ioPreviousChunk)
{
	string materialName;	// This is used to hold the objects material name

	// Here we read the material name that is assigned to the current object.
	ioPreviousChunk->mBytesRead += readString(inFileHandle, materialName);

	// Now that we have a material name, we need to go through all of the materials
	// and check the name against each material.  When we find a material in our material
	// list that matches this name we just read in, then we assign that material index to
	// the mMaterialID of each face in the object.

	// Go through all of the material
	int materialID = -1;
	int materialIndex = 0;
	TMaterialVec_t::iterator material;
	for (material = mMaterials.begin(); material != mMaterials.end(); material++)
	{
		// If the material we just read in matches the current texture name
		if (material->mName == materialName)
		{
			materialID = materialIndex;
			break;
		}

		materialIndex++;
	}

	// Now read in which faces use this material
	if (materialID >= 0)
	{
		uint16_t numMaterialFaceAssociations = 0;
		ioPreviousChunk->mBytesRead += read(inFileHandle, sizeof(numMaterialFaceAssociations), 1, &numMaterialFaceAssociations);
		if (numMaterialFaceAssociations > 0)
		{
			uint16_t* faceIndices = new uint16_t[numMaterialFaceAssociations];
			if (faceIndices)
			{
				ioPreviousChunk->mBytesRead += read(inFileHandle, sizeof(uint16_t), numMaterialFaceAssociations, faceIndices);

				for (uint16_t i = 0; i < numMaterialFaceAssociations; i++)
				{
					if (faceIndices[i] < ioObject->mNumFaces)
						ioObject->mFaces[faceIndices[i]].mMaterialID = materialID;
				}

				delete[] faceIndices;
			}
		}
	}
	else
	{
		// Important - skip the un used bytes, we can't find this material, but we still need to skip the data
		ioPreviousChunk->mBytesRead += read(inFileHandle, 1, ioPreviousChunk->mLength - ioPreviousChunk->mBytesRead);
	}
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Post-load processing
//----------------------------------------------------------------------
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//	T3DSModel::sortFaces
//
//	After model is loaded, we need to sort the faces in each object by
//	descending opacity.
//----------------------------------------------------------------------
void T3DSModel::sortFaces()
{
	// If there are no objects, we can skip this part
	if (mObjects.size() <= 0)
		return;

	// Iterate over each object
	T3DSObjectVec_t::iterator object;
	for (object = mObjects.begin(); object != mObjects.end(); object++)
		std::sort(object->mFaces.begin(), object->mFaces.end(), T3DSFaceComparator(this));
}

//----------------------------------------------------------------------
//	T3DSModel::computeBoundingRadius
//
//	After model is loaded, we need to know the radius of the smallest
//	sphere that would completely enclose the model. Without this
//	knowledge, we can't properly set near and far clipping planes to 
//	maximize the effectiveness of the depth buffer. The difficulty is
//	that the vertex coordinates don't necessarily need to be centered at
//	origin. So we first have to compute a center vector	which we
//	translate all vertices by to move the model center to the origin.
//----------------------------------------------------------------------
void T3DSModel::computeBoundingRadius()
{
	mModelBoundingRadius = 0.0;

	// If there are no objects, we can skip this part
	if (mObjects.size() <= 0)
		return;

	// After inspecting various models with my ModelViewer, it is clear that 3DS models DO NOT
	// necessarily cluster around the origin. So we have to compute the center of the model. But
	// How to define "center" of model. This could be the done by summing the position of all
	// vertices, and dividing by the number of vertices to determine the average, but this does not
	// work very well with the concept of bounding radius since the computed center is a weighted
	// distribution of vertex complexity. In other words, the computed center will be skewed toward
	// higher vertex densities.
	// Instead, we will compute a bounding box for the model and then use the center of the box.

	// Compute bounding box around model
	int i;
	const GLfloat kBigFloat = 1e9f;
	Vec3f maxPt(-kBigFloat, -kBigFloat, -kBigFloat);
	Vec3f minPt(kBigFloat, kBigFloat, kBigFloat);
	T3DSObjectVec_t::iterator object;
	for (object = mObjects.begin(); object != mObjects.end(); object++)
	{
		// Get Steve Sanders to remove these objects from his models
		if (object->mName == "Plane")
			continue;

		// Find maximum and minimum ordinate values.
		for (i = 0; i < object->mNumVertices; i++)
		{
			if (object->mVertices[i].mVertex.x > maxPt.x)
				maxPt.x = object->mVertices[i].mVertex.x;
			if (object->mVertices[i].mVertex.y > maxPt.y)
				maxPt.y = object->mVertices[i].mVertex.y;
			if (object->mVertices[i].mVertex.z > maxPt.z)
				maxPt.z = object->mVertices[i].mVertex.z;

			if (object->mVertices[i].mVertex.x < minPt.x)
				minPt.x = object->mVertices[i].mVertex.x;
			if (object->mVertices[i].mVertex.y < minPt.y)
				minPt.y = object->mVertices[i].mVertex.y;
			if (object->mVertices[i].mVertex.z < minPt.z)
				minPt.z = object->mVertices[i].mVertex.z;
		}
	}

	// Now compute the vector to the center of the bounding box
	Vec3f diagonal = maxPt - minPt;
	Vec3f halfDiagonal(diagonal);
	halfDiagonal /= 2;
	Vec3f centerVec = minPt + halfDiagonal;

	// Translate all vertices to center of bounding volume and determine the length of the largest vertex vector
	float maxLengthSquared = 0.0f;
	for (object = mObjects.begin(); object != mObjects.end(); object++)
	{
		// Go though all of the vertices
		for (i = 0; i < object->mNumVertices; i++)
		{
			// Translate each vertex by the center vector so that our model is always centered at origin
			object->mVertices[i].mVertex -= centerVec;

			// Find the vertex furthest from center. We can skip the sqrt() until we're finished.
			float lengthSquared = object->mVertices[i].mVertex.lengthSquared();
			if (lengthSquared > maxLengthSquared)
				maxLengthSquared = lengthSquared;
		}
	}

	mModelBoundingRadius = sqrt(maxLengthSquared);
}

//----------------------------------------------------------------------
//	T3DSModel::computeNormals
//
//	This method is expensive. It must traverse every face of every object
//	and compute a normal vector for each face. These normal vector are needed
//	if we want the model to respond to lighting. Unfortunately, one lighting
//	normal per face is usually not sufficient. What is really needed is an
//	averaged normal at each vertex. The average is computed by summing together
//	the normal of each face that a particular vertex is part of and then
//	scalar-dividing the normal. All this because AutoDesk didn't want to include
//	per-vertex lighing normals in the model.
//----------------------------------------------------------------------
void T3DSModel::computeNormals()
{
	Vec3f vVector1, vVector2, vNormal, vPoly[3];

	// If there are no objects, we can skip this part
	if (mObjects.size() <= 0)
		return;

	// Go through each of the objects to calculate their normals
	T3DSObjectVec_t::iterator object;
	for (object = mObjects.begin(); object != mObjects.end(); object++)
	{
		if (object->mNumFaces > 0)
		{
			// Here we allocate all the memory we need to calculate the normals
			Vec3f* tempFaceNormals = new Vec3f[object->mNumFaces];
			if (tempFaceNormals)
			{
				// Go though all of the faces of this object and calculate the face normal
				int i;
				for (i = 0; i < object->mNumFaces; i++)
				{
					// To cut down LARGE code, we extract the 3 points of this face
					vPoly[0] = object->mVertices[object->mFaces[i].mVertIndex[0]].mVertex;
					vPoly[1] = object->mVertices[object->mFaces[i].mVertIndex[1]].mVertex;
					vPoly[2] = object->mVertices[object->mFaces[i].mVertIndex[2]].mVertex;

					// Now let's calculate the face normals (Get 2 vectors and find the cross product of those 2)

					vVector1 = vPoly[0] - vPoly[2];			// Get the vector of the polygon (we just need 2 sides for the normal)
					vVector2 = vPoly[2] - vPoly[1];			// Get a second vector of the polygon

					vNormal = vVector2 ^ vVector1;			// Return the cross product of the 2 vectors (normalize vector, but not a unit vector)
					tempFaceNormals[i] = vNormal;			// Save the un-normalized normal for the vertex normals
					object->mFaces[i].mNormal = vNormal;	// Normalize the cross product to give us the polygons normal
					object->mFaces[i].mNormal.normalize();
				}

				// Rant Alert!
				// The following code is bracketed by some limiting size factor thing since calculating
				// lighting normals per-vertex is bloody expensive. Autodesk should've included per-vertex
				// lighting normals in the 3DS file specification. So what if the file is a little larger. Jerks!
				// Anyway, rendering with per-polygon lighting normals isn't the end of the world, but using
				// per-vertex normals gives much nicer results. To avoid a major nested loop against object vertices
				// and object faces, each vertex has a list of faces it belongs to.
				if (object->mNumVertices && object->mHasSmoothingInfo)
				{
					// First, we have to determine all the smoothing groups associated with each vertex
					for (i = 0; i < object->mNumFaces; i++)				// Go through all of the faces
					{
						object->mVertices[object->mFaces[i].mVertIndex[0]].mSmoothingGroups |= object->mFaces[i].mSmoothingGroup;
						object->mVertices[object->mFaces[i].mVertIndex[1]].mSmoothingGroups |= object->mFaces[i].mSmoothingGroup;
						object->mVertices[object->mFaces[i].mVertIndex[2]].mSmoothingGroups |= object->mFaces[i].mSmoothingGroup;
					}

					//////////////// Now Get The Vertex Normals /////////////////
					for (i = 0; i < object->mNumVertices; i++)			// Go through all of the vertices
					{
						T3DSVertex* theVertex = &object->mVertices[i];
						T3DSFaceList_t::iterator faceIt;
						for (faceIt = theVertex->mFaceList.begin(); faceIt != theVertex->mFaceList.end(); faceIt++)
						{
							T3DSFace* theFace = &object->mFaces[*faceIt];

							// Check if the vertex smoothing groups intersects the face smoothing group
							uint32_t faceSmoothingGroup = theFace->mSmoothingGroup;
							if (faceSmoothingGroup | theVertex->mSmoothingGroups)
							{
								theVertex->mNormalMap[faceSmoothingGroup].mNormal = theVertex->mNormalMap[faceSmoothingGroup].mNormal + tempFaceNormals[*faceIt];
								theVertex->mNormalMap[faceSmoothingGroup].mSharedCount++;
							}
						}

						// Now we divide the mNormalMap Vec3f by the number shared
						T3DSNormalMap_t::iterator it;
						for (it = theVertex->mNormalMap.begin(); it != theVertex->mNormalMap.end(); it++)
						{
							if (it->second.mSharedCount > 1)
								it->second.mNormal /= (float_t)(it->second.mSharedCount);
							it->second.mNormal.normalize();
						}
					}

					object->mPerVertexNormals = true;
				}

				// Free our memory and start over on the next object
				delete[] tempFaceNormals;
			}
		}
	}
}

//----------------------------------------------------------------------
//	T3DSModel::loadTextures
//
//	After model is loaded, we need to load any external textures that the
//	model references. The external textures are assumed	to be located in
//	a subfolder with the same name of the model	file (less the extension).
//----------------------------------------------------------------------
void T3DSModel::loadTextures()
{
	//	We want to use a "default" texture if the conditions are right. Conditions under
	//	which the default texture is used are as follows:
	//
	//		- The model has only one object
	//		- The model has no material definitions
	//		- The model has texture coordinates defined
	//		- The number of texture coordinates is equal to the number of vertices
	//		- A file called "default.jpg" exists in the models texture folder
	//	
	//	These conditions will typically apply to planet-type objects, which is what we want.
	if ((mObjects.size() == 1) && (mMaterials.size() == 0))
	{
		T3DSObject* theObject = &mObjects[0];
		if ((theObject->mNumTexCoords > 0) && (theObject->mNumTexCoords == theObject->mNumVertices))
		{
			// Create a new T3DSMaterialInfo object and add it to mMaterials
			T3DSMaterialInfo theMaterial;

			theMaterial.mName = "default texture";
			theMaterial.mFilename = "default.png";
			memset(theMaterial.mDiffuseColor, 0, sizeof(theMaterial.mDiffuseColor));
			mMaterials.push_back(theMaterial);
		}
	}

	// Go through all the materials
	TMaterialVec_t::iterator material;
	for (material = mMaterials.begin(); material != mMaterials.end(); material++)
	{
		// Check to see if there is a file name to load in this material
		if (!material->mFilename.empty())
		{
			// Now look in the texture map to see if we already have this texture
			TModelTextureMap_t::iterator it = mTextureMap.find(material->mFilename);
			if (it != mTextureMap.end())
			{
				// The texture has already been loaded for another material so we just set the pointer
				material->mTexture = it->second;
			}
			else
			{
				// We didn't find the texture in the map so we need to create the texture and add it
				// to the map.

				// Create a LFile for the file containing the texture.
				// Rather than pollute the Data/Models folder with a whole wack of texture files for various
				// models, we require all external texture files to be located in a sub-folder under Models. The
				// name of the folder is the name of the 3DS file (less the .3ds extension). So for HST.3ds, we
				// require a "HST" folder in the Models folder.
				string texturePath = File::getModelsFolder().append("/").append(mTextureFolderName).append("/").append(material->mFilename);
				File textureFile(texturePath);
				if (textureFile.exists())
				{
					Texture* newTexture = new Texture(textureFile);
					if (newTexture)
					{
						// Get the texture up on the GPU
						if (newTexture->getImageBufferOK() && newTexture->sendToGPU())
						{
							// The new texture is successfully created so we can add it to the texture map
							// and point this material at it.
							mTextureMap[material->mFilename] = newTexture;
							material->mTexture = newTexture;

							// Set clamping mode to repeat for 3DS model textures
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
						}
						else
						{
							// If we fail to load the texture, delete the TOGLTexture object
							delete newTexture;
							newTexture = NULL;
						}
					}
				}
				else
				{
					// The model needs a texture file we do not have, report which file is missing
					LOG(ERROR) << "Missing 3DS model external texture file; " << material->mFilename << " while loading " << mFilename << ".";
				}
			}
		}
	}
}

//----------------------------------------------------------------------
//	T3DSModel::adjustTextureCoordinates
//
//	Applies material scaling and offset parameters to texture coordinates
//	used in materials.
//----------------------------------------------------------------------
void T3DSModel::adjustTextureCoordinates()
{
	// Iterate over objects
	T3DSObjectVec_t::iterator object;
	for (object = mObjects.begin(); object != mObjects.end(); object++)
	{
		// Skip object if it has no texture coords
		if (object->mTexCoords == NULL)
			continue;

		// Iterate over each face in the object
		for (int face = 0; face < object->mNumFaces; face++)
		{
			T3DSFace* theFace = &(object->mFaces[face]);

			// Obtain the material for the face
			if (theFace->mMaterialID >= 0)
			{
				T3DSMaterialInfo* theMaterial = &mMaterials[theFace->mMaterialID];

				// Scale values of zero are illegal
				if ((theMaterial->mUScale != 0.0f) && (theMaterial->mVScale != 0.0f))
				{
					// If scaling and translations params are different from 1 and 0, respectively,
					// then apply the parameters to each texture coordinate
					if ((theMaterial->mUScale != 1.0f) || ((theMaterial->mVScale != 1.0f)) ||
						(theMaterial->mUOffset != 0.0f) || ((theMaterial->mVOffset != 0.0f)))
					{
						// Iterate over each triangle composing the face
						for (int whichVertex = 0; whichVertex < 3; whichVertex++)
						{
							// Get the index for each point of the face
							int index = theFace->mVertIndex[whichVertex];

							// How to use mUScale, mVScale, mUOffset and mVOffset in material definition?
							// Taking a guess until some model with non-default info proves me wrong
							object->mTexCoords[index].x = (object->mTexCoords[index].x / theMaterial->mUScale) + theMaterial->mUOffset;
							object->mTexCoords[index].y = (object->mTexCoords[index].y / theMaterial->mVScale) + theMaterial->mVOffset;
						}
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------
//	T3DSModel::loadMetaData
//
//	Given full path to model, attempts to read an associated meta data
//	file containing application-specific info not stored in a 3DS file.
//----------------------------------------------------------------------
void T3DSModel::loadMetaData(File& inModelFile)
{
	string metaFilePath = inModelFile.getRelativePath();
	size_t dotPos = metaFilePath.find_last_of('.');
	if (dotPos != string::npos)
	{
		metaFilePath.erase(dotPos + 1);
		metaFilePath.append("meta");

		ConfigFileReader metaConfig(metaFilePath);
		if (metaConfig.hasValues())
		{
			float_t physicalRadiusInMetres;
			if (metaConfig.getConfigValue("PhysicalRadiusInMetres", physicalRadiusInMetres))
				mPhysicalRadiusInAU = physicalRadiusInMetres * (float_t)kAuPerMetre;
			if (metaConfig.getConfigValue("ModelUpVector", mModelUpVector))
				mModelUpVector.normalize();
			metaConfig.getConfigValue("InclinationAngleInDegrees", mInclinationAngleInDegrees);

			if (metaConfig.getConfigValue("RotationPeriodInDays", mRotationRateInRadiansPerCentury))
			{
				// For convenience to user, we store period, but we need rate.
				if (mRotationRateInRadiansPerCentury)
					mRotationRateInRadiansPerCentury = (float_t)(kTwicePi * kDaysPerCentury / mRotationRateInRadiansPerCentury);
			}

			mMetaDataLoaded = true;
		}
	}
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Render-related methods
//----------------------------------------------------------------------
//----------------------------------------------------------------------

void T3DSModel::setFBOSize(GLuint inWidth, GLuint inHeight)
{
	mFBOSize.x = (GLint)inWidth;
	mFBOSize.y = (GLint)inHeight;
}

//----------------------------------------------------------------------
//	T3DSModel::render
//
//	Renders the model.
//
//	We have a model that has a certain amount of objects and textures.  We want 
//	to go through each object in the model, bind it's texture map or set it's
//	material properties, and then render it.
//	To render the current object, we go through all of it's faces (polygons).  
//	A face (in this case) is just a triangle of the object.
//	For instance, a cube has 12 faces because each side has 2 triangles.
//	You might be thinking.  Well, if there are 12 faces in a cube, that makes
//	36 vertices that we need to read in for that object.  Not really true because
//	a lot of the vertices are the same, since they share sides, we only need to save
//	8 vertices, and ignore the duplicates.  Then, you have an array of all the
//	unique vertices in that object.  No 2 vertices will be the same.  This cuts down
//	on memory.  Then, another array is saved, which is the index numbers for each face,
//	which index in to that array of vertices.  That might sound silly, but it is better
//	than saving tons of duplicate vertices.  The same thing happens for UV coordinates.
//	You don't save duplicate UV coordinates, you just save the unique ones, then an array
//	that index's into them.  This might be confusing, but most 3D files use this format.
//----------------------------------------------------------------------
bool T3DSModel::render(Object& inObject)
{
	if (mShaderHandle == 0)
	{
		// This is a good time load the shader program
		ShaderProgram* shaderProgram = ShaderFactory::inst()->getShaderProgram("Models/3ds.vert",
																			   "Models/3ds.frag");
		if (shaderProgram)
			mShaderHandle = shaderProgram->getHandle();
	}
	if (mShaderHandle == 0)
		return false;

	// This is the rudiments of a viewer model coordinate system. We're positioning a viewer
	// and a model in Cartesian space, computing a viewer-model vector, applying a rotation
	// to the model, and a rotation to the viewer, and then letting the shader do the hard
	// work of fisheye projecting the resulting image.
	// We have to perform the viewer-model vector computation in software as in general,
	// all objects (including the viewer) will maintain 128-bit integer Cartesian positions.
	// We will perform the vector subtraction in 128-bit integer math, and then cast to
	// Vec3f.

	Camera* theCamera = gOpenGLWindow->getCamera();
	Vec3f viewerModelVector = theCamera->getCameraRelativePosition(inObject);

	// Decide if model is big enough (in pixels) to warrant rendering.
	const float_t kMinPixelDiameter = 5;
	float_t objectDistance = viewerModelVector.length();
	float_t pixelDiameter = theCamera->getObjectPixelDiameter(objectDistance, mPhysicalRadiusInAU);
	if (pixelDiameter < kMinPixelDiameter)
		return false;

	// viewerModelVector is in AU. We need to use mModelBoundingRadius and mPhysicalRadiusInAU
	// to correctly scale viewerModelVector so that things look correct. Compute modelUnitsPerAU.
	float_t modelUnitsPerAU = mModelBoundingRadius / mPhysicalRadiusInAU;
	viewerModelVector *= (GLfloat)modelUnitsPerAU;

	// Given the above info, we can compute if model is visible
	Vec3f viewDirection, upDirection, leftDirection;
	theCamera->getViewerOrthoNormalBasis(viewDirection, upDirection, leftDirection);
	GLfloat fisheyeAperture = theCamera->getAperture();
	float_t viewerModelLength = objectDistance * modelUnitsPerAU;
	double_t modelAngularRadius = atan(mModelBoundingRadius / viewerModelLength);
	Vec3f viewerModelVectorNorm = viewerModelVector / viewerModelLength;
	double_t angleBetween = acos((double_t)(viewDirection * viewerModelVectorNorm));
	if (angleBetween - modelAngularRadius > fisheyeAperture / 2)
	{
		// Model is not visible. Bail.
		return false;
	}

	// Apply translation to model to position it in world coordinates
	Mat4f modelTranslation = Mat4f::translation(viewerModelVector);

	// Orient model using mModelUpVector. For now, mModelUpVector is relative to universal coordinate system,
	// where (0,1,0) is considered "up".
	Quatf upRotation = Quatf::vecToVecRotation(mModelUpVector, Vec3f(0, 1, 0));

	// Apply rotation to model
	static GLfloat rotationY = 0;
	static GLfloat dRotationY = 0.25f;
	Quatf modelAxisRotation = Quatf::yrotation(degToRad(rotationY));
	Quatf modelRotation = upRotation * modelAxisRotation;

	// Compute model matrix to transform model coordinates to world coordinates
	Mat4f modelMatrix = modelTranslation * modelRotation.toMatrix4();

	// As long as we don't have any scaling, we can simply take the upper-left 3x3
	// matrix for transforming normals.
	Mat3f normalMatrix(modelMatrix);

	// Setup orthographic projection
	Vec2i sceneSize;
	gOpenGLWindow->getSceneSize(sceneSize);
	float h = 1, v = 1;
	if (sceneSize.x > sceneSize.y)
		h = (float)sceneSize.x / (float)sceneSize.y;
	else
		v = (float)sceneSize.y / (float)sceneSize.x;

	// We need to know (in eye coordinates) where the center of the model is
	// to correctly set the near and far plane of the ortho viewing volume.
	Vec4f modelPositionEye = modelMatrix * Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
	float_t modelDotView = modelPositionEye * viewDirection;
	float_t n = modelDotView - (GLfloat)mModelBoundingRadius;
	float_t f = modelDotView + (GLfloat)mModelBoundingRadius;
	Mat4f projectionMatrix = Mat4f::orthographic(-h, h, -v, v, n, f);

	// Light position will be set at eye location for now
	const GLfloat lightPositionEye[] = { 0, 0, 0 };
	const GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f };
	const GLfloat lightDiffuse[] = { 1, 1, 1 };
	const GLfloat lightSpecular[] = { 1, 1, 1 };

	// Need this to affect clipping vertices behind viewer
	glEnable(GL_CLIP_DISTANCE0);

	glUseProgram(mShaderHandle);
	{
//		gOpenGLWindow->mTimer.reset();

		// Draw the untextured vertices
		glUniform1i(glGetUniformLocation(mShaderHandle, "uIsTexturing"), GL_FALSE);
		glUniform1f(glGetUniformLocation(mShaderHandle, "uAperture"), fisheyeAperture);
		glUniform3fv(glGetUniformLocation(mShaderHandle, "uViewDirection"), 1, viewDirection.data);
		glUniform3fv(glGetUniformLocation(mShaderHandle, "uUpDirection"), 1, upDirection.data);
		glUniform3fv(glGetUniformLocation(mShaderHandle, "uLeftDirection"), 1, leftDirection.data);

		glUniform3fv(glGetUniformLocation(mShaderHandle, "uLight.position"), 1, lightPositionEye);
		glUniform3fv(glGetUniformLocation(mShaderHandle, "uLight.ambient"), 1, lightAmbient);
		glUniform3fv(glGetUniformLocation(mShaderHandle, "uLight.diffuse"), 1, lightDiffuse);
		glUniform3fv(glGetUniformLocation(mShaderHandle, "uLight.specular"), 1, lightSpecular);

		glUniformMatrix4fv(glGetUniformLocation(mShaderHandle, "uModelMatrix"), 1, 0, modelMatrix.data);
		glUniformMatrix4fv(glGetUniformLocation(mShaderHandle, "uProjectionMatrix"), 1, 0, projectionMatrix.data);
		glUniformMatrix3fv(glGetUniformLocation(mShaderHandle, "uNormalMatrix"), 1, 0, normalMatrix.data);
		
		glBindVertexArray(mVAOs[eUntexturedVAO]);
		glMultiDrawArrays(GL_TRIANGLES, mArrayFirstUntextured.data(), mArrayCountUntextured.data(), (GLsizei)mArrayCountUntextured.size());
		glBindVertexArray(0);

		// Draw the textured vertices
		glUniform1i(glGetUniformLocation(mShaderHandle, "uIsTexturing"), GL_TRUE);
		glBindVertexArray(mVAOs[eTexturedVAO]);
		for (int i = 0; i < mArrayFirstTextured.size(); i++)
		{
			const GLenum kTextureUnit = 0;
			glActiveTexture(GL_TEXTURE0 + kTextureUnit);
			glBindTexture(GL_TEXTURE_2D, mTextureIDs[i]);
			glUniform1i(glGetUniformLocation(mShaderHandle, "uTexture"), kTextureUnit);
			glDrawArrays(GL_TRIANGLES, mArrayFirstTextured[i], mArrayCountTextured[i]);
		}
		glBindVertexArray(0);

		// Unbind the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Done with the shader
		glUseProgram(0);

//		glFinish();
//		LOG(INFO) << "3DS model render time: " << gOpenGLWindow->mTimer.elapsedMicroseconds() << " us.";
	}
	glCheckForError();
	glDisable(GL_CLIP_DISTANCE0);

	rotationY += dRotationY;
	if (rotationY > 360)
		rotationY -= 360;

#if 0
	// Debugging shader
	Vec4f vPosition(5, 0, 0, 1);
	Vec4f vPositionEye = modelMatrix * vPosition;
	Vec3f vPositionEyeNorm(vPositionEye.x, vPositionEye.y, vPositionEye.z);
	float depthValue = vPositionEyeNorm.length();
	vPositionEyeNorm.normalize();

	Vec2f point;
	GLfloat eyePointViewDirectionAngle = acos(viewDirection * vPositionEyeNorm);
	if (eyePointViewDirectionAngle > 0)
	{
		Vec2f xyComponents(vPositionEyeNorm * leftDirection, vPositionEyeNorm * upDirection);
		xyComponents.normalize();
		GLfloat halfAperture = kFisheyeAperture * 0.5f;
		point.x = eyePointViewDirectionAngle * xyComponents.x / halfAperture;
		point.y = -eyePointViewDirectionAngle * xyComponents.y / halfAperture;
	}

	Vec4f gl_Position = projectionMatrix * Vec4f(point.x, point.y, -depthValue, 1.0f);
#endif

#if 0
	// Draw the model's coordinate axes
	if (mRenderCoordinateAxes)
	{
		GLdouble axisLength = mModelBoundingRadius;
//		inOpenGL->StateSet(TOGLDrawer::kOpaque);
//		inOpenGL->StateSetTexturingOff();
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(axisLength, 0.0, 0.0);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(0.0, axisLength, 0.0);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(0.0, 0.0, axisLength);
		glEnd();
	}
#endif

	return true;
}

void T3DSModel::setAtmosphereParameters(GLint inExpansionIterations, GLint inConvolutionIterations)
{
	mExpansionIterations = inExpansionIterations;
	mConvolutionIterations = inConvolutionIterations;
}

/*
//----------------------------------------------------------------------
//	T3DSModel::renderAtmosphere
//
//	Renders a hazy "atmosphere" around the model.
//
//	David Bradstreet and Steve Sanders have created 3DS models for
//	specific binary stars. However, without rendering an "atmosphere"
//	around the star, the model render looks, well, not convincing.
//	This code uses FBO textures and shaders to perform some	post-processing
//	on the model render to ultimately apply a nice haze around the model.
//----------------------------------------------------------------------
void T3DSModel::renderAtmosphere()
{
	mConvolvedTextureID = 0;
	TRenderTextureFactory* theFBOInstance = TRenderTextureFactory::Instance();
	if (theFBOInstance)
	{
		GLint fSavedViewport[4];
		glGetIntegerv(GL_VIEWPORT, fSavedViewport);

		// Change viewport to match FBO texture size
		glViewport(0, 0, mFBOSize.x, mFBOSize.y);

		const GLuint k3DSModelFBOName1 = 31;	// Some number likely not used elsewhere

		// Render the model to a texture
		if (theFBOInstance->BeginCapture(k3DSModelFBOName1, mFBOSize.x, mFBOSize.y, false, false, false, true))
		{
			glShadeModel(GL_SMOOTH);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glCallList(mDisplayList);

			theFBOInstance->EndCapture();

			// We are now about to perform some image filtering using more FBO's and shaders. We need to modify
			// the projection and modelview matrices for these operations
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(0.0, (GLdouble)mFBOSize.x, (GLdouble)mFBOSize.y, 0.0);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			// We need to expand the image in the results of k3DSModelFBOName1 so that when we subseqently
			// convolve the image, it is larger than the original, looking like atmosphere.
			GLuint srcTex = theFBOInstance->GetTextureID(k3DSModelFBOName1, mFBOSize.x, mFBOSize.y, true);
			GLuint expandedImage = performImageExpansion(srcTex, mExpansionIterations);

			// Now perform separable convolution
			mConvolvedTextureID = performConvolution(expandedImage, mConvolutionIterations);
		}

		// Restore the viewport
		glViewport(fSavedViewport[0], fSavedViewport[1], fSavedViewport[2], fSavedViewport[3]);
	}
}

// CLW: Oct 31, 2014: Just built this to compute coefficients for convolution shaders.
void computeWeightsAndOffsets(int n)
{
	// Compute n'th line of binomial coefficients
	vector<int> row(n + 1);

	row[0] = 1; //First element is always 1
	for (int i = 1; i<n / 2 + 1; i++){ //Progress up, until reaching the middle value
		row[i] = row[i - 1] * (n - i + 1) / i;
	}
	for (int i = n / 2 + 1; i <= n; i++){ //Copy the inverse of the first part
		row[i] = row[n - i];
	}

	int numDiscrete = (n + 1 - 4) / 2 + 1;
	vector<int> dOffsets(numDiscrete);
	for (int i = 0; i < numDiscrete; i++)
		dOffsets[i] = i;

	int dDenom = 0;
	for (int i = 0; i < numDiscrete * 2 - 1; i++)
		dDenom += row[2 + i];

	vector<double> dWeights(numDiscrete);
	int j = 0;
	for (int i = 2 + numDiscrete - 1; i >= 2; i--)
		dWeights[j++] = (double)row[i] / (double)dDenom;

	int numLinear = numDiscrete / 2 + 1;
	vector<double> lOffsets(numLinear);
	vector<double> lWeights(numLinear);
	lWeights[0] = dWeights[0];
	lOffsets[0] = 0.0;
	j = 1;
	for (int i = 1; i < numLinear; i++)
	{
		lWeights[i] = dWeights[j] + dWeights[j + 1];
		lOffsets[i] = (dOffsets[j] * dWeights[j] + dOffsets[j + 1] * dWeights[j + 1]) / lWeights[i];
		j += 2;
	}

	int i = 0;
	i++;
}

//----------------------------------------------------------------------
//	T3DSModel::performImageExpansion
//
//	Enlarges the rendered areas in the given source texture.
//
//	Prior to applying a convolution filter to the 3DS model, we need to
//	expand the portions in the texture so that the blury haze extends
//	beyond the edges of the model.
//----------------------------------------------------------------------
GLuint T3DSModel::performImageExpansion(GLuint inSrcTextureID, int inIterations)
{
	TRenderTextureFactory* theFBOInstance = TRenderTextureFactory::Instance();
	if (theFBOInstance == NULL)
		return 0;

	TGLSLProgramFactory* theGLSLProgramFactory = TGLSLProgramFactory::Instance();
	if (theGLSLProgramFactory == NULL)
		return 0;

	TGLSLProgram* expandShader = theGLSLProgramFactory->GetInstance("Vertex/FixedFunc", NULL, "ImageProc/Expand");
	if (expandShader == NULL)
		return 0;

	if (!expandShader->IsUsable())
		return 0;

	const GLuint kImageExpansionFBOName1 = 32;	// Some number likely not used elsewhere
	const GLuint kImageExpansionFBOName2 = 33;	// Some number likely not used elsewhere

	GLuint result = 0;

	GLuint inputTextureID = inSrcTextureID;

	int i = 0;
	while (i < inIterations)
	{
		if (theFBOInstance->BeginCapture(kImageExpansionFBOName1, mFBOSize.x, mFBOSize.y, false, false, false, false))
		{
			if (expandShader->Activate())
			{
				expandShader->SetUniformSampler2D("uSourceImage", 0, inputTextureID);
				expandShader->SetUniformVariable("uImageWidth", (GLfloat)mFBOSize.x);
				expandShader->SetUniformVariable("uImageHeight", (GLfloat)mFBOSize.y);

				glBegin(GL_QUADS);
				glVertex2i(0, 0);
				glVertex2i(0, mFBOSize.y);
				glVertex2i(mFBOSize.x, mFBOSize.y);
				glVertex2i(mFBOSize.x, 0);
				glEnd();

				expandShader->Deactivate();
			}

			theFBOInstance->EndCapture();

			GLuint fbo2Tex = theFBOInstance->GetTextureID(kImageExpansionFBOName1, mFBOSize.x, mFBOSize.y, false);
			i++;
			if (i == inIterations)
			{
				result = fbo2Tex;
				break;
			}
			
			if (theFBOInstance->BeginCapture(kImageExpansionFBOName2, mFBOSize.x, mFBOSize.y, false, false, false, false))
			{
				if (expandShader->Activate())
				{
					expandShader->SetUniformSampler2D("uSourceImage", 0, fbo2Tex);
					expandShader->SetUniformVariable("uImageWidth", (GLfloat)mFBOSize.x);
					expandShader->SetUniformVariable("uImageHeight", (GLfloat)mFBOSize.y);

					glBegin(GL_QUADS);
					glVertex2i(0, 0);
					glVertex2i(0, mFBOSize.y);
					glVertex2i(mFBOSize.x, mFBOSize.y);
					glVertex2i(mFBOSize.x, 0);
					glEnd();

					expandShader->Deactivate();
				}

				theFBOInstance->EndCapture();

				result = theFBOInstance->GetTextureID(kImageExpansionFBOName2, mFBOSize.x, mFBOSize.y, false);
			}
		}

		inputTextureID = result;
		i++;
	}

	glActiveTexture(GL_TEXTURE0_ARB);

	return result;
}

//----------------------------------------------------------------------
//	T3DSModel::performConvolution
//
//	Applies a separable Gaussian blur to the source texture.
//----------------------------------------------------------------------
GLuint T3DSModel::performConvolution(GLuint inSrcTextureID, int inIterations)
{
	TRenderTextureFactory* theFBOInstance = TRenderTextureFactory::Instance();
	if (theFBOInstance == NULL)
		return 0;

	TGLSLProgramFactory* theGLSLProgramFactory = TGLSLProgramFactory::Instance();
	if (theGLSLProgramFactory == NULL)
		return 0;

	TGLSLProgram* convHShader = theGLSLProgramFactory->GetInstance("Vertex/FixedFunc", NULL, "ImageProc/ConvH");
	if (convHShader == NULL)
		return 0;

	if (!convHShader->IsUsable())
		return 0;

	TGLSLProgram* convVShader = theGLSLProgramFactory->GetInstance("Vertex/FixedFunc", NULL, "ImageProc/ConvV");
	if (convVShader == NULL)
		return 0;

	if (!convVShader->IsUsable())
		return 0;

	const GLuint kHorizontalConvolveFBOName = 32;	// Some number likely not used elsewhere
	const GLuint kVerticalConvolveFBOName = 33;		// Some number likely not used elsewhere

	GLuint result = 0;

	GLuint inputTextureID = inSrcTextureID;

	for (int i = 0; i < inIterations; i++)
	{
		// Perform horizontal convolution
		if (theFBOInstance->BeginCapture(kHorizontalConvolveFBOName, mFBOSize.x, mFBOSize.y, false, false, false, false))
		{
			if (convHShader->Activate())
			{
				convHShader->SetUniformSampler2D("uSourceImage", 0, inputTextureID);
				convHShader->SetUniformVariable("uImageWidth", (GLfloat)mFBOSize.x);
				convHShader->SetUniformVariable("uImageHeight", (GLfloat)mFBOSize.y);

				glBegin(GL_QUADS);
				glVertex2i(0, 0);
				glVertex2i(0, mFBOSize.y);
				glVertex2i(mFBOSize.x, mFBOSize.y);
				glVertex2i(mFBOSize.x, 0);
				glEnd();

				convHShader->Deactivate();
			}

			theFBOInstance->EndCapture();

			// Perform vertical convolution
			GLuint fbo2Tex = theFBOInstance->GetTextureID(kHorizontalConvolveFBOName, mFBOSize.x, mFBOSize.y, false);
			if (theFBOInstance->BeginCapture(kVerticalConvolveFBOName, mFBOSize.x, mFBOSize.y, false, false, false, false))
			{
				if (convVShader->Activate())
				{
					convVShader->SetUniformSampler2D("uSourceImage", 0, fbo2Tex);
					convVShader->SetUniformVariable("uImageWidth", (GLfloat)mFBOSize.x);
					convVShader->SetUniformVariable("uImageHeight", (GLfloat)mFBOSize.y);

					glBegin(GL_QUADS);
					glVertex2i(0, 0);
					glVertex2i(0, mFBOSize.y);
					glVertex2i(mFBOSize.x, mFBOSize.y);
					glVertex2i(mFBOSize.x, 0);
					glEnd();

					convVShader->Deactivate();
				}

				theFBOInstance->EndCapture();

				result = theFBOInstance->GetTextureID(kVerticalConvolveFBOName, mFBOSize.x, mFBOSize.y, false);
			}

			inputTextureID = result;
		}
	}

	glActiveTexture(GL_TEXTURE0_ARB);

	return result;
}
*/