#ifndef __nurbs_h__
#define __nurbs_h__
/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2011 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* nurbs.h - nurbs prototypes */

/* act.c */

/** Do a least squares approximation.
 */
int ay_act_leastSquares(double *Q, int m, int n, int p,
			double **U, double **P);

/** Do a least squares approximation for a closed curve.
 */
int ay_act_leastSquaresClosed(double *Q, int m, int n, int p,
			      double **U, double **P);

/** Resize a approximating curve.
 */
int ay_act_resize(ay_acurve_object *curve, int new_length);

/** Revert a approximating curve.
 */
int ay_act_revert(ay_acurve_object *curve);

/** Get point from approximating curve.
 */
int ay_act_getpntfromindex(ay_acurve_object *curve, int index, double **p);


/* bevelt.c */

/** Parse BP (BevelParameter) tags.
 */
void ay_bevelt_parsetags(ay_tag *tag, ay_bparam *params);

/** Add bevels to a surface object.
 */
int ay_bevelt_addbevels(ay_bparam *bparams, ay_cparam *cparams, ay_object *o,
			ay_object **dst);

/** Create bevel surface from cross section bevel curve.
 */
int ay_bevelt_createc(double radius, ay_object *o1, ay_object *o2,
		      ay_nurbpatch_object **bevel);

/** Create bevel surface from 3D curve and cross section bevel curve.
 */
int ay_bevelt_createc3d(double radius, int revert,
			ay_object *o1, ay_object *o2,
			double *n, int nstride,
			double *t, int tstride,
			ay_nurbpatch_object **bevel);

/** Create bevel surface from 3D curve that rounds to the cap.
 */
int ay_bevelt_createroundtocap(double radius, int revert,
			       ay_object *o1, double *t, int tstride,
			       ay_nurbpatch_object **bevel);

/** Create bevel surface from 3D curve that rounds to the mean normal.
 */
int ay_bevelt_createroundtonormal(double radius, int revert, ay_object *o1,
			      double *nt, int nstride, double *ta, int tstride,
			      double *mn,
			      ay_nurbpatch_object **bevel);

/** Find cross section bevel curve.
 */
int ay_bevelt_findbevelcurve(int index, ay_object **c);

/** Create bevel by surface concatenations fillet creation.
 */
int ay_bevelt_createconcat(int side, ay_object *o, ay_object *c,
			   ay_object **bevel);


/* capt.c */

/** Create all caps for a surface.
 */
int ay_capt_addcaps(ay_cparam *cparams, ay_bparam *bparams, ay_object *o,
		    ay_object **dst);

/** Create simple cap from (convex) curve.
 */
int ay_capt_crtsimplecap(ay_object *c, int mode, double frac, double *mp,
			 ay_object **cap);

/** Integrate cap into the progenitor surface.
 */
int ay_capt_integrate(ay_object *c, int side, int knottype, ay_object *s);

/** Create trim cap from (planar) curve(s).
 */
int ay_capt_crttrimcap(ay_object *c, ay_object **cap);

/** Create gordon cap from non planar curve.
 */
int ay_capt_crtgordoncap(ay_object *c, ay_object **cap);

/** Parse CP (CapParameter) tags.
 */
void ay_capt_parsetags(ay_tag *tag, ay_cparam *params);

/** Create CP (CapParameter) tags.
 */
void ay_capt_createtags(ay_object *o, int *caps);


/* ict.c */

/** Do a global C2 cubic interpolation.
 */
int ay_ict_interpolateC2C(int length, double sdlen, double edlen,
			  int param_type,
			  int have_end_derivs, double *sderiv, double *ederiv,
			  double *controlv,
			  ay_nurbcurve_object **c);

/** Do a global C2 cubic interpolation for a closed curve.
 */
int ay_ict_interpolateC2CClosed(int length, double sdlen, double edlen,
				int param_type,
			   int have_end_derivs, double *sderiv, double *ederiv,
			       double *controlv,
			       ay_nurbcurve_object **c);

/** Do a global 3D interpolation.
 */
int ay_ict_interpolateG3D(int iorder, int length, double sdlen, double edlen,
			  int have_end_derivs, int param_type,
			  double *controlv, double *sderiv, double *ederiv,
			  ay_nurbcurve_object **c);

/** Do a global 3D interpolation for a closed curve.
 */
int ay_ict_interpolateG3DClosed(int iorder, int length,
				double sdlen, double edlen,
				int have_end_derivs, int param_type,
				double *controlv,
				double *sderiv, double *ederiv,
				ay_nurbcurve_object **c);

/** Resize a interpolating curve.
 */
int ay_ict_resize(ay_icurve_object *curve, int new_length);

/** Revert a interpolating curve.
 */
int ay_ict_revert(ay_icurve_object *curve);

/** Get point from a interpolating curve.
 */
int ay_ict_getpntfromindex(ay_icurve_object *curve, int index, double **p);


/* ipt.c */

/** Get control point from index.
 */
int ay_ipt_getpntfromindex(ay_ipatch_object *patch, int indexu, int indexv,
			   double **p);

/** Flip UV of interpolating patch.
 */
int ay_ipt_swapuv(ay_ipatch_object *ip);

/** Revert interpolating patch in U direction.
 */
int ay_ipt_revertu(ay_ipatch_object *ip);

/** Revert interpolating patch in V direction.
 */
int ay_ipt_revertv(ay_ipatch_object *ip);

/** Interpolate NURBS patch in U direction.
 */
int ay_ipt_interpolateu(ay_nurbpatch_object *np, int order, int ktype);

/** Interpolate NURBS patch with end derivatives in U direction.
 */
