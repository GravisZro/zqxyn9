////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 RWS Inc, All Rights Reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as published by
// the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
///////////////////////////////////////////////////////////////
//	PIPELINE - History
///////////////////////////////////////////////////////////////
//
//	07/23/97	JRD	Added support for generating shadows.  Currently
//						all shadows are hard coded to be cast upon the
//						plane y = 0, based on postal needs.
//
///////////////////////////////////////////////////////////////


// This is the highest level considered actually part of the 3d engine.
// It is the highest level control -> it decides how 3d pts map to 2d.
// You can customize 3d efects by instantiating your own versions of the 3d pipeline!
// 
#ifndef PIPELINE_H
#define PIPELINE_H
//================================================== 
#include <BLUE/System.h>
#include <ORANGE/color/colormatch.h>
#include <3dtypes.h>
#include "zbuffer.h"
#include "render.h"

//================================================== 

// The point of this class is to hold configurable
// scratch space for doing trsnaformations
//
class RPipeLine
	{
public:
	//-------------------------------------
	RPipeLine();
	~RPipeLine();
   int16_t Create(size_t lScratchSpace=0,int16_t sZBufWidth=0);
	int16_t CreateShadow(int16_t sAngleY,double dTanDeclension,int16_t sBufSize = -1);
	void Destroy(); // will NOT kill transform scratch space

	//-------------------------------------
	int16_t NotCulled(Vector3D *p1,Vector3D *p2,Vector3D *p3);
	void Transform(RSop* pPts,RTransform& tObj);
	void TransformShadow(RSop* pPts,RTransform& tObj,
		int16_t sHeight = 0,int16_t *psOffX = nullptr,int16_t *psOffY = nullptr);

	// Do NOT use a z-buffer.  Return offset to current position to
	// draw the image m_pimShadowBuf
	void	RenderShadow(RImage* pimDst,RMesh* pMesh,uint8_t ucColor); // Unicolored!

	void Render(RImage* pimDst,int16_t sDstX,int16_t sDstY,
		RMesh* pMesh,uint8_t ucColor); // wire frame!

	// Flat shaded
	void Render(RImage* pimDst,int16_t sDstX,int16_t sDstY,
		RMesh* pMesh,RZBuffer* pZB,RTexture* pColors,
		int16_t sOffsetX = 0,		// In: 2D offset for pimDst and pZB.
		int16_t sOffsetY = 0); 	// In: 2D offset for pimDst and pZB.

	// Note that pFog must be 256 x # of colors.
	// the offset value moves the fog towards 
	// the front of the z-buffer
	//
	void Render(RImage* pimDst,int16_t sDstX,int16_t sDstY,
		RMesh* pMesh,RZBuffer* pZB,RTexture* pColors,
		int16_t sFogOffset,RAlpha* pFog,
		int16_t sOffsetX = 0,		// In: 2D offset for pimDst and pZB.
		int16_t sOffsetY = 0); 	// In: 2D offset for pimDst and pZB.

	// WARNING: May be inhomogeneous!
	void GetScreenXF(RTransform& tDst)
		{
      tDst.makeIdentity();
      tDst.Mul(m_tScreen,m_tView);
		}

	// Strictly for convenience:
	//
	void ClearClipBuffer();
	void ClearShadowBuffer();

	// Project a point onto a screen.
	void PointToScreen(RTransform& tObj,Vector3D& v3d,int16_t &sDstX,int16_t &sDstY)
		{
		RTransform tFull;
		Vector3D ptDst;

      tFull.makeIdentity();
      tFull.Mul(m_tView,tObj);
      tFull.PreMulBy(m_tScreen);

		tFull.TransformInto(v3d,ptDst);
      sDstX = int16_t(ptDst.x());
      sDstY = int16_t(ptDst.y());
		}
	
	// THIS WILL CHANGE WITH TIME:
	// Currently the bounding sphere is described by two points:
	//
	void BoundingSphereToScreen(Vector3D& ptCenter, Vector3D& ptRadius, 
		RTransform& tObj);

	//-------------------------------------
	// Configurable by instance:
	RZBuffer* m_pZB;
	RImage*	m_pimClipBuf; // For clipping (2 pass rendering)
	RTransform m_tScreen; // map to window
	RTransform m_tView; // lens

	RImage* m_pimShadowBuf;	// For drawing shadows
	RTransform	m_tShadow;	// Turn it into a shadow
	double	m_dShadowScale;// Needed extra parameter
	//-------------------------------------
	// WARNING: THIS WILL LIKELY CHANGE:
	// store a transformed bounding rect for object being rendered:
	// These are screend coordinates relative to the center of
	// the zbuf square / clipping square
   int16_t m_sX; // far cube point
   int16_t m_sY;
	int16_t m_sZ;
	int16_t m_sW;
	int16_t m_sH;
	int16_t m_sD;

   int16_t m_sCenX; // for convenience - the cube center
	int16_t m_sCenY; // in 3d screen coordinates
	int16_t m_sCenZ;
	// TRUE of FALSE
	int16_t m_sUseBoundingRect;

	//-------------------------------------
	// static storage:

	// Transformation buffer:
   static uint32_t ms_lNumPts;
	static Vector3D* ms_pPts;
   static uint32_t	ms_lNumPipes; // used to free ms_pPts
	};

//================================================== 
//================================================== 

//================================================== 
#endif
