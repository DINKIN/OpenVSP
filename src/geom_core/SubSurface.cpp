//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//////////////////////////////////////////////////////////////////////
// SubSurface.cpp
// Alex Gary
//////////////////////////////////////////////////////////////////////

#include "SubSurface.h"
#include "Geom.h"
#include "Vehicle.h"
#include "VehicleMgr.h"
#include "VspSurf.h"
#include "ParmMgr.h"
#include "Util.h"
#include "Matrix.h"
#include "Defines.h"

SubSurface::SubSurface( string compID, int type )
{
    m_Type = type;
    m_CompID = compID;
    m_Tag = 0;
    m_UpdateDrawFlag = true;
    m_LineColor = vec3d( 0, 0, 0 );
    m_PolyPntsReadyFlag = false;
    m_FirstSplit = true;
    m_PolyFlag = true;
}

SubSurface::~SubSurface()
{
}

void SubSurface::ParmChanged( Parm* parm_ptr, int type )
{
    Update();

    Vehicle* veh = VehicleMgr::getInstance().GetVehicle();
    if ( veh )
    {
        veh->ParmChanged( parm_ptr, type );
    }
}

void SubSurface::LoadDrawObjs( std::vector< DrawObj* > & draw_obj_vec )
{
    for ( int i = 0 ; i < ( int )m_DrawObjVec.size() ; i++ )
    {
        m_DrawObjVec[i].m_LineColor = m_LineColor;
        m_DrawObjVec[i].m_GeomID = ( m_ID + to_string( ( long long )i ) );
        draw_obj_vec.push_back( &m_DrawObjVec[i] );
    }
}

vector< TMesh* > SubSurface::CreateTMeshVec()
{
    vector<TMesh*> tmesh_vec;
    tmesh_vec.resize( m_LVec.size() );
    for ( int ls = 0 ; ls < ( int ) m_LVec.size() ; ls++ )
    {
        tmesh_vec[ls] = m_LVec[ls].CreateTMesh();
    }

    return tmesh_vec;
}

void SubSurface::UpdateDrawObjs()
{
    if ( !m_UpdateDrawFlag )
    {
        return;
    }

    Vehicle* veh = VehicleMgr::getInstance().GetVehicle();
    if ( !veh ) return;
    Geom* geom = veh->FindGeom( m_CompID );
    m_DrawObjVec.clear();
    m_DrawObjVec.resize( m_LVec.size(), DrawObj() );
    if ( geom )
    {
        for ( int ls = 0 ; ls < ( int )m_LVec.size() ; ls++ )
        {
            int num_pnts = CompNumDrawPnts( geom );
            int *num_pnts_ptr = NULL;
            if ( num_pnts > 0 )
            {
                num_pnts_ptr = &num_pnts;
            }
            m_LVec[ls].UpdateDrawObj( geom, m_DrawObjVec[ls], num_pnts_ptr );
        }
    }
}

void SubSurface::Update()
{
    m_PolyPntsReadyFlag = false;
    UpdateDrawObjs();
}

std::string SubSurface::GetTypeName( int type )
{
    if ( type == SubSurface::SS_LINE )
    {
        return string( "Line" );
    }
    if ( type == SubSurface::SS_RECTANGLE )
    {
        return string( "Rectangle" );
    }
    if ( type == SubSurface::SS_ELLIPSE )
    {
        return string( "Ellipse" );
    }
    return string( "NONE" );
}

bool SubSurface::Subtag( const vec3d & center )
{
    if ( !m_PolyPntsReadyFlag ) // Update polygon vector
    {
        m_PolyPnts.clear();

        int last_ind = m_LVec.size() - 1;
        vec3d pnt;
        for ( int ls = 0 ; ls < last_ind + 1 ; ls++ )
        {
            pnt = m_LVec[ls].GetP0();
            m_PolyPnts.push_back( vec2d( pnt.x(), pnt.y() ) );
        }
        pnt = m_LVec[last_ind].GetP1();
        m_PolyPnts.push_back( vec2d( pnt.x(), pnt.y() ) );

        m_PolyPntsReadyFlag = true;
    }

    bool inPoly = PointInPolygon( vec2d( center.x(), center.y() ) , m_PolyPnts );

    if ( ( inPoly && m_TestType() == INSIDE ) || ( !inPoly && m_TestType() == OUTSIDE ) )
    {
        return true;
    }

    return false;
}
bool SubSurface::Subtag( TTri* tri )
{
    vec3d center = tri->ComputeCenterUW();
    return Subtag( center );
}


void SubSurface::SplitSegs( const vector<int> & split_u, const vector<int> & split_w )
{
    // Method to Split subsurfaces for CFDMesh surfs
    CleanUpSplitVec();

    PrepareSplitVec();

    for ( int ui = 0 ; ui < ( int )split_u.size() ; ui++ )
    {
        SplitSegsU( split_u[ui] );
    }

    for ( int wi = 0 ; wi < ( int )split_w.size() ; wi++ )
    {
        SplitSegsW( split_w[wi] );
    }
}

