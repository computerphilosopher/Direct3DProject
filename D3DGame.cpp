#include <windows.h>
#include "d3dx9.h"

#include "XFileUtil.h"
#include "Camera.h"
#include "terrain.h"
#include "PhysicalObject.h"

VOID SetupViewProjection();
VOID SetupLight();
//-----------------------------------------------------------------------------
// ���� ���� 
ZCamera*				g_pCamera = NULL;	// Camera Ŭ����
Terrain*				g_pTerrain = NULL;	// Terrain
CXFileUtil              g_XFile;			// X ���� ����� ���� Ŭ���� ��ü 
PhysicalObj				g_Tiger;

BOOL					g_bWoodTexture = TRUE;
LPDIRECT3D9             g_pD3D = NULL; // Direct3D ��ü 
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // ������ ��ġ (����ī��)

LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // ���ؽ� ���� 
LPDIRECT3DVERTEXBUFFER9 g_pVBLight = NULL; // ����Ʈ�� ���ؽ� ���� 

PDIRECT3DVERTEXBUFFER9  g_pVBTexture = NULL; // �ؽ��� ��¿���ؽ� ����
LPDIRECT3DTEXTURE9      g_pTexture = NULL; // �ؽ��� �ε��� ����


// Ŀ���� ���ؽ� Ÿ�� ����ü 
struct CUSTOMVERTEX
{
	FLOAT x, y, z;    // 3D ��ǥ��
	DWORD color;      // ���ؽ� ����
};

// Ŀ���� ���ؽ��� ������ ǥ���ϱ� ���� FVF(Flexible Vertex Format) �� 
// D3DFVF_XYZ(3D ���� ��ǥ) ��  D3DFVF_DIFFUSE(���� ����) Ư���� ��������.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

// ���� ó���� ���� ���ؽ� ����ü�� ���� ������� �ʴ´�
struct LIGHTVERTEX {
	D3DXVECTOR3 position;    // 3D ��ǥ ����ü 
	D3DXVECTOR3 normal;     // ���ؽ� �븻
};

// ���ؽ� ������ �����ϴ� FVF ����
#define D3DFVF_LIGHTVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL)

// �ؽ��� ��ǥ�� ������ ���ؽ� ����ü ����
struct TEXTUREVERTEX
{
	D3DXVECTOR3     position;  // ���ؽ��� ��ġ
	D3DCOLOR        color;     // ���ؽ��� ����
	FLOAT           tu, tv;    // �ؽ��� ��ǥ 
};

// �� ����ü�� ������ ǥ���ϴ� FVF �� ����
#define D3DFVF_TEXTUREVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)



//-----------------------------------------------------------------------------
// �̸�: InitD3D()
// ���: Direct3D �ʱ�ȭ, ���� �� �⺻ ���º��� �ʱ�ȭ 
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	// Direct3D ��ü ���� 
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// ��ġ ������ ����Ÿ �غ�

	D3DPRESENT_PARAMETERS d3dpp;         // ��ġ ������ ���� ����ü ���� ����

	ZeroMemory(&d3dpp, sizeof(d3dpp));                  // ����ü Ŭ����
	d3dpp.BackBufferWidth = 1024;               // ���� �ػ� ���� ����
	d3dpp.BackBufferHeight = 800;               // ���� �ػ� ���� ���� 
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;   // ���� ���� ���� 
	d3dpp.BackBufferCount = 1;                 // ����� �� 
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;  // ���� ��� ����
	d3dpp.hDeviceWindow = hWnd;              // ������ �ڵ� ���� 
	d3dpp.Windowed = true;              // ������ ���� ���� �ǵ��� �� 
	d3dpp.EnableAutoDepthStencil = true;              // ���Ľ� ���۸� ����ϵ��� �� 
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;      // ���Ľ� ���� ���� ���� 


	// D3D��ü�� ��ġ ���� �Լ� ȣ�� (����Ʈ ����ī�� ���, HAL ���,
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice)))
	{
		return E_FAIL;
	}
	// ���� ��ġ�� ���������� �����Ǿ���.

	// zbuffer ����ϵ��� ����
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	// �ﰢ���� ��/�� ���� ��� �������ϵ��� �ø� ����� ����.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// �� �� �������� ��ȯ ����
	SetupViewProjection();
	SetupLight();

	return S_OK;
}


