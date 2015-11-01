#pragma once
#include <Plugin/Commands/DeviceExplorerCommandFactory.hpp>

#include <iscore/command/AggregateCommand.hpp>

class RemoveNodes : public iscore::AggregateCommand
{
        ISCORE_AGGREGATE_COMMAND_DECL(DeviceExplorerCommandFactoryName(), RemoveNodes, "RemoveNodes")
};
