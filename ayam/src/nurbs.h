/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* nurbs.h - nurbs prototypes */

/* capt.c */
int ay_capt_createfromcurve(ay_object *c, ay_object **ca);

/* ict.c */
int ay_ict_interpolateC2C(double iparam, int closed, int length,
			  double *controlv, ay_nurbcurve_object **c);

int ay_ict_interpolateG4D(int iorder, int length,
			  double *controlv, ay_nurbcurve_object **c);

int ay_ict_interpolateG4DC(int iorder, int length, double iparam,
			   double *controlv, ay_nurbcurve_object **c);

int ay_ict_resize(ay_icurve_object *curve, int new_length);

int ay_ict_revert(ay_icurve_object *curve);

int ay_ict_reverttcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

int ay_ict_getpntfromindex(ay_icurve_object *curve, int index, double **p);

/* knots.c */
int ay_knots_createnp(ay_nurbpatch_object *patch);

int ay_knots_createnc(ay_nurbcurve_object *curve);

int ay_knots_check(int length, int order, int knot_count, double *knotv);

int ay_knots_rescaleknotv(int n, double *knotv);

int ay_knots_unify(double *Ua, int Ualen, double *Ub, int Ublen,
		   double **Ubar, int *Ubarlen);

int ay_knots_merge(ay_nurbcurve_object *curve, double *Ubar, int Ubarlen);

int ay_knots_mergesurf(ay_nurbpatch_object *patch,
		       double *Ubar, int Ubarlen, double *Vbar, int Vbarlen);

/* nb.c */
int ay_nb_LUDecompose(int n, double *A, int *pivot);

int ay_nb_LUInvert(int n, double *inv, int *pivot);

int ay_nb_GlobalInterpolation4D(int n, double *Q, double *ub, double *Uc,
				int d);

int ay_nb_GlobalInterpolation4DD(int n, double *Q, double *ub, double *Uc,
				 int d, double *D1, double *D2);

int ay_nb_RemoveKnot(int stride, int n, int p, double *U, double *Pw,
		     int r, int s, int num, double *Ubar, double *Qw);

void ay_nb_Bin(int maxn, int maxk, double *bin);

int ay_nb_DegreeElevateCurve(int stride, int n, int p, double *U, double *Pw, 
			     int t, int *nh, double *Uh, double *Qw);

int ay_nb_SolveTridiagonal(int n, double *Q, double *U, double *P);

int ay_nb_CurveInsertKnot4D(int np, int p, double *UP, double *Pw, double u,
			    int k, int s, int r, int *nq, double *UQ,
			    double *Qw);

int ay_nb_CurveInsertKnot3D(int np, int p, double *UP, double *P, double u,
			    int k, int s, int r, int *nq, double *UQ,
			    double *Q);

int ay_nb_FindSpan(int n, int p, double u, double *U);

int ay_nb_FindSpanMult(int n, int p, double u, double *U, int *s);

void ay_nb_BasisFuns(int i, double u, int p, double *U, double *N);

int ay_nb_CurvePoint4D(int n, int p, double *U, double *Pw, double u,
		       double *C);

int ay_nb_CurvePoint3D(int n, int p, double *U, double *P, double u,
		       double *C);

int ay_nb_SurfacePoint4D(int n, int m, int p, int q, double *U, double *V,
			 double *Pw, double u, double v, double *C);

int ay_nb_SurfacePoint3D(int n, int m, int p, int q, double *U, double *V,
			 double *P, double u, double v, double *C);

void ay_nb_DersBasisFuns(int i, double u, int p, int n, double *U,
			 double *ders);

void ay_nb_ComputeFirstDer3D(int n, int p, double *U, double *P, double u,
			     double *CK);

void ay_nb_ComputeSecDer3D(int n, int p, double *U, double *P, double u,
			   double *CK);

void ay_nb_ComputeFirstDer4D(int n, int p, double *U, double *Pw, double u,
			     double *CK);

void ay_nb_ComputeSecDer4D(int n, int p, double *U, double *Pw, double u,
			   double *CK);