void SubSurface::SplitSegsU( const double & u )
{
    double tol = 1e-10;
    int num_l_segs = m_SplitLVec.size();
    int num_splits = 0;
    bool reorder = false;
    vector<SSLineSeg> new_lsegs;
    vector<int> inds;
    for ( int i = 0 ; i < num_l_segs ; i++ )
    {
        SSLineSeg& seg = m_SplitLVec[i];
        vec3d p0 = seg.GetP0();
        vec3d p1 = seg.GetP1();

        double t = ( u - p0.x() ) / ( p1.x() - p0.x() );

        if ( t < 1 - tol && t > 0 + tol )
        {
            if ( m_FirstSplit )
            {
                m_FirstSplit = false;
                reorder = true;
            }
            // Split the segments
            vec3d int_pnt = point_on_line( p0, p1, t );
            SSLineSeg split_seg = SSLineSeg( seg );

            seg.SetP1( int_pnt );
            split_seg.SetP0( int_pnt );
            inds.push_back( i + num_splits + 1 );
            new_lsegs.push_back( split_seg );
            num_splits++;
        }
    }

    for ( int i = 0; i < ( int )inds.size() ; i++ )
    {
        m_SplitLVec.insert( m_SplitLVec.begin() + inds[i], new_lsegs[i] );
    }

    if ( reorder )
    {
        ReorderSplitSegs( inds[0] );
    }
}

void SubSurface::SplitSegsW( const double & w )
{
    double tol = 1e-10;
    int num_l_segs = m_SplitLVec.size();
    int num_splits = 0;
    bool reorder = false;
    vector<SSLineSeg> new_lsegs;
    vector<int> inds;
    for ( int i = 0 ; i < num_l_segs ; i++ )
    {

        SSLineSeg& seg = m_SplitLVec[i];
        vec3d p0 = seg.GetP0();
        vec3d p1 = seg.GetP1();

        double t = ( w - p0.y() ) / ( p1.y() - p0.y() );

        if ( t < 1 - tol && t > 0 + tol )
        {
            if ( m_FirstSplit )
            {
                m_FirstSplit = false;
                reorder = true;
            }
            // Split the segments
            vec3d int_pnt = point_on_line( p0, p1, t );
            SSLineSeg split_seg = SSLineSeg( seg );

            seg.SetP1( int_pnt );
            split_seg.SetP0( int_pnt );
            inds.push_back( i + num_splits + 1 );
            new_lsegs.push_back( split_seg );
            num_splits++;
        }
    }

    for ( int i = 0; i < ( int )inds.size() ; i++ )
    {
        m_SplitLVec.insert( m_SplitLVec.begin() + inds[i], new_lsegs[i] );
    }

    if ( reorder )
    {
        ReorderSplitSegs( inds[0] );
    }
}

void SubSurface::ReorderSplitSegs( int ind )
{
    if ( ind < 0 || ind > m_SplitLVec.size() - 1 )
    {
        return;
    }

    vector<SSLineSeg> ret_vec;
    ret_vec.resize( m_SplitLVec.size() );

    int cnt = 0;
    for ( int i = ind ; i < m_SplitLVec.size() ; i++ )
    {
        ret_vec[cnt] = m_SplitLVec[i];
        cnt++;
    }
    for ( int i = 0 ; i < ind ; i++ )
    {
        ret_vec[cnt] = m_SplitLVec[i];
        cnt++;
    }

    m_SplitLVec = ret_vec;
}

void SubSurface::CleanUpSplitVec()
{
    m_SplitLVec.clear();
}

void SubSurface::PrepareSplitVec()
{
    CleanUpSplitVec();
    m_FirstSplit = true;
    m_SplitLVec = m_LVec;
}

//////////////////////////////////////////////////////
//=================== SSLineSeg =====================//
//////////////////////////////////////////////////////

SSLineSeg::SSLineSeg()
{
    m_TestType = GT;
}

SSLineSeg::~SSLineSeg()
{
}

bool SSLineSeg::Subtag( const vec3d & center ) const
{
    // Compute cross product of line and first point to center
    vec3d v0c = center - m_P0;
    vec3d c_prod = cross( m_line, v0c );

    if ( m_TestType == GT && c_prod.z() > 0 )
    {
        return true;
    }
    if ( m_TestType == LT && c_prod.z() < 0 )
    {
        return true;
    }

    return false;
}

bool SSLineSeg::Subtag( TTri* tri ) const
{
    vec3d center = tri->ComputeCenterUW();

    return Subtag( center );
}

void SSLineSeg::Update( Geom* geom )
{
    VspSurf* surf = geom->GetSurfPtr();
    if ( !surf )
    {
        return;
    }

    int num_u = surf->GetNumSectU();
    int num_w = surf->GetNumSectW();
    // Update none scaled points
    m_P0.set_xyz( m_SP0[0]*num_u, m_SP0[1]*num_w, 0 );
    m_P1.set_xyz( m_SP1[0]*num_u, m_SP1[1]*num_w, 0 );

    // Update line
    m_line = m_P1 - m_P0;
}

