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
// napalm.cpp
// Project: Postal
//
// This module implements the CNapalm weapon class which is a canister of
// napalm gel that breaks apart when it hits the ground lays down a smear
//	of fire.  The canister may be an alternate ordinate for the rocket launcher.
// 
//
// History:
//		01/17/97 BRH	Started this weapon object.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/09/97 BRH	Started the CNapalm from the CGrenade object since their
//							initial movement logic is similar.
//
//		02/10/97	JMI	rspReleaseResource() now takes a ptr to a ptr.
//
//		02/18/97 BRH	Added a time setting to the Fire with some randomness
//							to make it look better.
//
//		02/19/97 BRH	Added message processing to check for ObjectDeleted 
//							message.
//
//		02/19/97 BRH	Changed this from 2D to 3D animation.
//
//		02/23/97 BRH	Updated the transform with the angle so that the canister
//							faces the direction it is traveling.  Also changed the
//							coordinate system to x,-z
//
//		02/23/97 BRH	Added Preload() function to cache resources for this object
//							before play begins.
//
//		02/24/97	JMI	No longer sets the m_type member of the m_sprite b/c it
//							is set by m_sprite's constructor.
//
//		02/24/97 BRH	Changed fire to thin fire for more alpha effect since it
//							lays down many layers of fire.  Also hides the napalm
//							canister by skipping the render when in the hidden state.
//
//		02/24/97 BRH	Added sound effects for canister shooting, hitting things,
//							and when it breaks open.  Used reality.h motion templates
//							and changed the algorithm for detecting ground and
//							walls.
//
//		03/03/97 BRH	Derived this from the CWeapon base class.
//
//		03/03/97	JMI	Commented out dHorizVelocity and dVertVelocity parameters
//							to Setup() so that this version would be a virtual over-
//							ride of CWeapon's.
//
//		03/06/97	JMI	Upgraded to current rspMod360 usage.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/19/97 BRH	Changed ProcessMessages to return a void so that it matches
//							the new virtual function in the CWeapon base class.
//
//		03/21/97 BRH	Now ignores the ATTRIBUTE_NOT_WALKABLE so that the napalm
//							canisters don't bounce off of the edge of the world.
//
//		04/10/97 BRH	Converted to using the new multi layer attribute maps and
//							the helper functions that go with them.
//
//		05/04/97 BRH	Took out an old unused reference to an STL iterator.
//
//		05/29/97	JMI	Removed ASSERT on realm()->m_pAttribMap which no longer
//							exists.
//
//		06/11/97 BRH	Added shooter ID passing to the fire that is created.
//
//		06/12/97	JMI	Now handles State_Hide by setting m_sprite's InHidden flag.
//
//		06/16/97 BRH	Fixed starting condition in not walkable area.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/25/97 BRH	Added use of base class 2D shadow on the ground, but loaded
//							a smaller shadow resource.
//
//		06/30/97 BRH	Added sound effect cache to Preload function.
//
//		07/01/97	JMI	Replaced GetFloorMapValue() with GetHeight() call.
//
//		07/09/97	JMI	Now uses realm()->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/30/97	JMI	Same old delete error showed up on Alpha.  
//							ProcessMessages() was deleting the napalm on a delete msg
//							but, once returned to Update(), it was checking the 
//							m_eState member to see if it should return.  Unfortunately,
//							since 'this' had already been deallocated, it was too late
//							to do such a thing.
//							Also, m_dFireX and m_dFireZ were uninitialized causing
//							floating point exceptions (due to bad values) on the Alpha.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.
//
//		08/17/97	JMI	Changed m_pthingParent to m_idParent.
//
//		08/27/97 BRH	Added large fire sound which had not been used until now.
//
//		08/28/97 BRH	Added cache of large fire sound.
//
////////////////////////////////////////////////////////////////////////////////

#include "napalm.h"

