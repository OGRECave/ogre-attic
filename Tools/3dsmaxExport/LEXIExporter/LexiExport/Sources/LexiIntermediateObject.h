/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Author(s):
Marty Rabens

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

#ifndef __LexiExporter_IntermediateObject__
#define __LexiExporter_IntermediateObject__

class CIntermediateObject
{
public:
	CIntermediateObject(unsigned int iNodeID);
	virtual	~CIntermediateObject();

	Ogre::String GetObjectClass() {return m_sObjectClass;}

	unsigned int	GetNodeID() const {return m_iNodeID;}
	void	AddInstanceID(unsigned int id) {m_setInstanceIDs.insert(id);}
	std::set<unsigned int>	*GetInstanceIDs() {return &m_setInstanceIDs;}
	void	SetBaseInstanceObject(CIntermediateObject *pObj) {m_pBaseInstanceObject = pObj;}
	CIntermediateObject	*GetBaseInstanceObject() {return (m_pBaseInstanceObject == NULL) ? this : m_pBaseInstanceObject;}

	// Generic value storage
	void	SetValue(const Ogre::String &sName, void *pVal);
	void	SetValue(const Ogre::String &sName, float fVal);
	void	SetValue(const Ogre::String &sName, Ogre::ColourValue val);
	void	SetValue(const Ogre::String &sName, Ogre::String sVal);
	void	SetValue(const Ogre::String &sName, const char* sVal);
	void	SetValue(const Ogre::String &sName, bool);
	void	SetValue(const Ogre::String &sName, Ogre::Quaternion qVal);
	void	SetValue(const Ogre::String &sName, Ogre::Vector3 vVal);

	void*	GetPointerValue(const Ogre::String &sName);
	float	GetFloatValue(const Ogre::String &sName);
	const Ogre::ColourValue GetColourValue(const Ogre::String &sName);
	Ogre::String	GetStringValue(const Ogre::String &sName);
	bool	GetBoolValue(const Ogre::String &sName);
	Ogre::Quaternion	GetQuatValue(const Ogre::String &sName);
	Ogre::Vector3	GetVec3Value(const Ogre::String &sName);

protected:
	std::string m_sObjectClass;
	unsigned int m_iNodeID;
	std::set<unsigned int>	m_setInstanceIDs;
	CIntermediateObject	*m_pBaseInstanceObject;
	
	std::map<Ogre::String,void*>	m_mapPointerVal;
	std::map<Ogre::String,float>	m_mapFloatVal;
	std::map<Ogre::String,Ogre::ColourValue>	m_mapColorVal;
	std::map<Ogre::String,Ogre::String>	m_mapStringVal;
	std::map<Ogre::String,bool>		m_mapBoolVal;
	std::map<Ogre::String,Ogre::Quaternion>	m_mapQuatVal;
	std::map<Ogre::String,Ogre::Vector3>	m_mapVec3Val;
};

#endif __LexiExporter_IntermediateObject__
	