# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2014 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# myicons.tcl - load user defined icons

# Ayam uses the following icons (the list may also be obtained using the
# Tcl command "image names")
#ay_Disk_img ayam-splash ay_view2_img ay_Torus_img ay_Paraboloid_img ay_EditW_img ay_Insert_img ay_Skin_img ay_MMGlob_img ay_FindU_img ay_Scale2DA_img ay_Scale1DYA_img emptyimg ay_Gordon_img ay_NPExtrNC_img ay_Stretch2D_img ay_Stretch2DA_img ay_NRect_img ay_Split_img ay_Scale1DX_img ay_Triangle_img ay_Material_img ay_Scale1DZ_img ay_RotateA_img ay_Level_img ay_Show_img ay_TriangleL_img ay_Mark_img ay_View_img ay_FindUV_img ay_Cap_img ay_Grid025_img ay_Edit_img ay_Grid01_img ay_Scale3D_img ay_TriangleR_img ay_MoveY_img ay_Light_img ay_Concat_img ay_Refine_img ay_Grid05_img ay_EditN_img ay_Sweep_img ay_Birail2_img ay_Cone_img ay_Clamp_img ay_Scale1DXA_img ay_Undo_img ay_Pick_img ay_Grid10_img ay_Move_img ay_NPRevU_img ay_ResetW_img ay_Delete_img ay_view1_img ay_Instance_img ay_view3_img ay_Grid_img ay_MMLocLev_img ay_DMDraw_img ay_Box_img ay_Notify_img ay_NCurve_img ay_Revolve_img ay_DMShadeDraw_img ay_Tag_img ay_Cr_img ay_Boxt_img ay_Scale2D_img ay_Scale1DY_img ay_Rotate_img ay_Scale1DZA_img ay_Hyperboloid_img ay_MMLocObj_img ay_ZoomV_img ay_MoveV_img ay_Resizehandle_img ay_DMShade_img ay_GridX_img ay_MoveX_img ay_Scale3DA_img ay_Revert_img ay_MoveZ_img emptybm ay_Extrude_img ay_Cylinder_img ay_RotV_img ay_Birail1_img ay_NPSwapUV_img ay_ICurve_img ay_NPatch_img ay_MoveVZ_img ay_Empty_img ay_Sphere_img ay_NCircle_img ay_NPRevV_img ay_Convert_img

# provided image files should be 25x25 pixel GIF
# i.e. to replace the Disk icon, create and place the file
# "ay_Disk_img.gif" into the "icons" directory (relative
# to the Ayam executable

# set path relative to the Ayam executable
set iconDir icons

# Action icon variants (e.g. scale about) can be created
# automatically by this script by changing the following
# variable to 1
set createVariants 0

set oldcd [pwd]

cd $iconDir
set imgfiles [glob *.gif]

# load all image files
foreach imgfile $imgfiles {
    set imgname [file rootname [file tail $imgfile]]
    image create photo $imgname -file $imgfile
}

# create more icons as variations of basic icons above?
if { $createVariants == 1 } {

# RotateA
image create photo ay_RotateA_img -w 25 -h 25
ay_RotateA_img copy ay_Rotate_img
ay_RotateA_img copy ay_Cr_img -to 9 9

# Scale1DXA
image create photo ay_Scale1DXA_img -w 25 -h 25
ay_Scale1DXA_img copy ay_Scale1DX_img
ay_Scale1DXA_img copy ay_Cr_img -to 1 18

# Scale1DYA
image create photo ay_Scale1DYA_img -w 25 -h 25
ay_Scale1DYA_img copy ay_Scale1DY_img
ay_Scale1DYA_img copy ay_Cr_img -to 1 18

# Scale1DZA
image create photo ay_Scale1DZA_img -w 25 -h 25
ay_Scale1DZA_img copy ay_Scale1DZ_img
ay_Scale1DZA_img copy ay_Cr_img -to 18 18

# Scale2DA
image create photo ay_Scale2DA_img -w 25 -h 25
ay_Scale2DA_img copy ay_Scale2D_img
ay_Scale2DA_img copy ay_Cr_img -to 1 18

# Scale3DA
image create photo ay_Scale3DA_img -w 25 -h 25
ay_Scale3DA_img copy ay_Scale3D_img
ay_Scale3DA_img copy ay_Cr_img -to 18 1

# StretchA
image create photo ay_Stretch2DA_img -w 25 -h 25
ay_Stretch2DA_img copy ay_Stretch2D_img
#ay_Stretch2DA_img copy ay_Cr_img -to 12 9
ay_Stretch2DA_img copy ay_Cr_img -to 1 18

# MoveX
image create photo ay_MoveX_img -w 25 -h 25
ay_MoveX_img copy ay_Move_img
ay_MoveX_img copy ay_Scale1DX_img -from 0 0 7 7

# MoveY
image create photo ay_MoveY_img -w 25 -h 25
ay_MoveY_img copy ay_Move_img
ay_MoveY_img copy ay_Scale1DY_img -from 0 0 7 8

# MoveZ
image create photo ay_MoveZ_img -w 25 -h 25
ay_MoveZ_img copy ay_Move_img
ay_MoveZ_img copy ay_Scale1DZ_img -from 0 0 7 8

# MoveVZ
image create photo ay_MoveVZ_img -w 25 -h 25
ay_MoveVZ_img copy ay_MoveV_img
ay_MoveVZ_img copy ay_Scale1DZ_img -from 0 0 7 8 -to 17 0

}

cd $oldcd