int ay_ipt_interpolateud(ay_nurbpatch_object *np, int order, int ktype,
			 int dmode, double sdlen, double edlen,
			 double *sd, double *ed);

/** Interpolate NURBS patch with end derivatives in U direction (closed).
 */
int ay_ipt_interpolateudc(ay_nurbpatch_object *np, int order, int ktype,
			  int dmode, double sdlen, double edlen,
			  double *sd, double *ed);

/** Interpolate NURBS patch in V direction.
 */
int ay_ipt_interpolatev(ay_nurbpatch_object *np, int order, int ktype);

/** Interpolate NURBS patch with end derivatives in V direction.
 */
int ay_ipt_interpolatevd(ay_nurbpatch_object *np, int order, int ktype,
			 int dmode, double sdlen, double edlen,
			 double *sd, double *ed);

/** Interpolate NURBS patch with end derivatives in V direction (closed).
 */
int ay_ipt_interpolatevdc(ay_nurbpatch_object *np, int order, int ktype,
			  int dmode, double sdlen, double edlen,
			  double *sd, double *ed);

/** Tcl command to interpolate selected surfaces.
 */
int ay_ipt_interpuvtcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Create default derivative vectors.
 */
int ay_ipt_crtderiv(int mode, ay_ipatch_object *ip);


/* knots.c */

/** Create knots for a NURBS patch.
 */
int ay_knots_createnp(ay_nurbpatch_object *patch);

/** Create knots for a NURBS curve.
 */
int ay_knots_createnc(ay_nurbcurve_object *curve);

/** Check a knot vector for errors.
 */
int ay_knots_check(int length, int order, int knot_count, double *knotv);

/** Print knot error message received by knots_check() to Ayam console.
 */
void ay_knots_printerr(char *location, int errcode);

/** Rescale a knot vector to new lower/upper bounds.
 */
int ay_knots_rescaletorange(int n, double *knotv, double rmin, double rmax);

/** Rescale a knot vector to a minimum knot distance.
 */
int ay_knots_rescaletomindist(int n, double *knotv, double mindist);

/** Unify two knot vectors.
 */
int ay_knots_unify(double *Ua, int Ualen, double *Ub, int Ublen,
		   double **Ubar, int *Ubarlen);

/** Merge knot vector into curve knots.
 */
int ay_knots_mergenc(ay_nurbcurve_object *curve, double *Ubar, int Ubarlen);

/** Merge knot vectors into patch knots.
 */
int ay_knots_mergenp(ay_nurbpatch_object *patch,
		     double *Ubar, int Ubarlen, double *Vbar, int Vbarlen);

/** Calculate valid knot bounds.
 */
int ay_knots_getuminmax(ay_object *o, int order, int knots, double *knotv,
			double *umin, double *umax);

/** Calculate valid knot bounds.
 */
int ay_knots_getvminmax(ay_object *o, int order, int knots, double *knotv,
			double *vmin, double *vmax);

/** Set valid knot bounds (create UMM tag).
 */
int ay_knots_setuminmax(ay_object *o, double umin, double umax);

/** Set valid knot bounds (create VMM tag).
 */
int ay_knots_setvminmax(ay_object *o, double vmin, double vmax);

/** Coarsen a knot vector.
 */
int ay_knots_coarsen(int order, int knotvlen, double *knotv, int count,
		     double **newknotv);

/** Create chordal parameterized knots.
 */
int ay_knots_chordparam(double *Q, int Qlen, int stride, double **U);

/** Create centripetal parameterized knots.
 */
int ay_knots_centriparam(double *Q, int Qlen, int stride, double **U);

/** Create chordal parameterized knots for a surface.
 */
int ay_knots_chordparamnp(int dir, double *Q, int width, int height,
			  int stride, double **U);

/** Create centripetal parameterized knots for a surface.
 */
int ay_knots_centriparamnp(int dir, double *Q, int width, int height,
			   int stride, double **U);

/** Determine whether a knot vector is clamped.
 */
int ay_knots_isclamped(unsigned int side, unsigned int order,
		       double *U, unsigned int Ulen,
		       double eps);

/** Classify a knot vector (returns AY_KT*).
 */
int ay_knots_classify(unsigned int order, double *U, unsigned int Ulen,
		      double eps);

/** Revert a knot vector.
 */
int ay_knots_revert(double *U, int ulen);

/** Get discontinuous parametric values of a NURBS curve/surface.
 */
int ay_knots_getdisc(int Ulen, double *U, int order, int *Udlen, double **Ud);

/** Remove knot from a knot vector.
 */
int ay_knots_remove(unsigned int index, int order, int length, double **U);

/** Insert knot into a knot vector.
 */
int ay_knots_insert(unsigned int index, int order, int length, double **U);

/** Find matching knot.
 */
void ay_knots_match(ay_nurbcurve_object *nc, double eps, double *u);

/** Initialize the knots module.
 */
int ay_knots_init(Tcl_Interp *interp);


/* nb.c */

/** Do a LU decomposition of the nxn matrix A.
 */
int ay_nb_LUDecompose(int n, double *A, int *pivot);

/** Invert the nxn matrix inv.
 */
int ay_nb_LUInvert(int n, double *inv, int *pivot);

/** Interpolate the n+1 4D points in Q.
 */
int ay_nb_GlobalInterpolation4D(int n, double *Q, double *ub, double *Uc,
				int d);

/** Interpolate the n+1 4D points in Q with end derivatives.
 */
int ay_nb_GlobalInterpolation4DD(int n, double *Q, double *ub, double *Uc,
				 int d, double *D1, double *D2);

/** Remove a knot from a NURBS curve.
 */
int ay_nb_RemoveKnotCurve4D(int n, int p, double *U, double *Pw, double tol,
			    int r, int s, int num, double *Ubar, double *Qw);

