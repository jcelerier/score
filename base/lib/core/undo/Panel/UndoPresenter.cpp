#include "UndoPresenter.hpp"
#include "UndoView.hpp"
#include <iscore/plugins/panel/PanelModelInterface.hpp>
#include <core/document/DocumentPresenter.hpp>
#include <core/document/DocumentModel.hpp>

UndoPresenter::UndoPresenter(
        iscore::Presenter* parent_presenter,
        iscore::PanelViewInterface* view) :
    iscore::PanelPresenterInterface{parent_presenter, view}
{
}


QString UndoPresenter::modelObjectName() const
{
    return "UndoPanelModel";
}


void UndoPresenter::on_modelChanged()
{
    if (model())
    {
        auto doc = static_cast<iscore::Document *>(model()->parent()->parent());
        static_cast<UndoView *>(view())->setStack(&doc->commandStack());
    }
    else
    {
        static_cast<UndoView *>(view())->setStack(nullptr);
    }
}
