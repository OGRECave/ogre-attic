#include "LexiStdAfx.h"
#include "LexiIntermediateAPI.h"

CIntermediateObject::CIntermediateObject(unsigned int iNodeID)
{
	m_sObjectClass = "CIntermediateObject";
	m_iNodeID = iNodeID;
	m_pBaseInstanceObject = NULL;
}


CIntermediateObject::~CIntermediateObject()
{

}

void	CIntermediateObject::SetValue(const Ogre::String &sName, void *pVal)
{
	m_mapPointerVal[sName] = pVal;
}

void	CIntermediateObject::SetValue(const Ogre::String &sName, float fVal)
{
	m_mapFloatVal[sName] = fVal;
}

void	CIntermediateObject::SetValue(const Ogre::String &sName, Ogre::ColourValue val)
{
	m_mapColorVal[sName] = val;
}

void	CIntermediateObject::SetValue(const Ogre::String &sName, Ogre::String sVal)
{
	m_mapStringVal[sName] = sVal;
}

void	CIntermediateObject::SetValue(const Ogre::String &sName, const char* sVal)
{
	m_mapStringVal[sName] = sVal;
}

void	CIntermediateObject::SetValue(const Ogre::String &sName, bool bVal)
{
	m_mapBoolVal[sName] = bVal;
}

void	CIntermediateObject::SetValue(const Ogre::String &sName, Ogre::Quaternion qVal)
{
	m_mapQuatVal[sName] = qVal;
}

void	CIntermediateObject::SetValue(const Ogre::String &sName, Ogre::Vector3 vVal)
{
	m_mapVec3Val[sName] = vVal;
}


void*	CIntermediateObject::GetPointerValue(const Ogre::String &sName)
{
	std::map<Ogre::String,void*>::iterator it = m_mapPointerVal.find(sName);
	return it != m_mapPointerVal.end() ? it->second : NULL;
}

float	CIntermediateObject::GetFloatValue(const Ogre::String &sName)
{
	std::map<Ogre::String,float>::iterator it = m_mapFloatVal.find(sName);
	return it != m_mapFloatVal.end() ? it->second : 0;
}

const Ogre::ColourValue CIntermediateObject::GetColourValue(const Ogre::String &sName)
{
	std::map<Ogre::String,Ogre::ColourValue>::iterator it = m_mapColorVal.find(sName);
	return it != m_mapColorVal.end() ? it->second : Ogre::ColourValue::Black;
}

Ogre::String	CIntermediateObject::GetStringValue(const Ogre::String &sName)
{
	std::map<Ogre::String,Ogre::String>::iterator it = m_mapStringVal.find(sName);
	return it != m_mapStringVal.end() ? it->second : "";
}

bool	CIntermediateObject::GetBoolValue(const Ogre::String &sName)
{
	std::map<Ogre::String,bool>::iterator it = m_mapBoolVal.find(sName);
	return it != m_mapBoolVal.end() ? it->second : false;
}

Ogre::Quaternion CIntermediateObject::GetQuatValue(const Ogre::String &sName)
{
	std::map<Ogre::String,Ogre::Quaternion>::iterator it = m_mapQuatVal.find(sName);
	return it != m_mapQuatVal.end() ? it->second : Ogre::Quaternion::IDENTITY;
}

Ogre::Vector3 CIntermediateObject::GetVec3Value(const Ogre::String &sName)
{
	std::map<Ogre::String,Ogre::Vector3>::iterator it = m_mapVec3Val.find(sName);
	return it != m_mapVec3Val.end() ? it->second : Ogre::Vector3::ZERO;
}

