import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.3

import StateEnum 1.0
import Controller 1.0
import Camera 1.0
import Generator 1.0
import AudioOutput 1.0
import AudioConfig 1.0
import SharedAmplitudes 1.0

ApplicationWindow {
    id: window
    title: qsTr("Whiteroom v0.3")
    visible: true
    width: 800
    height: 640
    minimumWidth: 600
    minimumHeight: 640
    Material.theme: Material.Light
    Material.accent: Material.DeepPurple

    function combineStates(x, y) {
        if (x === StateEnum.STOPPING || y === StateEnum.STOPPING) {
            return StateEnum.STOPPING;
        }
        if (x === StateEnum.STARTED || y === StateEnum.STARTED) {
            return StateEnum.STARTED;
        }
        return StateEnum.STOPPED;
    }

    function midiToFreq(x) { return Math.round(Math.pow(2, (x - 69) / 12.0) * 440).toString(); }

    function midiToNote(x) {
        return ["C", "C♯", "D", "D♯", "E", "F", "F♯", "G", "G♯", "A", "A♯", "B"][Math.round(x) % 12] + "<sub>" + (Math.round(x / 12) - 1) + "</sub>";
    }

    AudioConfig {
        id: config
        samples: parseInt(audioSamples.currentText) || 0
        device: audioOutput.currentIndex
    }

    Controller {
        id: controller
        camera1: camera1
        generator: generator
        output: output
        config: config
    }

    SharedAmplitudes {
        id: amplitudes
        maxKeysNumber: 200
    }

    Camera {
        id: camera1
        url: camera1Url.text
        cameraSensitivity: sensitivitySlider.value
        verticalWeighting: verticalWeightingSlider.value
        audioHeight: 20
        keysNumber: keysNumberSlider.value
        amplitudes: amplitudes
        reverb: reverbSlider.value
    }

    Generator {
        id: generator
        volume: volumeSlider.value
        minNote: rangeSlider.first.value
        maxNote: rangeSlider.second.value
        keysNumber: keysNumberSlider.value
        amplitudes: amplitudes
        maxKeysNumber: amplitudes.maxKeysNumber
    }

    AudioOutput {
        id: output
        generator: generator
    }

    ColumnLayout {
        anchors.fill: parent

        GridLayout {
            id: grid
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.margins: 30
            columns: 2
            rowSpacing: 5
            columnSpacing: 20

            Label {
                text: qsTr("Camera")
            }

            TextField {
                id: camera1Url
                selectByMouse: true
                text: qsTr("rtsp://")
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Output")
            }

            ComboBox {
                id: audioOutput
                model: output.devices
                Layout.fillWidth: true
                textRole: "display"
            }

            Label {
                text: qsTr("Buffer size")
            }

            ComboBox {
                id: audioSamples
                model: [64, 128, 192, 256, 320, 384, 448, 512, 1024]
                currentIndex: 2
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Camera sensitivity")
            }

            RowLayout {
                Slider {
                    id: sensitivitySlider
                    stepSize: 1
                    to: 100
                    value: 10
                    Layout.fillWidth: true
                }
                Label {
                    horizontalAlignment: Text.AlignRight
                    text: sensitivitySlider.value
                    Layout.minimumWidth: 30
                }
            }

            Label {
                text: qsTr("Vertical sensitivity")
            }

            RowLayout {
                Slider {
                    id: verticalWeightingSlider
                    stepSize: 1
                    to: 100
                    value: 30
                    Layout.fillWidth: true
                }
                Label {
                    horizontalAlignment: Text.AlignRight
                    text: verticalWeightingSlider.value
                    Layout.minimumWidth: 30
                }
            }

            Label {
                text: rangeValues.notes ? qsTr("Range") : qsTr("Range (Hz)")
                MouseArea {
                    anchors.fill: parent
                    onClicked: rangeValues.notes = !rangeValues.notes
                }
            }

            RowLayout {
                property bool notes: false
                id: rangeValues

                RangeSlider {
                    id: rangeSlider
                    stepSize: 1
                    from: 40
                    to: 123
                    first.value: 47
                    second.value: 102
                    snapMode: RangeSlider.SnapAlways
                    Layout.fillWidth: true
                }
                Label {
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: 37
                    textFormat: Text.RichText
                    text: rangeValues.notes ? midiToNote(rangeSlider.first.value) : midiToFreq(rangeSlider.first.value)

                    MouseArea {
                        anchors.fill: parent
                        onClicked: rangeValues.notes = !rangeValues.notes
                    }
                }
                Label {
                    horizontalAlignment: Text.AlignRight
                    Layout.minimumWidth: 37
                    textFormat: Text.RichText
                    text: rangeValues.notes ? midiToNote(rangeSlider.second.value) : midiToFreq(rangeSlider.second.value)

                    MouseArea {
                        anchors.fill: parent
                        onClicked: rangeValues.notes = !rangeValues.notes
                    }
                }
            }

            Label {
                text: qsTr("Keys number")
            }

            RowLayout {
                Slider {
                    id: keysNumberSlider
                    stepSize: 1
                    from: 2
                    to: amplitudes.maxKeysNumber
                    value: 150
                    Layout.fillWidth: true
                }
                Label {
                    horizontalAlignment: Text.AlignRight
                    text: keysNumberSlider.value
                    Layout.minimumWidth: 30
                }
            }

            Label {
                text: qsTr("Reverb")
            }

            RowLayout {
                Slider {
                    id: reverbSlider
                    from: 0
                    stepSize: 1
                    to: 99
                    value: 10
                    Layout.fillWidth: true
                }
                Label {
                    horizontalAlignment: Text.AlignRight
                    text: reverbSlider.value
                    Layout.minimumWidth: 30
                }
            }

            Label {
                text: qsTr("Volume")
            }

            RowLayout {
                Slider {
                    id: volumeSlider
                    from: 0
                    stepSize: 1
                    to: 100
                    value: 10
                    Layout.fillWidth: true
                }
                Label {
                    horizontalAlignment: Text.AlignRight
                    text: volumeSlider.value
                    Layout.minimumWidth: 30
                }
            }
        }

        Label {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            text: camera1.lastError
        }

        RowLayout {
            Layout.bottomMargin: 30
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            spacing: 20

            Button {
                text: qsTr("Start")
                onClicked: controller.onStartClicked()
                enabled: combineStates(controller.state, camera1.state) === StateEnum.STOPPED
            }
            Button {
                text: qsTr("Stop")
                onClicked: controller.onStopClicked()
                enabled: combineStates(controller.state, camera1.state) === StateEnum.STARTED
            }
        }
    }
}
