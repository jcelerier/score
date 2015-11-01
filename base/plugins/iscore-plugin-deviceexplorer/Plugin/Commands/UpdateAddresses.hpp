#pragma once
#include <Plugin/Commands/DeviceExplorerCommandFactory.hpp>
#include <iscore/command/SerializableCommand.hpp>
#include <iscore/tools/ModelPath.hpp>

#include "Plugin/Panel/DeviceExplorerModel.hpp"

#include <DeviceExplorer/Node/DeviceExplorerNode.hpp>


namespace DeviceExplorer
{
    namespace Command
    {
    // TODO Moveme
        class UpdateAddressesValues : public iscore::SerializableCommand
        {
            ISCORE_SERIALIZABLE_COMMAND_DECL(DeviceExplorerCommandFactoryName(), UpdateAddressesValues, "UpdateAddressesValues")
            public:
                UpdateAddressesValues(Path<DeviceExplorerModel>&& device_tree,
                              const QList<QPair<const iscore::Node*, iscore::Value>>& nodes);

                void undo() const override;
                void redo() const override;

            protected:
                virtual void serializeImpl(QDataStream&) const override;
                virtual void deserializeImpl(QDataStream&) override;

            private:
                Path<DeviceExplorerModel> m_deviceTree;

                QList<
                    QPair<
                        iscore::NodePath,
                        QPair< // First is old, second is new
                            iscore::Value,
                            iscore::Value
                        >
                    >
                > m_data;
        };
    }
}

