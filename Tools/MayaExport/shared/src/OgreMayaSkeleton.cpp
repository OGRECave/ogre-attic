/*
============================================================================
This source file is part of the Ogre-Maya Tools.
Distributed as part of Ogre (Object-oriented Graphics Rendering Engine).
Copyright (C) 2003 Fifty1 Software Inc., Bytelords

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
or go to http://www.gnu.org/licenses/gpl.txt
============================================================================
*/
#include "OgreMayaSkeleton.h"
#include "OgreMayaOptions.h"

#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MAnimControl.h>

#include <maya/MFnMesh.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnSet.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnPhongShader.h>

#include <maya/MItGeometry.h>
#include <maya/MItDag.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>

#include <maya/MPlug.h>
#include <maya/MDagPathArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MPointArray.h>
#include <maya/MMatrix.h>
#include <maya/MGlobal.h>
#include <maya/MStatus.h>

#include <iostream>

namespace OgreMaya {
	
	using namespace std;

    void printMMatrix(MMatrix const& m) {
        cout.setf(ios::showpos | ios::fixed);
        cout.precision(5);
        cout << "("<<m(0,0)<<", "<<m(0,1)<<", "<<m(0,2)<<", "<<m(0,3)<<")" << '\n';
        cout << "("<<m(1,0)<<", "<<m(1,1)<<", "<<m(1,2)<<", "<<m(1,3)<<")" << '\n';
        cout << "("<<m(2,0)<<", "<<m(2,1)<<", "<<m(2,2)<<", "<<m(2,3)<<")" << '\n';
        cout << "("<<m(3,0)<<", "<<m(3,1)<<", "<<m(3,2)<<", "<<m(3,3)<<")" << '\n';
    }

    void printMQuaternion(MQuaternion const& q) {
        cout.setf(ios::showpos | ios::fixed);
        cout.precision(5);
        cout << "("<<q[0]<<", "<<q[1]<<", "<<q[2]<<", "<<q[3]<<")" << '\n';
    }

    void printMVector(MVector const& v) {
        cout.setf(ios::showpos | ios::fixed);
        cout.precision(5);
        cout << "("<<v[0]<<", "<<v[1]<<", "<<v[2]<<")" << '\n';
    }

	//	--------------------------------------------------------------------------
	/** Standard constructor. Creates Ogre Mesh and defines known options.
	*/	
	//	--------------------------------------------------------------------------
	SkeletonGenerator::SkeletonGenerator() {
	}


	//	--------------------------------------------------------------------------
	/** Destructor.
	*/	
	//	--------------------------------------------------------------------------
	SkeletonGenerator::~SkeletonGenerator()
	{
	}


