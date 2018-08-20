//
// Copyright (C) 2017 James Turner  zakalawe@mac.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "canvasconnection.h"

#include <QUrl>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDataStream>

#include "localprop.h"
#include "fgqcanvasfontcache.h"
#include "fgqcanvasimageloader.h"

CanvasConnection::CanvasConnection(QObject *parent) : QObject(parent)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &CanvasConnection::onWebSocketConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &CanvasConnection::onWebSocketClosed);
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &CanvasConnection::onTextMessageReceived);

    m_destRect = QRectF(50, 50, 400, 400);
}

CanvasConnection::~CanvasConnection()
{
    disconnect(&m_webSocket, &QWebSocket::disconnected,
               this, &CanvasConnection::onWebSocketClosed);
    m_webSocket.close();
}

void CanvasConnection::setNetworkAccess(QNetworkAccessManager *dl)
{
    m_netAccess = dl;
}

void CanvasConnection::setRootPropertyPath(QByteArray path)
{
    m_rootPropertyPath = path;
    emit rootPathChanged();
}

QJsonObject CanvasConnection::saveState() const
{
    QJsonObject json;
    json["url"] = m_webSocketUrl.toString();
    json["path"] = QString::fromUtf8(m_rootPropertyPath);
    json["rect"] = QJsonArray{m_destRect.x(), m_destRect.y(), m_destRect.width(), m_destRect.height()};
    return json;
}

bool CanvasConnection::restoreState(QJsonObject state)
{
    m_webSocketUrl = state.value("url").toString();
    m_rootPropertyPath = state.value("path").toString().toUtf8();
    QJsonArray rect = state.value("rect").toArray();
    if (rect.size() < 4) {
        return false;
    }

    m_destRect = QRectF(rect[0].toDouble(), rect[1].toDouble(),
            rect[2].toDouble(), rect[3].toDouble());

    emit geometryChanged();
    emit rootPathChanged();
    emit webSocketUrlChanged();

    return true;
}

void CanvasConnection::saveSnapshot(QDataStream &ds) const
{
    ds << m_webSocketUrl << m_rootPropertyPath << m_destRect;
    m_localPropertyRoot->saveToStream(ds);
}

void CanvasConnection::restoreSnapshot(QDataStream &ds)
{
    ds >> m_webSocketUrl >> m_rootPropertyPath >> m_destRect;
    m_localPropertyRoot.reset(LocalProp::restoreFromStream(ds, nullptr));
    setStatus(Snapshot);

    emit geometryChanged();
    emit rootPathChanged();
    emit webSocketUrlChanged();

    emit updated();
}

void CanvasConnection::reconnect()
{
    m_webSocket.open(m_webSocketUrl);
    setStatus(Connecting);
}

void CanvasConnection::showDebugTree()
{
    qWarning() << Q_FUNC_INFO << "implement me!";
}

void CanvasConnection::setOrigin(QPointF c)
{
    if (m_destRect.topLeft() == c)
        return;

    m_destRect.moveTopLeft(c);
    emit geometryChanged();
}

void CanvasConnection::setSize(QSizeF sz)
{
    if (size() == sz)
        return;

    m_destRect.setSize(sz);
    emit geometryChanged();
}

void CanvasConnection::connectWebSocket(QByteArray hostName, int port)
{
    QUrl wsUrl;
    wsUrl.setScheme("ws");
    wsUrl.setHost(hostName);
    wsUrl.setPort(port);
    wsUrl.setPath("/PropertyTreeMirror" + m_rootPropertyPath);

    m_webSocketUrl = wsUrl;
    emit webSocketUrlChanged();

    m_webSocket.open(wsUrl);
    setStatus(Connecting);
}

QPointF CanvasConnection::origin() const
{
    return m_destRect.topLeft();
}

QSizeF CanvasConnection::size() const
{
    return m_destRect.size();
}

LocalProp *CanvasConnection::propertyRoot() const
{
    return m_localPropertyRoot.get();
}