#include "realm.h"
#include "dude.h"
#include "fire.h"
#include "SampleMaster.h"
#include "game.h"
#include "reality.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SMALL_SHADOW_FILE "smallshadow.img"

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
double CNapalm::ms_dAccDrag     = 300.0;				// Acceleration due to drag
double CNapalm::ms_dThrowVertVel = 30.0;				// Throw up at this velocity
double CNapalm::ms_dThrowHorizVel = 300;				// Throw out at this velocity
double CNapalm::ms_dMinFireInterval = 5*5;
int32_t CNapalm::ms_lGrenadeFuseTime = 1500;			// Time from throw to blow

// Let this auto-init to 0
int16_t CNapalm::ms_sFileCount;


CNapalm::CNapalm(void)
{
}

CNapalm::~CNapalm(void)
{
  // Free resources
  FreeResources();
}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CNapalm::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	int16_t sFileCount,										// In:  File count (unique per file, never 0)
	uint32_t	ulFileVersion)									// In:  Version of file format to load.
	{
	int16_t sResult = CWeapon::Load(pFile, bEditMode, sFileCount, ulFileVersion);

	if (sResult == SUCCESS)
	{
		// Load common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			// Load static data
			switch (ulFileVersion)
				{
				default:
				case 1:
					pFile->Read(&ms_dAccDrag);
					pFile->Read(&ms_dThrowVertVel);
					pFile->Read(&ms_dThrowHorizVel);
					pFile->Read(&ms_dMinFireInterval);
					pFile->Read(&ms_lGrenadeFuseTime);
					break;
				}
			}

		// Load object data
		switch (ulFileVersion)
			{
			default:
			case 1:
				break;
			}
		
		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == SUCCESS)
			{
			// Get resources
			sResult = GetResources();
			}
		else
			{
			sResult = FAILURE;
			TRACE("CNapalm::Load(): Error reading from file!\n");
			}

	}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CNapalm::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	int16_t sFileCount)										// In:  File count (unique per file, never 0)
	{
	CWeapon::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
		{
		ms_sFileCount = sFileCount;

		// Save static data
		pFile->Write(&ms_dAccDrag);
		pFile->Write(&ms_dThrowVertVel);
		pFile->Write(&ms_dThrowHorizVel);
		pFile->Write(&ms_dMinFireInterval);
		pFile->Write(&ms_lGrenadeFuseTime);
		}

	// Save object data

   return SUCCESS;
	}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CNapalm::Update(void)
	{
	int16_t sHeight; 
	double dNewX;
	double dNewY;
	double dNewZ;
	double dX;
	double dZ;
	double dDistance;

	if (!m_sSuspend)
		{
		// Get new time
		int32_t lThisTime = realm()->m_time.GetGameTime(); 

		// If elapsed time is too short, skip this update.

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

      ProcessMessages();
		// Check the current state
		switch (m_eState)
		{
        UNHANDLED_SWITCH;
			case CWeapon::State_Idle:
				break;

			case CWeapon::State_Fire:
				// Make sure it starts in a valid location.  If it is inside
				// a wall, delete it now.
            sHeight = realm()->GetHeight((int16_t) position.x, (int16_t) position.z);
            if (position.y < sHeight)
				{
               Object::enqueue(SelfDestruct);
					return;
				}
				m_eState = State_Go;
				m_lTimer = lThisTime + ms_lGrenadeFuseTime;
				PlaySample(
					g_smidNapalmShot,
					SampleMaster::Weapon,
               DistanceToVolume(position.x, position.y, position.z, LaunchSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
				break;

//-----------------------------------------------------------------------
// Go - fly through the air until hit the ground, change directions on
//		  obstacle collision.
//-----------------------------------------------------------------------
			case CWeapon::State_Go:
				// Do horizontal velocity
            dNewX = position.x + COSQ[(int16_t)rotation.y] * (m_dHorizVel * dSeconds);
            dNewZ = position.z - SINQ[(int16_t)rotation.y] * (m_dHorizVel * dSeconds);

				// Do vertical velocity
            dNewY = position.y;
				AdjustPosVel(&dNewY, &m_dVertVel, dSeconds);

				// Check the height to see if it hit the ground
				sHeight = realm()->GetHeight(int16_t(dNewX), int16_t(dNewZ));

				// If its lower than the last and current height, assume it
				// hit the ground.
            if (dNewY < sHeight && position.y >= sHeight)
				{
               position.y = sHeight;
					m_eState = CWeapon::State_Slide;	
					PlaySample(
						g_smidNapalmFire,
						SampleMaster::Destruction,
                  DistanceToVolume(position.x, position.y, position.z, NapalmSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)

					PlaySample(
						g_smidFireLarge,
						SampleMaster::Destruction,
                  DistanceToVolume(position.x, position.y, position.z, NapalmSndHalfLife) );
				}
				else
				{
					// If it is above the last known ground and is now lower
					// than the height at its new position, assume it hit
					// a wall and should bounce.
               if (dNewY < sHeight && position.y < sHeight)
					{
                  dNewX = position.x;	// Restore last x position
                  dNewZ = position.z;	// Restore last z position
                  rotation.y = BounceAngle(rotation.y);	// Change directions
						PlaySample(
							g_smidNapalmHit,
							SampleMaster::Weapon,
                     DistanceToVolume(position.x, position.y, position.z, SideEffectSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
					}
					else
                  position.y = dNewY;
				}

            position.x = dNewX;
            position.z = dNewZ;
				break;

//-----------------------------------------------------------------------
// Slide - Once it hits the ground, slide until it stops.
//-----------------------------------------------------------------------
			case CWeapon::State_Slide:

				// As the Napalm canister slides on the ground, it lays 
				// down fire at intervals.  Check the interval to see
				// it its time to creat a new fire yet.
            dX = position.x - m_dFireX;
            dZ = position.z - m_dFireZ;
				dDistance = (dX*dX) + (dZ*dZ);
				if (dDistance > ms_dMinFireInterval)
				{
               m_dFireX = position.x;
               m_dFireZ = position.z;
					// Start a fire here
               managed_ptr<CFire> pFire = realm()->AddThing<CFire>();
               if (pFire)
					{
                  if (pFire->Setup(position.x - 20 + (GetRand() % 40), position.y, position.z - 20 + (GetRand() % 40),
						                 4000 + (GetRand() % 9000), false, CFire::LargeFire) != SUCCESS)
                     pFire.reset();
						else
							pFire->m_shooter = m_shooter;
					}
				}
				// Ground causes drag
				// Decelerate to zero.  When you reach zero, go
				// to find state.
				if (m_dHorizVel > 0)
				{
					AdjustVel(&m_dHorizVel, dSeconds, -ms_dAccDrag);
					if (m_dHorizVel < 0)
						m_dHorizVel = 0;
				}
				else if (m_dHorizVel < 0)
				{
					AdjustVel(&m_dHorizVel, dSeconds, ms_dAccDrag);
					m_dHorizVel = 0;
				}
				// If it has stopped, then change to find state
				if (m_dHorizVel == 0)
					m_eState = CWeapon::State_Explode;

            dNewX = position.x + COSQ[(int16_t)rotation.y] * (m_dHorizVel * dSeconds);
            dNewZ = position.z - SINQ[(int16_t)rotation.y] * (m_dHorizVel * dSeconds);
				// Check for obstacles
				sHeight = realm()->GetHeight(int16_t(dNewX), int16_t(dNewZ));
				// If it hit any obstacles, make it bounce off
            if (sHeight > position.y)
				{
					// Restore previous position 
               dNewX = position.x;
               dNewZ = position.z;
					// Change directions
               rotation.y = BounceAngle(rotation.y);
					PlaySample(
						g_smidNapalmHit,
						SampleMaster::Weapon,
                  DistanceToVolume(position.x, position.y, position.z, SideEffectSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
				}

				// See if it fell off of something.  If so make it go back
				// to the airborne state
            if (sHeight < (int16_t) position.y)
				{
					m_dVertVel = 0;
					m_eState = State_Go;
				}

            position.x = dNewX;
            position.z = dNewZ;

				break;


//-----------------------------------------------------------------------
// Explode
//-----------------------------------------------------------------------
			case CWeapon::State_Explode:
            Object::enqueue(SelfDestruct);
            return;
		}

		// Save time for next time
		m_lPrevTime = lThisTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CNapalm::Render(void)
{
	int32_t lThisTime = realm()->m_time.GetGameTime();

   m_pmesh = &m_anim.m_pmeshes->atTime(lThisTime);
   m_psop = &m_anim.m_psops->atTime(lThisTime);
   m_ptex = &m_anim.m_ptextures->atTime(lThisTime);
   m_psphere = &m_anim.m_pbounds->atTime(lThisTime);

	// Eventually this should be channel driven also
   m_sRadius = m_sCurRadius;

	// Reset rotation so it is not cumulative
	m_trans.makeIdentity();

	// Set its pointing direction
   m_trans.Ry(rspMod360(rotation.y));

   flags.Hidden = m_eState == State_Hide;

	// If we're not a child of someone else...
   if (!isChild())
	{
		// Map from 3d to 2d coords
     realm()->Map3Dto2D(position.x, position.y, position.z,
                        m_sX2, m_sY2);

		// Priority is based on bottom edge of sprite
      m_sPriority = position.z;

		// Layer should be based on info we get from attribute map
      m_sLayer = CRealm::GetLayerViaAttrib(realm()->GetLayer((int16_t) position.x, (int16_t) position.z));

      m_ptrans		= &m_trans;

      Object::enqueue(SpriteUpdate); // Update sprite in scene

		// Render the 2D shadow sprite
		CWeapon::Render();
	}
	else
	{
		// m_idParent is setting our transform relative to its position
		// and we are drawn by the scene with the parent.
	}
}

////////////////////////////////////////////////////////////////////////////////
// Setup new object - called by object that created this object
////////////////////////////////////////////////////////////////////////////////

int16_t CNapalm::Setup(									// Returns 0 if successfull, non-zero otherwise
	int16_t sX,												// In:  New x coord
	int16_t sY,												// In:  New y coord
	int16_t sZ/*,												// In:  New z coord
	double dHorizVel,										// In:  Starting Horizontal Velocity (has default)
	double dVertVel*/)									// In:  Starting Vertical Velocity (has default)
{
	int16_t sResult = SUCCESS;
	
	// Use specified position
   position.x = (double)sX;
   position.y = (double)sY;
   position.z = (double)sZ;
	m_dHorizVel = ms_dThrowHorizVel;//dHorizVelocity;
	m_dVertVel = ms_dThrowVertVel;//dVertVelocity;

	// Default these to the start position so that, when we first enter
	// the slide state, we'll create some fire right away.
   m_dFireX	= position.x;
   m_dFireZ	= position.z;

	// Load resources
	sResult = GetResources();

	// Enable the 2D shadow
	PrepareShadow();

	m_sCurRadius = 10;

	return sResult;
}



////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
int16_t CNapalm::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
  bool bResult = m_anim.Get("napalmcan");
  bResult &= rspGetResource(&g_resmgrGame, realm()->Make2dResPath(SMALL_SHADOW_FILE), &(m_spriteShadow.m_pImage), RFile::LittleEndian) == SUCCESS;
  return bResult ? SUCCESS : FAILURE;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
int16_t CNapalm::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	m_anim.Release();

   return SUCCESS;
	}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources 
//				 for this object so there won't be a delay the first time it is
//				 created.
////////////////////////////////////////////////////////////////////////////////

int16_t CNapalm::Preload(
	CRealm* prealm)				// In:  Calling realm.
	{
	CAnim3D anim;
   RImage* pimage;
   bool bResult = anim.Get("napalmcan");
	anim.Release();
   bResult &= rspGetResource(&g_resmgrGame, prealm->Make2dResPath(SMALL_SHADOW_FILE), &pimage, RFile::LittleEndian) == SUCCESS;
	rspReleaseResource(&g_resmgrGame, &pimage);
	CacheSample(g_smidNapalmShot);
	CacheSample(g_smidNapalmHit);
	CacheSample(g_smidNapalmFire);
	CacheSample(g_smidFireLarge);
   return bResult ? SUCCESS : FAILURE;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
