#include "broadcastingiodevice.h"

namespace TA3D
{
    BroadCastingIODevice::BroadCastingIODevice(QObject *parent)
    : QIODevice(parent)
    {

    }

    const QList<QIODevice*> &BroadCastingIODevice::devices() const
    {
        return _devices;
    }

    bool BroadCastingIODevice::isSequential() const
    {
        return true;
    }

    qint64 BroadCastingIODevice::readData(char *data, qint64 maxlen)
    {
        return 0;
    }

    qint64 BroadCastingIODevice::writeData(const char *data, qint64 len)
    {
        for(QIODevice *dev : _devices)
            dev->write(data, len);
        return len;
    }

    void BroadCastingIODevice::addSink(QIODevice *dev)
    {
        _devices.removeAll(dev);
        _devices.push_back(dev);
        dev->setParent(this);
    }

    void BroadCastingIODevice::deleteSink(QIODevice *dev)
    {
        _devices.removeAll(dev);
        dev->deleteLater();
    }
}
