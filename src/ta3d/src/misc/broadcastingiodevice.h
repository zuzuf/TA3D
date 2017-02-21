/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2017  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/
#ifndef __TA3D_XX__MISC_BROADCASTINGIODEVICE_H__
# define __TA3D_XX__MISC_BROADCASTINGIODEVICE_H__

#include <QIODevice>
#include <QList>

namespace TA3D
{

    class BroadCastingIODevice : public QIODevice
    {
        Q_OBJECT
    public:
        BroadCastingIODevice(QObject *parent = nullptr);

        const QList<QIODevice*> &devices() const;

        virtual bool isSequential() const;

        void addSink(QIODevice *dev);
        void deleteSink(QIODevice *dev);
    protected:
        virtual qint64 readData(char *data, qint64 maxlen);
        virtual qint64 writeData(const char *data, qint64 len);
    private:
        QList<QIODevice*> _devices;
    };

} // namespace TA3D


#endif // __TA3D_XX__MISC_BROADCASTINGIODEVICE_H__
