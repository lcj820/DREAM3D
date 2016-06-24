/*
 * PrimaryPhaseWidget.h
 *
 *  Created on: Jun 14, 2012
 *      Author: mjackson
 */

#ifndef _primaryphasewidget_h_
#define _primaryphasewidget_h_

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QString>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataContainers/AttributeMatrix.h"

#include "StatsGenerator/Widgets/SGWidget.h"
#include "StatsGenerator/Widgets/Presets/AbstractMicrostructurePreset.h"
#include "StatsGenerator/ui_PrimaryPhaseWidget.h"


#ifndef QwtArray
#define QwtArray QVector
#endif


class QwtPlotGrid;
class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotShapeItem;
class QDoubleValidator;
class QTreeWidget;
class StatsGenFeatureSizeWidget;
class StatsGenPlotWidget;
class StatsGenODFWidget;
class SGAxisODFWidget;


/*
 * LaTeX to generate the equations
 x^{\left( \mu - max*\sigma \right) }

 */

class PrimaryPhaseWidget : public SGWidget, private Ui::PrimaryPhaseWidget
{
    Q_OBJECT

  public:
    PrimaryPhaseWidget(QWidget* parent = 0);
    virtual ~PrimaryPhaseWidget();

    void updatePlots();

    void setPhaseName(const QString &phaseName);

    SIMPL_INSTANCE_PROPERTY(unsigned int, PhaseType)
    void setCrystalStructure(unsigned int xtal);
    unsigned int getCrystalStructure() const;
    void setPhaseIndex(int index);
    int getPhaseIndex() const;
    SIMPL_INSTANCE_PROPERTY(float, PhaseFraction)
    SIMPL_INSTANCE_PROPERTY(float, TotalPhaseFraction)
    SIMPL_INSTANCE_PROPERTY(bool, DataHasBeenGenerated)
    SIMPL_INSTANCE_PROPERTY(bool, BulkLoadFailure)

    void extractStatsData(AttributeMatrix::Pointer attrMat, int index);

    QString getComboString();


    int gatherStatsData(AttributeMatrix::Pointer attrMat, bool preflight = false);
    virtual QIcon getPhaseIcon();

  public slots:

  protected slots:
    void on_m_GenerateDefaultData_clicked();
    void on_m_FeatureSizeDistBtn_toggled();
    void on_m_OdfBtn_toggled();
    void on_m_AxisOdfBtn_toggled();

    void on_microstructurePresetCombo_currentIndexChanged(int index);

    void dataWasEdited();

    void bulkLoadEvent(bool fail);

  protected:

    /**
      * @brief Enables or Disables all the widgets in a list
      * @param b
      */
    void setWidgetListEnabled(bool b);

    /**
     * @brief setupGui
     */
    void setupGui();


  private:
    int m_PhaseIndex;
    unsigned int m_CrystalStructure;

    QList<QWidget*>      m_WidgetList;

    AbstractMicrostructurePreset::Pointer m_MicroPreset;

    StatsGenPlotWidget* m_Omega3Plot;
    StatsGenPlotWidget* m_BOverAPlot;
    StatsGenPlotWidget* m_COverAPlot;
    StatsGenPlotWidget* m_NeighborPlot;
    StatsGenODFWidget* m_ODFWidget;
    SGAxisODFWidget* m_AxisODFWidget;

    PrimaryPhaseWidget(const PrimaryPhaseWidget&); // Copy Constructor Not Implemented
    void operator=(const PrimaryPhaseWidget&); // Operator '=' Not Implemented
};

#endif /* PRIMARYPHASEWIDGET_H_ */

