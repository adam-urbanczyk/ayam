/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2016 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* NURBS for x3dom */

/*
  The NURBS Tesselator is based on idea and example code from
  A. J. Chung and A. J. Field (https://sourceforge.net/projects/emvise/)
*/

function findSpan(n, p, u, U)
{
 var low, mid, high;

  if(u >= U[n])
    return n;

  if(u <= U[p])
    return p;

  low = 0;
  high = n+1;
  mid = Math.floor((low+high)/2);

  while(u < U[mid] || u >= U[mid+1])
    {
      if(u < U[mid])
	high = mid;
      else
	low = mid;

      mid = Math.floor((low+high)/2);
    }

 return mid;
} /* findSpan */


function basisFuns(i, u, p, U)
{
 var N = [], left = [], right = [], saved, temp;
 var j, r;

  N[0] = 1.0;
  for(j = 0; j <= p; j++){
      left[j] = 0;
      right[j] = 0;
  }
  for(j = 1; j <= p; j++)
    {
      left[j] = u - U[i+1-j];
      right[j] = U[i+j] - u;
      saved = 0.0;

      for(r = 0; r < j; r++)
	{
	  temp = N[r] / (right[r+1] + left[j-r]);
	  N[r] = saved + right[r+1] * temp;
	  saved = left[j-r] * temp;
	}

      N[j] = saved;
    }

 return N;
} /* basisFuns */


function surfacePoint3DH(n, m, p, q, U, V, P, W, u, v)
{
 var spanu, spanv, indu, indv, l, k, i, j = 0;
 var Nu, Nv, C = [], Cw = [0.0,0.0,0.0,0.0], temp = [];

  spanu = findSpan(n, p, u, U);
  Nu = basisFuns(spanu, u, p, U);
  spanv = findSpan(m, q, v, V);
  Nv = basisFuns(spanv, v, q, V);

  indu = spanu - p;
  for(l = 0; l <= q; l++)
    {
      indv = spanv - q + l;
      for(k = 0; k < 4; k++)
	  temp[j+k] = 0.0;
      for(k = 0; k <= p; k++)
	{
	  i = indu+k+(indv*(n+1));
	  temp[j+0] += Nu[k]*P[i].x;
	  temp[j+1] += Nu[k]*P[i].y;
	  temp[j+2] += Nu[k]*P[i].z;
	  temp[j+3] += Nu[k]*W[i];
	}
      j += 4;
    }

  j = 0;
  for(l = 0; l <= q; l++)
    {
      Cw[0] += Nv[l]*temp[j+0];
      Cw[1] += Nv[l]*temp[j+1];
      Cw[2] += Nv[l]*temp[j+2];
      Cw[3] += Nv[l]*temp[j+3];
      j += 4;
    }

  for(j = 0; j < 3; j++)
    C[j] = Cw[j]/Cw[3];

 return new x3dom.fields.SFVec3f(C[0], C[1], C[2]);
} /* surfacePoint3DH */


function surfacePoint3D(n, m, p, q, U, V, P, u, v)
{
 var spanu, spanv, indu, indv, l, k, i, j = 0;
 var Nu, Nv, C = [0.0, 0.0, 0.0], temp = [];

  spanu = findSpan(n, p, u, U);
  Nu = basisFuns(spanu, u, p, U);
  spanv = findSpan(m, q, v, V);
  Nv = basisFuns(spanv, v, q, V);

  indu = spanu - p;
  for(l = 0; l <= q; l++)
    {
      indv = spanv - q + l;
      temp[j+0] = 0.0;
      temp[j+1] = 0.0;
      temp[j+2] = 0.0;
      for(k = 0; k <= p; k++)
	{
	  i = indu+k+(indv*(n+1));
	  temp[j+0] += Nu[k]*P[i].x;
	  temp[j+1] += Nu[k]*P[i].y;
	  temp[j+2] += Nu[k]*P[i].z;
	}
      j += 3;
    }

  j = 0;
  for(l = 0; l <= q; l++)
    {
      C[0] += Nv[l]*temp[j+0];
      C[1] += Nv[l]*temp[j+1];
      C[2] += Nv[l]*temp[j+2];
      j += 3;
    }

 return new x3dom.fields.SFVec3f(C[0], C[1], C[2]);
} /* surfacePoint3D */


