function DimSelectOperator(viewer) {
    this._viewer = viewer;
    this._dimColor;
    this._command;
}

DimSelectOperator.prototype.setDimColor = function (color) {
    var _this = this;
    _this._dimColor = color;
    _this._command = "check";
};

DimSelectOperator.prototype.setCommand = function (command) {
    var _this = this;
    _this._command = command;
};

DimSelectOperator.prototype._onSelect = function (event) {
    var _this = this;
    
    var point = event.getPosition();
    var items = _this._viewer.getMeasureManager().getAllMeasurements();
    for (var i = 0; i < items.length; i++) {
        var item = items[i];
        
        var className = item.getClassName();
        var position;
        if (className == "Communicator.Markup.Measure.MeasureStraightEdgeLengthMarkup" || 
            className == "Communicator.Markup.Measure.MeasurePointPointDistanceMarkup")
            var position = item._positions[2];
        else if (className == "Communicator.Markup.Measure.MeasureCircleEdgeLengthMarkup")
            var position = item._positions[4];
        else if (className == "Communicator.Markup.Measure.MeasureFaceFaceAngleMarkup" || 
                 className == "Communicator.Markup.Measure.MeasureFaceFaceDistanceMarkup")
            var position = item._textPos;
        else
            continue;
        
        var position2 = Communicator.Point2.fromPoint3(_this._viewer.getView().projectPoint(position));
        
        if (point.x < position2.x)
            continue;
        if (point.x > position2.x + 60)
            continue;
        if (point.y < position2.y)
            continue;
        if (point.y > position2.y + 23)
            continue;
        
        if (_this._command == "check") {
            item._textShape._box._fillColor.r = _this._dimColor.r;
            item._textShape._box._fillColor.g = _this._dimColor.g;
            item._textShape._box._fillColor.b = _this._dimColor.b;
        } else if (_this._command == "delete") {
            _this._viewer.getMeasureManager().removeMeasurement(item);
        }
    }
};

DimSelectOperator.prototype.onTouchEnd = function (event) {
    var _this = this;
    _this._onSelect(event);
};

DimSelectOperator.prototype.onMouseUp = function (event) {
    var _this = this;
    _this._onSelect(event);    
};

class Tooltip {
    constructor(viewer, nodes) {
        this._viewer = viewer;
        this._nodes = nodes;
    }

    onMouseMove (event) {
        var _this = this;
        
        var pickConfig = new Communicator.PickConfig(Communicator.SelectionMask.Point);
        _this._viewer.getView().pickFromPoint(event.getPosition(), pickConfig).then(function (selectionItem) {
            var p = selectionItem.getPointEntity();
            if (p != null) {
                var i = p.getPointId();
                var node = _this._nodes[i];
                $("#result0").text("Node no.: " + node.id);
                $("#result1").text("Mises stress: " + node.mises.toFixed(2) + "(MPa)");
                $("#result2").text("Tresca stress: " + node.tresca.toFixed(2) + "(MPa)");
                $("#result3").text("Sigma 1: " + node.sigma1.toFixed(2) + "(MPa)");
                $("#result4").text("Sigma 2: " + node.sigma2.toFixed(2) + "(MPa)");
                $("#result5").text("Sigma 3: " + node.sigma3.toFixed(2) + "(MPa)");
                $("#result6").text("Prinsiple strain: " + node.strain.toFixed(6) + "(mm)");
            }
        });
    }
}

function crossMarkup(viewer, point) {
    this._viewer = viewer;
    this._point = point.copy();
    this._line1 = new Communicator.Markup.Shape.Line();
    this._line2 = new Communicator.Markup.Shape.Line();
    this._line1.setStrokeColor(new Communicator.Color(255, 0, 0));
    this._line2.setStrokeColor(new Communicator.Color(255, 0, 0));
}

crossMarkup.prototype.draw = function () {
    var _this = this;
    var p_st = Communicator.Point2.fromPoint3(_this._viewer.getView().projectPoint(_this._point));
    var p_en = Communicator.Point2.fromPoint3(_this._viewer.getView().projectPoint(_this._point));
    
    var size = 5;
    
    var p1_st = p_st.copy();
    var p1_en = p_en.copy();
    p1_st.x -= size;
    p1_en.x += size;
    _this._line1.set(p1_st, p1_en);
    
    var p2_st = p_st.copy();
    var p2_en = p_en.copy();
    p2_st.y -= size;
    p2_en.y += size;
    _this._line2.set(p2_st, p2_en);
    
    _this._viewer.markupManager.getRenderer().drawLine(_this._line1);
    _this._viewer.markupManager.getRenderer().drawLine(_this._line2);
};

crossMarkup.prototype.hit = function () {
    return false;
}

