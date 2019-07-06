#ifndef RENDER_CACHE_H
#define RENDER_CACHE_H

class RenderCache
{
public:
	RenderCache(IDirect3DDevice9* device, Material* material);
	~RenderCache();

	void AddFace();
	void Prepare(unsigned long totalVertices);

	void Begin();
	void RenderFace(unsigned short vertex0, unsigned short vertex1, unsigned short vertex2);
	void End();

	Material* GetMaterial();

private:
	IDirect3DDevice9* m_device;
	Material* m_material;

	IDirect3DIndexBuffer9* m_indexBuffer;
	unsigned short* m_indexPointer;// pointer to indexBuffer

	unsigned long  m_totalVertices;
	unsigned long m_totalFace;

	unsigned long m_totalIndices;
};
#endif