function curvePoint2DH(n, p, U, P, W, u)
{
 var span, j, k;
 var N, Cw = [0.0, 0.0, 0.0], C = [];

  span = findSpan(n, p, u, U);
  N = basisFuns(span, u, p, U);

  for(j = 0; j <= p; j++)
    {
      k = (span-p+j)*2;
      Cw[0] = Cw[0] + N[j]*P[k];
      Cw[1] = Cw[1] + N[j]*P[k+1];
      Cw[2] = Cw[2] + N[j]*W[span-p+j];
    }

  for(j = 0; j < 2; j++)
    C[j] = Cw[j]/Cw[2];

 return C;
} /* curvePoint2DH */


function curvePoint2D(n, p, U, P, u)
{
 var span, j, k;
 var N, C = [0.0, 0.0];

  span = findSpan(n, p, u, U);
  N = basisFuns(span, u, p, U);

  for(j = 0; j <= p; j++)
    {
      k = (span-p+j)*2;
      C[0] = C[0] + N[j]*P[k];
      C[1] = C[1] + N[j]*P[k+1];
    }

 return C;
} /* curvePoint2D */


function Tesselator(lnn) {
    this.edge_thresh = 0.1;
    this.trim_thresh = 0.1;
    this.split_bias = 0.7;
    this.skew_thresh = 0.0001;

    this.w = lnn._vf.uDimension-1;
    this.h = lnn._vf.vDimension-1;
    this.p = lnn._vf.uOrder-1;
    this.q = lnn._vf.vOrder-1;
    this.U = lnn._vf.uKnot;
    this.V = lnn._vf.vKnot;
    var coordNode = lnn._cf.controlPoint.node;
    x3dom.debug.assert(coordNode);
    this.P = coordNode.getPoints();
    x3dom.debug.assert(this.P);
    this.W = lnn._vf.weight;
    this.surfaceHash = [];
    this.indexHash = [];
    this.curveHash = null;
    this.coordinates = [];
    this.indices = [];
    this.coordIndex = 0;

    this.tesselate = function () {
	var u0 = this.U[this.p];
	var u1 = this.U[this.U.length-this.p];
	var v0 = this.V[this.q];
	var v1 = this.V[this.V.length-this.q];
	this.tessTri([[u0,v0],[u0,v1],[u1,v0]]);
	this.tessTri([[u1,v1],[u1,v0],[u0,v1]]);
    }

    this.tessTri = function (tri) {
	var work = [tri];
	while( work.length ) {
	    var cur = work.splice(0, 1);
	    var pieces = this.refineTri(cur[0]);
	    work = pieces.concat(work);
	}
    } /* tessTri */

    this.refineTri = function (tri) {
	/* cull entire tile? */
	if (this.tloops && this.inOut(tri) < 0)
	    return [];

	/***** Measure facet degeneracy ****/

	//area of triangle
	var area = tri[0][0]*tri[1][1] - tri[1][0]*tri[0][1]
	    + tri[1][0]*tri[2][1] - tri[2][0]*tri[1][1]
	    + tri[2][0]*tri[0][1] - tri[0][0]*tri[2][1];
	if (area < 0)
	    area = -area;

	//calc sum of squares of edge lengths
	var a = [], b = [];
	a[0] = tri[0][0] - tri[1][0];
	a[1] = tri[0][1] - tri[1][1];
	var max_ed = a[0]*a[0] + a[1]*a[1];
	a[0] = tri[1][0] - tri[2][0];
	a[1] = tri[1][1] - tri[2][1];
	max_ed += a[0]*a[0] + a[1]*a[1];
	a[0] = tri[2][0] - tri[0][0];
	a[1] = tri[2][1] - tri[0][1];
	max_ed += a[0]*a[0] + a[1]*a[1];

	area /= max_ed;
	if (area <= this.skew_thresh) {
	    this.diceTri(tri);
	    return [];
	}

	//split edges
	var eds = [];
	max_ed = 0.0;
	for (var i = 0; i < 3; i++) {
	    var j0 = (i+1)%3;
	    var j1 = (i+2)%3;
	    a = tri[j0];
	    b = tri[j1];
	    eds[i] = this.splitEdge(a, b);
	    if (eds[i] > max_ed)
		max_ed = eds[i];
	}

	max_ed *= this.split_bias;

	var m = [], mv = [], co = 0;
	for (var i = 0; i < 3; i++) {
	    var j0 = (i+1)%3;
	    var j1 = (i+2)%3;

	    if ((eds[i] > this.edge_thresh) && (eds[i] >= max_ed)) {
		co++;
		mv[i] = [];
		mv[i][0] = 0.5*(tri[j0][0] + tri[j1][0]);
		mv[i][1] = 0.5*(tri[j0][1] + tri[j1][1]);
		m[i] = i - 3;
	    }
	    else {
		//move midpt to vertex closer to center
		if (eds[j0] > eds[j1]) {
		    mv[i] = tri[j0];
		    m[i] = j0;
		} else {
		    mv[i] = tri[j1];
		    m[i] = j1;
		}
	    }
	}
	var res = [];
	if (co) {
	    //add one for center tile
	    co++;

	    //corner tiles
	    var j = co;
	    for (var i = 0; i < 3; i++) {
		var j0 = (i+1)%3;
		var j1 = (i+2)%3;
		if ((m[j1] != i) && (m[j0] != i)) {
		    res[--j] = [tri[i], mv[j1], mv[j0]];
		}
	    }

	    //center tile
	    if (j) {
		if ((m[0]==m[1]) || (m[1]==m[2]) || (m[2]==m[0])) {
		    //cerr << "degenerate center tile\n";
		    return [];
		}
		res[0] = [mv[0], mv[1], mv[2]];
	    }
	    return res;
	}
	else if (this.splitCenter( mv )) {
	    //no edges split; add vertex to center?
	    a[0] = (1./3.)*(tri[0][0] + tri[1][0] + tri[2][0]);
	    a[1] = (1./3.)*(tri[0][1] + tri[1][1] + tri[2][1]);

	    for(var i = 0; i < 3; i++) {
		res[i] = [tri[i], tri[(i+1)%2], a];
	    }
	    return res;
	}

	this.trimFinal(tri);

	return [];
    } /* refineTri */

    this.diceTri = function ( tri ) {
	//pick a central point
	var cv = [];
	cv[0] = (tri[0][0] + tri[1][0] + tri[2][0])/3.0;
	cv[1] = (tri[0][1] + tri[1][1] + tri[2][1])/3.0;
	this.computeSurface(cv);
	for(var ed = 0; ed < 3; ed++) {
	    var divs = [], d = [];
	    var e1 = (ed+1)%3;

	    d[0] = tri[e1][0] - tri[ed][0];
	    d[1] = tri[e1][1] - tri[ed][1];

	    divs[0] = 1.0;
	    var beg = 0.0;
	    while( divs.length ) {
		var a = [], b = [];
		var end = divs[0];
		a[0] = tri[ed][0] + d[0]*beg;
		a[1] = tri[ed][1] + d[1]*beg;
		b[0] = tri[ed][0] + d[0]*end;
		b[1] = tri[ed][1] + d[1]*end;
		if (this.splitEdge(a, b) > this.edge_thresh) {
		    //split edge
		    divs.splice(0, 0, 0.5*(beg+end));
		} else {
		    //render it
		    this.computeSurface(a);
		    this.computeSurface(b);
		    var slice = [cv, a, b];
		    this.trimFinal(slice);
		    divs.pop();
		    beg = end;
		}
	    }
	}
    } /* diceTri */

    this.computeSurface = function(uv) {
	// first try the hash
	var indu = Math.floor(uv[0]*10e6);
	var indv = Math.floor(uv[1]*10e6);
	if(this.surfaceHash[indu]) {
	    var memoizedPoint = this.surfaceHash[indu][indv];
	    if(memoizedPoint)
		return memoizedPoint;
	}
	// hash lookup failed, compute the point
	var pnt;
	if (this.W) {
	    pnt = surfacePoint3DH(this.w, this.h, this.p, this.q,
				  this.U, this.V, this.P, this.W,
				  uv[0], uv[1]);
	} else {
	    pnt = surfacePoint3D(this.w, this.h, this.p, this.q,
				 this.U, this.V, this.P,
				 uv[0], uv[1]);
	}

	if(this.curveHash)
	    return pnt;

	// memoize pnt
	if(!this.surfaceHash[indu])
	    this.surfaceHash[indu] = [];
	this.surfaceHash[indu][indv] = pnt;

	if(!this.indexHash[indu])
	    this.indexHash[indu] = [];
	this.indexHash[indu][indv] = this.coordIndex;
	this.coordIndex++;
	this.coordinates.push(pnt);

     return pnt;
    } /* computeSurface */

    this.computeCurve = function(loop, seg, u) {
	// first try the hash
	var indu = Math.floor(u*10e6);
	if(this.curveHash[loop][seg]) {
	    var memoizedPoint = this.curveHash[loop][seg][indu];
	    if(memoizedPoint)
		return memoizedPoint;
	}
	// hash lookup failed, compute the point
	var pnt, crv = this.tloops[loop][seg];
	if(crv[4].length){
	    pnt = curvePoint2DH(crv[0], crv[1], crv[2], crv[3], crv[4], u);
	} else {
	    pnt = curvePoint2D(crv[0], crv[1], crv[2], crv[3], u);
	}
	// memoize pnt
	if(!this.curveHash[loop][seg])
	    this.curveHash[loop][seg] = [];
	this.curveHash[loop][seg][indu] = pnt;
     return pnt;
    } /* computeCurve */

    /*
      User function that decides if an edge should be split
      Return value gives a measure on which to prioritise edge splitting
      (e.g. edge length) and values below the set threshold will not be
      split; must be commutative
    */
    this.splitEdge = function (a, b) {
	var pa = this.computeSurface(a);
	var pb = this.computeSurface(b);
	if(Math.abs(pa.x-pb.x) > 10e-6 ||
	   Math.abs(pa.y-pb.y) > 10e-6 ||
	   Math.abs(pa.z-pb.z) > 10e-6)
	    return Math.sqrt((pa.x-pb.x)*(pa.x-pb.x)+
			     (pa.y-pb.y)*(pa.y-pb.y)+
			     (pa.z-pb.z)*(pa.z-pb.z));
	else
	    return 0.0;
    } /* splitEdge */

    /*
      To avoid missing high frequency detail (e.g. spikes in functions)
      should all three edges fall below the split threshold, this function
      is called to determine if the triangle should be split by adding
      a vertex to its center.  An interval bound on the function over the
      domain of the triangle is one possible immplementation.
    */
    this.splitCenter = function (tri) {
	return false;
    } /* splitCenter */

    /*
      User supplied function typically for rendering the refined tile
      once all edges match the acceptance criteria.
    */
    this.renderFinal = function (tri) {
	for(var i = 0; i < 3; i++) {
	    var uv = tri[i];
	    var indu = Math.floor(uv[0]*10e6);
	    var indv = Math.floor(uv[1]*10e6);
	    if(this.indexHash[indu])
		this.indices.push(this.indexHash[indu][indv]);
	}
    } /* renderFinal */

    /*
      User supplied function for rendering the trimmed tile.
    */
    this.renderTrimmed = function (tri) {
	this.renderFinal(tri);
    } /* renderTrimmed */

    this.inOut = function (tri, ips) {
	var a = [], ad = [];

	//counters for intersections
	var cl = []; //less
	var cg = []; //greater
	var ndx = [];

	for(var i = 0; i < 3; i++) {
	    a[i] = [];
	    var va = tri[i];
	    var vb = tri[(i+1)%3];

	    //make line equation for edge
	    a[i][0] = va[1] - vb[1];
	    a[i][1] = vb[0] - va[0];
	    ad[i] = a[i][0]*va[0] + a[i][1]*va[1];

	    cl[i] = cg[i] = 0;
	    ndx[i] = (Math.abs(a[i][0]) > Math.abs(a[i][1])) ? 1 : 0;
	}

	for(var ilp = 0; ilp < this.ttloops.length; ilp++) {
	    var lp = this.ttloops[ilp];

	    for(var k = 0; k < lp.length-1; k++ ) {
		var p0 = lp[k];
		var j = k+1;
		var p1 = lp[j];
		var ni = 0;
		for(var i = 0; i < 3; i++) {
		    var d0 = p0[0]*a[i][0] + p0[1]*a[i][1] - ad[i];
		    var d1 = p1[0]*a[i][0] + p1[1]*a[i][1] - ad[i];

		    ni += (d0<0) ? 1 : -1;

		    if ((d0<0) ? (d1<0) : (d1>=0))
			continue;//no intersection

		    //find intersection point
		    var ip = (p1[ndx[i]]*d0 - p0[ndx[i]]*d1) / (d0-d1);
		    var ba = ip<tri[i][ndx[i]];
		    var bb = ip<tri[(i+1)%3][ndx[i]];
		    if (ba && bb) {
			cl[i]++;
		    } else {
			if (!(ba || bb)) {
			    cg[i]++;
			} else {
			    return 0;
			}
		    }
		}

		//point inside tile
		if ((ni == 3) || (ni == -3))
		    return 0;
	    }
	}

	if ((cl[0]&1) && (cl[1]&1) && (cl[2]&1)) return 1;
	if ( !((cl[0]&1) || (cl[1]&1) || (cl[2]&1)) ) return -1;

     return 0;
    } /* inOut */

    /*
      Function to decide if straight edge approximation of the trim curve
      should be refined more.  A default function using
      splitEdge() is provided but can be overridden by the user
    */
    this.refineTrim = function (loop, u1, u2) {
	var pa = this.computeCurve(loop, Math.floor(u1), u1);
	var pb = this.computeCurve(loop, Math.floor(u2), u2);
	return (this.splitEdge(pa, pb) > this.edge_thresh);
    }

    /* Construct straight edged approximation of the trimming curves */
    this.initTrims = function ( ) {
	this.ttloops = [];
	this.curveHash = [];
	var edt = this.edge_thresh;
	this.edge_thresh = this.trim_thresh;
	for(var ilp = 0; ilp < this.tloops.length; ilp++) {
	    var lp = this.tloops[ilp];
	    this.curveHash[ilp] = [];
	    this.ttloops[ilp] = [];
	    var ttus = [];
	    var pnts = [];
	    var ue;
	    // sample trim segments at every distinct knot
	    for(var j = 0; j < lp.length; j++) {
		var U = lp[j][2];

		// rewrite knots to form a continuous range over the loop;
		// this way, we can derive the segment from u: Math.floor(u)
		var uf = 1.0/(U[U.length-1]-U[0]);
		if(Math.abs(1.0-uf) > 10e-12)
		    for(var i = 0; i < U.length; i++)
			U[i] = (U[i]*uf);
		var ud = U[0]-j;
		if(Math.abs(ud) > 10e-12)
		    for(var i = 0; i < U.length; i++)
			U[i] = (U[i]-ud);

		var ui = lp[j][1];//p
		var u = U[ui];//U[p]
		ue = U[(U.length-ui)];//U[Ulen-p]
		while(u < ue) {
		    this.ttloops[ilp].push(this.computeCurve(ilp, j, u));
		    ttus.push(u);
		    ui++;
		    while(Math.abs(u-U[ui]) < 10e-6)
			ui++;
		    u = U[ui];
		}
	    }

	    // refine trim edges
	    var tlp = this.ttloops[ilp];

	    var x = 0;
	    while( x < tlp.length ) {
		var p0u = ttus[x];
		var p0seg = Math.floor(p0u);
		var y = x+1;
		if(y == tlp.length){
		    y = 0;
		}
		var p1u = ttus[y];

		if(lp[p0seg][1] > 1 && this.refineTrim(ilp, p0u, p1u)) {
		    //split edge
		    var um;
		    if(y == 0) {
			um = 0.5 * (p0u + ue);
			y = x+1;
		    } else {
			um = 0.5 * (p0u + p1u);
		    }
		    var v = this.computeCurve(ilp, p0seg, um);
		    tlp.splice(y, 0, v)
		    ttus.splice(y, 0, um);
		} else {
		    // proceed to next edge
		    if(y == 0)
			break;
		    x = y;
		}
	    } // foreach edge
	    tlp.push(tlp[0]);

	} // foreach loop
	this.curveHash = null;
	this.edge_thresh = edt;
    } /* initTrims */

    this.trimFinal = function (tri) {
	if (this.tloops && this.inOut(tri) == 0)
	    this.renderTrimmed(tri);
	else
	    this.renderFinal(tri);
    } /* trimFinal */
} /* Tesselator */


