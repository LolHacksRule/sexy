#include "SexyMatrix.h"

#include <math.h>

using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SexyMatrix3::SexyMatrix3()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyMatrix3::ZeroMatrix()
{
	m00 = m01 = m02 =
	m10 = m11 = m12 = 
	m20 = m21 = m22 = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyMatrix3::LoadIdentity()
{
	m01	= m02 = m10 = m12 = m20 = m21 = 0;
	m00 = m11 = m22 = 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SexyMatrix3 SexyMatrix3::operator*(const SexyMatrix3 &theMat) const
{
	SexyMatrix3 aResult;

	for(int i=0; i<3; i++)
	{
		for(int j=0; j<3; j++)
		{
			float x = 0;
			for(int k=0; k<3; k++)				
				x += m[i][k]*theMat.m[k][j];

			aResult.m[i][j] = x;
		}
	}

	return aResult;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SexyVector2 SexyMatrix3::operator*(const SexyVector2 &theVec) const
{
	return SexyVector2(
		m00*theVec.x + m01*theVec.y + m02,
		m10*theVec.x + m11*theVec.y + m12);
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SexyVector3 SexyMatrix3::operator*(const SexyVector3 &theVec) const
{
	return SexyVector3(
		m00*theVec.x + m01*theVec.y + m02*theVec.z,
		m10*theVec.x + m11*theVec.y + m12*theVec.z,
		m20*theVec.x + m21*theVec.y + m22*theVec.z);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const SexyMatrix3& SexyMatrix3::operator*=(const SexyMatrix3 &theMat)
{
	return operator=(operator*(theMat));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SexyTransform2D::SexyTransform2D()
{
	LoadIdentity();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SexyTransform2D::SexyTransform2D(bool loadIdentity)
{
	if (loadIdentity)
		LoadIdentity();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SexyTransform2D::SexyTransform2D(const SexyMatrix3& theMatrix) : SexyMatrix3(theMatrix)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const SexyTransform2D& SexyTransform2D::operator=(const SexyMatrix3 &theMat)
{
	SexyMatrix3::operator=(theMat);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyTransform2D::Translate(float tx, float ty)
{
	SexyMatrix3 aMat;
	aMat.LoadIdentity();
	aMat.m02 = tx;
	aMat.m12 = ty;
	aMat.m22 = 1;

	*this = aMat * (*this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyTransform2D::RotateRad(float rot)
{
	SexyMatrix3 aMat;
	aMat.LoadIdentity();

	float sinRot = -sinf(rot);
	float cosRot = cosf(rot);

	aMat.m00 = cosRot;
	aMat.m01 = -sinRot;
	aMat.m10 = sinRot;
	aMat.m11 = cosRot;

	*this = aMat * (*this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyTransform2D::RotateDeg(float rot)
{
	RotateRad(3.1415926535897932384626433832795028841971f * rot / 180.0f);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyTransform2D::Scale(float sx, float sy)
{
	SexyMatrix3 aMat;
	aMat.LoadIdentity();
	aMat.m00 = sx;
	aMat.m11 = sy;

	*this = aMat * (*this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Transform::Transform() : mMatrix(false)
{
	Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Transform::Reset()
{
	mNeedCalcMatrix = true;
	mComplex = false;
	mTransX1 = mTransY1 = 0;
	mTransX2 = mTransY2 = 0;
	mScaleX = mScaleY = 1;
	mRot = 0;
	mHaveRot = false;
	mHaveScale = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Transform::Translate(float tx, float ty)
{
	if (!mComplex)
	{
		mNeedCalcMatrix = true;
		if (mHaveRot || mHaveScale)
		{
			mTransX2 += tx;
			mTransY2 += ty;
		}
		else
		{
			mTransX1 += tx;
			mTransY1 += ty;
		}
	}
	else
		mMatrix.Translate(tx,ty);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Transform::RotateRad(float rot)
{
	if (!mComplex)
	{
		if (mHaveScale)
		{
			MakeComplex();
			mMatrix.RotateRad(rot);
		}
		else 
		{
			mNeedCalcMatrix = true;
			mHaveRot = true;
			mRot += rot;
		}
	}
	else
		mMatrix.RotateRad(rot);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Transform::RotateDeg(float rot)
{
	Transform::RotateRad(3.1415926535897932384626433832795028841971f * rot / 180.0f);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Transform::Scale(float sx, float sy)
{
	if (!mComplex)
	{
		if (mHaveRot || mTransX1!=0 || mTransY1!=0 || (sx<0 && mScaleX*sx!=-1) || sy<0)
		{
			MakeComplex();
			mMatrix.Scale(sx,sy);
		}
		else
		{
			mNeedCalcMatrix = true;
			mHaveScale = true;
			mScaleX *= sx;
			mScaleY *= sy;
		}
	}
	else
		mMatrix.Scale(sx,sy);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Transform::MakeComplex()
{
	if (!mComplex)
	{
		mComplex = true;
		CalcMatrix();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Transform::CalcMatrix() const
{
	if (mNeedCalcMatrix)
	{
		mNeedCalcMatrix = false;

		mMatrix.LoadIdentity();
		mMatrix.m02 = mTransX1;
		mMatrix.m12 = mTransY1;
		mMatrix.m22 = 1;

		if (mHaveScale)
		{
			mMatrix.m00 = mScaleX;
			mMatrix.m11 = mScaleY;
		}
		else if (mHaveRot)
			mMatrix.RotateRad(mRot);

		if (mTransX2!=0 || mTransY2!=0)
			mMatrix.Translate(mTransX2,mTransY2);
	}

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const SexyTransform2D& Transform::GetMatrix() const
{
	CalcMatrix();
	return mMatrix;
}

double Sexy::SexyTransform2D::determinantOfMinor( int theRowHeightY, int theColumnWidthX) const
{
	int x1 = theColumnWidthX == 0 ? 1 : 0;  /* always either 0 or 1 */
	int x2 = theColumnWidthX == 2 ? 1 : 2;  /* always either 1 or 2 */
	int y1 = theRowHeightY   == 0 ? 1 : 0;  /* always either 0 or 1 */
	int y2 = theRowHeightY   == 2 ? 1 : 2;  /* always either 1 or 2 */

	return ( m [y1] [x1]  *  m [y2] [x2] )
		-  ( m [y1] [x2]  *  m [y2] [x1] );
}

double Sexy::SexyTransform2D::GetDeterminant() const
{
	return ( m [0] [0]  *  determinantOfMinor( 0, 0) )
		-  ( m [0] [1]  *  determinantOfMinor( 0, 1) )
		+  ( m [0] [2]  *  determinantOfMinor( 0, 2) );
}

Sexy::SexyTransform2D Sexy::SexyTransform2D::Inverse() const
{
	SexyTransform2D aRetTrans(true);
	double det = GetDeterminant();

	if ( abs(det) < 1e-2 )
		return aRetTrans;

	double oneOverDeterminant = 1.0 / det;

	for (   int y = 0;  y < 3;  y ++ )
	{
		for ( int x = 0;  x < 3;  x ++   )
		{
			/* Rule is Inverse = 1/det * minor of the TRANSPOSE matrix.  *
			* Note (y,x) becomes (x,y) INTENTIONALLY here!              */
			aRetTrans.m[y][x] = determinantOfMinor( x, y ) * oneOverDeterminant;

			/* (y0,x1)  (y1,x0)  (y1,x2)  and (y2,x1)  all need to be negated. */
			if( 1 == ((x + y) % 2) )
				aRetTrans.m[y][x] = - aRetTrans.m[y][x];
		}
	}

	return aRetTrans;
}