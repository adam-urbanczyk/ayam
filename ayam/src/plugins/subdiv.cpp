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

// Ayam includes
#include "ayam.h"

// libsub includes
#include "vertex.h"
#include "quadmesh.h"
#include "trimesh.h"


char subdiv_version_ma[] = AY_VERSIONSTR;
char subdiv_version_mi[] = AY_VERSIONSTRMI;

// prototypes of functions local to this module:

extern "C" {

int subdiv_notifycb(ay_object *o);

#ifdef WIN32
  __declspec (dllexport)
#endif // WIN32
int Subdiv_Init(Tcl_Interp *interp);

} // extern "C"


// functions:

/* subdiv_notifycb:
 *  replacement notification callback function of sdmesh object
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

  if(sdmesh->face_normals)
    free(sdmesh->face_normals);
  sdmesh->face_normals = NULL;

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
      // reference all verts so that deleting a mesh object
      // does not delete the verts (with the wrong delete)
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
      // re-use existing pomesh
      po = (ay_pomesh_object*)sdmesh->pomesh->refine;

      if(po->controlv)
	free(po->controlv);
      po->controlv = NULL;

      if(po->verts)
	free(po->verts);
      po->verts = NULL;

      if(po->nverts)
	free(po->nverts);
      po->nverts = NULL;

      if(po->nloops)
	free(po->nloops);
      po->nloops = NULL;
    }

  if(sdmesh->scheme == AY_SDSCATMULL)
    {
      try {
	qm = new QuadMesh(cv, sdmesh->nfaces, sdmesh->nverts, sdmesh->verts);
	qm->subdivide(sdmesh->level);
      } catch (...) {
	ay_status = AY_ERROR;
	delete qm;
	goto cleanup;
      }

      qm->toAyam(&po->controlv, &po->ncontrols,
		 &po->nverts, &po->verts, &po->npolys);

      delete qm;
    }
  else
    {
      try {
	tm = new TriMesh(cv, sdmesh->nfaces, sdmesh->nverts, sdmesh->verts);
	tm->subdivide(sdmesh->level);
      } catch (...) {
	ay_status = AY_ERROR;
	delete qm;
	goto cleanup;
      }

      tm->toAyam(&po->controlv, &po->ncontrols,
		 &po->nverts, &po->verts, &po->npolys);

      delete tm;
    }

  if(!(po->nloops = (unsigned int*)malloc(po->npolys*sizeof(unsigned int))))
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

  if(ay_checkversion(fname, subdiv_version_ma, subdiv_version_mi))
    {
      return TCL_ERROR;
    }

  ay_status = ay_notify_register(subdiv_notifycb, AY_IDSDMESH);

  ay_error(AY_EOUTPUT, fname,
	   "Plugin Subdiv successfully loaded.");

 return TCL_OK;
} /* Subdiv_Init */