//-----------------------------------------------------------------------------
// �̸�: InitGameData()
// ���: ���ӿ� ���õ� ���� �����͸� �ʱ�ȭ �Ѵ�. 
//-----------------------------------------------------------------------------
HRESULT InitData()
{
	g_XFile.XFileLoad(g_pd3dDevice, "./images/tiger.x");

	// bounding information
	D3DXVECTOR3 min, max;
	D3DXVECTOR3 center;
	float radius;

	BYTE* v = 0;
	g_XFile.GetMesh()->LockVertexBuffer(0, (void**)&v);
	D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		g_XFile.GetMesh()->GetNumVertices(),
		D3DXGetFVFVertexSize(g_XFile.GetMesh()->GetFVF()),
		&min,
		&max);
	g_XFile.GetMesh()->UnlockVertexBuffer();

	D3DXComputeBoundingSphere(
		(D3DXVECTOR3*)v,
		g_XFile.GetMesh()->GetNumVertices(),
		D3DXGetFVFVertexSize(g_XFile.GetMesh()->GetFVF()),
		&center,
		&radius);
	g_XFile.GetMesh()->UnlockVertexBuffer();

	g_Tiger.SetBoundingBox(min, max);
	g_Tiger.SetBoundingSphere(center, radius);


	// ���� ������
	D3DXVECTOR3 directionToLight(0, 1, 0);
	g_pTerrain = new Terrain(g_pd3dDevice, "./images/coastMountain64.raw", 64, 64, 6, 0.4f);
	//	g_pTerrain = new Terrain(g_pd3dDevice, "./images/height64.bmp", 10, 0.4f);
	//	g_pTerrain->loadTexture("./images/desert.bmp");
	//	g_pTerrain->loadTexture("./images/tile.tga");
	g_pTerrain->genTexture(&directionToLight);

	g_Tiger.SetPosition(0, 0, 100.0f);
	g_Tiger.SetVelocity(0, 0, -0.1f);
	g_Tiger.SetAcceleration(0, -0.2f, 0);
	g_Tiger.SetScale(20); // ȣ������ ũ�� ����

	return S_OK;
}




//-----------------------------------------------------------------------------
// �̸�: InitGeometryTexture()
// ���: �ؽ��� ����� ���� ���ؽ� ���۸� ������ �� ���ؽ��� ä���. 
//-----------------------------------------------------------------------------
HRESULT InitGeometryTexture()
{
	// �ؽ��� �ε� 
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, "./Images/tree35s.dds", &g_pTexture)))
		//"./Images/tree35s.dds"
		//"./Images/tree1.bmp"
		return E_FAIL;


	// ���ؽ� ���� ���� 
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(4 * sizeof(TEXTUREVERTEX), 0,
		D3DFVF_TEXTUREVERTEX, D3DPOOL_DEFAULT, &g_pVBTexture, NULL)))
	{
		return E_FAIL;
	}

	float altitude = g_pTerrain->getHeight(0, 0);

	// ������ ���ؽ� ���� ���� 
	TEXTUREVERTEX* pVertices;
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0)))
		return E_FAIL;
	pVertices[0].position = D3DXVECTOR3(-50, 100 + altitude, 0);  // ���ؽ� ��ġ 
	pVertices[0].color = 0xffffffff;                  // ���ؽ� ���� �� ���� 
	pVertices[0].tu = 0.0f;                        // ���ؽ� U �ؽ��� ��ǥ 
	pVertices[0].tv = 0.0f;                        // ���ؽ� V �ؽ��� ��ǥ 

	pVertices[1].position = D3DXVECTOR3(50, 100 + altitude, 0);
	pVertices[1].color = 0xffffffff;
	pVertices[1].tu = 1;
	pVertices[1].tv = 0.0f;

	pVertices[2].position = D3DXVECTOR3(-50, 0 + altitude, 0);
	pVertices[2].color = 0xffffffff;
	pVertices[2].tu = 0.0f;
	pVertices[2].tv = 1.0f;

	pVertices[3].position = D3DXVECTOR3(50, 0 + altitude, 0);
	pVertices[3].color = 0xffffffff;
	pVertices[3].tu = 1;
	pVertices[3].tv = 1.0f;

	g_pVBTexture->Unlock();

	return S_OK;
}


