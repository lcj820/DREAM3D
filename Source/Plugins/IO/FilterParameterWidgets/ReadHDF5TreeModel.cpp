/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
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
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <QtGui>
#include <QIcon>
#include <QPixmap>

#include "H5Support/H5Utilities.h"

#include "ReadHDF5TreeModel.h"
#include "ReadHDF5TreeModelItem.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ReadHDF5TreeModel::ReadHDF5TreeModel(hid_t fileId, QObject *parent) :
  QAbstractItemModel(parent), _fileId(fileId)
{
  rootItem = new ReadHDF5TreeModelItem(_fileId, "HEADER");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ReadHDF5TreeModel::~ReadHDF5TreeModel()
{
  delete rootItem;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ReadHDF5TreeModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return static_cast<ReadHDF5TreeModelItem*>(parent.internalPointer())->columnCount();
  else
    return rootItem->columnCount();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant ReadHDF5TreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  ReadHDF5TreeModelItem *item = static_cast<ReadHDF5TreeModelItem*>(index.internalPointer());

  if (role == Qt::DecorationRole) {
    return item->icon();
  }

  if (role != Qt::DisplayRole)
    return QVariant();



  return item->data(index.column());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Qt::ItemFlags ReadHDF5TreeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant ReadHDF5TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    return QVariant("HEADER");
    // return rootItem->data(section);
  }
  return QVariant();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QModelIndex ReadHDF5TreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  ReadHDF5TreeModelItem *parentItem;

  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<ReadHDF5TreeModelItem*>(parent.internalPointer());

  ReadHDF5TreeModelItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QModelIndex ReadHDF5TreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  ReadHDF5TreeModelItem *childItem = static_cast<ReadHDF5TreeModelItem*>(index.internalPointer());
  ReadHDF5TreeModelItem *parentItem = childItem->parent();

  if (parentItem == rootItem)
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool ReadHDF5TreeModel::hasChildren(const QModelIndex &parent) const
{
  //  if (!parent.isValid()) //
  //      return true; //

  if (parent.column() > 0)
    return false;

  ReadHDF5TreeModelItem *parentItem;
  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<ReadHDF5TreeModelItem*>(parent.internalPointer());

  return parentItem->isGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ReadHDF5TreeModel::rowCount(const QModelIndex &parent) const
{
  ReadHDF5TreeModelItem *parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<ReadHDF5TreeModelItem*>(parent.internalPointer());

  return parentItem->childCount();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ReadHDF5TreeModel::setupModelData()
{
  if (this->_fileId < 0)
  {
    return;
  }

  this->rootItem->appendChild(new ReadHDF5TreeModelItem(this->_fileId, QString("/"), this->rootItem));

#if 0
  std::list<std::string> itemList;
  herr_t err = H5Utilities::getGroupObjects(this->_fileId, H5Utilities::MXA_ANY, itemList);
  if (err < 0)
  {
    std::cout << "Error getting group objects. " <<__FILE__ << ":" << __LINE__ << std::endl;
    return;
  }


  for (std::list<std::string>::iterator iter = itemList.begin(); iter != itemList.end(); ++iter )
  {
    this->rootItem->appendChild(new ReadHDF5TreeModelItem(this->_fileId, QString( (*iter).c_str() ), this->rootItem) );
  }
#endif
  //parents.last()->appendChild(new ReadHDF5TreeModelItem(columnData, parents.last()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ReadHDF5TreeModel::hdfPathForIndex(const QModelIndex &index)
{
  ReadHDF5TreeModelItem* item = static_cast<ReadHDF5TreeModelItem*>(index.internalPointer());
  return item->generateHDFPath();
}


