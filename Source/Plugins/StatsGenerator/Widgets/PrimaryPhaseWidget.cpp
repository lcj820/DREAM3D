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

#include "PrimaryPhaseWidget.h"


#include <iostream>
#include <limits>

#include <QtCore/QSettings>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>
#include <QtGui/QIntValidator>
#include <QtGui/QDoubleValidator>

// Needed for AxisAngle_t and Crystal Symmetry constants
#include "EbsdLib/EbsdConstants.h"

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Math/SIMPLibMath.h"
#include "SIMPLib/Common/AbstractFilter.h"
#include "SIMPLib/DataArrays/StatsDataArray.h"
#include "SIMPLib/StatsData/StatsData.h"
#include "SIMPLib/StatsData/PrimaryStatsData.h"

#include "OrientationLib/Texture/StatsGen.hpp"

#include "StatsGenerator/Widgets/Presets/MicrostructurePresetManager.h"
#include "StatsGenerator/Widgets/Presets/PrimaryEquiaxedPreset.h"
#include "StatsGenerator/Widgets/Presets/PrimaryRolledPreset.h"
#include "StatsGenerator/Widgets/Presets/PrimaryRecrystallizedPreset.h"
#include "StatsGenerator/Widgets/PhaseTreeWidgetItem.h"
#include "StatsGenerator/Widgets/SGAxisODFWidget.h"
#include "StatsGenerator/Widgets/StatsGenPlotWidget.h"
#include "StatsGenerator/Widgets/StatsGenODFWidget.h"
#include "StatsGenerator/Widgets/StatsGenPlotWidget.h"


//-- Qwt Includes AFTER SIMPLib Math due to improper defines in qwt_plot_curve.h
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_shapeitem.h>
#include <qwt_symbol.h>


#define CHECK_ERROR_ON_WRITE(var, msg)\
  if (err < 0) {\
  QMessageBox::critical(this, tr("StatsGenerator"),\
  tr("There was an error writing the " msg " to the HDF5 file"),\
  QMessageBox::Ok,\
  QMessageBox::Ok);\
  return err;\
  }


#define CHECK_STATS_READ_ERROR(err, group, dataset)\
  if (err < 0) {\
  qDebug() << "PrimaryPhaseWidget::on_actionOpen_triggered Error: Could not read '" << group << "' data set '" << dataset << "'" << "\n";\
  qDebug() << "  File: " << __FILE__ << "\n";\
  qDebug() << "  Line: " << __LINE__ << "\n";\
  return err;\
  }

