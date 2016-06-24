#ifndef _svtoolgrid_H_
#define _svtoolgrid_H_

#include <QtCore/QObject>
#include <QtWidgets/QFrame>



#include "ui_SVToolGrid.h"


class QToolButton;
class QAction;


class SVToolGrid : public QFrame, private Ui::SVToolGrid
{
    Q_OBJECT
  public:
    SVToolGrid(QWidget* parent = nullptr);
    virtual ~SVToolGrid();


    void setButtonSize(QSize size);
    QSize getButtonSize();

    void setIconSize(QSize size);
    QSize getIconSize();

    void  setToolButtonStyle(Qt::ToolButtonStyle style);
    Qt::ToolButtonStyle getToolButtonStyle();
    void setColumnCount(int columns);
    int columns();
    void clear();
    void insertAction(QAction* before, QAction* action);
    void setActions(QList<QAction*> actions);

    void insertActionButton(int index, QIcon icon, QAction* action);
    int count();

  signals:

    void currentSelectionChanged(int index);

  protected slots:

    void buttonTriggered(bool b);

  protected:

    void setupGui();

    QToolButton* buttonForAction(QAction* action);

    QToolButton* createButtonForAction(QAction* action);


  //  void actionEvent(QActionEvent *event);
    void removeActionButton(QAction* action);
    void shiftGrid( int start, int count=1);
    void relayout();
    int indexOf(QToolButton* button);
    void onButtonRightClick(QToolButton* button);
    void onButtonEnter(QToolButton* button);
    void onClicked(QAction* action);
    void paintEvent(QPaintEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);


  private:
    QSize m_ButtonSize;
    QSize m_IconSize;
    int m_Columns;
    Qt::ToolButtonStyle m_ToolButtonStyle; //Qt::ToolButtonTextUnderIcon
    QVector<QToolButton*>  m_GridSlots;


    SVToolGrid(const SVToolGrid&); // Copy Constructor Not Implemented
    void operator=(const SVToolGrid&); // Operator '=' Not Implemented

};

#endif /* _svtoolgrid_H_ */
