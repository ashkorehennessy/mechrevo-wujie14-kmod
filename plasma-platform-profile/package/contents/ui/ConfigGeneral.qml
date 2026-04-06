import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: configItem
    property real cfg_updateInterval: 1.0
    property alias cfg_makeFontBold: makeFontBold.checked

    ColumnLayout {
        RowLayout {
            Label {
                id: updateIntervalLabel
                text: i18n(" Update interval:")
            }
            SpinBox {
                id: updateInterval
                stepSize: 1
                from: 1
                to: 100
                
                value: configItem.cfg_updateInterval * 10
                
                onValueChanged: {
                    configItem.cfg_updateInterval = value / 10.0
                }

                textFromValue: function(value, locale) {
                    return Number(value / 10.0).toLocaleString(locale, 'f', 1) + " " + i18nc("Abbreviation for seconds", "s")
                }

                valueFromText: function(text, locale) {
                    return Number.fromLocaleString(locale, text.replace(" " + i18nc("Abbreviation for seconds", "s"), "")) * 10
                }
            }
        }

        CheckBox {
            id: makeFontBold
            text: i18n("Bold Text")
        }

    }
}
