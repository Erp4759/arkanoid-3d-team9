////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: ¹ÚÃ¢Çö Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>

IDirect3DDevice9* Device = NULL;

// window size
const int Width  = 1024;
const int Height = 768;

// There are four balls
// Arkanoid-like layout: vertical field, three bricks at the top row and one white ball at the bottom center
const float spherePos[4][2] = { {-2.0f, 3.6f} , {0.0f, 3.6f} , {+2.0f, 3.6f} , {0.0f, -3.6f} }; 
const D3DXCOLOR sphereColor[4] = {d3d::RED, d3d::RED, d3d::YELLOW, d3d::WHITE};

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982

// -----------------------------------------------------------------------------
// CSphere class definition (expanded formatting restored)
// -----------------------------------------------------------------------------
class CSphere {
private:
    float center_x, center_y, center_z;
    float m_radius;
    float m_velocity_x;
    float m_velocity_z;

public:
    CSphere(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_radius = 0;
        m_velocity_x = 0;
        m_velocity_z = 0;
        m_pSphereMesh = NULL;
    }
    ~CSphere(void) {}

    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
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

    void destroy(void)
    {
        if (m_pSphereMesh) { m_pSphereMesh->Release(); m_pSphereMesh = NULL; }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (!pDevice) return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
        if (m_pSphereMesh) m_pSphereMesh->DrawSubset(0);
    }

    bool hasIntersected(CSphere& /*ball*/)
    {
        // TODO: implement collision detection
        return false;
    }

    void hitBy(CSphere& /*ball*/)
    {
        // TODO: implement collision response
    }

    void ballUpdate(float timeDiff)
    {
        const float TIME_SCALE = 3.3f;
        D3DXVECTOR3 cord = getCenter();
        double vx = fabs(getVelocity_X());
        double vz = fabs(getVelocity_Z());

        if (vx > 0.01 || vz > 0.01) {
            float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
            float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;
            // Wall clamping could be re-added when collisions are implemented
            setCenter(tX, cord.y, tZ);
        } else {
            setPower(0, 0);
        }
        double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400;
        if (rate < 0) rate = 0;
        setPower(getVelocity_X() * rate, getVelocity_Z() * rate);
    }

    double getVelocity_X() const { return m_velocity_x; }
    double getVelocity_Z() const { return m_velocity_z; }

    void setPower(double vx, double vz) { m_velocity_x = (float)vx; m_velocity_z = (float)vz; }

    void setCenter(float x, float y, float z)
    {
        center_x = x; center_y = y; center_z = z;
        D3DXMATRIX m; D3DXMatrixTranslation(&m, x, y, z);
        setLocalTransform(m);
    }

    float getRadius(void) const { return (float)(M_RADIUS); }

    const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

    D3DXVECTOR3 getCenter(void) const { return D3DXVECTOR3(center_x, center_y, center_z); }

private:
    D3DXMATRIX   m_mLocal;
    D3DMATERIAL9 m_mtrl;
    ID3DXMesh*   m_pSphereMesh;
};

// -----------------------------------------------------------------------------
// CWall class definition (expanded formatting restored)
// -----------------------------------------------------------------------------
class CWall {
private:
    float m_x;
    float m_z;
    float m_width;
    float m_depth;
    float m_height;

public:
    CWall(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_width = 0; m_depth = 0; m_height = 0; m_pBoundMesh = NULL;
    }
    ~CWall(void) {}

    bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
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

    void destroy(void)
    {
        if (m_pBoundMesh) { m_pBoundMesh->Release(); m_pBoundMesh = NULL; }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (!pDevice) return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
        if (m_pBoundMesh) m_pBoundMesh->DrawSubset(0);
    }

    bool hasIntersected(CSphere& /*ball*/)
    {
        // TODO: implement
        return false;
    }

    void hitBy(CSphere& /*ball*/)
    {
        // TODO: implement
    }

    void setPosition(float x, float y, float z)
    {
        m_x = x; m_z = z;
        D3DXMATRIX m; D3DXMatrixTranslation(&m, x, y, z);
        setLocalTransform(m);
    }

