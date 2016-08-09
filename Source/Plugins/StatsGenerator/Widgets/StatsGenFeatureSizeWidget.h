#ifndef _statsGenFeatureSizeWidget_H_
#define _statsGenFeatureSizeWidget_H_

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QEvent>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataContainers/AttributeMatrix.h"

#include "StatsGenerator/Widgets/SGWidget.h"
#include "StatsGenerator/ui_StatsGenFeatureSizeWidget.h"


#ifndef QwtArray
#define QwtArray QVector
#endif


class QwtPlotGrid;
class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotShapeItem;
class QDoubleValidator;
class QTreeWidget;
class QLineEdit;
class StatsGenFeatureSizeWidget;
class StatsGenPlotWidget;
class StatsGenODFWidget;
class PrimaryStatsData;



class StatsGenFeatureSizeWidget : public QWidget, private Ui::StatsGenFeatureSizeWidget
{
    Q_OBJECT
  public:
    StatsGenFeatureSizeWidget(QWidget* parent = 0);
    virtual ~StatsGenFeatureSizeWidget();


    SIMPL_INSTANCE_PROPERTY(float, Mu)
    SIMPL_INSTANCE_PROPERTY(float, Sigma)
    SIMPL_INSTANCE_PROPERTY(float, MinCutOff)
    SIMPL_INSTANCE_PROPERTY(float, MaxCutOff)
    SIMPL_INSTANCE_PROPERTY(float, BinStep)

    SIMPL_INSTANCE_PROPERTY(int, PhaseIndex)
    SIMPL_INSTANCE_PROPERTY(unsigned int, CrystalStructure)

    SIMPL_INSTANCE_PROPERTY(QwtArray<float>, BinSizes);

    void plotSizeDistribution();
    void updateSizeDistributionPlot();
    int computeBinsAndCutOffs( float mu, float sigma,
                               float minCutOff, float maxCutOff,
                               float binStepSize,
                               QwtArray<float>& binsizes,
                               QwtArray<float>& xCo,
                               QwtArray<float>& yCo,
                               float& xMax, float& yMax,
                               QwtArray<float>& x,
                               QwtArray<float>& y);
    int calculateNumberOfBins();
    int calculateNumberOfBins(float mu, float sigma, float minCutOff, float maxCutOff, float stepSize);
    int gatherSizeDistributionFromGui(float& mu, float& sigma, float& minCutOff, float& maxCutOff, float& stepSize);


    virtual int getStatisticsData(PrimaryStatsData* primaryStatsData);

    virtual void extractStatsData(PrimaryStatsData* primaryStatsData, int index);


  signals:

    void phaseParametersChanged();

    void binSelected(int index);

  protected:

      /**
       * @brief setupGui
       */
      void setupGui();

    /**
     * @brief validateValue
     * @param val
     * @param lineEdit
     * @return
     */
    bool validateValue(QDoubleValidator* val, QLineEdit* lineEdit);

    /**
     * @brief validateMuSigma
     * @return
     */
    bool validateMuSigma();

    void mousePressEvent(QMouseEvent* event);


  protected slots:

    void on_m_Mu_SizeDistribution_textChanged(const QString& text);
    void on_m_Sigma_SizeDistribution_textChanged(const QString& text);
    void on_m_MinSigmaCutOff_textChanged(const QString& text);
    void on_m_MaxSigmaCutOff_textChanged(const QString& text);
    void on_m_BinStepSize_valueChanged(double v);

    void plotPointSelected(const QPointF& point);

private:
      QwtPlotCurve*        m_SizeDistributionCurve;
      QwtPlotCurve*        m_SizeDistCenterPointCurve;
      QwtPlotMarker*       m_CutOffMin;
      QwtPlotMarker*       m_CutOffMax;
      QwtPlotGrid*         m_grid;
      QVector<QwtPlotShapeItem*> m_BarChartItems;

      QDoubleValidator* m_MuValidator;
      QDoubleValidator* m_SigmaValidator;

};

#endif /* _statsGenFeatureSizeWidget_H_ */
