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

/* mopsi.c - Mops scene file import */

/* functions local to this module */
int ay_mopsi_string(FILE *fileptr, char **result);

int ay_mopsi_skip(FILE *fileptr);

int ay_mopsi_header(FILE *fileptr);

int ay_mopsi_rioptions(FILE *fileptr, int insert);

int ay_mopsi_views(Tcl_Interp *interp, FILE *fileptr, int insert);

int ay_mopsi_root(Tcl_Interp *interp, FILE *fileptr, int insert);

int ay_mopsi_level(FILE *fileptr, ay_object **optr);

int ay_mopsi_attributes(FILE *fileptr);

int ay_mopsi_trafos(FILE *fileptr);

int ay_mopsi_shaders(FILE *fileptr);

int ay_mopsi_tags(FILE *fileptr);

int ay_mopsi_nurbcurve(FILE *fileptr, ay_object *o);

int ay_mopsi_nurbpatch(FILE *fileptr, ay_object *o);

int ay_mopsi_box(FILE *fileptr, ay_object *o);

int ay_mopsi_sphere(FILE *fileptr, ay_object *o);

int ay_mopsi_cone(FILE *fileptr, ay_object *o);

int ay_mopsi_disk(FILE *fileptr, ay_object *o);

int ay_mopsi_cylinder(FILE *fileptr, ay_object *o);

int ay_mopsi_hyperboloid(FILE *fileptr, ay_object *o);

int ay_mopsi_paraboloid(FILE *fileptr, ay_object *o);

int ay_mopsi_torus(FILE *fileptr, ay_object *o);

int ay_mopsi_instance(FILE *fileptr, ay_object *o);

int ay_mopsi_light(FILE *fileptr, ay_object *o);

int ay_mopsi_object(FILE *fileptr);

int ay_mopsi_scene(Tcl_Interp *interp, char *filename, int insert);


/* global variables */ 
static int ay_mopsi_version;

static ay_object *ay_mopsi_lastread;

static int ay_mopsi_materialnumber;

static ay_mat_object ay_mopsi_readattr;

static Tcl_HashTable ay_mopsiht;

/* local definitions */
/* Mops core object types */
#define MOPS_OTCUSTOM        0
#define MOPS_OTNURBPATCH     1
#define MOPS_OTNURBCURVE     2
#define MOPS_OTLEVEL         3
#define MOPS_OTLIGHT         4
#define MOPS_OTROOT          5
#define MOPS_OTSPHERE        6
#define MOPS_OTDISK          7
#define MOPS_OTCONE          8
#define MOPS_OTCYLINDER      9
#define MOPS_OTPARABOLOID   10
#define MOPS_OTHYPERBOLOID  11
#define MOPS_OTTORUS        12
#define MOPS_OTBOX          13
#define MOPS_OTINSTANCE     14
#define MOPS_OTATTRIB       20
#define MOPS_OTTRAFO        21
#define MOPS_OTSHADER       30
#define MOPS_OTVIEW         40
#define MOPS_OTTAG          50

/* Mops Level Types */
#define MOPS_LTUNION        0
#define MOPS_LTINTERSECTION 1
#define MOPS_LTDIFFERENCE   2
#define MOPS_LTPRIMITIVE    3
#define MOPS_LTLEVEL        4
#define MOPS_LTTRIMLOOP     5
#define MOPS_LTDOWN         6
#define MOPS_LTENDLEVEL    10

/* Mops Light Types */
#define MOPS_LITCUSTOM  0
#define MOPS_LITPOINT   1
#define MOPS_LITSPOT    2
#define MOPS_LITDIST    3

/* Shader Argument Types */
#define MOPS_SACOLOR 0
#define MOPS_SAPOINT 1
#define MOPS_SASCALAR 2
#define MOPS_SASTRING 3
#define MOPS_SAVECTOR 4

/* functions */

int
ay_mopsi_string(FILE *fileptr, char **result)
{
 int ay_status = AY_OK;
 char readchar, *str;
 int read;
 Tcl_DString ds;

  Tcl_DStringInit(&ds);
  read = getc(fileptr);

  if(read == EOF)
    {Tcl_DStringFree(&ds); return AY_EUEOF;}

  if((char)read == '\n')
    {Tcl_DStringFree(&ds); return AY_OK;}

  while((char)read != '\n')
    {
      readchar = (char)read;

      Tcl_DStringAppend(&ds, &readchar, 1);
      read = getc(fileptr);

      if(read == EOF)
	{Tcl_DStringFree(&ds); return AY_EUEOF;}

    }

  Tcl_DStringAppend(&ds,"\0", 1);

  str = Tcl_DStringValue(&ds);

  if(str[strlen(str)-1] == '\r')
    {
      str[strlen(str)-1] = '\0';
    }

  if(!(*result = calloc(strlen(str)+1,sizeof(char))))
    {Tcl_DStringFree(&ds); return AY_EOMEM;}

  strcpy(*result,str);

  Tcl_DStringFree(&ds);

 return ay_status;
}

/* skip to next entity, marked by a single Bell, or two Bell's! */
int
ay_mopsi_skip(FILE *fileptr)
{
 int ay_status = AY_OK;
 int read;

  read = getc(fileptr);

  if(read == EOF)
    {return AY_EEOF;}

  if((char)read == '\a')
    {return AY_OK;}

  while((char)read != '\a')
    {
      read = getc(fileptr);

      if(read == EOF)
	return AY_EEOF;
    }

 return ay_status;
}


int 
ay_mopsi_header(FILE *fileptr)
{
 int ay_status = AY_OK;
 char *version = NULL, *nbuffer = NULL /*nbuffer[7]*/;
 

 /*
   fgets(nbuffer,7,fileptr);
   if(strcmp(nbuffer,"Mops"))
   return AY_NOTAMOPS;
 */

 ay_mopsi_string(fileptr, &nbuffer);
 if(!strstr(nbuffer,"Mops"))
   return AY_EFORMAT;

 ay_mopsi_string(fileptr, &version);

 ay_mopsi_version = 6;

 if(!strcmp(version,"0.42a"))
   ay_mopsi_version = 0;
 else if(!strcmp(version,"0.42a2"))
   ay_mopsi_version = 1;
 else if(!strcmp(version,"0.42a3"))
   ay_mopsi_version = 2;
 else if(!strcmp(version,"0.42a4"))
   ay_mopsi_version = 3;
 else if(!strcmp(version,"0.42a5"))
   ay_mopsi_version = 4;
 else if(!strcmp(version,"0.42b"))
   ay_mopsi_version = 5;
 else if(!strcmp(version,"0.42c"))
   ay_mopsi_version = 6;
 else if(!strcmp(version,"0.42d"))
   ay_mopsi_version = 7;

 free(version);

 return ay_status;
}


