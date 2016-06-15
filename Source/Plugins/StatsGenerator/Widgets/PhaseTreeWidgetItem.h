#ifndef _phaseTreeWidgetItem_H_
#define _phaseTreeWidgetItem_H_

#include <QtWidgets/QTreeWidgetItem>

class QTreeWidget;


class PhaseTreeWidgetItem : public QTreeWidgetItem
{

  public:
    PhaseTreeWidgetItem(QTreeWidget *parent = nullptr);
    PhaseTreeWidgetItem(QTreeWidgetItem* parent = nullptr);
    virtual ~PhaseTreeWidgetItem();

    void setStatsWidget(QWidget* statsWidget);
    QWidget* getStatsWidget();

private:
    QWidget* m_StatsWidget;


    PhaseTreeWidgetItem(const PhaseTreeWidgetItem&); // Copy Constructor Not Implemented
    void operator=(const PhaseTreeWidgetItem&); // Operator '=' Not Implemented

};

#endif /* _phaseTreeWidgetItem_H_ */