// Include the MOC generated CPP file which has all the QMetaObject methods/data
#include "moc_PrimaryPhaseWidget.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PrimaryPhaseWidget::PrimaryPhaseWidget(QWidget* parent) :
  SGWidget(parent),
  m_PhaseType(SIMPL::PhaseType::PrimaryPhase),
  m_PhaseFraction(1.0),
  m_TotalPhaseFraction(1.0),
  m_DataHasBeenGenerated(false),
  m_BulkLoadFailure(false),
  m_PhaseIndex(-1),
  m_CrystalStructure(Ebsd::CrystalStructure::Cubic_High),
  m_Omega3Plot(NULL),
  m_BOverAPlot(NULL),
  m_COverAPlot(NULL),
  m_NeighborPlot(NULL),
  m_ODFWidget(NULL),
  m_AxisODFWidget(NULL)
{
  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PrimaryPhaseWidget::~PrimaryPhaseWidget()
{
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::on_microstructurePresetCombo_currentIndexChanged(int index)
{
  //qDebug() << "on_microstructurePresetCombo_currentIndexChanged" << "\n";
  QString presetName = microstructurePresetCombo->currentText();

  //Factory Method to get an instantiated object of the correct type?
  MicrostructurePresetManager::Pointer manager = MicrostructurePresetManager::instance();
  m_MicroPreset = manager->createNewPreset(presetName);
  m_MicroPreset->displayUserInputDialog();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template<typename T>
AbstractMicrostructurePresetFactory::Pointer RegisterPresetFactory(QComboBox* microstructurePresetCombo)
{
  AbstractMicrostructurePresetFactory::Pointer presetFactory = T::New();
  MicrostructurePresetManager::registerFactory(presetFactory);
  QString displayString = (presetFactory->displayName());
  microstructurePresetCombo->addItem(displayString);
  return presetFactory;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::setPhaseName(const QString &phaseName)
{
  m_PhaseNameLabel->setText(phaseName);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::setupGui()
{

  QLocale loc = QLocale::system();

  microstructurePresetCombo->blockSignals(true);
  // Register all of our Microstructure Preset Factories
  AbstractMicrostructurePresetFactory::Pointer presetFactory = AbstractMicrostructurePresetFactory::NullPointer();

  //Register the Equiaxed Preset
  presetFactory = RegisterPresetFactory<PrimaryEquiaxedPresetFactory>(microstructurePresetCombo);
  QString presetName = (presetFactory->displayName());
  MicrostructurePresetManager::Pointer manager = MicrostructurePresetManager::instance();
  m_MicroPreset = manager->createNewPreset(presetName);

  // Register the Rolled Preset
  presetFactory = RegisterPresetFactory<PrimaryRolledPresetFactory>(microstructurePresetCombo);



  // Select the first Preset in the list
  microstructurePresetCombo->setCurrentIndex(0);
  microstructurePresetCombo->blockSignals(false);

  float mu = m_FeatureSizeWidget->getMu();
  float sigma = m_FeatureSizeWidget->getSigma();
  float minCutOff = m_FeatureSizeWidget->getMinCutOff();
  float maxCutOff = m_FeatureSizeWidget->getMaxCutOff();
  float binStepSize = m_FeatureSizeWidget->getBinStep();


  //m_grid->attach(m_SizeDistributionPlot);
  while(m_PlotToolBox->count() > 0)
  {
    m_PlotToolBox->removeItem(0);
  }


  m_Omega3Plot = new StatsGenPlotWidget(nullptr);
  StatsGenPlotWidget* w = m_Omega3Plot;
  w->setDataTitle(QString("Omega 3"));
  w->setXAxisName(QString("Omega 3"));
  w->setYAxisName(QString("Frequency"));
  w->setDistributionType(SIMPL::DistributionType::Beta);
  w->setStatisticsType(SIMPL::StatisticsType::Feature_SizeVOmega3);
  w->blockDistributionTypeChanges(true);
  w->setRowOperationEnabled(false);
  w->setMu(mu);
  w->setSigma(sigma);
  w->setMinCutOff(minCutOff);
  w->setMaxCutOff(maxCutOff);
  w->setBinStep(binStepSize);
  connect(m_Omega3Plot, SIGNAL(userEditedData()),
          this, SLOT(dataWasEdited()));
  connect(m_Omega3Plot, SIGNAL(userEditedData()),
          this, SIGNAL(phaseParametersChanged()));
  connect(m_FeatureSizeWidget, SIGNAL(binSelected(int)),
          m_Omega3Plot, SLOT(highlightCurve(int)));

  m_PlotToolBox->addItem(w, QString("Omega 3 Distribution"));

  m_BOverAPlot = new StatsGenPlotWidget(nullptr);
  w = m_BOverAPlot;
  w->setDataTitle(QString("B/A"));
  w->setXAxisName(QString("B/A"));
  w->setYAxisName(QString("Frequency"));
  w->setDistributionType(SIMPL::DistributionType::Beta);
  w->setStatisticsType(SIMPL::StatisticsType::Feature_SizeVBoverA);
  w->blockDistributionTypeChanges(true);
  w->setRowOperationEnabled(false);
  w->setMu(mu);
  w->setSigma(sigma);
  w->setMinCutOff(minCutOff);
  w->setMaxCutOff(maxCutOff);
  w->setBinStep(binStepSize);
  connect(m_BOverAPlot, SIGNAL(userEditedData()),
          this, SLOT(dataWasEdited()));
  connect(m_BOverAPlot, SIGNAL(userEditedData()),
          this, SIGNAL(phaseParametersChanged()));
  connect(m_FeatureSizeWidget, SIGNAL(binSelected(int)),
          m_BOverAPlot, SLOT(highlightCurve(int)));
  m_PlotToolBox->addItem(w, QString("B/A Shape Distribution"));


  m_COverAPlot = new StatsGenPlotWidget(nullptr);
  w = m_COverAPlot;
  w->setDataTitle(QString("C/A"));
  w->setXAxisName(QString("C/A"));
  w->setYAxisName(QString("Frequency"));
  w->setDistributionType(SIMPL::DistributionType::Beta);
  w->setStatisticsType(SIMPL::StatisticsType::Feature_SizeVCoverA);
  w->blockDistributionTypeChanges(true);
  w->setRowOperationEnabled(false);
  w->setMu(mu);
  w->setSigma(sigma);
  w->setMinCutOff(minCutOff);
  w->setMaxCutOff(maxCutOff);
  w->setBinStep(binStepSize);
  connect(m_COverAPlot, SIGNAL(userEditedData()),
          this, SLOT(dataWasEdited()));
  connect(m_COverAPlot, SIGNAL(userEditedData()),
          this, SIGNAL(phaseParametersChanged()));
  connect(m_FeatureSizeWidget, SIGNAL(binSelected(int)),
          m_COverAPlot, SLOT(highlightCurve(int)));
  m_PlotToolBox->addItem(w, QString("C/A Shape Distribution"));


  m_NeighborPlot = new StatsGenPlotWidget(nullptr);
  w = m_NeighborPlot;
  w->setDataTitle(QString("Number of Neighbors"));
  w->setXAxisName(QString("Number of Features (within 1 diameter)"));
  w->setYAxisName(QString("Frequency"));
  w->setDistributionType(SIMPL::DistributionType::LogNormal);
  w->setStatisticsType(SIMPL::StatisticsType::Feature_SizeVNeighbors);
  w->blockDistributionTypeChanges(true);
  w->setRowOperationEnabled(false);
  w->setMu(mu);
  w->setSigma(sigma);
  w->setMinCutOff(minCutOff);
  w->setMaxCutOff(maxCutOff);
  w->setBinStep(binStepSize);
  connect(m_NeighborPlot, SIGNAL(userEditedData()),
          this, SLOT(dataWasEdited()));
  connect(m_NeighborPlot, SIGNAL(userEditedData()),
          this, SIGNAL(phaseParametersChanged()));
  connect(m_FeatureSizeWidget, SIGNAL(binSelected(int)),
          m_NeighborPlot, SLOT(highlightCurve(int)));
  m_PlotToolBox->addItem(w, QString("Neighbors Distribution"));

  // For the ODF Tab we want the MDF functionality
  m_ODFWidget = new StatsGenODFWidget(m_StackedWidget);
  m_ODFWidget->setWidgetTitle("ODF | MDF Settings");
  m_ODFWidget->enableMDFTab(true);
  // Remove any Axis Decorations. The plots are explicitly know to have a -1 to 1 axis min/max
  m_ODFWidget->setEnableAxisDecorations(false);
  connect(m_ODFWidget, SIGNAL(odfParametersChanged()),
          this, SIGNAL(phaseParametersChanged()));
  connect(m_ODFWidget, SIGNAL(bulkLoadEvent(bool)),
          this, SLOT(bulkLoadEvent(bool)));

  m_AxisODFWidget = new SGAxisODFWidget(m_StackedWidget);
  m_AxisODFWidget->setWidgetTitle("Axis ODF Settings");
  // Remove any Axis Decorations. The plots are explicitly know to have a -1 to 1 axis min/max
  m_AxisODFWidget->setEnableAxisDecorations(false);
  connect(m_AxisODFWidget, SIGNAL(axisODFParametersChanged()),
          this, SIGNAL(phaseParametersChanged()));

  m_StackedWidget->insertWidget(1, m_ODFWidget);
  m_StackedWidget->insertWidget(2, m_AxisODFWidget);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::setPhaseIndex(int index)
{
  m_PhaseIndex = index;
  m_FeatureSizeWidget->setPhaseIndex(index);
  m_Omega3Plot->setPhaseIndex(index);
  m_BOverAPlot->setPhaseIndex(index);
  m_COverAPlot->setPhaseIndex(index);
  m_NeighborPlot->setPhaseIndex(index);
  m_ODFWidget->setPhaseIndex(index);
  m_AxisODFWidget->setPhaseIndex(index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PrimaryPhaseWidget::getPhaseIndex() const
{
  return m_PhaseIndex;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::setCrystalStructure(unsigned int xtal)
{
  m_FeatureSizeWidget->setCrystalStructure(xtal);
  m_Omega3Plot->setCrystalStructure(xtal);
  m_BOverAPlot->setCrystalStructure(xtal);
  m_COverAPlot->setCrystalStructure(xtal);
  m_NeighborPlot->setCrystalStructure(xtal);
  m_ODFWidget->setCrystalStructure(xtal);
  /* Note that we do NOT want to set the crystal structure for the AxisODF widget
   * because we need that crystal structure to be OrthoRhombic in order for those
   * calculations to be performed correctly */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
unsigned int PrimaryPhaseWidget::getCrystalStructure() const
{
  return m_CrystalStructure;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PrimaryPhaseWidget::getComboString()
{
  QString s = QString::number(m_PhaseIndex);
  s.append(" - ");
  if ( Ebsd::CrystalStructure::Cubic_High == m_CrystalStructure)
  {
    s.append("Cubic");
  }
  else if ( Ebsd::CrystalStructure::Hexagonal_High == m_CrystalStructure)
  {
    s.append("Hexagonal");
  }
  return s;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::dataWasEdited()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::setWidgetListEnabled(bool b)
{
  foreach (QWidget* w, m_WidgetList)
  {
    w->setEnabled(b);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::updatePlots()
{
  if (m_DataHasBeenGenerated == true)
  {
    int i = 0;
    QString msg;
    QTextStream out(&msg);
    QProgressDialog progress("Generating Data ....", "Cancel", 0, 8, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(1);

    progress.setValue(++i);
    msg = QString("");
    out << "[" << i << "/8]";
    progress.setLabelText(msg + " Calculating Size Distributions ...");
    m_FeatureSizeWidget->plotSizeDistribution();

    float mu = m_FeatureSizeWidget->getMu();
    float sigma = m_FeatureSizeWidget->getSigma();
    float minCutOff = m_FeatureSizeWidget->getMinCutOff();
    float maxCutOff = m_FeatureSizeWidget->getMaxCutOff();
    float stepSize = m_FeatureSizeWidget->getBinStep();
    QwtArray<float> binSizes = m_FeatureSizeWidget->getBinSizes();

    // Now that we have bins and feature sizes, push those to the other plot widgets
    // Setup Each Plot Widget
    // The MicroPreset class will set the distribution for each of the plots
    progress.setValue(++i);
    msg = QString("");
    out << "[" << i << "/8]";
    progress.setLabelText(msg + " Calculating Omega3 Distributions ...");
    m_Omega3Plot->setSizeDistributionValues(mu, sigma, minCutOff, maxCutOff, stepSize);
    m_MicroPreset->initializeOmega3TableModel(m_Omega3Plot, binSizes);

    progress.setValue(++i);
    msg = QString("");
    out << "[" << i << "/8]";
    progress.setLabelText(msg + " Calculating B/A Size Distributions ...");
    m_BOverAPlot->setSizeDistributionValues(mu, sigma, minCutOff, maxCutOff, stepSize);
    m_MicroPreset->initializeBOverATableModel(m_BOverAPlot, binSizes);

    progress.setValue(++i);
    msg = QString("");
    out << "[" << i << "/8]";
    progress.setLabelText(msg + " Calculating C/A Size Distributions ...");
    m_COverAPlot->setSizeDistributionValues(mu, sigma, minCutOff, maxCutOff, stepSize);
    m_MicroPreset->initializeCOverATableModel(m_COverAPlot, binSizes);

    progress.setValue(++i);
    msg = QString("");
    out << "[" << i << "/8]";
    progress.setLabelText(msg + " Calculating Neighbor Distributions ...");
    m_NeighborPlot->setSizeDistributionValues(mu, sigma, minCutOff, maxCutOff, stepSize);
    m_MicroPreset->initializeNeighborTableModel(m_NeighborPlot, binSizes);

    // Get any presets for the ODF/AxisODF/MDF also
    m_MicroPreset->initializeODFTableModel(m_ODFWidget);
    m_MicroPreset->initializeAxisODFTableModel(m_AxisODFWidget);
    m_MicroPreset->initializeMDFTableModel(m_ODFWidget->getMDFWidget());

    progress.setValue(++i);
    msg = QString("");
    out << "[" << i << "/8]";
    progress.setLabelText(msg + " Calculating ODF & MDF ...");
    m_ODFWidget->updatePlots();

    progress.setValue(++i);
    msg = QString("");
    out << "[" << i << "/8]";
    progress.setLabelText(msg + " Calculating Axis ODF ...");
    m_AxisODFWidget->updatePlots();

    progress.setValue(++i);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::on_m_GenerateDefaultData_clicked()
{
  m_DataHasBeenGenerated = true;
  updatePlots();
  emit phaseParametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::on_m_FeatureSizeDistBtn_toggled()
{
  if(m_FeatureSizeDistBtn->isChecked())
  {
    m_StackedWidget->setCurrentIndex(0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::on_m_OdfBtn_toggled()
{
  if(m_OdfBtn->isChecked())
  {
    m_StackedWidget->setCurrentIndex(1);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::on_m_AxisOdfBtn_toggled()
{
  if(m_AxisOdfBtn->isChecked())
  {
    m_StackedWidget->setCurrentIndex(2);
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::bulkLoadEvent(bool fail)
{
  m_BulkLoadFailure = fail;
}


#define SGWIGET_WRITE_ERROR_CHECK(var)\
  if (err < 0)  {\
  QString msg ("Error Writing Data ");\
  msg.append((var));\
  msg.append(" to the HDF5 file");\
  QMessageBox::critical(this, tr("StatsGenerator"),\
  msg,\
  QMessageBox::Default);\
  retErr = -1;\
  }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PrimaryPhaseWidget::gatherStatsData(AttributeMatrix::Pointer attrMat, bool preflight)
{
  if (m_PhaseIndex < 1)
  {
    QMessageBox::critical(this, tr("StatsGenerator"),
                          tr("The Phase Index is Less than 1. This is not allowed."),
                          QMessageBox::Default);
    return -1;
  }
  int retErr = 0;
  int err = 0;

  float calcPhaseFraction = m_PhaseFraction / m_TotalPhaseFraction;





  // Get pointers
  IDataArray::Pointer iDataArray = attrMat->getAttributeArray(SIMPL::EnsembleData::CrystalStructures);
  unsigned int* crystalStructures = std::dynamic_pointer_cast< UInt32ArrayType >(iDataArray)->getPointer(0);
  iDataArray = attrMat->getAttributeArray(SIMPL::EnsembleData::PhaseTypes);
  unsigned int* phaseTypes = std::dynamic_pointer_cast< UInt32ArrayType >(iDataArray)->getPointer(0);

  crystalStructures[m_PhaseIndex] = m_CrystalStructure;
  phaseTypes[m_PhaseIndex] = m_PhaseType;

  StatsDataArray* statsDataArray = StatsDataArray::SafeObjectDownCast<IDataArray*, StatsDataArray*>(attrMat->getAttributeArray(SIMPL::EnsembleData::Statistics).get());
  if (NULL != statsDataArray)
  {
    StatsData::Pointer statsData = statsDataArray->getStatsData(m_PhaseIndex);
    PrimaryStatsData* primaryStatsData = PrimaryStatsData::SafePointerDownCast(statsData.get());

    primaryStatsData->setPhaseFraction(calcPhaseFraction);

    m_FeatureSizeWidget->getStatisticsData(primaryStatsData);

    // Now that we have bins and feature sizes, push those to the other plot widgets
    {
      VectorOfFloatArray data = m_Omega3Plot->getStatisticsData();
      primaryStatsData->setFeatureSize_Omegas(data);
      primaryStatsData->setOmegas_DistType(m_Omega3Plot->getDistributionType());
    }
    {
      VectorOfFloatArray data = m_BOverAPlot->getStatisticsData();
      primaryStatsData->setFeatureSize_BOverA(data);
      primaryStatsData->setBOverA_DistType(m_BOverAPlot->getDistributionType());
    }
    {
      VectorOfFloatArray data = m_COverAPlot->getStatisticsData();
      primaryStatsData->setFeatureSize_COverA(data);
      primaryStatsData->setCOverA_DistType(m_COverAPlot->getDistributionType());
    }
    {
      VectorOfFloatArray data = m_NeighborPlot->getStatisticsData();
      primaryStatsData->setFeatureSize_Neighbors(data);
      primaryStatsData->setNeighbors_DistType(m_NeighborPlot->getDistributionType());
    }

    m_ODFWidget->getOrientationData(primaryStatsData, SIMPL::PhaseType::PrimaryPhase, preflight);

    err = m_AxisODFWidget->getOrientationData(primaryStatsData, SIMPL::PhaseType::PrimaryPhase, preflight);
  }
  return retErr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PrimaryPhaseWidget::extractStatsData(AttributeMatrix::Pointer attrMat, int index)
{
  setWidgetListEnabled(true);
  float mu = 1.0f;
  float sigma = 1.0f;
  float minCutOff = 5.0f;
  float maxCutOff = 5.0f;
  float binStepSize = 0.5f;

  setPhaseIndex(index);

  IDataArray::Pointer iDataArray = attrMat->getAttributeArray(SIMPL::EnsembleData::CrystalStructures);
  unsigned int* attributeArray = std::dynamic_pointer_cast< UInt32ArrayType >(iDataArray)->getPointer(0);
  m_CrystalStructure = attributeArray[index];

  iDataArray = attrMat->getAttributeArray(SIMPL::EnsembleData::PhaseTypes);
  attributeArray = std::dynamic_pointer_cast< UInt32ArrayType >(iDataArray)->getPointer(0);
  m_PhaseType = attributeArray[index];

  iDataArray = attrMat->getAttributeArray(SIMPL::EnsembleData::Statistics);
  StatsDataArray* statsDataArray = StatsDataArray::SafeObjectDownCast<IDataArray*, StatsDataArray*>(iDataArray.get());
  if (statsDataArray == NULL)
  {
    return;
  }
  StatsData::Pointer statsData = statsDataArray->getStatsData(index);
  PrimaryStatsData* primaryStatsData = PrimaryStatsData::SafePointerDownCast(statsData.get());

  m_PhaseFraction = primaryStatsData->getPhaseFraction();
  m_FeatureSizeWidget->setCrystalStructure(m_CrystalStructure);
  m_Omega3Plot->setCrystalStructure(m_CrystalStructure);
  m_BOverAPlot->setCrystalStructure(m_CrystalStructure);
  m_COverAPlot->setCrystalStructure(m_CrystalStructure);
  m_NeighborPlot->setCrystalStructure(m_CrystalStructure);
  m_ODFWidget->setCrystalStructure(m_CrystalStructure);
  // m_AxisODFWidget->setCrystalStructure(m_CrystalStructure);


  /* Set the BinNumbers data set */
  FloatArrayType::Pointer bins = primaryStatsData->getBinNumbers();



  QVector<float> qbins (bins->getNumberOfTuples());
  for(int i = 0; i < qbins.size(); ++i)
  {
    qbins[i] = bins->getValue(i);
  }

  m_FeatureSizeWidget->extractStatsData(primaryStatsData, index);
  m_FeatureSizeWidget->gatherSizeDistributionFromGui(mu, sigma, minCutOff, maxCutOff, binStepSize);

  // Now have each of the plots set it's own data
  m_Omega3Plot->setDistributionType(primaryStatsData->getOmegas_DistType(), false);
  m_Omega3Plot->extractStatsData(index, qbins, primaryStatsData->getFeatureSize_Omegas());
  m_Omega3Plot->setSizeDistributionValues(mu, sigma, minCutOff, maxCutOff, binStepSize);

  m_BOverAPlot->setDistributionType(primaryStatsData->getBOverA_DistType(), false);
  m_BOverAPlot->extractStatsData(index, qbins, primaryStatsData->getFeatureSize_BOverA());
  m_BOverAPlot->setSizeDistributionValues(mu, sigma, minCutOff, maxCutOff, binStepSize);

  m_COverAPlot->setDistributionType(primaryStatsData->getCOverA_DistType(), false);
  m_COverAPlot->extractStatsData(index, qbins, primaryStatsData->getFeatureSize_COverA());
  m_COverAPlot->setSizeDistributionValues(mu, sigma, minCutOff, maxCutOff, binStepSize);

  m_NeighborPlot->setDistributionType(primaryStatsData->getNeighbors_DistType(), false);
  m_NeighborPlot->extractStatsData(index, qbins, primaryStatsData->getFeatureSize_Neighbors());
  m_NeighborPlot->setSizeDistributionValues(mu, sigma, minCutOff, maxCutOff, binStepSize);

  // Set the ODF Data
  m_ODFWidget->extractStatsData(index, primaryStatsData, SIMPL::PhaseType::PrimaryPhase);

  // Set the Axis ODF Data
  m_AxisODFWidget->extractStatsData(index, primaryStatsData, SIMPL::PhaseType::PrimaryPhase);


  m_DataHasBeenGenerated = true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QIcon PrimaryPhaseWidget::getPhaseIcon()
{
  QIcon icon;
  icon.addFile(QStringLiteral(":/StatsGenerator/icons/Primary.png"), QSize(), QIcon::Normal, QIcon::Off);
  icon.addFile(QStringLiteral(":/StatsGenerator/icons/Primary_Selected.png"), QSize(), QIcon::Normal, QIcon::On);
  return icon;
}
