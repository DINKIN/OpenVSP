

//==== Init Is Called Once During Each Custom Geom Construction  ============================//
//==== Avoid Global Variables Unless You Want Shared With All Custom Geoms of This Type =====//
void Init()
{
	//==== Define Parameters  =====//
	string height = AddParm( PARM_DOUBLE_TYPE, "Height", "Design" );
	SetParmValLimits( height, 6.0, 0.001, 1.0e12 );

	string hip_angle = AddParm( PARM_DOUBLE_TYPE, "HipAngle", "Design" );
	SetParmValLimits( hip_angle, 0.0, 0, 150 );

	string knee_angle = AddParm( PARM_DOUBLE_TYPE, "KneeAngle", "Design" );
	SetParmValLimits( knee_angle, 0.0, -150, 0 );

	string shoulder_angle = AddParm( PARM_DOUBLE_TYPE, "ShoulderAngle", "Design" );
	SetParmValLimits( shoulder_angle, 0.0, 0, 180 );

	string elbow_angle = AddParm( PARM_DOUBLE_TYPE, "ElbowAngle", "Design" );
	SetParmValLimits( elbow_angle, 0.0, 0, 180 );

	string presets  = AddParm( PARM_INT_TYPE, "Presets", "Design" );
	SetParmValLimits( presets, 0, 0, 100 );

	string preset_trigger = AddParm( PARM_BOOL_TYPE, "PresetTrigger", "Design" );
	SetParmVal( preset_trigger, 0.0 );

	//==== Set Some Decent Tess Vals ====//
	string geom_id = GetCurrCustomGeom();
	SetParmVal( GetParm( geom_id, "Tess_U",  "Shape" ), 9 );
	SetParmVal( GetParm( geom_id, "Tess_W",  "Shape" ), 10 );
}


//==== InitGui Is Called Once During Each Custom Geom Construction ====//
void InitGui()
{
	AddGui( GDEV_TAB, "Design"  );
	AddGui( GDEV_YGAP  );
	AddGui( GDEV_DIVIDER_BOX, "Design" );

    //==== Store the GUI Index to Use In UpdateGui ====//
	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Height", "Height", "Design"  );

 	AddGui( GDEV_YGAP  );
    AddGui( GDEV_DIVIDER_BOX, "Angles" );

	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Hip", "HipAngle", "Design"  );
	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Knee", "KneeAngle", "Design"  );
	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Shoulder", "ShoulderAngle", "Design"  );
	AddGui( GDEV_SLIDER_ADJ_RANGE_INPUT, "Elbow", "ElbowAngle", "Design"  );

 	AddGui( GDEV_YGAP  );
    AddGui( GDEV_DIVIDER_BOX, "Presets" );

    //===== Put Gui Devices On Same Line - Fill Line And Scale Rel To Starting Widths ====//
    AddGui( GDEV_BEGIN_SAME_LINE );
	AddGui( GDEV_CHOICE, "Presets:", "Presets", "Design" );
    AddGui( GDEV_ADD_CHOICE_ITEM, "Stand" );
    AddGui( GDEV_ADD_CHOICE_ITEM, "Sit" );
    AddGui( GDEV_ADD_CHOICE_ITEM, "Crouch" );
    AddGui( GDEV_ADD_CHOICE_ITEM, "Stretch" );
    AddGui( GDEV_ADD_CHOICE_ITEM, "Drive" );
    AddGui( GDEV_FORCE_WIDTH, "30" );           // Force the Button Small
    AddGui( GDEV_TRIGGER_BUTTON, "Go", "PresetTrigger", "Design" );
    AddGui( GDEV_END_SAME_LINE );

} 

//==== UpdateGui Is Called Every Time The Gui is Updated - Use It To Deactivate/Show/Hide Gui ====//
void UpdateGui()
{

} 