/** Calculate binomial coefficients.
 */
void ay_nb_Bin(int maxn, int maxk, double *bin);

/** Elevate degree of a NURBS curve.
 */
int ay_nb_DegreeElevateCurve4D(int stride, int n, int p,
			       double *U, double *Pw,
			       int t, int *nh, double *Uh, double *Qw);

/** Solve tridiagonal equation system.
 */
int ay_nb_SolveTridiagonal(int n, double *Q, double *U, double *P);

/** Insert knot into rational NURBS curve.
 */
int ay_nb_InsertKnotCurve4D(int np, int p, double *UP, double *Pw, double u,
			    int k, int s, int r, int *nq, double *UQ,
			    double *Qw);

/** Insert knot into non-rational NURBS curve.
 */
int ay_nb_InsertKnotCurve3D(int np, int p, double *UP, double *P, double u,
			    int k, int s, int r, int *nq, double *UQ,
			    double *Q);

/** Find knot span containing parameter u.
 */
int ay_nb_FindSpan(int n, int p, double u, double *U);

/** Find knot span containing parameter u and calculate multiplicity.
 */
int ay_nb_FindSpanMult(int n, int p, double u, double *U, int *s);

/** Calculate NURBS basis functions.
 */
void ay_nb_BasisFuns(int i, double u, int p, double *U, double *N);

/** Calculate a point on a rational NURBS curve.
 */
int ay_nb_CurvePoint4D(int n, int p, double *U, double *Pw, double u,
		       double *C);

/** Calculate a point on a non-rational NURBS curve.
 */
int ay_nb_CurvePoint3D(int n, int p, double *U, double *P, double u,
		       double *C);

/** Calculate a point on a rational NURBS surface.
 */
int ay_nb_SurfacePoint4D(int n, int m, int p, int q, double *U, double *V,
			 double *Pw, double u, double v, double *C);

/** Calculate a point on a non-rational NURBS surface.
 */
int ay_nb_SurfacePoint3D(int n, int m, int p, int q, double *U, double *V,
			 double *P, double u, double v, double *C);

/** Calculate derivatives of NURBS basis funs.
 */
void ay_nb_DersBasisFuns(int i, double u, int p, int n, double *U,
			 double *ders);

/** Calculate first derivative of non-rational NURBS curve.
 */
void ay_nb_ComputeFirstDer3D(int n, int p, double *U, double *P, double u,
			     double *C1);

/** Calculate second derivative of non-rational NURBS curve.
 */
void ay_nb_ComputeSecDer3D(int n, int p, double *U, double *P, double u,
			   double *C2);

/** Calculate first derivative of rational NURBS curve.
 */
void ay_nb_ComputeFirstDer4D(int n, int p, double *U, double *Pw, double u,
			     double *C1);

/** Calculate second derivative of rational NURBS curve.
 */
void ay_nb_ComputeSecDer4D(int n, int p, double *U, double *Pw, double u,
			   double *C2);

/** Calculate first derivative of rational NURBS surface.
 */
void ay_nb_CompFirstDerSurf4D(int n, int m, int p, int q, double *U, double *V,
			      double *Pw, double u, double v, double *C);

/** Calculate first derivative of non-rational NURBS surface.
 */
void ay_nb_CompFirstDerSurf3D(int n, int m, int p, int q, double *U, double *V,
			      double *P, double u, double v, double *C);

/** Create NURBS circle or arc.
 */
int ay_nb_CreateNurbsCircleArc(double r, double ths, double the,
			       int *length, double **knotv, double **controlv);

/** Refine knot vector of rational NURBS curve with a new vector.
 */
void ay_nb_RefineKnotVectCurve4D(int stride, int n, int p,
				 double *U, double *Pw,
				 double *X, int r, double *Ubar, double *Qw);

/** Refine knot vector of non-rational NURBS curve with a new vector.
 */
void ay_nb_RefineKnotVectCurve3D(int stride, int n, int p,
				 double *U, double *Pw,
				 double *X, int r, double *Ubar, double *Qw);

/** Elevate degree of NURBS surface in U direction.
 */
int ay_nb_DegreeElevateSurfU4D(int stride, int w, int h, int p, double *U,
			       double *Pw, int t,
			       int *nw, double *Uh, double *Qw);

/** Elevate degree of NURBS surface in V direction.
 */
int ay_nb_DegreeElevateSurfV4D(int stride, int w, int h, int p, double *V,
			       double *Pw, int t,
			       int *nh, double *Vh, double *Qw);

/** Refine U knot vector of NURBS surface with a new vector.
 */
int ay_nb_RefineKnotVectSurfU4D(int stride, int w, int h, int p, double *U,
				double *Pw, double *X, int r,
				double *Ubar, double *Qw);

/** Refine V knot vector of NURBS surface with a new vector.
 */
int ay_nb_RefineKnotVectSurfV4D(int stride, int w, int h, int p, double *V,
				double *Pw, double *X, int r,
				double *Vbar, double *Qw);

/** Decompose NURBS curve into Bezier segments.
 */
int ay_nb_DecomposeCurve(int stride, int n, int p, double *U, double *Pw,
			 int *nb, double **Qw);

/** Insert knot into NURBS surface in U direction.
 */
int ay_nb_InsertKnotSurfU(int stride, int w, int h, int p, double *UP,
			  double *Pw,
			  double u, int k, int s, int r,
			  double *UQ, double *Qw);

/** Insert knot into NURBS surface in V direction.
 */
int ay_nb_InsertKnotSurfV(int stride, int w, int h, int q, double *VP,
			  double *Pw,
			  double v, int k, int s, int r,
			  double *VQ, double *Qw);

/** Remove knot from NURBS surface in V direction.
 */
