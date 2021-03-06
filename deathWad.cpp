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
// deathWad.cpp
// Project: Nostril (aka Postal)
//
// This module implements the CDeathWad weapon class which is an unguided
//	projectile.
// 
//
// History:
//		07/30/97 JMI	Started this weapon object from the CRocket.
//
//		08/07/97	JMI	Added additional parameter to CAnim3D::Get() call.
//
//		08/17/97	JMI	Changed m_pthingParent to m_idParent.
//
////////////////////////////////////////////////////////////////////////////////
//
// Wad vt wad-ded; wad-ding (1579)  1  a: to insert a wad into <~ a gun>
// b: to hold in by a wad <~ a bullet in a gun>  2: to form into a wad or
// wadding; esp : to roll or crush into a tight wad  3: to stuff or line with 
// some soft substance -- wad-der n
//
// This weapon is a wad of several ammunitions stuffed (or wadded) into a rocket
// cylinder and a napalm canister.  Fuel is used to propel the weapon, grenades 
// for extra (in addition to the rockets normal payload) explosive power, and 
// napalm for lasting fire(burn) power.  The rocket cylinder stores the fuel and
// provides the propulsion.  The napalm canister stores the extra explosive/fire
// powerload using some up with every collision.  It is for these reasons this
// weapon requires:
//  - exactly 1 rocket cylinder (including its original payload (solid fuel and 
//		explosive power))
//  - exactly 1 napalm canister (including its original payload (let's call it
//		liquid fire) )
//  - at least 1 canister fluid fuel (e.g., from flame thrower) (more provides
//		greater distance).
//  - at least 1 grenade (more provides greater explosive power over longer
//		distances).
//
////////////////////////////////////////////////////////////////////////////////

#include "deathWad.h"

#include "realm.h"
#include "dude.h"
#include "explode.h"
#include "fire.h"
#include "SampleMaster.h"
#include "fireball.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SMALL_SHADOW_FILE	"smallshadow.img"

// Define this if you want the empty missile casing (when there's no final 
// explosive power) to become a powerup flung through the air from the point
// at which it runs out of fuel.
//#define CAN_CHANGE_TO_POWERUP	1

// Gets a random between -range / 2 and range / 2.
#define RAND_SWAY(sway)	((GetRand() % sway) - sway / 2)

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are constant values!

// Internal acceleration.
const double	CDeathWad::ms_dAccInternal					= 350.0;
// Maximum forward velocity
const double	CDeathWad::ms_dMaxVelFore					= 350.0;
// Maximum backward velocity
const double	CDeathWad::ms_dMaxVelBack					= -350.0;
// Units moved each iteration while traversing the weapon path.
const double	CDeathWad::ms_dTraversalRate				= 3.0;		
// Distance between thrust feedbacks.
const double	CDeathWad::ms_dThrustDelta					= 9.0;
// Go off screen this far before blowing up
const int16_t		CDeathWad::ms_sOffScreenDist				= 200;
// Time for smoke to stick around.
const int32_t		CDeathWad::ms_lSmokeTimeToLive			= 500;
// Time for fireball to stick around.
const int32_t		CDeathWad::ms_lFireBallTimeToLive		= 500;
// Amount to stagger final explosions.
const int16_t		CDeathWad::ms_sFinalExplosionStagger	= 5;
// Radius of collision area (whether sphere or cylinder).
const int16_t		CDeathWad::ms_sCollisionRadius			= 30;
// Velocity for kick from launch.
const double	CDeathWad::ms_dKickVelocity				= 350.0;
// Max a WAD can hold.
const CStockPile CDeathWad::ms_stockpileMax				=
{
  0,  // m_sHitPoints

  5,  // m_sNumGrenades
  0,  // m_sNumFireBombs
  1,  // m_sNumMissiles
  1,  // m_sNumNapalms
  0,  // m_sNumBullets
  0,  // m_sNumShells
  50, // m_sNumFuel
  0,  // m_sNumMines
  0,  // m_sNumHeatseekers

  0,  // m_sMachineGun
  0,  // m_sMissileLauncher
  0,  // m_sShotGun
  0,  // m_sSprayCannon
  0,  // m_sFlameThrower
  0,  // m_sNapalmLauncher
  0,  // m_sDeathWadLauncher
  0,  // m_sDoubleBarrel

  0,  // m_sKevlarLayers

  0,  // m_sBackpack
};

// Let this auto-init to 0
int16_t CDeathWad::ms_sFileCount;




