import QtQuick
import QtLocation
import QtPositioning

Item {

    function setMarkerCoordinates(lat, lon) {
        map.markerCoordinate = QtPositioning.coordinate(lat, lon);
        map.center = map.markerCoordinate;
    }

    Plugin {
        id: mapPlugin
        name: "osm"
        parameters: [
            PluginParameter {
                name: "osm.mapping.offline.directory"
                value: "your_tile_directory"
            },
            PluginParameter {
                name: "osm.mapping.providersrepository.address"
                value: "https://tile.thunderforest.com/atlas/{z}/{x}/{y}.png?apikey=@API_KEY@"
            }
        ]
    }

    Map {
        id: map
        objectName: "pichascanmap"
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(59.91, 10.75) // Initial center
        zoomLevel: 14
        property geoCoordinate startCentroid
        property var markerCoordinate: null

        signal mapMarkerSignal(double latitude, double longitude)

        MapQuickItem {
            id: marker
            coordinate: map.markerCoordinate
            visible: marker.coordinate !== null
            anchorPoint.x: 12
            anchorPoint.y: 12
            sourceItem: Text {
                id: text
                text: "+"
                font.pixelSize: 24
                color: "red"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: function (mouse) {
                let coord = map.toCoordinate(Qt.point(mouse.x, mouse.y), false);
                map.markerCoordinate = coord;
                map.mapMarkerSignal(coord.latitude, coord.longitude);
            }
        }

        WheelHandler {
            id: wheel
            target: null
            acceptedDevices: PointerDevice.Mouse
            rotationScale: 1 / 120
            onWheel: (event)=> {
                let zoomDelta = event.angleDelta.y > 0 ? 0.5 : -0.5; // Smaller zoom steps
                let newZoomLevel = Math.max(map.minimumZoomLevel, Math.min(map.maximumZoomLevel, map.zoomLevel + zoomDelta));

                if (map.markerCoordinate) {
                    let mouseCoord = map.toCoordinate(Qt.point(event.x, event.y), false);
                    map.alignCoordinateToPoint(mouseCoord, Qt.point(event.x, event.y));
                }

                map.zoomLevel = newZoomLevel;
            }
        }

        PinchHandler {
            id: pinch
            target: null
            onActiveChanged: if (active) {
                map.startCentroid = map.toCoordinate(pinch.centroid.position, false);
            }
            onScaleChanged: delta => {
                map.zoomLevel += Math.log2(delta);
                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position);
            }
            onRotationChanged: delta => {
                map.bearing -= delta;
                map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position);
            }
        }

        DragHandler {
            id: drag
            target: null
            onTranslationChanged: delta => {
                map.pan(-delta.x, -delta.y);
            }
        }
    }
}
