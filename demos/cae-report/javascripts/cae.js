import createViewer from '/javascripts/common/create_viewer.js';

export default function CAE_report (language) {
    this._viewer;
    this._language = language;
    this._modelName;
    this._defaultCamera;
    this._flatOverlayId;
    this._annot;
    this._mises;
    this._tresca;
    this._sigma1;
    this._sigma2;
    this._sigma3;
    this._strain;
    this._nodes = [];
}

CAE_report.prototype.start = function (modelName) {
    var _this = this;
    _this._modelName = modelName;
    _this._initEvents();
    _this._loadParameters(modelName).then(function () {
        _this._createViewer(modelName, [modelName], "container")
    });
}

CAE_report.prototype._loadParameters = function (modelName) {
    var _this = this;
    
    return new Promise(function (resolve, reject) {
        var fileName = "/demos/cae-report/json/" + _this._modelName + ".json";
        $.get(fileName).done(function(data, textStatus, jqXHR){
            $.getJSON(fileName, function (data) {
                if (data.header) {
                    var header = data.header;
                    $('.headerDate').text(header.date);
                    $('.headerNum').text(header.number);
                    $('.headerAuthor').text(header.author);
                }
                if (data.results) {
                    var results = data.results;
                    $('#misesNode').val(results.mises.node);
                    $('#misesStress').val(results.mises.stress.toFixed(2));
                    $('#trescaNode').val(results.tresca.node);
                    $('#trescaStress').val(results.tresca.stress.toFixed(2));
                    $('#principleNode1').val(results.principleStress.signam1.node);
                    $('#principleStress1').val(results.principleStress.signam1.stress.toFixed(2));
                    $('#principleNode2').val(results.principleStress.signam2.node);
                    $('#principleStress2').val(results.principleStress.signam2.stress.toFixed(2));
                    $('#principleNode3').val(results.principleStress.signam3.node);
                    $('#principleStress3').val(results.principleStress.signam3.stress.toFixed(2));
                    $('#principleNode').val(results.principleStrain.node);
                    $('#principleStrain').val(results.principleStrain.strain.toFixed(6));

                    var range = results.mises.stress - results.mises.min;
                    for (var i = 0; i < 16; i++) {
                        var val = results.mises.stress - range / 15 * i;
                        $("#colorLabel" + i).text(val.toFixed(2));
                    }
                }
                if (data.camera) {
                    _this._defaultCamera = data.camera;
                }
                if (data.annotations) {
                    _this._mises = data.annotations.mises;
                    _this._tresca = data.annotations.tresca;
                    _this._sigma1 = data.annotations.sigma1;
                    _this._sigma2 = data.annotations.sigma2;
                    _this._sigma3 = data.annotations.sigma3;
                    _this._strain = data.annotations.strain;
                }
                if (data.nodes) {
                    _this._nodes = data.nodes;
                }
                resolve();
            });
        }).fail(function(jqXHR, textStatus, errorThrown){
            resolve();
        });
    });
}

