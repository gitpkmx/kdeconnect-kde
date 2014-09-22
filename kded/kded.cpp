/**
 * Copyright 2014 Yuri Samoilenko <kinnalru@gmail.com>
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

#include "kded.h"

#include <QTimer>
#include <QDebug>

#include <KPluginFactory>

#include "config-kded.h"

K_PLUGIN_FACTORY(KdeConnectFactory, registerPlugin<Kded>();)

Q_LOGGING_CATEGORY(KDECONNECT_KDED, "kdeconnect.kded")

Kded::Kded(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , m_daemon(0)
{
    QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
    qDebug(KDECONNECT_KDED) << "kded_kdeconnect started";
}

Kded::~Kded()
{
    stop();
    qDebug(KDECONNECT_KDED) << "kded_kdeconnect stopped";
}

void Kded::start()
{
    if (m_daemon) {
        return;
    }

    const QString daemon = QStringLiteral(KDECONNECTD_BIN);
    qDebug(KDECONNECT_KDED) << "Starting daemon " << daemon;
    m_daemon = new QProcess(this);
    connect(m_daemon, SIGNAL(started()), SLOT(daemonStarted()));
    connect(m_daemon, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
    connect(m_daemon, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished(int,QProcess::ExitStatus)));
    connect(m_daemon, SIGNAL(finished(int,QProcess::ExitStatus)), m_daemon, SLOT(deleteLater()));

    m_daemon->setProgram(daemon);
    m_daemon->closeReadChannel(QProcess::StandardOutput);
    m_daemon->start();
}

void Kded::stop()
{
    if (!m_daemon) {
        return;
    }

    m_daemon->terminate();
    m_daemon->setProperty("terminate", true);
    QTimer::singleShot(10000, this, SLOT(checkIfDaemonTerminated()));
}

void Kded::restart()
{
    stop();
    return start();
}

void Kded::onError(QProcess::ProcessError errorCode)
{
    qCWarning(KDECONNECT_KDED) << "Process error code=" << errorCode;
}

void Kded::daemonStarted()
{
    qDebug(KDECONNECT_KDED) << "Daemon successfuly started";
    Q_EMIT started();
}

void Kded::onFinished(int exitCode, QProcess::ExitStatus status)
{
    if (status == QProcess::CrashExit) {
        qCWarning(KDECONNECT_KDED) << "Process crashed with code=" << exitCode;
        qCWarning(KDECONNECT_KDED) << m_daemon->readAllStandardError();
        qCWarning(KDECONNECT_KDED) << "Restarting in 5 sec...";
        QTimer::singleShot(5000, this, SLOT(start()));        
    } else {
        qCWarning(KDECONNECT_KDED) << "Process finished with code=" << exitCode;
    }

    Q_EMIT stopped();
    m_daemon = 0;
}

void Kded::checkIfDaemonTerminated()
{
    if (!m_daemon || !m_daemon->property("terminate").isValid()) {
        return;
    }

    m_daemon->kill();
    qCWarning(KDECONNECT_KDED) << "Daemon  killed";
}

#include "kded.moc"
