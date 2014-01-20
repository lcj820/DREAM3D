/* ============================================================================
 * Copyright (c) 2011 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2011 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
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
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "QuickSurfaceMesh.h"

#include <sstream>

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/DataArrays/IDataArray.h"

#define QSM_GETCOORD(index, res, coord, origin)\
  coord = float((float(index)*float(res)) + float(origin));\

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QuickSurfaceMesh::QuickSurfaceMesh() :
  AbstractFilter(),
  m_DataContainerName(DREAM3D::Defaults::VolumeDataContainerName),
  m_SurfaceDataContainerName(DREAM3D::Defaults::SurfaceDataContainerName),
  m_CellAttributeMatrixName(DREAM3D::Defaults::CellAttributeMatrixName),
  m_VertexAttributeMatrixName(DREAM3D::Defaults::VertexAttributeMatrixName),
  m_FaceAttributeMatrixName(DREAM3D::Defaults::FaceAttributeMatrixName),
  m_TransferPhaseId(false),
  m_FeatureIdsArrayName(DREAM3D::CellData::FeatureIds),
  m_FeatureIds(NULL),
  m_CellPhasesArrayName(DREAM3D::CellData::Phases),
  m_CellPhases(NULL)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QuickSurfaceMesh::~QuickSurfaceMesh()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QuickSurfaceMesh::setupFilterParameters()
{
  QVector<FilterParameter::Pointer> parameters;
  {
    FilterParameter::Pointer option = FilterParameter::New();
    option->setHumanLabel("Transfer Phase Id");
    option->setPropertyName("TransferPhaseId");
    option->setWidgetType(FilterParameterWidgetType::BooleanWidget);
    option->setValueType("bool");
    parameters.push_back(option);
  }
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QuickSurfaceMesh::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  /* Code to read the values goes between these statements */
  /* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE BEGIN*/
  setTransferPhaseId(reader->readValue("TransferPhaseId", getTransferPhaseId()));
  /* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE END*/
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int QuickSurfaceMesh::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  writer->writeValue("TransferPhaseId", getTransferPhaseId() );
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QuickSurfaceMesh::dataCheck()
{
  setErrorCondition(0);

  VolumeDataContainer* m = getDataContainerArray()->getPrereqDataContainer<VolumeDataContainer, AbstractFilter>(this, getDataContainerName(), false);
  if(getErrorCondition() < 0) { return; }
  AttributeMatrix::Pointer cellAttrMat = m->getPrereqAttributeMatrix<AbstractFilter>(this, getCellAttributeMatrixName(), -301);
  if(getErrorCondition() < 0) { return; }

  QVector<size_t> dims(1, 1);
  m_FeatureIdsPtr = cellAttrMat->getPrereqArray<DataArray<int32_t>, AbstractFilter>(this, m_FeatureIdsArrayName, -300, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_FeatureIdsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  if(m_TransferPhaseId == true)
  {
    m_CellPhasesPtr = cellAttrMat->getPrereqArray<DataArray<int32_t>, AbstractFilter>(this, m_CellPhasesArrayName, -301, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
    if( NULL != m_CellPhasesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
    { m_CellPhases = m_CellPhasesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  }

  //// Create a SufaceMesh Data Container with Faces, Vertices, Feature Labels and optionally Phase labels
  SurfaceDataContainer* sm = getDataContainerArray()->createNonPrereqDataContainer<SurfaceDataContainer, AbstractFilter>(this, getDataContainerName());
  if(getErrorCondition() < 0) { return; }
//  QVector<size_t> tDims(1, 0);
//  AttributeMatrix* vertexAttrMat = m->createNonPrereqAttributeMatrix<AbstractFilter>(this, getVertexAttributeMatrixName(), tDims, DREAM3D::AttributeMatrixType::Vertex);
//  if(getErrorCondition() < 0) { return; }
//  AttributeMatrix* faceAttrMat = m->createNonPrereqAttributeMatrix<AbstractFilter>(this, getFaceAttributeMatrixName(), tDims, DREAM3D::AttributeMatrixType::Face);
//  if(getErrorCondition() < 0) { return; }

  VertexArray::Pointer vertices = VertexArray::CreateArray(1, DREAM3D::VertexData::SurfaceMeshNodes);
  FaceArray::Pointer triangles = FaceArray::CreateArray(1, DREAM3D::FaceData::SurfaceMeshFaces, vertices.get());
  dims[0] = 2;
  DataArray<int32_t>::Pointer faceLabelPtr = DataArray<int32_t>::CreateArray(1, dims, DREAM3D::FaceData::SurfaceMeshFaceLabels);
  DataArray<int8_t>::Pointer nodeTypePtr = DataArray<int8_t>::CreateArray(1, DREAM3D::VertexData::SurfaceMeshNodeType);

  sm->setVertices(vertices);
  sm->setFaces(triangles);
  sm->getAttributeMatrix(getFaceAttributeMatrixName())->addAttributeArray(faceLabelPtr->getName(), faceLabelPtr);
  sm->getAttributeMatrix(getVertexAttributeMatrixName())->addAttributeArray(nodeTypePtr->getName(), nodeTypePtr);
  if(m_TransferPhaseId == true)
  {
    DataArray<int32_t>::Pointer phaseLabelPtr = DataArray<int32_t>::CreateArray(1, dims, DREAM3D::FaceData::SurfaceMeshPhaseLabels);
    sm->getAttributeMatrix(getFaceAttributeMatrixName())->addAttributeArray(phaseLabelPtr->getName(), phaseLabelPtr);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QuickSurfaceMesh::preflight()
{
  dataCheck();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QuickSurfaceMesh::execute()
{
  setErrorCondition(0);
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  VolumeDataContainer* m = getDataContainerArray()->getDataContainerAs<VolumeDataContainer>(getDataContainerName());
  SurfaceDataContainer* sm = getDataContainerArray()->getDataContainerAs<SurfaceDataContainer>(getSurfaceDataContainerName());
  //int64_t totalPoints = m->getAttributeMatrix(getCellAttributeMatrixName())->getNumTuples();

  float m_OriginX, m_OriginY, m_OriginZ;
  m->getOrigin(m_OriginX, m_OriginY, m_OriginZ);

  size_t udims[3] = {0, 0, 0};
  m->getDimensions(udims);
#if (CMP_SIZEOF_SIZE_T == 4)
  typedef int32_t DimType;
#else
  typedef int64_t DimType;
#endif
  DimType dims[3] =
  {
    static_cast<DimType>(udims[0]),
    static_cast<DimType>(udims[1]),
    static_cast<DimType>(udims[2]),
  };

  size_t xP = dims[0];
  size_t yP = dims[1];
  size_t zP = dims[2];
  float xRes = m->getXRes();
  float yRes = m->getYRes();
  float zRes = m->getZRes();

  QVector<QSet<int> > ownerLists;

  size_t possibleNumNodes = (xP + 1) * (yP + 1) * (zP + 1);
  QVector<int> m_NodeIds(possibleNumNodes, -1);

  int nodeCount = 0;
  int triangleCount = 0;

  size_t point, neigh1, neigh2, neigh3;

  size_t nodeId1, nodeId2, nodeId3, nodeId4;

  //first determining which nodes are actually boundary nodes and
  //count number of nodes and triangles that will be created
  for(size_t k = 0; k < zP; k++)
  {
    for(size_t j = 0; j < yP; j++)
    {
      for(size_t i = 0; i < xP; i++)
      {
        point = (k * xP * yP) + (j * xP) + i;
        neigh1 = point + 1;
        neigh2 = point + xP;
        neigh3 = point + (xP * yP);

        if(i == 0)
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          if(m_NodeIds[nodeId1] == -1)
          {
            m_NodeIds[nodeId1] = nodeCount;
            nodeCount++;
          }
          nodeId2 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          if(m_NodeIds[nodeId2] == -1)
          {
            m_NodeIds[nodeId2] = nodeCount;
            nodeCount++;
          }
          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          if(m_NodeIds[nodeId3] == -1)
          {
            m_NodeIds[nodeId3] = nodeCount;
            nodeCount++;
          }
          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          if(m_NodeIds[nodeId4] == -1)
          {
            m_NodeIds[nodeId4] = nodeCount;
            nodeCount++;
          }
          triangleCount++;
          triangleCount++;
        }
        if(j == 0)
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          if(m_NodeIds[nodeId1] == -1)
          {
            m_NodeIds[nodeId1] = nodeCount;
            nodeCount++;
          }
          nodeId2 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId2] == -1)
          {
            m_NodeIds[nodeId2] = nodeCount;
            nodeCount++;
          }
          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          if(m_NodeIds[nodeId3] == -1)
          {
            m_NodeIds[nodeId3] = nodeCount;
            nodeCount++;
          }
          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId4] == -1)
          {
            m_NodeIds[nodeId4] = nodeCount;
            nodeCount++;
          }
          triangleCount++;
          triangleCount++;
        }
        if(k == 0)
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          if(m_NodeIds[nodeId1] == -1)
          {
            m_NodeIds[nodeId1] = nodeCount;
            nodeCount++;
          }
          nodeId2 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId2] == -1)
          {
            m_NodeIds[nodeId2] = nodeCount;
            nodeCount++;
          }
          nodeId3 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          if(m_NodeIds[nodeId3] == -1)
          {
            m_NodeIds[nodeId3] = nodeCount;
            nodeCount++;
          }
          nodeId4 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId4] == -1)
          {
            m_NodeIds[nodeId4] = nodeCount;
            nodeCount++;
          }
          triangleCount++;
          triangleCount++;
        }
        if(i == (xP - 1))
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId1] == -1)
          {
            m_NodeIds[nodeId1] = nodeCount;
            nodeCount++;
          }
          nodeId2 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId2] == -1)
          {
            m_NodeIds[nodeId2] = nodeCount;
            nodeCount++;
          }
          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId3] == -1)
          {
            m_NodeIds[nodeId3] = nodeCount;
            nodeCount++;
          }
          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId4] == -1)
          {
            m_NodeIds[nodeId4] = nodeCount;
            nodeCount++;
          }
          triangleCount++;
          triangleCount++;
        }
        else if(m_FeatureIds[point] != m_FeatureIds[neigh1])
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId1] == -1)
          {
            m_NodeIds[nodeId1] = nodeCount;
            nodeCount++;
          }
          nodeId2 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId2] == -1)
          {
            m_NodeIds[nodeId2] = nodeCount;
            nodeCount++;
          }
          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId3] == -1)
          {
            m_NodeIds[nodeId3] = nodeCount;
            nodeCount++;
          }
          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId4] == -1)
          {
            m_NodeIds[nodeId4] = nodeCount;
            nodeCount++;
          }
          triangleCount++;
          triangleCount++;
        }
        if(j == (yP - 1))
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId1] == -1)
          {
            m_NodeIds[nodeId1] = nodeCount;
            nodeCount++;
          }
          nodeId2 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          if(m_NodeIds[nodeId2] == -1)
          {
            m_NodeIds[nodeId2] = nodeCount;
            nodeCount++;
          }
          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId3] == -1)
          {
            m_NodeIds[nodeId3] = nodeCount;
            nodeCount++;
          }
          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          if(m_NodeIds[nodeId4] == -1)
          {
            m_NodeIds[nodeId4] = nodeCount;
            nodeCount++;
          }
          triangleCount++;
          triangleCount++;
        }
        else if(m_FeatureIds[point] != m_FeatureIds[neigh2])
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId1] == -1)
          {
            m_NodeIds[nodeId1] = nodeCount;
            nodeCount++;
          }
          nodeId2 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          if(m_NodeIds[nodeId2] == -1)
          {
            m_NodeIds[nodeId2] = nodeCount;
            nodeCount++;
          }
          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId3] == -1)
          {
            m_NodeIds[nodeId3] = nodeCount;
            nodeCount++;
          }
          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          if(m_NodeIds[nodeId4] == -1)
          {
            m_NodeIds[nodeId4] = nodeCount;
            nodeCount++;
          }
          triangleCount++;
          triangleCount++;
        }
        if(k == (zP - 1))
        {
          nodeId1 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId1] == -1)
          {
            m_NodeIds[nodeId1] = nodeCount;
            nodeCount++;
          }
          nodeId2 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          if(m_NodeIds[nodeId2] == -1)
          {
            m_NodeIds[nodeId2] = nodeCount;
            nodeCount++;
          }
          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId3] == -1)
          {
            m_NodeIds[nodeId3] = nodeCount;
            nodeCount++;
          }
          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          if(m_NodeIds[nodeId4] == -1)
          {
            m_NodeIds[nodeId4] = nodeCount;
            nodeCount++;
          }
          triangleCount++;
          triangleCount++;
        }
        else if(k < zP - 1 && m_FeatureIds[point] != m_FeatureIds[neigh3])
        {
          nodeId1 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId1] == -1)
          {
            m_NodeIds[nodeId1] = nodeCount;
            nodeCount++;
          }
          nodeId2 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          if(m_NodeIds[nodeId2] == -1)
          {
            m_NodeIds[nodeId2] = nodeCount;
            nodeCount++;
          }
          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          if(m_NodeIds[nodeId3] == -1)
          {
            m_NodeIds[nodeId3] = nodeCount;
            nodeCount++;
          }
          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          if(m_NodeIds[nodeId4] == -1)
          {
            m_NodeIds[nodeId4] = nodeCount;
            nodeCount++;
          }
          triangleCount++;
          triangleCount++;
        }
      }
    }
  }

  //now create node and triangle arrays knowing the number that will be needed
  VertexArray::Pointer vertices = VertexArray::CreateArray(nodeCount, DREAM3D::VertexData::SurfaceMeshNodes);
  FaceArray::Pointer triangles = FaceArray::CreateArray(triangleCount, DREAM3D::FaceData::SurfaceMeshFaces, vertices.get());
  QVector<size_t> dim(1, 2);
  DataArray<int32_t>::Pointer faceLabelPtr = DataArray<int32_t>::CreateArray(triangleCount, dim, DREAM3D::FaceData::SurfaceMeshFaceLabels);
  DataArray<int8_t>::Pointer nodeTypePtr = DataArray<int8_t>::CreateArray(nodeCount, DREAM3D::VertexData::SurfaceMeshNodeType);
  VertexArray::Vert_t* vertex = vertices.get()->getPointer(0);
  FaceArray::Face_t* triangle = triangles.get()->getPointer(0);
  int32_t* faceLabels = faceLabelPtr->getPointer(0);
  int8_t* nodeTypes = nodeTypePtr->getPointer(0);

  int32_t* phaseLabel = NULL;
  DataArray<int32_t>::Pointer phaseLabelPtr = Int32ArrayType::NullPointer();
  if (m_TransferPhaseId == true)
  {
    phaseLabelPtr = DataArray<int32_t>::CreateArray(triangleCount, dim, DREAM3D::FaceData::SurfaceMeshPhaseLabels);
    phaseLabel = phaseLabelPtr->getPointer(0);
  }

  ownerLists.resize(nodeCount);

  //Cycle through again assigning coordinates to each node and assigning node numbers and feature labels to each triangle
  triangleCount = 0;
  //const float k_Two = static_cast<float>(2.0);
  for(size_t k = 0; k < zP; k++)
  {
    for(size_t j = 0; j < yP; j++)
    {
      for(size_t i = 0; i < xP; i++)
      {
        point = (k * xP * yP) + (j * xP) + i;
        neigh1 = point + 1;
        neigh2 = point + xP;
        neigh3 = point + (xP * yP);

        if(i == 0)
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId1]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId1]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId1]].pos[2], m_OriginZ);

          nodeId2 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId2]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId2]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId2]].pos[2], m_OriginZ);

          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId3]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId3]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId3]].pos[2], m_OriginZ);

          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId4]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId4]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId4]].pos[2], m_OriginZ);

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId1];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId3];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId4];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId3];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId1]].insert(-1);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId2]].insert(-1);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId3]].insert(-1);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId4]].insert(-1);
        }
        if(j == 0)
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId1]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId1]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId1]].pos[2], m_OriginZ);

          nodeId2 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId2]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId2]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId2]].pos[2], m_OriginZ);

          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId3]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId3]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId3]].pos[2], m_OriginZ);

          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId4]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId4]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId4]].pos[2], m_OriginZ);

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId1];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId3];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId2];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId3];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId4];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId1]].insert(-1);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId2]].insert(-1);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId3]].insert(-1);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId4]].insert(-1);
        }
        if(k == 0)
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId1]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId1]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId1]].pos[2], m_OriginZ);

          nodeId2 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId2]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId2]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId2]].pos[2], m_OriginZ);

          nodeId3 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId3]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId3]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId3]].pos[2], m_OriginZ);

          nodeId4 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId4]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId4]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId4]].pos[2], m_OriginZ);

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId1];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId3];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId4];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId3];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId1]].insert(-1);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId2]].insert(-1);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId3]].insert(-1);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId4]].insert(-1);
        }
        if(i == (xP - 1))
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId1]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId1]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId1]].pos[2], m_OriginZ);

          nodeId2 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId2]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId2]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId2]].pos[2], m_OriginZ);

          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId3]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId3]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId3]].pos[2], m_OriginZ);

          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId4]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId4]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId4]].pos[2], m_OriginZ);

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId3];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId1];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId3];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId4];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId2];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId1]].insert(-1);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId2]].insert(-1);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId3]].insert(-1);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId4]].insert(-1);
        }
        else if(m_FeatureIds[point] != m_FeatureIds[neigh1])
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId1]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId1]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId1]].pos[2], m_OriginZ);

          nodeId2 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId2]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId2]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId2]].pos[2], m_OriginZ);

          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId3]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId3]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId3]].pos[2], m_OriginZ);

          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId4]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId4]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId4]].pos[2], m_OriginZ);

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId1];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId3];
          faceLabels[triangleCount * 2] = m_FeatureIds[neigh1];
          faceLabels[triangleCount * 2 + 1] = m_FeatureIds[point];
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[neigh1];phaseLabel[triangleCount*2+1] = m_CellPhases[point];}
          triangleCount++;

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId4];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId3];
          faceLabels[triangleCount * 2] = m_FeatureIds[neigh1];
          faceLabels[triangleCount * 2 + 1] = m_FeatureIds[point];
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[neigh1];phaseLabel[triangleCount*2+1] = m_CellPhases[point];}
          triangleCount++;

          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[neigh1]);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[neigh1]);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[neigh1]);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[neigh1]);
        }
        if(j == (yP - 1))
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId1]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId1]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId1]].pos[2], m_OriginZ);

          nodeId2 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId2]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId2]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId2]].pos[2], m_OriginZ);

          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId3]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId3]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId3]].pos[2], m_OriginZ);

          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId4]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId4]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId4]].pos[2], m_OriginZ);

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId3];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId1];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId3];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId4];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId2];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId1]].insert(-1);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId2]].insert(-1);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId3]].insert(-1);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId4]].insert(-1);
        }
        else if(m_FeatureIds[point] != m_FeatureIds[neigh2])
        {
          nodeId1 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId1]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId1]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId1]].pos[2], m_OriginZ);

          nodeId2 = (k * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId2]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId2]].pos[1], m_OriginY);
          QSM_GETCOORD(k, zRes, vertex[m_NodeIds[nodeId2]].pos[2], m_OriginZ);

          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId3]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId3]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId3]].pos[2], m_OriginZ);

          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId4]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId4]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId4]].pos[2], m_OriginZ);

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId1];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId3];
          faceLabels[triangleCount * 2] = m_FeatureIds[neigh2];
          faceLabels[triangleCount * 2 + 1] = m_FeatureIds[point];
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[neigh2];phaseLabel[triangleCount*2+1] = m_CellPhases[point];}
          triangleCount++;

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId4];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId3];
          faceLabels[triangleCount * 2] = m_FeatureIds[neigh2];
          faceLabels[triangleCount * 2 + 1] = m_FeatureIds[point];
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[neigh2];phaseLabel[triangleCount*2+1] = m_CellPhases[point];}
          triangleCount++;

          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[neigh2]);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[neigh2]);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[neigh2]);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[neigh2]);
        }
        if(k == (zP - 1))
        {
          nodeId1 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId1]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId1]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId1]].pos[2], m_OriginZ);

          nodeId2 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId2]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId2]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId2]].pos[2], m_OriginZ);

          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId3]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId3]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId3]].pos[2], m_OriginZ);

          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId4]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId4]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId4]].pos[2], m_OriginZ);

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId3];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId1];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId4];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId3];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId2];
          faceLabels[triangleCount * 2] = m_FeatureIds[point];
          faceLabels[triangleCount * 2 + 1] = -1;
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[point];phaseLabel[triangleCount*2+1] = 0;}
          triangleCount++;

          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId1]].insert(-1);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId2]].insert(-1);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId3]].insert(-1);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId4]].insert(-1);
        }
        else if(m_FeatureIds[point] != m_FeatureIds[neigh3])
        {
          nodeId1 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId1]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId1]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId1]].pos[2], m_OriginZ);

          nodeId2 = ((k + 1) * (xP + 1) * (yP + 1)) + (j * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId2]].pos[0], m_OriginX);
          QSM_GETCOORD(j, yRes, vertex[m_NodeIds[nodeId2]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId2]].pos[2], m_OriginZ);

          nodeId3 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + (i + 1);
          QSM_GETCOORD((i + 1), xRes, vertex[m_NodeIds[nodeId3]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId3]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId3]].pos[2], m_OriginZ);

          nodeId4 = ((k + 1) * (xP + 1) * (yP + 1)) + ((j + 1) * (xP + 1)) + i;
          QSM_GETCOORD(i, xRes, vertex[m_NodeIds[nodeId4]].pos[0], m_OriginX);
          QSM_GETCOORD((j + 1), yRes, vertex[m_NodeIds[nodeId4]].pos[1], m_OriginY);
          QSM_GETCOORD((k + 1), zRes, vertex[m_NodeIds[nodeId4]].pos[2], m_OriginZ);

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId1];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId3];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId2];
          faceLabels[triangleCount * 2] = m_FeatureIds[neigh3];
          faceLabels[triangleCount * 2 + 1] = m_FeatureIds[point];
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[neigh3];phaseLabel[triangleCount*2+1] = m_CellPhases[point];}
          triangleCount++;

          triangle[triangleCount].verts[0] = m_NodeIds[nodeId2];
          triangle[triangleCount].verts[1] = m_NodeIds[nodeId3];
          triangle[triangleCount].verts[2] = m_NodeIds[nodeId4];
          faceLabels[triangleCount * 2] = m_FeatureIds[neigh3];
          faceLabels[triangleCount * 2 + 1] = m_FeatureIds[point];
          if(m_TransferPhaseId == true) { phaseLabel[triangleCount*2] = m_CellPhases[neigh3];phaseLabel[triangleCount*2+1] = m_CellPhases[point];}
          triangleCount++;

          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId1]].insert(m_FeatureIds[neigh3]);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId2]].insert(m_FeatureIds[neigh3]);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId3]].insert(m_FeatureIds[neigh3]);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[point]);
          ownerLists[m_NodeIds[nodeId4]].insert(m_FeatureIds[neigh3]);
        }
      }
    }
  }

  for (int i = 0; i < nodeCount; i++)
  {
    nodeTypes[i] = ownerLists[i].size();
    if(nodeTypes[i] > 4) { nodeTypes[i] = 4; }
    if(ownerLists[i].find(-1) != ownerLists[i].end()) { nodeTypes[i] = nodeTypes[i] + 10; }
  }

  sm->setFaces(triangles);
  sm->getAttributeMatrix(getFaceAttributeMatrixName())->addAttributeArray(faceLabelPtr->getName(), faceLabelPtr);
  sm->setVertices(vertices);
  sm->getAttributeMatrix(getVertexAttributeMatrixName())->addAttributeArray(nodeTypePtr->getName(), nodeTypePtr);
  if(m_TransferPhaseId == true)
  {
    sm->getAttributeMatrix(getFaceAttributeMatrixName())->addAttributeArray(phaseLabelPtr->getName(), phaseLabelPtr);
  }
  notifyStatusMessage(getHumanLabel(), "Complete");
}