int ay_nb_RemoveKnotSurfV(int w, int h, int q, double *V, double *Pw,
			  double tol,
			  int r, int s, int num, double *Vbar, double *Qw);

/** Unclamp a NURBS curve.
 */
void ay_nb_UnclampCurve(int israt, int n, int p, int s, double *U, double *Pw,
			int updateknots);

/** Unclamp a NURBS surface in U direction.
 */
void ay_nb_UnclampSurfaceU(int israt, int w, int h, int p, int s,
			   double *U, double *Pw);

/** Unclamp a NURBS surface in V direction.
 */
void ay_nb_UnclampSurfaceV(int israt, int w, int h, int q, int s,
			   double *V, double *Pw);

/* nct.c */

/** Create NURBS curve object.
 */
int ay_nct_create(int order, int length, int knot_type,
		  double *controlv, double *knotv,
		  ay_nurbcurve_object **curveptr);

/** Gracefully destroy NURBS curve object.
 */
void ay_nct_destroy(ay_nurbcurve_object *curve);

/** Remove multiple points from curve.
 */
void ay_nct_clearmp(ay_nurbcurve_object *c);

/** Re-create multiple points for curve.
 */
void ay_nct_recreatemp(ay_nurbcurve_object *c);

/** Collapse selected points to multiple points.
 */
int ay_nct_collapseselp(ay_object *o);

/** Explode selected multiple points.
 */
int ay_nct_explodemp(ay_object *o);

/** Resize NURBS curve to new length.
 */
int ay_nct_resize(ay_nurbcurve_object *curve, int new_length);

/** Close NURBS curve.
 */
int ay_nct_close(ay_nurbcurve_object *curve);

/** Revert array.
 */
int ay_nct_revertarr(double *cv, int cvlen, int stride);

/** Revert NURBS curve.
 */
int ay_nct_revert(ay_nurbcurve_object *curve);

/** Refine NURBS curve knots.
 */
int ay_nct_refinekn(ay_nurbcurve_object *curve, int maintain_ends,
		    double *newknotv, int newknotvlen);

/** Refine NURBS curve control points.
 */
int ay_nct_refinecv(ay_nurbcurve_object *curve, ay_point *selp);


/** Tcl command to refine selected NURBS curves.
 */
int ay_nct_refinetcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Clamp periodic NURBS curve on both ends.
 */
int ay_nct_clampperiodic(ay_nurbcurve_object *curve);

/** Clamp NURBS curve.
 */
int ay_nct_clamp(ay_nurbcurve_object *curve, int side);

/** Tcl command to clamp selected NURBS curves.
 */
int ay_nct_clamptcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

/** Elevate order of NURBS curve.
 */
int ay_nct_elevate(ay_nurbcurve_object *curve, int new_order);


/** Tcl command to elevate order of selected NURBS curves.
 */
int ay_nct_elevatetcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Tcl command to insert knot into selected NURBS curves.
 */
int ay_nct_insertkntcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Tcl command to collapse selected points of selected NURBS
 *  curves/patches to multiple points.
 */
int ay_nct_collapsetcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Tcl command to explode selected multiple points of selected NURBS
 *  curves/patches to single points.
 */
int ay_nct_explodetcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Find point on NURBS curve.
 */
int ay_nct_findu(struct Togl *togl, ay_object *o,
		 double *winXY, double *worldXYZ, double *u);

/** Find point on NURBS curve modelling action.
 */
int ay_nct_finducb(struct Togl *togl, int argc, char *argv[]);

/** Split NURBS curve.
 */
int ay_nct_split(ay_object *src, double u, ay_object **result);

/** Tcl command to split selected NURBS curves.
 */
int ay_nct_splittcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

/** Tcl command to concatenate selected NURBS curves.
 */
int ay_nct_concattcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Create a NURBS circle.
 */
int ay_nct_crtncircle(double radius, ay_nurbcurve_object **curve);

/** Create a circular NURBS arc.
 */
int ay_nct_crtncirclearc(double radius, double arc,
			 ay_nurbcurve_object **curve);

/** Create a NURBS half-circle.
 */
int ay_nct_crtnhcircle(double radius, ay_nurbcurve_object **curve);

/** Tcl command to create a NURBS circle.
 */
int ay_nct_crtncircletcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

/** Tcl command to create a NURBS rectangle.
 */
int ay_nct_crtrecttcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Create control points for a circular B-Spline curve.
 */
int ay_nct_crtcircbspcv(int sections, double radius, double arc, int order,
			double **result);

/** Create a circular B-Spline curve.
 */

int ay_nct_crtcircbsp(int sections, double radius, double arc, int order,
		      ay_nurbcurve_object **result);

/** Tcl command to create a circular B-Spline curve.
 */
int ay_nct_crtclosedbsptcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);

/** Calculate orientation of planar aligned NURBS curve.
 */
int ay_nct_getorientation(ay_nurbcurve_object *curve, int stride,
			  int report, int plane, double *oangle);

/** Calculate orientation of NURBS curve.
 */
int
ay_nct_getorientation3d(ay_nurbcurve_object *curve, int stride,
			int report, double *orient);

/** Calculate winding of arbitrary (3D) NURBS curve.
 */
int
ay_nct_getwinding(ay_nurbcurve_object *curve, double *normals,
		  int normalstride);

/** Check NURBS curve closeness.
 */
int ay_nct_isclosed(ay_nurbcurve_object *nc);

/** Set NURBS curve type.
 */
void ay_nct_settype(ay_nurbcurve_object *nc);

/** Apply transformations to all control points.
 */
int ay_nct_applytrafo(ay_object *c);

/** Get control point from index.
 */
