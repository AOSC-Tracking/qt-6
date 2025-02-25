import QtQuick
import QtQuick.Shapes

Item {
    width: 320
    height: 480

    ListModel {
        id: renderers
        ListElement { renderer: Shape.GeometryRenderer }
        ListElement { renderer: Shape.CurveRenderer }
    }

    Row {
        Repeater {
            model: renderers
            Column {
                Shape {
                    preferredRendererType: renderer
                    width: 160
                    height: 150

                    ShapePath {
                        strokeColor: "transparent"
                        fillGradient: LinearGradient {
                            y1: 50; y2: 80
                            GradientStop { position: 0; color: "black" }
                            GradientStop { position: 1; color: "cyan" }
                        }
                        fillTransform: PlanarTransform.fromAffineMatrix(0.8, 0.2, 0.3, 1.5, 20, -50 + startY)

                        startX: 10; startY: 10
                        PathLine { relativeX: 140; relativeY: 0 }
                        PathLine { relativeX: 0; relativeY: 100 }
                        PathLine { relativeX: -140; relativeY: 0 }
                        PathLine { relativeX: 0; relativeY: -100 }
                    }

                    ShapePath {
                        strokeColor: "transparent"
                        fillGradient: RadialGradient {
                            centerX: 80
                            centerY: 75
                            centerRadius: centerY
                            focalX: centerX
                            focalY: centerY
                            GradientStop { position: 0; color: "black" }
                            GradientStop { position: .5; color: "cyan" }
                            GradientStop { position: 1; color: "black" }
                        }
                        fillTransform: PlanarTransform.fromAffineMatrix(0.8, 0.2, 0.3, 1.5, 20, -50 + startY)

                        startX: 10; startY: 10 + 1 * 140
                        PathLine { relativeX: 140; relativeY: 0 }
                        PathLine { relativeX: 0; relativeY: 100 }
                        PathLine { relativeX: -140; relativeY: 0 }
                        PathLine { relativeX: 0; relativeY: -100 }
                    }

                    ShapePath {
                        strokeColor: "transparent"
                        fillGradient: ConicalGradient {
                            centerX: 80
                            centerY: 75
                            GradientStop { position: 0; color: "black" }
                            GradientStop { position: .5; color: "cyan" }
                            GradientStop { position: 1; color: "black" }
                        }
                        fillTransform: PlanarTransform.fromAffineMatrix(0.8, 0.2, 0.3, 1.5, 20, -50 + startY)

                        startX: 10; startY: 10 + 2 * 140
                        PathLine { relativeX: 140; relativeY: 0 }
                        PathLine { relativeX: 0; relativeY: 100 }
                        PathLine { relativeX: -140; relativeY: 0 }
                        PathLine { relativeX: 0; relativeY: -100 }
                    }
                }
            }
        }
    }
}
