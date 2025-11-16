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
#include <mmsystem.h>

#include "CSphere.h"
#include "CWall.h"
#include "CLight.h"

#pragma comment(lib, "winmm.lib")

IDirect3DDevice9* Device = NULL;

// window size
const int Width  = 1024;
const int Height = 768;

// ------------------------------------------------------------
// Per-level ball speed constants (used for player ball only)
// ------------------------------------------------------------
const float BALL_SPEED_LEVEL1 = 5; // slow
const float BALL_SPEED_LEVEL2 = 6; // medium
const float BALL_SPEED_LEVEL3 = 7; // fast
int ball_speed = BALL_SPEED_LEVEL1; //referenced in CSphere.cpp

//------------------------------------------------------------
// NUMBER OF BRICKS
//------------------------------------------------------------
const int NUM_BRICKS = 47;                   
const int NUM_SPHERES = NUM_BRICKS + 1;      // (bricks + 1 player ball)

//------------------------------------------------------------
// Positions of all spheres (bricks + player ball)
//------------------------------------------------------------

float level1Pos[NUM_SPHERES][2] = {
    // Top
    {-1.2f,  3.6f}, {-0.8f, 3.6f}, {-0.4f, 3.6f}, {0.0f, 3.6f}, {0.4f, 3.6f}, {0.8f, 3.6f}, {1.2f, 3.6f}, 
    {-1.6f, 3.2f}, {1.6f, 3.2f},

    // Side
    {-2.0f, 2.8f}, {-2.0f, 2.4f},{-2.0f, 2.0f},{-2.0f, 1.6f},{-2.0f, 1.2f},{-2.0f, 0.8f},{-2.0f, 0.4f},{-2.0f, 0.0f},{-2.0f, -0.4f},
    {2.0f, 2.8f}, {2.0f, 2.4f},{2.0f, 2.0f},{2.0f, 1.6f},{2.0f, 1.2f},{2.0f, 0.8f},{2.0f, 0.4f},{2.0f, 0.0f},{2.0f, -0.4f},

    // Bottom
    {-1.2f,  -1.2f}, {-0.8f, -1.2f}, {-0.4f, -1.2f}, {0.0f, -1.2f}, {0.4f, -1.2f}, {0.8f, -1.2f}, {1.2f, -1.2f},
    {-1.6f, -0.8f}, {1.6f, -0.8f},

    // Eyes
    {-1.1f, 2.2f}, {-1.1f, 1.8f},
    {1.1f, 2.2f},  {1.1f, 1.8f},

    // Nose
    {0.0f, 1.2f},

    // Smile
    {-1.0f, 0.0f}, {-0.6f, -0.1f}, {-0.2f, -0.15f},
    {0.2f, -0.15f}, {0.6f, -0.1f}, {1.0f, 0.0f},

    { 0.0f, -3.6f }
}; 
float level2Pos[NUM_SPHERES][2] = { 
    // Center
    { 0.0f, 1.2f },

    // Circle 1
    { 0.50f, 1.20f },
    { 0.35f, 1.55f },
    { 0.00f, 1.70f },
    { -0.35f, 1.55f },
    { -0.50f, 1.20f },
    { -0.35f, 0.85f },
    { 0.00f, 0.70f },
    { 0.35f, 0.85f },

    // Circle 2
    { 1.00f, 1.20f },
    { 0.90f, 1.62f },
    { 0.62f, 1.98f },
    { 0.22f, 2.17f },
    { -0.22f, 2.17f },
    { -0.62f, 1.98f },
    { -0.90f, 1.62f },
    { -1.00f, 1.20f },
    { -0.90f, 0.78f },
    { -0.62f, 0.42f },
    { -0.22f, 0.23f },
    { 0.22f, 0.23f },
    { 0.62f, 0.42f },
    { 0.90f, 0.78f },

    // Circle 3
    {  1.586f,  0.991f },
    {  1.586f,  1.409f },
    {  1.478f,  1.812f },
    {  1.269f,  2.174f },
    {  0.974f,  2.469f },
    {  0.612f,  2.678f },
    {  0.209f,  2.786f },
    { -0.209f,  2.786f },
    { -0.612f,  2.678f },
    { -0.974f,  2.469f },
    { -1.269f,  2.174f },
    { -1.478f,  1.812f },
    { -1.586f,  1.409f },
    { -1.586f,  0.991f },
    { -1.478f,  0.588f },
    { -1.269f,  0.226f },
    { -0.974f, -0.069f },
    { -0.612f, -0.278f },
    { -0.209f, -0.386f },
    {  0.209f, -0.386f },
    {  0.612f, -0.278f },
    {  0.974f, -0.069f },
    {  1.269f,  0.226f },
    {  1.478f,  0.588f },

    { 0.0f, -3.6f }
};
float level3Pos[NUM_SPHERES][2] = { 

    {-1.95f, 3.6f}, { -1.45f,  3.6f }, {-0.95f, 3.6f}, {-0.45f, 3.6f}, {0.0f, 3.6f}, {0.45f, 3.6f}, {0.95f, 3.6f}, {1.45f, 3.6f}, {1.95f, 3.6f},
    
    {-1.75f, 3.2f}, {-1.25f, 3.2f}, {-0.75f, 3.2f}, {-0.25f, 3.2f}, {0.25f, 3.2f}, {0.75f, 3.2f}, {1.25f, 3.2f}, {1.75f, 3.2f},

    {-1.45f, 2.8f}, {-0.95f, 2.8f}, {-0.45f, 2.8f}, {0.0f, 2.8f}, {0.45f, 2.8f}, {0.95f, 2.8f}, {1.45f, 2.8f},

    {-1.25f, 2.4f}, {-0.75f, 2.4f}, {-0.25f, 2.4f}, {0.25f, 2.4f}, {0.75f, 2.4f}, {1.25f, 2.4f},

    {-0.95f, 2.0f}, {-0.45f, 2.0f}, {0.0f, 2.0f}, {0.45f, 2.0f}, {0.95f, 2.0f},

    {-0.75f, 1.6f}, {-0.25f, 1.6f}, {0.25f, 1.6f}, {0.75f, 1.6f},

    {-0.45f, 1.2f}, {0.0f, 1.2f}, {0.45f, 1.2f},

    {-0.25f, 0.8f}, {0.25f, 0.8f}, 

    {0.0f, 0.4f}, {0.0f, 0.4f}, {0.0f, 0.4f},

    
    { 0.0f, -3.6f }
};
float (*currentLevelPos)[2] = level1Pos;

