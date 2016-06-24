#include "StatsGenFeatureSizeWidget.h"

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
#include <QtGui/QMouseEvent>

// Needed for AxisAngle_t and Crystal Symmetry constants
#include "EbsdLib/EbsdConstants.h"

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/StatsData/PrimaryStatsData.h"

#include "OrientationLib/Texture/StatsGen.hpp"


//-- Qwt Includes AFTER SIMPLib Math due to improper defines in qwt_plot_curve.h
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_shapeitem.h>
#include <qwt_symbol.h>
#include <qwt_picker_machine.h>



#include "curvetracker.h"

// Include the MOC generated CPP file which has all the QMetaObject methods/data
#include "moc_StatsGenFeatureSizeWidget.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGenFeatureSizeWidget::StatsGenFeatureSizeWidget(QWidget* parent) :
  QWidget(parent),
  m_Mu(1.0f),
  m_Sigma(0.1f),
  m_MinCutOff(3.0f),
  m_MaxCutOff(3.0f),
  m_BinStep(1.0f),
  m_PhaseIndex(-1),
  m_CrystalStructure(Ebsd::CrystalStructure::Cubic_High),
  m_SizeDistributionCurve(NULL),
  m_SizeDistCenterPointCurve(NULL),
  m_CutOffMin(NULL),
  m_CutOffMax(NULL),
  m_grid(NULL),
  m_MuValidator(NULL),
  m_SigmaValidator(NULL)
{
  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGenFeatureSizeWidget::~StatsGenFeatureSizeWidget()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::setupGui()
{
  QLocale loc = QLocale::system();

  distributionTypeCombo->addItem(SIMPL::StringConstants::BetaDistribution.toLatin1().data());
  distributionTypeCombo->addItem(SIMPL::StringConstants::LogNormalDistribution.toLatin1().data());
  distributionTypeCombo->addItem(SIMPL::StringConstants::PowerLawDistribution.toLatin1().data());
  distributionTypeCombo->setCurrentIndex(SIMPL::DistributionType::LogNormal);
  distributionTypeCombo->setVisible(false);

  m_MuValidator = new QDoubleValidator(m_Mu_SizeDistribution);
  m_MuValidator->setLocale(loc);
  m_MuValidator->setRange(0.0001, 10.0, 4);

  m_Mu_SizeDistribution->setValidator(m_MuValidator);

  m_SigmaValidator = new QDoubleValidator(m_Sigma_SizeDistribution);
  m_SigmaValidator->setLocale(loc);
  m_SigmaValidator->setRange(0.0000, 1.0, 4);
  m_Sigma_SizeDistribution->setValidator(m_SigmaValidator);

  QDoubleValidator* minVal = new QDoubleValidator(0.000, std::numeric_limits<double>::infinity(), 4, m_MinSigmaCutOff);
  minVal->setLocale(loc);
  m_MinSigmaCutOff->setValidator(minVal);

  QDoubleValidator* maxVal = new QDoubleValidator(0.000, std::numeric_limits<double>::infinity(), 4, m_MinSigmaCutOff);
  maxVal->setLocale(loc);
  m_MaxSigmaCutOff->setValidator(maxVal);


  QwtPlotPicker* plotPicker = new QwtPlotPicker(m_SizeDistributionPlot->xBottom, m_SizeDistributionPlot->yLeft, QwtPicker::CrossRubberBand, QwtPicker::AlwaysOn, m_SizeDistributionPlot->canvas());
  QwtPickerMachine* pickerMachine = new QwtPickerClickPointMachine();
  plotPicker->setStateMachine(pickerMachine);
  connect(plotPicker, SIGNAL(selected(const QPointF&)), this, SLOT(plotPointSelected(const QPointF&)));


  m_SizeDistributionPlot->setCanvasBackground(QColor(Qt::white));
  m_SizeDistributionPlot->setTitle("Size Distribution");

#if 1
  m_grid = new QwtPlotGrid;
  m_grid->enableXMin(true);
  m_grid->enableYMin(true);
  m_grid->setMajorPen(QPen(Qt::gray, 0, Qt::SolidLine));
  m_grid->setMinorPen(QPen(Qt::lightGray, 0, Qt::DotLine));
#endif


  updateSizeDistributionPlot();
  calculateNumberOfBins();

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::plotPointSelected(const QPointF& point)
{
  int selectedBin = -1;
  for(int i = 0; i < m_BarChartItems.size(); i++)
  {
    QwtPlotShapeItem* item = m_BarChartItems[i];

    //qDebug()  << "\t" << item->boundingRect();
    if( item->boundingRect().contains(point) )
    {
      QColor fillColor = QColor("IndianRed");
      fillColor.setAlpha( 200 );
      item->setBrush( fillColor );
      selectedBin = i;
    }
    else
    {
      QColor fillColor = QColor("RoyalBlue");
      fillColor.setAlpha( 200 );
      item->setBrush( fillColor );
    }
  }
  m_SizeDistributionPlot->replot();
  emit binSelected(selectedBin);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::mousePressEvent(QMouseEvent *event)
{
  qDebug() << "mousePressedEvent";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int StatsGenFeatureSizeWidget::gatherSizeDistributionFromGui(float& mu, float& sigma, float& minCutOff, float& maxCutOff, float& stepSize)
{
  QLocale loc = QLocale::system();

  bool ok = false;
  mu = loc.toFloat(m_Mu_SizeDistribution->text(), &ok);
  if (ok == false)
  {
    return 0;
  }
  sigma = loc.toFloat(m_Sigma_SizeDistribution->text(), &ok);
  if (ok == false)
  {
    return 0;
  }
  minCutOff = loc.toFloat(m_MinSigmaCutOff->text(), &ok);
  if (ok == false)
  {
    return 0;
  }
  maxCutOff = loc.toFloat(m_MaxSigmaCutOff->text(), &ok);
  if (ok == false)
  {
    return 0;
  }
  stepSize = loc.toFloat(m_BinStepSize->text(), &ok);
  if (ok == false)
  {
    return 0;
  }
  return 1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool StatsGenFeatureSizeWidget::validateValue(QDoubleValidator* val, QLineEdit* lineEdit)
{
  QString value(lineEdit->text());
  int not_used = 0;
  QValidator::State state = val->validate(value, not_used);
  if(state != QValidator::Acceptable)
  {
    lineEdit->setStyleSheet("border: 1px solid red;");
    return false;
  }
  else
  {
    lineEdit->setStyleSheet("");
  }
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool StatsGenFeatureSizeWidget::validateMuSigma()
{
  bool muValid = validateValue(m_MuValidator, m_Mu_SizeDistribution);
  bool sigmaValid = validateValue(m_SigmaValidator, m_Sigma_SizeDistribution);

  if (muValid && sigmaValid)
  {
  //  m_GenerateDefaultData->setEnabled(true);
    return true;
  }

  m_NumberBinsGenerated->setText("Error");
 // m_GenerateDefaultData->setEnabled(false);
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::on_m_Mu_SizeDistribution_textChanged(const QString& text)
{
  Q_UNUSED(text)
  if (!validateMuSigma())
  {
    return;
  }
  updateSizeDistributionPlot();
  m_Mu_SizeDistribution->setFocus();
  calculateNumberOfBins();
  emit phaseParametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::on_m_Sigma_SizeDistribution_textChanged(const QString& text)
{
  Q_UNUSED(text)
  if (!validateMuSigma())
  {
    return;
  }
  updateSizeDistributionPlot();
  m_Sigma_SizeDistribution->setFocus();
  calculateNumberOfBins();
  emit phaseParametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::on_m_MinSigmaCutOff_textChanged(const QString& text)
{
  Q_UNUSED(text)
  updateSizeDistributionPlot();
  m_MinSigmaCutOff->setFocus();
  calculateNumberOfBins();
  emit phaseParametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::on_m_MaxSigmaCutOff_textChanged(const QString& text)
{
  Q_UNUSED(text)
  updateSizeDistributionPlot();
  m_MaxSigmaCutOff->setFocus();
  calculateNumberOfBins();
  emit phaseParametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::on_m_BinStepSize_valueChanged(double v)
{
  Q_UNUSED(v)
  calculateNumberOfBins();
  emit phaseParametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::calculateNumberOfBins()
{
  float mu = 1.0;
  float sigma = 1.0;
  float minCutOff = 1.0;
  float maxCutOff = 1.0;
  float stepSize = 1.0;
  float max, min;
  int err = gatherSizeDistributionFromGui(mu, sigma, minCutOff, maxCutOff, stepSize);
  if (err == 0)
  {
    return;
  }

  int n = StatsGen::ComputeNumberOfBins(mu, sigma, minCutOff, maxCutOff, stepSize, max, min);
  if(err < 0)
  {
    m_NumberBinsGenerated->setText("Error");
  }
  else
  {
    m_NumberBinsGenerated->setText(QString::number(n));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int StatsGenFeatureSizeWidget::calculateNumberOfBins(float mu, float sigma, float minCutOff, float maxCutOff, float stepSize)
{
  float max, min; // Only needed for the method. Not used otherwise.

  return StatsGen::ComputeNumberOfBins(mu, sigma, minCutOff, maxCutOff, stepSize, max, min);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int StatsGenFeatureSizeWidget::computeBinsAndCutOffs( float mu, float sigma,
                                               float minCutOff, float maxCutOff,
                                               float binStepSize,
                                               QwtArray<float>& binsizes,
                                               QwtArray<float>& xCo,
                                               QwtArray<float>& yCo,
                                               float& xMax, float& yMax,
                                               QwtArray<float>& x,
                                               QwtArray<float>& y)
{
  int err = 0;
  int size = 250;


  err = StatsGen::GenLogNormalPlotData<QwtArray<float> > (mu, sigma, x, y, size);
  if (err == 1)
  {
    //TODO: Present Error Message
    return -1;
  }

  //  float xMax = std::numeric_limits<float >::min();
  //  float yMax = std::numeric_limits<float >::min();
  for (int i = 0; i < size; ++i)
  {
    //   qDebug() << x[i] << "  " << y[i] << "\n";
    if (x[i] > xMax)
    {
      xMax = x[i];
    }
    if (y[i] > yMax)
    {
      yMax = y[i];
    }
  }

  xCo.clear();
  yCo.clear();
  int numsizebins = 1;
  binsizes.clear();
  // QwtArray<int> numfeatures;
  err = StatsGen::GenCutOff<float, QwtArray<float> > (mu, sigma, minCutOff, maxCutOff, binStepSize, xCo, yCo, yMax, numsizebins, binsizes);

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::updateSizeDistributionPlot()
{
  float mu = 1.0;
  float sigma = 1.0;
  float minCutOff = 1.0;
  float maxCutOff = 1.0;
  float stepSize = 1.0;
  int err = gatherSizeDistributionFromGui(mu, sigma, minCutOff, maxCutOff, stepSize);
  if (err == 0)
  {
    return;
  }

  QwtArray<float> xCo;
  QwtArray<float> yCo;
  QwtArray<float> binsizes;
  float xMax = std::numeric_limits<float >::min();
  float yMax = std::numeric_limits<float >::min();
  QwtArray<float> x;
  QwtArray<float> y;
  err = computeBinsAndCutOffs(mu, sigma, minCutOff, maxCutOff, stepSize, binsizes, xCo, yCo, xMax, yMax, x, y);
  if (err < 0) { return; }

  if (NULL == m_SizeDistributionCurve)
  {
    m_SizeDistributionCurve = new QwtPlotCurve("Size Distribution");
    m_SizeDistributionCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_SizeDistributionCurve->setPen(QPen(QColor("IndianRed"), 2));
    m_SizeDistributionCurve->attach(m_SizeDistributionPlot);
  }

  if(nullptr == m_SizeDistCenterPointCurve)
  {
    m_SizeDistCenterPointCurve = new QwtPlotCurve("Size Distribution Bin Centers");
    m_SizeDistCenterPointCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_SizeDistCenterPointCurve->setPen(QPen(Qt::blue, 2));

    QColor color = QColor("IndianRed");
    m_SizeDistCenterPointCurve->setPen(color, 2);
    m_SizeDistCenterPointCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    m_SizeDistCenterPointCurve->setStyle(QwtPlotCurve::NoCurve);
    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
        QBrush( Qt::white ), QPen( color, 2 ), QSize( 6, 6 ) );
    m_SizeDistCenterPointCurve->setSymbol( symbol );

    m_SizeDistCenterPointCurve->attach(m_SizeDistributionPlot);
  }

  // Place a vertical Line on the plot where the Min and Max Cutoff values are
  if (NULL == m_CutOffMin)
  {
    m_CutOffMin = new QwtPlotMarker();
    m_CutOffMin->attach(m_SizeDistributionPlot);
  }
  m_CutOffMin->setLabel(QString::fromLatin1("Cut Off Min Feature Diameter"));
  m_CutOffMin->setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
  m_CutOffMin->setLabelOrientation(Qt::Vertical);
  m_CutOffMin->setLineStyle(QwtPlotMarker::VLine);
  m_CutOffMin->setLinePen(QPen(Qt::blue, 1, Qt::SolidLine));
  m_CutOffMin->setXValue(xCo[0]);

  if (NULL == m_CutOffMax)
  {
    m_CutOffMax = new QwtPlotMarker();
    m_CutOffMax->attach(m_SizeDistributionPlot);
  }
  m_CutOffMax->setLabel(QString::fromLatin1("Cut Off Max Feature Diameter"));
  m_CutOffMax->setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
  m_CutOffMax->setLabelOrientation(Qt::Vertical);
  m_CutOffMax->setLineStyle(QwtPlotMarker::VLine);
  m_CutOffMax->setLinePen(QPen(Qt::blue, 1, Qt::SolidLine));
  m_CutOffMax->setXValue(xCo[1]);

  QwtArray<double> xD(x.size());
  QwtArray<double> yD(x.size());
  for (int i = 0; i < x.size(); ++i)
  {
    xD[i] = static_cast<double>(x[i]);
    yD[i] = static_cast<double>(y[i]);
  }

#if QWT_VERSION >= 0x060000
  m_SizeDistributionCurve->setSamples(xD, yD);
#else
  m_SizeDistributionCurve->setData(xD, yD);
#endif

  m_SizeDistributionPlot->setAxisScale(QwtPlot::xBottom, xCo[0] - (xCo[0] * 0.1), xCo[1] * 1.10);
  m_SizeDistributionPlot->setAxisScale(QwtPlot::yLeft, 0.0, yMax);

  //float barWidth = ((xCo[1] - xCo[0])/binsizes.count() ) * 0.98;

  for(int i = 0; i < m_BarChartItems.count(); i++)
  {
    m_BarChartItems[i]->detach();
    delete m_BarChartItems[i];
  }

  m_BarChartItems.clear();

 // int step = static_cast<int>(y.count()/binsizes.count());
  int halfStep = static_cast<int>(y.count()/binsizes.count() * 0.5);
  //int currentStep = halfStep;

  int indexStart = 0;
  int indexMid = halfStep - 1;
  int indexEnd = halfStep * 2 - 1;

  QwtArray<double> xBinCenter(binsizes.count());
  QwtArray<double> yBinCenter(binsizes.count());

  qDebug();
  for(int i = 0; i < binsizes.count(); i++)
  {
    QwtPlotShapeItem *item = new QwtPlotShapeItem( "" );
    item->setItemAttribute( QwtPlotItem::Legend, false );
    item->setLegendMode( QwtPlotShapeItem::LegendShape );
    item->setLegendIconSize( QSize( 20, 20 ) );
    item->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    //qDebug() << indexStart << "\t" << indexMid << "\t" << indexEnd;
    float barWidth = x[indexEnd] - x[indexStart];
    xBinCenter[i] = xD[indexMid];
    yBinCenter[i] = yD[indexMid];

    QSizeF size(barWidth, y[indexMid]);
    QPointF pos(xBinCenter[i], y[indexMid]/2.0);


    indexStart = indexEnd + 1;
    indexMid = indexStart + halfStep - 1;
    indexEnd = indexStart + 2 * halfStep - 1;
    if(indexEnd >= x.size())
    {
      indexEnd = x.size() - 1;
    }

    QRectF rect;
    rect.setSize(size);
    rect.moveCenter(pos);


    QPainterPath path;
    path.addRect(rect);

    QColor fillColor = QColor("RoyalBlue");
    fillColor.setAlpha( 200 );

    QPen pen( Qt::black, 1 );
    pen.setJoinStyle( Qt::MiterJoin );
    item->setPen( pen );
    item->setBrush( fillColor );

    item->setShape( path );
    item->attach(m_SizeDistributionPlot);
    m_BarChartItems.push_back(item);
  }

  m_SizeDistCenterPointCurve->setSamples(xBinCenter, yBinCenter);

  m_SizeDistributionPlot->replot();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::plotSizeDistribution()
{
  float mu = 1.0;
  float sigma = 1.0;
  float minCutOff = 1.0;
  float maxCutOff = 1.0;
  float stepSize = 1.0;
  int err = gatherSizeDistributionFromGui(mu, sigma, minCutOff, maxCutOff, stepSize);
  if (err == 0) { return; }

  QwtArray<float> xCo;
  QwtArray<float> yCo;
  QwtArray<float> binSizes;
  float xMax = std::numeric_limits<float >::min();
  float yMax = std::numeric_limits<float >::min();
  QwtArray<float> x;
  QwtArray<float> y;
  err = computeBinsAndCutOffs(mu, sigma, minCutOff, maxCutOff, stepSize, binSizes, xCo, yCo, xMax, yMax, x, y);
  if (err < 0) { return; }

  m_Mu = mu;
  m_Sigma = sigma;
  m_MaxCutOff = maxCutOff;
  m_MinCutOff = minCutOff;
  m_BinStep = stepSize;
  m_BinSizes = binSizes;

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int StatsGenFeatureSizeWidget::getStatisticsData(PrimaryStatsData* primaryStatsData)
{

  float mu = 1.0f;
  float sigma = 1.0f;
  float minCutOff = 1.0f;
  float maxCutOff = 1.0f;
  float binStep = 1.0f;
  gatherSizeDistributionFromGui(mu, sigma, minCutOff, maxCutOff, binStep);

  QwtArray<float> xCo;
  QwtArray<float> yCo;
  QwtArray<float> binsizes;
  float xMax = std::numeric_limits<float>::min();
  float yMax = std::numeric_limits<float>::min();
  QwtArray<float> x;
  QwtArray<float> y;
  int err = computeBinsAndCutOffs(mu, sigma, minCutOff, maxCutOff, binStep, binsizes, xCo, yCo, xMax, yMax, x, y);
  if (err < 0)
  {
    return err;
  }

  // We need to compute the Max and Min Diameter Bin Values
  float mindiameter = xCo[0];
  float maxdiameter = xCo[1];
  float avglogdiam = mu;
  float sdlogdiam = sigma;
  float stepSize = binStep;

  // Feature Diameter Info
  primaryStatsData->setBinStepSize(stepSize);
  primaryStatsData->setMaxFeatureDiameter(maxdiameter);
  primaryStatsData->setMinFeatureDiameter(mindiameter);
  // Feature Size Distribution
  {
    VectorOfFloatArray data;
    FloatArrayType::Pointer d1 = FloatArrayType::CreateArray(1, SIMPL::StringConstants::Average);
    FloatArrayType::Pointer d2 = FloatArrayType::CreateArray(1, SIMPL::StringConstants::StandardDeviation);
    data.push_back(d1);
    data.push_back(d2);
    d1->setValue(0, avglogdiam);
    d2->setValue(0, sdlogdiam);
    primaryStatsData->setFeatureSizeDistribution(data);
    primaryStatsData->setFeatureSize_DistType(SIMPL::DistributionType::LogNormal);
  }


  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenFeatureSizeWidget::extractStatsData(PrimaryStatsData* primaryStatsData, int index)
{
  float binStepSize = 0.0f;
  float maxFeatureSize = 0.0f, minFeatureSize = 0.0f;
  float mu = 0.0f;
  float sigma = 0.0f;
  float minCutOff = 0.0f;
  float maxCutOff = 0.0f;

  /* Set the Feature_Diameter_Info Data */
  binStepSize = primaryStatsData->getBinStepSize();
  m_BinStepSize->blockSignals(true);
  m_BinStepSize->setValue(binStepSize);
  m_BinStepSize->blockSignals(false);
  maxFeatureSize = primaryStatsData->getMaxFeatureDiameter();
  minFeatureSize = primaryStatsData->getMinFeatureDiameter();

  /* Set the Feature_Size_Distribution Data */
  VectorOfFloatArray distData = primaryStatsData->getFeatureSizeDistribution();
  mu = distData[0]->getValue(0);
  sigma = distData[1]->getValue(0);
  m_Mu_SizeDistribution->blockSignals(true);
  m_Sigma_SizeDistribution->blockSignals(true);

  m_Mu_SizeDistribution->setText(QString::number(mu));
  m_Sigma_SizeDistribution->setText(QString::number(sigma));

  m_Mu_SizeDistribution->blockSignals(false);
  m_Sigma_SizeDistribution->blockSignals(false);

  minCutOff = (mu - log(minFeatureSize)) / sigma;
  maxCutOff = (log(maxFeatureSize) - mu) / sigma;

  m_MinSigmaCutOff->blockSignals(true);
  m_MinSigmaCutOff->setText(QString::number(minCutOff));
  m_MinSigmaCutOff->blockSignals(false);

  m_MaxSigmaCutOff->blockSignals(true);
  m_MaxSigmaCutOff->setText(QString::number(maxCutOff));
  m_MaxSigmaCutOff->blockSignals(false);

  // Update the Size/Weights Plot
  updateSizeDistributionPlot();
//  m_NumberBinsGenerated->setText(QString::number(bins->getNumberOfTuples()));
  m_NumberBinsGenerated->setText(QString("!! LINE 635 !!"));
}
