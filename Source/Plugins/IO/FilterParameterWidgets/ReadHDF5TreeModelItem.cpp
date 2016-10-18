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

#include "ReadHDF5TreeModelItem.h"

#include <QtCore/QStringList>

#include "H5Support/H5Utilities.h"

ReadHDF5TreeModelItem::ReadHDF5TreeModelItem(hid_t fileId, const QString &data, ReadHDF5TreeModelItem *parent)
{
  this->_fileId = fileId;
  parentItem = parent;
  itemData = QVariant(data);
  childItemsInitialized = 0;
  _childCount = -1;
  this->_num_attrs = -1;
  this->_numDims = -1;
  this->_isGroup = false;
  this->_isImage = false;
  this->_isTable = false;
  this->_isString = false;

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ReadHDF5TreeModelItem::~ReadHDF5TreeModelItem()
{
  qDeleteAll(childItems);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ReadHDF5TreeModelItem::appendChild(ReadHDF5TreeModelItem *item)
{
  childItems.append(item);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ReadHDF5TreeModelItem *ReadHDF5TreeModelItem::child(int row)
{
  if (childItemsInitialized == 0)
  {
    initializeChildItems();
  }
  return childItems.value(row);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ReadHDF5TreeModelItem::childCount()
{
  if (_childCount < 0)
  {
    this->initializeChildCount();
  }
  return this->_childCount;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ReadHDF5TreeModelItem::columnCount()
{
  return 1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant ReadHDF5TreeModelItem::data(int column)
{
  return itemData;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ReadHDF5TreeModelItem *ReadHDF5TreeModelItem::parent()
{
  return parentItem;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ReadHDF5TreeModelItem::row()
{
  if (parentItem)
    return parentItem->childItems.indexOf(const_cast<ReadHDF5TreeModelItem*>(this));

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool ReadHDF5TreeModelItem::isGroup()
{
  if (this->_childCount < 0)
  {
    this->initializeChildCount();
  }
  return this->_isGroup;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ReadHDF5TreeModelItem::numAttributes()
{
  if (this->_num_attrs < 0)
  {
    this->initializeChildCount();
  }
  return this->_num_attrs;
}

int ReadHDF5TreeModelItem::numDimensions()
{
  return this->_numDims;
}

bool ReadHDF5TreeModelItem::isImage()
{
  if (childItemsInitialized == 0)
  {
    initializeChildItems();
  }
  return this->_isImage;
}

bool ReadHDF5TreeModelItem::isString()
{
  if (childItemsInitialized == 0)
  {
    initializeChildItems();
  }
  return this->_isString;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QIcon ReadHDF5TreeModelItem::icon()
{
  QString iconName = "";
  if (this->isGroup() == true)
  {
    iconName = ":/folderclose";
  }
  else if (this->isImage() )
  {
    iconName = ":/image";
  }
  else
  {
    iconName = ":/dataset";
  }

  if (this->numAttributes() > 0)
  {
    iconName = iconName.append("A");
  }

  iconName = iconName.append(".png");
  return QIcon(iconName);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ReadHDF5TreeModelItem::initializeChildCount()
{
  if (this->_fileId < 0)
  {
    return;
  }
  // std::cout << "ReadHDF5TreeModelItem::initializeChildCount()" << std::endl;
  herr_t err = 0;
  // Build up an HDF path to the current group
  ReadHDF5TreeModelItem* currentParent = this->parentItem;
  if (NULL == currentParent)
  {
    QString name( "/");  // Add the "HDF5 Root Directory"
    ReadHDF5TreeModelItem* item = new ReadHDF5TreeModelItem(this->_fileId,
                                                            name,
                                                            const_cast<ReadHDF5TreeModelItem*>(this));
    childItems.append(item);
    _childCount = 1;
    this->_num_attrs = 0;
    this->_numDims = 0;
    this->_isGroup = true;
    return;
  }

  QString path = generateHDFPath();

  hid_t obj_id = H5Utilities::openHDF5Object(this->_fileId, path.toStdString());
  if (obj_id > 0) {
    H5O_info_t object_info;
    err = H5Oget_info(obj_id, &object_info);
    this->_num_attrs = object_info.num_attrs;
  }
  H5Utilities::closeHDF5Object(obj_id);

  if (H5Utilities::isGroup(this->_fileId, path.toStdString()) )
  {
    this->_isGroup = true;
    hid_t groupId = H5Gopen(this->_fileId, path.toStdString().c_str(), H5P_DEFAULT);
    if (groupId < 0)
    {
      std::cout << "Could not open Group '" << path.toStdString() << "'" <<__FILE__ << ":" << __LINE__ <<  std::endl;
      return;
    }

    H5G_info_t group_info;
    err = H5Gget_info(groupId, &group_info);
    this->_childCount = group_info.nlinks;

    err = H5Gclose(groupId);
  }
  else
  {
    this->_childCount = 0;
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ReadHDF5TreeModelItem::initializeChildItems()
{
  if (this->_fileId < 0)
  {
    return;
  }
  //std::cout << "ReadHDF5TreeModelItem::initializeChildItems()" << std::endl;
  herr_t err = 0;
  // Build up an HDF path to the current group
  ReadHDF5TreeModelItem* currentParent = this->parentItem;
  if (NULL == currentParent)
  {
    QString name( "/");  // Add the "HDF5 Root Directory"
    ReadHDF5TreeModelItem* item = new ReadHDF5TreeModelItem(this->_fileId,
                                                            name,
                                                            const_cast<ReadHDF5TreeModelItem*>(this));
    childItems.append(item);
    childItemsInitialized = 1;
    return;
  }

  QString path = generateHDFPath();

  //std::cout << "ReadHDF5TreeModelItem::initializeChildItems() - Generated Path as: " << path.toStdString() << std::endl;
  // Check to see if the path is a group or data set
  if (H5Utilities::isGroup(this->_fileId, path.toStdString()) )
  {
    this->_isGroup = true;
    hid_t groupId = H5Gopen(this->_fileId, path.toStdString().c_str(), H5P_DEFAULT);
    if (groupId < 0)
    {
      std::cout << "Could not open Group '" << path.toStdString() << "'" <<__FILE__ << ":" << __LINE__ <<  std::endl;
      return;
    }

    H5O_info_t object_info;
    err = H5Oget_info(groupId, &object_info);
    this->_num_attrs = object_info.num_attrs;

    std::list<std::string> itemList;
    herr_t err = H5Utilities::getGroupObjects(groupId, H5Utilities::H5Support_ANY, itemList);
    if (err < 0)
    {
      std::cout << "Error getting group objects. " <<__FILE__ << ":" << __LINE__ << std::endl;
      err = H5Gclose(groupId);
      return;
    }

    for (std::list<std::string>::iterator iter = itemList.begin(); iter != itemList.end(); ++iter )
    {
      // std::cout << "Adding Child with name '" << *iter << "'" << std::endl;
      QString name( (*iter).c_str() );
      ReadHDF5TreeModelItem* item = new ReadHDF5TreeModelItem(this->_fileId,
                                                              name,
                                                              const_cast<ReadHDF5TreeModelItem*>(this));
      childItems.append(item);
    }
    this->_childCount = itemList.size();
    err = H5Gclose(groupId);
  }
  else  // Get some basic information about the data set
  {
    //std::cout << "TreeModelItem is a DataSet" << std::endl;
    hid_t obj_id = H5Utilities::openHDF5Object(this->_fileId, path.toStdString());
    if (obj_id > 0) {
      H5O_info_t object_info;
      err = H5Oget_info(obj_id, &object_info);
      this->_num_attrs = object_info.num_attrs;
    }
    if (_num_attrs > 0)
    {
      // Test for Image Class attribute
      std::string data;
      err = H5Lite::readStringAttribute(this->_fileId, path.toStdString(), "CLASS", data);
      if (err >= 0)
      {
        this->_isImage = true;
      }
    }

    std::vector<hsize_t> dims;
    H5T_class_t data_type;
    size_t type_size;
    err = H5Lite::getDatasetInfo(this->_fileId, path.toStdString(), dims, data_type, type_size);
    this->_numDims = dims.size();

    switch (data_type)
    {
    case H5T_STRING:
      this->_isString = true;
      this->_dataType = "H5T_STRING";
      break;
    default:
      this->_dataType = "OTHER";
    }

    err = H5Utilities::closeHDF5Object(obj_id);
    if (err < 0)
    {
      //TODO: Catch this error
    }
  }
  childItemsInitialized = 1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ReadHDF5TreeModelItem::generateHDFPath()
{
  if (this->_fileId < 0)
  {
    return QString();
  }

  // Build up an HDF path to the current group
  ReadHDF5TreeModelItem* currentParent = this->parentItem;
  QString path = itemData.toString();
  //std::cout << "Current Item Data: " << path.toStdString() << std::endl;
  if (currentParent != NULL && currentParent->itemData.toString().compare("HEADER") == 0)
  {
    // std::cout << "path=" << path.toStdString() << std::endl;
    currentParent = NULL; // We are at the top
  }
  while (currentParent != NULL)
  {
    QString parentName = currentParent->data(0).toString();
    if (parentName.compare("/") == 0)
    {
      path =  parentName + path;
    }
    else
    {
      path =  parentName + "/" + path;
    }
    // std::cout << "path=" << path.toStdString() << std::endl;
    // Get the parents Parent..
    currentParent = currentParent->parent();
    if (currentParent->parent() == NULL)
    {
      currentParent = NULL; // We are at the top
    }
  }
  return path;
}

