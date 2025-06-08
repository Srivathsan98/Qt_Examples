// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "qmlmqttclient.h"
#include <QDebug>

QmlMqttSubscription::QmlMqttSubscription(QMqttSubscription *s, QmlMqttClient *c)
    : sub(s)
    , client(c)
{
    connect(sub, &QMqttSubscription::messageReceived, this, &QmlMqttSubscription::handleMessage);
}

QmlMqttSubscription::~QmlMqttSubscription()
{
}

QmlMqttClient::QmlMqttClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_client, &QMqttClient::hostnameChanged, this, &QmlMqttClient::hostnameChanged);
    connect(&m_client, &QMqttClient::portChanged, this, &QmlMqttClient::portChanged);
    connect(&m_client, &QMqttClient::stateChanged, this, &QmlMqttClient::stateChanged);
}

void QmlMqttClient::connectToHost()
{
    qDebug() << "Connecting to MQTT broker at" << m_client.hostname() << ":" << m_client.port();
    m_client.connectToHost();
}

void QmlMqttClient::disconnectFromHost()
{
    qDebug() << "Disconnecting from MQTT broker";
    m_client.disconnectFromHost();
}

QmlMqttSubscription* QmlMqttClient::subscribe(const QString &topic)
{
    qDebug() << "Subscribing to topic:" << topic;
    auto sub = m_client.subscribe(topic, 1); // Using QoS 1 to match ROS2
    auto result = new QmlMqttSubscription(sub, this);
    return result;
}

void QmlMqttSubscription::handleMessage(const QMqttMessage &qmsg)
{
    qDebug() << "Received message on topic:" << qmsg.topic() << "with payload:" << qmsg.payload();
    emit messageReceived(qmsg.payload());
}

const QString QmlMqttClient::hostname() const
{
    return m_client.hostname();
}

void QmlMqttClient::setHostname(const QString &newHostname)
{
    m_client.setHostname(newHostname);
}

int QmlMqttClient::port() const
{
    return m_client.port();
}

void QmlMqttClient::setPort(int newPort)
{
    if (newPort < 0 || newPort > std::numeric_limits<quint16>::max()) {
        qWarning() << "Trying to set invalid port number";
        return;
    }
    m_client.setPort(static_cast<quint16>(newPort));
}

QMqttClient::ClientState QmlMqttClient::state() const
{
    return m_client.state();
}

void QmlMqttClient::setState(const QMqttClient::ClientState &newState)
{
    m_client.setState(newState);
}

void QmlMqttClient::publishMessage(const QString &topic, const QString &message)
{
    qDebug() << "Publishing message to topic:" << topic << "with payload:" << message;
    m_client.publish(topic, message.toUtf8(), 1); // Using QoS 1 to match ROS2
}
