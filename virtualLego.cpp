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
#include <iostream>
#include "CSphere.h"
#include "CWall.h"
#include "CLight.h"

IDirect3DDevice9* Device = NULL;

// window size
const int Width  = 1024/2;
const int Height = 768/2;

//------------------------------------------------------------
// CONFIGURABLE NUMBER OF BRICKS
//------------------------------------------------------------
const int NUM_BRICKS = 6;                   
const int NUM_SPHERES = NUM_BRICKS + 1;      // (bricks + 1 player ball)

//------------------------------------------------------------
// Positions of all spheres (bricks + player ball)
//------------------------------------------------------------
const float spherePos[NUM_SPHERES][2] =
{
    {-2.0f, 3.6f},
    { 0.0f, 3.6f},
    { 2.0f, 3.6f},
    {-2.0f, 2.8f},
    { 0.0f, 2.8f},
    { 2.0f, 2.8f},

    { 0.0f, -3.6f }   // player ball
};

//------------------------------------------------------------
// Colors of the spheres
//------------------------------------------------------------
const D3DXCOLOR sphereColor[NUM_SPHERES] =
{
    d3d::RED,
    d3d::RED,
    d3d::YELLOW,
    d3d::BLUE,
    d3d::GREEN,
    d3d::BLUE,

    d3d::WHITE      // player ball
};

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
CSphere g_sphere[NUM_SPHERES];
CSphere g_target_blueball;
CLight  g_light;
int ball_speed = 2;
double g_camera_pos[3] = {0.0, 5.0, -8.0};

// Game state variables
bool g_brickDestroyed[NUM_BRICKS] = {false};
int  g_activeBricks = NUM_BRICKS;
bool g_ballLaunched = false;

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

    for (i = 0; i < NUM_SPHERES; ++i)
    {
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

        // Update ball physics only if launched
        if (g_ballLaunched) {
            g_sphere[NUM_BRICKS].ballUpdate(timeDelta); // last sphere = player ball

            // walls
            for (int j = 0; j < 4; ++j)
                g_legowall[j].hitBy(g_sphere[NUM_BRICKS]);

            // paddle
            if (g_target_blueball.hasIntersected(g_sphere[NUM_BRICKS]))
                g_target_blueball.hitBy(g_sphere[NUM_BRICKS]);

            // bricks collision
            for (int i = 0; i < NUM_BRICKS; ++i)
            {
                if (!g_brickDestroyed[i] && g_sphere[i].hasIntersected(g_sphere[NUM_BRICKS]))
                {
                    g_sphere[i].hitBy(g_sphere[NUM_BRICKS]);
                    g_brickDestroyed[i] = true;
                    g_activeBricks--;
                    OutputDebugStringA("Destroyed brick\n");
                }
            }

            // ball fell bottom
            D3DXVECTOR3 ballPos = g_sphere[NUM_BRICKS].getCenter();
            if (ballPos.z < -4.2)
            {
                OutputDebugStringA("OUT\n");
                D3DXVECTOR3 p = g_target_blueball.getCenter();
                g_sphere[NUM_BRICKS].setCenter(p.x, g_sphere[NUM_BRICKS].getRadius(), p.z + 0.5f);
                g_sphere[NUM_BRICKS].setPower(0, 0);
                g_ballLaunched = false;
            }

            // all bricks destroyed → respawn
            if (g_activeBricks == 0)
            {
                for (int i = 0; i < NUM_BRICKS; ++i)
                {
                    g_brickDestroyed[i] = false;
                    g_sphere[i].setCenter(spherePos[i][0], g_sphere[i].getRadius(), spherePos[i][1]);
                    g_sphere[i].setPower(0, 0);
                }
                g_activeBricks = NUM_BRICKS;

                D3DXVECTOR3 p = g_target_blueball.getCenter();
                g_sphere[NUM_BRICKS].setCenter(p.x, g_sphere[NUM_BRICKS].getRadius(), p.z + 0.5f);
                g_sphere[NUM_BRICKS].setPower(0, 0);
                g_ballLaunched = false;
            }
        }
        else
        {
            // ball sticks to paddle
            D3DXVECTOR3 p = g_target_blueball.getCenter();
            g_sphere[NUM_BRICKS].setCenter(p.x, g_sphere[NUM_BRICKS].getRadius(), p.z + 0.5f);
        }

        // draw world
        g_legoPlane.draw(Device, g_mWorld);
        for (int i = 0; i < 4; ++i)
            g_legowall[i].draw(Device, g_mWorld);

        // draw bricks
        for (int i = 0; i < NUM_BRICKS; ++i)
            if (!g_brickDestroyed[i])
                g_sphere[i].draw(Device, g_mWorld);

        // draw player ball + paddle
        g_sphere[NUM_BRICKS].draw(Device, g_mWorld);
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
            // Launch white ball with angle based on paddle position
            if (!g_ballLaunched) {
                D3DXVECTOR3 paddlePos = g_target_blueball.getCenter();
                D3DXVECTOR3 ballPos = g_sphere[NUM_BRICKS].getCenter();
                
                // Calculate launch angle based on where ball is on paddle
                float offset = ballPos.x - paddlePos.x;
                float horizontalPower = offset * 1.0f; // Adjust multiplier for more/less angle
                
                g_sphere[NUM_BRICKS].setPower(horizontalPower, ball_speed);
                g_ballLaunched = true;
            }
            break;
        case VK_LEFT:
        case 'A':
            {
                D3DXVECTOR3 coord = g_target_blueball.getCenter();
                float newX = coord.x - 0.15f;
                float r = g_target_blueball.getRadius();
                float minX = -3.0f + r;
                if (newX > minX) {
                    g_target_blueball.setCenter(newX, coord.y, -3.6f);
                }
            }
            break;
        case VK_RIGHT:
        case 'D':
            {
                D3DXVECTOR3 coord = g_target_blueball.getCenter();
                float newX = coord.x + 0.15f;
                float r = g_target_blueball.getRadius();
                float maxX = 3.0f - r;
                if (newX < maxX) {
                    g_target_blueball.setCenter(newX, coord.y, -3.6f);
                }
            }
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