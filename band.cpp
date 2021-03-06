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
// band.cpp
// Project: Postal
//
//	This module implements the marching band member.
//
// History:
//		03/04/97 BRH	Started this file.
//
//		03/05/97 BRH	Implemented the functionality of this class in this
//							function and in the base class CCharacter.  This uses
//							many of the default base class functions for motion, states
//							etc.  Currently has the logic to follow the parade route
//							and simple after Parade mingling.  Also reacts to
//							shot, fire and explosions.  Still need to add panic
//							mode and panic message sending.
//
//		03/06/97 BRH	Fixed panic message so it won't interrupt dying.  Also
//							Use AlignToBouy function to set the direction to bouy
//							and to periodically readjust the alighment to the bouy.
//
//		03/06/97	JMI	Upgraded to current rspMod360 usage.
//							Was commented out, but just in case it ever gets re-
//							instated.
//
//		03/07/97 BRH	Added dialog box to select starting bouy and also 
//							saves and loads that bouy ID.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/18/97	JMI	Now saves and loads child ID.
//							Render() now checks for child item and, if present, updates
//							its transform via the band member's rigid body transform.
//							EditModify() now allows one to select type of child.
//							OnExplosionMsg() now detaches child items.
//
//		03/18/97	JMI	Load() was ignoring versions 2 and 3.  Fixed.
//
//		03/18/97	JMI	OnDead() now drops current child instrument.
//
//		03/18/97	JMI	Update() was calling CCharacter::OnDead() bypassing the
//							CBand::OnDead() override.
//
//		03/19/97 BRH	Added a check to the OnPanicMsg to make sure that the
//							bouy chosen exists before getting its position.
//
//		03/27/97	JMI	Now you cannot create a CBand when there is no bouy.
//
//		04/10/97 BRH	Changed it to work with the new multi layer attribute maps.
//
//		04/16/97 BRH	Changed references to the realm's list of CThings to use
//							the new non-STL method.
//
//		04/29/97	JMI	Now, in Render(), the band guy has safer interaction with
//							his child instrument which, also, does not keep him from
//							having a weapon, like it did before.
//
//		05/12/97 BRH	Added the randomness to the falling down dead state so
//							when you kill the marchers they don't all fall the same
//							direction.
//
//		05/26/97 BRH	Added avoidance of obstacles.
//
//		05/27/97 BRH	Fixed problem in Panic where no random bouy was being
//							selected.
//
//		05/29/97	JMI	Changed instance of REALM_ATTR_FLOOR_MASK to 
//							REALM_ATTR_NOT_WALKABLE.
//
//		06/03/97 BRH	Changed the mingle so they walk around a lot more.  Added
//							screaming sound effects when they panic so that they aren't
//							totally silent as they run around.  Also changed the song
//							to be played internally rather than using a sound thing
//							object.  
//
//		06/04/97	JMI	Now aborts ms_pBandSongSound, if not NULL, in destructor.
//							Also, added ms_bDonePlaying so marchers know when to not
//							restart ms_pBandSongSound.
//
//		06/05/97	JMI	Changed m_sHitPoints to m_stockpile.m_sHitPoints to 
//							accommodate new m_stockpile in base class, CThing3d (which
//							used to contain the m_sHitPoints).
//
//		06/08/97	JMI	Added override for WhileDying() and WhileShot().  In which
//							we make sure to drop child items with some random
//							rotation velocity and our direction and velocity.
//							Also, OnExplosionMsg() now passes the message on to the
//							child item immediately after dropping it.
//
//		06/10/97 BRH	Added message passing to CDemon for all band members.
//
//		06/16/97 BRH	Added more sound effects for the band members.
//
//		06/17/97	JMI	Added NULL in call to PlaySample() corresponding to new
//							param.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/24/97	JMI	Added some LOG() calls for the synchronization log 
//							mechanism.
//
//		06/25/97	JMI	Now calls PrepareShadow() in Init() which loads and sets up
//							a shadow sprite.
//
//		06/30/97 MJR	Replaced SAFE_GUI_REF with new GuiItem.h-defined macro.
//
//		06/30/97 BRH	Changed most of the PlaySample calls to PlaySampleThenPurge
//							to save memory.  
//
//		07/01/97	JMI	Replaced GetFloorMapValue() with GetHeightAndNoWalk() call.
//
//		07/01/97	JMI	Now passes rigid body transform to DetachChild().
//
//		07/08/97 BRH	Renamed some of the bandguy's animations since the
//							filenames were too long for the delicate MacOS.
//
//		07/09/97	JMI	Removed unused 2D res name macros.
//
//		07/17/97	JMI	Changed ms_pBandSongSound to ms_siBandSongInstance.
//							Now uses new SampleMaster interface for volume and play
//							instance reference.
//
//		07/18/97	JMI	Got rid of bogus immitation PlaySample functions.
//							Now there is one PlaySample() function.  Also, you now
//							MUST specify a category and you don't have to specify a
//							SoundInstance ptr to specify a volume.
//
//		07/31/97 BRH	Changed destination bouy from always being 1 to an editable
//							item in the EditModify dialog box.
//
//		08/12/97	JMI	Now one band member maintains the volume for the band 
//							sample.
//
//		08/26/97 BRH	Added a few more voices to the list.
//
//		09/03/97	JMI	Replaced Good Smash bit with Civilian.
//
//		09/24/97 BRH	Band members will not appear for the non US version.  This
//							needs to be tested with the new project setup once 
//							the band is moved into the localized projects.
//
//		10/03/97	JMI	Now includes CompileOptions.h so it knows what US is when
//							comparing to LOCALE.
//
//		10/14/97	JMI	Added GetInstanceID() as parameter to LOG() calls.
//
//		09/27/99	JMI	Changed to allow band mebmers only in any locale 
//							satisfying the CompilerOptions macro VIOLENT_LOCALE.
//
////////////////////////////////////////////////////////////////////////////////

