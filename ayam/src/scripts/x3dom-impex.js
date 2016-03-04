/** x3dom-impex.js
 * Improve x3dom 1.4 examine rotation interaction.
 * Just load this script after x3dom.js.
 * These changes to the original x3dom code are public domain.
 * Randolf Schultz, 2013
 */

x3dom.Viewarea.prototype.onMousePress = function (x, y, buttonState)
{
    this._needNavigationMatrixUpdate = true;

    this.prepareEvents(x, y, buttonState, "onmousedown");
    this._pickingInfo.lastClickObj = this._pickingInfo.pickObj;

    this._dx = 0;
    this._dy = 0;
    this._lastX = x;
    this._lastY = y;
    this._pressX = x;
    this._pressY = y;
    this._lastButton = buttonState;
    this._lastAlpha = 0.0;
};

x3dom.Viewarea.prototype.onDrag = function (x, y, buttonState)
{
    // should onmouseover/-out be handled on drag?
    this.handleMoveEvt(x, y, buttonState);

    var navi = this._scene.getNavigationInfo();
    if (navi._vf.type[0].length <= 1 || navi._vf.type[0].toLowerCase() === "none") {
        return;
    }

    var dx = x - this._lastX;
    var dy = y - this._lastY;
    var min, max, ok, d, vec;
    var viewpoint = this._scene.getViewpoint();

    if (navi._vf.type[0].toLowerCase() === "examine")
    {
        if (buttonState & 1) //left
        {
            var alpha = (dy * 2 * Math.PI) / this._width;
            var beta = (dx * 2 * Math.PI) / this._height;
	    this._lastAlpha += alpha;

            var mx = x3dom.fields.SFMatrix4f.rotationX(alpha);
            var my = x3dom.fields.SFMatrix4f.rotationY(beta);

            var mxb = x3dom.fields.SFMatrix4f.rotationY(-this._lastAlpha);
            var mxf = x3dom.fields.SFMatrix4f.rotationY(this._lastAlpha);

            var center = viewpoint.getCenterOfRotation();

            var mat = this.getViewMatrix();
            mat.setTranslate(new x3dom.fields.SFVec3f(0,0,0));

            this._rotMat = this._rotMat.
                mult(x3dom.fields.SFMatrix4f.translation(center)).
                mult(mat.inverse()).
		mult(mx).
                mult(mat).
                mult(x3dom.fields.SFMatrix4f.translation(center.negate()));

            this._rotMat = this._rotMat.
                mult(x3dom.fields.SFMatrix4f.translation(center)).
		mult(mxb).mult(my).mult(mxf).
                mult(x3dom.fields.SFMatrix4f.translation(center.negate()));

        }
        if (buttonState & 4) //middle
        {
			if (this._scene._lastMin && this._scene._lastMax)
			{
				d = (this._scene._lastMax.subtract(this._scene._lastMin)).length();
			}
			else
			{
				min = x3dom.fields.SFVec3f.MAX();
				max = x3dom.fields.SFVec3f.MIN();
				
				ok = this._scene.getVolume(min, max, true);
				if (ok) {
                    this._scene._lastMin = min;
                    this._scene._lastMax = max;
                }
				
				d = ok ? (max.subtract(min)).length() : 10;
			}
			d = ((d < x3dom.fields.Eps) ? 1 : d) * navi._vf.speed;

            vec = new x3dom.fields.SFVec3f(d*dx/this._width, d*(-dy)/this._height, 0);
            this._movement = this._movement.add(vec);

	    var mat = this.getViewpointMatrix().mult(this._transMat);
            //TODO; move real distance along viewing plane
            this._transMat = mat.inverse().
                mult(x3dom.fields.SFMatrix4f.translation(this._movement)).
                mult(mat);
        }
        if (buttonState & 2) //right
        {
			if (this._scene._lastMin && this._scene._lastMax)
			{
				d = (this._scene._lastMax.subtract(this._scene._lastMin)).length();
			}
			else
			{
				min = x3dom.fields.SFVec3f.MAX();
				max = x3dom.fields.SFVec3f.MIN();
				
				ok = this._scene.getVolume(min, max, true);
				if (ok) {
                    this._scene._lastMin = min;
                    this._scene._lastMax = max;
                }
				
				d = ok ? (max.subtract(min)).length() : 10;
			}
			d = ((d < x3dom.fields.Eps) ? 1 : d) * navi._vf.speed;

            vec = new x3dom.fields.SFVec3f(0, 0, d*(dx+dy)/this._height);
            this._movement = this._movement.add(vec);

	    var mat = this.getViewpointMatrix().mult(this._transMat);
            //TODO; move real distance along viewing ray
            this._transMat = mat.inverse().
                mult(x3dom.fields.SFMatrix4f.translation(this._movement)).
                mult(mat);
        }
    }

    this._dx = dx;
    this._dy = dy;

    this._lastX = x;
    this._lastY = y;
};
