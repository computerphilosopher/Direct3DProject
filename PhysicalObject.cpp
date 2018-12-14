// PhysicalObj.cpp: implementation of the PhysicalObj class.
//
//////////////////////////////////////////////////////////////////////

#include "PhysicalObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


PhysicalObj::PhysicalObj()
{
	p = v = a = D3DXVECTOR3(0,0,0);
	clock = 0;
	scale = 50;
	fGround = 0;
	min = max = D3DXVECTOR3(0,0,0);
	center = D3DXVECTOR3(0,0,0);
	radius = 0;
}

PhysicalObj::~PhysicalObj()
{

}

void PhysicalObj::SetScale(float s)
{
	scale = s;
}

void PhysicalObj::SetGround(float ground)
{
	fGround = ground;// + scale;
}

void PhysicalObj::SetPosition(float x, float y, float z)
{
	p.x = x;
	p.y = y;
	p.z = z;
}
void PhysicalObj::SetVelocity(float x, float y, float z)
{
	v.x = x;
	v.y = y;
	v.z = z;
}
void PhysicalObj::AddVelocity(float x, float y, float z)
{
	v.x += x;
	v.y += y;
	v.z += z;
}

void PhysicalObj::SetAcceleration(float x, float y, float z)
{
	a.x = x;
	a.y = y;
	a.z = z;
}

void PhysicalObj::BoundCheck()
{
	float e = 0.5f;

	if(p.y + min.y*scale< fGround ) {
		if( fabs(v.y) < 1.0) { // stop condition
			p.y = -min.y*scale + fGround;
			v.y = 0;
		} else {
			v.y = (float)fabs(v.y) * e;
		}
	}
	if(p.x + min.x*scale < -200) {
		p.x = -200 - min.x*scale;
		v.x = (float)fabs(v.x) * e;
	}
	if(p.x + max.x*scale > 200) {
		p.x = 200 - max.x*scale;
		v.x = (float)-fabs(v.x) * e;
	}
	if(p.z + min.z*scale < -200) {
		p.z = -200 - min.z*scale;
		v.z = (float)fabs(v.z) * e;
	}
	if(p.z + max.z*scale > 200) {
		p.z = 200 - max.z*scale;
		v.z = (float)-fabs(v.z) * e;
	}

}

void PhysicalObj::Move(float current)
{
	if(current == -1) { // defafult

		p.x += v.x + 0.5f*a.x;
		p.y += v.y + 0.5f*a.y;
		p.z += v.z + 0.5f*a.z;

		v.x += a.x;
		v.y += a.y;
		v.z += a.z;
	}
	BoundCheck();
}

void PhysicalObj::Collision(PhysicalObj *target)
{
	D3DXVECTOR3 distance = (p+center) - (target->p + target->center); // 엄밀하게는 scale도 포함!
	float length = D3DXVec3Length(&distance);
	float rsum = radius*scale + target->radius * target->scale;
	if(rsum > length) { // collision!

		exit(0);
		D3DXVECTOR3 d = target->p - p; // normal
		D3DXVec3Normalize(&d, &d);

		D3DXVECTOR3 mv1 = d * D3DXVec3Dot(&d, &v);
		D3DXVECTOR3 mv2 = d * D3DXVec3Dot(&d, &target->v);

		v = (v - mv1) + mv2;
		target->v = (target->v - mv2) + mv1;
	}
}

void PhysicalObj::SetBoundingBox(D3DXVECTOR3 m, D3DXVECTOR3 M)
{
	min = m;
	max = M;
}

void PhysicalObj::SetBoundingSphere(D3DXVECTOR3 c, float r)
{
	center = c;
	radius = r;
}

D3DXMATRIXA16 PhysicalObj::GetWorldMatrix()
{
	D3DXMATRIXA16 matWorld, matScale;
	D3DXMatrixTranslation(&matWorld, p.x, p.y, p.z);
	D3DXMatrixScaling(&matScale, scale, scale, scale);

	D3DXMatrixMultiply(&matWorld, &matScale, &matWorld);
	return matWorld;
}