int 
ay_mopsi_rioptions(FILE *fileptr, int insert)
{
 int ay_status = AY_OK;
 int itemp = 0;
 int read;
 ay_root_object *root = NULL;
 ay_riopt_object *riopt = NULL;

  root = (ay_root_object *)(ay_root->refine);

  if(!root)
    return AY_ENULL;

  /* read RiOptions */
  if(!(riopt = calloc(1, sizeof(ay_riopt_object))))
    { return AY_EOMEM; }


  fscanf(fileptr,"%lg\n",&riopt->Variance);
  fscanf(fileptr,"%lg\n",&riopt->Samples_X);
  fscanf(fileptr,"%lg\n",&riopt->Samples_Y);
  fscanf(fileptr,"%d\n",&itemp);
  riopt->FilterFunc = (char)itemp;
  fscanf(fileptr,"%d\n",&riopt->FilterWidth);
  fscanf(fileptr,"%d\n",&riopt->FilterHeight);
  fscanf(fileptr,"%lg\n",&riopt->ExpGain);
  fscanf(fileptr,"%lg\n",&riopt->ExpGamma);
  fscanf(fileptr,"%lg\n",&riopt->RGBA_ONE);
  fscanf(fileptr,"%lg\n",&riopt->RGBA_MIN);
  fscanf(fileptr,"%lg\n",&riopt->RGBA_MAX);
  fscanf(fileptr,"%lg\n",&riopt->RGBA_Dither);
  fscanf(fileptr,"%d\n",&riopt->MinSamples);
  fscanf(fileptr,"%d\n",&riopt->MaxSamples);
  fscanf(fileptr,"%d\n",&riopt->MaxRayLevel);
  fscanf(fileptr,"%lg\n",&riopt->ShadowBias);
  fscanf(fileptr,"%d\n",&itemp);
  riopt->PRManSpec = itemp;
  fscanf(fileptr,"%d\n",&riopt->RadSteps);
  fscanf(fileptr,"%d",&riopt->PatchSamples);
  read = fgetc(fileptr);

  if(read == '\r')
    fgetc(fileptr);

  if(riopt->textures)
    {
      free(riopt->textures);
      riopt->textures = NULL;
    }
  ay_mopsi_string(fileptr, &(riopt->textures));
  if(riopt->archives)
    {
      free(riopt->archives);
      riopt->archives = NULL;
    }
  ay_mopsi_string(fileptr, &(riopt->archives));
  if(riopt->shaders)
    {
      free(riopt->shaders);
      riopt->shaders = NULL;
    }
  ay_mopsi_string(fileptr, &(riopt->shaders));

  fscanf(fileptr,"%d\n",&riopt->texturemem);
  fscanf(fileptr,"%d\n",&riopt->geommem);

  if(ay_mopsi_version > 4)
    {
      fscanf(fileptr,"%d\n",&riopt->width);
      fscanf(fileptr,"%d\n",&riopt->height);
    }

  if(root->riopt)
    free(root->riopt);

  root->riopt = riopt;

 return ay_status;
}


int 
ay_mopsi_views(Tcl_Interp *interp, FILE *fileptr, int insert)
{
 int ay_status = AY_OK;
 int viewcount = 0, i;
 int width, height;
 char command[255] = {0}, update_cmd[] = "update";
 ay_object *root = ay_root, *o;
 ay_view_object vtemp = {0}, *v = NULL;
 double ddummy;
 int idummy;

  /* skip to next entity */
  ay_status = ay_mopsi_skip(fileptr);
  fscanf(fileptr,"%d\n", &viewcount);

  for(i=0;i<viewcount;i++)
    {
      /* skip to next entity */
      ay_status = ay_mopsi_skip(fileptr);
      fscanf(fileptr,"%d\n", &width);
      fscanf(fileptr,"%d\n", &height);
      fscanf(fileptr,"%d\n", &vtemp.type);
      vtemp.up[0] = 0.0;
      vtemp.up[1] = 0.0;
      vtemp.up[2] = 0.0;
      if(vtemp.type != AY_VTTOP)
	{
	  vtemp.up[1] = 1.0;
	}
      else
	{
	  vtemp.up[2] = -1.0;
	}
      fscanf(fileptr,"%lg\n", &vtemp.grid);
      fscanf(fileptr,"%lg\n", &ddummy);
      fscanf(fileptr,"%lg\n", &ddummy);
      fscanf(fileptr,"%lg\n", &ddummy);
      fscanf(fileptr,"%lg\n", &ddummy);
      fscanf(fileptr,"%d\n", &idummy);
      fscanf(fileptr,"%d\n", &vtemp.shade);
      fscanf(fileptr,"%d\n", &vtemp.drawsel);
      fscanf(fileptr,"%d\n", &vtemp.redraw);
      fscanf(fileptr,"%d\n", &vtemp.drawgrid);
      if(vtemp.drawgrid)
	vtemp.usegrid = AY_TRUE;
      fscanf(fileptr,"%lg\n", &vtemp.from[0]);
      fscanf(fileptr,"%lg\n", &vtemp.from[1]);
      fscanf(fileptr,"%lg\n", &vtemp.from[2]);
      fscanf(fileptr,"%lg\n", &vtemp.to[0]);
      fscanf(fileptr,"%lg\n", &vtemp.to[1]);
      fscanf(fileptr,"%lg\n", &vtemp.to[2]);
      fscanf(fileptr,"%lg\n", &vtemp.roll);
      fscanf(fileptr,"%lg\n", &vtemp.zoom);
      if(ay_mopsi_version > 0)
	fscanf(fileptr,"%d\n", &idummy);
      if(ay_mopsi_version > 2)
	{
	  fscanf(fileptr,"%d\n", &vtemp.pos_x);
	  fscanf(fileptr,"%d\n", &vtemp.pos_y);

	}

      if(ay_mopsi_version > 4)
	{
	  fscanf(fileptr,"%lg\n", &vtemp.rotx);
	  fscanf(fileptr,"%lg\n", &vtemp.roty);
	  fscanf(fileptr,"%lg\n", &vtemp.rotz);
	  fscanf(fileptr,"%lg\n", &vtemp.up[0]);
	  fscanf(fileptr,"%lg\n", &vtemp.up[1]);
	  fscanf(fileptr,"%lg\n", &vtemp.up[2]);
	}
      else
	{
	  vtemp.rotx = 0.0;
	  vtemp.rotz = 0.0;
	  vtemp.roty = 180+AY_R2D(atan2((vtemp.to[0]-vtemp.from[0]),
					  (vtemp.to[2]-vtemp.from[2])));

	}

      vtemp.drawhandles = AY_TRUE;

      /* open the view */
      sprintf(command, "viewOpen %d %d 0\n", width, height);
      Tcl_Eval(interp, command);
      Tcl_Eval(interp, update_cmd);
      
      /* configure it */
      o = root->down;
      if(o->next)
	v = (ay_view_object *)o->refine;
      while(o->next)
	{
	  v = (ay_view_object *)o->refine;
	  o = o->next;
	}

      vtemp.togl = v->togl;
      memcpy(v, &vtemp, sizeof(ay_view_object));
      ay_toglcb_reshape(v->togl);


      /* set window title */
      if(vtemp.type == AY_VTFRONT)
	sprintf(command,
	 "global ay;viewTitle [lindex $ay(views) end] Front Pick");
      if(vtemp.type == AY_VTSIDE)
	sprintf(command,
	 "global ay;viewTitle [lindex $ay(views) end] Side Pick");
      if(vtemp.type == AY_VTTOP)
	sprintf(command,
	"global ay;viewTitle [lindex $ay(views) end] Top Pick");
      if(vtemp.type == AY_VTPERSP)
	sprintf(command,
         "global ay;viewTitle [lindex $ay(views) end] Persp Pick");
      if(vtemp.type == AY_VTTRIM)
	sprintf(command,
         "global ay;viewTitle [lindex $ay(views) end] Trim Pick");

      Tcl_Eval(interp, command);
      /* position window */
      sprintf(command,
"global ay;winMoveOrResize [lindex $ay(views) end] \"+%d+%d\"\n",
	      vtemp.pos_x, vtemp.pos_y);

      Tcl_Eval(interp, command);
      Tcl_Eval(interp, update_cmd);

    }

 return ay_status;
}