#include "band.h"

#include "realm.h"
#include "navnet.h"
#include "item3d.h"
#include "SampleMaster.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define BRIGHTNESS_PER_LIGHT_ATTRIBUTE 15
#define NUM_ELEMENTS(a) (sizeof(a) / sizeof(a[0]) )

// Notification message lParm1's.
#define BLOOD_POOL_DONE_NOTIFICATION	1	// Blood pool is done animating.

// Random amount the blood splat can adjust.
#define BLOOD_SPLAT_SWAY		10

// Gets a random between -range / 2 and range / 2.
#define RAND_SWAY(sway)	((GetRand() % sway) - sway / 2)

#define GUI_ID_OK						1

#define BAND_SONG_HALF_LIFE		500

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

double CBand::ms_dCloseToBouy = 6*6;
double CBand::ms_dMingleBouyDist = 15*15;
double CBand::ms_dExplosionVelocity = 180.0;
double CBand::ms_dMaxMarchVel = 30.0;
double CBand::ms_dMaxRunVel = 80.0;
int32_t CBand::ms_lMingleTime = 400;
int16_t CBand::ms_sStartingHitPoints = 100;
SampleMaster::SoundInstance CBand::ms_siBandSongInstance = 0;
CBand* CBand::ms_bandLeader = nullptr;		// The person who adjusts the band sound
																	// volume or IdNil.

// Let this auto-init to 0
int16_t CBand::ms_sFileCount;

// This value indicates whether the marchers have stopped playing in this level.
bool	CBand::ms_bDonePlaying	= false;

CBand::CBand(void)
{
  m_ucNextBouyID = 1;
  m_ucDestBouyID = 1;
  m_bCivilian = true;
}

