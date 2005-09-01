/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#ifndef __OgreSimpleManualObject_H__
#define __OgreSimpleManualObject_H__

#include "OgrePrerequisites.h"
#include "OgreMovableObject.h"
#include "OgreRenderable.h"


namespace Ogre
{
	/** Class providing a much simplified interface to generating manual
	 	objects with custom geometry.
	@remarks
		Building one-off geometry objects manually usually requires getting
		down and dirty with the vertex buffer and vertex declaration API, 
		which some people find a steep learning curve. This class gives you 
		a simpler interface specifically for the purpose of building a one-off
		3D object simply and quickly. Note that if you intend to instance your
		object you will still need to become familiar with the Mesh class. 
	@par
		This class draws heavily on the interface for OpenGL 
		immediate-mode (glBegin, glVertex, glNormal etc), since this
		is generally well-liked by people. There are a couple of differences
		in the results though - internally this class still builds hardware 
		buffers which can be re-used, so you can render the resulting object
		multiple times without re-issuing all the same commands again. 
		Secondly, the rendering is not immediate, it is still queued just like
		all OGRE objects. This makes this object more efficient than the 
		equivalent GL immediate-mode commands, so it's feasible to use it for
		large objects if you really want to.
	@par
		To construct some geometry with this object:
		  -# If you know roughly how many vertices (and indices, if you use them)
		     you're going to submit, call estimateVertexCount and estimateIndexCount.
			 This is not essential but will make the process more efficient by saving
			 memory reallocations.
		  -# Call begin() to begin entering data
		  -# For each vertex, call position(), normal(), textureCoord(), colour()
		     to define your vertex data. Note that each time you call position()
			 you start a new vertex. Note that the first vertex defines the 
			 components of the vertex - you can't add more after that. For example
			 if you didn't call normal() in the first vertex, you cannot call it
			 in any others. You ought to call the same combination of methods per
			 vertex.
		  -# If you want to define triangles (or lines/points) by indexing into the vertex list, 
			 you can call index() as many times as you need to define them.
			 If you don't do this, the class will assume you want triangles drawn
			 directly as defined by the vertex list, ie non-indexed geometry.
		  -# Call end() to finish entering data. 
	    After calling end(), the class will organise the data internally and make
		it ready to render with. Like any other MovableObject you should attach 
		the object to a SceneNode to make it visible. Other aspects like the 
		material to use and the relative render order can be controlled
		using the methods setMaterialName and setRenderQueueGroup.
	*/
	class _OgreExport SimpleManualObject : public MovableObject, public Renderable
	{
	public:
		SimpleManualObject(const String& name);
		virtual ~SimpleManualObject();

		/** Start defining the object.
		@param opType The type of operation to use to render. 
		*/
		virtual void begin(RenderOperation::OperationType opType = RenderOperation::OT_TRIANGLE_LIST);
		/** Add a vertex position, starting a new vertex at the same time. 
		@remarks A vertex position is slightly special among the other vertex data
			methods like normal() and textureCoord(), since calling it indicates
			the start of a new vertex. All other vertex data methods you call 
			after this are assumed to be adding more information (like normals or
			texture coordinates) to the last vertex started with position().
		*/
		virtual void position(const Vector3& pos);
		/// @copydoc SimpleManualObject::position(const Vector3&)
		virtual void position(Real x, Real y, Real z);

		/** Add a vertex normal to the current vertex.
		@remarks
			Vertex normals are most often used for dynamic lighting, and 
			their components should be normalised.
		*/
		virtual void normal(const Vector4& norm);
		/// @copydoc SimpleManualObject::normal(const Vector3&)
		virtual void normal(Real x, Real y, Real z);

		/** Add a texture coordinate to the current vertex.
		@remarks
			You can call this method multiple times between position() calls
			to add multiple texture coordinates to a vertex. Each one can have
			between 1 and 3 dimensions, depending on your needs, although 2 is
			most common. There are several versions of this method for the 
			variations in number of dimensions.
		*/
		virtual void textureCoord(Real u);
		/// @copydoc SimpleManualObject::textureCoord(Real)
		virtual void textureCoord(Real u, Real v);
		/// @copydoc SimpleManualObject::textureCoord(Real)
		virtual void textureCoord(Real u, Real v, Real w);
		/// @copydoc SimpleManualObject::textureCoord(Real)
		virtual void textureCoord(const Vector2& uv);
		/// @copydoc SimpleManualObject::textureCoord(Real)
		virtual void textureCoord(const Vector3& uvw);

		/** Add a vertex colour to a vertex.
		*/
		virtual void colour(const ColourValue& col);
		/** Add a vertex colour to a vertex.
		@param r,g,b,a Colour components expressed as floating point numbers from 0-1
		*/
		virtual void colour(Real r, Real g, Real b, Real a = 1.0f);
		/** Add a vertex colour to a vertex.
		@param col Packed colour value which should have been generated from 
			RenderSystem::convertColourValue. 
		*/
		virtual void colour(RGBA col);

		/** Add a vertex index to construct faces / lines / points via indexing
			rather than just by a simple list of vertices. 
		@remarks
			You will have to call this 3 times for each face for a triangle list, 
			or use the alternative 3-parameter version. Other operation types
			require different numbers of indexes, @see RenderOperation::OperationType.
		@note
			32-bit indexes are not supported on all cards which is why this 
			class only allows 16-bit indexes, for simplicity and ease of use.
		@param idx A vertex index from 0 to 65535. 
		*/
		virtual void index(uint16 idx);
		/** Add a set of 3 vertex indices to construct a face; this is a shortcut
			to calling the single-parameter version 3 times for for triangle lists.
		@note
			32-bit indexes are not supported on all cards which is why this 
			class only allows 16-bit indexes, for simplicity and ease of use.
		@param i1, i2, i3 3 vertex indices from 0 to 65535 defining a face. 
		*/
		virtual void index(uint16 i1, uint16 i2, uint16 i3);

		/** Finish defining the object and compile the final renderable version. */
		virtual void end(void);

		// MovableObject overrides

		/** @copydoc MovableObject::getMovableType. */
		const String& getMovableType(void) const;
		/** @copydoc MovableObject::getBoundingBox. */
		const AxisAlignedBox& getBoundingBox(void) const;
		/** @copydoc MovableObject::getBoundingRadius. */
		Real getBoundingRadius(void) const;
		/** @copydoc MovableObject::_updateRenderQueue. */
		void _updateRenderQueue(RenderQueue* queue);

		// Renderable overrides
		/** @copydoc Renderable::getMaterial. */
		const MaterialPtr& getMaterial(void) const;
		/** @copydoc Renderable::getRenderOperation. */
		void getRenderOperation(RenderOperation& op);
		/** @copydoc Renderable::getWorldTransforms. */
		void getWorldTransforms(Matrix4* xform) const;
		/** @copydoc Renderable::getWorldOrientation. */
		const Quaternion& getWorldOrientation(void) const;
		/** @copydoc Renderable::getWorldPosition. */
		const Vector3& getWorldPosition(void) const;


	};


	/** Factory object for creating SimpleManualObject instances */
	class _OgreExport SimpleManualObjectFactory : public MovableObjectFactory
	{
	protected:
		MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params);
	public:
		SimpleManualObjectFactory() {}
		~SimpleManualObjectFactory() {}

		static String FACTORY_TYPE_NAME;

		const String& getType(void) const;
		void destroyInstance( MovableObject* obj);  

	};


#endif