x3dom.registerNodeType(
    "NurbsPatchSurface",
    "Rendering",
    defineClass(x3dom.nodeTypes.X3DComposedGeometryNode,
        function (ctx) {
            x3dom.nodeTypes.NurbsPatchSurface.superClass.call(this, ctx);

            this.addField_SFInt32(ctx, 'uDimension', 0);
            this.addField_SFInt32(ctx, 'vDimension', 0);
            this.addField_SFInt32(ctx, 'uOrder', 3);
            this.addField_SFInt32(ctx, 'vOrder', 3);
            this.addField_MFFloat(ctx, "uKnot", []);
            this.addField_MFFloat(ctx, "vKnot", []);
            this.addField_MFFloat(ctx, "weight", []);

            this.addField_SFNode ('controlPoint',
				  x3dom.nodeTypes.X3DCoordinateNode);

            this._needReRender = true;
	    this.myctx = ctx;
        },
        {
            nodeChanged: function() {
                this._needReRender = true;
		this._vf.ccw = false;
		this._vf.solid = false;

		var tess = new Tesselator(this);
		tess.tesselate();

		var its = new x3dom.nodeTypes.IndexedTriangleSet();
		its._nameSpace = this._nameSpace;
		its._vf.solid = false;
		its._vf.ccw = false;
		its._vf.index = tess.indices;

		var co = new x3dom.nodeTypes.Coordinate();
		co._nameSpace = this._nameSpace;
		co._vf.point =
		    new x3dom.fields.MFVec3f(tess.coordinates);

		its.addChild(co)
		its.nodeChanged();

		its._xmlNode = this._xmlNode;
		this._mesh = its._mesh;
            },

            fieldChanged: function(fieldName) {
		//nodeChanged();
            }

        }
    )
);

