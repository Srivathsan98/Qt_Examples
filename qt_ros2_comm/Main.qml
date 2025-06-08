import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("ROS 2 MQTT Client")
    id: root

    property var tempSubscription: 0
    property string latestResponse: ""

    MqttClient {
        id: client
        hostname: "localhost"
        port: 1883
    }

    ListModel {
        id: messageModel
    }

    function addMessage(payload) {
        latestResponse = payload
        messageModel.insert(0, { "payload": payload })
        if (messageModel.count >= 100)
            messageModel.remove(99)
    }

    Component.onCompleted: {
        client.connectToHost()
    }

    Connections {
        target: client
        onStateChanged: {
            if (client.state === MqttClient.Connected) {
                console.log("Connected to MQTT broker.")
                if (root.tempSubscription === 0) {
                    tempSubscription = client.subscribe("sensor/response")
                    tempSubscription.messageReceived.connect(addMessage)
                }
            } else if (client.state === MqttClient.Disconnected) {
                console.log("Disconnected from MQTT broker.")
                // Try to reconnect after a delay
                reconnectTimer.start()
            }
        }
    }

    Timer {
        id: reconnectTimer
        interval: 5000  // 5 seconds
        repeat: false
        onTriggered: {
            console.log("Attempting to reconnect...")
            client.connectToHost()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        GroupBox {
            title: "Latest Response"
            Layout.fillWidth: true

            Label {
                text: latestResponse.length > 0 ? latestResponse : "No response yet"
                font.bold: true
                wrapMode: Text.WordWrap
                padding: 10
            }
        }

        GroupBox {
            title: "Send Request"
            Layout.fillWidth: true

            RowLayout {
                spacing: 10
                Button {
                    text: "Get Time"
                    onClicked: {
                        client.publishMessage("sensor/request", "time")
                        console.log("Published time request")
                    }
                }
                Button {
                    text: "Get Storage"
                    onClicked: {
                        client.publishMessage("sensor/request", "storage")
                        console.log("Published storage request")
                    }
                }
                Button {
                    text: "Get Temperature"
                    onClicked: {
                        client.publishMessage("sensor/request", "temperature")
                        console.log("Published temperature request")
                    }
                }
            }
        }

        GroupBox {
            title: "Response History"
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: messageView
                model: messageModel
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                delegate: Rectangle {
                    id: delegatedRectangle
                    required property int index
                    required property string payload
                    width: ListView.view.width
                    height: 30
                    color: index % 2 ? "#DDDDDD" : "#888888"
                    radius: 5
                    Text {
                        text: delegatedRectangle.payload
                        anchors.centerIn: parent
                    }
                }
            }
        }

        Label {
            function stateToString(value) {
                if (value === 0)
                    return "Disconnected"
                else if (value === 1)
                    return "Connecting"
                else if (value === 2)
                    return "Connected"
                else
                    return "Unknown"
            }

            Layout.fillWidth: true
            text: "Status: " + stateToString(client.state) + " (" + client.state + ")"
            color: client.state === MqttClient.Connected ? "green" : "red"
        }
    }
}