int ay_nct_getpntfromindex(ay_nurbcurve_object *curve, int index, double **p);

/** Concatenate multiple objects.
 */
void ay_nct_concatobjs(ay_object *o, ay_object **result);


/** Concatenate multiple NURBS curves.
 */
int ay_nct_concatmultiple(int closed, int knot_type, int fillgaps,
			  ay_object *curves,
			  ay_object **result);

/** Create fillet.
 */
int ay_nct_fillgap(int order, double tanlen,
		   ay_nurbcurve_object *c1, ay_nurbcurve_object *c2,
		   ay_object **result);

/** Create fillets.
 */
int ay_nct_fillgaps(int closed, int order, double tanlen, ay_object *curves);

/** Arrange objects.
 */
int ay_nct_arrange(ay_object *o, ay_object *t, int rotate);

/** Tcl command to rescale the knot vectors of selected curves.
 */
int ay_nct_rescaleknvtcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

/** Calculate curvature at parameter value \a t.
 */
double ay_nct_getcurvature(ay_nurbcurve_object *c, double t);

/** Tcl command to create a curvature plot curve.
 */
int ay_nct_curvplottcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Unfinished.
 */
int ay_nct_intersect(ay_nurbcurve_object *cu, ay_nurbcurve_object *cv,
		     double *intersection);

/** Unfinished.
 */
int ay_nct_intersectca(ay_object *cu, ay_object *cv, double *intersections);

/** Check a number of curves for compatibility.
 */
int ay_nct_iscompatible(ay_object *curves, int *result);

/** Make a number of curves compatible.
 */
int ay_nct_makecompatible(ay_object *curves);

/** Shift control points of a 1D (curve) control vector.
 */
int ay_nct_shiftarr(int dir, int stride, int cvlen, double *cv);

/** Shift control points of selected curves.
 */
int
ay_nct_shiftctcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[]);

/** Rotate curve to XY plane.
 */
int ay_nct_toxy(int allow_flip, ay_object *c);

/** Tcl command to rotate selected NURBS curves to XY plane.
 */
int ay_nct_toxytcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

/** Tcl command to check compatibility of the selected NURBS curves.
 */
int ay_nct_iscomptcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Tcl command to make selected NURBS curves compatible.
 */
int ay_nct_makecomptcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Check whether any weight of a NURBS curve is != 1.0.
 */
int ay_nct_israt(ay_nurbcurve_object *curve);

/** Coarsen NURBS curve.
 */
int ay_nct_coarsen(ay_nurbcurve_object *curve);

/** Tcl command to coarsen selected NURBS curves.
 */
int ay_nct_coarsentcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Tcl command to remove a knot from selected NURBS curves.
 */
int ay_nct_removekntcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Trim selected NURBS curves.
 */
int ay_nct_trimtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

/** Trim NURBS curve.
 */
int ay_nct_trim(ay_nurbcurve_object **curve, double umin, double umax);

/** Check curve for degeneracy.
 */
int ay_nct_isdegen(ay_nurbcurve_object *curve);

/** Create offset curve.
 */
int ay_nct_offset(ay_object *o, int mode, double offset,
		  ay_nurbcurve_object **nc);

/** Compare two points (helper for qsort).
 */
int ay_nct_cmppnt(const void *p1, const void *p2);

/** Compare two points given as pointers (helper for qsort).
 */
int ay_nct_cmppntp(const void *p1, const void *p2);

/** Do curve length estimation.
 */
int ay_nct_estlen(ay_nurbcurve_object *nc, double *len);

/** Tcl command to estimate length of selected NURBS curves.
 */
int ay_nct_estlentcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Tcl command to reparameterise selected NURBS curves.
 */
int ay_nct_reparamtcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Convert rational coordinates from euclidean to homogeneous style.
*/
int ay_nct_euctohom(ay_nurbcurve_object *nc);

/** Convert rational coordinates from homogeneous to euclidean style.
*/
int ay_nct_homtoeuc(ay_nurbcurve_object *nc);

/** Check planarity of curve.
*/
void ay_nct_isplanar(ay_object *c, ay_object **cp, int *is_planar);

/** Tcl command to unclamp the selected NURBS curves.
 */
int ay_nct_unclamptcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Extend NURBS curve to a point.
 */
int ay_nct_extend(ay_nurbcurve_object *curve, double *p);

/** Tcl command to extend the selected NURBS curves.
 */
int ay_nct_extendtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Compute mean distance of two 1D control point arrays.
 */
double ay_nct_meandist(int cvlen, int cvstride, double *cva, double *cvb);

/** Shift 1D control point array to minimum mean distance to a second array.
 */
int ay_nct_shifttominmeandist(int cvlen, int cvstride,
			      double *cva, double *cvb);

/** Rotate 1D control point array to minimum mean distance to a second array.
 */
int ay_nct_rotatetominmeandist(int cvlen, int cvstride,
			       double *cva, double *cvb);

/** Compute curve tangent vectors from control points.
 */
int ay_nct_getcvtangents(ay_nurbcurve_object *nc, double **result);


/* npt.c */

/** Create NURBS patch object.
 */
int ay_npt_create(int uorder, int vorder, int width, int height,
		  int uknot_type, int vknot_type,
		  double *controlv, double *uknotv, double *vknotv,
		  ay_nurbpatch_object **patchptr);

/** Gracefully destroy NURBS patch object.
 */
void ay_npt_destroy(ay_nurbpatch_object *patch);

/** Create Ayam object for NURBS patch.
 */
int ay_npt_createnpatchobject(ay_object **result);

/** Reset the display attributes of a NURBS patch.
 */
void ay_npt_resetdisplay(ay_object *o);

/** Resize width (U) of 2D control point array.
 */
