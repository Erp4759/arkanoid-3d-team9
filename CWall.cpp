#include "CWall.h"
#include "CSphere.h"
#include <cmath>

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

bool CWall::hasIntersected(CSphere& ball)
{
    D3DXVECTOR3 ballCenter = ball.getCenter();
    float radius = ball.getRadius();
    
    // Calculate wall boundaries
    float minX = m_x - m_width/2.0f;
    float maxX = m_x + m_width/2.0f;
    float minZ = m_z - m_depth/2.0f;
    float maxZ = m_z + m_depth/2.0f;
    
    // Find closest point on wall to ball
    float closestX = ballCenter.x;
    float closestZ = ballCenter.z;
    
    if (closestX < minX) closestX = minX;
    if (closestX > maxX) closestX = maxX;
    if (closestZ < minZ) closestZ = minZ;
    if (closestZ > maxZ) closestZ = maxZ;
    
    // Distance from ball to closest point
    float dx = ballCenter.x - closestX;
    float dz = ballCenter.z - closestZ;
    float distance = sqrt(dx*dx + dz*dz);
    
    return (distance < radius);
}

void CWall::hitBy(CSphere& ball)
{
    if (!hasIntersected(ball)) return;
    
    D3DXVECTOR3 ballCenter = ball.getCenter();
    float radius = ball.getRadius();
    
    float minX = m_x - m_width/2.0f;
    float maxX = m_x + m_width/2.0f;
    float minZ = m_z - m_depth/2.0f;
    float maxZ = m_z + m_depth/2.0f;
    
    double vx = ball.getVelocity_X();
    double vz = ball.getVelocity_Z();
    
    // Determine which face was hit and reflect
    float overlapLeft = (ballCenter.x + radius) - minX;
    float overlapRight = maxX - (ballCenter.x - radius);
    float overlapTop = maxZ - (ballCenter.z - radius);
    float overlapBottom = (ballCenter.z + radius) - minZ;
    
    float minOverlap = overlapLeft;
    int face = 0; // 0=left, 1=right, 2=top, 3=bottom
    
    if (overlapRight < minOverlap) { minOverlap = overlapRight; face = 1; }
    if (overlapTop < minOverlap) { minOverlap = overlapTop; face = 2; }
    if (overlapBottom < minOverlap) { minOverlap = overlapBottom; face = 3; }
    
    // Reflect velocity based on face and apply damping
    if (face == 0 || face == 1) {
        // Hit vertical wall (left or right)
        ball.setPower(-vx * 0.95, vz);
        // Push ball out of wall
        if (face == 0) {
            ball.setCenter(minX - radius, ballCenter.y, ballCenter.z);
        } else {
            ball.setCenter(maxX + radius, ballCenter.y, ballCenter.z);
        }
    } else {
        // Hit horizontal wall (top or bottom)
        ball.setPower(vx, -vz * 0.95);
        // Push ball out of wall
        if (face == 3) {
            ball.setCenter(ballCenter.x, ballCenter.y, minZ - radius);
        } else {
            ball.setCenter(ballCenter.x, ballCenter.y, maxZ + radius);
        }
    }
}

void CWall::setPosition(float x, float y, float z)
{
    m_x = x; m_z = z;
    D3DXMATRIX m; D3DXMatrixTranslation(&m, x, y, z);
    setLocalTransform(m);
}

float CWall::getHeight(void) const { return 0.01f; }

void CWall::setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