	//	--------------------------------------------------------------------------
	/** Find and export all joints

		\return		True if exported ok, false otherwise
	*/	
	//	--------------------------------------------------------------------------
	bool SkeletonGenerator::exportAll()	{        

        if(!_querySkeleton())
            return false;

        if(!_querySkeletonAnim())
            return false;

        
        MGlobal::executeCommand("ikSystem -e -sol 0;");
        MGlobal::selectByName(root->name.c_str());
        MGlobal::executeCommand("dagPose -r -g -bp");
        MGlobal::executeCommand("dagPose -r -g -bp");
		//MGlobal::executeCommand("currentTime -edit 0");
        
        
/*        
        MGlobal::executeCommand("ikSystem -e -sol 0;");
	    MGlobal::executeCommand((string("select ")+root->name).c_str());
	    MGlobal::executeCommand("dagPose -r -g -bp");
*/
        
        /////////////////////////////////////////////

        
        {
            ofstream out(OPTIONS.outSkelFile.c_str());

            out.precision(5);
            out.setf(ios::fixed);

            out << "<skeleton>\n";

            
            //
            // BONES
            //
            out << "\t<bones>\n";


            SkeletonJointList::iterator it, end;
            MVector axis;
            double angle;

            for(it=jointList.begin(), end=jointList.end(); it!=end; ++it) {
                
                SkeletonJoint& j = **it;
                                
                /*
                cout << "* worldMatrix:\n";
                printMMatrix(j.worldMatrix);
                cout << "* relPos:\n";
                printMVector(j.relPos);
                cout << "* relRot:\n";
                printMQuaternion(j.relRot);
                cout << "----------------------------\n";
                */

                j.relRot.getAxisAngle(axis, angle);

                out << "\t\t<bone id=\""<<j.index<<"\" name=\""<<j.name<<"\">\n";
                out << "\t\t\t<position x=\""<<j.relPos.x<<"\" y=\""<<j.relPos.y<<"\" z=\""<<j.relPos.z<<"\"/>\n";
                out << "\t\t\t<rotation angle=\""<<((float)angle)<<"\">\n";
                out << "\t\t\t\t<axis x=\""<<axis.x<<"\" y=\""<<axis.y<<"\" z=\""<<axis.z<<"\"/>\n";
                out << "\t\t\t</rotation>\n";
                out << "\t\t</bone>\n";

            }

            out << "\t</bones>\n";

            
            //
            // HIERARCHY
            //
            out << "\t<bonehierarchy>\n";
            for(it=jointList.begin(), end=jointList.end(); it!=end; ++it) {
                SkeletonJoint& j = **it;
                if(j.hasParent) {
                    out << "\t\t<boneparent bone=\""<<j.name<<"\" parent=\""<<j.parentName<<"\"/>\n";
                }
            }

            out << "\t</bonehierarchy>\n";
            

            //
            // ANIMATIONS
            //
            out << "\t<animations>\n";
            AnimationMap::iterator animIt = animations.begin();
            AnimationMap::iterator animEnd = animations.end();
            for(; animIt!=animEnd; ++animIt) {
                string animName = (*animIt).first;
                Animation& anim = (*animIt).second;

                out << "\t\t<animation name=\""<<animName.c_str()<<"\" ";
                out << "length=\""<<anim.time<<"\">\n";
                out << "\t\t\t<tracks>\n";

                KeyframesMap::iterator keyframesIt = anim.keyframes.begin();
                KeyframesMap::iterator keyframesEnd = anim.keyframes.end();
                for(; keyframesIt!=keyframesEnd; ++keyframesIt) {
                    string boneName = (*keyframesIt).first;
                    KeyframeList& l = (*keyframesIt).second;

                    out << "\t\t\t\t<track bone=\""<<boneName.c_str()<<"\">\n";
                    out << "\t\t\t\t\t<keyframes>\n";

                    KeyframeList::iterator it  = l.begin();
                    KeyframeList::iterator end = l.end();
                    for(;it!=end; ++it) {
                        Keyframe& k = *it;

                        MVector axis;
                        double angle;
                        k.rot.getAxisAngle(axis, angle);

                        out << "\t\t\t\t\t\t<keyframe time=\""<<k.time<<"\">\n";                        
                        out << "\t\t\t\t\t\t\t<translate x=\""<<k.pos.x<<"\" y=\""<<k.pos.y<<"\" z=\""<<k.pos.z<<"\"/>\n";
                        out << "\t\t\t\t\t\t\t<rotate angle=\""<<((float)angle)<<"\">\n";
                        out << "\t\t\t\t\t\t\t\t<axis x=\""<<axis.x<<"\" y=\""<<axis.y<<"\" z=\""<<axis.z<<"\"/>\n";
                        out << "\t\t\t\t\t\t\t</rotate>\n";
                        out << "\t\t\t\t\t\t</keyframe>\n";
                    }

                    out << "\t\t\t\t\t</keyframes>\n";
                    out << "\t\t\t\t</track>\n";
                }                    

                out << "\t\t\t</tracks>\n";
                out << "\t\t</animation>\n";
            }
            out << "\t</animations>\n";


            out << "</skeleton>\n";
        }


        deleteAll(jointList.begin(), jointList.end());

        return true;
    }
        
    //	--------------------------------------------------------------------------
	/** Finds and exports all joints

		\return		True if exported ok, false otherwise
	*/	
	//	--------------------------------------------------------------------------