x3dom.registerNodeType(
    "NurbsCurve2D",
    "Grouping",
    defineClass(x3dom.nodeTypes.X3DGroupingNode,
        function (ctx) {
            x3dom.nodeTypes.NurbsCurve2D.superClass.call(this, ctx);

            this.addField_SFInt32(ctx, 'order', 3);
            this.addField_MFFloat(ctx, "knot", []);
            this.addField_MFFloat(ctx, "controlPoint", []);
            this.addField_MFFloat(ctx, "weight", []);
	}, { }
    )
);

x3dom.registerNodeType(
    "Contour2D",
    "Grouping",
    defineClass(x3dom.nodeTypes.X3DGroupingNode,
        function (ctx) {
            x3dom.nodeTypes.Contour2D.superClass.call(this, ctx);
            this.addField_MFNode('children', x3dom.nodeTypes.NurbsCurve2D);
	}, { }
    )
);

x3dom.registerNodeType(
    "NurbsTrimmedSurface",
    "Rendering",
    defineClass(x3dom.nodeTypes.NurbsPatchSurface,
        function (ctx) {
            x3dom.nodeTypes.NurbsTrimmedSurface.superClass.call(this, ctx);

            this.addField_MFNode ('trimmingContour', x3dom.nodeTypes.Contour2D);

            this._needReRender = true;
	    this.myctx = ctx;
        },
        {
            nodeChanged: function() {
                this._needReRender = true;
		this._vf.ccw = false;
		this._vf.solid = false;

		var tess = new Tesselator(this);
		if(this._cf.trimmingContour &&
		   this._cf.trimmingContour.nodes.length){
		    tess.tloops = [];
		    var len = this._cf.trimmingContour.nodes.length;
		    for(var i = 0; i < len; i++) {
			var c2dnode = this._cf.trimmingContour.nodes[i];
			if(c2dnode._cf.children) {
			    tess.tloops[i] = [];
			    var trim = c2dnode._cf.children.nodes;
			    for(var j = 0; j < trim.length; j++) {
				var tc = trim[j];
				tess.tloops[i].push([
				    tc._vf.controlPoint.length-1,
				    tc._vf.order-1, tc._vf.knot,
				    tc._vf.controlPoint, tc._vf.weight]);
			    }
			}
		    }
		    tess.initTrims();
		}
		tess.tesselate();

		var its = new x3dom.nodeTypes.IndexedTriangleSet();
		its._nameSpace = this._nameSpace;
		its._vf.solid = false;
		its._vf.ccw = false;
		its._vf.index = tess.indices;
		x3dom.debug.logInfo("num triangles:"+tess.indices.length/3);
		var co = new x3dom.nodeTypes.Coordinate();
		co._nameSpace = this._nameSpace;
		co._vf.point =
		    new x3dom.fields.MFVec3f(tess.coordinates);

		its.addChild(co)
		its.nodeChanged();

		its._xmlNode = this._xmlNode;
		this._mesh = its._mesh;
            },

            fieldChanged: function(fieldName) {
		//nodeChanged();
            }

        }
    )
);