int 
ay_mopsi_root(Tcl_Interp *interp, FILE *fileptr, int insert)
{
 int ay_status = AY_OK;
 ay_object *root = ay_root;

 if(!root)
   return AY_OK;

 ay_status = ay_mopsi_rioptions(fileptr, insert);
 if(ay_status)
   return ay_status;

 ay_status = ay_mopsi_views(interp, fileptr, insert);
 if(ay_status)
   return ay_status;

 return ay_status;
}


int 
ay_mopsi_level(FILE *fileptr, ay_object **optr)
{
 int ay_status = AY_OK;
 ay_level_object *level = NULL;
 int itemp = 0;
 ay_object *o = NULL;
 char read, *c;

  o = *optr;

  if(!(level = calloc(1, sizeof(ay_level_object))))
    { return AY_EOMEM; }

  o->refine = level;
  o->parent = AY_TRUE;

  if(ay_mopsi_version > 0)
    {
      fscanf(fileptr,"%d",&itemp);
      read = (char)fgetc(fileptr);
      if(read == '\r')
	fgetc(fileptr);
    }
  else
    {
      fscanf(fileptr,"%d\n",&itemp);
    }

  switch(itemp)
    {
    case MOPS_LTUNION:
      level->type = AY_LTUNION;
      break;
    case MOPS_LTINTERSECTION:
      level->type = AY_LTINT;
      break;
    case MOPS_LTDIFFERENCE:
      level->type = AY_LTDIFF;
      break;
    case MOPS_LTPRIMITIVE:
      level->type = AY_LTPRIM;
      break;
    case MOPS_LTLEVEL:
      level->type = AY_LTLEVEL;
      break;
    case MOPS_LTTRIMLOOP:
      level->type = AY_LTLEVEL;
      break;
    case MOPS_LTENDLEVEL:
      level->type = AY_LTEND;
      break;
    default:
      level->type = AY_LTLEVEL;
      break;
    }

  if(itemp == MOPS_LTENDLEVEL)
    {
      if(ay_mopsi_version > 0)
	fscanf(fileptr,"\n");

      ay_object_link(o);
      *optr = NULL;
      ay_clevel_del();
      ay_mopsi_lastread = ay_currentlevel->object;
      ay_next = &(ay_mopsi_lastread->next);
      ay_clevel_del();

      return ay_status;
    }

  if(itemp == MOPS_LTDOWN)
    {
      if(ay_mopsi_version > 0)
	fscanf(fileptr,"\n");
      free(level);
      free(o);
      *optr = NULL;

      ay_clevel_add(ay_mopsi_lastread);
      ay_clevel_add(ay_mopsi_lastread->down);
      ay_next = &(ay_mopsi_lastread->down);
      return ay_status;
    }

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  /* convert all spaces to underscores in object names
     (they confuse the tree) */
  if(o->name)
    {
      c = o->name;
      while(*c != '\0')
	{
	  if(*c == ' ')
	    {
	      *c = '_';
	    }
	  c++;
	}
    }

  /*  ay_status = ay_object_defaults(o);*/
  o->parent = AY_TRUE;
  ay_status = ay_object_link(o);
  ay_clevel_add(o);
  ay_clevel_add(o->down);
  ay_next = &(o->down);
  *optr = NULL;


 return ay_status;
}


int 
ay_mopsi_attributes(FILE *fileptr)
{
 int ay_status = AY_OK;
 ay_object *o = ay_mopsi_lastread;
 int idummy;

  if(!o)
    return AY_OK;


  memset(&ay_mopsi_readattr, 0, sizeof(ay_mat_object));
  ay_mopsi_readattr.colr = 220;
  ay_mopsi_readattr.colg = 220;
  ay_mopsi_readattr.colb = 220;

  ay_mopsi_readattr.opr = 255;
  ay_mopsi_readattr.opg = 255;
  ay_mopsi_readattr.opb = 255;

  ay_mopsi_readattr.camera = 1;
  ay_mopsi_readattr.reflection = 1;
  ay_mopsi_readattr.shadow = 1;
  ay_mopsi_readattr.shading_rate = 1.0;



  fscanf(fileptr,"%lg\n",&o->movx);
  fscanf(fileptr,"%lg\n",&o->movy);
  fscanf(fileptr,"%lg\n",&o->movz);

  fscanf(fileptr,"%lg\n",&o->rotx);
  fscanf(fileptr,"%lg\n",&o->roty);
  fscanf(fileptr,"%lg\n",&o->rotz);

  fscanf(fileptr,"%lg\n",&o->quat[0]);
  fscanf(fileptr,"%lg\n",&o->quat[1]);
  fscanf(fileptr,"%lg\n",&o->quat[2]);
  fscanf(fileptr,"%lg\n",&o->quat[3]);

  fscanf(fileptr,"%lg\n",&o->scalx);
  fscanf(fileptr,"%lg\n",&o->scaly);
  fscanf(fileptr,"%lg\n",&o->scalz);

  fscanf(fileptr,"%d\n",&ay_mopsi_readattr.colr);
  fscanf(fileptr,"%d\n",&ay_mopsi_readattr.colg);
  fscanf(fileptr,"%d\n",&ay_mopsi_readattr.colb);
  fscanf(fileptr,"%d\n",&idummy);

  fscanf(fileptr,"%d\n",&ay_mopsi_readattr.opr);
  fscanf(fileptr,"%d\n",&ay_mopsi_readattr.opg);
  fscanf(fileptr,"%d\n",&ay_mopsi_readattr.opb);
  fscanf(fileptr,"%d\n",&idummy);

  fscanf(fileptr,"%lg\n",&ay_mopsi_readattr.shading_rate);
  fscanf(fileptr,"%d\n",&ay_mopsi_readattr.shading_interpolation);

  fscanf(fileptr,"%d\n",&ay_mopsi_readattr.cast_shadows);

  fscanf(fileptr,"%d\n",&ay_mopsi_readattr.true_displacement);
  fscanf(fileptr,"%d\n",&ay_mopsi_readattr.dbound);
  fscanf(fileptr,"%lg\n",&ay_mopsi_readattr.dbound_val);

  if(ay_mopsi_version > 1)
    {

      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.avr);
      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.avg);
      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.avb);
      fscanf(fileptr,"%d\n",&idummy);

      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.emr);
      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.emg);
      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.emb);
      fscanf(fileptr,"%d\n",&idummy);

      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.spr);
      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.spg);
      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.spb);
      fscanf(fileptr,"%d\n",&idummy);

      fscanf(fileptr,"%lg\n",&ay_mopsi_readattr.patch_size);
      fscanf(fileptr,"%lg\n",&ay_mopsi_readattr.elem_size);
      fscanf(fileptr,"%lg\n",&ay_mopsi_readattr.min_size);

      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.zonal);
      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.has_caustics);

    }
  if(ay_mopsi_version > 2)
    {
      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.camera);
      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.reflection);
      fscanf(fileptr,"%d\n",&ay_mopsi_readattr.shadow);
    }

  if(ay_mopsi_version > 4)
    {
      fscanf(fileptr,"%d\n",&idummy);
      if(!idummy)
	o->hide = AY_TRUE;
    }

  /* 
     fscanf(fileptr,"%d\n",&o->colr);
     fscanf(fileptr,"%d\n",&o->colg);
     fscanf(fileptr,"%d\n",&o->colb);
     fscanf(fileptr,"%d\n",&o->cola);

     fscanf(fileptr,"%d\n",&o->opr);
     fscanf(fileptr,"%d\n",&o->opg);
     fscanf(fileptr,"%d\n",&o->opb);
     fscanf(fileptr,"%d\n",&o->opa);

     fscanf(fileptr,"%lg\n",&o->shading_rate);
     fscanf(fileptr,"%d\n",&o->shading_interpolation);

     fscanf(fileptr,"%d\n",&o->cast_shadows);

     fscanf(fileptr,"%d\n",&o->true_displacement);
     fscanf(fileptr,"%d\n",&o->d_bound);
     fscanf(fileptr,"%lg\n",&o->d_bound_val);

     if(ay_mopsi_version > 1)
     {

     fscanf(fileptr,"%d\n",&o->avr);
     fscanf(fileptr,"%d\n",&o->avg);
     fscanf(fileptr,"%d\n",&o->avb);
     fscanf(fileptr,"%d\n",&o->ava);

     fscanf(fileptr,"%d\n",&o->emr);
     fscanf(fileptr,"%d\n",&o->emg);
     fscanf(fileptr,"%d\n",&o->emb);
     fscanf(fileptr,"%d\n",&o->ema);

     fscanf(fileptr,"%d\n",&o->spr);
     fscanf(fileptr,"%d\n",&o->spg);
     fscanf(fileptr,"%d\n",&o->spb);
     fscanf(fileptr,"%d\n",&o->spa);

     fscanf(fileptr,"%lg\n",&o->patch_size);
     fscanf(fileptr,"%lg\n",&o->elem_size);
     fscanf(fileptr,"%lg\n",&o->min_size);

     fscanf(fileptr,"%d\n",&o->zonal);
     fscanf(fileptr,"%d\n",&o->has_caustics);

     }
     if(ay_mopsi_version > 2)
     {
     fscanf(fileptr,"%d\n",&o->camera);
     fscanf(fileptr,"%d\n",&o->reflection);
     fscanf(fileptr,"%d\n",&o->shadow);
     }

     if(ay_mopsi_version > 4)
     {
     fscanf(fileptr,"%d\n",&o->editor);
     }
  */

  return ay_status;
}

