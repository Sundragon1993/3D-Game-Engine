#include "Engine.h"

Bounding_Extended_Volume::Bounding_Extended_Volume()
{
	X_box = new Bounding_BoxEx;
	X_sphere = new Bounding_Sphere_Ex;

	m_ellipsoidRadius = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
}


Bounding_Extended_Volume::~Bounding_Extended_Volume()
{
	SAFE_DELETE( X_box );
	SAFE_DELETE( X_sphere );
}


void Bounding_Extended_Volume::BoundingVolumeFromMesh( ID3DXMesh *mesh, D3DXVECTOR3 ellipsoidRadius )
{
	D3DXVECTOR3 *vertices;
	if( SUCCEEDED( mesh->LockVertexBuffer( D3DLOCK_READONLY, (void**)&vertices ) ) )
	{
		D3DXComputeBoundingBox( vertices, mesh->GetNumVertices(), D3DXGetFVFVertexSize( mesh->GetFVF() ), &X_box->min, &X_box->max );
		D3DXComputeBoundingSphere( vertices, mesh->GetNumVertices(), D3DXGetFVFVertexSize( mesh->GetFVF() ), &X_sphere->centre, &X_sphere->radius );
		mesh->UnlockVertexBuffer();
	}

	X_sphere->centre.x = X_box->min.x + ( ( X_box->max.x - X_box->min.x ) / 2.0f );
	X_sphere->centre.y = X_box->min.y + ( ( X_box->max.y - X_box->min.y ) / 2.0f );
	X_sphere->centre.z = X_box->min.z + ( ( X_box->max.z - X_box->min.z ) / 2.0f );

	X_box->halfSize = (float)max( fabs( X_box->max.x ), max( fabs( X_box->max.y ), fabs( X_box->max.z ) ) );
	X_box->halfSize = (float)max( X_box->halfSize, max( fabs( X_box->min.x ), max( fabs( X_box->min.y ), fabs( X_box->min.z ) ) ) );

	m_originalMin = X_box->min;
	m_originalMax = X_box->max;
	m_originalCentre = X_sphere->centre;

	SetEllipsoidRadius( ellipsoidRadius );
}


void Bounding_Extended_Volume::BoundingVolumeFromVertices( D3DXVECTOR3 *vertices, unsigned long totalVertices, unsigned long vertexStride, D3DXVECTOR3 ellipsoidRadius )
{
	D3DXComputeBoundingBox( vertices, totalVertices, vertexStride, &X_box->min, &X_box->max );
	D3DXComputeBoundingSphere( vertices, totalVertices, vertexStride, &X_sphere->centre, &X_sphere->radius );

	X_sphere->centre.x = X_box->min.x + ( ( X_box->max.x - X_box->min.x ) / 2.0f );
	X_sphere->centre.y = X_box->min.y + ( ( X_box->max.y - X_box->min.y ) / 2.0f );
	X_sphere->centre.z = X_box->min.z + ( ( X_box->max.z - X_box->min.z ) / 2.0f );

	X_box->halfSize = (float)max( fabs( X_box->max.x ), max( fabs( X_box->max.y ), fabs( X_box->max.z ) ) );
	X_box->halfSize = (float)max( X_box->halfSize, max( fabs( X_box->min.x ), max( fabs( X_box->min.y ), fabs( X_box->min.z ) ) ) );

	m_originalMin = X_box->min;
	m_originalMax = X_box->max;
	m_originalCentre = X_sphere->centre;

	SetEllipsoidRadius( ellipsoidRadius );
}


void Bounding_Extended_Volume::CloneBoundingVolume( Bounding_BoxEx *box, Bounding_Sphere_Ex *sphere, D3DXVECTOR3 ellipsoidRadius )
{
	X_box->min = box->min;
	X_box->max = box->max;
	X_sphere->centre = sphere->centre;
	X_sphere->radius = sphere->radius;

	X_box->halfSize = (float)max( fabs( X_box->max.x ), max( fabs( X_box->max.y ), fabs( X_box->max.z ) ) );
	X_box->halfSize = (float)max( X_box->halfSize, max( fabs( X_box->min.x ), max( fabs( X_box->min.y ), fabs( X_box->min.z ) ) ) );

	m_originalMin = X_box->min;
	m_originalMax = X_box->max;
	m_originalCentre = X_sphere->centre;

	SetEllipsoidRadius( ellipsoidRadius );
}


void Bounding_Extended_Volume::RepositionBoundingVolume( D3DXMATRIX *location )
{
	D3DXVec3TransformCoord( &X_box->min, &m_originalMin, location );
	D3DXVec3TransformCoord( &X_box->max, &m_originalMax, location );
	D3DXVec3TransformCoord( &X_sphere->centre, &m_originalCentre, location );
}


void Bounding_Extended_Volume::SetBoundingBox( D3DXVECTOR3 min, D3DXVECTOR3 max )
{
	m_originalMin = X_box->min = min;
	m_originalMax = X_box->max = max;

	X_box->halfSize = (float)max( fabs( X_box->max.x ), max( fabs( X_box->max.y ), fabs( X_box->max.z ) ) );
	X_box->halfSize = (float)max( X_box->halfSize, max( fabs( X_box->min.x ), max( fabs( X_box->min.y ), fabs( X_box->min.z ) ) ) );
}

Bounding_BoxEx *Bounding_Extended_Volume::GetBoundingBox()
{
	return X_box;
}


void Bounding_Extended_Volume::SetBoundingSphere( D3DXVECTOR3 centre, float radius, D3DXVECTOR3 ellipsoidRadius )
{
	m_originalCentre = X_sphere->centre = centre;
	X_sphere->radius = radius;

	SetEllipsoidRadius( ellipsoidRadius );
}


Bounding_Sphere_Ex *Bounding_Extended_Volume::GetBoundingSphere()
{
	return X_sphere;
}


void Bounding_Extended_Volume::SetEllipsoidRadius( D3DXVECTOR3 ellipsoidRadius )
{
	m_ellipsoidRadius = D3DXVECTOR3(
		X_sphere->radius * ellipsoidRadius.x, 
		X_sphere->radius * ellipsoidRadius.y, 
		X_sphere->radius * ellipsoidRadius.z );
}

D3DXVECTOR3 Bounding_Extended_Volume::GetEllipsoidRadius()
{
	return m_ellipsoidRadius;
}