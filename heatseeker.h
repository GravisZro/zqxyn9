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
// heatseeker.h
// Project: Postal
//
// History:
//		05/13/97 BRH	Started this weapon object fromt the CHeatseeker code.
//
//		07/01/97 BRH	Added smoke timer for making smoke trails.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		08/14/97 BRH	Added SetCollideBits and the collision bit fields so that
//							they can be set by the Doofus or Dude when they shoot it
//							and they can collide differently, rather than the
//							standard default behavoir.
//
//		08/17/97 BRH	Added thrust sound instance so it is like the rocket.
//
//		08/23/97	JMI	Added CSmash::AlmostDead to exclude bits.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef HEATSEEKER_H
#define HEATSEEKER_H

#include "weapon.h"

#include "Anim3D.h"
#include "SampleMaster.h"

// CHeatseeker is a heat seeking missile
class CHeatseeker
    : public CWeapon,
      public CSprite3
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		// Collision bits

	protected:
		CAnim3D		m_anim;					// 3D animation
		RTransform	m_trans;					// Transform
		CSmash		m_smash;					// Smash used for explosion collisions (small)
		CSmash		m_smashSeeker;			// Smash used to detect heat sources (larger)
		bool			m_bArmed;				// Initially missile is not armed so it doesn't
                                       // collide with the person who shot it.
		int32_t			m_lSmokeTimer;			// Time to wait between emitting smoke
		uint32_t			m_u32CollideBitsInclude;	// Bits that cause a collision
		uint32_t			m_u32CollideBitsDontCare;	// Bits that are ignored for collisions
		uint32_t			m_u32CollideBitsExclude;	// Bits that invalidate a collision


		uint32_t			m_u32SeekBitsInclude;		// Bits that cause a collision
		uint32_t			m_u32SeekBitsDontCare;		// Bits that are ignored for collisions
		uint32_t			m_u32SeekBitsExclude;		// bits taht invalidate a collision

		SampleMaster::SoundInstance m_siThrust; // Looping thrust play instance

		// Tracks file counter so we know when to load/save "common" data 
		static int16_t ms_sFileCount;

		// "Constant" values that we want to be able to tune using the editor
		static double ms_dAccUser;			// Acceleration due to user
		static double ms_dMaxVelFore;		// Maximum forward velocity
		static double ms_dMaxVelBack;		// Maximum backward velocity
		static double ms_dCloseDistance;	// Close enough to hit CDude
		static double ms_dLineCheckRate;	// Pixel distance for line checking
		static int32_t ms_lArmingTime;		// Time before weapons arms.
		static int32_t ms_lSeekRadius;		// Radius of Heatseeking circle
		static int16_t ms_sOffScreenDist;  // Distance off screen before self destructing
		static int16_t ms_sAngularVelocity;// Degrees per second that it can turn
		static uint32_t ms_u32CollideIncludeBits;
		static uint32_t ms_u32CollideDontcareBits;
		static uint32_t ms_u32CollideExcludeBits;
		static uint32_t ms_u32SeekIncludeBits;
		static uint32_t ms_u32SeekDontcareBits;
		static uint32_t ms_u32SeekExcludeBits;
		static int32_t ms_lSmokeTrailInterval;	// Time between smoke releases
		static int32_t ms_lSmokeTimeToLive;		// Time for smoke to stick around.

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	public:
      CHeatseeker(void);
      virtual ~CHeatseeker(void);


	//---------------------------------------------------------------------------
	// Optional static functions
	//---------------------------------------------------------------------------

		// Called before play begins to cache resources for this object
		static int16_t Preload(
			CRealm* prealm);				// In:  Calling realm.

	public:
		void SetTransform(RTransform* pTransform)
			{
            m_ptrans = pTransform;
         }

	//---------------------------------------------------------------------------
	// Optional virtual functions
	//---------------------------------------------------------------------------
	public:
		
		// Used to set the collision bit fields
		virtual
		void SetCollideBits(	   // Returns nothing
			uint32_t u32BitsInclude,	// Bits to detect in collisions
			uint32_t u32BitsDontCare,	// Bits that don't matter for collision detection
			uint32_t u32BitsExclude)	// Bits that invalidate collision
		{
			m_u32CollideBitsInclude = u32BitsInclude | CSmash::Fire;
			m_u32CollideBitsDontCare = u32BitsDontCare;
			m_u32CollideBitsExclude = u32BitsExclude | CSmash::Ducking | CSmash::AlmostDead;
		}

		// Used to set the detection bit fields
		virtual
		void SetDetectionBits(	// Returns nothing
			uint32_t u32BitsInclude,	// Bits to detect in collisions
			uint32_t u32BitsDontcare,	// Bits that don't matter for collision detection
			uint32_t u32BitsExclude)	// Bits that invalidate collision
		{
			m_u32SeekBitsInclude = u32BitsInclude | CSmash::Fire;
			m_u32SeekBitsDontCare = u32BitsDontcare;
			m_u32SeekBitsExclude = u32BitsExclude | CSmash::Ducking | CSmash::AlmostDead;
		}


	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		int16_t Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			int16_t sFileCount,										// In:  File count (unique per file, never 0)
			uint32_t	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		int16_t Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			int16_t sFileCount);									// In:  File count (unique per file, never 0)

		// Update object
		void Update(void);

		// Render object
		void Render(void);

		// Called by another object to set start a new rocket
		int16_t Setup(
			int16_t sX,
			int16_t sY,
			int16_t sZ);

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		int16_t GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		int16_t FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		inline int16_t FindAngleTo(double dX, double dZ)
		{
         return rspATan((position.z - dZ), (dX - position.x));
		}
	};



#endif //HEATSEEKER_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
