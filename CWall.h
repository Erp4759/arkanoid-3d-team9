#pragma once

#include "d3dUtility.h"
class CSphere;

class CWall {
private:
    float m_x;
    float m_z;
    float m_width;
    float m_depth;
    float m_height;

    D3DXMATRIX   m_mLocal;
    D3DMATERIAL9 m_mtrl;
    ID3DXMesh*   m_pBoundMesh;

public:
    CWall(void);
    ~CWall(void);

    bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE);
    void destroy(void);
    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);

    bool hasIntersected(CSphere& ball);
    void hitBy(CSphere& ball);

    void setPosition(float x, float y, float z);
    float getHeight(void) const;

private:
    void setLocalTransform(const D3DXMATRIX& mLocal);
};
