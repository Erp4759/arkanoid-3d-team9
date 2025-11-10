#include "CSphere.h"
#include <cmath>

#define M_RADIUS 0.21f
#define DECREASE_RATE 0.9982f

CSphere::CSphere(void)
{
    D3DXMatrixIdentity(&m_mLocal);
    ZeroMemory(&m_mtrl, sizeof(m_mtrl));
    m_radius = 0;
    m_velocity_x = 0;
    m_velocity_z = 0;
    m_pSphereMesh = NULL;
    center_x = center_y = center_z = 0.0f;
}

CSphere::~CSphere(void) {}

bool CSphere::create(IDirect3DDevice9* pDevice, D3DXCOLOR color)
{
    if (NULL == pDevice) return false;
    m_mtrl.Ambient  = color;
    m_mtrl.Diffuse  = color;
    m_mtrl.Specular = color;
    m_mtrl.Emissive = d3d::BLACK;
    m_mtrl.Power    = 5.0f;
    if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL))) return false;
    return true;
}

void CSphere::destroy(void)
{
    if (m_pSphereMesh) { m_pSphereMesh->Release(); m_pSphereMesh = NULL; }
}

void CSphere::draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
{
    if (!pDevice) return;
    pDevice->SetTransform(D3DTS_WORLD, &mWorld);
    pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
    pDevice->SetMaterial(&m_mtrl);
    if (m_pSphereMesh) m_pSphereMesh->DrawSubset(0);
}

bool CSphere::hasIntersected(CSphere& ball)
{
    D3DXVECTOR3 myCenter = this->getCenter();
    D3DXVECTOR3 ballCenter = ball.getCenter();
    
    float dx = myCenter.x - ballCenter.x;
    float dy = myCenter.y - ballCenter.y;
    float dz = myCenter.z - ballCenter.z;
    float distance = sqrt(dx*dx + dy*dy + dz*dz);
    
    return (distance < (this->getRadius() + ball.getRadius()));
}

void CSphere::hitBy(CSphere& ball)
{
    if (!hasIntersected(ball)) return;
    
    D3DXVECTOR3 myCenter = this->getCenter();
    D3DXVECTOR3 ballCenter = ball.getCenter();
    
    // Collision normal (from ball to this)
    float dx = myCenter.x - ballCenter.x;
    float dz = myCenter.z - ballCenter.z;
    float distance = sqrt(dx*dx + dz*dz);
    
    if (distance < 0.001f) return; // Too close, skip
    
    // Normalize
    dx /= distance;
    dz /= distance;
    
    // Reflect ball's velocity
    float ballVx = (float)ball.getVelocity_X();
    float ballVz = (float)ball.getVelocity_Z();
    
    // Dot product
    float dot = ballVx * dx + ballVz * dz;
    
    // Reflect
    float newVx = ballVx - 2 * dot * dx;
    float newVz = ballVz - 2 * dot * dz;
    
    ball.setPower(newVx, newVz);
    
    // Separate balls to prevent sticking
    float overlap = (this->getRadius() + ball.getRadius()) - distance;
    if (overlap > 0) {
        ballCenter.x -= dx * overlap * 0.5f;
        ballCenter.z -= dz * overlap * 0.5f;
        ball.setCenter(ballCenter.x, ballCenter.y, ballCenter.z);
    }
}

void CSphere::ballUpdate(float timeDiff)
{
    const float TIME_SCALE = 3.3f;
    D3DXVECTOR3 cord = getCenter();
    double vx = fabs(getVelocity_X());
    double vz = fabs(getVelocity_Z());

    if (vx > 0.01 || vz > 0.01) {
        float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
        float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;
        setCenter(tX, cord.y, tZ);
    } else {
        setPower(0, 0);
    }
    double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400;
    if (rate < 0) rate = 0;
    setPower(getVelocity_X() * rate, getVelocity_Z() * rate);
}

double CSphere::getVelocity_X() const { return m_velocity_x; }

double CSphere::getVelocity_Z() const { return m_velocity_z; }

void CSphere::setPower(double vx, double vz) { m_velocity_x = (float)vx; m_velocity_z = (float)vz; }

void CSphere::setCenter(float x, float y, float z)
{
    center_x = x; center_y = y; center_z = z;
    D3DXMATRIX m; D3DXMatrixTranslation(&m, x, y, z);
    setLocalTransform(m);
}

float CSphere::getRadius(void) const { return (float)(M_RADIUS); }

const D3DXMATRIX& CSphere::getLocalTransform(void) const { return m_mLocal; }
void CSphere::setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

D3DXVECTOR3 CSphere::getCenter(void) const { return D3DXVECTOR3(center_x, center_y, center_z); }
