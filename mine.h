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
// mine.h
// Project: Postal
//
//	History:
//		03/19/97 BRH	Started this weapon object.
//
//		04/29/97	JMI	Added GetSprite() virtual override to provide access
//							to m_sprite from a lower level.
//							Replaced Setup() with default parm eType = Proximity with
//							a Setup() that matches the base class virtual Setup() to
//							make sure it gets overriden.  The functionality is the
//							same (the new Setup() just calls the four parm Setup()
//							with Proximity as the type).
//
//		04/30/97	JMI	Changed the Setup() override of the CWeapon's Setup() to
//							pass the current mine type to the Setup() with eType.
//							Changed Construct() to take an ID as a parameter and added
//							ConstructProximity(), ConstructTimed(), 
//							ConstructBouncingBetty(), and ConstructRemoteControl() to 
//							allocate that type of mine.
//							Removed m_eMineType (now uses Class ID instead).
//							Removed Setup() that took an eType.
//							Fixed EditRect() and added EditHotSpot().
//
//		06/12/97 BRH	Initialized the Shooter ID to IdNil for mines that
//							are placed in the level, and not placed by a CDude.
//
//		06/27/97	JMI	Modified EditRect() to use Map3Dto2D().
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/21/97	JMI	Now handles delete messages.
//
//		08/16/97 BRH	Added a sound handle so that we could have a looping 
//							arming sound that could be stopped when the mine was
//							armed.
//
//		08/17/97	JMI	Destructor now stops looping the arming sound, if it is
//							still running.
//
//		08/28/97 BRH	Added preload function to load the sounds and images.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MINE_H
#define MINE_H

#include "weapon.h"
#include "bulletFest.h"


// CMine is an unguided missile weapon class
class CMine
    : public CWeapon,
      public CSprite2
	{
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

	typedef uint8_t MineType;

   enum
	{
		ProximityMine = 3,
		TimedMine,
		BouncingBettyMine,
		RemoteControlMine,
		NumMineTypes
	};

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:

	protected:
		int16_t m_sPrevHeight;							// Previous height

		CSmash		m_smash;							// Collision object
		CBulletFest	m_bulletfest;					// Used for bouncing betty
		double		m_dVertVel;						// Vertical velocity 
		double		m_dVertDeltaVel;				// Change in vertical velocity
		int32_t			m_lFuseTime;					// Time before timed mine goes off
		SampleMaster::SoundInstance m_siMineBeep;// Arming beep sound that loops
		// Tracks file counter so we know when to load/save "common" data 
		static int16_t ms_sFileCount;

	public:
		// "Constant" values that we want to be able to tune using the editor
		static int16_t ms_sProximityRadius;		// Distance at which mine goes off
		static int16_t ms_sBettyRadius;				// Distance at which mine goes off
		static int16_t ms_sBettyRange;				// Affected area for Bouncing Betty
		static int32_t ms_lFuseTime;					// Timed mine explodes after this time
		static int32_t ms_lArmingTime;				// Proximity mines arm after this time
		static int32_t ms_lExplosionDelay;			// Delay before explosion triggers mine
		static double ms_dInitialBounceVelocity;//Bouncing Betty popup velocity


	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
   public:
      CMine(void);
      virtual ~CMine(void);

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
   public:
		// Called after load to start the object
      void Startup(void);

		// Init - common initialization code for startup, setup & edit new
		int16_t Init(void);

#if !defined(EDITOR_REMOVED)
		// Puts up a dialog box in the editor to select mine type
		int16_t EditModify(void);

		// Sets up new item in the editor
		int16_t EditNew(int16_t sX, int16_t sY, int16_t sZ);

      void EditRect(RRect* pRect);
		void EditHotSpot(			// Returns nothiing.
			int16_t*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
         int16_t*	psY);			// Out: Y coord of 2D hotspot relative to
                              // EditRect() pos.
#endif // !defined(EDITOR_REMOVED)

	//---------------------------------------------------------------------------
	// Optional static functions
	//---------------------------------------------------------------------------

		// Called before play begins to cache resources for this object.
		static int16_t Preload(
			CRealm* prealm);				// In:  Calling realm.

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

		// Called by the object that is creating this weapon - this
		// overloaded version is for timed mines so that the fuse time
		// can be set
		int16_t Setup(
			int16_t sX,												// In: New x coord
			int16_t sY,												// In: New y coord
			int16_t sZ,												// In: New z coord
			int32_t lFuseTime);										// In: Time in ms for fuse

		// Override base class Setup().
		virtual				// Overridden here.
		int16_t Setup(
			int16_t sX,												// In: Starting X position
			int16_t sY,												// In: Starting Y position
			int16_t sZ);												// In: Starting Z position

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		int16_t GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		int16_t FreeResources(void);						// Returns 0 if successfull, non-zero otherwise

		// Handle Explosion message
		void OnExplosionMsg(Explosion_Message* pMessage);

		// Handle Trigger message (for remote trigger mines)
		void OnTriggerMsg(Trigger_Message* pMessage);
	};


class CProximityMine : public CMine
{ };

class CTimedMine : public CMine
{ };

class CBouncingBettyMine : public CMine
{ };

class CRemoteControlMine : public CMine
{ };

#endif // MINE_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