int 
ay_mopsi_trafos(FILE *fileptr)
{
 int ay_status = AY_OK;
 ay_object *o = ay_mopsi_lastread;

  if(!o)
    return AY_OK;

  fscanf(fileptr,"%lg\n",&o->movx);
  fscanf(fileptr,"%lg\n",&o->movy);
  fscanf(fileptr,"%lg\n",&o->movz);

  fscanf(fileptr,"%lg\n",&o->rotx);
  fscanf(fileptr,"%lg\n",&o->roty);
  fscanf(fileptr,"%lg\n",&o->rotz);

  fscanf(fileptr,"%lg\n",&o->quat[0]);
  fscanf(fileptr,"%lg\n",&o->quat[1]);
  fscanf(fileptr,"%lg\n",&o->quat[2]);
  fscanf(fileptr,"%lg\n",&o->quat[3]);

  fscanf(fileptr,"%lg\n",&o->scalx);
  fscanf(fileptr,"%lg\n",&o->scaly);
  fscanf(fileptr,"%lg\n",&o->scalz); 

 return ay_status;
}


/* XXXX leaks memory in low-mem situation */
int 
ay_mopsi_shaders(FILE *fileptr)
{
 int ay_status = AY_OK;
 ay_object *o = ay_mopsi_lastread, *o2 = NULL, *ml = ay_root->next;
 ay_object *last = NULL;
 int shcount = 0, argcount = 0, i = 0, j = 0;
 ay_shader *shader = NULL;
 ay_shader_arg *sarg = NULL, **prevargptr = NULL;
 int read;
 int idummy;
 float fdummy;
 ay_light_object *light = NULL;
 int sargtype;
 double p[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
 ay_root_object *root = NULL;
 ay_mat_object *m = NULL;
 char buffer[128], *argv[3];
 
 if(!o)
   return AY_OK;

 fscanf(fileptr,"%d\n",&shcount);

 argv[0] = NULL;
 argv[1] = NULL;
 argv[2] = buffer;

 for(i=0;i<shcount;i++)
   {
     shader = NULL;
     if(!(shader = calloc(1, sizeof(ay_shader))))
       return AY_EOMEM;

     ay_mopsi_string(fileptr,&(shader->name));

     fscanf(fileptr,"%d\n", &(shader->type));
     /*     fscanf(fileptr,"%d\n", &(shader->subtype));*/
     fscanf(fileptr,"%d\n", &idummy);
     fscanf(fileptr,"%d\n", &argcount);

     prevargptr = &(shader->arg);

     for(j=0; j < argcount; j++)
       {
	 if(!(sarg = calloc(1, sizeof(ay_shader_arg))))
	   return AY_EOMEM; 

	 ay_mopsi_string(fileptr, &(sarg->name));
	 fscanf(fileptr,"%d", &(sargtype));
	 read = fgetc(fileptr);
	 switch(sargtype)
	   {
	   case MOPS_SACOLOR:
	     sarg->type = AY_SACOLOR;
	     fscanf(fileptr,"%g\n", &((sarg->val).color[0]));
	     fscanf(fileptr,"%g\n", &((sarg->val).color[1]));
	     fscanf(fileptr,"%g\n", &((sarg->val).color[2]));
	     fscanf(fileptr,"%g\n", &fdummy);
	     break;
	   case MOPS_SAVECTOR:
	     sarg->type = AY_SAVECTOR;
	     fscanf(fileptr,"%g\n", &((sarg->val).point[0]));
	     fscanf(fileptr,"%g\n", &((sarg->val).point[1]));
	     fscanf(fileptr,"%g\n", &((sarg->val).point[2]));
	     break;
	   case MOPS_SAPOINT:
	     sarg->type = AY_SAPOINT;
	     fscanf(fileptr,"%g\n", &((sarg->val).point[0]));
	     fscanf(fileptr,"%g\n", &((sarg->val).point[1]));
	     fscanf(fileptr,"%g\n", &((sarg->val).point[2]));
	     break;
	   case MOPS_SASCALAR:
	     sarg->type = AY_SASCALAR;
	     fscanf(fileptr,"%g\n", &((sarg->val).scalar));
	     break;
	   case MOPS_SASTRING:
	     sarg->type = AY_SASTRING;
	     ay_mopsi_string(fileptr, &((sarg->val).string));
	     if(!((sarg->val).string))
	       {
		 if(!((sarg->val).string = calloc(1, sizeof(char))))
		    return AY_EOMEM;
	       }
	     break;
	   } /* switch */
	  
	 *prevargptr = sarg;
	 prevargptr = &(sarg->next);

	} /* for */

     /*
     ay_status = ay_add_shader(o, shader);
     */

     if(o->type == AY_IDLIGHT && shader->type == AY_STLIGHT)
       {
	 light = (ay_light_object*)o->refine;

	 light->lshader = shader;
	 ay_status = ay_pact_getpoint(1, o, p);
	 shader = NULL;
       }

     if(o->type == AY_IDROOT && shader->type == AY_STVOLUME)
       {
	 root = (ay_root_object *)o->refine;
	 root->atmosphere = shader;
	 shader = NULL;
       }

     if(o->type == AY_IDROOT && shader->type == AY_STIMAGER)
       {
	 root = (ay_root_object *)o->refine;
	 root->imager = shader;
	 shader = NULL;
       }

     if(shader)
       {
	 /* create/link material object */
	 if(!o2)
	   {
	     sprintf(buffer,"mat%d", ay_mopsi_materialnumber);
	     ay_mopsi_materialnumber++;

	     ay_status = ay_object_createargs(AY_IDMATERIAL, 3, argv, &o2);
	     if(ay_status)
	       {
		 /* material already registered */
		 ay_status = AY_OK;
		 o2 = NULL;
		 ay_shader_free(shader);
		 break;
	       }

	     while(ml->next)
	       {
		 last = ml;
		 ml = ml->next;
	       }
	     last->next = o2;
	     o2->next = ml;

	     m = (ay_mat_object *)(o2->refine);

	     /* link last read object to this material */
	     if(o)
	       o->mat = m;

	     o2->refcount++;


	   } /* if */

	 /* attach shader */
	 if(m)
	   {
	     switch(shader->type)
	       {
	       case AY_STSURFACE:
		 m->sshader = shader;
		 break;
	       case AY_STDISPLACEMENT:
		 m->dshader = shader;
		 break;
	       case AY_STINTERIOR:
		 m->ishader = shader;
		 break;
	       case AY_STEXTERIOR:
		 m->eshader = shader;
		 break;
	       default:
		 ay_shader_free(shader);
		 shader = NULL;
		 break;
	       } /* switch */

	   } /* if */

       } /* if */

   } /* for */

 /* copy RiAttributes */
 if(m)
   {
     /* copy currently read RiAttributes to this material */
     m->colr = ay_mopsi_readattr.colr;
     m->colg = ay_mopsi_readattr.colg;
     m->colb = ay_mopsi_readattr.colb;

     m->opr = ay_mopsi_readattr.opr;
     m->opg = ay_mopsi_readattr.opg;
     m->opb = ay_mopsi_readattr.opb;

     m->shading_rate = ay_mopsi_readattr.shading_rate;
     m->shading_interpolation = ay_mopsi_readattr.shading_interpolation;

     m->cast_shadows = ay_mopsi_readattr.cast_shadows;
     m->true_displacement = ay_mopsi_readattr.true_displacement;
     m->dbound = ay_mopsi_readattr.dbound;
     m->dbound_val = ay_mopsi_readattr.dbound_val;

     m->avr = ay_mopsi_readattr.avr;
     m->avg = ay_mopsi_readattr.avg;
     m->avb = ay_mopsi_readattr.avb;
     
     m->emr = ay_mopsi_readattr.emr;
     m->emg = ay_mopsi_readattr.emg;
     m->emb = ay_mopsi_readattr.emb;     

     m->spr = ay_mopsi_readattr.spr;
     m->spg = ay_mopsi_readattr.spg;
     m->spb = ay_mopsi_readattr.spb;

     m->patch_size = ay_mopsi_readattr.patch_size;
     m->elem_size = ay_mopsi_readattr.elem_size;
     m->min_size = ay_mopsi_readattr.min_size;

     m->zonal = ay_mopsi_readattr.zonal;
     m->has_caustics = ay_mopsi_readattr.has_caustics;
     m->camera = ay_mopsi_readattr.camera;
     m->reflection = ay_mopsi_readattr.reflection;
     m->shadow = ay_mopsi_readattr.shadow;

   } /* if */


 return ay_status;
}


int 
ay_mopsi_tags(FILE *fileptr)
{
 int ay_status = AY_OK;
 ay_object *o = ay_mopsi_lastread;
 ay_tag_object *tag = NULL;
 Tcl_HashEntry *entry = NULL;
 int tcount = 0, i = 0;

 if(!o)
   return AY_OK;

 fscanf(fileptr,"%d\n",&tcount);

 for(i=0;i<tcount;i++)
   {
     tag = NULL;
     if(!(tag = calloc(1, sizeof(ay_tag_object))))
       return AY_EOMEM;

     ay_mopsi_string(fileptr,&(tag->name));

     if(!(entry = Tcl_FindHashEntry(&ay_tagtypesht, tag->name)))
       {
	 /* 
	    return AY_HASH;
	    XXXX warn the user?
	 */

       }

     if(entry)
       tag->type = (char *)Tcl_GetHashValue(entry);

     ay_mopsi_string(fileptr,&(tag->val));

     tag->next = o->tags;
     o->tags = tag;
   } /* for */

 return ay_status;
}


int 
ay_mopsi_nurbcurve(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve = NULL;
 int i = 0, a = 0;
 int idummy, has_weights;
 double ddummy;

  if(!(curve = calloc(1, sizeof(ay_nurbcurve_object))))
    { return AY_EOMEM; }

  o->refine = curve;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  fscanf(fileptr,"%d\n",&curve->length);
  fscanf(fileptr,"%d\n",&curve->order);
  fscanf(fileptr,"%d\n",&curve->knot_type);

  if(curve->knot_type == AY_KTCUSTOM)
    {
      if(!(curve->knotv =
	   calloc((curve->length + curve->order), sizeof(double))))
	{ free(curve); return AY_EOMEM; }

      for(i=0; i<(curve->length + curve->order); i++)
	fscanf(fileptr,"%lg\n",&(curve->knotv[i]));
    }
  else
    {
      ay_status = ay_knots_createnc(curve);
      if(ay_status)
	{ free(curve); return ay_status; }
    }

  fscanf(fileptr,"%d\n",&has_weights);

  if(!(curve->controlv = calloc(curve->length*4, sizeof(double))))
    { free(curve->knotv); free(curve); return AY_EOMEM;}

  a = 0;
  for(i=0; i < curve->length; i++)
    {
      fscanf(fileptr,"%lg\n",&(curve->controlv[a]));
      a++;
      fscanf(fileptr,"%lg\n",&(curve->controlv[a]));
      a++;
      fscanf(fileptr,"%lg\n",&(curve->controlv[a]));
      a++;
      if(has_weights)
	{
	  fscanf(fileptr,"%lg\n",&(curve->controlv[a]));
	  a++;
	}
      else
	{
	  curve->controlv[a] = 1.0;
	  a++;
	}
    }

  fscanf(fileptr,"%d\n",&(curve->closed));
  fscanf(fileptr,"%d\n",&idummy);
  fscanf(fileptr,"%lg\n",&(curve->glu_sampling_tolerance));
  if(ay_prefs.mopsiresettolerance)
    curve->glu_sampling_tolerance = 0.0;
  fscanf(fileptr,"%lg\n",&ddummy);

  ay_nct_recreatemp(curve);


 return ay_status;
}


int 
ay_mopsi_nurbpatch(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *patch = NULL;
 int i = 0, j = 0, a = 0;
 int has_weights, idummy;
 double ddummy;

  if(!(patch = calloc(1, sizeof(ay_nurbpatch_object))))
    { return AY_EOMEM; }

  o->refine = patch;
  o->parent = AY_TRUE;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  fscanf(fileptr,"%d\n",&patch->width);
  fscanf(fileptr,"%d\n",&patch->height);
  fscanf(fileptr,"%d\n",&patch->uorder);
  fscanf(fileptr,"%d\n",&patch->vorder);
  fscanf(fileptr,"%d\n",&patch->uknot_type);
  fscanf(fileptr,"%d\n",&patch->vknot_type);
  
  ay_status = ay_knots_createnp(patch);
  if(ay_status)
    { free(patch); return ay_status; }

  if(patch->uknot_type == AY_KTCUSTOM)
    {
      for(i=0; i<(patch->width + patch->uorder); i++)
	fscanf(fileptr,"%lg\n",&(patch->uknotv[i]));
    }

  if(patch->vknot_type == AY_KTCUSTOM)
    {
      for(i=0; i<(patch->height + patch->vorder); i++)
	fscanf(fileptr,"%lg\n",&(patch->vknotv[i]));
    }

  fscanf(fileptr,"%d\n",&has_weights);

  if(!(patch->controlv = calloc(patch->width*patch->height*4,
				sizeof(double))))
    { free(patch->uknotv); free(patch->vknotv); free(patch);
      return AY_EOMEM; }

  a = 0;
  for(i=0; i < patch->width; i++)
    for(j=0; j < patch->height; j++)
      {
	fscanf(fileptr,"%lg\n",&(patch->controlv[a]));
	a++;
	fscanf(fileptr,"%lg\n",&(patch->controlv[a]));
	a++;
	fscanf(fileptr,"%lg\n",&(patch->controlv[a]));
	a++;
	if(has_weights)
	  {
	    fscanf(fileptr,"%lg\n",&(patch->controlv[a]));
	    a++;
	  }
	else
	  {
	    patch->controlv[a] = 1.0;
	    a++;
	  }
      }

  fscanf(fileptr,"%d\n",&idummy);
  fscanf(fileptr,"%d\n",&(patch->glu_display_mode));
  patch->glu_display_mode++;
  if(ay_prefs.mopsiresetdisplaymode)
    patch->glu_display_mode = 0;
  fscanf(fileptr,"%lg\n",&(patch->glu_sampling_tolerance));
  if(ay_prefs.mopsiresettolerance)
    patch->glu_sampling_tolerance = 0.0;
  fscanf(fileptr,"%lg\n",&ddummy);

 return ay_status;
}


int 
ay_mopsi_box(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_box_object *box = NULL;

  if(!(box = calloc(1, sizeof(ay_box_object))))
    { return AY_EOMEM; }

  o->refine = box;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  fscanf(fileptr,"%lg\n",&box->width);
  fscanf(fileptr,"%lg\n",&box->length);
  fscanf(fileptr,"%lg\n",&box->height);

 return ay_status;
}


int 
ay_mopsi_sphere(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_sphere_object *sphere = NULL;
 int itemp;

  if(!(sphere = calloc(1, sizeof(ay_sphere_object))))
    { return AY_EOMEM; }

  o->refine = sphere;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  fscanf(fileptr,"%d\n", &itemp);
  sphere->closed = (char)itemp;
  fscanf(fileptr,"%lg\n",&sphere->radius);
  fscanf(fileptr,"%lg\n",&sphere->zmin);
  fscanf(fileptr,"%lg\n",&sphere->zmax);
  fscanf(fileptr,"%lg\n",&sphere->thetamax);

 return ay_status;
}


int 
ay_mopsi_cone(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_cone_object *cone = NULL;
 int itemp;

  if(!(cone = calloc(1, sizeof(ay_cone_object))))
    { return AY_EOMEM; }

  o->refine = cone;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  fscanf(fileptr,"%d\n", &itemp);
  cone->closed = (char)itemp;
  fscanf(fileptr,"%lg\n",&cone->radius);
  fscanf(fileptr,"%lg\n",&cone->height);
  fscanf(fileptr,"%lg\n",&cone->thetamax);

 return ay_status;
}


int 
ay_mopsi_disk(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_disk_object *disk = NULL;

  if(!(disk = calloc(1, sizeof(ay_disk_object))))
    { return AY_EOMEM; }

  o->refine = disk;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  fscanf(fileptr,"%lg\n",&disk->radius);
  fscanf(fileptr,"%lg\n",&disk->height);
  fscanf(fileptr,"%lg\n",&disk->thetamax);

 return ay_status;
}


int 
ay_mopsi_cylinder(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_cylinder_object *cylinder = NULL;
 int itemp;

  if(!(cylinder = calloc(1, sizeof(ay_cylinder_object))))
    { return AY_EOMEM; }

  o->refine = cylinder;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  fscanf(fileptr,"%d\n", &itemp);
  cylinder->closed = (char)itemp;
  fscanf(fileptr,"%lg\n",&cylinder->radius);
  fscanf(fileptr,"%lg\n",&cylinder->zmin);
  fscanf(fileptr,"%lg\n",&cylinder->zmax);
  fscanf(fileptr,"%lg\n",&cylinder->thetamax);

 return ay_status;
}


int 
ay_mopsi_hyperboloid(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_hyperboloid_object *hyperboloid = NULL;
 int itemp;

  if(!(hyperboloid = calloc(1, sizeof(ay_hyperboloid_object))))
    { return AY_EOMEM; }

  o->refine = hyperboloid;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  fscanf(fileptr,"%d\n", &itemp);
  hyperboloid->closed = (char)itemp;

  fscanf(fileptr,"%lg\n",&hyperboloid->p1[0]);
  fscanf(fileptr,"%lg\n",&hyperboloid->p1[1]);
  fscanf(fileptr,"%lg\n",&hyperboloid->p1[2]);

  fscanf(fileptr,"%lg\n",&hyperboloid->p2[0]);
  fscanf(fileptr,"%lg\n",&hyperboloid->p2[1]);
  fscanf(fileptr,"%lg\n",&hyperboloid->p2[2]);

  fscanf(fileptr,"%lg\n",&hyperboloid->thetamax);

 return ay_status;
}


int 
ay_mopsi_paraboloid(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_paraboloid_object *paraboloid = NULL;
 int itemp;

  if(!(paraboloid = calloc(1, sizeof(ay_paraboloid_object))))
    { return AY_EOMEM; }

  o->refine = paraboloid;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  fscanf(fileptr,"%d\n", &itemp);
  paraboloid->closed = (char)itemp;
  fscanf(fileptr,"%lg\n",&paraboloid->rmax);
  fscanf(fileptr,"%lg\n",&paraboloid->zmin);
  fscanf(fileptr,"%lg\n",&paraboloid->zmax);
  fscanf(fileptr,"%lg\n",&paraboloid->thetamax);

 return ay_status;
}


int 
ay_mopsi_torus(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_torus_object *torus = NULL;
 int itemp;

  if(!(torus = calloc(1, sizeof(ay_torus_object))))
    { return AY_EOMEM; }

  o->refine = torus;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  fscanf(fileptr,"%d\n", &itemp);
  torus->closed = (char)itemp;
  fscanf(fileptr,"%lg\n",&torus->majorrad);
  fscanf(fileptr,"%lg\n",&torus->minorrad);
  fscanf(fileptr,"%lg\n",&torus->phimin);
  fscanf(fileptr,"%lg\n",&torus->phimax);
  fscanf(fileptr,"%lg\n",&torus->thetamax);

 return ay_status;
}


int 
ay_mopsi_instance(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;

  o->refine = NULL;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

 return ay_status;
}


int 
ay_mopsi_light(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_light_object *light = NULL;
 double dummy;
 int idummy, ltype;

  if(!(light = calloc(1, sizeof(ay_light_object))))
    { return AY_EOMEM; }

  /* Mops lights always have a to.z of -10.0! */
  light->tto[2] = -10.0;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &(o->name));

  fscanf(fileptr,"%d\n",&light->shadows);

  if(ay_mopsi_version > 0)
    fscanf(fileptr,"%d\n",&light->samples);

  /* default for very old scenes */
  light->on = AY_TRUE;

  if(ay_mopsi_version > 3)
    {
      fscanf(fileptr,"%d\n",&ltype);

      switch(ltype)
	{
	case MOPS_LITCUSTOM:
	  light->type = AY_LITCUSTOM;
	  break;
	case MOPS_LITPOINT:
	  light->type = AY_LITPOINT;
	  break;
	case MOPS_LITDIST:
	  light->type = AY_LITDISTANT;
	  break;
	case MOPS_LITSPOT:
	  light->type = AY_LITSPOT;
	  break;

	}


      fscanf(fileptr,"%d\n",&light->on);
      fscanf(fileptr,"%lg\n",&light->intensity);
      fscanf(fileptr,"%d\n",&light->colr);
      fscanf(fileptr,"%d\n",&light->colg);
      fscanf(fileptr,"%d\n",&light->colb);
      fscanf(fileptr,"%d\n",&idummy);
      if(ay_mopsi_version < 5)
	{
	  fscanf(fileptr,"%lg\n",&dummy);
	  fscanf(fileptr,"%lg\n",&dummy);
	  fscanf(fileptr,"%lg\n",&dummy);
	  fscanf(fileptr,"%lg\n",&dummy);
	  fscanf(fileptr,"%lg\n",&dummy);
	  fscanf(fileptr,"%lg\n",&dummy);
	}
      fscanf(fileptr,"%lg\n",&light->cone_angle);
      fscanf(fileptr,"%lg\n",&light->cone_delta_angle);
      fscanf(fileptr,"%lg\n",&light->beam_distribution);

    }

  o->refine = light;
  o->parent = AY_TRUE;
  o->inherit_trafos = AY_TRUE;

 return ay_status;
}

