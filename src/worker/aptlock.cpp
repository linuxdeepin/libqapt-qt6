/***************************************************************************
 *   Copyright © 2012 Jonathan Thomas <echidnaman@kubuntu.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "aptlock.h"

#include <apt-pkg/error.h>
#include <QDebug>

#include <unistd.h>

AptLock::AptLock(const QString &path)
    : m_path(path.toUtf8())
    , m_fd(-1)
{
    qDebug() << "AptLock initialized with path:" << path;
}

bool AptLock::isLocked() const
{
    bool locked = m_fd != -1;
    qDebug() << "AptLock::isLocked() returning:" << locked;
    return locked;
}

bool AptLock::acquire()
{
    qDebug() << "AptLock::acquire() called";
    if (isLocked()) {
        qDebug() << "Lock already acquired";
        return true;
    }

    std::string str = m_path.data();
    m_fd = GetLock(str + "lock");
    m_lock.Fd(m_fd);

    bool success = isLocked();
    qDebug() << "AptLock::acquire()" << (success ? "succeeded" : "failed");
    return success;
}

void AptLock::release()
{
    qDebug() << "AptLock::release() called";
    if (!isLocked()) {
        qDebug() << "No lock to release";
        return;
    }

    m_lock.Close();
    ::close(m_fd);
    m_fd = -1;
    qDebug() << "Lock released successfully";
}