int ay_npt_resizearrayw(double **controlvptr, int stride,
			int width, int height, int new_width);

/** Resize width (U) of NURBS patch.
 */
int ay_npt_resizew(ay_nurbpatch_object *np, int new_width);

/** Resize height (V) of 2D control point array.
 */
int ay_npt_resizearrayh(double **controlvptr, int stride,
			int width, int height, int new_height);

/** Resize height (V) of NURBS patch.
 */
int ay_npt_resizeh(ay_nurbpatch_object *np, int new_height);

/** Revert NURBS patch in U direction.
 */
int ay_npt_revertu(ay_nurbpatch_object *np);

/** Tcl command to revert selected surfaces in U direction.
 */
int ay_npt_revertutcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Revert NURBS patch in V direction.
 */
int ay_npt_revertv(ay_nurbpatch_object *np);

/** Tcl command to revert selected surfaces in V direction.
 */
int ay_npt_revertvtcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Flip UV of 2D control point array.
 */
int ay_npt_swaparray(double **controlvptr, int stride,
		     int width, int height);

/** Flip UV of NURBS patch.
 */
int ay_npt_swapuv(ay_nurbpatch_object *np);

/** Draw a single trim curve using GLU.
 */
int ay_npt_drawtrimcurve(ay_object *o, GLUnurbsObj *no, unsigned int refine);

/** Draw all trim curves using GLU.
 */
int ay_npt_drawtrimcurves(ay_object *o, unsigned int refine_trims);

/** Create Cobbs NURBS sphere patch.
 */
int ay_npt_crtcobbsphere(ay_nurbpatch_object **cobbsphere);

/** Create standard NURBS sphere (revolved half circle).
 */
int ay_npt_crtnsphere(double radius, ay_nurbpatch_object **nsphere);

/** Tcl command to create a standard NURBS sphere (revolved half circle).
 */
int ay_npt_crtnspheretcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

/** Tcl command to create a multi patch Cobbs NURBS sphere.
 */
int ay_npt_crtnsphere2tcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

/** Break NURBS patch into curves along U.
 */
int ay_npt_breakintocurvesu(ay_object *o, int apply_trafo,
			    ay_object **curves, ay_object ***last);

/** Break NURBS patch into curves along V.
 */
int ay_npt_breakintocurvesv(ay_object *o, int apply_trafo,
			    ay_object **curves, ay_object ***last);

/** Tcl command to break a NURBS patch into curves.
 */
int ay_npt_breakintocurvestcmd(ClientData clientData, Tcl_Interp *interp,
			       int argc, char *argv[]);

/** Build NURBS patch from curves.
 */
int ay_npt_buildfromcurves(ay_list_object *curves, int ncurves, int type,
			   int order, int knot_type, int apply_trafo,
			   ay_object **patch);


/** Tcl command to build NURBS patch from curves.
 */
int ay_npt_buildfromcurvestcmd(ClientData clientData, Tcl_Interp *interp,
			       int argc, char *argv[]);

/** Create fillet patch.
 */
int ay_npt_fillgap(ay_object *o1, ay_object *o2,
		   double tanlen, char *uv, ay_object **result);

/** Create setback (instead of fillet).
 */
int ay_npt_setback(ay_object *o1, ay_object *o2, double tanlen, char *uv);

/** Concatenate NURBS patches.
 */
int ay_npt_concat(ay_object *o, int type, int order,
		  int knot_type, int fillet_type, double ftlen,
		  int compatible, char *uv, ay_object **result);

/** Create surface of revolution.
 */
int ay_npt_revolve(ay_object *o, double arc, int sections, int order,
		   ay_nurbpatch_object **revolution);

/** Create swung surface.
 */
int ay_npt_swing(ay_object *o1, ay_object *o2,
		 ay_nurbpatch_object **swing);

/** Create swept surface.
 */
int ay_npt_sweep(ay_object *o1, ay_object *o2, ay_object *o3, int sections,
		 int rotate, int closed, ay_nurbpatch_object **sweep);

/** Create periodic swept surface.
 */
int ay_npt_sweepperiodic(ay_object *o1, ay_object *o2, ay_object *o3,
		       int sections, int rotate,
		       ay_nurbpatch_object **sweep);

/** Create birailed surface from three curves.
 */
int ay_npt_birail1(ay_object *o1, ay_object *o2, ay_object *o3, int sections,
		   int closed, ay_nurbpatch_object **birail1);

/** Create periodic birailed surface from three curves.
 */
int ay_npt_birail1periodic(ay_object *o1, ay_object *o2, ay_object *o3,
			   int sections,
			   ay_nurbpatch_object **birail1);

/** Create birailed surface from four curves.
 */
int ay_npt_birail2(ay_object *o1, ay_object *o2, ay_object *o3, ay_object *o4,
		   ay_object *o5,
		   int sections, int closed, int fullinterpolctrl,
		   ay_nurbpatch_object **birail2);

/** Create loft/skinned surface.
 */
int ay_npt_skinu(ay_object *curves, int order, int knot_type,
		 int interpolate, ay_nurbpatch_object **skin);

/** Create loft/skinned surface.
 */
int ay_npt_skinv(ay_object *curves, int order, int knot_type,
		 int interpolate, ay_nurbpatch_object **skin);

/** Create extruded surface.
 */
int ay_npt_extrude(double height, ay_object *o,
		   ay_nurbpatch_object **extrusion);

/** Calculate 2D tangent from control points.
 */
int ay_npt_gettangentfromcontrol2D(int ctype, int n, int p, int stride,
				   double *P, int a, double *T);


/** Create cap surface.
 */
int ay_npt_createcap(double z, ay_nurbcurve_object *curve,
		     double *ominx, double *omaxx,
		     double *ominy, double *omaxy,
		     ay_nurbpatch_object **cap);