int SSLineSeg::CompNumDrawPnts( Geom* geom )
{
    VspSurf* surf = geom->GetSurfPtr();
    if ( !surf )
    {
        return 0;
    }
    double avg_num_secs = ( double )( surf->GetNumSectU() + surf->GetNumSectW() ) / 2.0;
    double avg_tess = ( double )( geom->m_TessU() + geom->m_TessW() ) / 2.0;

    return ( int )avg_num_secs * ( avg_tess - 1 );
}

void SSLineSeg::UpdateDrawObj( Geom* geom, DrawObj& draw_obj, const int *num_pnts_ptr )
{
    int num_pnts;
    if ( num_pnts_ptr )
    {
        num_pnts = *num_pnts_ptr;
    }
    else
    {
        num_pnts = CompNumDrawPnts( geom );
    }

    draw_obj.m_PntVec.resize( num_pnts * 2 );

    VspSurf* surf = geom->GetSurfPtr();
    draw_obj.m_PntVec[0] = CompPnt( surf, m_P0 );
    int pi = 1;
    for ( int i = 1 ; i < num_pnts ; i ++ )
    {
        vec3d uw = ( m_P0 + m_line * ( ( double )i / num_pnts ) );
        draw_obj.m_PntVec[pi] = CompPnt( surf, uw );
        draw_obj.m_PntVec[pi + 1] = draw_obj.m_PntVec[pi];
        pi += 2;
    }

    draw_obj.m_PntVec[pi] = CompPnt( surf, m_P1 );

    draw_obj.m_LineWidth = 3.0;
    draw_obj.m_LineColor = vec3d( 177.0 / 255, 1, 58.0 / 255 );
    draw_obj.m_Type = DrawObj::VSP_LINES;
    draw_obj.m_GeomChanged = true;
}

vec3d SSLineSeg::CompPnt( VspSurf* surf, vec3d uw_pnt ) const
{
    if ( !surf )
    {
        return vec3d();
    }

    int num_u = surf->GetNumSectU();
    int num_w = surf->GetNumSectW();

    if ( uw_pnt.x() < 0 )
    {
        uw_pnt.set_x( 0 );
    }
    else if ( uw_pnt.x() > num_u )
    {
        uw_pnt.set_x( num_u );
    }

    if ( uw_pnt.y() < 0 )
    {
        uw_pnt.set_y( 0 );
    }
    else if ( uw_pnt.y() > num_w )
    {
        uw_pnt.set_y( num_w );
    }

    return surf->CompPnt( uw_pnt.x(), uw_pnt.y() );
}

TMesh* SSLineSeg::CreateTMesh()
{
    int num_cut_lines = 0;
    int num_z_lines = 0;

    TMesh* tmesh = new TMesh();

    vec3d dc = m_line / ( num_cut_lines + 1.0 );
    vec3d dz = vec3d( 0, 0, 2.0 ) / ( num_z_lines + 1 );
    vec3d start = m_P0 + vec3d( 0, 0, -1 );

    int c, cz;

    vector< vector< vec3d > > pnt_mesh;
    pnt_mesh.resize( num_cut_lines + 2 );
    for ( int i = 0; i < ( int )pnt_mesh.size(); i++ )
    {
        pnt_mesh[i].resize( num_z_lines + 2 );
    }

    // Build plane
    for ( c = 0 ; c < num_cut_lines + 2 ; c++ )
    {
        for ( cz = 0 ; cz < num_z_lines + 2 ; cz++ )
        {
            pnt_mesh[c][cz] = start + dc * c + dz * cz;
        }
    }

    // Build triangles on that plane

    for ( c = 0 ; c < ( int )pnt_mesh.size() - 1 ; c++ )
    {
        for ( cz = 0 ; cz < ( int )pnt_mesh[c].size() - 1 ; cz ++ )
        {
            vec3d v0, v1, v2, v3, d01, d21, d20, d03, d23, norm;

            v0 = pnt_mesh[c][cz];
            v1 = pnt_mesh[c + 1][cz];
            v2 = pnt_mesh[c + 1][cz + 1];
            v3 = pnt_mesh[c][cz + 1];

            d21 = v2 - v1;
            d01 = v0 - v1;
            d20 = v2 - v0;

            if ( d21.mag() > 0.000001 && d01.mag() > 0.000001 && d20.mag() > 0.000001 )
            {
                norm = cross( d21, d01 );
                norm.normalize();
                tmesh->AddUWTri( v0, v1, v2, norm );
            }

            d03 = v0 - v3;
            d23 = v2 - v3;
            if ( d03.mag() > 0.000001 && d23.mag() > 0.000001 && d20.mag() > 0.000001 )
            {
                norm = cross( d03, d23 );
                norm.normalize();
                tmesh->AddUWTri( v0, v2, v3, norm );
            }
        }
    }
    return tmesh;
}