CBand::~CBand(void)
{
  realm()->m_smashatorium.Remove(&m_smash);

  // Free resources
  FreeResources();

  // If sample playing . . .
  if (ms_siBandSongInstance != 0)
  {
    AbortSample(ms_siBandSongInstance);
    ms_siBandSongInstance = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CBand::Load(					// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,						// In:  File to load from
	bool bEditMode,					// In:  True for edit mode, false otherwise
	int16_t sFileCount,					// In:  File count (unique per file, never 0)
	uint32_t	ulFileVersion)				// In:  Version of file format to load.
{
	int16_t sResult = SUCCESS;

	// Call the base class load to get the instance ID, position, motion etc.
	sResult	= CDoofus::Load(pFile, bEditMode, sFileCount, ulFileVersion);
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
					break;
				}

			// Clear this so we know no one has paniced yet.
			// Note that, if no band members are loaded (i.e., they are all
			// created during play (maybe via a dispenser, although dispensers
			// use load) ), this won't get reset.
			ms_bDonePlaying		= false;
			}

      uint16_t idChildItem = 0;
		// Load Rocket Man specific data
			switch (ulFileVersion)
			{
				default:
				case 37:
               pFile->Read(&m_ucDestBouyID);
				case 36:
				case 35:
				case 34:
				case 33:
				case 32:
				case 31:
				case 30:
				case 29:
				case 28:
				case 27:
				case 26:
				case 25:
				case 24:
				case 23:
				case 22:
				case 21:
				case 20:
				case 19:
				case 18:
				case 17:
				case 16:
				case 15:
				case 14:
				case 13:
				case 12:
				case 11:
				case 10:
				case 9:
				case 8:
				case 7:
				case 6:
				case 5:
				case 4:
               pFile->Read(&idChildItem);
				case 3:
				case 2:
				case 1:
               pFile->Read(reinterpret_cast<uint8_t*>(&m_eWeaponType));
					pFile->Read(&m_ucNextBouyID);
					break;
			}

         m_child = realm()->GetOrAddThingById<CItem3d>(idChildItem);
			
		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == SUCCESS)
		{
			// Get resources
			sResult = GetResources();
		}
		else
		{
			sResult = FAILURE;
			TRACE("CBand::Load(): Error reading from file!\n");
		}
	}
	else
	{
	TRACE("CGrenader::Load(): CDoofus::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CBand::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	int16_t sFileCount)										// In:  File count (unique per file, never 0)
{
	int16_t sResult = SUCCESS;

	// Call the base class save to save the instance ID, position etc.
	CDoofus::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
	{
		ms_sFileCount = sFileCount;

		// Save static data
	}

	// Save band member specific data
   pFile->Write(&m_ucDestBouyID);
   uint16_t child_id = child() ? child()->GetInstanceID() : UINT16_MAX;
   pFile->Write(child_id);
   pFile->Write(reinterpret_cast<uint8_t*>(&m_eWeaponType));
	pFile->Write(&m_ucNextBouyID);

	if (!pFile->Error())
	{
		sResult = SUCCESS;
	}
	else
	{
		TRACE("CBand::Save() - Error writing to file\n");
		sResult = FAILURE;
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Init - Call this after the resources are in place
////////////////////////////////////////////////////////////////////////////////

int16_t CBand::Init(void)
{
	int16_t sResult = SUCCESS;

	// Prepare shadow (get resources and setup sprite).
	sResult	= PrepareShadow();

	// Init position, rotation and velocity
	m_dVel = 0.0;
   rotation.y = 0.0;
   m_lPrevTime = realm()->m_time.GetGameTime();
	m_state = CCharacter::State_Idle;
   m_lTimer = realm()->m_time.GetGameTime() + 500;
	m_sBrightness = 0;	// Default brightness

	m_smash.m_bits		= CSmash::Civilian | CSmash::Character;
   m_smash.m_pThing = this;

	m_lAnimTime = 0;
	m_panimCur = &m_animMarch;
	m_stockpile.m_sHitPoints = ms_sStartingHitPoints;

	// Set them facing their first bouy so they are lined up ready to march
//	m_ucDestBouyID = 1;		// This is the end of the parade route bouy
//	m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);
	m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
//	ASSERT(m_pNextBouy);
   if (m_pNextBouy)
		{
		m_sNextX = m_pNextBouy->GetX();
		m_sNextZ = m_pNextBouy->GetZ();
   //	rotation.y = rspATan(position.z - m_sNextZ, m_sNextX - position.x);
		AlignToBouy();
		}
	else
		{
		TRACE("Init():  Where's the dang, blam, dangin, blamin, BOUY?!\n");
		sResult = FAILURE;
		}

	m_state = CCharacter::State_March;
	m_dAcc = 150;

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
void CBand::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
// If not a violent locale . . . 
#if !VIOLENT_LOCALE
	// We must kill band members in these countries b/c of their lack of tolerance.
        Object::enqueue(SelfDestruct);
        return;
#else

	// Set the current height, previous time, and Nav Net by calling the
	// base class startup
	CDoofus::Startup();

	// Init other stuff
   Init();
#endif
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CBand::Update(void)
{
	int16_t sHeight = m_sPrevHeight;
	double dNewX;
	double dNewY;
	double dNewZ;
	double dX;
	double dZ;
	double dStartX;
	double dStartZ;
   milliseconds_t lThisTime;
   milliseconds_t lTimeDifference;

	if (!m_sSuspend)
	{
		// Get new time
      lThisTime = realm()->m_time.GetGameTime();
		lTimeDifference = lThisTime - m_lPrevTime;

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		// Check for new messages that may change the state
		ProcessMessages();

		// Increment animation time
		m_lAnimTime += lTimeDifference;

		// Check the current state
		switch (m_state)
		{
        UNHANDLED_SWITCH;

//-----------------------------------------------------------------------
// March - follow the parade route until you get to the end
//-----------------------------------------------------------------------

			case State_March:
				// The first guy should start the song, if we are not done playing music . . .
				if (ms_siBandSongInstance == 0 && ms_bDonePlaying == false)
				{
					PlaySample(										// Returns nothing.
																		// Does not fail.
						g_smidParadeSong,							// In:  Identifier of sample you want played.
						SampleMaster::Unspecified,				// In:  Sound Volume Category for user adjustment
						255,											// In:  Initial Sound Volume (0 - 255)
						&ms_siBandSongInstance,					// Out: Handle for adjusting sound volume
						nullptr,											// Out: Sample duration in ms, if not nullptr.
						0,												// In:  Where to loop back to in milliseconds.
																		//	-1 indicates no looping (unless m_sLoop is
																		// explicitly set).
						-1,											// In:  Where to loop back from in milliseconds.
																		// In:  If less than 1, the end + lLoopEndTime is used.
						false);										// In:  Call ReleaseAndPurge rather than Release after playing

					// Make this guy the band leader.
               ms_bandLeader = this;
				}

				// If I am the band leader . . .
            if (ms_bandLeader == this)
				{
					// If the band song is running . . .
					if (ms_siBandSongInstance != 0)
						{
						// Adjust the sound volume.  This doesn't need to be exact.  So
						// his previous position will be fine.
						SetInstanceVolume(
							ms_siBandSongInstance,
                     DistanceToVolume(position.x, position.y, position.z, BAND_SONG_HALF_LIFE) );
						}
				}

				// Check distance to target bouy
            dX = position.x - m_sNextX;
            dZ = position.z - m_sNextZ;
				if ((dX*dX + dZ*dZ) < ms_dCloseToBouy)
				{
					// Set next bouy, x, z, and rotation
					m_ucNextBouyID = m_pNextBouy->NextRouteNode(m_ucDestBouyID);
					if (m_ucNextBouyID == 0)
					{
						// Note that we're done playing music.
						ms_bDonePlaying	= true;

						m_lTimer = lThisTime;
						m_state = State_Wait;
						// At the end of the parade, end the song
						if (ms_siBandSongInstance != 0)
						{
							AbortSample(ms_siBandSongInstance);
							ms_siBandSongInstance = 0;
							ms_bDonePlaying		= true;
						}
					}
					else
					{
						m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
						m_sNextX = m_pNextBouy->GetX();
						m_sNextZ = m_pNextBouy->GetZ();
//						rotation.y = rspATan(position.z - m_sNextZ, m_sNextX - position.x);
						AlignToBouy();
					}
				}
				// Move towards the bouy
				AlignToBouy();
				UpdateVelocities(dSeconds, ms_dMaxMarchVel, ms_dMaxMarchVel);
				GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);
				if (MakeValidPosition(&dNewX, &dNewY, &dNewZ, 10) == true)
				{
				// Update Values /////////////////////////////////////////////////////////

               position.x	= dNewX;
               position.y	= dNewY;
               position.z	= dNewZ;

					UpdateFirePosition();
				}
				else
				{
				// Restore Values ////////////////////////////////////////////////////////
	
					m_dVel			-= m_dDeltaVel;
				}
		
				break;

//-----------------------------------------------------------------------
// Wait - stand around for a while
//-----------------------------------------------------------------------

			case State_Wait:
				// See if its time to go yet
				if (lThisTime > m_lTimer)
				{
					m_state = State_Mingle;
					m_ucDestBouyID = SelectRandomBouy();
               m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);
					m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
					m_lTimer = lThisTime + ms_lMingleTime;
               if (m_ucDestBouyID == 0 || !m_pNextBouy)
					{
						m_state = State_Wait;
					}
					else
					{
						m_ucNextBouyID = m_pNextBouy->NextRouteNode(m_ucDestBouyID);
						m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
                  if (m_pNextBouy)
						{
							m_sNextX = m_pNextBouy->GetX();
							m_sNextZ = m_pNextBouy->GetZ();
//							rotation.y = rspATan(position.z - m_sNextZ, m_sNextX - position.x);
							AlignToBouy();
							m_dAcc = 150;
							m_state = State_Mingle;
						}
						else
							m_state = State_Wait;
					}
				}
				break;

//-----------------------------------------------------------------------
// Mingle - Mingle around at the park after the parade
//-----------------------------------------------------------------------
/*
			case State_Mingle:
				// Check distance to target bouy
            dX = position.x - m_sNextX;
            dZ = position.z - m_sNextZ;
				if ((dX*dX + dZ*dZ) < ms_dMingleBouyDist)
				{
					m_lTimer = lThisTime + ms_lMingleTime;
					m_state = State_Wait;
				}
				// Move towards the bouy
				AlignToBouy();
				UpdateVelocities(dSeconds, ms_dMaxMarchVel, ms_dMaxMarchVel);
				GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);
				if (MakeValidPosition(&dNewX, &dNewY, &dNewZ, 10) == true)
				{
				// Update Values /////////////////////////////////////////////////////////

               position.x	= dNewX;
               position.y	= dNewY;
               position.z	= dNewZ;

					UpdateFirePosition();
				}
				else
				{
				// Restore Values ////////////////////////////////////////////////////////
	
					m_dVel			-= m_dDeltaVel;
				}
					
				break;
*/
//-----------------------------------------------------------------------
// Panic - Pick a random bouy and run to it.  When you are there, pick
//			  a different random bouy and run to it.  
//-----------------------------------------------------------------------

			case State_Panic:
			case State_Mingle:
				// Check distance to target bouy
            dStartX = position.x;
            dStartZ = position.z;
            dX = position.x - m_sNextX;
            dZ = position.z - m_sNextZ;

				// BEGIN TEMP.
				LOG(dX, GetInstanceID() );
				LOG(dZ, GetInstanceID() );

            LOG(position.x, GetInstanceID() );
            LOG(position.z, GetInstanceID() );

				LOG(m_sNextX, GetInstanceID() );
				LOG(m_sNextZ, GetInstanceID() );
				// END TEMP.

				if ((dX*dX + dZ*dZ) < ms_dCloseToBouy)
				{
					// Set next bouy, x, z, and rotation
					m_ucNextBouyID = m_pNextBouy->NextRouteNode(m_ucDestBouyID);
					// BEGIN TEMP.
					LOG(m_pNextBouy->m_ucID, GetInstanceID() );
					LOG(m_ucDestBouyID, GetInstanceID() );
					LOG(m_ucNextBouyID, GetInstanceID() );
					// END TEMP.

					if (m_ucNextBouyID == 0 || m_ucNextBouyID == 255)
					{
						if (m_panimCur != &m_animRun)
							m_panimCur = &m_animRun;
						m_ucDestBouyID = SelectRandomBouy();
						m_ucNextBouyID = m_pNextBouy->NextRouteNode(m_ucDestBouyID);
						m_sNextX = m_pNextBouy->GetX();
						m_sNextZ = m_pNextBouy->GetZ();
						AlignToBouy();
						// BEGIN TEMP.
						LOG(m_ucDestBouyID, GetInstanceID() );
						LOG(m_ucNextBouyID, GetInstanceID() );
						LOG(m_sNextX, GetInstanceID() );
						LOG(m_sNextZ, GetInstanceID() );
						// END TEMP.

						m_sRotateDir = GetRand() % 2;
						if (m_state == State_Mingle)
						{
							m_state = State_Wait;
							m_lTimer = lThisTime + ms_lMingleTime;
						}
						else
						{
							int16_t sRandom = GetRand() % 16;
							switch (sRandom)
							{
								case 0:
									PlaySample(g_smidSteveAhFire, SampleMaster::Voices);
									break;

								case 1:
									PlaySample(g_smidBlownupFemaleYell, SampleMaster::Voices);
									break;

								case 2:
									PlaySample(g_smidCarynScream, SampleMaster::Voices);
									break;

								case 3:
									PlaySample(g_smidTinaScream1, SampleMaster::Voices);
									break;

								case 4:
									PlaySample(g_smidPaulAhah, SampleMaster::Voices);
									break;

								case 5:
									PlaySample(g_smidTinaOhMyGod, SampleMaster::Voices);
									break;

								case 6:
									PlaySample(g_smidAndreaHesPostal, SampleMaster::Voices);
									break;

								case 7:
									PlaySample(g_smidAndreaHesManiac, SampleMaster::Voices);
									break;

								case 8:
									PlaySample(g_smidCelinaRun, SampleMaster::Voices);
									break;

								case 9:
									PlaySample(g_smidPaulCantFeelLegs, SampleMaster::Voices);
									break;

								case 10:
									PlaySample(g_smidAndreaYell, SampleMaster::Voices);
									break;

								case 11:
									PlaySample(g_smidSteveWaFire, SampleMaster::Voices);
									break;

								case 12:
									PlaySample(g_smidRandyCantFeelLegs, SampleMaster::Voices);
									break;

								case 13:
									PlaySample(g_smidSteveMyEyes, SampleMaster::Voices);
									break;

								case 14:
									PlaySample(g_smidSteveMyLeg, SampleMaster::Voices);
									break;

								case 15:
									PlaySample(g_smidSteveCantSeeAny, SampleMaster::Voices);
									break;
							}
						}
					}
					else
					{
						m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
                  if (m_pNextBouy)
						{
							m_sNextX = m_pNextBouy->GetX();
							m_sNextZ = m_pNextBouy->GetZ();
							AlignToBouy();
						}
					}
				}

				if (lThisTime > m_lAlignTimer)
					AlignToBouy();

				// Move towards the bouy
//				DeluxeUpdatePosVel();
				if (m_state == State_Mingle)
					UpdateVelocities(dSeconds, ms_dMaxMarchVel, ms_dMaxMarchVel);
				else
					UpdateVelocities(dSeconds, ms_dMaxRunVel, ms_dMaxRunVel);
				GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);

				// Get height and 'no walk' status at new position.
				bool		bNoWalk;
            sHeight	= realm()->GetHeightAndNoWalk(dNewX, dNewY, &bNoWalk);

				// If too big a height difference or completely not walkable . . .
				if (bNoWalk == true
					|| (sHeight - dNewY > 10) )// && m_bAboveTerrain == false && m_dExtHorzVel == 0.0))
				{
					m_ucDestBouyID = SelectRandomBouy();
               m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);
					m_sNextX = m_pNextBouy->GetX();
					m_sNextZ = m_pNextBouy->GetZ();
					m_lAlignTimer = lThisTime + 3000;
					AlignToBouy();
				}

				if (MakeValidPosition(&dNewX, &dNewY, &dNewZ, 10) == true)
				{
				// Update Values /////////////////////////////////////////////////////////

               position.x	= dNewX;
               position.y	= dNewY;
               position.z	= dNewZ;

					UpdateFirePosition();
				}
				else
				{
				// Restore Values ////////////////////////////////////////////////////////
	
					m_dVel			-= m_dDeltaVel;
					m_ucDestBouyID = SelectRandomBouy();
               m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);
					m_sNextX = m_pNextBouy->GetX();
					m_sNextZ = m_pNextBouy->GetZ();
					m_lAlignTimer = lThisTime + 3000;
               rotation.y = rspMod360(rotation.y + 20);
				}

				// If not moving when you are trying to, rotate
            if (position.x == dStartX && position.z == dStartZ)
				{
					if (m_sRotateDir)
                  m_dAnimRot = rotation.y = rspMod360(rotation.y + 20);
					else
                  m_dAnimRot = rotation.y = rspMod360(rotation.y - 20);
				}
				else
				{
					m_sRotateDir = GetRand() % 2;
				}

				break;

//-----------------------------------------------------------------------
// Burning - Run around on fire until dead
//-----------------------------------------------------------------------

			case State_Burning:
				if (!WhileBurning())
				{
					m_state = State_Die;
					m_lAnimTime = 0;
					m_panimCur = &m_animShot;
					// Fall down in a more random direction.
               rotation.y = rspMod360(rotation.y - 90 + (GetRand() % 180));
				}				
				break;

//-----------------------------------------------------------------------
// Blown Up - Do motion into the air until you hit the ground again
//-----------------------------------------------------------------------

			case State_BlownUp:
				if (!WhileBlownUp())
					m_state = State_Dead;
				else
					UpdateFirePosition();
				break;

//-----------------------------------------------------------------------
// Shot - Dies in one shot
//-----------------------------------------------------------------------

			case State_Shot:
				if (!WhileShot())
				{
					m_state = State_Die;
               rotation.y = rspMod360(rotation.y - 90 + (GetRand() % 180));
				}
				break;

//-----------------------------------------------------------------------
// Die - run die animation until done, the you are dead
//-----------------------------------------------------------------------

			case State_Die:
				if (!WhileDying())
					m_state = State_Dead;
				else
					UpdateFirePosition();
				
				break;

//-----------------------------------------------------------------------
// Dead - paste yourself in the background and delete yourself
//-----------------------------------------------------------------------

			case State_Dead:
				GameMessage msg;
				msg.msg_Death.eType = typeDeath;
				msg.msg_Death.sPriority = 0;
            auto list = realm()->GetThingsByType(CDemonID);
            if (!list.empty())
               SendThingMessage(msg, list.front());
				OnDead();
            Object::enqueue(SelfDestruct);
            return;
		}

      m_smash.m_sphere.sphere.X			= position.x;
		// Fudge center of sphere as half way up the dude.
		// Doesn't work if dude's feet leave the origin.
      m_smash.m_sphere.sphere.Y			= position.y + m_sRadius;
      m_smash.m_sphere.sphere.Z			= position.z;
      m_smash.m_sphere.sphere.lRadius	= m_sRadius;

		// Update the smash.
      realm()->m_smashatorium.Update(&m_smash);

		// Save height for next time
		m_sPrevHeight = sHeight;

		// Save time for next time
		m_lPrevTime = lThisTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CBand::Render(void)
{
	// Call base class.
	CDoofus::Render();

	// Update child, if any . . .
   if (child())
   {

     managed_ptr<CThing3d> child3d = child();
			// Set transform from our rigid body transfanimation for the child
			// sprite.
         child3d->m_ptrans = &m_panimCur->m_ptransRigid->atTime(m_lAnimTime);
			// If the item is not our child . . .
         if (child3d->m_psprParent != this)
				{
				// Make it so.
            AddChild(child3d.pointer());
            }
	}
}

#if !defined(EDITOR_REMOVED)
////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
int16_t CBand::EditNew(									// Returns 0 if successfull, non-zero otherwise
	int16_t sX,												// In:  New x coord
	int16_t sY,												// In:  New y coord
	int16_t sZ)												// In:  New z coord
{
	int16_t sResult = SUCCESS;

	// Call the base class to place the item.
	sResult = CDoofus::EditNew(sX, sY, sZ);

	if (sResult == SUCCESS)
	{
		// Load resources
		sResult = GetResources();
		if (sResult == SUCCESS)
		{
			sResult	= Init();
		}
	}
	
	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// EditModify - Show dialog box for selecting starting bouy
////////////////////////////////////////////////////////////////////////////////

int16_t CBand::EditModify(void)
{
	int16_t sResult = SUCCESS;
	RGuiItem* pGui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/band.gui"));
	if (pGui)
	{
		RGuiItem*	pguiStartBouy = pGui->GetItemFromId(10);
		RGuiItem*	pguiDestBouy  = pGui->GetItemFromId(11);

		if (pguiStartBouy)
		{
			RSP_SAFE_GUI_REF_VOID(pguiStartBouy, SetText("%d", m_ucNextBouyID));
			RSP_SAFE_GUI_REF((REdit*) pguiStartBouy, m_sCaretPos = strlen(pguiStartBouy->m_szText));
			RSP_SAFE_GUI_REF_VOID(pguiStartBouy, Compose());
			
			RSP_SAFE_GUI_REF_VOID(pguiDestBouy, SetText("%d", m_ucDestBouyID));
			RSP_SAFE_GUI_REF((REdit*) pguiDestBouy, m_sCaretPos = strlen(pguiDestBouy->m_szText));
			RSP_SAFE_GUI_REF_VOID(pguiDestBouy, Compose());

			CItem3d::ItemType	itChild	= CItem3d::None;
			// If there's currently a child . . .

         if (child())
            itChild	= managed_ptr<CItem3d>(child())->m_itemType;

			RListBox*	plbChildTypes	= (RListBox*)pGui->GetItemFromId(3);
			if (plbChildTypes != nullptr)
				{
				ASSERT(plbChildTypes->m_type == RGuiItem::ListBox);
				
				// Add all built-in 3D Item types.
				int16_t	i;
				RGuiItem*	pguiItem;
				for (i = CItem3d::None; i < CItem3d::NumTypes; i++)
					{
					// Don't allow Custom . . .
					if (i != CItem3d::Custom)
						{
						pguiItem	= plbChildTypes->AddString(CItem3d::ms_apszKnownAnimDescriptions[i]);
						if (pguiItem != nullptr)
							{
							// Set item number.
							pguiItem->m_ulUserData	= i;
							// If this item is the current item type . . .
                     if (i == itChild)
								{
								plbChildTypes->SetSel(pguiItem);
								}
							}
						}
					}

				plbChildTypes->AdjustContents();
				}

			if (DoGui(pGui) == GUI_ID_OK)
			{
				m_ucNextBouyID = RSP_SAFE_GUI_REF(pguiStartBouy, GetVal());
				m_ucDestBouyID = RSP_SAFE_GUI_REF(pguiDestBouy, GetVal());
				if (plbChildTypes != nullptr)
					{
					RGuiItem*	pguiSel	= plbChildTypes->GetSel();
					if (pguiSel != nullptr)
						{
						itChild	= (CItem3d::ItemType)pguiSel->m_ulUserData;
						}
					else
						{
						// None.
						itChild	= CItem3d::None;
						}

               if (child())
                  {
						// If it is not of the new type . . .
                  if (managed_ptr<CItem3d>(child())->m_itemType != itChild)
                     {
                    managed_ptr<CThing3d> child3d = child();
							// Disable item.
							DetachChild(
                        child3d,
                        m_panimCur->m_ptransRigid->atTime(m_lAnimTime) );
							// Be gone.
                      Object::enqueue(SelfDestruct);
							}
						}

					// If there is no child item . . .
               if (!child())
						{
						// If a child is desired . . .
						if (itChild != CItem3d::None)
							{
                    managed_ptr<CItem3d> pitem2 = realm()->AddThing<CItem3d>();
                        if (pitem2)
								{
								// Remember who our child is.
                        setChild(pitem2);
								// Setup the child.
                        pitem2->Setup(0, 0, 0, itChild, nullptr, this);
								}
							else
								{
								TRACE("EditModify(): ConstructWithID failed for CItem3d.\n");
								}
							}
						}
					}
			}
			else
			{
				// User Abort
				sResult = 1;
			}	
		}
	}
	delete pGui;

	return sResult;

}
#endif // !defined(EDITOR_REMOVED)

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
int16_t CBand::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
  bool bResult = true;
  bResult &= m_animRun.Get("bandg_run", 0, "instrument");
  bResult &= m_animStand.Get("bandg_stand", 0, "instrument");
  bResult &= m_animMarch.Get("bandg_march", 0, "instrument");
  bResult &= m_animShot.Get("bandg_shot", 0, "instrument");
  bResult &= m_animShot.Get("bandg_blownup", 0, "instrument");
  bResult &= m_animOnFire.Get("bandg_onfire", 0, "instrument");
  return bResult ? SUCCESS : FAILURE;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
int16_t CBand::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_animRun.Release();
	m_animStand.Release();
	m_animMarch.Release();
	m_animShot.Release();
	m_animBlownup.Release();
	m_animOnFire.Release();
		
	return SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
// ProcessMessages - Similar to the base class version but handles a few more
////////////////////////////////////////////////////////////////////////////////

void CBand::ProcessMessages(void)
{
   while (!m_MessageQueue.empty())
	{
     GameMessage& msg = m_MessageQueue.front();
		ProcessMessage(&msg);

      if(msg.msg_Generic.eType == typePanic)
        OnPanicMsg(&(msg.msg_Panic));

      m_MessageQueue.pop_front();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Message handlers
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Shot Message
////////////////////////////////////////////////////////////////////////////////

void CBand::OnShotMsg(Shot_Message* pMessage)
{
	if (m_state != State_BlownUp &&
	    m_state != State_Shot &&
		 m_state != State_Die &&
		 m_state != State_Dead)
	{
		CCharacter::OnShotMsg(pMessage);

		// Start shot animation if he hasn't already.
		m_lAnimTime = 0;
		m_panimCur = &m_animShot;
		switch (GetRand() % 8)
		{
			case 0:
				PlaySample(g_smidAmyMyEyes, SampleMaster::Voices);
				break;

			case 1:
				PlaySample(g_smidAndreaMyLeg, SampleMaster::Voices);
				break;

			case 2:
				PlaySample(g_smidBillGrunt, SampleMaster::Voices);
				break;

			case 3:
				PlaySample(g_smidMikeGrunt, SampleMaster::Voices);
				break;

			case 4:
				PlaySample(g_smidPaulCantFeelLegs, SampleMaster::Voices);
				break;

			case 5:
				PlaySample(g_smidRandyHuu, SampleMaster::Voices);
				break;

			case 6:
				PlaySample(g_smidRandyUg, SampleMaster::Voices);
				break;

			case 7:
				PlaySample(g_smidSteveUrl, SampleMaster::Voices);
				break;
		}
		m_state = State_Shot;
		// Dies in one shot
		m_stockpile.m_sHitPoints =0; 
		AlertBand();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Explosion message
////////////////////////////////////////////////////////////////////////////////

void CBand::OnExplosionMsg(Explosion_Message* pMessage)
{
  if (m_state != State_BlownUp)
  {
    CCharacter::OnExplosionMsg(pMessage);
    managed_ptr<CThing3d> child3d = child();
    // Drop item, if we have one still.
    DetachChild(
          child3d,
          m_panimCur->m_ptransRigid->atTime(m_lAnimTime));
    // If we got something back . . .
    if (child3d)
    {
      // Let it know about the explosion.
      GameMessage msg;
      msg.msg_Explosion	= *pMessage;

      SendThingMessage(msg, child());
    }

    // Explosion kills the guy
    m_stockpile.m_sHitPoints = 0;
    m_state = State_BlownUp;
    m_lAnimTime = 0;
    m_panimCur = &m_animBlownup;
    switch (GetRand() % 8)
    {
      case 0:
        PlaySample(g_smidBlownupFemaleYell, SampleMaster::Voices);
        break;

      case 1:
        PlaySample(g_smidCarynScream, SampleMaster::Voices);
        break;

      case 2:
        PlaySample(g_smidTinaScream1, SampleMaster::Voices);
        break;

      case 3:
        PlaySample(g_smidPaulAhah, SampleMaster::Voices);
        break;

      case 4:
        PlaySample(g_smidScottYell1, SampleMaster::Voices);
        break;

      case 5:
        PlaySample(g_smidScottYell2, SampleMaster::Voices);
        break;

      case 6:
        PlaySample(g_smidMikeOhh, SampleMaster::Voices);
        break;

      case 7:
        PlaySample(g_smidSteveAhBlowup, SampleMaster::Voices);
        break;
    }
    AlertBand();
    GameMessage msg;
    msg.msg_Explosion.eType = typeExplosion;
    msg.msg_Explosion.sPriority = 0;
    auto list = realm()->GetThingsByType(CDemonID);
    if (!list.empty())
      SendThingMessage(msg, list.front());
  }
}

////////////////////////////////////////////////////////////////////////////////
// Burning message
////////////////////////////////////////////////////////////////////////////////

void CBand::OnBurnMsg(Burn_Message* pMessage)
{
    UnlockAchievement(ACHIEVEMENT_FIREBOMB_THE_BAND);

	CCharacter::OnBurnMsg(pMessage);
	m_stockpile.m_sHitPoints -= pMessage->sDamage;

	if (m_state != State_Burning &&
	    m_state != State_BlownUp &&
		 m_state != State_Die &&
		 m_state != State_Dead)
	{
		m_state = State_Burning;
		m_panimCur = &m_animOnFire;
		m_lAnimTime = 0;
		switch (GetRand() % 8)
		{
			case 0:
				PlaySample(g_smidAmyScream, SampleMaster::Voices);
				break;

			case 1:
				PlaySample(g_smidTinaScream2, SampleMaster::Voices);
				break;

			case 2:
				PlaySample(g_smidTinaScream3, SampleMaster::Voices);
				break;

			case 3:
				PlaySample(g_smidMikeAhh, SampleMaster::Voices);
				break;

			case 4:
				PlaySample(g_smidSteveAhFire, SampleMaster::Voices);
				break;

			case 5:
				PlaySample(g_smidSteveWaFire, SampleMaster::Voices);
				break;

			case 6:
				PlaySample(g_smidAndreaHelp, SampleMaster::Voices);
				break;

			case 7:
				PlaySample(g_smidCarynScream, SampleMaster::Voices);
				break;
		}
		AlertBand();
		GameMessage msg;
		msg.msg_Burn.eType = typeBurn;
		msg.msg_Burn.sPriority = 0;
      auto list = realm()->GetThingsByType(CDemonID);
      if (!list.empty())
         SendThingMessage(msg, list.front());
	}
}

////////////////////////////////////////////////////////////////////////////////
// Panic message
////////////////////////////////////////////////////////////////////////////////

void CBand::OnPanicMsg(Panic_Message* pMessage)
{
  UNUSED(pMessage);
	if (m_state != State_Die &&
	    m_state != State_Dead &&
		 m_state != State_BlownUp &&
		 m_state != State_Shot &&
		 m_state != State_Burning &&
		 m_state != State_Panic)
	{
		m_state = State_Panic;
		m_panimCur = &m_animOnFire;
      m_lAnimTime = GetRand() % m_panimCur->m_psops->totalTime;
		// Pick a random bouy to run to
		m_ucDestBouyID = SelectRandomBouy();
      m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);
		m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
		if (m_pNextBouy)
		{
			m_sNextX = m_pNextBouy->GetX();
			m_sNextZ = m_pNextBouy->GetZ();
			AlignToBouy();
		}
		int16_t sRandom = GetRand() % 6;
		switch (sRandom)
		{
			case 0:
				PlaySample(g_smidScottYell1, SampleMaster::Voices);
				break;

			case 1:
				PlaySample(g_smidBlownupFemaleYell, SampleMaster::Voices);
				break;

			case 2:
				PlaySample(g_smidTinaScream1, SampleMaster::Voices);
				break;

			default:
				break;
		}

	}
}

////////////////////////////////////////////////////////////////////////////////
// AlertBand
////////////////////////////////////////////////////////////////////////////////

void CBand::AlertBand(void)
{
	GameMessage msg;
#ifdef UNUSED_VARIABLES
	GameMessage msgStopSound;

	msgStopSound.msg_ObjectDelete.eType = typeObjectDelete;
	msgStopSound.msg_ObjectDelete.sPriority = 0;
#endif

	msg.msg_Panic.eType = typePanic;
	msg.msg_Panic.sPriority = 0;
   msg.msg_Panic.sX = (int16_t) position.x;
   msg.msg_Panic.sY = (int16_t) position.y;
   msg.msg_Panic.sZ = (int16_t) position.z;

   for(const managed_ptr<CThing>& pThing : realm()->GetThingsByType(CBandID))
   {
     if(pThing != this)
       SendThingMessage(msg, pThing);
   }
	if (ms_siBandSongInstance != 0)
	{
		AbortSample(ms_siBandSongInstance);
		ms_siBandSongInstance = 0;
		ms_bDonePlaying	= true;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Implements basic one-time functionality for each time State_Dead is
// entered.
////////////////////////////////////////////////////////////////////////////////
void CBand::OnDead(void)
	{
	// Drop item.  This does nothing if we've already dropped it.
	DropItem();

	// Call base class.
	CDoofus::OnDead();
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while dying and returns true
// until the state is completed.
// (virtual -- Overriden here)
////////////////////////////////////////////////////////////////////////////////
bool CBand::WhileDying(void)	// Returns true until state is complete.
	{
	// Drop item.  This does nothing if we've already dropped it.
	DropItem();

	// Call base class.
	return CDoofus::WhileDying();
	}


////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while being shot and returns true
// until the state is completed.
// (virtual -- Overriden here)
////////////////////////////////////////////////////////////////////////////////
bool CBand::WhileShot(void)	// Returns true until state is complete.
	{
	// Drop item.  This does nothing if we've already dropped it.
	DropItem();

	// Call base class.
	return CDoofus::WhileShot();
	}

		
////////////////////////////////////////////////////////////////////////////////
// Drop item and apply appropriate forces.
////////////////////////////////////////////////////////////////////////////////
void CBand::DropItem(void)	// Returns nothing.
{
  // If we still have the child item . . .
  if (child())
  {
    managed_ptr<CThing3d> child3d = child();
    // Drop it.
    DetachChild(
          child3d,
          m_panimCur->m_ptransRigid->atTime(m_lAnimTime) );

    // Send it spinning.
    child3d->m_dExtRotVelY = GetRand() % 720;
    child3d->m_dExtRotVelZ = GetRand() % 720;
    // Send it forward (from our perspective)
    // with our current velocity.
    child3d->m_dExtHorzRot = rotation.y;
    child3d->m_dExtHorzVel = m_dVel;
    // ... and air drag.
    if (child3d->m_dExtHorzVel > 0.0)
    {
      child3d->m_dExtHorzDrag = -ms_dDefaultAirDrag;
    }
    else if (child3d->m_dExtHorzVel < 0.0)
    {
      child3d->m_dExtHorzDrag = ms_dDefaultAirDrag;
    }

    // Similar enough to blown up.
    child3d->m_state = State_BlownUp;
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
