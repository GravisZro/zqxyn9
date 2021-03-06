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
// Chunk.h
// Project: Nostril (aka Postal)
// 
// History:
//		05/13/97 JMI	Started.
//
//		05/22/97	JMI	Can support several types of 'chunks'.
//
//		08/18/97	JMI	Now uses its own internal GetRand() and has randomization
//							arguments to Setup() so the caller can still control the
//							the randomization.
//							Now Construct() will not construct a CChunk if particle
//							effects are disabled.
//
//		09/08/97	JMI	Added Kevlar type for pieces of kevlar vest that get
//							splatter off of dudes with vest.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will represent pieces of bloody yee (or chunks)
// that fly off a recently damaged, complex organism or something.
// Also, can now support bullet casings and shells.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef CHUNK_H
#define CHUNK_H

#include <newpix/collisiondetection.h>

class CChunk
    : public Collidable,
      public CSpriteLine2d
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		typedef enum
			{
			Blood,
			BulletCasing,
			Shell,
			Kevlar,

			NumTypes
			} Type;

		typedef struct
			{
			uint8_t		u8ColorIndex;
			int16_t	sLen;
			} TypeInfo;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
   public:
		double m_dVel;
		double m_dVertVel;

		int32_t	m_lPrevTime;

		int16_t m_sSuspend;								// Suspend flag

		Type	m_type;

		int16_t	m_sLen;									// Length of item.
														
   protected:

		// Note that this is never reseeded b/c this is just an 'effect'
		// that does not and SHOULD not affect game play as it can be
		// turned off.
		static int32_t			ms_lGetRandomSeed;	// Seed for GetRand[om]().

		// Chunk info for each type.
		static TypeInfo	ms_atiChunks[NumTypes];

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
   public:
      CChunk(void);
      virtual ~CChunk(void);

	//---------------------------------------------------------------------------
	// Virtual functions (implementing them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		// Note that this setup accepts the amount of random sway you want to
		// apply to the particle so you don't have to.  You should not, otherwise
		// you'll ruin it (the game synch that is). Seriously.
		int16_t Setup(					// Returns 0 on success.
			int16_t sX,					// In:  New x coord
			int16_t sY,					// In:  New y coord
			int16_t sZ,					// In:  New z coord
			double dRot,				// In:  Initial direction.
			int16_t	sRandRotSway,		// In:  Random sway on rotation or zero.
			double dVel,				// In:  Initial velocity.
			int16_t	sRandVelSway,		// In:  Random sway on velocity or zero.
			double dVertVel,			// In:  Initial vertical velocity.
			int16_t	sRandVertVelSway,	// In:  Random sway on velocity or zero.
			Type	type);				// In:  Type of chunk.

		// Get a random number that is in no way related to the game's main
		// GetRand().
		static int32_t GetChunkRand(void)
			{
         return (((ms_lGetRandomSeed = ms_lGetRandomSeed * 214013L + 2531011L) >> 16) & 0x7FFF);
			}

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
	};


#endif // CHUNK_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