	bool SkeletonGenerator::_querySkeleton() {

        cout << "\nSkeletonGenerator::_querySkeleton\n";
		
        jointList.clear();


	    MItDag    kDagIt(MItDag::kDepthFirst, MFn::kJoint);
	    MDagPath  kRootPath;

	    MStatus   kStatus;

        kDagIt.getPath(kRootPath);

		// check if valid path
		if(!kRootPath.isValid()) {
			cout << "\tcan not find parent joint\n"; 
            return false;
		}
        else {
            cout << "\tfound parent joint \""<<kRootPath.partialPathName().asChar()<<"\"\n"; 
        }

        /*
        // is this really necessary?
	    // check for skeleton root joint
	    {
		    MFnIkJoint    kJointFn( kRootPath.node() );

		    MObject kParentObj = kJointFn.parent(0);

		    // root joint can not have parent joint
            if(!kParentObj.hasFn( MFn::kJoint)) {
			    cout << "\tParent joint found: \"" << kJointFn.partialPathName().asChar() << "\"\n";
		    }
		    else {
                MFnDagNode kDagNodeFn(kParentObj);
			    cout << "\troot joint can not have joint as parent, PATH:\""<<kDagNodeFn.partialPathName().asChar()<<"\"\n";
			    return 0;
		    }
	    }
        */

	    //Setup skeleton
        cout << "\tsetup skeleton\n";
	    int uiNumJoints = 0;

	    for( ; !kDagIt.isDone(); kDagIt.next(), ++uiNumJoints ) {
		    MDagPath kDagPath;

		    kDagIt.getPath( kDagPath );
		    MFnIkJoint kJointFn( kDagPath.node() );

		    SkeletonJoint *pkJoint = new SkeletonJoint;

		    jointList.push_back( pkJoint );

            pkJoint->dagPath = kDagPath;
		    pkJoint->name    = kJointFn.partialPathName().asChar();
		    pkJoint->index   = uiNumJoints;		    

		    unsigned int uiNumParents = kJointFn.parentCount();

		    // can only have one parent
		    if( uiNumParents != 1 ) {
			    cout << "\t[ERROR] joint has " << uiNumParents << " parents (only 1 allowed)" << '\n';
			    return 0;
		    }

		    MObject kParentObj = kJointFn.parent(0);
			    
		    if(kParentObj.hasFn(MFn::kJoint)) {
			    MFnIkJoint kParentJointFn(kParentObj); 

			    pkJoint->parentName = kParentJointFn.partialPathName().asChar();
			    pkJoint->hasParent  = true;			    
		    }
		    else {
                // we've found root here -> mark
                root = pkJoint;

                pkJoint->parentName = "";
				pkJoint->hasParent  = false;                
		    }


		    //Get bindpose world matrix for joint

		    MPlug   kBindMatrixPlug = kJointFn.findPlug("bindPose");
		    MObject kBindMatrixObject;
	    
		    kStatus = kBindMatrixPlug.getValue(kBindMatrixObject);

		    if( kStatus != MStatus::kSuccess ) {
			    cout << "\t[ERROR] unable to get bind matrix plug object\n";
			    return 0;
		    } 

		    MFnMatrixData kMatrixDataFn( kBindMatrixObject );
		    
		    MMatrix kBindMatrix = kMatrixDataFn.matrix( &kStatus );

		    if( kStatus != MStatus::kSuccess ) {
			    cout << "\t[ERROR] unable to get bind matrix data from plug object\n";
			    return 0;
		    }

            pkJoint->worldMatrix    = kBindMatrix;
            pkJoint->invWorldMatrix = kBindMatrix.inverse();
	    }


        // if  numJoints == 0, we only have single root bone in skeleton
        if(!uiNumJoints) {
		    return true;            
        }        


	    //Calculate relative position and rotation data
        cout << "\tcalculate relative position and rotation data\n";
	    
		SkeletonJointList::iterator jointIt  = jointList.begin();
		SkeletonJointList::iterator jointEnd = jointList.end();
		  
        for(;jointIt!=jointEnd; ++jointIt) {

            SkeletonJoint* j = *jointIt;

            // search for parent node
            if(j->hasParent) {
                SkeletonJointList::iterator parentJointIt  = jointList.begin();
			    SkeletonJointList::iterator parentJointEnd = jointList.end();
			    for( ; parentJointIt != parentJointEnd; ++parentJointIt )
                    if( (*parentJointIt)->name == (*jointIt)->parentName ) {
                        (*jointIt)->parent = *parentJointIt;
				        break;
                    }
            }

            if(j->hasParent)
                j->localMatrix = j->worldMatrix * j->parent->invWorldMatrix;
            else
                j->localMatrix = j->worldMatrix;

            j->invLocalMatrix = j->localMatrix.inverse();

            j->relPos.x = j->localMatrix(3,0);
            j->relPos.y = j->localMatrix(3,1);
            j->relPos.z = j->localMatrix(3,2);
            
            j->relRot = j->localMatrix;        
        }

		
		// ===== Done
		return true;
	}


