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
// Chunk.cpp
// Project: Nostril (aka Postal)
// 
// History:
//		05/13/97 JMI	Started.
//
//		05/15/97	JMI	Added alpha'ing of blood on the ground.
//
//		05/22/97	JMI	Changed blood alpha level to 60 (was 70).
//
//		05/22/97	JMI	Can support several types of 'chunks'.
//
//		05/26/97	JMI	Changed bullet casing color to 7 (gray) was 3 
//							(dark yellow).
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		08/18/97	JMI	Now uses its own internal GetRand() and has randomization
//							arguments to Setup() so the caller can still control the
//							the randomization.
//
//		08/25/97	JMI	Setup() was mod'ing rotation.y before adding in the random
//							sway value causing it to sometimes exceed 359.  Fixed.
//
//		09/08/97	JMI	Added Kevlar type for pieces of kevlar vest that 
//							splatter off of dudes with vest.
//
//		09/08/97	JMI	Took out CHUNK_* macros to verify/make-sure they're not
//							used.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will represent pieces of bloody yee (or chunks)
// that fly off a recently damaged, complex organism or something.
//
//////////////////////////////////////////////////////////////////////////////

#include "chunk.h"

#include "realm.h"
#include "reality.h"

// This class defines its own GetRand()
#undef GetRand
#undef GetRandom

#define GetRand	CChunk::GetChunkRand
#define GetRandom	CChunk::GetChunkRand

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

// Gets a random between -range / 2 and range / 2.
#define RAND_SWAY(sway)	((CChunk::GetChunkRand() % sway) - sway / 2)


// Level at which to alpha blood on the ground.
#define ALPHA_LEVEL		60

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Note that this is never reseeded b/c this is just an 'effect'
// that does not and SHOULD not affect game play as it can be
// turned off.
int32_t		CChunk::ms_lGetRandomSeed	= 0;	// Seed for GetRand[om]().

// Chunk info for each type.
CChunk::TypeInfo	CChunk::ms_atiChunks[CChunk::NumTypes]	=
	{	// u8ColorIndex, sLen
		{	1,		4,	},	// Blood.
		{	7,		3,	},	// BulletCasing.
		{	2,		4,	},	// Shell.
		{	7,		4,	},	// Kevlar.
	};


CChunk::CChunk(void)
{
  m_sSuspend = 0;
  rotation.y = 0.0;
  m_dVel = 0.0;
  m_dVertVel = 0.0;
  m_sLen = 0;

  //			m_sprite.m_pthing		= this;
  m_u8Color	= 1;

  m_type = Blood;
}

CChunk::~CChunk(void)
{
}

////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CChunk::Suspend(void)
	{
	m_sSuspend++;
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CChunk::Resume(void)
	{
	m_sSuspend--;
	}

////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CChunk::Update(void)
	{
	int32_t	lCurTime		= realm()->m_time.GetGameTime();

	double	dSeconds	= (lCurTime - m_lPrevTime) / 1000.0;
	m_lPrevTime			= lCurTime;

	double	dDist		= m_dVel	* dSeconds;

   position.x					+= COSQ[(int16_t)rotation.y] * dDist;
   position.z					-= SINQ[(int16_t)rotation.y] * dDist;

	double dVertDeltaVel	= g_dAccelerationDueToGravity * dSeconds;
	m_dVertVel			+= dVertDeltaVel;

   position.y					+= (m_dVertVel - dVertDeltaVel / 2) * dSeconds;

	// If we have hit terrain . . .
   if (realm()->GetHeight(position.x, position.z) >= position.y)
		{
		int16_t	sX2d, sY2d;
		// Map from 3d to 2d coords.
      realm()->Map3Dto2D(position.x, position.y, position.z,
                         sX2d, sY2d);

		switch (m_type)
			{
        UNHANDLED_SWITCH;
         case Blood:
				{
            RImage*	pim	= realm()->Hood()->m_pimBackground;

				if (	sX2d >= 0 && sY2d >= 0 
					&&	sX2d < pim->m_sWidth
					&& sY2d < pim->m_sHeight)
					{
					// Pixel.  8bpp only!
					uint8_t*	pu8Dst	= pim->m_pData + sX2d + sY2d * pim->m_lPitch;
					
					*pu8Dst	= rspBlendColor(						// Alpha color/index.
						ALPHA_LEVEL,									// Alpha level.
                  realm()->Hood()->m_pmaTransparency,	// Multialpha.
                  m_u8Color,							// Src color/index to blend.
						*pu8Dst);										// Dst color/index to blend.
					}
				
				break;
				}
			
			case BulletCasing:
			case Shell:
#if 0	// Looks bad.
            rspPlot<uint8_t>(
               251,
               realm()->Hood()->m_pimBackground,
					sX2d, 
					sY2d);

				rspLine(
               m_u8Color,
               realm()->Hood()->m_pimBackground,
					sX2d, 
					sY2d,
					sX2d + RAND_SWAY(BLOOD_SWAY),
					sY2d + RAND_SWAY(BLOOD_SWAY),
					nullptr);
#endif
				break;
			}

		// We're done.
        Object::enqueue(SelfDestruct);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CChunk::Render(void)
{
  // Map from 3d to 2d coords
  realm()->Map3Dto2D(position.x, position.y, position.z,
                     m_sX2, m_sY2);

  m_sX2End	= m_sX2 + RAND_SWAY(m_sLen);
  m_sY2End	= m_sY2 + RAND_SWAY(m_sLen);

  // Priority is based on bottom edge of sprite on X/Z plane.
  m_sPriority = position.z;

  // Layer should be based on info we get from attribute map.
  m_sLayer = CRealm::GetLayerViaAttrib(realm()->GetLayer((int16_t) position.x, (int16_t) position.z));

  Object::enqueue(SpriteUpdate); // Update sprite in scene
}


////////////////////////////////////////////////////////////////////////////////
// Setup object.
////////////////////////////////////////////////////////////////////////////////
int16_t CChunk::Setup(			// Returns 0 if successfull, non-zero otherwise
	int16_t sX,					// In: New x coord
	int16_t sY,					// In: New y coord
	int16_t sZ,					// In: New z coord
	double dRot,				// In: Initial direction.
	int16_t	sRandRotSway,		// In:  Random sway on rotation or zero.
	double dVel,				// In:  Initial velocity.
	int16_t	sRandVelSway,		// In:  Random sway on velocity or zero.
	double dVertVel,			// In:  Initial vertical velocity.
	int16_t	sRandVertVelSway,	// In:  Random sway on velocity or zero.
	Type	type)					// In:  Type of chunk.
	{
	int16_t sResult = SUCCESS;
	
	// Use specified position
   position.x = sX;
   position.y = sY;
   position.z = sZ;

	m_dVel		= dVel;
	m_dVertVel	= dVertVel;

	// Apply randomizations.
	if (sRandRotSway)
		{
      rotation.y	= rspMod360(dRot + RAND_SWAY(sRandRotSway) );
		}
	else
		{
      rotation.y		= rspMod360(dRot);
		}

	if (sRandVelSway)
		{
		m_dVel	+= RAND_SWAY(sRandVelSway);
		}

	if (sRandVertVelSway)
		{
		m_dVertVel	+= RAND_SWAY(sRandVertVelSway);
		}

	m_lPrevTime	= realm()->m_time.GetGameTime();

	m_type		= type;

	ASSERT(type < NumTypes);
   m_u8Color	= ms_atiChunks[type].u8ColorIndex;
	m_sLen					= ms_atiChunks[type].sLen;

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
