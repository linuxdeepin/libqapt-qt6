/***************************************************************************
 *   Copyright © 2012 Jonathan Thomas <echidnaman@kubuntu.org>             *
 *   Copyright © 2008-2009 Sebastian Heinlein <devel@glatzor.de>           *
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

#include "transactionqueue.h"

// Qt includes
#include <QStringList>
#include <QTimer>
#include <QDebug>

// Own includes
#include "aptworker.h"
#include "transaction.h"

TransactionQueue::TransactionQueue(QObject *parent, AptWorker *worker)
    : QObject(parent)
    , m_worker(worker)
    , m_activeTransaction(nullptr)
{
    qDebug() << "TransactionQueue initialized";
}

QList<Transaction *> TransactionQueue::transactions() const
{
    return m_queue;
}

Transaction *TransactionQueue::activeTransaction() const
{
    return m_activeTransaction;
}

bool TransactionQueue::isEmpty() const
{
    return (m_queue.isEmpty() && m_pending.isEmpty());
}

Transaction *TransactionQueue::pendingTransactionById(const QString &id) const
{
    Transaction *transaction = nullptr;

    for (Transaction *trans : m_pending) {
        if (trans->transactionId() == id) {
            transaction = trans;
            break;
        }
    }

    return transaction;
}

Transaction *TransactionQueue::transactionById(const QString &id) const
{
    Transaction *transaction = nullptr;

    for (Transaction *trans : m_queue) {
        if (trans->transactionId() == id) {
            transaction = trans;
            break;
        }
    }

    return transaction;
}

void TransactionQueue::addPending(Transaction *trans)
{
    qDebug() << "Adding pending transaction:" << trans->transactionId();
    m_pending.append(trans);
    connect(trans, SIGNAL(idleTimeout(Transaction*)),
            this, SLOT(removePending(Transaction*)));
}

void TransactionQueue::removePending(Transaction *trans)
{
    qDebug() << "Removing pending transaction:" << trans->transactionId();
    m_pending.removeAll(trans);

    trans->deleteLater();
}

void TransactionQueue::enqueue(QString tid)
{
    qDebug() << "Enqueueing transaction:" << tid;
    Transaction *trans = pendingTransactionById(tid);

    if (!trans) {
        qWarning() << "Transaction not found:" << tid;
        return;
    }

    connect(trans, SIGNAL(finished(int)), this, SLOT(onTransactionFinished()));
    m_pending.removeAll(trans);
    m_queue.enqueue(trans);

    if (!m_worker->currentTransaction()) {
        qDebug() << "No active transaction, running next";
        runNextTransaction();
    } else {
        qDebug() << "Transaction queued, waiting for current to finish";
        trans->setStatus(QApt::WaitingStatus);
    }

    emitQueueChanged();
}

void TransactionQueue::remove(QString tid)
{
    qDebug() << "Removing transaction:" << tid;
    Transaction *trans = transactionById(tid);

    if (!trans) {
        qWarning() << "Transaction not found:" << tid;
        return;
    }

    m_queue.removeAll(trans);

    if (trans == m_activeTransaction) {
        qDebug() << "Active transaction removed:" << tid;
        m_activeTransaction = nullptr;
    }

    emitQueueChanged();

    // Wait in case clients are a bit slow.
    QTimer::singleShot(5000, trans, SLOT(deleteLater()));
}

void TransactionQueue::onTransactionFinished()
{
    Transaction *trans = qobject_cast<Transaction *>(sender());

    if (!trans) {
        qWarning() << "Invalid transaction finished signal";
        return;
    }

    qDebug() << "Transaction finished:" << trans->transactionId()
            << "Exit status:" << trans->exitStatus();

    // TODO: Transaction chaining

    remove(trans->transactionId());
    if (m_queue.count()) {
        qDebug() << "Running next transaction in queue";
        runNextTransaction();
    } else {
        qDebug() << "Transaction queue empty";
    }
    emitQueueChanged();
}

void TransactionQueue::runNextTransaction()
{
    m_activeTransaction = m_queue.head();
    qDebug() << "Starting transaction:" << m_activeTransaction->transactionId();

    QMetaObject::invokeMethod(m_worker, "runTransaction", Qt::QueuedConnection,
                              Q_ARG(Transaction *, m_activeTransaction));
}

void TransactionQueue::emitQueueChanged()
{
    QString tid;
    QStringList queued;

    if (m_activeTransaction)
        tid = m_activeTransaction->transactionId();

    for (Transaction *trans : m_queue)
        queued << trans->transactionId();

    emit queueChanged(tid, queued);
}
