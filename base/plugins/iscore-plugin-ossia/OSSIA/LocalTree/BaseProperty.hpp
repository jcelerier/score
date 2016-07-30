#pragma once
#include <ossia/network/base/address.hpp>
#include <ossia/network/base/node.hpp>
#include <iscore_plugin_ossia_export.h>

namespace Ossia
{
namespace LocalTree
{
class ISCORE_PLUGIN_OSSIA_EXPORT BaseProperty
{
    public:
        ossia::net::node& node;
        ossia::net::address& addr;

        BaseProperty(
                ossia::net::node& n,
                ossia::net::address& a):
            node{n},
            addr{a}
        {

        }

        virtual ~BaseProperty();
};
}
}