FGQCanvasImageLoader *CanvasConnection::imageLoader() const
{
    if (!m_imageLoader) {
        m_imageLoader = new FGQCanvasImageLoader(m_netAccess, const_cast<CanvasConnection*>(this));
        m_imageLoader->setHost(m_webSocketUrl.host(),
                               m_webSocketUrl.port());
    }

    return m_imageLoader;
}

FGQCanvasFontCache *CanvasConnection::fontCache() const
{
    if (!m_fontCache) {
        m_fontCache = new FGQCanvasFontCache(m_netAccess, const_cast<CanvasConnection*>(this));
        m_fontCache->setHost(m_webSocketUrl.host(),
                             m_webSocketUrl.port());
    }

    return m_fontCache;
}

void CanvasConnection::onWebSocketConnected()
{
    m_localPropertyRoot.reset(new LocalProp{nullptr, NameIndexTuple("")});
    setStatus(Connected);
}

void CanvasConnection::onTextMessageReceived(QString message)
{
    QJsonDocument json = QJsonDocument::fromJson(message.toUtf8());
    if (json.isObject()) {
        // process new nodes
        QJsonArray created = json.object().value("created").toArray();
        Q_FOREACH (QJsonValue v, created) {
            QJsonObject newProp = v.toObject();

            QByteArray nodePath = newProp.value("path").toString().toUtf8();
            if (nodePath.indexOf(m_rootPropertyPath) != 0) {
                qWarning() << "not a property path we are mirroring:" << nodePath;
                continue;
            }

            QByteArray localPath = nodePath.mid(m_rootPropertyPath.size() + 1);
            LocalProp* newNode = propertyFromPath(localPath);
            newNode->setPosition(newProp.value("position").toInt());
            // store in the global dict
            unsigned int propId = newProp.value("id").toInt();
            if (idPropertyDict.contains(propId)) {
                qWarning() << "duplicate add of:" << nodePath << "old is" << idPropertyDict.value(propId)->path();
            } else {
                idPropertyDict.insert(propId, newNode);
            }

            // set initial value
            newNode->processChange(newProp.value("value"));
        }

        // process removes
        QJsonArray removed = json.object().value("removed").toArray();
        Q_FOREACH (QJsonValue v, removed) {
            unsigned int propId = v.toInt();
            if (!idPropertyDict.contains(propId)) {
                continue;
            }

            auto prop = idPropertyDict.value(propId);
            idPropertyDict.remove(propId);

            // depending on the order removes are sent, the LocalProp
            // may already have been deleted when its parent was removed,
            // so check if the QPointer is null
            if (!prop.isNull()) {
                prop->parent()->removeChild(prop);
            }
        } // of removes processing

        // process changes
        QJsonArray changed = json.object().value("changed").toArray();

        Q_FOREACH (QJsonValue v, changed) {
            QJsonArray change = v.toArray();
            if (change.size() != 2) {
                qWarning() << "malformed change notification";
                continue;
            }

            unsigned int propId = change.at(0).toInt();
            if (!idPropertyDict.contains(propId)) {
                qWarning() << "ignoring unknown prop ID " << propId;
                continue;
            }

            LocalProp* lp = idPropertyDict.value(propId);
            if (lp != nullptr) {
                lp->processChange(change.at(1));
            }
        } // of change processing
    }

    emit updated();
}

void CanvasConnection::onWebSocketClosed()
{
    qDebug() << "saw web-socket closed";
    m_localPropertyRoot.reset();
    idPropertyDict.clear();

    setStatus(Closed);

    // if we're in automatic mode, start reconnection timer
// update state

    //   ui->stack->setCurrentIndex(0);
}

void CanvasConnection::setStatus(CanvasConnection::Status newStatus)
{
    if (newStatus == m_status)
        return;

    m_status = newStatus;
    emit statusChanged(m_status);
}

LocalProp *CanvasConnection::propertyFromPath(QByteArray path) const
{
    return m_localPropertyRoot->getOrCreateWithPath(path);
}

