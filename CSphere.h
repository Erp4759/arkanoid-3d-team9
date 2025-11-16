#pragma once

#include "d3dUtility.h"

class CSphere {
private:
    float center_x, center_y, center_z;
    float m_radius; // unused in current logic, kept for compatibility
    float m_velocity_x;
    float m_velocity_z;

    D3DXMATRIX   m_mLocal;
    D3DMATERIAL9 m_mtrl;
    ID3DXMesh*   m_pSphereMesh;

public:
    CSphere(void);
    ~CSphere(void);

    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE);
    void destroy(void);
    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);

    bool hasIntersected(CSphere& ball);
    void hitBy(CSphere& ball);
    void paddleHitBy(CSphere& ball);
	bool ballClose(CSphere& ball);
	void greenTime(CSphere& ball);

    void ballUpdate(float timeDiff);

    double getVelocity_X() const;
    double getVelocity_Z() const;

    void setPower(double vx, double vz);
    void setCenter(float x, float y, float z);

    float getRadius(void) const;

    const D3DXMATRIX& getLocalTransform(void) const;
    void setLocalTransform(const D3DXMATRIX& mLocal);

    D3DXVECTOR3 getCenter(void) const;
};