/** Apply transformations to all control points.
 */
int ay_npt_applytrafo(ay_object *p);

/** Get control point from index.
 */
int ay_npt_getpntfromindex(ay_nurbpatch_object *patch, int indexu, int indexv,
			   double **p);

/** Elevate U order of NURBS patch.
 */
int ay_npt_elevateu(ay_nurbpatch_object *patch, int t);

/** Elevate V order of NURBS patch.
 */
int ay_npt_elevatev(ay_nurbpatch_object *patch, int t);

/** Tcl command to elevate U/V order of selected NURBS patches.
 */
int ay_npt_elevateuvtcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** Tcl command to swap U and V of selected surfaces.
 */
int ay_npt_swapuvtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Create Gordon surface.
 */
int ay_npt_gordon(ay_object *cu, ay_object *cv, ay_object *in,
		  int uorder, int vorder,
		  ay_nurbpatch_object **gordon);

/** Watch and correct Gordon surface corners.
 */
void ay_npt_gordonwc(ay_object *g);

/** Extract boundary curve from NURBS patch.
 */
int ay_npt_extractboundary(ay_object *o, int apply_trafo,
			   ay_nurbcurve_object **result);

/** Extract trim curve from NURBS patch.
 */
int
ay_npt_extracttrim(ay_object *o, int tnum, double param, int apply_trafo,
		   int extractnt, double **pvnt, ay_nurbcurve_object **result);

/** Extract curve from patch.
 */
int ay_npt_extractnc(ay_object *npatch, int side, double param, int relative,
		     int apply_trafo, int extractnt,
		     double **pvnt,
		     ay_nurbcurve_object **result);

/** Check whether any weight of a NURBS patch is != 1.0.
 */
int ay_npt_israt(ay_nurbpatch_object *np);

/** Check whether curve is boundary.
 */
int ay_npt_isboundcurve(ay_object *o,
			double b1, double b2, double b3, double b4,
			int *result);

/** Check whether NURBS patch is trimmed.
 */
int ay_npt_istrimmed(ay_object *o, int mode);

/** Close NURBS patch in U direction.
 */
int ay_npt_closeu(ay_nurbpatch_object *np, int mode);

/** Tcl command to close selected NURBS patches in U direction.
 */
int ay_npt_closeutcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Close NURBS patch in V direction.
 */
int ay_npt_closev(ay_nurbpatch_object *np, int mode);

/** Tcl command to close selected NURBS patches in V direction.
 */
int ay_npt_closevtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Check NURBS patch planarity.
 */
int ay_npt_isplanar(ay_nurbpatch_object *np, double *n);

/** Check NURBS patch closeness in U direction.
 */
int ay_npt_isclosedu(ay_nurbpatch_object *np);

/** Check NURBS patch closeness in V direction.
 */
int ay_npt_isclosedv(ay_nurbpatch_object *np);

/** Set the NURBS patch utype and vtype attributes.
 */
void ay_npt_setuvtypes(ay_nurbpatch_object *np, int dir);

/** Remove multiple points from patch \a np.
 */
void ay_npt_clearmp(ay_nurbpatch_object *np);

/** Re-create multiple points for patch \a np.
 */
void ay_npt_recreatemp(ay_nurbpatch_object *np);

/** Collapse selected points to multiple points.
 */
int ay_npt_collapseselp(ay_object *o);

/** Explode selected multiple points.
 */
int ay_npt_explodemp(ay_object *o);

/** Find and parse bevel parameter tags.
 */
int ay_npt_getbeveltags(ay_object *o, int place,
			int *has_bevel, int *type, double *radius, int *sense);

/** Clamp surface in U direction.
 */
int ay_npt_clampu(ay_nurbpatch_object *np, int side);

/** Clamp surface in V direction.
 */
int ay_npt_clampv(ay_nurbpatch_object *np, int side);

/** Tcl command to clamp selected NURBS patches in U/V direction.
 */
int ay_npt_clampuvtcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Rescale trim curves according to surface knot vector scaling.
 */
int ay_npt_rescaletrims(ay_object *trim,
			int mode, double omin, double omax,
			double nmin, double nmax);

/** Tcl command to rescale knots of selected NURBS patches.
 */
int ay_npt_rescaleknvnptcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);

/** Tcl command to insert knots into selected NURBS patches (U direction).
 */
int ay_npt_insertknutcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** Tcl command to insert knots into selected NURBS patches (V direction).
 */
int ay_npt_insertknvtcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** Split a surface in U direction (along V).
 */
int ay_npt_splitu(ay_object *src, double u, ay_object **result);

/** Split a surface in V direction (along U).
 */
int ay_npt_splitv(ay_object *src, double v, ay_object **result);

/** Tcl command to split selected NURBS patches in U/V direction.
 */
int ay_npt_splituvtcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Extract sub surface.
 */
int ay_npt_extractnp(ay_object *src, double umin, double umax,
		     double vmin, double vmax, int relative,
		     ay_object **result);

/** Tcl command to extract a sub surface from the selected NURBS patches.
 */
int ay_npt_extractnptcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

/** Calculate offset surface.
 */
int ay_npt_offset(ay_object *o, int mode, double offset,
		  ay_nurbpatch_object **np);

/** Find point on NURBS surface.
 */
int ay_npt_finduv(struct Togl *togl, ay_object *o,
		  double *winXY, double *worldXYZ, double *u, double *v);

/** Find point on NURBS surface modelling action.
 */
int ay_npt_finduvcb(struct Togl *togl, int argc, char *argv[]);


/** Compute average control point distances in U direction.
 */
int ay_npt_avglensu(double *cv, int width, int height, int stride,
		    double **avlens);

