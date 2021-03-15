const createViewer = (modelName, models, containerId, hwp_version) => {
    return new Promise(function (resolve, reject) {
        const viewer = new Communicator.WebViewer({
            containerId: containerId,
            endpointUri: "./data/"+ modelName +".scs",
            boundingPreviewMode: "none",
        });

        resolve(viewer);
    });
}

export default createViewer;