int 
ay_mopsi_custom(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 char *customname = NULL;
 Tcl_HashEntry *entry;
 ay_mopsicb *customreadfunc = NULL;
 char *name = NULL;

  /* get name of object */
  if(ay_mopsi_version > 0)
    ay_mopsi_string(fileptr, &name);
  o->name = name;

  ay_mopsi_string(fileptr, &customname);

  /* get callback from name */
  entry = Tcl_FindHashEntry(&ay_mopsiht, customname);
  if (entry != NULL)
    {
      customreadfunc = (ay_mopsicb *) Tcl_GetHashValue(entry);
    }
  else
    {
      free(customname);
      return AY_ERROR;
    }

  if(customreadfunc)
    ay_status = customreadfunc(fileptr, o);

  if(ay_status)
    {
      free(customname);
      return ay_status;
    }


  free(customname);

 return ay_status;
}


int
ay_mopsi_revolve(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_revolve_object *r = NULL;

  if(!(r = calloc(1, sizeof(ay_revolve_object))))
    { return AY_EOMEM; }

  o->refine = r;
  o->type = AY_IDREVOLVE;
  o->parent = AY_TRUE;

  
  fscanf(fileptr,"%d\n",&r->has_upper_cap);
  fscanf(fileptr,"%d\n",&r->has_lower_cap);
  r->thetamax = 360.0;
  r->glu_sampling_tolerance = 30.0;

  if(ay_mopsi_version > 4)
    {
      fscanf(fileptr,"%lg\n",&r->thetamax);
    }

  if(ay_mopsi_version > 5)
    {
      fscanf(fileptr,"%lg\n",&r->glu_sampling_tolerance);
      if(ay_prefs.mopsiresettolerance)
	r->glu_sampling_tolerance = 0.0;
      fscanf(fileptr,"%d\n",&r->glu_display_mode);
      if(ay_prefs.mopsiresetdisplaymode)
	r->glu_display_mode = 0;
    }

 return ay_status;
}