void ay_nb_CompFirstDerSurf4D(int n, int m, int p, int q, double *U, double *V,
			      double *Pw, double u, double v, double *C);

void ay_nb_CompFirstDerSurf3D(int n, int m, int p, int q, double *U, double *V,
			      double *P, double u, double v, double *C);

int ay_nb_CreateNurbsCircle(double r, double ths, double the,
			int *length, double **knotv, double **controlv);

void ay_nb_FinduFromPoint(ay_nurbcurve_object *curve, double *point,
			  double guess, int max_try, double e1, double e2,
			  double *u);

void ay_nb_RefineKnotVectCurve(int stride, int n, int p, double *U, double *Pw,
			       double *X, int r, double *Ubar, double *Qw);

int ay_nb_DegreeElevateSurfU(int stride, int w, int h, int p, double *U,
			     double *Pw, int t,
			     int *nw, double *Uh, double *Qw);

int ay_nb_DegreeElevateSurfV(int stride, int w, int h, int p, double *V,
			     double *Pw, int t,
			     int *nh, double *Vh, double *Qw);

int ay_nb_RefineKnotVectSurfU(int stride, int w, int h, int p, double *U,
			      double *Pw, double *X, int r,
			      double *Ubar, double *Qw);

int ay_nb_RefineKnotVectSurfV(int stride, int w, int h, int p, double *V,
			      double *Pw, double *X, int r,
			      double *Vbar, double *Qw);

int ay_nb_DecomposeCurve(int stride, int n, int p, double *U, double *Pw,
			 int *nb, double **Qw);

/* nct.c */
int ay_nct_create(int order, int length, int knot_type,
		  double *controlv, double *knotv,
		  ay_nurbcurve_object **curveptr);

void ay_nct_clearmp(ay_nurbcurve_object *c);

int ay_nct_recreatemp(ay_nurbcurve_object *c);

int ay_nct_collapseselp(ay_object *o);

int ay_nct_explodemp(ay_object *o);

int ay_nct_resize(ay_nurbcurve_object *curve, int new_length);

int ay_nct_close(ay_nurbcurve_object *curve);

int ay_nct_revert(ay_nurbcurve_object *curve);

int ay_nct_reverttcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

int ay_nct_refine(ay_nurbcurve_object *curve, double *newknotv,
		  int newknotvlen);

int ay_nct_refinetcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

int ay_nct_clamp(ay_nurbcurve_object *curve);

int ay_nct_clamptcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

int ay_nct_elevate(ay_nurbcurve_object *curve, int new_order);

int ay_nct_elevatetcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

int ay_nct_insertkntcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int ay_nct_collapsetcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int ay_nct_explodetcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

int ay_nct_findu(struct Togl *togl, ay_object *o,
		 double *winX, double *winY, double *u);

int ay_nct_finducb(struct Togl *togl, int argc, char *argv[]);

int ay_nct_split(ay_object *src, double u);

int ay_nct_splittcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

int ay_nct_concattcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

int ay_nct_crtncircle(ay_nurbcurve_object **curve);

int ay_nct_crtncirclearc(double arc, ay_nurbcurve_object **curve);

int ay_nct_crtnhcircle(ay_nurbcurve_object **curve);

int ay_nct_crtncircletcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

int ay_nct_crtrecttcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

int ay_nct_crtclosedbsptcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);

void ay_nct_getorientation(ay_nurbcurve_object *curve, double *oangle);

int ay_nct_isclosed(ay_nurbcurve_object *nc);

int ay_nct_createfrompatch(ay_object *p, int where, ay_object **cu);

int ay_nct_applytrafo(ay_object *c);

int ay_nct_getpntfromindex(ay_nurbcurve_object *curve, int index, double **p);

int ay_nct_concatmultiple(int closed, int knot_type, int fillgaps,
			  ay_object *curves,
			  ay_object **result);

int ay_nct_fillgap(int order, double tanlen,
		   ay_nurbcurve_object *c1, ay_nurbcurve_object *c2,
		   ay_object **result);

int ay_nct_fillgaps(int closed, int order, double tanlen, ay_object *curves);

