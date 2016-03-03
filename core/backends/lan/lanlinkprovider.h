/**
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LANLINKPROVIDER_H
#define LANLINKPROVIDER_H

#include <QObject>
#include <QTcpServer>
#include <QSslSocket>
#include <QUdpSocket>
#include <QtNetwork/qsslsocket.h>

#include "../linkprovider.h"
#include "server.h"
#include "landevicelink.h"

class LanPairingHandler;
class LanLinkProvider
    : public LinkProvider
{
    Q_OBJECT

public:
    LanLinkProvider(bool testMode = false);
    ~LanLinkProvider();

    QString name() override { return "LanLinkProvider"; }
    int priority() override { return PRIORITY_HIGH; }

    void userRequestsPair(const QString &deviceId);
    void userRequestsUnpair(const QString &deviceId);
    void incomingPairPackage(DeviceLink* device, const NetworkPackage& np);

public Q_SLOTS:
    virtual void onNetworkChange() override;
    virtual void onStart() override;
    virtual void onStop() override;
    void connected();
    void encrypted();
    void connectError();

private Q_SLOTS:
    void newUdpConnection();
    void newConnection();
    void dataReceived();
    void deviceLinkDestroyed(QObject* destroyedDeviceLink);
    void sslErrors(const QList<QSslError>& errors);

private:
    static void configureSocket(QSslSocket* socket);
    LanPairingHandler* createPairingHandler(DeviceLink* link);

    void onNetworkConfigurationChanged(const QNetworkConfiguration &config);
    void addLink(const QString& deviceId, QSslSocket* socket, NetworkPackage* receivedPackage, LanDeviceLink::ConnectionStarted connectionOrigin);

    Server* mServer;
    QUdpSocket* mUdpServer;
    QUdpSocket mUdpSocket;
    const static quint16 port = 1714;
    quint16 mTcpPort;

    QMap<QString, LanDeviceLink*> mLinks;
    QMap<QString, LanPairingHandler*> mPairingHandlers;

    struct PendingConnect {
        NetworkPackage* np;
        QHostAddress sender;
    };
    QMap<QSslSocket*, PendingConnect> receivedIdentityPackages;
    QNetworkConfiguration m_lastConfig;
    const bool mTestMode;
};

#endif
