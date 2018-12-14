// PhysicalObj.h: interface for the PhysicalObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHYSICALOBJ_H__49719AF0_6560_45FB_A928_78DD371377F2__INCLUDED_)
#define AFX_PHYSICALOBJ_H__49719AF0_6560_45FB_A928_78DD371377F2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "d3dx9.h"

class PhysicalObj  
{
private:
	D3DXVECTOR3 p, v, a;
	float clock;
	float fGround;
	float scale;

	// bounding box/sphere
	D3DXVECTOR3 min, max;
	D3DXVECTOR3 center;
	float radius;

public:
	PhysicalObj();
	D3DXVECTOR3 GetPosition() { return p; }
	void SetScale(float s);
	void SetGround(float ground);
	void SetPosition(float x, float y, float z);
	void SetVelocity(float x, float y, float z);
	void SetAcceleration(float x, float y, float z);
	void AddVelocity(float x, float y, float z);
	void Move(float current=-1);
	void BoundCheck();
	void Collision(PhysicalObj *target);
	void SetBoundingBox(D3DXVECTOR3 m, D3DXVECTOR3 M);
	void SetBoundingSphere(D3DXVECTOR3 c, float r);

	D3DXMATRIXA16 GetWorldMatrix();
	virtual ~PhysicalObj();

};

#endif // !defined(AFX_PHYSICALOBJ_H__49719AF0_6560_45FB_A928_78DD371377F2__INCLUDED_)