int ay_nct_arrange(ay_object *o, ay_object *t, int rotate);

int ay_nct_addinternalcps(ay_object *curve, int where);

int ay_nct_rescaleknvnctcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);

double ay_nct_getcurvature(ay_nurbcurve_object *c, double t);

int ay_nct_curvplottcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int ay_nct_makecompatible(ay_object *cu);

/* npt.c */
int ay_npt_create(int uorder, int vorder, int width, int height,
		  int uknot_type, int vknot_type,
		  double *controlv, double *uknotv, double *vknotv,
		  ay_nurbpatch_object **patchptr);

int ay_npt_revolve(ay_object *o, double arc, ay_nurbpatch_object **patch);

int ay_npt_drawtrimcurve(struct Togl *togl, ay_object *o, GLUnurbsObj *no);

int ay_npt_resizearrayw(double **controlvptr, int stride,
			int width, int height, int new_width);

int ay_npt_resizew(ay_nurbpatch_object *patch, int new_width);

int ay_npt_resizearrayh(double **controlvptr, int stride,
			int width, int height, int new_height);

int ay_npt_resizeh(ay_nurbpatch_object *patch, int new_height);

int ay_npt_revertu(ay_nurbpatch_object *patch);

int ay_npt_revertutcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

int ay_npt_revertv(ay_nurbpatch_object *patch);

int ay_npt_revertvtcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

int ay_npt_swapuv(ay_nurbpatch_object *np);

int ay_npt_wribtrimcurves(ay_object *o);

int ay_npt_crtcobbsphere(ay_nurbpatch_object **patch);

int ay_npt_crtnspheretcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

int ay_npt_crtnsphere2tcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

int ay_npt_splittocurvestcmd(ClientData clientData, Tcl_Interp *interp,
			     int argc, char *argv[]);

int ay_npt_buildfromcurvestcmd(ClientData clientData, Tcl_Interp *interp,
			       int argc, char *argv[]);

int ay_npt_sweep(ay_object *o1, ay_object *o2, ay_object *o3, int sections,
		 int rotate, int closed, ay_nurbpatch_object **patch,
		 int has_start_cap, ay_object **start_cap,
		 int has_end_cap, ay_object **end_cap);

int ay_npt_interpolateu(ay_nurbpatch_object *patch, int order);

int ay_npt_skinu(ay_object *curves, int order, int knot_type,
		 int interpolate, ay_nurbpatch_object **skin);

int ay_npt_skinv(ay_object *curves, int order, int knot_type,
		 int interpolate, ay_nurbpatch_object **skin);

int ay_npt_extrude(double height, ay_object *o,
		   ay_nurbpatch_object **patch);

int ay_npt_gettangentfromcontrol(int closed, int n, int p,
				 int stride, double *P, int a, double *t);

int ay_npt_bevel(int type, double radius, ay_object *o,
		 ay_nurbpatch_object **patch);

int ay_npt_createcap(double z, ay_nurbcurve_object *curve,
		     double *ominx, double *omaxx,
		     double *ominy, double *omaxy, double *oangle,
		     ay_nurbpatch_object **cap);

int ay_npt_getpntfromindex(ay_nurbpatch_object *patch, int indexu, int indexv,
			   double **p);

int ay_npt_topolymesh(ay_object *o, int smethod, double sparam,
		      ay_object **pm);

int ay_npt_topolytcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

int ay_npt_elevateu(ay_nurbpatch_object *patch, int t);

int ay_npt_elevateutcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int ay_npt_elevatev(ay_nurbpatch_object *patch, int t);

int ay_npt_elevatevtcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int ay_npt_swapuvtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

int ay_npt_gordon(ay_object *cu, ay_object *cv, ay_object *in,
		  int uorder, int vorder,
		  ay_nurbpatch_object **gordon);

/* pmt.c */

int ay_pmt_tonpatch(ay_pamesh_object *pamesh, ay_object **result);

int ay_pmt_valid(ay_pamesh_object *pamesh, int *detail);

int ay_pmt_getpntfromindex(ay_pamesh_object *patch, int indexu, int indexv,
			   double **p);
