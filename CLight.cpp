#include "CLight.h"

CLight::CLight(void)
{
    static DWORD i = 0; m_index = i++;
    D3DXMatrixIdentity(&m_mLocal);
    ::ZeroMemory(&m_lit, sizeof(m_lit));
    m_pMesh = NULL;
    m_bound._center = D3DXVECTOR3(0, 0, 0);
    m_bound._radius = 0.0f;
}

CLight::~CLight(void) {}

bool CLight::create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius)
{
    if (!pDevice) return false;
    if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL))) return false;
    m_bound._center = lit.Position;
    m_bound._radius = radius;
    m_lit = lit;
    return true;
}

void CLight::destroy(void)
{
    if (m_pMesh) { m_pMesh->Release(); m_pMesh = NULL; }
}

bool CLight::setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
{
    if (!pDevice) return false;
    D3DXVECTOR3 pos(m_bound._center);
    D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
    D3DXVec3TransformCoord(&pos, &pos, &mWorld);
    m_lit.Position = pos;
    pDevice->SetLight(m_index, &m_lit);
    pDevice->LightEnable(m_index, TRUE);
    return true;
}

void CLight::draw(IDirect3DDevice9* pDevice)
{
    if (!pDevice) return;
    D3DXMATRIX m; D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
    pDevice->SetTransform(D3DTS_WORLD, &m);
    pDevice->SetMaterial(&d3d::WHITE_MTRL);
    if (m_pMesh) m_pMesh->DrawSubset(0);
}

D3DXVECTOR3 CLight::getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }
