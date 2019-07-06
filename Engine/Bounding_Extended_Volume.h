
#ifndef BOUNDING_VOLUME_H
#define BOUNDING_VOLUME_H
struct Bounding_BoxEx
{
	D3DXVECTOR3 min; // Minimum extent of the bounding box.
	D3DXVECTOR3 max; // Maximum extent of the bounding box.
	float halfSize; // Distance from the centre of the volume to the furthest point on any axis.
};

struct Bounding_Sphere_Ex
{
	D3DXVECTOR3 centre; // Centre point of the bounding sphere.
	float radius; // Radius of the bounding sphere.
};

class Bounding_Extended_Volume
{
public:
	Bounding_Extended_Volume();
	virtual ~Bounding_Extended_Volume();

	void BoundingVolumeFromMesh(ID3DXMesh *mesh, D3DXVECTOR3 ellipsoidRadius = D3DXVECTOR3(1.0f, 1.0f, 1.0f));
	void BoundingVolumeFromVertices(D3DXVECTOR3 *vertices, unsigned long totalVertices, unsigned long vertexStride, D3DXVECTOR3 ellipsoidRadius = D3DXVECTOR3(1.0f, 1.0f, 1.0f));
	void CloneBoundingVolume(Bounding_BoxEx *box, Bounding_Sphere_Ex *sphere, D3DXVECTOR3 ellipsoidRadius = D3DXVECTOR3(1.0f, 1.0f, 1.0f));
	void RepositionBoundingVolume(D3DXMATRIX *location);

	void SetBoundingBox(D3DXVECTOR3 min, D3DXVECTOR3 max);
	Bounding_BoxEx *GetBoundingBox();

	void SetBoundingSphere(D3DXVECTOR3 centre, float radius, D3DXVECTOR3 ellipsoidRadius = D3DXVECTOR3(1.0f, 1.0f, 1.0f));
	Bounding_Sphere_Ex *GetBoundingSphere();

	void SetEllipsoidRadius(D3DXVECTOR3 ellipsoidRadius);
	D3DXVECTOR3 GetEllipsoidRadius();

private:
	Bounding_BoxEx *X_box; // Box representation of the bounding volume.
	Bounding_Sphere_Ex *X_sphere; // Sphere representation of the bounding volume.

	D3DXVECTOR3 m_originalMin; // Original minimum extents of the bounding box.
	D3DXVECTOR3 m_originalMax; // Original maximum extents of the bounding box.
	D3DXVECTOR3 m_originalCentre; // Original centre point of the bounding sphere.

	D3DXVECTOR3 m_ellipsoidRadius; // Ellipsoid radius (i.e. the radius along each axis).
};

#endif