int
ay_mopsi_extrude(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_extrude_object *ext = NULL;

  if(!(ext = calloc(1, sizeof(ay_extrude_object))))
    { return AY_EOMEM; }

  o->refine = ext;
  o->type = AY_IDEXTRUDE;
  o->parent = AY_TRUE;

  ext->height = 1.0;
  ext->glu_sampling_tolerance = 30.0;
  ext->bevel_radius = 0.1;

  fscanf(fileptr,"%d\n",&ext->has_upper_cap);
  fscanf(fileptr,"%d\n",&ext->has_lower_cap);

  if(ay_mopsi_version > 4)
    {
      fscanf(fileptr,"%lg\n",&ext->height);
      fscanf(fileptr,"%d\n",&ext->has_upper_bevels);
      fscanf(fileptr,"%d\n",&ext->has_lower_bevels);
      fscanf(fileptr,"%lg\n",&ext->bevel_radius);
      fscanf(fileptr,"%d\n",&ext->bevel_type);
      fscanf(fileptr,"%lg\n",&ext->glu_sampling_tolerance);
      if(ay_prefs.mopsiresettolerance)
	ext->glu_sampling_tolerance = 0.0;
      fscanf(fileptr,"%d\n",&ext->glu_display_mode);
      if(ay_prefs.mopsiresetdisplaymode)
	ext->glu_display_mode = 0;
    }


 return ay_status;
}