//------------------------------------------------------------
// Colors of the spheres
//------------------------------------------------------------

D3DXCOLOR sphereColorLevel1[NUM_SPHERES] = {
    d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED,
    d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED,
    d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED,
    d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW,
    d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW,d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::WHITE
};
D3DXCOLOR sphereColorLevel2[NUM_SPHERES] = {
   d3d::BLUE, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN,
   d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW,
   d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED,
   d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED, d3d::RED,
   d3d::WHITE
}; 
D3DXCOLOR sphereColorLevel3[NUM_SPHERES] = {
   d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN,
   d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN,
   d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN,
   d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN,
   d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::GREEN, d3d::WHITE

};
D3DXCOLOR *sphereColor = sphereColorLevel1;

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
CWall   g_legowall[3];
CSphere g_sphere[NUM_SPHERES];
CSphere g_target_blueball;
CLight  g_light;
double g_camera_pos[3] = {0.0, 5.0, -8.0};

// Game state variables
bool g_brickDestroyed[NUM_BRICKS] = {false};
int  g_activeBricks = NUM_BRICKS;
bool g_ballLaunched = false;
int  g_playerLives = 3;
int  g_lastMilestone = 0;  // Track last milestone for win sound

// Keyboard state for smooth movement
bool g_keyLeft = false;
bool g_keyRight = false;

