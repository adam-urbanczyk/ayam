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

#include "ayam.h"
#include <ctype.h>

/* comp.c - compare objects */


/* global variables: */

ay_table ay_comparecbt;


/* prototypes of functions local to this module: */

int ay_comp_tag(ay_tag_object *t1, ay_tag_object *t2);

int ay_comp_sarg(ay_shader_arg *a1, ay_shader_arg *a2);

int ay_comp_shader(ay_shader *s1, ay_shader *s2);

int ay_comp_level(ay_object *o1, ay_object *o2);

int ay_comp_disk(ay_object *o1, ay_object *o2);

int ay_comp_cone(ay_object *o1, ay_object *o2);

int ay_comp_cylinder(ay_object *o1, ay_object *o2);

int ay_comp_sphere(ay_object *o1, ay_object *o2);

int ay_comp_torus(ay_object *o1, ay_object *o2);

int ay_comp_parab(ay_object *o1, ay_object *o2);

int ay_comp_hyperb(ay_object *o1, ay_object *o2);

int ay_comp_box(ay_object *o1, ay_object *o2);

int ay_comp_bpatch(ay_object *o1, ay_object *o2);

int ay_comp_camera(ay_object *o1, ay_object *o2);

int ay_comp_riinc(ay_object *o1, ay_object *o2);

int ay_comp_light(ay_object *o1, ay_object *o2);

int ay_comp_ncurve(ay_object *o1, ay_object *o2);

int ay_comp_icurve(ay_object *o1, ay_object *o2);

int ay_comp_npatch(ay_object *o1, ay_object *o2);

int ay_comp_extrude(ay_object *o1, ay_object *o2);

int ay_comp_revolve(ay_object *o1, ay_object *o2);

int ay_comp_sweep(ay_object *o1, ay_object *o2);

int ay_comp_skin(ay_object *o1, ay_object *o2);

int ay_comp_material(ay_object *o1, ay_object *o2);

int ay_comp_pamesh(ay_object *o1, ay_object *o2);

int ay_comp_cap(ay_object *o1, ay_object *o2);

int ay_comp_concatnc(ay_object *o1, ay_object *o2);

int ay_comp_clone(ay_object *o1, ay_object *o2);

int ay_comp_pomesh(ay_object *o1, ay_object *o2);

int ay_comp_sdmesh(ay_object *o1, ay_object *o2);

int ay_comp_gordon(ay_object *o1, ay_object *o2);

int ay_comp_text(ay_object *o1, ay_object *o2);

int ay_comp_birail1(ay_object *o1, ay_object *o2);

int ay_comp_birail2(ay_object *o1, ay_object *o2);

int ay_comp_extrnc(ay_object *o1, ay_object *o2);


/* functions */

/* ay_comp_strcase:
 *  compare two strings ignoring case, s2 must point to an
 *  already lowercase version e.g.:
 *  ay_comp_strcase(s1, "lowercase_string");
 */
int
ay_comp_strcase(char *s1, char *s2)
{

  if(!s1 || !s2)
    return 1;

  while(*s1 != '\0')
    {
      if(tolower(*s1) != *s2)
	return 1;

      s1++;
      s2++;
    }

  if(*s1 != *s2)
    return 1;

 return 0;
} /* ay_comp_strcase */


/* ay_comp_trafos:
 *
 */
int
ay_comp_trafos(ay_object *o1, ay_object *o2)
{
  if((o1->movx == o2->movx) && (o1->movy == o2->movy) &&
     (o1->movz == o2->movz) && (o1->scalx == o2->scalx) &&
     (o1->scaly == o2->scaly) && (o1->scalz == o2->scalz) &&
     (!memcmp(o1->quat, o2->quat, sizeof(4*sizeof(double)))))
    {
      return AY_TRUE;
    }
  else
    {
      return AY_FALSE;
    }
} /* ay_comp_trafos */


/* ay_comp_tag:
 *
 */
int
ay_comp_tag(ay_tag_object *t1, ay_tag_object *t2)
{
  if(((t1->name != t2->name) && ((t1->name != NULL) || (t2->name != NULL)) &&
      (strcmp(t1->name, t2->name))))
    return AY_FALSE;
  if(((t1->type != t2->type) && ((t1->type != NULL) || (t2->type != NULL)) &&
      (strcmp(t1->type, t2->type))))
    return AY_FALSE;
  if(((t1->val != t2->val) && ((t1->val != NULL) || (t2->val != NULL)) &&
      (strcmp(t1->val, t2->val))))
    return AY_FALSE;

  return AY_TRUE;
} /* ay_comp_tag */


