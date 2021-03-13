function ColorContour(viewer, width, height) {
    this._overlayId;
    var model = viewer.getModel();
    var meshData = new Communicator.MeshData();
    meshData.setFaceWinding(Communicator.FaceWinding.Clockwise);
    meshData.addFaces([
        -width / 2, -height / 2, 0,
        -width / 2, 0, 0,
        width / 2, -height / 2, 0,
        -width / 2, 0, 0,
        width / 2, 0, 0,
        width / 2, -height / 2, 0,
        -width / 2, 0, 0,
        -width / 2, height / 2, 0,
        width / 2, 0, 0,
        -width / 2, height / 2, 0,
        width / 2, height / 2, 0,
        width / 2, 0, 0
    ], [
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
    ], new Uint8Array([
        0, 0, 255, 255,
        0, 255, 0, 255,
        0, 0, 255, 255,
        0, 255, 0, 255,
        0, 255, 0, 255,
        0, 0, 255, 255,
        0, 255, 0, 255,
        255, 0, 0, 255,
        0, 255, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        0, 255, 0, 255,
    ]));

    var overlayMgr = viewer.getOverlayManager();
    this._overlayId = overlayMgr.maxIndex();
    overlayMgr.setViewport(
        this._overlayId, Communicator.OverlayAnchor.RightCenter,
        50, Communicator.OverlayUnit.Pixels, 0, Communicator.OverlayUnit.Pixels,
        width, Communicator.OverlayUnit.Pixels, height, Communicator.OverlayUnit.Pixels);
    overlayMgr.setVisibility(this._overlayId, true);

    model.createMesh(meshData).then(function (meshId) {
        var meshInstanceData = new Communicator.MeshInstanceData(meshId);
        console.log(model);
        console.log("meshid="+meshId);
        console.log(meshInstanceData);
        model.createMeshInstance(meshInstanceData).then(function (instanceId) {
            viewer.getModel().setInstanceModifier(Communicator.InstanceModifier.IgnoreLighting, [instanceId], true);
            overlayMgr.addNodes(this._overlayId, [instanceId]);
        }).catch(function (err) {
            console.error(err);
        });
        var camera = new Communicator.Camera();
        camera.setPosition(new Communicator.Point3(0, 0, 1));
        camera.setWidth(width);
        camera.setHeight(height);
        camera.setTarget(Communicator.Point3.zero());
        camera.setUp(new Communicator.Point3(0, 1, 0));
        camera.setProjection(Communicator.Projection.Orthographic);
        overlayMgr.setCamera(this._overlayId, camera);
    });
}