//==== UpdateSurf Is Called Every Time The Geom is Updated ====//
void UpdateSurf()
{
	string geom_id = GetCurrCustomGeom();

	//==== Check For Preset Trigger Before Getting Curr Vals ====//
    string preset_trigger_parm = GetParm( geom_id, "PresetTrigger", "Design" );
	if ( GetBoolParmVal( preset_trigger_parm )  )
    {
        int preset_choice = GetIntParmVal( GetParm( geom_id, "Presets", "Design" ) );

        //==== Check Choice Val and Set Number of Boxes =====//
        if ( preset_choice == 0 )       // Stand
        {
            SetParmVal( GetParm( geom_id, "HipAngle",  "Design" ), 0.0 );
            SetParmVal( GetParm( geom_id, "KneeAngle",  "Design" ), 0.0 );
            SetParmVal( GetParm( geom_id, "ShoulderAngle",  "Design" ), 0.0 );
            SetParmVal( GetParm( geom_id, "ElbowAngle",  "Design" ), 0.0 );
        }
        else if ( preset_choice == 1 )  //Sit
        {
            SetParmVal( GetParm( geom_id, "HipAngle",  "Design" ), 90.0 );
            SetParmVal( GetParm( geom_id, "KneeAngle",  "Design" ), -90.0 );
            SetParmVal( GetParm( geom_id, "ShoulderAngle",  "Design" ), 0.0 );
            SetParmVal( GetParm( geom_id, "ElbowAngle",  "Design" ), 10.0 );
        }
        else if ( preset_choice == 2 )  //Crouch
        {
            SetParmVal( GetParm( geom_id, "HipAngle",  "Design" ), 45.0 );
            SetParmVal( GetParm( geom_id, "KneeAngle",  "Design" ), -90.0 );
            SetParmVal( GetParm( geom_id, "ShoulderAngle",  "Design" ), 0.0 );
            SetParmVal( GetParm( geom_id, "ElbowAngle",  "Design" ), 20.0 );
        }
        else if ( preset_choice == 3 )  //Stretch
        {
            SetParmVal( GetParm( geom_id, "HipAngle",  "Design" ), 0.0 );
            SetParmVal( GetParm( geom_id, "KneeAngle",  "Design" ), 0.0 );
            SetParmVal( GetParm( geom_id, "ShoulderAngle",  "Design" ), 180.0 );
            SetParmVal( GetParm( geom_id, "ElbowAngle",  "Design" ), 0.0 );
        }
        else if ( preset_choice == 4 )  //Drive
        {
            SetParmVal( GetParm( geom_id, "HipAngle",  "Design" ), 90.0 );
            SetParmVal( GetParm( geom_id, "KneeAngle",  "Design" ), -90.0 );
            SetParmVal( GetParm( geom_id, "ShoulderAngle",  "Design" ), 45.0 );
            SetParmVal( GetParm( geom_id, "ElbowAngle",  "Design" ), 45.0 );
        }

        //==== Reset Trigger ====//
        SetParmVal( preset_trigger_parm, 0 );          // Reset Tigger
    }
    
    //==== Get Curr Vals ====//
	double height_val = GetParmVal( GetParm( geom_id, "Height", "Design" ) );
	double hip_angle_val  = GetParmVal( GetParm( geom_id, "HipAngle",  "Design" ) );
	double knee_angle_val  = GetParmVal( GetParm( geom_id, "KneeAngle",  "Design" ) );
	double shoulder_angle_val  = GetParmVal( GetParm( geom_id, "ShoulderAngle",  "Design" ) );
	double elbow_angle_val  = GetParmVal( GetParm( geom_id, "ElbowAngle",  "Design" ) );



    double hd = height_val/7.0;

    //==== Clear Old Surfs ====//
    ClearXSecSurfs();

    //==== Build Head ====//
    string head = AddXSecSurf();
    BuildPod( head, hd, hd, .1, .8 );

    //==== Build Torso ====//
    string torso = AddXSecSurf();
    BuildPod( torso, 3.0*hd, 1.5*hd, 0.01, 1.2 );
    Matrix4d torso_mat;
    torso_mat.translatef( 1.2*hd, 0, 0 );
    SetXSecSurfGlobalXForm( torso, torso_mat );

    //==== Build Left Arm ====//
    string upper_left_arm = AddXSecSurf();
    BuildPod( upper_left_arm, 1.5*hd, 0.5*hd, 0.05, 1.2 );
    Matrix4d left_arm_mat = torso_mat;
    left_arm_mat.translatef( 0, 1.0*hd, 0 );
    left_arm_mat.rotateY( shoulder_angle_val );
    SetXSecSurfGlobalXForm( upper_left_arm, left_arm_mat );

    string lower_left_arm = AddXSecSurf();
    BuildPod( lower_left_arm, 1.5*hd, 0.35*hd, 0.05, 1.2 );
    left_arm_mat.translatef( 1.5*hd, 0.0, 0 );
    left_arm_mat.rotateY( elbow_angle_val );
    SetXSecSurfGlobalXForm( lower_left_arm, left_arm_mat );

    //==== Build Right Arm ====//
    string upper_right_arm = AddXSecSurf();
    BuildPod( upper_right_arm, 1.5*hd, 0.5*hd, 0.05, 1.2 );
    Matrix4d right_arm_mat = torso_mat;
    right_arm_mat.translatef( 0, -1.0*hd, 0 );
    right_arm_mat.rotateY( shoulder_angle_val );
    SetXSecSurfGlobalXForm( upper_right_arm, right_arm_mat );

    string lower_right_arm = AddXSecSurf();
    BuildPod( lower_right_arm, 1.5*hd, 0.35*hd, 0.05, 1.2 );
    right_arm_mat.translatef(  1.5*hd, 0.0, 0 );
    right_arm_mat.rotateY( elbow_angle_val );
    SetXSecSurfGlobalXForm( lower_right_arm, right_arm_mat );

    //==== Build Left Leg ====//
    string upper_left_leg = AddXSecSurf();
    BuildPod( upper_left_leg, 1.5*hd, 0.6*hd, 0.05, 1.2 );
    Matrix4d left_leg_mat = torso_mat;
    left_leg_mat.translatef( 3.0*hd, 0.4*hd, 0 );
    left_leg_mat.rotateY( hip_angle_val );
    SetXSecSurfGlobalXForm( upper_left_leg, left_leg_mat );

    string lower_left_leg = AddXSecSurf();
    BuildPod( lower_left_leg, 1.5*hd, 0.5*hd, 0.05, 1.2 );
    left_leg_mat.translatef( 1.5*hd , 0.0, 0 );
    left_leg_mat.rotateY( knee_angle_val );
    SetXSecSurfGlobalXForm( lower_left_leg, left_leg_mat );

    //==== Build Left Leg ====//
    string upper_right_leg = AddXSecSurf();
    BuildPod( upper_right_leg, 1.5*hd, 0.6*hd, 0.05, 1.2 );
    Matrix4d right_leg_mat = torso_mat;
    right_leg_mat.translatef( 3.0*hd, -0.4*hd, 0 );
    right_leg_mat.rotateY( hip_angle_val );
    SetXSecSurfGlobalXForm( upper_right_leg, right_leg_mat );

    string lower_right_leg = AddXSecSurf();
    BuildPod( lower_right_leg, 1.5*hd, 0.5*hd, 0.05, 1.2 );
    right_leg_mat.translatef( 1.5*hd , 0.0, 0 );
    right_leg_mat.rotateY( knee_angle_val );
    SetXSecSurfGlobalXForm( lower_right_leg, right_leg_mat );

    SkinXSecSurf();

}