    bool SkeletonGenerator::_querySkeletonAnim() {

        cout << "\nSkeletonGenerator::_querySkeletonAnim\n";

        animations.clear();

	    MTime kTimeMin   = MAnimControl::minTime();
	    MTime kTimeMax   = MAnimControl::maxTime();
	    MTime kTimeTotal = kTimeMax - kTimeMin;
	    float fLength    = (float)kTimeTotal.as(MTime::kSeconds);
	    int iTimeMin     = (int)kTimeMin.value();
	    int iTimeMax     = (int)kTimeMax.value();
	    int iFrames      = (iTimeMax-iTimeMin)+1;
        float secondsPerFrame = fLength / (float)iFrames;
	    
	    MAnimControl kAnimControl;

	    cout << "\tanimation start: " << iTimeMin << " end: " << iTimeMax << '\n';

	    if( iFrames <= 1 )
		    return false;
	    

        Options::KeyframeRangeMap& m = OPTIONS.animations;
        Options::KeyframeRangeMap::iterator it  = m.begin();
        Options::KeyframeRangeMap::iterator end = m.end();

        for(;it!=end; ++it) {
            string animationName = (*it).first;
            int from    = (*it).second.from;
            int to      = (*it).second.to;
            int step    = (*it).second.step;
            int frameCount = to - from + 1;
            
            if(from < iTimeMin || to > iTimeMax || !(frameCount>0)) {
                cout << "\t[ERROR] Illegal Animation Range\n";
                continue;
            }

            Animation& anim = animations[animationName];

            anim.time = (float)(frameCount)*secondsPerFrame;            

            SkeletonJointList::iterator ppkJoint = jointList.begin();
            SkeletonJointList::iterator ppkJointEnd = jointList.end();		    
                
			for( ; ppkJoint != ppkJointEnd; ++ppkJoint ) {				
                MTime kFrame = kTimeMin + (from - 1);

	            for(int iFrame=0; iFrame<frameCount; iFrame+=step, kFrame+=step) {
		            kAnimControl.setCurrentTime( kFrame );

                    MVector kTranslation;
			        MQuaternion kRotation;
					
					MMatrix kIncMat    = (*ppkJoint)->dagPath.inclusiveMatrix();
					MMatrix kExcMat    = (*ppkJoint)->dagPath.exclusiveMatrix();
					MMatrix kExcInvMat = (*ppkJoint)->dagPath.exclusiveMatrixInverse();                    			                            										
					
					if((*ppkJoint)->hasParent) {
						MMatrix kLocalMat = kIncMat * kExcInvMat * (*ppkJoint)->invLocalMatrix;

						kRotation = kLocalMat;

						kTranslation.x = (float)kLocalMat(3, 0);
						kTranslation.y = (float)kLocalMat(3, 1);
						kTranslation.z = (float)kLocalMat(3, 2);
					}
					else {
						// root has to be handled differently
						// cause when exporting root bone to ogre
						// we remove all maya parents
						kRotation = kIncMat * (*ppkJoint)->invLocalMatrix;

						kTranslation.x = (float)(kIncMat(3, 0) - kExcMat(3, 0));
						kTranslation.y = (float)(kIncMat(3, 1) - kExcMat(3, 1));
						kTranslation.z = (float)(kIncMat(3, 2) - kExcMat(3, 2));
					}

                    float timePos = 
                        (float)iFrame * secondsPerFrame;
                
                    anim.keyframes[(*ppkJoint)->name].push_back(
                        Keyframe(timePos, kTranslation, kRotation)
                    );
		        }   
            }
        }

	    return true;
    }

} // namespace OgreMaya