/* ay_comp_tags:
 *
 */
int
ay_comp_tags(ay_object *o1, ay_object *o2)
{
  ay_tag_object *t1, *t2;

  t1 = o1->tags;
  t2 = o2->tags;

  while((t1) && (t2) && (ay_comp_tag(t1, t2) == AY_TRUE))
    {
      t1 = t1->next;
      t2 = t2->next;
    }

  if((t1) || (t2))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_tags */


/* ay_comp_sarg:
 *
 */
int
ay_comp_sarg(ay_shader_arg *a1, ay_shader_arg *a2)
{
  if((a1->type != a2->type) || (strcmp(a1->name, a2->name)))
    return AY_FALSE;

  switch (a1->type) {
  case AY_SACOLOR:
    if(memcmp(a1->val.color, a2->val.color, 3*sizeof(float)))
      return AY_FALSE;
    break;
  case AY_SAPOINT:
  case AY_SANORMAL:
  case AY_SAVECTOR:
    if(memcmp(a1->val.point, a2->val.point, 4*sizeof(float)))
      return AY_FALSE;
    break;
  case AY_SAMATRIX:
    if(memcmp(a1->val.matrix, a2->val.matrix, 16*sizeof(float)))
      return AY_FALSE;
    break;
  case AY_SASCALAR:
    if(a1->val.scalar != a2->val.scalar)
      return AY_FALSE;
    break;
  case AY_SASTRING:
    if(strcmp(a1->val.string, a2->val.string))
      return AY_FALSE;
    break;
  }

  return AY_TRUE;
} /* ay_comp_sarg */


/* ay_comp_shader:
 *
 */
int
ay_comp_shader(ay_shader *s1, ay_shader *s2)
{
  ay_shader_arg *a1, *a2;

  if((s1->type != s2->type) || (strcmp(s1->name, s2->name)))
    return AY_FALSE;

  /* compare shader args */
  a1 = s1->arg;
  a2 = s2->arg;

  while((a1) && (a2) && (ay_comp_sarg(a1, a2) == AY_TRUE))
    {
      a1 = a1->next;
      a2 = a2->next;
    }

  if((a1 == NULL) && (a2 == NULL))
    return AY_TRUE;
  else
    return AY_FALSE;
} /* ay_comp_shader */


/* ay_comp_level:
 *
 */
int
ay_comp_level(ay_object *o1, ay_object *o2)
{
  /* do we compare the same level type? */
  if(memcmp(o1->refine, o2->refine, sizeof(ay_level_object)))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_level */


/* ay_comp_disk:
 *
 */
int 
ay_comp_disk(ay_object *o1, ay_object *o2)
{
  if(memcmp(o1->refine, o2->refine, sizeof(ay_disk_object)))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_disk */


/* ay_comp_cone:
 *
 */
int
ay_comp_cone(ay_object *o1, ay_object *o2)
{
  if(memcmp(o1->refine, o2->refine, sizeof(ay_cone_object)))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_cone */


/* ay_comp_cylinder:
 *
 */
int
ay_comp_cylinder(ay_object *o1, ay_object *o2)
{
  if(memcmp(o1->refine, o2->refine, sizeof(ay_cylinder_object)))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_cylinder */


/* ay_comp_sphere:
 *
 */
int
ay_comp_sphere(ay_object *o1, ay_object *o2)
{
  if(memcmp(o1->refine, o2->refine, sizeof(ay_sphere_object)))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_sphere */


/* ay_comp_torus:
 *
 */
int
ay_comp_torus(ay_object *o1, ay_object *o2)
{
  if(memcmp(o1->refine, o2->refine, sizeof(ay_torus_object)))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_torus */


/* ay_comp_parab:
 *
 */
int
ay_comp_parab(ay_object *o1, ay_object *o2)
{
  if(memcmp(o1->refine, o2->refine, sizeof(ay_paraboloid_object)))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_parab */


/* ay_comp_hyperb:
 *
 */
int
ay_comp_hyperb(ay_object *o1, ay_object *o2)
{
  if(memcmp(o1->refine, o2->refine, sizeof(ay_hyperboloid_object)))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_hyperb */


/* ay_comp_box:
 *
 */
int
ay_comp_box(ay_object *o1, ay_object *o2)
{
  if(memcmp(o1->refine, o2->refine, sizeof(ay_box_object)))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_box */


/* ay_comp_bpatch:
 *
 */
int
ay_comp_bpatch(ay_object *o1, ay_object *o2)
{
  if(memcmp(o1->refine, o2->refine, sizeof(ay_bpatch_object)))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_bpatch */


/* ay_comp_camera:
 *
 */
int
ay_comp_camera(ay_object *o1, ay_object *o2)
{
  if(memcmp(o1->refine, o2->refine, sizeof(ay_camera_object)))
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_comp_camera */


/* ay_comp_riinc:
 *
 */
int
ay_comp_riinc(ay_object *o1, ay_object *o2)
{
  ay_riinc_object *r1, *r2;

  r1 = (ay_riinc_object *)o1->refine;
  r2 = (ay_riinc_object *)o2->refine;

  if((r1->width != r2->width) ||
     (r1->length != r2->length) ||
     (r1->height != r2->height))
    return AY_FALSE;
  
  if(strcmp(r1->file, r2->file))
    return AY_FALSE;

  return AY_TRUE;
} /* ay_comp_riinc */


/* ay_comp_light:
 *
 */
int
ay_comp_light(ay_object *o1, ay_object *o2)
{
 ay_light_object *l1, *l2;

  l1 = (ay_light_object *)o1->refine;
  l2 = (ay_light_object *)o2->refine;

  if((l1->type != l2->type) ||
     (l1->on != l2->on) ||
     (l1->shadows != l2->shadows) ||
     (l1->samples != l2->samples) ||
     (l1->colr != l2->colr) ||
     (l1->colg != l2->colg) ||
     (l1->colb != l2->colb) ||
     (l1->intensity != l2->intensity) ||
     (l1->cone_angle != l2->cone_angle) ||
     (l1->cone_delta_angle != l2->cone_delta_angle) ||
     (l1->beam_distribution != l2->beam_distribution) ||
     (l1->use_sm != l2->use_sm) ||
     (l1->sm_resolution != l2->sm_resolution)
     )
    return AY_FALSE;

  if(memcmp(&(l1->tfrom), &(l2->tfrom), 3*sizeof(double)))
    return AY_FALSE;
  if(memcmp(&(l1->tto), &(l2->tto), 3*sizeof(double)))
    return AY_FALSE;

  if(l1->lshader && l2->lshader)
    {
      if(!(ay_comp_shader(l1->lshader, l2->lshader)))
	return AY_FALSE;
    }
  else
    {
      if(l1->lshader != l2->lshader)
	return AY_FALSE;
    }

  return AY_TRUE;
} /* ay_comp_light */


/* ay_comp_ncurve:
 *
 */
int
ay_comp_ncurve(ay_object *o1, ay_object *o2) 
{
 ay_nurbcurve_object *n1, *n2;


  n1 = (ay_nurbcurve_object *)o1->refine;
  n2 = (ay_nurbcurve_object *)o2->refine;

  if((n1->length != n2->length) ||
     (n1->order != n2->order) ||
     (n1->knot_type != n2->knot_type) ||
     (n1->closed != n2->closed))
    return AY_FALSE;

  /* now compare the knots and control points */
  if(memcmp(n1->knotv, n2->knotv, (n1->length+n1->order)*sizeof(double)))
    return AY_FALSE;
  if(memcmp(n1->controlv, n2->controlv, 4*n1->length*sizeof(double)))
    return AY_FALSE;
      
  return AY_TRUE;
} /* ay_comp_ncurve */


/* ay_comp_icurve:
 *
 */
int
ay_comp_icurve(ay_object *o1, ay_object *o2) 
{
 ay_icurve_object *i1, *i2;

  i1 = (ay_icurve_object *)o1->refine;
  i2 = (ay_icurve_object *)o2->refine;

  if((i1->length != i2->length) ||
     (i1->closed != i2->closed) ||
     (i1->imode != i2->imode) ||
     (i1->iorder != i2->iorder) ||
     (i1->iparam != i2->iparam)
     )
    return AY_FALSE;

  if(memcmp(i1->controlv, i2->controlv, 3*i1->length*sizeof(double)))
    return AY_FALSE;
      
  return AY_TRUE;
} /* ay_comp_icurve */


/* ay_comp_npatch:
 *
 */
int
ay_comp_npatch(ay_object *o1, ay_object *o2)
{
 ay_nurbpatch_object *n1, *n2;
 
  n1 = (ay_nurbpatch_object *)o1->refine;
  n2 = (ay_nurbpatch_object *)o2->refine;

  if((n1->width != n2->width) ||
     (n1->height != n2->height) ||
     (n1->uorder != n2->uorder) ||
     (n1->vorder != n2->vorder) ||
     (n1->uknot_type != n2->uknot_type) ||
     (n1->vknot_type != n2->vknot_type))
    return AY_FALSE;

  /* now compare the knots and control points */
  if(memcmp(n1->uknotv, n2->uknotv, (n1->width+n1->uorder)*sizeof(double)))
    return AY_FALSE;
  if(memcmp(n1->vknotv, n2->vknotv, (n1->height+n1->vorder)*sizeof(double)))
    return AY_FALSE;
  if(memcmp(n1->controlv, n2->controlv,4*n1->width*n1->height*sizeof(double)))
    return AY_FALSE;
 
 return AY_TRUE;
} /* ay_comp_npatch */


/* ay_comp_extrude:
 *
 */
int 
ay_comp_extrude(ay_object *o1, ay_object *o2)
{
  ay_extrude_object *e1, *e2;

  e1 = (ay_extrude_object *)o1->refine;
  e2 = (ay_extrude_object *)o2->refine;

  if((e1->height != e2->height) ||
     (e1->has_upper_cap != e2->has_upper_cap) ||
     (e1->has_lower_cap != e2->has_lower_cap) ||
     (e1->has_upper_bevels != e2->has_upper_bevels) ||
     (e1->has_lower_bevels != e2->has_lower_bevels) ||
     (e1->bevel_type != e2->bevel_type) ||
     (e1->bevel_radius != e2->bevel_radius))
    return AY_FALSE;

  return AY_TRUE;
} /* ay_comp_extrude */


/* ay_comp_revolve:
 *
 */
int
ay_comp_revolve(ay_object *o1, ay_object *o2)
{
  ay_revolve_object *r1, *r2;

  r1 = (ay_revolve_object *)o1->refine;
  r2 = (ay_revolve_object *)o2->refine;

  if((r1->has_upper_cap != r2->has_upper_cap) ||
     (r1->has_lower_cap != r2->has_lower_cap) ||
     (r1->has_start_cap != r2->has_start_cap) ||
     (r1->has_end_cap != r2->has_end_cap) ||
     (r1->thetamax != r2->thetamax) ||
     (r1->sections != r2->sections) ||
     (r1->order != r2->order)
     )
    return AY_FALSE;

  return AY_TRUE;
} /* ay_comp_revolve */


/* ay_comp_sweep:
 *
 */
int
ay_comp_sweep(ay_object *o1, ay_object *o2)
{
  ay_sweep_object *s1, *s2;

  s1 = (ay_sweep_object *)o1->refine;
  s2 = (ay_sweep_object *)o2->refine;

  if((s1->rotate != s2->rotate) ||
     (s1->interpolate != s2->interpolate) ||
     (s1->close != s2->close) ||
     (s1->sections != s2->sections) ||
     (s1->has_start_cap != s2->has_start_cap) ||
     (s1->has_end_cap != s2->has_end_cap) 
     )
    return AY_FALSE;

  return AY_TRUE;
} /* ay_comp_sweep */


/* ay_comp_skin:
 *
 */
int
ay_comp_skin(ay_object *o1, ay_object *o2)
{
  ay_skin_object *s1, *s2;

  s1 = (ay_skin_object *)o1->refine;
  s2 = (ay_skin_object *)o2->refine;

  if((s1->interpolate != s2->interpolate) ||
     (s1->uorder != s2->uorder) ||
     (s1->uknot_type != s2->uknot_type) ||
     (s1->has_start_cap != s2->has_start_cap) ||
     (s1->has_end_cap != s2->has_end_cap))
    return AY_FALSE;

 return AY_TRUE;
} /* ay_comp_skin */


/* ay_comp_material:
 *
 */
int
ay_comp_material(ay_object *o1, ay_object *o2)
{
 ay_mat_object *m1, *m2;

  m1 = (ay_mat_object *)o1->refine;
  m2 = (ay_mat_object *)o2->refine;

  if((m1->colr != m2->colr) || (m1->colg != m2->colg) ||
     (m1->colb != m2->colb) ||
     (m1->opr != m2->opr) || (m1->opg != m2->opg) ||
     (m1->opb != m2->opb) ||
     (m1->matte != m2->matte) ||
     (m1->shading_rate != m2->shading_rate) ||
     (m1->shading_interpolation != m2->shading_interpolation) ||
     (m1->dbound_val != m2->dbound_val) ||
     (m1->dbound != m2->dbound) ||
     (m1->sides != m2->sides) ||
     (m1->cast_shadows != m2->cast_shadows) ||
     (m1->avr != m2->avr) || (m1->avg != m2->avg) ||
     (m1->avb != m2->avb) || (m1->ava != m2->ava) ||
     (m1->emr != m2->emr) || (m1->emg != m2->emg) ||
     (m1->emb != m2->emb) || (m1->ema != m2->ema) ||
     (m1->spr != m2->spr) || (m1->spg != m2->spg) ||
     (m1->spb != m2->spb) || (m1->spa != m2->spa) ||
     (m1->patch_size != m2->patch_size) ||
     (m1->elem_size != m2->elem_size) ||
     (m1->min_size != m2->min_size) ||
     (m1->true_displacement != m2->true_displacement) ||
     (m1->zonal != m2->zonal) ||
     (m1->has_caustics != m2->has_caustics))
    return AY_FALSE;

  /* compare shaders */
  if(m1->sshader && m2->sshader)
    {
      if(!(ay_comp_shader(m1->sshader, m2->sshader)))
	return AY_FALSE;
    }
  else
    {
      if(m1->sshader != m2->sshader)
	return AY_FALSE;
    }

  if(m1->dshader && m2->dshader)
    {
      if(!(ay_comp_shader(m1->dshader, m2->dshader)))
	return AY_FALSE;
    }
  else
    {
      if(m1->dshader != m2->dshader)
	return AY_FALSE;
    }

  if(m1->ishader && m2->ishader)
    {
      if(!(ay_comp_shader(m1->ishader, m2->ishader)))
	return AY_FALSE;
    }
  else
    {
      if(m1->ishader != m2->ishader)
	return AY_FALSE;
    }

  if(m1->eshader && m2->eshader)
    {
      if(!(ay_comp_shader(m1->eshader, m2->eshader)))
	return AY_FALSE;
    }
  else
    {
      if(m1->eshader != m2->eshader)
	return AY_FALSE;
    }

 return AY_TRUE;
} /* ay_comp_material */


/* ay_comp_pamesh:
 *
 */
int
ay_comp_pamesh(ay_object *o1, ay_object *o2)
{
 ay_pamesh_object *p1, *p2;

  p1 = (ay_pamesh_object *)o1->refine;
  p2 = (ay_pamesh_object *)o2->refine;

  if((p1->width != p2->width) || (p1->height != p2->height) ||
     (p1->close_u != p2->close_u) || (p1->close_v != p2->close_v) ||
     (p1->btype_u != p2->btype_u) || (p1->btype_v != p2->btype_v) ||
     (p1->ustep != p2->ustep) || (p1->vstep != p2->vstep) ||
     (p1->type != p2->type))
    return AY_FALSE;

  if(p1->ubasis && p2->ubasis)
    {
      if(memcmp(p1->ubasis, p2->ubasis, 16*sizeof(double)))
	return AY_FALSE;
    }
  else
    {
      if(p1->ubasis != p2->ubasis)
	return AY_FALSE;
    }

  if(p1->vbasis && p2->vbasis)
    {
      if(memcmp(p1->vbasis, p2->vbasis, 16*sizeof(double)))
	return AY_FALSE;
    }
  else
    {
      if(p1->vbasis != p2->vbasis)
	return AY_FALSE;
    }

  if(p1->controlv && p2->controlv)
    {
      if(memcmp(p1->controlv, p2->controlv,
		p1->width*p1->height*4*sizeof(double)))
	return AY_FALSE;
    }

 return AY_OK;
} /* ay_comp_pamesh */


/* ay_comp_cap:
 *
 */
int
ay_comp_cap(ay_object *o1, ay_object *o2)
{
 ay_cap_object *p1, *p2;

  p1 = (ay_cap_object *)o1->refine;
  p2 = (ay_cap_object *)o2->refine;

 return AY_TRUE;
} /* ay_comp_cap */


/* ay_comp_concatnc:
 *
 */
int
ay_comp_concatnc(ay_object *o1, ay_object *o2)
{
 ay_concatnc_object *p1, *p2;

  p1 = (ay_concatnc_object *)o1->refine;
  p2 = (ay_concatnc_object *)o2->refine;

  if(p1->closed != p2->closed)
    return AY_FALSE;

  if(p1->fillgaps != p2->fillgaps)
    return AY_FALSE;

  if(p1->revert != p2->revert)
    return AY_FALSE;

  if(p1->knot_type != p2->knot_type)
    return AY_FALSE;

  if(p1->ftlength != p2->ftlength)
    return AY_FALSE;

 return AY_TRUE;
} /* ay_comp_concatnc */


/* ay_comp_clone:
 *
 */
int
ay_comp_clone(ay_object *o1, ay_object *o2)
{
 ay_clone_object *p1, *p2;

  p1 = (ay_clone_object *)o1->refine;
  p2 = (ay_clone_object *)o2->refine;

  if(p1->numclones != p2->numclones)
    return AY_FALSE;

  if(p1->rotate != p2->rotate)
    return AY_FALSE;

  if(p1->mirror != p2->mirror)
    return AY_FALSE;

  if((p1->movx == p2->movx) && (p1->movy == p2->movy) &&
     (p1->movz == p2->movz) && (p1->scalx == p2->scalx) &&
     (p1->scaly == p2->scaly) && (p1->scalz == p2->scalz) &&
     (!memcmp(p1->quat, p2->quat, sizeof(4*sizeof(double)))))
    {
      return AY_TRUE;
    }
  else
    {
      return AY_FALSE;
    }

 return AY_TRUE;
} /* ay_comp_clone */


/* ay_comp_pomesh:
 *
 */
int
ay_comp_pomesh(ay_object *o1, ay_object *o2)
{
 ay_pomesh_object *p1, *p2;
 unsigned int total_loops = 0, total_verts = 0;
 unsigned int i = 0;

  p1 = (ay_pomesh_object *)o1->refine;
  p2 = (ay_pomesh_object *)o2->refine;

  if(p1->npolys != p2->npolys)
    return AY_FALSE;

  if(p1->has_normals != p2->has_normals)
    return AY_FALSE;

  if(p1->ncontrols != p2->ncontrols)
    return AY_FALSE;

  if(p1->has_normals)
    {
      if(!memcmp(p1->controlv, p2->controlv,
		 sizeof(p1->ncontrols*6*sizeof(double))))
	return AY_FALSE;
    }
  else
    {
      if(!memcmp(p1->controlv, p2->controlv,
		 sizeof(p1->ncontrols*3*sizeof(double))))
	return AY_FALSE;
    }

  if(!memcmp(p1->nloops, p2->nloops, sizeof(p1->npolys*sizeof(unsigned int))))
    return AY_FALSE;

  for(i = 0; i < p1->npolys; i++)
    {
      total_loops += p1->nloops[i];
    } /* for */

  if(!memcmp(p1->nverts, p2->nverts, sizeof(total_loops*sizeof(unsigned int))))
    return AY_FALSE;

  for(i = 0; i < total_loops; i++)
    {
      total_verts += p1->nverts[i];
    } /* for */
  
  if(!memcmp(p1->verts, p2->verts, sizeof(total_verts*sizeof(unsigned int))))
    return AY_FALSE;

 return AY_TRUE;
} /* ay_comp_pomesh */


/* ay_comp_sdmesh:
 *
 */
int
ay_comp_sdmesh(ay_object *o1, ay_object *o2)
{
 ay_sdmesh_object *p1, *p2;
 unsigned int total_verts = 0;
 unsigned int i = 0;

  p1 = (ay_sdmesh_object *)o1->refine;
  p2 = (ay_sdmesh_object *)o2->refine;

  if(p1->nfaces != p2->nfaces)
    return AY_FALSE;

  if(p1->ncontrols != p2->ncontrols)
    return AY_FALSE;

  if(!memcmp(p1->controlv, p2->controlv,
	     sizeof(p1->ncontrols * 3 * sizeof(double))))
    return AY_FALSE;

  if(!memcmp(p1->nverts, p2->nverts,
	     sizeof(p1->nfaces * sizeof(unsigned int))))
    return AY_FALSE;

  for(i = 0; i < p1->nfaces; i++)
    {
      total_verts += p1->nverts[i];
    } /* for */
  
  if(!memcmp(p1->verts, p2->verts,
	     sizeof(total_verts * sizeof(unsigned int))))
    return AY_FALSE;

 return AY_TRUE;
} /* ay_comp_sdmesh */


/* ay_comp_gordon:
 *
 */
int
ay_comp_gordon(ay_object *o1, ay_object *o2)
{
 ay_gordon_object *s1, *s2;

  s1 = (ay_gordon_object *)o1->refine;
  s2 = (ay_gordon_object *)o2->refine;

  if((s1->uorder != s2->uorder) ||
     (s1->vorder != s2->vorder))
    return AY_FALSE;

 return AY_TRUE;
} /* ay_comp_gordon */


/* ay_comp_text:
 *
 */
int
ay_comp_text(ay_object *o1, ay_object *o2)
{
 ay_text_object *s1, *s2;
 unsigned long len = 0;

  s1 = (ay_text_object *)o1->refine;
  s2 = (ay_text_object *)o2->refine;

  if((s1->height != s2->height) ||
     (s1->revert != s2->revert) ||
     (s1->has_upper_cap != s2->has_upper_cap) ||
     (s1->has_lower_cap != s2->has_lower_cap) ||
     (s1->has_upper_bevels != s2->has_upper_bevels) ||
     (s1->has_lower_bevels != s2->has_lower_bevels) ||
     (s1->bevel_type != s2->bevel_type) ||
     (s1->bevel_radius != s2->bevel_radius) ||
     (s1->revert_bevels != s2->revert_bevels))
    return AY_FALSE;

  if(s1->fontname && s2->fontname)
    {
      if(strcmp(s1->fontname, s2->fontname))
	return AY_FALSE;
    }
  else
    {
      if(s1->fontname || s2->fontname)
	return AY_FALSE;
    }


  if(s1->unistring && s2->unistring)
    {
      if((len = Tcl_UniCharLen(s1->unistring)) !=
	 Tcl_UniCharLen(s2->unistring))
	return AY_FALSE;

      if(Tcl_UniCharNcmp(s1->unistring, s2->unistring, len))
	return AY_FALSE;
    }
  else
    {
      if(s1->unistring || s2->unistring)
	return AY_FALSE;
    }

 return AY_TRUE;
} /* ay_comp_text */


/* ay_comp_birail1:
 *
 */
int
ay_comp_birail1(ay_object *o1, ay_object *o2)
{
 ay_birail1_object *s1, *s2;

  s1 = (ay_birail1_object *)o1->refine;
  s2 = (ay_birail1_object *)o2->refine;

  if((s1->close != s2->close) ||
     (s1->sections != s2->sections) ||
     (s1->has_start_cap != s2->has_start_cap) ||
     (s1->has_end_cap != s2->has_end_cap))
    return AY_FALSE;

 return AY_TRUE;
} /* ay_comp_birail1 */


/* ay_comp_birail2:
 *
 */
int
ay_comp_birail2(ay_object *o1, ay_object *o2)
{
 ay_birail2_object *s1, *s2;

  s1 = (ay_birail2_object *)o1->refine;
  s2 = (ay_birail2_object *)o2->refine;

  if((s1->close != s2->close) ||
     (s1->sections != s2->sections) ||
     (s1->has_start_cap != s2->has_start_cap) ||
     (s1->has_end_cap != s2->has_end_cap))
    return AY_FALSE;

 return AY_TRUE;
} /* ay_comp_birail2 */


/* ay_comp_extrnc:
 *
 */
int
ay_comp_extrnc(ay_object *o1, ay_object *o2)
{
 ay_extrnc_object *s1, *s2;

  s1 = (ay_extrnc_object *)o1->refine;
  s2 = (ay_extrnc_object *)o2->refine;

  if((s1->side != s2->side) ||
     (s1->parameter != s2->parameter))
    return AY_FALSE;

 return AY_TRUE;
} /* ay_comp_extrnc */


/* ay_comp_register:
 *  register the compare callback compcb for
 *  objects of type type_id
 */
int
ay_comp_register(ay_comparecb *compcb, unsigned int type_id)
{
 int ay_status = AY_OK;
 int overload = ay_prefs.overload;

  /* register comp callback */
  ay_status = ay_table_additem(&ay_comparecbt, (void*)compcb, type_id,
			       overload);

 return ay_status;
} /* ay_comp_register */


/* ay_comp_objects:
 *  compare object o1 with o2,
 *  return AY_TRUE if they are equal, else AY_FALSE
 *  Note: This is not a deep compare (children are not considered)!
 *  Note^2: Types of o1 and o2 should be checked beforehand, they
 *          must match!
 */
int
ay_comp_objects(ay_object *o1, ay_object *o2)
{
 int ay_status = AY_OK;
 /* char fname[] = "comp_objects";*/
 void **arr = NULL;
 ay_comparecb *cb = NULL;


  /* call the comp callback */
  arr = ay_comparecbt.arr;
  cb = (ay_comparecb *)(arr[o1->type]);
  if(cb)
    {
      ay_status = cb(o1, o2);
    }
  /*
    else
    {
    ay_error(AY_EWARN, fname, "No compare callback registered!"); 
    }
  */

 return ay_status;
} /* ay_comp_objects */


/* ay_comp_init:
 *
 */
int
ay_comp_init()
{
 int ay_status = AY_OK;
 char fname[] = "comp_init";

  if((ay_status = ay_table_init(&ay_comparecbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }


  ay_status = ay_comp_register(ay_comp_box, AY_IDBOX);
  ay_status = ay_comp_register(ay_comp_bpatch, AY_IDBPATCH);

  ay_status = ay_comp_register(ay_comp_sphere, AY_IDSPHERE);
  ay_status = ay_comp_register(ay_comp_disk, AY_IDDISK);
  ay_status = ay_comp_register(ay_comp_cylinder, AY_IDCYLINDER);
  ay_status = ay_comp_register(ay_comp_cone, AY_IDCONE);
  ay_status = ay_comp_register(ay_comp_torus, AY_IDTORUS);
  ay_status = ay_comp_register(ay_comp_hyperb, AY_IDHYPERBOLOID);
  ay_status = ay_comp_register(ay_comp_parab, AY_IDPARABOBOLOID);

  ay_status = ay_comp_register(ay_comp_ncurve, AY_IDNCURVE);
  ay_status = ay_comp_register(ay_comp_icurve, AY_IDICURVE);
  ay_status = ay_comp_register(ay_comp_npatch, AY_IDNPATCH);

  ay_status = ay_comp_register(ay_comp_riinc, AY_IDRIINC);
  ay_status = ay_comp_register(ay_comp_level, AY_IDLEVEL);

  ay_status = ay_comp_register(ay_comp_light, AY_IDLIGHT);
  ay_status = ay_comp_register(ay_comp_camera, AY_IDCAMERA);
  ay_status = ay_comp_register(ay_comp_material, AY_IDMATERIAL);

  ay_status = ay_comp_register(ay_comp_revolve, AY_IDREVOLVE);
  ay_status = ay_comp_register(ay_comp_extrude, AY_IDEXTRUDE);
  ay_status = ay_comp_register(ay_comp_sweep, AY_IDSWEEP);
  ay_status = ay_comp_register(ay_comp_skin, AY_IDSKIN);
  ay_status = ay_comp_register(ay_comp_pamesh, AY_IDPAMESH);
  ay_status = ay_comp_register(ay_comp_cap, AY_IDCAP);
  ay_status = ay_comp_register(ay_comp_concatnc, AY_IDCONCATNC);
  ay_status = ay_comp_register(ay_comp_clone, AY_IDCLONE);
  ay_status = ay_comp_register(ay_comp_pomesh, AY_IDPOMESH);
  ay_status = ay_comp_register(ay_comp_sdmesh, AY_IDSDMESH);
  ay_status = ay_comp_register(ay_comp_gordon, AY_IDGORDON);
  ay_status = ay_comp_register(ay_comp_text, AY_IDTEXT);
  ay_status = ay_comp_register(ay_comp_birail1, AY_IDBIRAIL1);
  ay_status = ay_comp_register(ay_comp_birail2, AY_IDBIRAIL2);
  ay_status = ay_comp_register(ay_comp_extrnc, AY_IDEXTRNC);


 return ay_status;
} /* ay_comp_init */

