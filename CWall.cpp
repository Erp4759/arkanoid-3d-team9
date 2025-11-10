#include "CWall.h"
#include "CSphere.h"

CWall::CWall(void)
{
    D3DXMatrixIdentity(&m_mLocal);
    ZeroMemory(&m_mtrl, sizeof(m_mtrl));
    m_width = 0; m_depth = 0; m_height = 0; m_pBoundMesh = NULL;
    m_x = m_z = 0.0f;
}

CWall::~CWall(void) {}

bool CWall::create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color)
{
    if (!pDevice) return false;
    m_mtrl.Ambient  = color;
    m_mtrl.Diffuse  = color;
    m_mtrl.Specular = color;
    m_mtrl.Emissive = d3d::BLACK;
    m_mtrl.Power    = 5.0f;
    m_width = iwidth; m_depth = idepth; m_height = iheight;
    return !FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL));
}

void CWall::destroy(void)
{
    if (m_pBoundMesh) { m_pBoundMesh->Release(); m_pBoundMesh = NULL; }
}

void CWall::draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
{
    if (!pDevice) return;
    pDevice->SetTransform(D3DTS_WORLD, &mWorld);
    pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
    pDevice->SetMaterial(&m_mtrl);
    if (m_pBoundMesh) m_pBoundMesh->DrawSubset(0);
}

bool CWall::hasIntersected(CSphere& /*ball*/)
{
    return false;
}

void CWall::hitBy(CSphere& /*ball*/)
{
}

void CWall::setPosition(float x, float y, float z)
{
    m_x = x; m_z = z;
    D3DXMATRIX m; D3DXMatrixTranslation(&m, x, y, z);
    setLocalTransform(m);
}

float CWall::getHeight(void) const { return 0.01f; }

void CWall::setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