// Font for rendering text
ID3DXFont* g_pFont = NULL;

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

    // initialize the font for life display on screen (top left)
    D3DXCreateFont(
        Device,
        24,                    // Height
        0,                     // Width
        FW_BOLD,              // Weight
        1,                     // MipLevels
        FALSE,                // Italic
        DEFAULT_CHARSET,      // CharSet
        OUT_DEFAULT_PRECIS,   // OutputPrecision
        DEFAULT_QUALITY,      // Quality
        DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily (everything default basically)
        "Arial",              // police used
        &g_pFont);

    // vertical field plane (width 6, depth 9)
    if (!g_legoPlane.create(Device, -1, -1, 6.0f, 0.03f, 9.0f, d3d::GREEN)) return false;
    g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

    // walls (only top, left, right with no bottom wall)
    if (!g_legowall[0].create(Device, -1, -1, 6.0f, 0.3f, 0.12f, d3d::DARKRED)) return false; // top
    g_legowall[0].setPosition(0.0f, 0.12f, 4.56f);
    if (!g_legowall[1].create(Device, -1, -1, 0.12f, 0.3f, 9.24f, d3d::DARKRED)) return false; // right
    g_legowall[1].setPosition(3.06f, 0.12f, 0.0f);
    if (!g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 9.24f, d3d::DARKRED)) return false; // left
    g_legowall[2].setPosition(-3.06f, 0.12f, 0.0f);

    for (i = 0; i < NUM_SPHERES; ++i)
    {
        if (!g_sphere[i].create(Device, sphereColor[i])) return false;
        g_sphere[i].setCenter(currentLevelPos[i][0], g_sphere[i].getRadius(), currentLevelPos[i][1]);
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
    for (int i = 0; i < 3; ++i) g_legowall[i].destroy();
    destroyAllLegoBlock();
    g_light.destroy();
    if (g_pFont) {
        g_pFont->Release();
        g_pFont = NULL;
    }
}

