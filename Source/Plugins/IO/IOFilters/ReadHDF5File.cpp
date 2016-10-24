/*
 * Your License or Copyright can go here
 */

#include "ReadHDF5File.h"

#include <QtCore/QFileInfo>

#include "SIMPLib/Common/Constants.h"

#include "IO/IOConstants.h"
#include "IO/IOVersion.h"
#include "IO/FilterParameters/ReadHDF5FileFilterParameter.h"

// Include the MOC generated file for this class
#include "moc_ReadHDF5File.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ReadHDF5File::ReadHDF5File() :
  AbstractFilter()
{
  initialize();
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ReadHDF5File::~ReadHDF5File()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ReadHDF5File::initialize()
{
  setErrorCondition(0);
  setCancel(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ReadHDF5File::setupFilterParameters()
{
  FilterParameterVector parameters;

  ReadHDF5FileFilterParameter::Pointer parameter = ReadHDF5FileFilterParameter::New();
  parameter->setHumanLabel("Select HDF5 File");
  parameter->setCategory(FilterParameter::Parameter);
  parameter->setFilter(this);
  parameters.push_back(parameter);

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ReadHDF5File::dataCheck()
{
  setErrorCondition(0);
  
  if (m_HDF5FilePath.isEmpty())
  {
    QString ss = "No HDF5 file has been selected";
    setErrorCondition(-20001);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  QFileInfo hdf5FileInfo(m_HDF5FilePath);
  QString ext = hdf5FileInfo.suffix();
  if (hdf5FileInfo.exists() && hdf5FileInfo.isFile() && ext != "h5" && ext != "hdf5" && ext != "dream3d")
  {
    QString ss = tr("The selected file '%1' is not an HDF5 file.").arg(hdf5FileInfo.fileName());
    setErrorCondition(-20002);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  if (m_SelectedHDF5Paths.size() <= 0)
  {
    QString ss = tr("No datasets are selected. At least one dataset must be selected.");
    setErrorCondition(-20003);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ReadHDF5File::preflight()
{
  // These are the REQUIRED lines of CODE to make sure the filter behaves correctly
  setInPreflight(true); // Set the fact that we are preflighting.
  emit preflightAboutToExecute(); // Emit this signal so that other widgets can do one file update
  emit updateFilterParameters(this); // Emit this signal to have the widgets push their values down to the filter
  dataCheck(); // Run our DataCheck to make sure everthing is setup correctly
  emit preflightExecuted(); // We are done preflighting this filter
  setInPreflight(false); // Inform the system this filter is NOT in preflight mode anymore.
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ReadHDF5File::execute()
{
  initialize();
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  hid_t fileId = H5Utilities::openFile(m_HDF5FilePath.toStdString(), true);
  if (fileId < 0)
  {
    std::cout << "Error Reading HDF5 file: " << m_HDF5FilePath.toStdString() << std::endl;
    return;
  }

  for (int i=0; i<m_SelectedHDF5Paths.size(); i++)
  {
    QString parentPath = QH5Utilities::getParentPath(m_SelectedHDF5Paths[i]);

    hid_t parentId = QH5Utilities::openHDF5Object(fileId, parentPath);

    // Read dataset into DREAM.3D structure

    QH5Utilities::closeFile(parentId);
  }

  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ReadHDF5File::newFilterInstance(bool copyFilterParameters)
{
  ReadHDF5File::Pointer filter = ReadHDF5File::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ReadHDF5File::getCompiledLibraryName()
{ return IOConstants::IOBaseName; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ReadHDF5File::getBrandingString()
{
  return "IO";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ReadHDF5File::getFilterVersion()
{
  QString version;
  QTextStream vStream(&version);
  vStream <<  IO::Version::Major() << "." << IO::Version::Minor() << "." << IO::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ReadHDF5File::getGroupName()
{ return SIMPL::FilterGroups::IOFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ReadHDF5File::getSubGroupName()
{ return SIMPL::FilterSubGroups::InputFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ReadHDF5File::getHumanLabel()
{ return "Read HDF5 File"; }

