/*
===============================================================================
This source file is part of the Ogre-Maya Tools.
Distributed as part of Ogre (Object-oriented Graphics Rendering Engine).
Copyright © 2003 Fifty1 Software Inc. 

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
===============================================================================
*/

#include "OgreMayaSkeleton.h"
#include "OgreMayaOptions.h"

#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MDagPath.h>

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

#include <iostream.h>

namespace OgreMaya {


    void printMMatrix(MMatrix const& m) {
        cout.setf(ios::showpos | ios::fixed);
        cout.precision(5);
        cout << "("<<m(0,0)<<", "<<m(0,1)<<", "<<m(0,2)<<", "<<m(0,3)<<")" << endl;
        cout << "("<<m(1,0)<<", "<<m(1,1)<<", "<<m(1,2)<<", "<<m(1,3)<<")" << endl;
        cout << "("<<m(2,0)<<", "<<m(2,1)<<", "<<m(2,2)<<", "<<m(2,3)<<")" << endl;
        cout << "("<<m(3,0)<<", "<<m(3,1)<<", "<<m(3,2)<<", "<<m(3,3)<<")" << endl;
    }

    void printMQuaternion(MQuaternion const& q) {
        cout.setf(ios::showpos | ios::fixed);
        cout.precision(5);
        cout << "("<<q[0]<<", "<<q[1]<<", "<<q[2]<<", "<<q[3]<<")" << endl;
    }

    void printMVector(MVector const& v) {
        cout.setf(ios::showpos | ios::fixed);
        cout.precision(5);
        cout << "("<<v[0]<<", "<<v[1]<<", "<<v[2]<<")" << endl;
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
	/** Finds and exports all joints

		\return		True if exported ok, false otherwise
	*/	
	//	--------------------------------------------------------------------------
	bool SkeletonGenerator::exportAll()	{
        bool ok;

        ok = _querySkeleton();
        if(!ok)
            return false;

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

                // OUT: BEGIN
                cout << "*** " << j.name << " [" << j.parentName << "]" << endl;
                cout << "* worldMatrix:" << endl;
                printMMatrix(j.worldMatrix);
                cout << "* worldQuaternion:" << endl;
                printMQuaternion(j.worldQuat);
                cout << "* relPos:" << endl;
                printMVector(j.relPos);
                cout << "* relRot:" << endl;
                printMQuaternion(j.relRot);
                cout << "----------------------------" << endl;
                // OUT: END

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
            // TODO
            out << "\t</animations>\n";


            out << "</skeleton>\n";
        }


        return true;
    }
        
    //	--------------------------------------------------------------------------
	/** Finds and exports all joints

		\return		True if exported ok, false otherwise
	*/	
	//	--------------------------------------------------------------------------