/** Compute average control point distances in V direction.
 */
int ay_npt_avglensv(double *cv, int width, int height, int stride,
		    double **avlens);

/** Tcl command to concatenate selected surfaces.
 */
int ay_npt_concatstcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Tcl command to remove knot from selected surfaces.
 */
int ay_npt_remknunptcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Tcl command to remove knot from selected surfaces.
 */
int ay_npt_remknvnptcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Convert rational coordinates from euclidean to homogeneous style.
*/
int ay_npt_euctohom(ay_nurbpatch_object *nc);

/** Convert rational coordinates from homogeneous to euclidean style.
*/
int ay_npt_homtoeuc(ay_nurbpatch_object *nc);

/** Refine NURBS surface in U direction.
*/
int ay_npt_refineu(ay_nurbpatch_object *patch,
		   double *newknotv, int newknotvlen);

/** Refine NURBS surface in V direction.
*/
int ay_npt_refinev(ay_nurbpatch_object *patch,
		   double *newknotv, int newknotvlen);

/** Tcl command to refine the knots of selected surfaces.
 */
int ay_npt_refineuvtcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/** Get all normals from control points.
 */
void ay_npt_getcvnormals(ay_nurbpatch_object *np, double *n);

/** Tcl command to unclamp the knot vectors of the selected surfaces.
 */
int ay_npt_unclamptcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Generate texture coordinates.
 */
void ay_npt_gentexcoords(ay_nurbpatch_object *np, ay_tag *tags,
			 double **result);


/* pmt.c */

/** Convert patchmesh to NURBS patch.
 */
int ay_pmt_tonpatch(ay_object *o, int btype, ay_object **result);

/** Check patchmesh for errors.
 */
int ay_pmt_valid(ay_pamesh_object *pamesh);

/** Get control point from index.
 */
int ay_pmt_getpntfromindex(ay_pamesh_object *patch, int indexu, int indexv,
			   double **p);

/** Flip UV of patchmesh.
 */
int ay_pmt_swapuv(ay_pamesh_object *pm);

/** Revert patchmesh in U direction.
 */
int ay_pmt_revertu(ay_pamesh_object *pm);

/** Revert patchmesh in V direction.
 */
int ay_pmt_revertv(ay_pamesh_object *pm);

/** Check whether any weight of a patch mesh is != 1.0.
 */
int ay_pmt_israt(ay_pamesh_object *pm);

/** Tcl command to convert selected patch meshes to a different basis matrix */
int ay_pmt_tobasistcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Initialize the patch mesh tools module.
 */
void ay_pmt_init();


/* stess.c */

/** Remove tesselation from NURBS patch.
 */
void ay_stess_destroy(ay_nurbpatch_object *np);

/** Calculate stess usable quality factor from GLU sampling tolerance.
 */
int ay_stess_GetQF(double gst);

/** Tesselate a (non-rational or rational) 2D (trim) curve.
 */
int ay_stess_CurvePoints2D(int n, int p, double *U, double *Pw, int stride,
			   int is_rat, int qf, int *Clen, double **C);

/** Tesselate a (non-rational or rational) 3D curve.
 */
int ay_stess_CurvePoints3D(int n, int p, double *U, double *Pw, int is_rat,
			   int qf, int *Clen, double **C);

/** Tesselate a untrimmed, non-rational NURBS surface.
 */
int ay_stess_SurfacePoints3D(int n, int m, int p, int q, double *U, double *V,
			     double *P, int qf, int *Cn, int *Cm, double **C);

/** Tesselate a untrimmed, rational NURBS surface.
 */
int ay_stess_SurfacePoints4D(int n, int m, int p, int q, double *U, double *V,
			     double *P, int qf, int *Cn, int *Cm, double **C);

/** Draw tesselation of a trimmed NURBS surface.
 */
void ay_stess_DrawTrimmedSurface(ay_stess *stess);

/** Shade tesselation of a trimmed NURBS surface.
 */
void ay_stess_ShadeTrimmedSurface(ay_stess *stess);

/** Tesselate all trim curves of a NURBS surface.
 */
int ay_stess_TessTrimCurves(ay_object *o, int qf, int *nt, double ***tt,
			    int **tl, int **td);

/** Tesselate all trim curves of a NURBS surface for drawing.
 */
int ay_stess_ReTessTrimCurves(ay_object *o, int qf, int nt, double **tt,
			      int *tl, double **tp);

/** Tesselate a trimmed NURBS surface.
 */
int ay_stess_TessTrimmedNP(ay_object *o, int qf);

/** Tesselate a NURBS surface.
 */
int ay_stess_TessNP(ay_object *o, int qf);


/* tess.c */

/** Tesselate NURBS patch (convert to PolyMesh) with GLU.
 */
int ay_tess_npatch(ay_object *o,
		   int smethod, double sparamu, double sparamv,
		   int use_tc, char *myst,
		   int use_vc, char *mycs,
		   int use_vn, char *myn,
		   int refine_trims,
		   ay_object **pm);

/** Tcl command to tesselate selected NURBS patches
 *  (convert to PolyMesh) with GLU.
 */
int ay_tess_npatchtcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** Tesselate a single polymesh face with GLU.
 */
int ay_tess_pomeshf(ay_pomesh_object *pomesh,
		    unsigned int f, unsigned int m, unsigned int n,
		    int optimize,
		    ay_pomesh_object **trpomesh);

/** Tesselate a polymesh with GLU.
 */
int ay_tess_pomesh(ay_pomesh_object *pomesh, int optimize, double *normal,
		   ay_pomesh_object **trpomesh);

/*! \file nurbs.h \brief Ayam NURBS API */

#endif /* __nurbs_h__ */