function AnnotationOperator(viewer) {
    this._viewer = viewer;
    this._annotationMap = {};
    this._activeMarkup = false;
    this._activeMarkupHandle = "";
    this._previousAnchorPlaneDragPoint;
    this._isClicked = 0;
}

AnnotationOperator.prototype._onStart = function (event) {
    var _this = this;
    
    var downPosition = event.getPosition();
    if (!this._selectAnnotation(downPosition)) {
        var config = new Communicator.PickConfig(Communicator.SelectionMask.Face);
        this._viewer.getView().pickFromPoint(event.getPosition(), config).then(function (selectionItem) {
            var nodeId = selectionItem.getNodeId();
            if (nodeId > 0) {
                var selectionPosition = selectionItem.getPosition();
                var annotationMarkup = new AnnotationMarkup(_this._viewer, selectionPosition, "", selectionPosition);
                var markupHandle = _this._viewer.markupManager.registerMarkup(annotationMarkup);
                _this._annotationMap[markupHandle] = annotationMarkup;
                
                _this._activeMarkup = annotationMarkup;
                _this._activeMarkupHandle = markupHandle;
                _this._previousAnchorPlaneDragPoint = _this._getDragPointOnAnchorPlane(downPosition);                
            }
        });
    }
};

AnnotationOperator.prototype.onMouseDown = function (event) {
    var _this = this;
    
    var mouseButton = event.getButton();
    if (mouseButton != Communicator.Button.Left)
        return;
    _this._onStart(event);
};

AnnotationOperator.prototype.onTouchStart = function (event) {
    var _this = this;
    
    _this._onStart(event);
};

AnnotationOperator.prototype._selectAnnotation = function (selectPoint) {
    var _this = this;
    
    var markup = this._viewer.markupManager.pickMarkupItem(selectPoint);
    if (markup) {
        this._activeMarkup = markup;
        this._previousAnchorPlaneDragPoint = this._getDragPointOnAnchorPlane(selectPoint);
        _this._isClicked = 0;
        return true;
    } else {
        return false;
    }
    
}

AnnotationOperator.prototype._onMove = function (event) {
    var _this = this;
    
    if (this._activeMarkup) {
        var currentAnchorPlaneDragPoint = this._getDragPointOnAnchorPlane(event.getPosition());
        var dragDelta = Communicator.Point3.subtract(currentAnchorPlaneDragPoint, this._previousAnchorPlaneDragPoint);
        var newAnchorPos = this._activeMarkup.getTextBoxAnchor().add(dragDelta);
        this._activeMarkup.setTextBoxAnchor(newAnchorPos);
        this._previousAnchorPlaneDragPoint.assign(currentAnchorPlaneDragPoint);
        this._viewer.markupManager.refreshMarkup();
        if (_this._activeMarkupHandle == "")
            _this._isClicked++;
        
        event.setHandled(true);
    }
};

AnnotationOperator.prototype.onMouseMove = function (event) {
    var _this = this;
    
   _this._onMove(event);
};

AnnotationOperator.prototype.onTouchMove = function (event) {
    var _this = this;
    
    _this._onMove(event);
}

AnnotationOperator.prototype._onEnd = function (event) {
    var _this = this;

    if (_this._activeMarkup && _this._isClicked <= 5) {
        var label = window.prompt("input text", _this._activeMarkup.getLabel());
        if(label == null){
            _this._viewer.markupManager.unregisterMarkup(_this._activeMarkupHandle);
            delete _this._annotationMap[_this._activeMarkupHandle];
        } else {
            _this._activeMarkup.setLabel(label)
            _this._activeMarkup.setValues();
            _this._viewer.markupManager.refreshMarkup();
        }
    }
    _this._activeMarkup = false;
    _this._activeMarkupHandle = "";
    _this._isClicked = 0;
};

AnnotationOperator.prototype.onMouseUp = function (event) {
    var _this = this;
    
    _this._onEnd(event);
};

AnnotationOperator.prototype.onTouchEnd = function (event) {
    var _this = this;
    
    _this._onEnd(event);
};

AnnotationOperator.prototype._getDragPointOnAnchorPlane = function (screenPoint) {
    var anchor = this._activeMarkup.getLeaderLineAnchor();
    var camera = this._viewer.getView().getCamera();
    var normal = Communicator.Point3.subtract(camera.getPosition(), anchor).normalize();
    var anchorPlane = Communicator.Plane.createFromPointAndNormal(anchor, normal);
    var raycast = this._viewer.getView().raycastFromPoint(screenPoint);
    var intersectionPoint = Communicator.Point3.zero();
    if (anchorPlane.intersectsRay(raycast, intersectionPoint)) {
        return intersectionPoint;
    }
    else {
        return null;
    }
};

