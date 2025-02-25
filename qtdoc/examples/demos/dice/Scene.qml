// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Helpers

Item {
    id: item
    property real settingGravity: 980.7
    property real settingsStaticFriction: 0.5
    property real settingsDynamicFriction: 0.5
    property real settingsRestitution: 0.5

    function spawnDice(numberOfDice, rollForce, diceWidth) {
        diceSpawner.spawnDice(numberOfDice, physicsMaterial, rollForce, diceWidth)
    }

    function setDiceWidth(diceWidth) {
        diceSpawner.setDiceWidth(diceWidth);
    }

    Screen.onPrimaryOrientationChanged: {
        var orientation = Screen.primaryOrientation
        var isPortrait = orientation === Qt.PortraitOrientation
                || orientation === Qt.InvertedPortraitOrientation
        viewport.camera.position = Qt.vector3d(0, -20, isPortrait ? 100 : 60)
    }

    PhysicsWorld {
        id: physicsWorld
        running: true
        enableCCD: true
        scene: viewport.scene
        gravity: Qt.vector3d(0, -item.settingGravity, 0)
        typicalLength: 1
        typicalSpeed: 1000
        minimumTimestep: 15
        maximumTimestep: 20
    }

    PhysicsMaterial {
        id: physicsMaterial
        staticFriction: item.settingsStaticFriction
        dynamicFriction: item.settingsDynamicFriction
        restitution: item.settingsRestitution
    }

    OrbitCameraController {
        anchors.fill: parent
        origin: originNode
        camera: camera
    }

    View3D {
        id: viewport
        anchors.fill: parent
        camera: camera

        Node {
            id: originNode
            eulerRotation: Qt.vector3d(-14.2885, 410.287, 0)
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, -20, 100)
                clipFar: 1000
                clipNear: 0.1
            }
        }

        environment: SceneEnvironment {
            clearColor: "white"
            backgroundMode: SceneEnvironment.SkyBox
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
            lightProbe: proceduralSky
        }

        Texture {
            id: proceduralSky
            textureData: ProceduralSkyTextureData {
                sunLongitude: -115
            }
        }

        Node {
            id: scene

            DirectionalLight {
                eulerRotation: Qt.vector3d(-45, 25, 0)
                castsShadow: true
                brightness: 1
                shadowFactor: 100
                shadowMapQuality: Light.ShadowMapQualityVeryHigh
                softShadowQuality: Light.PCF4
                shadowBias: 0.2
                shadowMapFar: camera.clipFar
                pcfFactor: 0.05
            }

            PhysicalTable {
                id: table
            }

            DiceSpawner {
                id: diceSpawner
            }

            Carpet {
                position: Qt.vector3d(0, -50, 0)
            }

            // floor
            StaticRigidBody {
                position: Qt.vector3d(0, -50, 0)
                eulerRotation: Qt.vector3d(-90, 0, 0)
                collisionShapes: PlaneShape {}
            }
        }
    }
}
