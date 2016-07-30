#include <QString>
#include <QVariant>
#include <memory>

#include <Device/Protocol/DeviceSettings.hpp>
#include <ossia/network/generic/generic_device.hpp>
#include <ossia/network/generic/generic_address.hpp>
#include <ossia/network/minuit/minuit.hpp>
#include "MinuitDevice.hpp"
#include <OSSIA/Protocols/Minuit/MinuitSpecificSettings.hpp>

namespace Ossia
{
namespace Protocols
{
MinuitDevice::MinuitDevice(const Device::DeviceSettings &settings):
    OwningOSSIADevice{settings}
{
    m_capas.canRefreshTree = true;

    reconnect();
}

bool MinuitDevice::reconnect()
{
    disconnect();

    try {
        auto stgs = settings().deviceSpecificSettings.value<MinuitSpecificSettings>();

        std::unique_ptr<ossia::net::protocol> ossia_settings = std::make_unique<impl::Minuit2>(
                    stgs.host.toStdString(),
                    stgs.inputPort,
                    stgs.outputPort);

        m_dev = std::make_unique<impl::BasicDevice>(
              std::move(ossia_settings),
              settings().name.toStdString());

        setLogging_impl(isLogging());
    }
    catch(std::exception& e)
    {
        qDebug() << "Could not connect: " << e.what();
    }
    catch(...)
    {
        // TODO save the reason of the non-connection.
    }

    return connected();
}
}
}
