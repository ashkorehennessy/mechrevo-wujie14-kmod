import QtQuick
import QtQuick.Layouts
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.plasmoid
import QtQuick.Window

PlasmoidItem {
    id: root

    //height and width, when the widget is placed in desktop
    width: 40
    height: 40

    preferredRepresentation: fullRepresentation

    function getPerfModeString(){
        var path = "file:///sys/devices/pci0000:00/0000:00:14.3/PNP0C09:00/wujie14_powermode"
        var req = new XMLHttpRequest();
        req.open("GET", path, false);
        req.send(null);
        var perfmodestr = req.responseText
        return perfmodestr
    }

    function convertPerfModeString(arg_perfmode_str){
        if (arg_perfmode_str.startsWith("perf")) {
            return "P";
        } else if (arg_perfmode_str.startsWith("balance")) {
            return "B";
        } else {
            return "E";
        }
    }

    fullRepresentation: Item {
        // dynamic sizing based on panel contents; avoids huge horizontal spacing on HiDPI
        Layout.minimumWidth: 20
        Layout.preferredWidth: 24
        Layout.fillHeight: true

        PlasmaComponents.Label {
            id: display

            anchors {
                fill: parent
                margins: Math.round(parent.width * 0.01)
            }

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter

            text: {
                return convertPerfModeString(getPerfModeString());
            }

            font.pixelSize: 800;
            minimumPointSize: 8
            fontSizeMode: Text.Fit
            font.bold: Plasmoid.configuration.makeFontBold
        }

        Timer {
            interval: Plasmoid.configuration.updateInterval * 1000
            running: true
            repeat: true
            onTriggered: {
                display.text = convertPerfModeString(getPerfModeString());
            }
        }
    }
}