int
ay_mopsi_sweep(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_sweep_object *swp = NULL;

  if(!(swp = calloc(1, sizeof(ay_sweep_object))))
    { return AY_EOMEM; }

  o->refine = swp;
  o->type = AY_IDSWEEP;
  o->parent = AY_TRUE;

  fscanf(fileptr,"%d\n",&swp->sections);
  fscanf(fileptr,"%d\n",&swp->rotate);

  if(ay_mopsi_version > 5)
    {
      fscanf(fileptr,"%lg\n",&swp->glu_sampling_tolerance);
      if(ay_prefs.mopsiresettolerance)
	swp->glu_sampling_tolerance = 0.0;
      fscanf(fileptr,"%d\n",&swp->glu_display_mode);
      if(ay_prefs.mopsiresetdisplaymode)
	swp->glu_display_mode = 0;
    }

  if(ay_mopsi_version > 7)
    {
      fscanf(fileptr,"%d\n",&swp->interpolate);
    }

 return ay_status;
}


int
ay_mopsi_skin(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_skin_object *sk = NULL;

  if(!(sk = calloc(1, sizeof(ay_skin_object))))
    { return AY_EOMEM; }

  o->refine = sk;
  o->type = AY_IDSKIN;
  o->parent = AY_TRUE;

  fscanf(fileptr,"%d\n",&sk->interpolate);
  fscanf(fileptr,"%d\n",&sk->uorder);
  fscanf(fileptr,"%d\n",&sk->uknot_type);
  fscanf(fileptr,"%lg\n",&sk->glu_sampling_tolerance);
  if(ay_prefs.mopsiresettolerance)
    sk->glu_sampling_tolerance = 0.0;
  fscanf(fileptr,"%d\n",&sk->glu_display_mode);
  if(ay_prefs.mopsiresetdisplaymode)
    sk->glu_display_mode = 0;

 return ay_status;
}

int
ay_mopsi_icurve(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_icurve_object *ic = NULL;
 double *controlv;
 int dlen, a, i;

  if(!(ic = calloc(1, sizeof(ay_icurve_object))))
    { return AY_EOMEM; }

  o->refine = ic;
  o->type = AY_IDICURVE;
  o->parent = AY_TRUE;

  fscanf(fileptr,"%lg\n",&ic->glu_sampling_tolerance);
  if(ay_prefs.mopsiresettolerance)
    ic->glu_sampling_tolerance = 0.0;

  fscanf(fileptr,"%lg\n",&ic->iparam);
  fscanf(fileptr,"%d\n",&ic->closed);
  if(ay_mopsi_version < 7)
    {
      fscanf(fileptr,"%d\n",&dlen);

      if(!(controlv = calloc(dlen*3, sizeof(double))))
	return AY_EOMEM;
      
      a = 0;
      for(i = 0; i < dlen; i++)
	{
	  fscanf(fileptr,"%lg\n",&controlv[a]);
	  fscanf(fileptr,"%lg\n",&controlv[a+1]);
	  fscanf(fileptr,"%lg\n",&controlv[a+2]);
	  a+=3;
	}

      ic->controlv = controlv;
      ic->length = dlen;
    }
  else
    {
      if(!(controlv = calloc(4*3, sizeof(double))))
	return AY_EOMEM;
      a = 0;
      for(i = 0; i < 4; i++)
	{
	  controlv[a] = i;
	  a+=3;
	}
      ic->controlv = controlv;
      ic->length = 4;
    }
  if(ay_mopsi_version > 5)
    {
      fscanf(fileptr,"%d\n",&ic->imode);
      fscanf(fileptr,"%d\n",&ic->iorder);
    }

 return ay_status;
}