//-----------------------------------------------------------------------------
// �̸�: InitGeometry()
// ���: ���������� �ʱ�ȭ�Ѵ�.
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
	// ���ؽ� ���ۿ� ���� ���ؽ� �ڷḦ �ӽ÷� �����. 
	CUSTOMVERTEX vertices[] =
	{
		{ -200.0f,  0.0f, 0.0f, 0xff00ff00, }, // x�� ������ ���� ���ؽ� 
		{ 200.0f,  0.0f, 0.0f, 0xff00ff00, },

		{ 0.0f, 0.0f, -200.0f, 0xffffff00, },  // z�� ������ ���� ���ؽ�
		{ 0.0f, 0.0f,  200.0f, 0xffffff00, },

		{ 0.0f, -200.0f, 0.0f, 0xffff0000, },  // y�� ������ ���� ���ؽ�
		{ 0.0f,  200.0f, 0.0f, 0xffff0000, },

		{ 0.0f, 50.0f, 0.0f, 0xffff0000, },  // �ﰢ���� ù ��° ���ؽ� 
		{ -50.0f,  0.0f, 0.0f, 0xffff0000, },  // �ﰢ���� �� ��° ���ؽ� 
		{ 50.0f,  0.0f, 0.0f, 0xffff0000, },  // �ﰡ���� �� ��° ���ؽ� 
	};

	// ���ؽ� ���۸� �����Ѵ�.
	// �� ���ؽ��� ������ D3DFVF_CUSTOMVERTEX ��� �͵� ���� 
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(9 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	// ���ؽ� ���ۿ� ���� �� �� ���ؽ��� �ִ´�. 
	VOID* pVertices;
	if (FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

	if (FAILED(InitGeometryTexture()))
		return E_FAIL;

	return S_OK;
}


//-----------------------------------------------------------------------------
// �̸�: Cleanup()
// ���: �ʱ�ȭ�Ǿ��� ��� ��ü���� �����Ѵ�. 
//-----------------------------------------------------------------------------
VOID Cleanup()
{
	if (g_pVB != NULL)
		g_pVB->Release();
	if (g_pVBLight != NULL)
		g_pVBLight->Release();
	if (g_pVBTexture != NULL)
		g_pVBTexture->Release();
	if (g_pTexture != NULL)
		g_pTexture->Release();

	if (g_pd3dDevice != NULL)    // ��ġ ��ü ���� 
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)          // D3D ��ü ���� 
		g_pD3D->Release();
}



//-----------------------------------------------------------------------------
// �̸�: SetupViewProjection()
// ���: �� ��ȯ�� �������� ��ȯ�� �����Ѵ�. 
//-----------------------------------------------------------------------------
VOID SetupViewProjection()
{
	// �� ��ȯ ���� 
	D3DXVECTOR3 vEyePt(100.0f, 100.0f, -200.0f);    // ī�޶��� ��ġ 
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);       // �ٶ󺸴� ���� 
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);          // ������ ���� 
	D3DXMATRIXA16 matView;                           // �亯ȯ�� ��Ʈ���� 
													 // �� ��Ʈ���� ���� 
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	// Direct3D ��ġ�� �� ��Ʈ���� ���� 
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	// �������� ��ȯ ���� 
	D3DXMATRIXA16 matProj;   // �������ǿ� ��Ʈ���� 
							 // �������� ��Ʈ���� ���� 
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 5000.0f);
	// Direct3D ��ġ�� �������� ��Ʈ���� ���� 
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	/// ī�޶� �ʱ�ȭ
	g_pCamera->SetView(&vEyePt, &vLookatPt, &vUpVec);
}