    float getHeight(void) const { return M_HEIGHT; }

private:
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

    D3DXMATRIX   m_mLocal;
    D3DMATERIAL9 m_mtrl;
    ID3DXMesh*   m_pBoundMesh;
};

// -----------------------------------------------------------------------------
// CLight class definition (expanded formatting restored)
// -----------------------------------------------------------------------------
class CLight {
public:
    CLight(void)
    {
        static DWORD i = 0; m_index = i++;
        D3DXMatrixIdentity(&m_mLocal);
        ::ZeroMemory(&m_lit, sizeof(m_lit));
        m_pMesh = NULL;
        m_bound._center = D3DXVECTOR3(0, 0, 0);
        m_bound._radius = 0.0f;
    }
    ~CLight(void) {}

    bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
    {
        if (!pDevice) return false;
        if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL))) return false;
        m_bound._center = lit.Position;
        m_bound._radius = radius;
        m_lit = lit;
        return true;
    }

    void destroy(void)
    {
        if (m_pMesh) { m_pMesh->Release(); m_pMesh = NULL; }
    }

    bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
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

    void draw(IDirect3DDevice9* pDevice)
    {
        if (!pDevice) return;
        D3DXMATRIX m; D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
        pDevice->SetTransform(D3DTS_WORLD, &m);
        pDevice->SetMaterial(&d3d::WHITE_MTRL);
        if (m_pMesh) m_pMesh->DrawSubset(0);
    }

    D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
    DWORD               m_index;
    D3DXMATRIX          m_mLocal;
    D3DLIGHT9           m_lit;
    ID3DXMesh*          m_pMesh;
    d3d::BoundingSphere m_bound;
};

// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall   g_legoPlane;
CWall   g_legowall[4];
CSphere g_sphere[4];
CSphere g_target_blueball;
CLight  g_light;

double g_camera_pos[3] = {0.0, 5.0, -8.0};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
void destroyAllLegoBlock(void) {}

bool Setup()
{
    int i;
    D3DXMatrixIdentity(&g_mWorld);
    D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mProj);

    // vertical field plane (width 6, depth 9)
    if (!g_legoPlane.create(Device, -1, -1, 6.0f, 0.03f, 9.0f, d3d::GREEN)) return false;
    g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

    // walls
    if (!g_legowall[0].create(Device, -1, -1, 6.0f, 0.3f, 0.12f, d3d::DARKRED)) return false; // top
    g_legowall[0].setPosition(0.0f, 0.12f, 4.56f);
    if (!g_legowall[1].create(Device, -1, -1, 6.0f, 0.3f, 0.12f, d3d::DARKRED)) return false; // bottom
    g_legowall[1].setPosition(0.0f, 0.12f, -4.56f);
    if (!g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 9.24f, d3d::DARKRED)) return false; // right
    g_legowall[2].setPosition(3.06f, 0.12f, 0.0f);
    if (!g_legowall[3].create(Device, -1, -1, 0.12f, 0.3f, 9.24f, d3d::DARKRED)) return false; // left
    g_legowall[3].setPosition(-3.06f, 0.12f, 0.0f);

    for (i = 0; i < 4; ++i) {
        if (!g_sphere[i].create(Device, sphereColor[i])) return false;
        g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
        g_sphere[i].setPower(0, 0);
    }

    if (!g_target_blueball.create(Device, d3d::BLUE)) return false;
    g_target_blueball.setCenter(0.0f, (float)M_RADIUS, -3.6f);

    // light
    D3DLIGHT9 lit; ::ZeroMemory(&lit, sizeof(lit));
    lit.Type = D3DLIGHT_POINT;
    lit.Diffuse = d3d::WHITE;
    lit.Specular = d3d::WHITE * 0.9f;
    lit.Ambient = d3d::WHITE * 0.9f;
    lit.Position = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
    lit.Range = 100.0f;
    lit.Attenuation0 = 0.0f;
    lit.Attenuation1 = 0.9f;
    lit.Attenuation2 = 0.0f;
    if (!g_light.create(Device, lit)) return false;

    // camera
    D3DXVECTOR3 pos(0.0f, 5.0f, -8.0f);
    D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
    D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
    Device->SetTransform(D3DTS_VIEW, &g_mView);

    D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4, (float)Width / (float)Height, 1.0f, 100.0f);
    Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

    Device->SetRenderState(D3DRS_LIGHTING, TRUE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

    g_light.setLight(Device, g_mWorld);
    return true;
}