int 
ay_mopsi_object(FILE *fileptr)
{
 int ay_status = AY_OK;
 int type = 0;
 ay_object *o = NULL;

 if(feof(fileptr))
   return AY_EEOF;

 /* skip to next entity */
 ay_status = ay_mopsi_skip(fileptr);

 if(ay_status)
   return ay_status;

 if(!(o = calloc(1,sizeof(ay_object))))
   return AY_EOMEM;

 ay_object_defaults(o);

 /* get type of object */
  if(ay_mopsi_version > 0)
    {
      fscanf(fileptr,"%d",&type);
      fgetc(fileptr);
    }
  else
    {
      fscanf(fileptr,"%d\n",&type);
    }

  /*  o->type = type;*/

 switch(type)
   {
   case MOPS_OTLEVEL:
     o->type = AY_IDLEVEL;
     ay_status = ay_mopsi_level(fileptr, &o);
     break;
   case MOPS_OTNURBCURVE:
     o->type = AY_IDNCURVE;
     ay_status = ay_mopsi_nurbcurve(fileptr, o);
     break;
   case MOPS_OTNURBPATCH:
     o->type = AY_IDNPATCH;
     ay_status = ay_mopsi_nurbpatch(fileptr, o);
     break;
   case MOPS_OTCUSTOM:
     ay_status = ay_mopsi_custom(fileptr, o);
     break;
   case MOPS_OTLIGHT:
     o->type = AY_IDLIGHT;
     ay_status = ay_mopsi_light(fileptr, o);
     break;
   case MOPS_OTBOX:
     o->type = AY_IDBOX;
     ay_status = ay_mopsi_box(fileptr, o);
     break;
   case MOPS_OTSPHERE:
     o->type = AY_IDSPHERE;
     ay_status = ay_mopsi_sphere(fileptr, o);
     break;
   case MOPS_OTCONE:
     o->type = AY_IDCONE;
     ay_status = ay_mopsi_cone(fileptr, o);
     break;
   case MOPS_OTCYLINDER:
     o->type = AY_IDCYLINDER;
     ay_status = ay_mopsi_cylinder(fileptr, o);
     break;
   case MOPS_OTDISK:
     o->type = AY_IDDISK;
     ay_status = ay_mopsi_disk(fileptr, o);
     break;
   case MOPS_OTTORUS:
     o->type = AY_IDTORUS;
     ay_status = ay_mopsi_torus(fileptr, o);
     break;
   case MOPS_OTPARABOLOID:
     o->type = AY_IDPARABOBOLOID;
     ay_status = ay_mopsi_paraboloid(fileptr, o);
     break;
   case MOPS_OTHYPERBOLOID:
     o->type = AY_IDHYPERBOLOID;
     ay_status = ay_mopsi_hyperboloid(fileptr, o);
     break;
   case MOPS_OTINSTANCE:
     o->type = AY_IDINSTANCE;
     ay_status = ay_mopsi_instance(fileptr, o);
     break;
   default:
     break;
   }

  if(ay_status)
   {
     free(o); o=NULL;
     return ay_status;
   }
 
 switch(type)
   {
   case MOPS_OTATTRIB:
     free(o); o=NULL;
     ay_status = ay_mopsi_attributes(fileptr);
     break;
   case MOPS_OTTRAFO:
     free(o); o=NULL;
     ay_status = ay_mopsi_trafos(fileptr);
     break;
   case MOPS_OTSHADER:
     free(o); o=NULL;
     ay_status = ay_mopsi_shaders(fileptr);
     break;
   case MOPS_OTTAG:
     free(o); o=NULL;
     ay_status = ay_mopsi_tags(fileptr);
     break;
   default:
     break;
   }

 if(o)
   {
     ay_status = ay_object_link(o);
     ay_mopsi_lastread = o;
   }

 return ay_status;
}


int 
ay_mopsi_scene(Tcl_Interp *interp, char *filename, int insert)
{
 int ay_status = AY_OK;
 FILE *fileptr = NULL;
 char *fname = "read_scene";
 ay_object *o = NULL;

 if(!(fileptr = fopen(filename,"rb")))
   return AY_EOPENFILE;

 ay_status = ay_mopsi_header(fileptr);

 if(ay_status)
   { fclose(fileptr); return ay_status; }

 ay_status = ay_mopsi_root(interp, fileptr, insert);
 if(feof(fileptr))
   {fclose(fileptr); return AY_OK;}

 if(ay_status)
   {fclose(fileptr); return ay_status;}

 while(!ay_status)
   {
     ay_status = ay_mopsi_object(fileptr);
     if(ay_status)
       if(ay_status != AY_EEOF)
	 {
	   /* issue errmsg */
	   ay_error(ay_status, fname, NULL);
	   if(ay_prefs.onerror)
	     {
	       ay_status = AY_OK;
	     }
	 }

   }

 if(ay_status == AY_EEOF)
   ay_status = AY_OK;

  /* link instance objects to their originals */
  /* clear hashtable of oid ptrs */
  ay_instt_createoidht(NULL);
  /* build it */
  ay_instt_createoidht(ay_root->next);
  /* link instance objects to their originals */
  ay_instt_connect(ay_root->next, &(ay_root->next));
  /* force rebuild of all custom objects, that rely on children */
  o = ay_root->next;
  while(o)
    {
      ay_notify_force(o);
      o = o->next;
    }

 return ay_status;
}


int
ay_mopsi_tcmd(ClientData clientData, Tcl_Interp *interp,
	      int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "import_mops";
 char *n1="mopsi_options";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj("ResetDM", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.mopsiresetdisplaymode);

  Tcl_SetStringObj(ton, "ResetST", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.mopsiresettolerance);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_mopsi_lastread = NULL;
  ay_mopsi_materialnumber = 0;

  ay_status = ay_mopsi_scene(interp, argv[1], AY_TRUE);

  if(ay_status)
    ay_error(ay_status, fname, NULL);

 return TCL_OK;
}


int
ay_mopsi_register(char *tname, ay_mopsicb *cb)
{
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;


  entry = Tcl_CreateHashEntry(&ay_mopsiht, tname, &new_item);
  Tcl_SetHashValue(entry, &(ay_mopsi_revolve));


 return AY_OK;
}


int
ay_mopsi_init()
{
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;

  Tcl_InitHashTable(&ay_mopsiht, TCL_STRING_KEYS);

  entry = Tcl_CreateHashEntry(&ay_mopsiht, "Revolve", &new_item);
  Tcl_SetHashValue(entry, &(ay_mopsi_revolve));

  entry = Tcl_CreateHashEntry(&ay_mopsiht, "Extrude", &new_item);
  Tcl_SetHashValue(entry, &(ay_mopsi_extrude));

  entry = Tcl_CreateHashEntry(&ay_mopsiht, "Sweep", &new_item);
  Tcl_SetHashValue(entry, &(ay_mopsi_sweep));

  entry = Tcl_CreateHashEntry(&ay_mopsiht, "Skin", &new_item);
  Tcl_SetHashValue(entry, &(ay_mopsi_skin));

  entry = Tcl_CreateHashEntry(&ay_mopsiht, "ICurve", &new_item);
  Tcl_SetHashValue(entry, &(ay_mopsi_icurve));


 return AY_OK;
}
