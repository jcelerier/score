#include <QQuickPaintedItem>

#include "TriggerModel.hpp"
#include "TriggerPresenter.hpp"
#include "TriggerView.hpp"
#include <iscore/tools/Todo.hpp>

namespace Scenario
{
TriggerPresenter::TriggerPresenter(
    const TriggerModel& model, QQuickPaintedItem* parentView, QObject* parent)
    : QObject{parent}, m_model{model}, m_view{new TriggerView{parentView}}
{
  m_view->setVisible(m_model.active());
  m_view->setPosition(QPointF(-7.5, -25.));
  con(m_model, &TriggerModel::activeChanged, this, [&]() {
    m_view->setVisible(m_model.active());
    //m_view->setToolTip(m_model.expression().toString());
  });

  //m_view->setToolTip(m_model.expression().toString());
  con(m_model, &TriggerModel::triggerChanged, this,
      [&](const State::Expression& t) {
  //  m_view->setToolTip(t.toString());
  });

  connect(
      m_view, &TriggerView::pressed, &m_model, &TriggerModel::triggeredByGui);
}
}
