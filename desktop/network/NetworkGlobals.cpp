#include "NetworkGlobals.h"

Device getLocalHostName()
{
    Device device;
    QHostInfo hostInfo;
    device.name = hostInfo.localHostName();

    return device;
}
