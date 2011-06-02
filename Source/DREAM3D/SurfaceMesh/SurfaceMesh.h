/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Grober (US Air Force Research Laboratories
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef SURFACEMESH_H_
#define SURFACEMESH_H_

#if defined (_MSC_VER)
#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers
#endif


#include <MXA/Common/MXASetGetMacros.h>
#include <MXA/MXATypes.h>
#include "DREAM3D/Common/Constants.h"
#include <DREAM3D/SurfaceMesh/SurfaceMeshFunc.h>
#include "DREAM3D/Common/AbstractPipeline.h"


/**
* @class SurfaceMesh SurfaceMesh.h AIM/SurfaceMesh/SurfaceMesh.h
* @brief This class serves as the main entry point to execute the Surface Mesh
* codes.
* @author Michael A. Jackson for BlueQuartz Software,
* @author Dr. Michael Groeber, USAFRL
* @date Nov 3, 2009
* @version 1.0
*/
class DREAM3DLib_EXPORT SurfaceMesh : public AbstractPipeline
{
  public:
    MXA_SHARED_POINTERS(SurfaceMesh);
    MXA_TYPE_MACRO(SurfaceMesh);
    MXA_STATIC_NEW_MACRO(SurfaceMesh);

    virtual ~SurfaceMesh();

    MXA_INSTANCE_STRING_PROPERTY(InputDirectory)
    MXA_INSTANCE_STRING_PROPERTY(InputFile)
    MXA_INSTANCE_STRING_PROPERTY(ScalarName)

    MXA_INSTANCE_PROPERTY(int, XDim)
    MXA_INSTANCE_PROPERTY(int, YDim)
    MXA_INSTANCE_PROPERTY(int, ZDim)
    MXA_INSTANCE_PROPERTY(double, XRes)
    MXA_INSTANCE_PROPERTY(double, YRes)
    MXA_INSTANCE_PROPERTY(double, ZRes)

    MXA_INSTANCE_STRING_PROPERTY(OutputDirectory)
    MXA_INSTANCE_STRING_PROPERTY(OutputFilePrefix)
    MXA_INSTANCE_PROPERTY(bool, ConformalMesh)
    MXA_INSTANCE_PROPERTY(bool, BinaryVTKFile)
    MXA_INSTANCE_PROPERTY(bool, WriteSTLFile)

    MXA_INSTANCE_PROPERTY(bool, DeleteTempFiles)

    MXA_INSTANCE_PROPERTY(bool, SmoothMesh)
    MXA_INSTANCE_PROPERTY(int, SmoothIterations)
    MXA_INSTANCE_PROPERTY(int, SmoothFileOutputIncrement)
    MXA_INSTANCE_PROPERTY(bool, SmoothLockQuadPoints)


    /**
     * @brief Main method to run the operation
     */
    void execute();

  protected:

    SurfaceMesh();

  private:
	  SurfaceMeshFunc::Pointer m;

    SurfaceMesh(const SurfaceMesh&);    // Copy Constructor Not Implemented
    void operator=(const SurfaceMesh&);  // Operator '=' Not Implemented
};


#endif /* SURFACEMESH_H_ */
