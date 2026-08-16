import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    anchors.fill: parent

    // Model / View properties bound to C++ backend or QML logic
    property string modelName: "Sponza.gltf"
    property real fpsCount: 60.0
    property bool pbrEnabled: true
    property bool frustumCullingEnabled: true
    property real lightIntensity: 1.0

    // Top-Left Header Badge (Glassmorphism design)
    Rectangle {
        id: headerBadge
        x: 20; y: 20
        width: 320; height: 110
        radius: 16
        color: Qt.rgba(0.08, 0.10, 0.15, 0.75)
        border.color: Qt.rgba(1.0, 1.0, 1.0, 0.15)
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 6

            RowLayout {
                spacing: 8
                Rectangle {
                    width: 10; height: 10; radius: 5
                    color: "#00E676" // Glow green status dot
                }
                Text {
                    text: "bgl::gfx 3D Engine — QML Overlay"
                    color: "#FFFFFF"
                    font.pixelSize: 14
                    font.bold: true
                    font.family: "Inter, Roboto, sans-serif"
                }
            }

            Text {
                text: "Asset: " + root.modelName + " | FPS: " + Math.round(root.fpsCount)
                color: "#B0BEC5"
                font.pixelSize: 12
                font.family: "Inter, Roboto, sans-serif"
            }

            Text {
                text: "Pipeline: OpenGL 4.6 DSA | GPU Frustum Culling"
                color: "#78909C"
                font.pixelSize: 11
                font.family: "Inter, Roboto, sans-serif"
            }
        }
    }

    // Right Side Control Panel (Interactive HUD Overlay)
    Rectangle {
        id: controlPanel
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.rightMargin: 20
        width: 260; height: 260
        radius: 16
        color: Qt.rgba(0.08, 0.10, 0.15, 0.80)
        border.color: Qt.rgba(0.3, 0.5, 1.0, 0.3)
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 18
            spacing: 14

            Text {
                text: "PBR & Pipeline Controls"
                color: "#64B5F6"
                font.pixelSize: 15
                font.bold: true
            }

            // Frustum Culling Toggle Button
            Rectangle {
                Layout.fillWidth: true
                height: 40
                radius: 10
                color: cullMouse.containsMouse ? "#1E88E5" : "#1565C0"

                Text {
                    anchors.centerIn: parent
                    text: root.frustumCullingEnabled ? "⚡ GPU Culling: ENABLED" : "⚠️ GPU Culling: DISABLED"
                    color: "#FFFFFF"
                    font.bold: true
                    font.pixelSize: 12
                }

                MouseArea {
                    id: cullMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: root.frustumCullingEnabled = !root.frustumCullingEnabled
                }
            }

            // PBR Shading Toggle Button
            Rectangle {
                Layout.fillWidth: true
                height: 40
                radius: 10
                color: pbrMouse.containsMouse ? "#43A047" : "#2E7D32"

                Text {
                    anchors.centerIn: parent
                    text: root.pbrEnabled ? "✨ Metallic-Roughness PBR: ON" : "🎨 Basic Shader: ON"
                    color: "#FFFFFF"
                    font.bold: true
                    font.pixelSize: 12
                }

                MouseArea {
                    id: pbrMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: root.pbrEnabled = !root.pbrEnabled
                }
            }

            // Light Intensity Slider Label
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Light Intensity"
                    color: "#ECEFF1"
                    font.pixelSize: 12
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: Math.round(slider.value * 100) + "%"
                    color: "#FFB74D"
                    font.pixelSize: 12
                    font.bold: true
                }
            }

            Slider {
                id: slider
                Layout.fillWidth: true
                from: 0.1
                to: 3.0
                value: 1.0
                onValueChanged: root.lightIntensity = value
            }
        }
    }

    // Bottom Banner Info Overlay
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 20
        width: 450; height: 42
        radius: 21
        color: Qt.rgba(0.05, 0.07, 0.10, 0.85)
        border.color: Qt.rgba(1.0, 1.0, 1.0, 0.2)

        RowLayout {
            anchors.centerIn: parent
            spacing: 12

            Text {
                text: "💡 QML UI Overlay seamlessly composited over 3D glTF viewport"
                color: "#E0E0E0"
                font.pixelSize: 12
                font.medium: true
            }
        }
    }
}
