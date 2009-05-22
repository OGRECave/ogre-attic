/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __Ogre_TerrainMaterialGenerator_H__
#define __Ogre_TerrainMaterialGenerator_H__

#include "OgreTerrainPrerequisites.h"


namespace Ogre
{
	class Terrain;

	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Terrain
	*  Some details on the terrain component
	*  @{
	*/


	/** Enumeration of types of data that can be read from textures that are
	specific to a given layer. Notice that global texture information 
	such as shadows, horizons and terrain normals are not represented
	here because they are not a per-layer attribute, and blending
	is stored in packed texture structures which are stored separately.
	*/
	enum TerrainLayerSamplerSemantic
	{
		/// Albedo colour (diffuse reflectance colour)
		LSS_ALBEDO = 0,
		/// Tangent-space normal information from a detail texture
		LSS_NORMAL = 1,
		/// Height information for the detail texture
		LSS_HEIGHT = 2,
		/// Specular reflectance
		LSS_SPECULAR = 3
	};

	/** Information about one element of a sampler / texture within a layer. 
	*/
	struct _OgreTerrainExport TerrainLayerSamplerElement
	{
		/// The source sampler index of this element relative to LayerDeclaration's list
		uint8 source;
		/// The semantic this element represents
		TerrainLayerSamplerSemantic semantic;
		/// The colour element at which this element starts
		uint8 elementStart;
		/// The number of colour elements this semantic uses (usually standard per semantic)
		uint8 elementCount;

		bool operator==(const TerrainLayerSamplerElement& e) const
		{
			return source == e.source &&
				semantic == e.semantic &&
				elementStart == e.elementStart &&
				elementCount == e.elementCount;
		}
	};
	typedef vector<TerrainLayerSamplerElement>::type TerrainLayerSamplerElementList;

	/** Description of a sampler that will be used with each layer. 
	*/
	struct _OgreTerrainExport TerrainLayerSampler
	{
		/// A descriptive name that is merely used to assist in recognition
		String alias;
		/// The format required of this texture
		PixelFormat format;

		bool operator==(const TerrainLayerSampler& s) const
		{
			return alias == s.alias && format == s.format;
		}
	};
	typedef vector<TerrainLayerSampler>::type TerrainLayerSamplerList;

	/** The definition of the information each layer will contain in this terrain.
	All layers must contain the same structure of information, although the
	input textures can be different per layer instance. 
	*/
	struct _OgreTerrainExport TerrainLayerDeclaration
	{
		TerrainLayerSamplerList samplers;
		TerrainLayerSamplerElementList elements;

		bool operator==(const TerrainLayerDeclaration& dcl) const
		{
			return samplers == dcl.samplers && elements == dcl.elements;
		}
	};

	/** Class that provides functionality to generate materials for use with a terrain.
	@remarks
		Terrains are composed of one or more layers of texture information, and
		require that a material is generated to render them. There are various approaches
		to rendering the terrain, which may vary due to:
		<ul><li>Hardware support (static)</li>
		<li>Texture instances assigned to a particular terrain (dynamic in an editor)</li>
		<li>User selection (e.g. changing to a cheaper option in order to increase performance, 
		or in order to test how the material might look on other hardware (dynamic)</li>
		</ul>
		Subclasses of this class are responsible for responding to these factors and
		to generate a terrain material. 
		@par
		In order to cope with both hardware support and user selection, the generator
		must expose a number of named 'profiles'. These profiles should function on
		a known range of hardware, and be graded by quality. At runtime, the user 
		should be able to select the profile they wish to use (provided hardware
		support is available). 
	*/
	class _OgreTerrainExport TerrainMaterialGenerator : public TerrainAlloc
	{
	public:
		/** Inner class which should also be subclassed to provide profile-specific 
			material generation.
		*/
		class _OgreTerrainExport Profile : public TerrainAlloc
		{
		protected:
			String mName;
			String mDesc;
		public:
			Profile(const String& name, const String& desc)
				: mName(name), mDesc(desc) {}
			Profile(const Profile& prof) 
				: mName(prof.mName), mDesc(prof.mDesc) {}
			virtual ~Profile() {}
			/// Get the name of this profile
			const String& getName() const { return mName; }
			/// Get the description of this profile
			const String& getDescription() const { return mDesc; }
			
			/// Generate / resuse a material for the terrain
			virtual MaterialPtr generate(const Terrain* terrain) = 0;				

		};

		TerrainMaterialGenerator() 
			: mActiveProfile(0), mProfileChangeCounter(0) {}
		virtual ~TerrainMaterialGenerator()
		{
			for (ProfileList::iterator i = mProfiles.begin(); i != mProfiles.end(); ++i)
				OGRE_DELETE *i;
		}

		/// List of profiles - NB should be ordered in descending complexity
		typedef vector<Profile*>::type ProfileList;
	
		/** Get the list of profiles that this generator supports.
		*/
		virtual const ProfileList& getProfiles() const { return mProfiles; }

		/** Set the active profile by name. */
		virtual void setActiveProfile(const String& name)
		{
			if (!mActiveProfile || mActiveProfile->getName() != name)
			{
				for (ProfileList::iterator i = mProfiles.begin(); i != mProfiles.end(); ++i)
				{
					if ((*i)->getName() == name)
					{
						setActiveProfile(*i);
						break;
					}
				}
			}

		}

		/** Set the active Profile. */
		virtual void setActiveProfile(Profile* p)
		{
			if (mActiveProfile != p)
			{
				mActiveProfile = p;
				++mProfileChangeCounter;
			}
		}
		/// Get the active profile
		Profile* getActiveProfile() const 
		{ 
			// default if not chosen yet
			if (!mActiveProfile && !mProfiles.empty())
				mActiveProfile = mProfiles[0];

			return mActiveProfile; 
		}

		/** Returns the number of times an active profile has been changed.
		This can be used to determine if the profile has changed since a material
		was last generated.
		*/
		unsigned long long int getProfileChangeCount() const { return mProfileChangeCounter; }

		/** Get the layer declaration that this material generator operates with.
		*/
		virtual const TerrainLayerDeclaration& getLayerDeclaration() const { return mLayerDecl; }
		/** Whether this generator can generate a material for a given declaration. 
			By default this only returns true if the declaration is equal to the 
			standard one returned from getLayerDeclaration, but if a subclass wants
			to be flexible to generate materials for other declarations too, it 
			can specify here. 
		*/
		virtual bool canGenerateUsingDeclaration(const TerrainLayerDeclaration& decl)
		{
			return decl == mLayerDecl;
		}

		/** Generate a material for the given terrain using the active profile.
		*/
		virtual MaterialPtr generate(const Terrain* terrain)
		{
			Profile* p = getActiveProfile();
			if (!p)
				return MaterialPtr();
			else
				return p->generate(terrain);
		}
	protected:

		ProfileList mProfiles;
		mutable Profile* mActiveProfile;
		unsigned long long int mProfileChangeCounter;
		TerrainLayerDeclaration mLayerDecl;



	};

	typedef vector<TerrainMaterialGenerator*>::type TerrainMaterialGeneratorList;

}
#endif

