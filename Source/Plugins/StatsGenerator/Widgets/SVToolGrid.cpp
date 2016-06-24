
#include "SVToolGrid.h"


#include <QtWidgets/QToolButton>
#include <QtWidgets/QAction>
#include <QtGui/QActionEvent>






#include "moc_SVToolGrid.cpp"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVToolGrid::SVToolGrid(QWidget* parent) :
  QFrame (parent),
  m_ButtonSize(QSize(64, 80)),
  m_IconSize(QSize(64, 64)),
  m_Columns(1),
  m_ToolButtonStyle(Qt::ToolButtonTextUnderIcon)
{
  setupUi(this);
  setupGui();

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVToolGrid::~SVToolGrid()
{

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVToolGrid::setupGui()
{


}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVToolGrid::setButtonSize(QSize size)
{
  if (m_ButtonSize != size) {
    m_ButtonSize = size;

    foreach(QToolButton* slot, m_GridSlots)
    {
      slot->setFixedSize(size);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSize SVToolGrid::getButtonSize()
{
  return m_ButtonSize;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVToolGrid::setIconSize(QSize size)
{
  if (m_IconSize != size) {
    m_IconSize = size;

    foreach(QToolButton* slot, m_GridSlots)
    {
      //slot->icon().setFixedSize(size);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSize SVToolGrid::getIconSize()
{
  return m_IconSize;
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void  SVToolGrid::setToolButtonStyle(Qt::ToolButtonStyle style)
{
  /*
    Set the tool button style.
    */
  if ( m_ToolButtonStyle != style )
  {
    m_ToolButtonStyle = style;
    foreach(QToolButton* slot, m_GridSlots) {
      slot->setToolButtonStyle(style);
    }
  }
}

/*
     Return the tool button style.
 */
Qt::ToolButtonStyle SVToolGrid::getToolButtonStyle()
{
  return m_ToolButtonStyle;
}

/*
     Set the number of button/action columns.
 */
void SVToolGrid::setColumnCount(int columns)
{
  if (m_Columns != columns) {
    m_Columns = columns;
    relayout();
  }
}

/*
     Return the number of columns in the grid.
 */
int SVToolGrid::columns()
{

  return m_Columns;
}

/*
     Clear all actions/buttons.
 */
void SVToolGrid::clear()
{

  //TODO: This was a Reversed list
  foreach(QToolButton* slot, m_GridSlots) {
    removeAction(slot->actions().first());
  }
  m_GridSlots.clear();
}

/*
 Insert a new action at the position currently occupied
 by `before` (can also be an index).

 Parameters
 ----------
 before : :class:`QAction` or int
     Position where the `action` should be inserted.
 action : :class:`QAction`
     Action to insert

 */
#if 0
void SVToolGrid::insertAction(QAction* before, QAction* action)
{

  //  if (before)
  //  {
  //    QList<QAction*> actions = list(actions());
  //    if (actions.size()) == 0 or before >= len(actions):
  //  // Insert as the first action or the last action.
  //      return self.addAction(action)

  //    before = actions[before]
  //  }
  return QFrame::insertAction(before, action);

}


/*
     Clear the grid and add `actions`.
 */
void SVToolGrid::setActions(QList<QAction*> actions)
{
  clear();

  foreach(QAction* action, actions) {
    addAction(action);
  }

}

/*
     Return the :class:`QToolButton` instance button for `action`.
 */
QToolButton* SVToolGrid::buttonForAction(QAction* action)
{
  QList<QAction*> storedActions = actions();
  foreach(QToolButton* slot, m_GridSlots)
  {
    QAction* buttonAction = slot->actions().first();
    int index = storedActions.indexOf(buttonAction);
    if(index >= 0)
    {
      return m_GridSlots[index];
    }
  }
  return nullptr;

}

/*
     Create and return a :class:`QToolButton` for action.
 */
QToolButton* SVToolGrid::createButtonForAction(QAction* action)
{

  QToolButton* button = new QToolButton(this);
  button->setDefaultAction(action);
  button->setCheckable(true);
  if (getButtonSize().isValid() )
    button->setFixedSize(m_ButtonSize);
  if (getIconSize().isValid() )
    button->setIconSize(m_IconSize);

  button->setToolButtonStyle(m_ToolButtonStyle);
  button->setProperty("tool-grid-button", true);
  return button;
}
#endif

/*
     Return the number of buttons/actions in the grid.
 */
int SVToolGrid::count()
{
  return m_GridSlots.size();
}
#if 0

void SVToolGrid::actionEvent(QActionEvent* event)
{
  QFrame::actionEvent(event);

  if (event->type() == QEvent::ActionAdded) {
    //# Note: the action is already in the self.actions() list.
    QList<QAction*> acts = actions();
    int index = acts.indexOf(event->action());
    insertActionButton(index, QIcon(), event->action());
  }
  else if(event->type() == QEvent::ActionRemoved) {
    removeActionButton(event->action());
  }
}
#endif

/* Create a button for the action and add it to the layout at index.
*/
void SVToolGrid::insertActionButton(int index, QIcon icon, QAction* action)
{

  //shiftGrid(index, 1);
  //QToolButton* button = createButtonForAction(action);
  action->setData(QVariant(index));

  QToolButton* toolButton = new QToolButton(this);
  toolButton->setObjectName(QStringLiteral("toolButton"));
  toolButton->setMinimumSize(m_IconSize);
  toolButton->setMouseTracking(false);
  toolButton->setAutoFillBackground(false);
  toolButton->setAutoExclusive(true);
  toolButton->setStyleSheet(QLatin1String("QToolButton:hover\n"
"{\n"
"	background-color: qlineargradient(spread:pad, x1:1, y1:1, x2:0, y2:1, stop:0 rgba(200, 200, 200, 255), stop:1 rgba(230, 230, 230, 255));\n"
"}\n"
""));

  toolButton->setToolButtonStyle(m_ToolButtonStyle);
  toolButton->setProperty("tool-grid-button", true);

  toolButton->setText(action->text());
  toolButton->setIcon(icon);

  if (getIconSize().isValid() )
    toolButton->setIconSize(m_IconSize);

  if (getButtonSize().isValid() )
    toolButton->setFixedSize(m_ButtonSize);

  toolButton->setCheckable(true);

  toolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  toolButton->setAutoRaise(true);
  toolButton->setArrowType(Qt::NoArrow);

  int row = index / m_Columns;
  int column = index % m_Columns;

  m_GridLayout->addWidget( toolButton, row, column, Qt::AlignCenter | Qt::AlignTop);

  m_GridSlots.insert(index, toolButton);

  connect(toolButton, SIGNAL(toggled(bool)),
          this, SLOT(buttonTriggered(bool)));
  if(index == 0) { toolButton->setChecked(true); }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVToolGrid::buttonTriggered(bool b)
{
  for(int i = 0; i < m_GridSlots.size(); i++)
  {
    if(m_GridSlots[i]->isChecked())
    {
      emit currentSelectionChanged(i);
    }
  }
}



/*
 *  Remove the button for the action from the layout and delete it.
*/
void SVToolGrid::removeActionButton(QAction* action)
{
  QList<QAction*> storedActions = actions();

  foreach(QToolButton* slot, m_GridSlots)
  {
    QAction* slotAction = slot->actions().first();
    int index = storedActions.indexOf(action);
    if(index >= 0)
    {
      QToolButton* button = m_GridSlots[index];
      m_GridSlots.removeAt(index);
      int gridIndex = m_GridLayout->indexOf(button);
      m_GridLayout->takeAt(gridIndex);
      button->deleteLater();
    }
  }
}

/*
 * Shift all buttons starting at index `start` by `count` cells.
*/
void SVToolGrid::shiftGrid( int start, int count)
{

  int button_count = m_GridLayout->count();
  int direction = 1;
  int end = 0;
  if(count < 0) { direction = -1; }
  if (direction == 1) {
    start = button_count - 1;
    end = start - 1;
  }
  else {
    start = start;
    end = button_count;
  }

  for (int index = start; index <= end; index = index - direction)
  {
    QLayoutItem* item = m_GridLayout->itemAtPosition(index / m_Columns,
                                                     index % m_Columns);
    if (item)
    {
      // QWidget* button = item->widget();
      int new_index = index + count;
      m_GridLayout->addItem(item, new_index / m_Columns,
                            new_index % m_Columns,
                            Qt::AlignLeft | Qt::AlignTop);
    }

  }
}



void SVToolGrid::relayout()
{
  /*
     * Relayout the buttons.
    */

  //    for i in reversed(range(self.layout().count())):
  //        self.layout().takeAt(i)

  //    self.__gridSlots = [_ToolGridSlot(slot.button, slot.action,
  //                                      i / m_Columns,
  //                                      i % m_Columns)
  //                        for i, slot in enumerate(self.__gridSlots)]

  //    foreach(QToolButton* slot, m_GridSlots)
  //        self.layout().addWidget(slot.button, slot.row, slot.column,
  //                                Qt.AlignLeft | Qt.AlignTop)

}

int SVToolGrid::indexOf(QToolButton* button)
{
  /*Return the index of button widget.
    */
  //    buttons = [slot.button for slot in self.__gridSlots]
  //    return buttons.index(button)
}

void SVToolGrid::onButtonRightClick(QToolButton* button)
{
  //QFrame::onButtonRightClick(button);
}

void SVToolGrid::onButtonEnter(QToolButton* button)
{
  //    action = button->defaultAction();
  //    self.actionHovered.emit(action);
}

void SVToolGrid::onClicked(QAction* action)
{
  //actionTriggered.emit(action);
}

void SVToolGrid::paintEvent(QPaintEvent* event)
{
  //    return utils.StyledWidget_paintEvent(self, event);
}

bool SVToolGrid::eventFilter(QObject* obj, QEvent* event)
{
  //    etype = event.type()
  //    if etype == QEvent.KeyPress and obj.hasFocus():
  //        key = event.key()
  //        if key in [Qt.Key_Up, Qt.Key_Down, Qt.Key_Left, Qt.Key_Right]:
  //            if self.__focusMove(obj, key):
  //                event.accept()
  //                return True

  //    return QFrame::eventFilter(self, obj, event);

}

#if 0
#endif
