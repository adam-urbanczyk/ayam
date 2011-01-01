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

#include "ayam.h"

// subdiv includes
#include "vertex.h"
#include "quadmesh.h"
#include "trimesh.h"


// prototypes of functions local to this module

extern "C" {

  int subdiv_notifycb(ay_object *o);

#ifdef WIN32
  __declspec (dllexport)
#endif // WIN32
int Subdiv_Init(Tcl_Interp *interp);

} // extern "C"


// functions

/* subdiv_notifycb:
 *  notification callback function of sdmesh object
 */
int
subdiv_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_sdmesh_object *sdmesh = NULL;
 unsigned int i, j = 0;
 Vertex *cv = NULL;
 QuadMesh *qm = NULL;
 TriMesh *tm = NULL;
 ay_pomesh_object *po = NULL;
 ay_object *newo = NULL;

  if(!o)
    return AY_ENULL;

  sdmesh = (ay_sdmesh_object *) o->refine;

  if(sdmesh->level == 0)
    {
      ay_object_delete(sdmesh->pomesh);
      sdmesh->pomesh = NULL;
      return AY_OK;
    }

  cv = new Vertex[sdmesh->ncontrols];

  for(i = 0; i < sdmesh->ncontrols; ++i)
    {
      cv[i].setPos(cvec3f((float)sdmesh->controlv[j],
			  (float)sdmesh->controlv[j+1],
			  (float)sdmesh->controlv[j+2]));
      Vertex::ref(&cv[i]);
      j += 3;
    }

  if(!sdmesh->pomesh)
    {
      if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      newo->type = AY_IDPOMESH;
      ay_object_defaults(newo);
      if(!(po = (ay_pomesh_object*)calloc(1, sizeof(ay_pomesh_object))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      po->has_normals = AY_TRUE;
      newo->refine = po;
      sdmesh->pomesh = newo;
    }
  else
    {
      po = (ay_pomesh_object*)sdmesh->pomesh->refine;
    }

  if(po->controlv)
    free(po->controlv);
  po->controlv = NULL;

  if(po->nverts)
    free(po->nverts);
  po->nverts = NULL;

  if(po->verts)
    free(po->verts);
  po->verts = NULL;

  if(sdmesh->scheme == AY_SDSCATMULL)
    {
      qm = new QuadMesh(cv, sdmesh->nfaces, sdmesh->nverts, sdmesh->verts);

      qm->subdivide(sdmesh->level);

      qm->toAyam(&po->controlv, &po->ncontrols,
		 &po->nverts, &po->verts, &po->npolys);

      delete qm;
    }
  else
    {
      tm = new TriMesh(cv, sdmesh->nfaces, sdmesh->nverts, sdmesh->verts);

      tm->subdivide(sdmesh->level);

      tm->toAyam(&po->controlv, &po->ncontrols,
		 &po->nverts, &po->verts, &po->npolys);

      delete tm;
    }

  if(po->nloops)
    free(po->nloops);
  po->nloops = NULL;

  if(!(po->nloops = (unsigned int*)calloc(po->npolys,
					  sizeof(unsigned int))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  for(i = 0; i < po->npolys; ++i )
    {
      po->nloops[i] = 1;
    }

cleanup:

  delete[] cv;

  if(ay_status)
    {
      ay_object_delete(sdmesh->pomesh);
      sdmesh->pomesh = NULL;
    }

 return ay_status;
} /* subdiv_notifycb */


/* Subdiv_Init:
 */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int
Subdiv_Init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "Subdiv_Init";

  ay_status = ay_notify_register(subdiv_notifycb, AY_IDSDMESH);

  ay_error(AY_EOUTPUT, fname,
	   "Plugin Subdiv successfully loaded.");

 return TCL_OK;
} /* Subdiv_Init */

