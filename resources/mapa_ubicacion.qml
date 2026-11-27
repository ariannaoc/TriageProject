import QtQuick
import QtLocation
import QtPositioning

Item {
    id: mapaItem
    width: 640
    height: 320

    Map {
        id: mapa
        anchors.fill: parent
        plugin: Plugin { name: "osm" }
        center: QtPositioning.coordinate(mapaControl.latitud, mapaControl.longitud)
        zoomLevel: 15
        minimumZoomLevel: 5
        maximumZoomLevel: 20

        MapQuickItem {
            id: marcador
            anchorPoint.x: 10
            anchorPoint.y: 10
            coordinate: QtPositioning.coordinate(mapaControl.latitud, mapaControl.longitud)
            sourceItem: Rectangle {
                width: 20
                height: 20
                radius: 10
                color: "#d32f2f"
                border.color: "white"
                border.width: 3
            }
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: function(event, point) {
                var coord = mapa.toCoordinate(point)
                mapaControl.latitud = coord.latitude
                mapaControl.longitud = coord.longitude
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 8
        radius: 4
        color: "#aa000000"
        Text {
            anchors.fill: parent
            anchors.margins: 6
            color: "white"
            font.pixelSize: 12
            text: "Toque el mapa para ajustar la ubicación"
        }
    }
}