// ������ �̸� ���� ������ ���ϴ�.
D3DCOLORVALUE black = { 0, 0, 0, 1 };
D3DCOLORVALUE dark_gray = { 0.2f, 0.2f, 0.2f, 1.0f };
D3DCOLORVALUE gray = { 0.5f, 0.5f, 0.5f, 1.0f };
D3DCOLORVALUE red = { 1.0f, 0.0f, 0.0f, 1.0f };
D3DCOLORVALUE white = { 1.0f, 1.0f, 1.0f, 1.0f };
VOID SetupLight()
{
	D3DLIGHT9 light;                         // Direct3D 9 ���� ����ü ���� ����

	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;       // ���� Ÿ���� �𷺼ųη� ����
	light.Diffuse = white;                   // ������ �� ����
	light.Specular = white;
	light.Direction = D3DXVECTOR3(10, -10, 10);       //  ������ ���� (�����ϴ� ����) 
	//light.Direction = D3DXVECTOR3(20*sin(g_counter*0.01f), -10, 10);       //  ������ ���� (�����ϴ� ����) 
	//light.Direction = D3DXVECTOR3(10, 25, -40);       //  ������ ���� (�����ϴ� ����) 
	g_pd3dDevice->SetLight(0, &light);      // ����Ʈ ��ȣ ���� (���⿡���� 0��)
	g_pd3dDevice->LightEnable(0, TRUE);     // 0�� ����Ʈ �ѱ�


	// ����Ʈ ��� ����� TRUE�� ��. (�� ����� ���� ��� ����Ʈ ����� ������)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	// ���������� �����Ʈ ����Ʈ �ѱ� (ȯ�汤�� ���� ����)
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00303030);
}



void DrawAxis()
{
	///// ���ؽ� ��� 
	// ���ؽ� ���� ���� 
	g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
	g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX); // ���ؽ� ���� ���� 

	D3DXMATRIXA16 matWorld;  // ���� ��ȯ�� ��Ʈ���� ���� 

	for (float x = -200; x <= 200; x += 20) {  // z �࿡ ������ ������ ���� �� �׸��� 
		D3DXMatrixTranslation(&matWorld, x, 0.0, 0.0);  // x�࿡ ���� ��ġ �̵� ��Ʈ����   
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld); // ��ȯ��Ʈ���� ���� 
		g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 2, 1);  // z�� ���� �׸��� 
	}

	for (float z = -200; z <= 200; z += 20) {  // x �࿡ ������ ������ ���� �� �׸��� 
		D3DXMatrixTranslation(&matWorld, 0.0, 0.0, z);  // z �࿡ ���� ��ġ �̵� ��Ʈ����
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);  // ��ȯ��Ʈ���� ���� 
		g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);   // x�� ���� �׸��� 
	}

	D3DXMatrixIdentity(&matWorld);   // ��Ʈ������ ���� ��ķ� ���� 
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);  // ��ȯ ��Ʈ���� ���� 
	g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 4, 1);   // y �� �׸��� 
}


void DrawTextureRectangle()
{
	// ���� ����
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// �ؽ��� ���� (�ؽ��� ������ ���Ͽ� g_pTexture�� ����Ͽ���.)
	g_pd3dDevice->SetTexture(0, g_pTexture);

	// �ؽ��� ��� ȯ�� ����
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	//g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0X08);
	//g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	// ����� ���ؽ� ���� ����
	g_pd3dDevice->SetStreamSource(0, g_pVBTexture, 0, sizeof(TEXTUREVERTEX));
	// FVF �� ����
	g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);
	// �簢�� ���� (�ﰢ�� 2���� �̿��Ͽ� �簢�� ������ �������) ��� 
	g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

	// �ؽ��� ���� ����
	g_pd3dDevice->SetTexture(0, NULL);

	// �������� ��� alpha blending�� ������� �ʴ´�.
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}
//-----------------------------------------------------------------------------
// �̸�: Render()
// ���: ȭ���� �׸���.
//-----------------------------------------------------------------------------
VOID Render()
{
	if (NULL == g_pd3dDevice)  // ��ġ ��ü�� �������� �ʾ����� ���� 
		return;

	// ����۸� ������ �������� �����.
	// ����۸� Ŭ����
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

										  // ȭ�� �׸��� ���� 
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		D3DXMATRIXA16 matWorld;  // ���� ��ȯ�� ��Ʈ���� ���� 

		// �� ����� �ʿ� ����.
		//DrawAxis();

		// ���� ���
		D3DXMatrixScaling(&matWorld, 1.0f, 1.0f, 1.0f);
		g_pTerrain->draw(&matWorld, FALSE);	// TRUE for edge

		// X ���� ��� (tiger)
		// ���� Ȱ��ȭ
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
		matWorld = g_Tiger.GetWorldMatrix();
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile.XFileDisplay(g_pd3dDevice);


		D3DXMatrixScaling(&matWorld, 1.0f, 1.0f, 1.0f);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

		//// �ؽ��� (����) ���
		DrawTextureRectangle();


		// ȭ�� �׸��� �� 
		g_pd3dDevice->EndScene();
	}

	// ������� ������ ȭ������ ������. 
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}