void Cleanup(void)
{
    g_legoPlane.destroy();
    for (int i = 0; i < 4; ++i) g_legowall[i].destroy();
    destroyAllLegoBlock();
    g_light.destroy();
}

bool Display(float timeDelta)
{
    if (Device) {
        Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
        Device->BeginScene();

        for (int i = 0; i < 4; ++i) {
            g_sphere[i].ballUpdate(timeDelta);
            for (int j = 0; j < 4; ++j) g_legowall[i].hitBy(g_sphere[j]);
        }

        for (int i = 0; i < 4; ++i)
            for (int j = i + 1; j < 4; ++j)
                g_sphere[i].hitBy(g_sphere[j]);

        g_legoPlane.draw(Device, g_mWorld);
        for (int i = 0; i < 4; ++i) {
            g_legowall[i].draw(Device, g_mWorld);
            g_sphere[i].draw(Device, g_mWorld);
        }
        g_target_blueball.draw(Device, g_mWorld);
        g_light.draw(Device);

        Device->EndScene();
        Device->Present(0, 0, 0, 0);
        Device->SetTexture(0, NULL);
    }
    return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static bool wire = false;
    static bool isReset = true;
    static int old_x = 0;
    static int old_y = 0;
    static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

    switch (msg) {
    case WM_DESTROY:
        ::PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_ESCAPE:
            ::DestroyWindow(hwnd);
            break;
        case VK_RETURN:
            if (Device) {
                wire = !wire;
                Device->SetRenderState(D3DRS_FILLMODE, (wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
            }
            break;
        case VK_SPACE:
            // Launch white ball straight upward (+z)
            g_sphere[3].setPower(0.0, 2.5);
            break;
        }
        break;
    case WM_MOUSEMOVE: {
        int new_x = LOWORD(lParam);
        int new_y = HIWORD(lParam);
        float dx, dy;
        if (LOWORD(wParam) & MK_LBUTTON) {
            if (isReset) {
                isReset = false;
            } else {
                dx = (old_x - new_x) * 0.01f;
                dy = (old_y - new_y) * 0.01f;
                D3DXMATRIX mX, mY;
                D3DXMatrixRotationY(&mX, dx);
                D3DXMatrixRotationX(&mY, dy);
                g_mWorld = g_mWorld * mX * mY;
            }
            old_x = new_x; old_y = new_y;
        } else {
            isReset = true;
            if (LOWORD(wParam) & MK_RBUTTON) {
                dx = (old_x - new_x);
                D3DXVECTOR3 coord = g_target_blueball.getCenter();
                float newX = coord.x + dx * (-0.007f);
                float minX = -3.0f + (float)M_RADIUS;
                float maxX =  3.0f - (float)M_RADIUS;
                if (newX < minX) newX = minX;
                if (newX > maxX) newX = maxX;
                g_target_blueball.setCenter(newX, coord.y, -3.6f);
            }
            old_x = new_x; old_y = new_y;
            move = WORLD_MOVE;
        }
        break;
    }
    }
    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    srand(static_cast<unsigned int>(time(NULL)));
    if (!d3d::InitD3D(hinstance, Width, Height, true, D3DDEVTYPE_HAL, &Device)) {
        ::MessageBox(0, "InitD3D() - FAILED", 0, 0);
        return 0;
    }
    if (!Setup()) {
        ::MessageBox(0, "Setup() - FAILED", 0, 0);
        return 0;
    }
    d3d::EnterMsgLoop(Display);
    Cleanup();
    if (Device) Device->Release();
    return 0;
}