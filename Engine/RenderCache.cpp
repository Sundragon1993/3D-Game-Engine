//-----------------------------------------------------------------------------
// RenderCache.h implementation.
// Refer to the RenderCache.h interface for more details.
//
// Copyright (c) 2016 Dang Cong Hoang
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// The render cache class constructor.
//-----------------------------------------------------------------------------

RenderCache::RenderCache(IDirect3DDevice9 * device, Material * material)
{
	m_device = device;
	m_indexBuffer = NULL;
	m_material = material;
	m_totalIndices = 0;
}

RenderCache::~RenderCache()
{
	SAFE_RELEASE(m_indexBuffer);
}

void RenderCache::AddFace()
{
	m_totalIndices += 3;
}
//Create index buffer
void RenderCache::Prepare(unsigned long totalVertices)
{
	m_totalVertices = totalVertices;
	m_device->CreateIndexBuffer(m_totalIndices * sizeof(unsigned short),
		D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_indexBuffer, NULL);
}
//Lock indexBuffer
void RenderCache::Begin()
{
	//third parameter is out 
	m_indexBuffer->Lock(0, 0, (void**)&m_indexPointer, 0);
	m_totalFace = 0;
}
//add given face to be rendered
void RenderCache::RenderFace(unsigned short vertex0, unsigned short vertex1, unsigned short vertex2) {
	*m_indexPointer++ = vertex0;
	*m_indexPointer++ = vertex1;
	*m_indexPointer++ = vertex2;

	m_totalFace++;
}

void RenderCache::End() //send IndexBuffer to VGA to RENDER
{
	m_indexBuffer->Unlock();
	if (m_totalFace == 0) return;
	// Check if the material should ignore fog.
	if (m_material->GetIgnoreFog() == true)
		m_device->SetRenderState(D3DRS_FOGENABLE, false);

	//set texture and material
	m_device->SetTexture(0, m_material->GetTexture());
	m_device->SetMaterial(m_material->GetLighting());

	m_device->SetIndices(m_indexBuffer);

	m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_totalVertices, 0, m_totalFace);
	// Restore the fog setting if it was changed.
	if (m_material->GetIgnoreFog() == true)
		m_device->SetRenderState(D3DRS_FOGENABLE, true);
}

Material *RenderCache::GetMaterial() {
	return m_material;
}