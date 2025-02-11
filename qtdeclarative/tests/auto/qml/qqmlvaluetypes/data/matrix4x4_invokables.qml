import QtQuick

Item {
    property bool success: false

    property variant m1: Qt.matrix4x4(1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4)
    property variant m2: Qt.matrix4x4(5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,8)
    property variant m3: Qt.matrix4x4(123,22,6,42,55,54,67,77,777,1,112,22,55,6696,77,777)
    property matrix4x4 m4: PlanarTransform.fromAffineMatrix(1, 2, 3, 4, 5, 6)
    property variant v1: Qt.vector4d(1,2,3,4)
    property variant v2: Qt.vector3d(1,2,3)
    property real factor: 2.23

    function testTransformation() {
        let m = Qt.matrix4x4();

        m.scale(1, 2, 4);
        if (m !== Qt.matrix4x4(1, 0, 0, 0,
                               0, 2, 0, 0,
                               0, 0, 4, 0,
                               0, 0, 0, 1))
            return false;
        m.scale(Qt.vector3d(-8, -4, -2));
        if (m !== Qt.matrix4x4(-8, 0, 0, 0,
                               0,-8, 0, 0,
                               0, 0, -8, 0,
                               0, 0, 0, 1))
            return false;
        m.scale(-1 / 8);
        if (m !== Qt.matrix4x4())
            return false;

        m.rotate(180, Qt.vector3d(1, 0, 0));
        if (m !== Qt.matrix4x4(1, 0, 0, 0,
                               0, -1, 0, 0,
                               0, 0, -1, 0,
                               0, 0, 0, 1))
            return false;
        m.rotate(180, Qt.vector3d(0, 1, 0));
        if (m !== Qt.matrix4x4(-1, 0, 0, 0,
                               0, -1, 0, 0,
                               0, 0, 1, 0,
                               0, 0, 0, 1))
            return false;
        m.rotate(180, Qt.vector3d(0, 0, 1));
        if (m !== Qt.matrix4x4())
            return false;
        m.rotate(Qt.quaternion(0.5, 0.5, 0.5, -0.5));
        if (m !== Qt.matrix4x4(0, 1, 0, 0,
                               0, 0, -1, 0,
                               -1, 0, 0, 0,
                               0, 0, 0, 1))
            return false;

        m = Qt.matrix4x4();
        m.translate(Qt.vector3d(1, 2, 4));
        if (m !== Qt.matrix4x4(1, 0, 0, 1,
                               0, 1, 0, 2,
                               0, 0, 1, 4,
                               0, 0, 0, 1))
            return false;

        m = Qt.matrix4x4();
        m.lookAt(Qt.vector3d(1, 2, 4), Qt.vector3d(1, 2, 0), Qt.vector3d(0, 1, 0));
        if (m !== Qt.matrix4x4(1, 0, 0, -1,
                               0, 1, 0, -2,
                               0, 0, 1, -4,
                               0, 0, 0, 1))
            return false;

        return true;
    }

    function testMatrixMapping()
    {
        let m = Qt.matrix4x4();
        m.scale(1, 2, 3);

        if (m.mapRect(Qt.rect(10, 15, 20, 20)) !== Qt.rect(10, 30, 20, 40))
            return false;

        if (m.map(Qt.point(10, 15)) !== Qt.point(10, 30))
            return false;

        return true;
    }

    Component.onCompleted: {
        success = true;
        if (m1.times(m2) != Qt.matrix4x4(26, 26, 26, 26, 52, 52, 52, 52, 78, 78, 78, 78, 104, 104, 104, 104)) success = false;
        if (m1.times(v1) != Qt.vector4d(10, 20, 30, 40)) success = false;
        if (m1.times(v2) != Qt.vector3d(0.25, 0.5, 0.75)) success = false;
        if (!m1.times(factor).fuzzyEquals(Qt.matrix4x4(2.23, 2.23, 2.23, 2.23, 4.46, 4.46, 4.46, 4.46, 6.69, 6.69, 6.69, 6.69, 8.92, 8.92, 8.92, 8.92))) success = false;
        if (m1.plus(m2) != Qt.matrix4x4(6, 6, 6, 6, 8, 8, 8, 8, 10, 10, 10, 10, 12, 12, 12, 12)) success = false;
        if (m2.minus(m1) != Qt.matrix4x4(4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4)) success = false;
        if (m1.row(2) != Qt.vector4d(3,3,3,3)) success = false;
        if (m1.column(2) != Qt.vector4d(1,2,3,4)) success = false;
        if (m1.determinant() != 0) success = false;
        if (m3.determinant() != -15260238498) success = false;
        if (m1.inverted() != Qt.matrix4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)) success = false; // non-invertible
        if (!m3.inverted().fuzzyEquals(Qt.matrix4x4(0.0028384, -0.00188321, 0.000970577, 0.00000571656, -0.00206701, -0.000598587, 0.000358192, 0.000160908, -0.0235917, 0.0122695, 0.00286765, -0.0000218643, 0.01995, 0.00407588, -0.00343969, -0.000097903), 0.00001)) success = false;
        if (m1.transposed() != Qt.matrix4x4(1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4)) success = false;
        if (m1.fuzzyEquals(m2)) success = false;
        if (!m1.fuzzyEquals(m2, 10)) success = false;
        if (m4 != Qt.matrix4x4(1, 3, 0, 5,  2, 4, 0, 6,  0, 0, 1, 0,  0, 0, 0, 1)) success = false;
        if (!testTransformation()) success = false;
        if (!testMatrixMapping()) success = false;
    }
}
