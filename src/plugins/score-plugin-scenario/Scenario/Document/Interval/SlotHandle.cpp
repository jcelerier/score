// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "SlotPresenter.hpp"

#include <Process/LayerPresenter.hpp>
#include <Process/LayerView.hpp>

#include <score/graphics/GraphicsItem.hpp>
#include <ossia/detail/algorithms.hpp>

#include <QGraphicsScene>
#include <QPointer>

#include <Scenario/Document/Interval/LayerData.hpp>

namespace Scenario
{

void LayerSlotPresenter::cleanupHeaderFooter()
{
  if (headerDelegate)
  {
    deleteGraphicsItem(headerDelegate);
    headerDelegate = nullptr;
  }
  if (footerDelegate)
  {
    deleteGraphicsItem(footerDelegate);
    footerDelegate = nullptr;
  }
}

void LayerSlotPresenter::cleanup(QGraphicsScene* sc)
{
  if (sc)
  {
    if (header)
    {
      sc->removeItem(header);
      delete header;
      header = nullptr;
    }
    if (footer)
    {
      sc->removeItem(footer);
      delete footer;
      footer = nullptr;
    }
  }
  else
  {
    delete header;
    header = nullptr;
    delete footer;
    footer = nullptr;
  }

  for (LayerData& ld : layers)
  {
    ld.cleanup();
  }
  layers.clear();
}

void LayerSlotPresenter::putToFront(const Id<Process::ProcessModel>& id)
{
  auto it = ossia::find_if(this->layers, [&] (const LayerData& l) { return l.mainPresenter()->model().id() == id; });
  if(it != this->layers.end())
  {
    std::iter_swap(it, this->layers.begin());
  }
}
}