AnnotationOperator.prototype.serialize = function () {
    var _this = this;
    var annotations = [];
    for (var key in _this._annotationMap) {
        var annot = _this._annotationMap[key];
        annotations.push(annot.selialize());
    }
    return annotations;
};

AnnotationOperator.prototype.createMarkup = function (label, labelAncor, textBoxAncor) {
    var _this = this;
    var annotationMarkup = new AnnotationMarkup(_this._viewer, labelAncor, label, textBoxAncor);
    var markupHandle = _this._viewer.markupManager.registerMarkup(annotationMarkup);
    _this._annotationMap[markupHandle] = annotationMarkup;
};

AnnotationOperator.prototype.deleteAll = function() {
    var _this = this;
    var MM = _this._viewer.markupManager;
    for (var key in _this._annotationMap) {
        MM.unregisterMarkup(key);
        delete _this._annotationMap[key];
    }
}

//
function AnnotationMarkup(viewer, anchorPoint, label, textBoxPoint) {
    this._leaderLine = new Communicator.Markup.Shape.Line();
    this._text1 = new Communicator.Markup.Shape.Text();
    this._frame = new Communicator.Markup.Shape.Rectangle();
    this._frame.setFillOpacity(1);
    this._frame.setFillColor(new Communicator.Color(255, 255, 255));
    this._frame.setStrokeColor(new Communicator.Color(255, 0, 0));
    this._viewer = viewer;
    this._leaderAnchor = anchorPoint.copy();
    this._textBoxAnchor = textBoxPoint.copy();
    this._text1.setText(label);
    this._leaderLine.setStartEndcapType(Communicator.Markup.Shape.EndcapType.Arrowhead);
    this._leaderLine.setStartEndcapColor(new Communicator.Color(255, 0, 0));
    this._leaderLine.setStrokeColor(new Communicator.Color(255, 0, 0));
    this.setValues();
}

AnnotationMarkup.prototype.draw = function () {
    this._behindView = false;
    var leaderPoint3d = this._viewer.getView().projectPoint(this._leaderAnchor);
    var boxAnchor3d = this._viewer.getView().projectPoint(this._textBoxAnchor);
    if (leaderPoint3d.z <= 0.0)
        this._behindView = true;
    if (boxAnchor3d.z <= 0.0)
        this._behindView = true;
    var leaderPoint2d = Communicator.Point2.fromPoint3(leaderPoint3d);
    var boxAnchor2d = Communicator.Point2.fromPoint3(boxAnchor3d);
    
    var frameSize = this._frame.getSize();
    
    var leaderEnd = boxAnchor2d.copy();
    if ((boxAnchor2d.x + frameSize.x / 2) < leaderPoint2d.x) {
        leaderEnd.x += frameSize.x;
    }
    if ((boxAnchor2d.y - frameSize.y / 2) < leaderPoint2d.y) {
        leaderEnd.y += frameSize.y;
    }
    this._leaderLine.set(leaderPoint2d, leaderEnd);
    
    var textAncor = boxAnchor2d.copy();
    textAncor.x += 1;
    textAncor.y += 1;
    this._text1.setPosition(textAncor);
    this._frame.setPosition(boxAnchor2d);
    var renderer = this._viewer.markupManager.getRenderer();
    renderer.drawRectangle(this._frame);
    renderer.drawLine(this._leaderLine);
    renderer.drawText(this._text1);
};

AnnotationMarkup.prototype.hit = function (point) {
    var measurement = this._frame.getSize();
    var position = this._text1.getPosition();
    if (point.x < position.x)
        return false;
    if (point.x > position.x + measurement.x)
        return false;
    if (point.y < position.y)
        return false;
    if (point.y > position.y + measurement.y)
        return false;
    return true;
};

AnnotationMarkup.prototype.getLeaderLineAnchor = function () {
    return this._leaderAnchor.copy();
};

AnnotationMarkup.prototype.getTextBoxAnchor = function () {
    return this._textBoxAnchor;
};

AnnotationMarkup.prototype.setTextBoxAnchor = function (newAnchorPoint) {
    this._textBoxAnchor.assign(newAnchorPoint);
};

AnnotationMarkup.prototype.getLabel = function () {
    return this._text1.getText();
};

AnnotationMarkup.prototype.setLabel = function (label) {
    this._text1.setText(label);
};

AnnotationMarkup.prototype.selialize = function () {
    return {
        label: this.getLabel(),
        leaderAnchor: this._leaderAnchor.forJson(),
        textBoxAnchor: this._textBoxAnchor.forJson(),
    };
}

AnnotationMarkup.prototype.setValues = function() {
    var renderer = this._viewer.markupManager.getRenderer();
    var measure1 = renderer.measureText(this._text1.getText(), this._text1);
    this._frame.setSize(new Communicator.Point2(measure1.x + 4, measure1.y + 6));
}