CAE_report.prototype._initEvents = function () {
    var _this = this;
    
    var resizeTimer;
    var interval = Math.floor(1000 / 60 * 10);
    $(window).resize(function() {
      if (resizeTimer !== false) {
        clearTimeout(resizeTimer);
      }
      resizeTimer = setTimeout(function () {
        layoutPage()
        _this._viewer.resizeCanvas();
      }, interval);
    });
    
    layoutPage();
    function layoutPage () {
        let wsize = $(window).width();
        let hsize = $(window).height();
        
        var mainWidth = $("#mainContainer").width()
        $("#container").height(550);
        
        if (wsize < 992) {
            $("#container").width(mainWidth);
        } else {
            $("#container").width(mainWidth / 12 * 8);
            $("#container").height($("table.report").height());
        }
        
        var offset = $("#container").offset();
        $("#colorTitle").offset({
            left: offset.left + $("#container").width() - 120,
            top: offset.top + ($("#container").height() - 400) / 2 - 30
        });
        
        $(".colorLabel").offset({
            left: offset.left + $("#container").width() - 40
        });
        
        for (var i = 0; i < 16; i++) {
            $("#colorLabel" + i).offset({
                top: offset.top + ($("#container").height() - 400) / 2 - 10 + (400 / 15) * i
            });
        }
        
        $("#results").offset({
            left: offset.left + 10,
            top: offset.top + 10
        });
        
        $("#deleteAnnot").offset({
            left: offset.left + 10,
            top: offset.top + $("#container").height() - 35
        });
    }
    
    $("#testBtn1").click(function (e) {
        var camera = _this._viewer.getView().getCamera();
        var json = camera.forJson();
        var str = JSON.stringify(json);
        
        var a = _this._annot.serialize();
        var str2 = JSON.stringify(a);
        
    });
    
    $(".bt").on("click", function (event) {
        var id = $(this).data("id");
        switch (id) {
            case 0:
                if (_this._mises.annotation._created == undefined) {
                    var anchorPoint = Communicator.Point3.construct(_this._mises.annotation.leaderAnchor);
                    var textBoxPoint = Communicator.Point3.construct(_this._mises.annotation.textBoxAnchor);
                    _this._annot.createMarkup(_this._mises.annotation.label, anchorPoint, textBoxPoint);
                    _this._mises.annotation._created = true;
                }
                if (_this._mises.camera != undefined)
                    _this._viewer.getView().setCamera(Communicator.Camera.construct(_this._mises.camera), 500);
                break;
            case 1:
                if (_this._tresca.annotation._created == undefined) {
                    var anchorPoint = Communicator.Point3.construct(_this._tresca.annotation.leaderAnchor);
                    var textBoxPoint = Communicator.Point3.construct(_this._tresca.annotation.textBoxAnchor);
                    _this._annot.createMarkup(_this._tresca.annotation.label, anchorPoint, textBoxPoint);
                    _this._tresca.annotation._created = true;
                }
                if (_this._tresca.camera != undefined)
                    _this._viewer.getView().setCamera(Communicator.Camera.construct(_this._tresca.camera), 500);
                break;
            case 2:
                if (_this._sigma1.annotation._created == undefined) {
                    var anchorPoint = Communicator.Point3.construct(_this._sigma1.annotation.leaderAnchor);
                    var textBoxPoint = Communicator.Point3.construct(_this._sigma1.annotation.textBoxAnchor);
                    _this._annot.createMarkup(_this._sigma1.annotation.label, anchorPoint, textBoxPoint);
                    _this._sigma1.annotation._created = true;
                }
                if (_this._sigma1.camera != undefined)
                    _this._viewer.getView().setCamera(Communicator.Camera.construct(_this._sigma1.camera), 500);
                break;
            case 3:
                if (_this._sigma2.annotation._created == undefined) {
                    var anchorPoint = Communicator.Point3.construct(_this._sigma2.annotation.leaderAnchor);
                    var textBoxPoint = Communicator.Point3.construct(_this._sigma2.annotation.textBoxAnchor);
                    _this._annot.createMarkup(_this._sigma2.annotation.label, anchorPoint, textBoxPoint);
                    _this._sigma2.annotation._created = true;
                }
                if (_this._sigma2.camera != undefined)
                    _this._viewer.getView().setCamera(Communicator.Camera.construct(_this._sigma2.camera), 500);
                break;
            case 4:
                if (_this._sigma3.annotation._created == undefined) {
                    var anchorPoint = Communicator.Point3.construct(_this._sigma3.annotation.leaderAnchor);
                    var textBoxPoint = Communicator.Point3.construct(_this._sigma3.annotation.textBoxAnchor);
                    _this._annot.createMarkup(_this._sigma3.annotation.label, anchorPoint, textBoxPoint);
                    _this._sigma3.annotation._created = true;
                }
                if (_this._sigma3.camera != undefined)
                    _this._viewer.getView().setCamera(Communicator.Camera.construct(_this._sigma3.camera), 500);
                break;
            case 5:
                if (_this._strain.annotation._created == undefined) {
                    var anchorPoint = Communicator.Point3.construct(_this._strain.annotation.leaderAnchor);
                    var textBoxPoint = Communicator.Point3.construct(_this._strain.annotation.textBoxAnchor);
                    _this._annot.createMarkup(_this._strain.annotation.label, anchorPoint, textBoxPoint);
                    _this._strain.annotation._created = true;
                }
                if (_this._strain.camera != undefined)
                    _this._viewer.getView().setCamera(Communicator.Camera.construct(_this._strain.camera), 500);
                break;
                  }
    }); 
    
    $("#deleteAnnot").on("click", function (event) {
        _this._annot.deleteAll();
        delete _this._mises.annotation._created;
        delete _this._tresca.annotation._created;
        delete _this._sigma1.annotation._created;
        delete _this._sigma2.annotation._created;
        delete _this._sigma3.annotation._created;
        delete _this._strain.annotation._created;

        $("#result0").text("");
        $("#result1").text("");
        $("#result2").text("");
        $("#result3").text("");
        $("#result4").text("");
        $("#result5").text("");
        $("#result6").text("");
    });
}
    
CAE_report.prototype._createViewer = function (modelName, models, containerId) {
    var _this = this;
    createViewer(modelName, models, containerId).then(function (hwv) {
        _this._viewer = hwv;
        
        function sceneReadyFunc() {
            var color = new Communicator.Color(0, 0, 0);
            _this._viewer.getView().setBackgroundColor(color, color);
            if (_this._defaultCamera != undefined) {
                _this._viewer.getView().setCamera(Communicator.Camera.construct(_this._defaultCamera));
            }

            loadModel();
        }
        
        function modelStructureReadyFunc() {
            ColorContour(_this._viewer, 25, 400);
            _this._viewer.getView().setPointSize(0, Communicator.PointSizeUnit.ScreenPixels);
        }

        function loadModel() {
            _this._viewer.model.clear().then(() => {
                const modelNodeId = _this._viewer.model.createNode(null, modelName);
                _this._viewer.model.loadSubtreeFromScsFile(modelNodeId, "/data/" + modelName + ".scs")
                    .then(() => {
                        _this._viewer.view.fitWorld();
                    });
            });
        }
        
        _this._viewer.setCallbacks({
            sceneReady: sceneReadyFunc,
            modelStructureReady: modelStructureReadyFunc,
        });
        
        var tt = new Tooltip(_this._viewer, _this._nodes);
        var tooltipHandle = _this._viewer.registerCustomOperator(tt);
        
        _this._annot = new AnnotationOperator(_this._viewer);
        let annotationHandle = _this._viewer.registerCustomOperator(_this._annot);
        
        _this._viewer.start();
        
        var OM = _this._viewer.operatorManager;
        OM.push(tooltipHandle);
    });
}