//-----------------------------------------------------------------------------
// �̸� : MsgProc()
// ��� : ������ �޽��� �ڵ鷯 
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		Cleanup();   // ���α׷� ����� ��ü ������ ���Ͽ� ȣ���� 
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		Render();    // ȭ�� ����� ����ϴ� ������ �Լ� ȣ�� 
		ValidateRect(hWnd, NULL);
		return 0;
	case WM_CHAR:
		break;

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

/**-----------------------------------------------------------------------------
* Ű���� �Է� ó��
*------------------------------------------------------------------------------
*/
void ProcessKey(void)
{
	if (GetAsyncKeyState(VK_UP)) g_pCamera->MoveLocalZ(1.5f);	// ī�޶� ����!
	if (GetAsyncKeyState(VK_DOWN)) g_pCamera->MoveLocalZ(-1.5f);	// ī�޶� ����!
	if (GetAsyncKeyState(VK_LEFT)) g_pCamera->MoveLocalX(-1.5f);	// ī�޶� ����
	if (GetAsyncKeyState(VK_RIGHT)) g_pCamera->MoveLocalX(1.5f);	// ī�޶� ������

	if (GetAsyncKeyState('A')) g_pCamera->RotateLocalY(-.02f);
	if (GetAsyncKeyState('D')) g_pCamera->RotateLocalY(.02f);
	if (GetAsyncKeyState('W')) g_pCamera->RotateLocalX(-.02f);
	if (GetAsyncKeyState('S')) g_pCamera->RotateLocalX(.02f);

	D3DXMATRIXA16*	pmatView = g_pCamera->GetViewMatrix();		// ī�޶� ����� ��´�.
	g_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView);			// ī�޶� ��� ����

	if (GetAsyncKeyState(' '))
		g_Tiger.AddVelocity(0, 0.5f, 0);
}

void Moving()
{
	ProcessKey();

	float y = g_pTerrain->getHeight(g_Tiger.GetPosition().x, g_Tiger.GetPosition().z);
	g_Tiger.SetGround(y);
	g_Tiger.Move();

}

void Action()
{
	Moving();
}

//-----------------------------------------------------------------------------
// �̸�: WinMain()
// ���: ���α׷��� ������ 
//-----------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// ������ Ŭ���� ���� ���� �� ���� 
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		"D3D Game", NULL };
	// ������ Ŭ���� ��� 
	RegisterClassEx(&wc);

	// ������ ���� 
	HWND hWnd = CreateWindow("D3D Game", "D3D Game Program",
		WS_OVERLAPPEDWINDOW, 100, 100, 1024, 768,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);
	g_pCamera = new ZCamera;

	if (!SUCCEEDED(InitD3D(hWnd))) goto END;
	if (!SUCCEEDED(InitData())) goto END;
	if (!SUCCEEDED(InitGeometry())) goto END;


	// ������ ��� 
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	// �޽��� ���� �����ϱ�
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		// �޽��ڰ� ������ ���� �´�. 
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			Action();
			InvalidateRect(hWnd, NULL, TRUE);
		}
	}

END:
	delete g_pCamera;
	delete g_pTerrain;
	UnregisterClass("D3D Game", wc.hInstance);
	return 0;
}