	bool SkeletonGenerator::_querySkeleton()	{
		
        jointList.clear();


	    MItDag    kDagIt( MItDag::kDepthFirst, MFn::kJoint );
	    MDagPath  kRootPath;

	    MStatus   kStatus;

	    kDagIt.getPath( kRootPath );

	    //Get the parent transform for the skeleton
	    {
		    MFnIkJoint    kJointFn( kRootPath.node() );
		    unsigned int  uiNumParents = kJointFn.parentCount();

		    if( uiNumParents != 1 )
			    return 0;

		    MObject kParentObj = kJointFn.parent(0);

		    // is the parent a transform node?
		    //if( kParentObj.hasFn( MFn::kTransform ) ) {
            if( !kParentObj.hasFn( MFn::kJoint ) ) {                            
                /*
			    MFnTransform kTransFn( kParentObj );
			    MTransformationMatrix kTransMatrix = kTransFn.transformation();
                */

			    cout << "\tParent object found: \"" << kJointFn.partialPathName().asChar() << "\"\n";
		    }
		    else {
                MFnDagNode kDagNodeFn(kParentObj );
			    cout << "\tthe parent isn't a root transform, PATH:\""<<kDagNodeFn.partialPathName().asChar()<<"\"\n";
			    return 0;
		    }
	    }	

	    //Setup skeleton
        cout << "\tSetup skeleton\n";
	    int uiNumJoints = 0;

	    for( ; !kDagIt.isDone(); kDagIt.next(), ++uiNumJoints ) {
		    MDagPath kDagPath;

		    kDagIt.getPath( kDagPath );
		    MFnIkJoint kJointFn( kDagPath.node() );

		    SkeletonJoint *pkJoint = new SkeletonJoint;

		    jointList.push_back( pkJoint );

		    pkJoint->name = kJointFn.partialPathName().asChar();
		    pkJoint->index  = uiNumJoints;

		    cout << "  joint " << uiNumJoints << ": " << pkJoint->name << endl;

		    unsigned int uiNumParents = kJointFn.parentCount();

		    //Only have one parent! (either a joint or the root transform)
		    if( uiNumParents != 1 ) {
			    cout << "ERROR: joint has " << uiNumParents << " parents (only 1 allowed)" << endl;
			    return 0;
		    }

		    MObject kParentObj = kJointFn.parent(0);
			    
		    if( kParentObj.hasFn( MFn::kJoint ) ) {
			    MFnIkJoint kParentJointFn( kParentObj ); 

			    pkJoint->parentName = kParentJointFn.partialPathName().asChar();
			    pkJoint->hasParent  = true;

			    cout << "    parent: " << pkJoint->parentName << "" << endl;
		    }
		    else {
                
                pkJoint->parentName = "";
				pkJoint->hasParent  = false;
                
                /*
			    //Should be the root transform node
			    if( kParentObj.hasFn( MFn::kTransform ) )
			    {
				    MFnDagNode kDagNode( kParentObj );

				    cout << "    parent is root transform: " << kDagNode.name().asChar() << endl;
		    
				    pkJoint->parentName = "";
				    pkJoint->hasParent    = false;
			    }
			    else
			    {
				    //What is parent?
				    MFnDependencyNode kDepNode( kParentObj );
				    cout << "ERROR: parent is NOT root transform: " << kDepNode.name().asChar() << " : " << kDepNode.typeName().asChar() << endl;
				    return 0;
			    }
                */
		    }


		    //Get bindpose world matrix for joint

		    MPlug   kBindMatrixPlug = kJointFn.findPlug( "bindPose" );
		    MObject kBindMatrixObject;
	    
		    kStatus = kBindMatrixPlug.getValue( kBindMatrixObject );

		    if( kStatus != MStatus::kSuccess ) {
			    cout << "ERROR: Unable to get bind matrix plug object" << endl;
			    return 0;
		    } 

		    MFnMatrixData kMatrixDataFn( kBindMatrixObject );
		    
		    MMatrix kBindMatrix = kMatrixDataFn.matrix( &kStatus );

		    if( kStatus != MStatus::kSuccess ) {
			    cout << "ERROR: Unable to get bind matrix data from plug object" << endl;
			    return 0;
		    }

            /*
            // CHECK
            for( int jj = 0; jj < 4; ++jj ) {
			    for( int ii = 0; ii < 4; ++ii ) {
				    if( kBindMatrix[jj][ii] > 0.0001 )
					    pkJoint->worldMatrix[jj][ii] = kBindMatrix[jj][ii];
				    else
					    pkJoint->worldMatrix[jj][ii] = 0.0f;
			    }
            }
            */
            pkJoint->worldMatrix = kBindMatrix;



		    // CHECK: pkJoint->worldQuat.FromMatrix( pkJoint->worldMatrix );
            pkJoint->worldQuat = pkJoint->worldMatrix;

            

		    pkJoint->invWorldQuat = pkJoint->worldQuat.inverse();





            /*
            // TEST:
            pkJoint->relRot  = pkJoint->worldQuat;
			pkJoint->relPos  = MVector(
                pkJoint->worldMatrix(3,0),
                pkJoint->worldMatrix(3,1),
                pkJoint->worldMatrix(3,2)
            );
            */
	    }


	    if( !uiNumJoints )
		    return true;


	    //Calculate relative position and rotation data
        cout << "\tCalculate relative position and rotation data\n";
	    {
		    SkeletonJointList::iterator ppkJoint    = jointList.begin();
		    SkeletonJointList::iterator ppkJointEnd = jointList.end();

		    for( ; ppkJoint != ppkJointEnd; ++ppkJoint ) {
			    if( (*ppkJoint)->hasParent ) {
				    SkeletonJointList::iterator ppkParent    = jointList.begin();
				    SkeletonJointList::iterator ppkParentEnd = jointList.end();

				    for( ; ppkParent != ppkParentEnd; ++ppkParent )
					    if( (*ppkParent)->name == (*ppkJoint)->parentName )
						    break;

				    if( ppkParent != ppkParentEnd ) {
					    (*ppkJoint)->parent = *ppkParent;

					    //Calculate
                        /*
					    (*ppkJoint)->relRot  = (*ppkParent)->invWorldQuat * (*ppkJoint)->worldQuat;                        
					    (*ppkJoint)->relPos  = (*ppkParent)->invWorldQuat * ( 
                            MVector(
                                (*ppkJoint)->worldMatrix(3,0) - (*ppkParent)->worldMatrix(3,0),
                                (*ppkJoint)->worldMatrix(3,1) - (*ppkParent)->worldMatrix(3,1),
                                (*ppkJoint)->worldMatrix(3,2) - (*ppkParent)->worldMatrix(3,2)
                            )
                        );
                        */

                        (*ppkJoint)->relRot  = MQuaternion();
					    (*ppkJoint)->relPos  = (
                            MVector(
                                (*ppkJoint)->worldMatrix(3,0) - (*ppkParent)->worldMatrix(3,0),
                                (*ppkJoint)->worldMatrix(3,1) - (*ppkParent)->worldMatrix(3,1),
                                (*ppkJoint)->worldMatrix(3,2) - (*ppkParent)->worldMatrix(3,2)
                            )
                        );

				    }
				    else {
					    cout << "WARNING: for bone [" << (*ppkJoint)->name << "], unable to locate parent bone [" << (*ppkJoint)->parentName << "]" << endl;

					    (*ppkJoint)->parent = 0;

					    //Store
					    (*ppkJoint)->relRot  = (*ppkJoint)->worldQuat;
					    (*ppkJoint)->relPos  = MVector(
                            (*ppkJoint)->worldMatrix(3,0),
                            (*ppkJoint)->worldMatrix(3,1),
                            (*ppkJoint)->worldMatrix(3,2)
                        );
				    }
			    }
			    else {
				    (*ppkJoint)->parent = 0;

				    //Store
				    (*ppkJoint)->relRot  = (*ppkJoint)->worldQuat;
				    (*ppkJoint)->relPos  =
                        MVector(
                            (*ppkJoint)->worldMatrix(3,0),
                            (*ppkJoint)->worldMatrix(3,1),
                            (*ppkJoint)->worldMatrix(3,2)
                        );
			    }
		    }
	    }	 


		
		// ===== Done
		return true;
	}

	

} // namespace OgreMaya
