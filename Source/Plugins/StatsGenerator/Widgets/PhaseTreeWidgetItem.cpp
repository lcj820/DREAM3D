#include <PhaseTreeWidgetItem.h>

#include <QtWidgets/QTreeWidget>


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PhaseTreeWidgetItem::PhaseTreeWidgetItem(QTreeWidget *parent) :
  QTreeWidgetItem(parent, QTreeWidgetItem::UserType + 1),
  m_StatsWidget(nullptr)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PhaseTreeWidgetItem::PhaseTreeWidgetItem(QTreeWidgetItem *parent) :
  QTreeWidgetItem(parent, QTreeWidgetItem::UserType + 1),
  m_StatsWidget(nullptr)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PhaseTreeWidgetItem::~PhaseTreeWidgetItem()
{

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PhaseTreeWidgetItem::setStatsWidget(QWidget* statsWidget)
{
  m_StatsWidget = statsWidget;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QWidget* PhaseTreeWidgetItem::getStatsWidget()
{
  return m_StatsWidget;
}