CDeathWad::CDeathWad(void)
{
  m_smash.m_pThing = this;
  m_siThrust						= 0;
  m_stockpile.Zero();
  m_bInsideTerrain				= false;
  m_u32CollideIncludeBits		= 0;
  m_u32CollideDontcareBits	= 0;
  m_u32CollideExcludeBits		= 0;
  m_dUnthrustedDistance		= 0.0;
}

CDeathWad::~CDeathWad(void)
{
  // Stop sound, if any.
  StopLoopingSample(m_siThrust);

  realm()->m_smashatorium.Remove(&m_smash);

  // Free resources
  FreeResources();
}


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CDeathWad::Load(										// Returns 0 if successfull, non-zero otherwise
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
		
		// Make sure there were no file errors
		if (!pFile->Error())
		{
			// Get resources
			sResult = GetResources();
		}
		else
		{
			sResult = FAILURE;
			TRACE("CDeathWad::Load(): Error reading from file!\n");
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CDeathWad::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	int16_t sFileCount)										// In:  File count (unique per file, never 0)
{
  UNUSED(pFile);
	// In most cases, the base class Save() should be called.  In this case it
	// isn't because the base class doesn't have a Save()!

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
	{
		ms_sFileCount = sFileCount;

		// Save static data
	}

	// Save object data

	return SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::Update(void)
{
	double dNewX;
	double dNewZ;

   ASSERT(rotation.y >= 0);
   ASSERT(rotation.y < 360);

	if (!m_sSuspend)
		{
		// Get new time
		int32_t lThisTime = realm()->m_time.GetGameTime(); 

		// Calculate elapsed time in seconds
		double dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;

		ProcessMessages();

		// Check the current state
		switch (m_eState)
      {
         UNHANDLED_SWITCH;

			case CWeapon::State_Hide:
			case CWeapon::State_Idle:
				break;

			case CWeapon::State_Fire:
				
				Launch();

				m_eState = State_Chase;
				break;

//-----------------------------------------------------------------------
// Chase
//-----------------------------------------------------------------------
			case CWeapon::State_Chase:
				{
				// Accelerate toward the target and check for proximity
				// and obstacles

				// Accelerate up to max velocity
				m_dHorizVel += ms_dAccInternal * dSeconds;

				// Limit to maximum velocity
				if (m_dHorizVel > ms_dMaxVelFore)
					m_dHorizVel = ms_dMaxVelFore;
				else if (m_dHorizVel < ms_dMaxVelBack)
					m_dHorizVel = ms_dMaxVelBack;

				// Adjust position based on velocity.
            dNewX = position.x + COSQ[(int16_t)rotation.y] * (m_dHorizVel * dSeconds);
            dNewZ = position.z - SINQ[(int16_t)rotation.y] * (m_dHorizVel * dSeconds);

				// If the new position is a ways off screen
            if (	position.z > ms_sOffScreenDist + realm()->GetRealmHeight()
               ||	position.z < -ms_sOffScreenDist
               ||	position.x > ms_sOffScreenDist + realm()->GetRealmWidth()
               ||	position.x < -ms_sOffScreenDist)
					{
					// Blow Up
					m_eState = CWeapon::State_Explode;
					}
				else 
					{
					// Traverse the path until we hit the newest position.
					while (TraversePath(		// Returns true, when destination reached; false,
													// if terrain change.                            
                  position.x,						// In:  Starting position.
                  position.y,						// In:  Starting position.
                  position.z,						// In:  Starting position.
						&m_bInsideTerrain,	// In:  true, if starting in terrain.            
													// Out: true, if ending in terrain.              
						dNewX,					// In:  Destination position.                    
						dNewZ,					// In:  Destination position.                    
                  &position.x,					// Out: Position of inside terrain status change.
                  &position.z)					// Out: Position of inside terrain status change.
						== false)
						{
						// Explosion at this point.
						Explosion();
						}
					}

				// If we have any charge left . . .
				if (m_stockpile.m_sNumGrenades)
					{
					// If we hit someone . . .
					CSmash* pSmashed = nullptr;
					if (realm()->m_smashatorium.QuickCheck(
						&m_smash, 
						m_u32CollideIncludeBits, 
						m_u32CollideDontcareBits,
						m_u32CollideExcludeBits, 
						&pSmashed) == true)
						{
						ASSERT(pSmashed->m_pThing);
						// Protect the launcher of the death wad . . .
                  if (pSmashed->m_pThing != m_shooter)
							{
							m_stockpile.m_sNumGrenades--;
							Explosion();
							}
						}
					}

				// If out of fuel . . .
				if (m_stockpile.m_sNumFuel <= 0)
					{
					// Blow Up
					m_eState = CWeapon::State_Explode;
					}

				// Update sound position.
				int16_t	sVolumeHalfLife	= LaunchSndHalfLife;
				// If inside terrain . . .
				if (m_bInsideTerrain == true)
					{
					// Half half life.
					sVolumeHalfLife	/= 4;
					}

            SetInstanceVolume(m_siThrust, DistanceToVolume(position.x, position.y, position.z, sVolumeHalfLife) );

				// If no rocket or napalm canister . . .
				if (m_stockpile.m_sNumMissiles < 0 || m_stockpile.m_sNumNapalms < 0)
					{
					// Blow Up.
					m_eState	= CWeapon::State_Explode;
					}

				break;
				}

//-----------------------------------------------------------------------
// Explode
//-----------------------------------------------------------------------
			case CWeapon::State_Explode:

				// Start explosion objects and then kill deathwad
				// object.
#ifdef CAN_CHANGE_TO_POWERUP
				if (m_stockpile.m_sNumGrenades > 0)
					{
#endif
					while (m_stockpile.m_sNumGrenades > 0 || m_stockpile.m_sNumMissiles > 0)
						{
						Explosion();

						// Stagger.
                  position.x	+= RAND_SWAY(ms_sFinalExplosionStagger);
                  position.z	+= RAND_SWAY(ms_sFinalExplosionStagger);

						m_stockpile.m_sNumGrenades--;
						m_stockpile.m_sNumMissiles--;
						}
#ifdef CAN_CHANGE_TO_POWERUP
					}
				else
					{
					// Otherwise, persist as powerup.
               managed_ptr<CPowerUp> ppowerup = realm()->AddThing<CPowerUp>();
               if (ppowerup)
						{
						// Copy whatever's left.
						ppowerup->m_stockpile.Copy(&m_stockpile);

						// Our's should now be empty for safety.  Matter cannot be created or destroyed
						// and all.
						m_stockpile.Zero();

						// Place powerup at our current location.
                  ppowerup->Setup(position.x, position.y, position.z);

						// Blow it up.
						GameMessage	msg;
						msg.msg_Explosion.eType				= typeExplosion;
						msg.msg_Explosion.sPriority		= 0;
						msg.msg_Explosion.sDamage			= 0;
                  msg.msg_Explosion.sX					= position.x;
                  msg.msg_Explosion.sY					= position.y;
                  msg.msg_Explosion.sZ					= position.z;
						msg.msg_Explosion.sVelocity		= 130;
                  msg.msg_Explosion.shooter	= m_shooter;

                  SendThingMessage(msg, msg.msg_Explosion.sPriority, ppowerup);
						}
					else
						{
						TRACE("Update(): Failed to allocate new CPowerUp.\n");
						}
					}
#endif
            Object::enqueue(SelfDestruct);
            return;
		}

		// Update sphere.
      m_smash.m_sphere.sphere.X			= position.x;
      m_smash.m_sphere.sphere.Y			= position.y;
      m_smash.m_sphere.sphere.Z			= position.z;
      m_smash.m_sphere.sphere.lRadius	= m_sRadius;

		// Update the smash.
		realm()->m_smashatorium.Update(&m_smash);

		// Save time for next time
		m_lPrevTime = lThisTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::Render(void)
{
	int32_t lThisTime = realm()->m_time.GetGameTime();

   m_pmesh = &m_anim.m_pmeshes->atTime(lThisTime);
   m_psop = &m_anim.m_psops->atTime(lThisTime);
   m_ptex = &m_anim.m_ptextures->atTime(lThisTime);
   m_psphere = &m_anim.m_pbounds->atTime(lThisTime);

	// Reset rotation so it is not cumulative
   m_trans.makeIdentity();

	// Set its pointing direction
   m_trans.Ry(rspMod360(rotation.y));

	// Eventually this should be channel driven also
   m_sRadius = ms_sCollisionRadius;

   flags.Hidden = m_eState == State_Hide || m_bInsideTerrain;

	// If we're not a child of someone else...
   if (!isChild())
	{
		// Map from 3d to 2d coords
     realm()->Map3Dto2D(position.x, position.y, position.z,
                        m_sX2, m_sY2);
		// Priority is based on Z.
      m_sPriority = position.z;

		// Layer should be based on info we get from the attribute map
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
// Setup
////////////////////////////////////////////////////////////////////////////////

int16_t CDeathWad::Setup(									// Returns 0 if successfull, non-zero otherwise
	int16_t sX,												// In:  New x coord
	int16_t sY,												// In:  New y coord
	int16_t sZ)												// In:  New z coord
{
	int16_t sResult = SUCCESS;
	
	// Use specified position
   position.x = sX;
   position.y = sY;
   position.z = sZ;
	m_dHorizVel = 0.0;

	// Load resources
	sResult = GetResources();

	// Enable the 2D shadow sprite
	PrepareShadow();

	// Set the collision bits
	m_u32CollideIncludeBits = CSmash::Character | CSmash::Misc | CSmash::Barrel;
	m_u32CollideDontcareBits = CSmash::Good | CSmash::Bad;
	m_u32CollideExcludeBits = 0;

	m_smash.m_bits = CSmash::Projectile;
   m_smash.m_pThing = this;

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
int16_t CDeathWad::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
  bool bResult = m_anim.Get("missile");
  bResult &= rspGetResource(&g_resmgrGame, realm()->Make2dResPath(SMALL_SHADOW_FILE), &(m_spriteShadow.m_pImage), RFile::LittleEndian) == SUCCESS;
  return bResult ? SUCCESS : FAILURE;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
int16_t CDeathWad::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	m_anim.Release();

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources 
//				 for this object so there won't be a delay the first time it is
//				 created.
////////////////////////////////////////////////////////////////////////////////

int16_t CDeathWad::Preload(
	CRealm* prealm)				// In:  Calling realm.
{
  CAnim3D anim;
  RImage* pimage;
  int16_t sResult = SUCCESS;
  if (anim.Get("missile"))
    anim.Release();

  if (rspGetResource(&g_resmgrGame, prealm->Make2dResPath(SMALL_SHADOW_FILE), &pimage, RFile::LittleEndian) == SUCCESS)
    rspReleaseResource(&g_resmgrGame, &pimage);
  else
    sResult = FAILURE;

  CacheSample(g_smidDeathWadLaunch);
  CacheSample(g_smidDeathWadThrust);
  CacheSample(g_smidDeathWadExplode);
  return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Traverse the path until the inside terrain status changes or
// the destination is reached.
////////////////////////////////////////////////////////////////////////////////
bool CDeathWad::TraversePath(	// Returns true, when destination reached; false, 
										// if terrain change.
	int16_t		sSrcX,				// In:  Starting position.
	int16_t		sSrcY,				// In:  Starting position.
	int16_t		sSrcZ,				// In:  Starting position.
	bool*		pbInTerrain,		// In:  true, if starting in terrain.
										// Out: true, if ending in terrain.
	int16_t		sDstX,				// In:  Destination position.
	int16_t		sDstZ,				// In:  Destination position.
	double*	pdCurX,				// Out: Position of inside terrain status change.
	double*	pdCurZ)				// Out: Position of inside terrain status change.
	{
	bool	bMadeDestination	= true;	// Assume we make it.

	ASSERT(pbInTerrain);
	ASSERT(pdCurX);
	ASSERT(pdCurZ);
   ASSERT(rotation.y >= 0);
   ASSERT(rotation.y < 360);

	// Determine distance on X/Z plane to destination.
	double	dDistance	= rspSqrt(ABS2(float(sSrcX - sDstX), float(sSrcZ - sDstZ) ) );

	// Set starting position.
	double	dX	= sSrcX;
	double	dZ	= sSrcZ;
	// Determine iteration rate on X and Z.
   double	dRateX	= COSQ[(int16_t)rotation.y] * ms_dTraversalRate;
   double	dRateZ	= -SINQ[(int16_t)rotation.y] * ms_dTraversalRate;
	// Store original status.
	bool	bInitiallyInTerrain	= *pbInTerrain;

	// Loop until change in status or we hit destination.
	while (
		dDistance > 0 &&
		*pbInTerrain == bInitiallyInTerrain)
		{
		if (realm()->GetHeight(dX, dZ) > sSrcY)
			*pbInTerrain	= true;
		else
			*pbInTerrain	= false;

		// See if it's time to create a thrust . . .
		m_dUnthrustedDistance	+= ms_dTraversalRate;
		if (m_dUnthrustedDistance >= ms_dThrustDelta)
			{
			// Thrustage.
			Thrust();
			// Reset for next.
			m_dUnthrustedDistance	= 0.0;
			}

		dX				+= dRateX;
		dZ				+= dRateZ;
		dDistance	-= ms_dTraversalRate;
		}

	// If we did not make the destination . . .
	if (dDistance > 0)
		{
		bMadeDestination	= false;
		}

	// Store new position.
	*pdCurX	= dX;
	*pdCurZ	= dZ;

	return bMadeDestination;
	}

////////////////////////////////////////////////////////////////////////////////
// Generate an explosion at the current position.
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::Explosion(void)
	{
	// Start an explosion object and some smoke (doesn't an explosion object
	// automatically make smoke??).
   managed_ptr<CExplode> pExplosion = realm()->AddThing<CExplode>();
   if (pExplosion)
		{
		// Don't blow us up.
      pExplosion->m_except = m_shooter;

      pExplosion->Setup(position.x, MAX(position.y-30, 0.0), position.z, m_shooter);
		PlaySample(										// Returns nothing.
															// Does not fail.
			g_smidDeathWadExplode,					// In:  Identifier of sample you want played.
			SampleMaster::Destruction,				// In:  Sound Volume Category for user adjustment
         DistanceToVolume(position.x, position.y, position.z, ExplosionSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
		}
	
   int16_t a;
   for (a = 0; a < 8; a++)
   {
     managed_ptr<CFire> pSmoke = realm()->AddThing<CFire>();
      if (pSmoke)
			{
         pSmoke->Setup(position.x - 4 + GetRandom() % 9, position.y-20, position.z - 4 + GetRandom() % 9, ms_lSmokeTimeToLive, true, CFire::Smoke);
         pSmoke->m_shooter = m_shooter;
			}
		}


	if (m_stockpile.m_sNumFuel > 0 || m_stockpile.m_sNumNapalms > 1)
		{
		// Also, create a fire.
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Generate some thrust at the current position.
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::Thrust(void)
	{
	m_stockpile.m_sNumFuel--;


	if (m_bInsideTerrain == false)
		{
      managed_ptr<CFire> pSmoke = realm()->AddThing<CFire>();
      if (pSmoke)
			{
			// This needs to be fixed by calculating the position of the back end of
			// the deathwad in 3D based on the rotation.  
         pSmoke->Setup(position.x, position.y, position.z, ms_lSmokeTimeToLive, true, CFire::SmallSmoke);
         pSmoke->m_shooter = m_shooter;
			}

		// Also, create a fire (moving at the wad's velocity?).
      managed_ptr<CFireball> pfireball = realm()->AddThing<CFireball>();
      if (pfireball)
			{
         pfireball->Setup(position.x, position.y, position.z, rotation.y, ms_lFireBallTimeToLive, m_shooter);
			pfireball->m_dHorizVel	= m_dHorizVel / 4.0;
			pfireball->m_eState		= State_Fire;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Generate the launch kick/debris.
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::Launch(void)
	{
	// The launch sound.
	PlaySample(										// Returns nothing.
														// Does not fail.
		g_smidDeathWadLaunch,					// In:  Identifier of sample you want played.
		SampleMaster::Weapon,					// In:  Sound Volume Category for user adjustment
      DistanceToVolume(position.x, position.y, position.z, LaunchSndHalfLife) );	// In:  Initial Sound Volume (0 - 255)
	
	// The looping thrust sound.
	PlaySample(										// Returns nothing.
														// Does not fail.
		g_smidDeathWadThrust,					// In:  Identifier of sample you want played.
		SampleMaster::Weapon,					// In:  Sound Volume Category for user adjustment
      DistanceToVolume(position.x, position.y, position.z, LaunchSndHalfLife),	// In:  Initial Sound Volume (0 - 255)
		&m_siThrust,								// Out: Handle for adjusting sound volume
		nullptr,											// Out: Sample duration in ms, if not nullptr.
		100,											// In:  Where to loop back to in milliseconds.
														//	-1 indicates no looping (unless m_sLoop is
														// explicitly set).
		500,											// In:  Where to loop back from in milliseconds.
														// In:  If less than 1, the end + lLoopEndTime is used.
		false);										// In:  Call ReleaseAndPurge rather than Release after playing

	Explosion();

	// Get the launcher . . .
   if(m_shooter)
		{
		// If it's a dude . . .
      if (m_shooter->type() == CDudeID)
         {
			// Add force vector for kick.  See ya.
         managed_ptr<CDude>(m_shooter)->AddForceVector(ms_dKickVelocity, rotation.y - 180);
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Feed the WAD prior to moving its state to State_Fire.
////////////////////////////////////////////////////////////////////////////////
void CDeathWad::FeedWad(
	CStockPile*	pstockpile)	// In:  Src for WAD's arsenal.
	{
	// Take needed ammo.
	m_stockpile.m_sNumMissiles	= pstockpile->m_sNumMissiles;
	m_stockpile.m_sNumNapalms	= pstockpile->m_sNumNapalms;
	m_stockpile.m_sNumFuel		= pstockpile->m_sNumFuel;
	m_stockpile.m_sNumGrenades	= pstockpile->m_sNumGrenades;
	// Truncate to max we can hold.
	m_stockpile.Intersect(&ms_stockpileMax);
	// Subtract from provider.
	pstockpile->Sub(&m_stockpile);
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
