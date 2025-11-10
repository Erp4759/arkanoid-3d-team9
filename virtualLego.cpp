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

#include "CSphere.h"
#include "CWall.h"
#include "CLight.h"

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
        g_sphere[i].setCenter(spherePos[i][0], g_sphere[i].getRadius(), spherePos[i][1]);
        g_sphere[i].setPower(0, 0);
    }

    if (!g_target_blueball.create(Device, d3d::BLUE)) return false;
    g_target_blueball.setCenter(0.0f, g_target_blueball.getRadius(), -3.6f);

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
                float r = g_target_blueball.getRadius();
                float minX = -3.0f + r;
                float maxX =  3.0f - r;
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