#include "lwo2mesh.h"
#include "ogre.h"

#pragma auto_inline( off ) 
LogManager *pLogMgr;
MaterialManager *pMatMgr;
SkeletonManager *pSkelMgr;
void Lwo2MeshWriter::createSingletons() 
{ 
   pLogMgr = new LogManager(); 
   pMatMgr = new MaterialManager(); 
   pSkelMgr = new SkeletonManager(); 
} 
#pragma auto_inline( on ) 

void Lwo2MeshWriter::destroySingletons() 
{ 
   delete pSkelMgr; 
   delete pMatMgr; 
   delete pLogMgr; 
}