bool Display(float timeDelta)
{
    if (Device) {
        Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
        Device->BeginScene();

        // Handle continuous paddle movement
        if (g_keyLeft || g_keyRight) {
            D3DXVECTOR3 coord = g_target_blueball.getCenter();
            float moveSpeed = 0.10f; // Adjust this value for speed
            
            if (g_keyLeft) {
                float newX = coord.x - moveSpeed;
                float r = g_target_blueball.getRadius();
                float minX = -3.0f + r;
                if (newX > minX) {
                    g_target_blueball.setCenter(newX, coord.y, -3.6f);
                }
            }
            if (g_keyRight) {
                float newX = coord.x + moveSpeed;
                float r = g_target_blueball.getRadius();
                float maxX = 3.0f - r;
                if (newX < maxX) {
                    g_target_blueball.setCenter(newX, coord.y, -3.6f);
                }
            }
        }

        // Update ball physics only if launched
        if (g_ballLaunched) {
            g_sphere[NUM_BRICKS].ballUpdate(timeDelta); // last sphere = player ball

            // walls
            for (int j = 0; j < 3; ++j)
                g_legowall[j].hitBy(g_sphere[NUM_BRICKS]);

            // paddle
            if (g_target_blueball.hasIntersected(g_sphere[NUM_BRICKS])) {
                g_target_blueball.paddleHitBy(g_sphere[NUM_BRICKS]);
                // Play sound when paddle hits ball
                PlaySound(TEXT("hit.wav"), NULL, SND_FILENAME | SND_ASYNC);
            }

            // bricks collision
            for (int i = 0; i < NUM_BRICKS; ++i)
            {
                if (!g_brickDestroyed[i] && g_sphere[i].hasIntersected(g_sphere[NUM_BRICKS]))
                {
                    g_sphere[i].hitBy(g_sphere[NUM_BRICKS]);
                    g_brickDestroyed[i] = true;
                    g_activeBricks--;
                    
                    // Check if reached milestone (every 10 destroyed)
                    int destroyed = NUM_BRICKS - g_activeBricks;
                    int currentMilestone = destroyed / 10;
                    if (currentMilestone > g_lastMilestone) {
                        g_lastMilestone = currentMilestone;
                        PlaySound(TEXT("win.wav"), NULL, SND_FILENAME | SND_ASYNC);
                    }
                }
            }

            // ball fell bottom == lose a life
            D3DXVECTOR3 ballPos = g_sphere[NUM_BRICKS].getCenter();
            if (ballPos.z < -4.8f)  // threshold for losing ball
            {
                g_playerLives--;  // Lose a life

                if (g_playerLives <= 0) {
                    // game over == return to level selection
                    PostQuitMessage(0);
                }
                else {
                    // reset ball position, back to base point
                    D3DXVECTOR3 p = g_target_blueball.getCenter();
                    g_sphere[NUM_BRICKS].setCenter(p.x, g_sphere[NUM_BRICKS].getRadius(), p.z + 0.5f);
                    g_sphere[NUM_BRICKS].setPower(0, 0);
                    g_ballLaunched = false;
                }
            }

            // all bricks destroyed → respawn
            if (g_activeBricks == 0)
            {
                for (int i = 0; i < NUM_BRICKS; ++i)
                {
                    g_brickDestroyed[i] = false;
                    g_sphere[i].setCenter(currentLevelPos[i][0], g_sphere[i].getRadius(), currentLevelPos[i][1]);
                    g_sphere[i].setPower(0, 0);
                }
                g_activeBricks = NUM_BRICKS;
                g_lastMilestone = 0;  // Reset milestone counter

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
        for (int i = 0; i < 3; ++i)
            g_legowall[i].draw(Device, g_mWorld);

        // draw bricks
        for (int i = 0; i < NUM_BRICKS; ++i)
            if (!g_brickDestroyed[i])
                g_sphere[i].draw(Device, g_mWorld);

        // draw player ball + paddle
        g_sphere[NUM_BRICKS].draw(Device, g_mWorld);
        g_target_blueball.draw(Device, g_mWorld);
        g_light.draw(Device);

        // draw life count text
        if (g_pFont) {
            RECT rect;
            SetRect(&rect, 10, 10, Width, Height);
            char lifeText[32];
            sprintf_s(lifeText, "Lives: %d", g_playerLives);
            g_pFont->DrawTextA(NULL, lifeText, -1, &rect, DT_LEFT | DT_TOP, D3DCOLOR_XRGB(255, 255, 255));
            
            // draw score text on the right side
            RECT scoreRect;
            SetRect(&scoreRect, 0, 10, Width - 20, Height);
            char scoreText[64];
            int destroyed = NUM_BRICKS - g_activeBricks;
            sprintf_s(scoreText, "Destroyed: %d/%d", destroyed, NUM_BRICKS);
            g_pFont->DrawTextA(NULL, scoreText, -1, &scoreRect, DT_RIGHT | DT_TOP, D3DCOLOR_XRGB(255, 255, 255));
        }

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

                g_sphere[NUM_BRICKS].setPower(horizontalPower, (float)ball_speed); // use global speed
                g_ballLaunched = true;
            }
            else {
                g_target_blueball.greenTime(g_sphere[NUM_BRICKS]);
            }
            break;
        case 'R':
            //Reset ball position, layout, score, and lives
        {
            // Reset all bricks to initial positions
            for (int i = 0; i < NUM_BRICKS; ++i)
            {
                g_brickDestroyed[i] = false;
                g_sphere[i].setCenter(currentLevelPos[i][0], g_sphere[i].getRadius(), currentLevelPos[i][1]);
                g_sphere[i].setPower(0, 0);
            }
            g_activeBricks = NUM_BRICKS;
            g_lastMilestone = 0;  // Reset milestone counter

            // Reset player lives
            g_playerLives = 3;

            // Reset ball position to paddle
            D3DXVECTOR3 p = g_target_blueball.getCenter();
            g_sphere[NUM_BRICKS].setCenter(p.x, g_sphere[NUM_BRICKS].getRadius(), p.z + 0.5f);
            g_sphere[NUM_BRICKS].setPower(0, 0);
            g_ballLaunched = false;
        }
        break;
        case VK_LEFT:
        case 'A':
            g_keyLeft = true;
            break;
        case VK_RIGHT:
        case 'D':
            g_keyRight = true;
            break;
        }
        break;
    case WM_KEYUP:
        switch (wParam) {
        case VK_LEFT:
        case 'A':
            g_keyLeft = false;
            break;
        case VK_RIGHT:
        case 'D':
            g_keyRight = false;
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

    int level = 1;
    int answer = MessageBox(
        0,
        "Choose a level:\n\nYes = Level 1\nNo = Level 2\nCancel = Level 3",
        "Select Difficulty",
        MB_YESNOCANCEL | MB_ICONQUESTION
    );

    switch (answer)
    {
    case IDYES: level = 1; break;
    case IDNO: level = 2; break;
    case IDCANCEL: level = 3; break;
    default: level = 1; break;
    }
    switch (level) {
    case 1: currentLevelPos = level1Pos; sphereColor = sphereColorLevel1; ball_speed = BALL_SPEED_LEVEL1; break;
    case 2: currentLevelPos = level2Pos; sphereColor = sphereColorLevel2; ball_speed = BALL_SPEED_LEVEL2; break;
    case 3: currentLevelPos = level3Pos; sphereColor = sphereColorLevel3; ball_speed = BALL_SPEED_LEVEL3; break;
    }

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