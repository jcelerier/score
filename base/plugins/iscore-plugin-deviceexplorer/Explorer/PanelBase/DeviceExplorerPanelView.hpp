#pragma once
#include <iscore/plugins/panel/PanelView.hpp>
#include <core/application/ApplicationContext.hpp>
namespace iscore
{
    class View;
}
class DeviceExplorerWidget;
class DeviceExplorerPanelView final : public iscore::PanelView
{
        friend class DeviceExplorerPanelPresenter;
    public:
        const iscore::DefaultPanelStatus& defaultPanelStatus() const override;
        explicit DeviceExplorerPanelView(
                const iscore::ApplicationContext& ctx,
                iscore::View* parent);

        QWidget* getWidget() override;
        const QString shortcut() const override
        { return tr("Ctrl+E"); }

    private:
        DeviceExplorerWidget* m_widget {};
};
