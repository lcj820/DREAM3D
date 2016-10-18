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

#ifndef _readhdf5filewidget_h_
#define _readhdf5filewidget_h_

#include <QtWidgets/QWidget>
#include <QtWidgets/QMainWindow>

#include "SVWidgetsLib/FilterParameterWidgets/FilterParameterWidget.h"

#include "hdf5.h"

//-- UIC generated Header
#include <ui_ReadHDF5FileWidget.h>

#define READ_HDF5_EXTRA_FEATURES 0

class IH5DataWindow;
class QDockWidget;

/* Uses the 'Private Inheritance' method from the *.ui file */
/**
 * @brief Main Window for the Qt based HDF5 Data Viewer
 * @author Mike Jackson
 * @date June 2008
 * @version $Revision: 1.7 $
 * @class ReadHDF5FileWidget ReadHDF5FileWidget.h ReadHDF5FileWidget.h
 */
class ReadHDF5FileWidget : public FilterParameterWidget, private Ui::ReadHDF5FileWidget
{
  Q_OBJECT

public:
  ReadHDF5FileWidget(FilterParameter* parameter, AbstractFilter* filter = nullptr, QWidget* parent = nullptr);

  /**
   * @brief focusInEvent
   * @param event
   */
  virtual void focusInEvent(QFocusEvent* event);

  /**
   * @brief focusOutEvent
   * @param event
   */
  virtual void focusOutEvent(QFocusEvent* event);

  /**
   * @brief openHDF5File
   * @param hdf5File
   */
  void openHDF5File(QString &hdf5File);

  /**
   * @brief getCurrentFile
   * @return
   */
  QString getCurrentFile() { return m_CurrentOpenFile; }

protected:

  /**
   * @brief Implements the CloseEvent to Quit the application and write settings
   * to the preference file
   */
  void closeEvent(QCloseEvent *event);

  /**
   * @brief Drag and drop implementation
   */
  void dragEnterEvent(QDragEnterEvent*);

  /**
   * @brief Drag and drop implementation
   */
  void dropEvent(QDropEvent*);

  /**
   * @brief Initializes some of the GUI elements with selections or other GUI related items
   */
  void setupGui();

  /**
   * @brief checks to make sure undocked widgets are correctly positioned on screen.
   * @param dock_widget the QDockWidget to check.
   */
  void sanityCheckDock(QDockWidget* dock_widget);

  /**
   * @brief Initializes the window by trying to open and populate the window
   * with values from the passed in file
   * @param mxaFile A Path to an MXA File
   */
  void initWithFile(const QString &mxaFile);

private slots:
  #if READ_HDF5_EXTRA_FEATURES
  void actionOpen_triggered();

  void hdfTreeView_doubleClicked(const QModelIndex & index);
#endif

  /**
   * @brief Slot to catch events when the DataRecord TreeView selection is changed
   * @param current The currently selected Index
   * @param previous The previously selected Index
   */
  void hdfTreeView_currentChanged ( const QModelIndex & current, const QModelIndex & previous );

  /**
   * @brief Qt Slot that fires in response to a click on a "Recent File' Menu entry.
   */
  void openRecentFile();

private:

  static QString                m_OpenDialogLastDirectory; //Stores the last directory the user visited
  QString                       m_CurrentOpenFile;    //Stores the currently open HDF5 File
  std::string                   m_CurrentHDFDataPath;  //Stores the currently viewed HDF data path
  hid_t                         m_FileId;

  /**
   * @brief Generates the path to the HDF dataset based on the values in the
   * data dimension table and the selected data record.
   * @param model The MXADataModel
   * @param indices The Indices for the Data Dimensions
   * @param record The selected MXADataRecord
   * @return The path as a std::string object
   */
  std::string generateHDFPath();

  /**
   * @brief Updates the QGraphicsView based on the current Data Dimension and Data record values
   * @param path The path to the HDF data set
   */
  void _updateViewFromHDFPath(std::string path);

  herr_t updateAttributeTable(const QString &datasetPath);
  herr_t updateGeneralTable(const QString &path);
  void addRow(QTableWidget* table, int row, const QString &key, const QString &value);

  qint32 _checkDirtyDocument();

  #if READ_HDF5_EXTRA_FEATURES
  IH5DataWindow* findDataWindow(const QString &hdfPath);
#endif

  void closeAllChildWindows();

  ~ReadHDF5FileWidget();
  ReadHDF5FileWidget(const ReadHDF5FileWidget&);   //Copy Constructor Not Implemented
  void operator=(const ReadHDF5FileWidget&); //Copy Assignment Not Implemented
};

#endif /* _readhdf5filewidget_h_ */