void BuildPod( string surf_id, double length, double diameter, double bluntness, double end_taper )
{
    //==== Add Cross-Sectopms ====//
	string xsec0 = AppendXSec( surf_id, XS_POINT);
	string xsec1 = AppendXSec( surf_id, XS_CIRCLE);
	string xsec2 = AppendXSec( surf_id, XS_CIRCLE);
	string xsec3 = AppendXSec( surf_id, XS_CIRCLE);
	string xsec4 = AppendXSec( surf_id, XS_POINT);


	//==== Define The Middle XSecs ====//
    double end_dia = end_taper*diameter;
    double center_dia = diameter;
    if ( end_taper > 1.0 )
    {
        end_dia = diameter;
        center_dia = (2.0 - end_taper)*diameter;
    }

	SetParmVal( GetXSecParm( xsec1, "Circle_Diameter" ), end_dia );
	SetCustomXSecLoc( xsec1, vec3d( length*bluntness, 0, 0 ) );
	SetParmVal( GetXSecParm( xsec2, "Circle_Diameter" ), center_dia );
	SetCustomXSecLoc( xsec2, vec3d( length/2, 0, 0 ) );
	SetParmVal( GetXSecParm( xsec3, "Circle_Diameter" ), end_dia );
	SetCustomXSecLoc( xsec3, vec3d( length*(1.0-bluntness), 0, 0 ) );

 	//==== Define The Last XSec Placement ====//
	SetCustomXSecLoc( xsec4, vec3d( length, 0, 0 ) );
   
}