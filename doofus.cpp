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
// doofus.cpp
// Project: Postal
//
// This module implements the CDoofus class which is the class of enemy
//	guys for the game.  
//
// History:
//		01/13/97 BRH	Started this file from CDude and modified it
//							to do some enemy logic using the same assets
//							as the sample 2D guy.  
//
//		01/15/97 BRH	Changed the render to draw the guy with his 
//							hotspot between his feet.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/05/97 BRH	Fixed problem loading Instance ID.  Also fixed some other
//							problems with the run routine.  He now will follow the 
//							bouys to get to the bouy nearest the CDude.
//
//		02/23/97 BRH	Changed coordinate system to x, -z in Find Direction.
//
//		02/25/97 BRH	Fixed problem with the Startup where the height was not
//							being multiplied by 4, so enemies would start too low
//							if placed on a roof.
//
//		03/04/97 BRH	Derived this from CCharacter instead of CThing
//
//		03/05/97 BRH	Added SelectRandomBouy function which picks a random
//							bouy number (a valid one) or returns 0 if there are
//							no bouys.
//
//		03/06/97 BRH	Added realignment timer and function so that the
//							direction to the bouy can be recalculated every
//							so often to avoid missing it.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/14/97	JMI	SelectDude() now chooses the closest dude on the X/Z plane.
//
//		04/04/97	JMI	SelectDude() no longer chooses dead dudes.
//							Also, last update to make SelectDude() find the closest 
//							CDude was not comparing the distance to the CDude from this
//							guy, but instead was comparing the distance to the CDude from
//							(0, 0, 0).
//
//		04/16/97 BRH	Changed references to the realm's list of CThings to use
//							the new non-STL methods.
//
//		04/22/97 BRH	Moved common code and some variables like the animations
//							to the base class.  Put common logic routines like the
//							reactions to weapons in this base class.
//
//		04/24/97 BRH	Added TryClearDirection function that uses the 
//							IsPathClear() funciton to try 3 directions to see if they
//							are clear.
//
//		05/02/97	JMI	Added check to make sure not already in shot state or
//							writhing state before changing to shot state in OnShotMsg.
//							Also, now OnBurnMsg() sets him into the m_animOnFire in-
//							stead of m_animRun.
//
//		05/04/97 BRH	Removed #ifdef code sections referring to STL lists.
//
//		05/06/97 BRH	Added Popout logic, detection smash.
//
//		05/09/97 BRH	Added Writhing and Shot logic from CPerson.
//
//		05/11/97 BRH	Fixed problem with Logic_Guard.
//
//		05/15/97 BRH	In the popout and run & shoot wait states, changed from
//							the temporary timeout method to checking for the
//							triggered pylon before running the logic.
//
//		05/18/97 BRH	Added some logic functions for the victims to use.
//
//		05/20/97 BRH	Added the hiding states.  Also added calls to 
//							ReevaluateState() at times in the action cycles
//							where the action can be changed.  Changed Logic_PylonDetect
//							to set flags for use in ReevaluateState() and
//							in the logic table variables evaluation.
//
//		05/21/97 BRH	Added m_dAnimRot to specify the direction the guy is 
//							facing and separating that from the direction he is
//							moving.  This will be used for the run and shoot
//							so that he can run and face sideways.  
//
//		05/22/97 BRH	Fixed typo bug in Logic_Shoot.
//
//		05/25/97 BRH	Added the m_ShootAngle variable and an override for
//							ShootWeapon that uses this angle to aim the weapon.
//
//		05/26/97 BRH	Changed ShootWeapon so that CSmash bits are passed in
//							so that enemy bullets don't hit other enemies.
//
//		05/26/97 BRH	Added a timer for shooting to limit the number of shots
//							for each type of gun.  Also added some avoidance 
//							of obstacles during MoveNext in case the guy gets stuck
//							trying to get to the next bouy.
//
//		05/27/97 BRH	Fixed a few problems with logic table transitions.
//
//		05/27/97 BRH	Added some avoidance of obstacles to Logic_PopBegin in
//							case he gets stuck on walls while trying to find the 
//							first pylon.
//
//		05/31/97	JMI	Replaced m_pDude with m_idDude.  The problem was that, by
//							just using a pointer to the dude, we never found out when
//							the dude was gone (deleted).  Although this is rare for
//							CDudes, it does happen.  For example, in the beginning of
//							a level all CDudes that do not have an associated player
//							are sent a Delete msg.  They do not process this message
//							until their respective Update() calls.  If a CDoofus 
//							derived guy happened to be placed in the level before a 
//							CDude (that is, the CDoofus' Update() got called before the
//							CDude's), and the CDoofus happened to point its m_pDude at 
//							this CDude (that was destined to soon be deleted), later 
//							when referencing the pointer to the freed and/or reallocated
//							memory, the CDoofus could cause a protection	fault or	math 
//							overflow (due to invalid values returned by 
//							m_pDude->GetX, Y, Z() with the non-CDude 'this' pointer).
//							Also, in the case that SelectDude() did not select a dude,
//							SQDistanceToDude() was returning an unitialized double
//							which, in some cases, could be a totally illegal double
//							value.
//
//		06/02/97 BRH	Added AdvanceHold action and state so that once he reaches
//							the end of the advancement, he goes into this hold state
//							rather than Engage automatically.  This way the logic
//							table can have more control over the next state.
//
//		06/02/97 BRH	Changed TryClearShot to use the CRealm version of
//							IsPathClear which just checks for terrain obstacles.
//
//		06/05/97	JMI	Changed m_sHitPoints to m_stockpile.m_sHitPoints to 
//							accommodate new m_stockpile in base class, CThing3d (which
//							used to contain the m_sHitPoints).
//
//		06/10/97 BRH	Now sends messages to CDemon for Explosion, and burning.
//
//		06/10/97 BRH	Added RunIdleAnimation() function which monitors the
//							idle timer and controls which of 3 idle animations to
//							use.  This should be called when in some kind of wait
//							state.
//
//		06/10/97 BRH	Fixed "YMCA" bug where the enemy guy would cycle 
//							each frame between Advance and AdvanceHold.
//
//		06/11/97 BRH	Fixed the crouch and search animations since the
//							search is done from the crouch posiiton.  It was
//							previously backwards because I thought the search
//							animation was done from a standing position.
//
//		06/13/97	JMI	Changed FindDirection() to return rotation.y if we cannot find
//							a CDude.
//							Also, changed Logic_Shoot() over to using events.
//
//		06/17/97 BRH	Changed NEAR_DEATH_HITPOINTS to a higher value since the
//							machine gun bullets were increased, it was diffucult to
//							get a writhing person.
//
//		06/17/97 BRH	Attempted to make enemies stop shooting dead CDudes.
//
//		06/17/97	JMI	Now doubles smash radius when in writhing state.
//
//		06/18/97	JMI	Changed PlaySoundWrithing() to return the duration of the
//							played sample.
//
//		06/18/97 BRH	Changed over to using GetRandom()
//
//		06/24/97	JMI	Added intialization of m_sRotateDir.
//
//		06/26/97 BRH	Added a special case for Writhers who get burned. 
//							Previously they didn't react because their smash bits
//							were changed when they went into writhing.  Then we wanted
//							them to get killed by fire so included the AlmostDead bits
//							but they they would jump to their feet and run around
//							once they got burned.  So now they will just lie there
//							and die.
//
//		06/26/97 BRH	When a doofus is killed after preparing a weapon but 
//							before shooting it, he will drop it if it was a throwing
//							weapon, but will just delete it if it was a launched
//							weapon.
//
//		06/27/97 BRH	Added a flag for recently stuck so that when a character
//							gets stuck on a wall, he sets the flag, then when he gets
//							free of the obstacle, it will get the closest bouy rather
//							than trying to align to the one it was trying to get to
//							previously which many times caused him to get stuck in 
//							the same manner.
//
//		06/28/97 BRH	Changed the RegisterBirth and RegisterDeath calls to pass 
//							m_bCivilian so that the scoring can updated for hostiles
//							or civilians.
//
//		07/09/97 BRH	Added logic for walking and running around on a bouy
//							network - used for victims.
//
//		07/11/97 BRH	Added call to inline Cheater() to disable game if necessary
//
//		07/12/97 BRH	Added addional text strings for the new actions that were
//							added.  Also changed PylonDetect function to use 
//							QuickCheckClosest rather than QuickCheck so that enemies
//							will detect the pylon you put them closest to and not
//							just a marker pylon.
//
//		07/12/97 BRH	Changed macro MAX_STEPUP_THRESHOLD to use the CThing3d
//							definitino MaxStepUpThreshold so that the detecting and
//							ability to move will be the same.
//
//		07/15/97 BRH	Added a few more calls to Cheater.
//
//		07/20/97 BRH	Effectively canceled the DelayShoot state by setting
//							the timeout to zero since it caused the guys to
//							not shoot very often on levels that were difficult
//							to move in.
//
//		07/21/97	JMI	Now Update() calls base class version.
//
//		07/23/97 BRH	Added tunable values for different timeouts which will get
//							set in doofus to the default static values that they were
//							before, and can be set in Person to the personatorium values
//							so that different people have different traits.
//
//		08/01/97 BRH	Changed the aiming so that it is based on the game
//							difficulty setting.
//
//		08/02/97 BRH	Added YellForHelp funciton that enemies can call when they
//							get shot to alert others in the are that they need help.
//
//		08/05/97 BRH	Fixed the problem with the doofus leaving the Run&Shoot
//							state at higher difficulty settings due to re-aligning
//							the angles with the shoot angle.	  
//
//		08/06/97 BRH	Changed TryClearShot to first do the point translation
//							to the rigid body where the weapon is shot from, in order
//							to get the correct height.  This will give the guys more
//							opportunities to shoot, and will work when the guys
//							are scaled larger or smaller.
//
//		08/06/97	JMI	Added m_ptransExecutionTarget link point for execution
//							sphere.  Also, added PositionSmash() to provide overridable
//							method for updating the collision sphere.
//
//		08/06/97	JMI	Now TryClearPath() returns false if there's no weapon
//							link point.
//
//		08/07/97	JMI	Added ms_awdWeapons[], ms_apszWeaponResNames[], and 
//							GetResources() and FreeResources() for loading these anims.
//							Also, added ms_lWeaponResRefCount so we could know when the
//							weapons were no longer needed.
//
//		08/08/97	JMI	Now Logic_Shoot() handles flamer.
//
//		08/08/97	JMI	Now Logic_Shoot() handles AutoRifle, Uzi, and SmallPistol.
//
//		08/08/97 BRH	Added start and end bouy ID's for special cases like
//							marching.  Added these to load and save.  They are set
//							by the dialog in the CPerson.  Also added marching logic.
//
//		08/09/97 BRH	Changed panic to be vicinity based so only the nearby
//							people panic.  Also added checks for prepared weapons
//							when guys get blown up or burned, where they should
//							either randomly discard their weapon, or delete it.
//
//		08/10/97	JMI	Moved CDoofus() and ~CDoofus() into doofus.cpp from 
//							doofus.h.
//							Was going to move the registering of the birth into the
//							constructor but I realized that m_bCivilian probably won't
//							get set until Load() so I left it in Startup() (it could
//							probably also get set via EditModify() but this doesn't 
//							matter b/c that's in edit mode only).
//							Now Registers death with the realm in the destructor if 
//							m_bRegisteredBirth is true.
//							Also, now OnShotMsg() calls base class version even if
//							other states don't permit a state change b/c this gives
//							better feedback to the user (base class OnShotMsg() creates
//							blood).
//
//		08/10/97	JMI	Moved NoWeapon up to enum value 0 and created a new one
//							to take its -1 place as an invalid weapon (InvalidWeapon).
//							Also, added block in PrepareWeapon() for the NoWeapon case.
//							Also, moved prepare weapon from the .H to the .CPP.
//
//		08/11/97 BRH	Found the problem with the parade level victims where they
//							were flipping around one bouy.  The problem was that they
//							asked how to get to a particular bouy which was unreachable
//							because the network was not fully connected, so when it
//							got the unreachable flag, it kept retrying the same bouy.  
//							I fixed the problem so they will pick a new bouy, but also
//							we are deleting the unconnected nodes in the parade level.
//								
//		08/11/97	JMI	Now sets backup weapon to default 'none' and uses the 
//							backup weapon when there's no animation for the current
//							weapon type.
//							Also, changed incorrectly name ms_awtType2Id to 
//							ms_awtId2Type mapping.
//
//		08/11/97 BRH	Fixed problem with run & shoot not using the correct
//							angle.
//
//		08/12/97	JMI	Now hides weapon if current event is 10 or more.
//
//		08/12/97 BRH	Checks for events channel in render so that people
//							without animation events like the band guys will still
//							work.  Also made shooting timing based on game
//							difficulty.
//
//		08/13/97	JMI	Changed so OnShotMsg() calls base class to generate blood
//							no matter what.
//
//		08/14/97	JMI	Switched references to g_GameSettings.m_sDifficulty to
//							realm()->m_flags.sDifficulty.
//
//		08/14/97 BRH	Added static default bits to pass to weapons for their
//							collision testing.  Changed call to WhileHoldingWeapon
//							to include these defaults.
//
//		08/15/97 BRH	Changed the check for available pylons to check for a
//							clear path to the pylon before saying it is available.
//							Some guys were getting stuck on fences between themselves
//							an an available pylon.  Also fixed the writhing to
//							executed transition so that the guys don't flip around
//							180 degrees.  Also fixed the smash bits that get passed
//							to the missile weapons.
//
//		08/16/97 BRH	Changed the person's PlaySound functions so that comments
//							are only made when the CDude is alive.  So in this module, I
//							made sure that the victims were also calling SelectDude()
//							for the logic that they were doing, just to keep track of
//							whether the CDude was alive or dead.  Also tuned the
//							shooting accuracy for the levels to make them more accurate
//							on the easier levels since the easy dudes were missing too
//							much.
//
//		08/18/97 BRH	Added virtual override for CCharacter's WhileHoldingWeapon
//							so that for higher difficulty settings where the enemies
//							re-aim after preparing the weapon, they could do it
//							every frame between PrepareWeapon and ShootWeapon so that
//							they turned smoothy and didn't just flip around when they
//							got to ShootWeapon.  This was a problem with the Rocketman
//							who had a long shoot animation.  If the target moved a
//							quite a bit between PrepareWeapon and ShootWeapon, he would
//							spin around just before releasing the shot.  Also fixed
//							a problem with the ShootWeapon adjustment of the
//							random shooting innacuracy when I changed the values 
//							last time, I adjusted the max left swing, but forgot to
//							adjust the random amount.
//
//		08/18/97	JMI	Now sends doofuses who are writhing to the back most sprite
//							layer.  As people pointed out this is another of two evils.
//							It is, however, a lessor.  It appears a little wierd in one
//							case but better in all others I've seen so far.
//
//		08/19/97	JMI	Now sends doofuses who are dying to the back as well.
//
//		08/19/97	JMI	No longer shoots CSmash::Misc items.
//
//		08/20/97	JMI	Now Logic_Shot() does not wait to the end of the animation
//							to check for death.  This way they don't seem to kick back
//							from the shot, shoot forward, and then die.
//
//		08/20/97 BRH	Trying a smaller tolerance on the Bouys so that the
//							enemies must get closer to the hotspot before
//							moving to the next one.  We wanted to see if this helps
//							certain situations where guys are getting stuck.  Changed
//							from 10 pixel radius to 5.
//
//		08/21/97	JMI	Now does deluxe reporting on whether this doofus finds his
//							NavNet. 
//
//		08/21/97	JMI	Now after reporting that the doofus did not find its NavNet
//							it sets him to the current NavNet.
//
//		08/21/97 BRH	Added a blooc counter so that the number of blood pools
//							created while writhing could be cut down a little bit.
//
//		08/24/97 BRH	Changed the TryClearShot to use the Character version of
//							IsPathClear which also checks for people in the way.  Added
//							an additional call to TryClearShot in ShootWeapon which is
//							after the weapon has been re-aimed to make sure that the
//							path is still clear.  It was a problem with the rocketman
//							who has the slowest animation and may turn completely
//							around before shooting.
//
//		08/24/97 BRH	Aborts sample that was playing when the guy gets executed
//							so that he doesn't keep making noises after he is dead.
//
//		08/25/97 BRH	Undid a bunch of changes made yesterday which changed the
//							logic too much which threw off the tuning.  Added two new
//							escape routes for the Engage mode instead.  Fixed the 
//							shoot timer which was being set to two different values
//							in two different places.  Made the shooting more accurate
//							and more of the difficulty tuning will be with the 
//							shooting times.
//
//		08/27/97 BRH	Victims now panic when they are shot.
//
//		09/03/97	JMI	Sentries now exclude CSmash::Bads and CSmash::Civilians.
//
//		09/07/97 BRH	As Steve requested, the medium difficulty will now
//							re-adjust aiming just like hard difficulty in ShootWeapon.
//
//		12/18/97	JMI	Changed SelectRandomBouy() to return 0 if a the number of
//							bouys is less than or equal to one.  It used to be less 
//							than one but, for some reason, GetNumNodes() returns 1 when
//							there's no bouys.  This caused it to lock up when no bouys
//							for some lgk files.
//
////////////////////////////////////////////////////////////////////////////////

#include "doofus.h"

#include "realm.h"
#include "navnet.h"
#include "dude.h"
#include "SampleMaster.h"
#include "grenade.h"
#include "pylon.h"
#include "bouy.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define	NEAR_DEATH_HITPOINTS 31							// Below this, start writhing
#define  MS_BETWEEN_SAMPLES	100						// Time between pain groans.
#define  BURNT_BRIGHTNESS		-40						// -128 to 127.

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!
double CDoofus::ms_dAccUser     = 150.0;				// Acceleration due to user
double CDoofus::ms_dAccDrag     = 80.0;				// Acceleration due to drag

double CDoofus::ms_dMaxVelFore  = 80.0;				// Maximum forward velocity
double CDoofus::ms_dMaxVelBack  = -60.0;				// Maximum backward velocity

double CDoofus::ms_dDegPerSec   = 150.0;				// Degrees of rotation per second
double CDoofus::ms_dOffScreenDistance = 500*500;	// Square distance off screen
double CDoofus::ms_dGuardDistance = 300*300;			// Square distance before attacking
double CDoofus::ms_dThrowHorizVel = 200;				// Throw out at this velocity
double CDoofus::ms_dMinFightDistance = 80.0;			// Closest you want to get
double CDoofus::ms_dMedFightDistance = 200.0;		// Median distance for fighting
double CDoofus::ms_dMaxFightDistance = 400.0;		// Farthest distance for fighting
double CDoofus::ms_dMinFightDistanceSQ = 80.0*80.0;// Square distance for 
double CDoofus::ms_dMedFightDistanceSQ	= 200.0*200.0;
double CDoofus::ms_dMaxFightDistanceSQ = 400.0*400.0;	
double CDoofus::ms_dMarchVelocity = 20.0;				// Speed at which to march
int32_t CDoofus::ms_lDefaultAlignTime = 100; //2000;	// Time to realign to bouy position
int32_t CDoofus::ms_lGuardTimeoutMin = 4000;				// Time to check for CDudes again
int32_t CDoofus::ms_lGuardTimeoutInc = 500;				// Interval to add for each easier difficulty level
int32_t CDoofus::ms_lShootTimeoutMin = 1000;				// Min time between shots, adjused for difficulty
int32_t CDoofus::ms_lShootTimeoutInc = 200;				// Adjustment time for difficulty level between shots
int32_t CDoofus::ms_lDetectionRadius = 100;//80			// Radius of detection smash sphere
int32_t CDoofus::ms_lRunShootInterval = 2000;			// Time between shots while running
int32_t CDoofus::ms_lReseekTime = 15 * 1000;				// Seek the dude again after this time
int32_t CDoofus::ms_lShotTimeout = 3000;					// Time between full shot reaction animations
																	// which gives him a chance to attack or run
int32_t CDoofus::ms_lStuckRecoveryTime = 5000;			// Time to stay in recoverys state
int32_t CDoofus::ms_lAvoidRadius = 40;						// Radius of fire detection smash
int32_t CDoofus::ms_lYellRadius = 150;						// Yell for help in this vicinity
int32_t CDoofus::ms_lHelpTimeout = 3000;					// Time to react to a call for help.
int32_t CDoofus::ms_lDelayShootTimeout = 0; //2000;			// Time before shooting
int32_t CDoofus::ms_lHelpingTimeout = 1000;				// Only shoot every this often

// Note that these seem to apply to all weapons except bullet weapons.  That is, these are
// passed to WhileHoldingWeapon() which passes them on to ShootWeapon(), but WhileHoldingWeapon()
// is only used for non-bullet weapons.  In the bullet weapons case, it uses the default parameters
// to ShootWeapon().
uint32_t CDoofus::ms_u32CollideBitsInclude = CSmash::Character | CSmash::Barrel;
uint32_t CDoofus::ms_u32CollideBitsDontcare = CSmash::Good | CSmash::Bad;
uint32_t CDoofus::ms_u32CollideBitsExclude = CSmash::SpecialBarrel | CSmash::Ducking | CSmash::Bad | CSmash::Civilian;

int16_t CDoofus::ms_sStuckLimit = 3;						// Number of times to retry before attempting to
																	// get free of whatever you are stuck on.

// Weapon animations.
CAnim3D	CDoofus::ms_aanimWeapons[NumWeaponTypes];
// Current ref count on ms_aanimWeapons[].
int32_t		CDoofus::ms_lWeaponResRefCount	= 0;

// Weapon details (descriptions, res names, etc.).
CDoofus::WeaponDetails	CDoofus::ms_awdWeapons[NumWeaponTypes]	=
{
	//////// NoWeapon
	{	// pszName, pszResName, CThing ID
		"Verbal Abuse",
		nullptr,
		TotalIDs,
	},

	//////// Rocket
	{	// pszName, pszResName, CThing ID
		"Rocket",
      "launcher",
		CRocketID,
	},

	//////// Grenade
	{	// pszName, pszResName, CThing ID
		"Grenade",
		nullptr,
		CGrenadeID,
	},

	//////// Napalm
	{	// pszName, pszResName, CThing ID
		"Napalm",
      "napalmer",
		CNapalmID,
	},

	//////// Firebomb
	{	// pszName, pszResName, CThing ID
		"Cocktail",
		nullptr,
		CFirebombID,
	},

	//////// ProximityMine
	{	// pszName, pszResName, CThing ID
		"ProximityMine",
		nullptr,
		CProximityMineID,
	},

	//////// TimedMine
	{	// pszName, pszResName, CThing ID
		"TimedMine",
		nullptr,
		CTimedMineID,
	},

	//////// RemoteMine
	{	// pszName, pszResName, CThing ID
		"RemoteMine",
		nullptr,
		CRemoteControlMineID,
	},

	//////// BouncingBetty
	{	// pszName, pszResName, CThing ID
		"BouncingBetty",
		nullptr,
		CBouncingBettyMineID,
	},

	//////// Flamer
	{	// pszName, pszResName, CThing ID
		"Flamer",
      "flmthrower",
		CFirestreamID,
	},

	//////// Pistol
	{	// pszName, pszResName, CThing ID
		"Pistol",		
      "bigpistol",
		CPistolID,
	},

	//////// MachineGun
	{	// pszName, pszResName, CThing ID
		"MachineGun",
      "submachine",
		CMachineGunID,
	},

	//////// ShotGun
	{	// pszName, pszResName, CThing ID
		"ShotGun",
      "shotgun",
		CShotGunID,
	},

	//////// Heatseeker
	{	// pszName, pszResName, CThing ID
		"Heatseeker",
      "launcher",
		CHeatseekerID,
	},

	//////// Assault
	{	// pszName, pszResName, CThing ID
		"Assault",
      "spraygun",
		CAssaultWeaponID,
	},

	//////// DeathWad
	{	// pszName, pszResName, CThing ID
		"DeathWad",
      "napalmer",
		CDeathWadID,
	},

	//////// DoubleBarrel
	{	// pszName, pszResName, CThing ID
		"DoubleBarrel",
      "shotgun",
		CDoubleBarrelID,
	},

	//////// Uzi
	{	// pszName, pszResName, CThing ID
		"Uzi",
      "uzi",
		CUziID,
	},

	//////// AutoRifle
	{	// pszName, pszResName, CThing ID
		"AutoRifle",
      "autorifle",
		CAutoRifleID,
	},

	//////// SmallPistol
	{	// pszName, pszResName, CThing ID
		"SmallPistol",
      "pistol",
		CSmallPistolID,
	},

	//////// Dynamite
	{	// pszName, pszResName, CThing ID
		"Dynamite",
		nullptr,
		CDynamiteID,
	},


};

// Maps a CThing ID to a WeaponType enum.
CDoofus::WeaponType	CDoofus::ms_awtId2Type[TotalIDs]	=	
{
	NoWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	Rocket,
	Grenade,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	Napalm,
	InvalidWeapon,
	InvalidWeapon,
	Firebomb,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	ProximityMine,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	Pistol,
	MachineGun,
	ShotGun,
	InvalidWeapon,
	TimedMine,
	BouncingBettyMine,
	RemoteControlMine,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	Heatseeker,
	InvalidWeapon,
	Assault,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	InvalidWeapon,
	Flamer,
	DeathWad,
	DoubleBarrel,
	Uzi,
	AutoRifle,
	SmallPistol,
	Dynamite,
};

const char* CDoofus::ms_apszActionNames[] =
{
	"Guard",
	"Advance",
	"Retreat",
	"Engage",
	"Popout",
	"Run&Shoot",
	"Hide",
	"Advance-Hold",
	"Walk",
	"Panic",
	"March",
	"Madness",
	"Help",
};

// Let this auto-init to 0
int16_t CDoofus::ms_sFileCount;


//
//

////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////
CDoofus::CDoofus(void)
{
  m_sSuspend = 0;
  m_sNextX = m_sNextZ = 0.0;
  m_ucDestBouyID = m_ucNextBouyID = 0;
  m_lAlignTimer = 0;
  m_lEvalTimer = 0;
  m_lShootTimer = 0;
  m_lShotTimeout = 0;
  m_lCommentTimer = 0;
  m_usCommentCounter = 0;
  m_lLastHelpCallTime = 0;
  m_lSampleTimeIsPlaying = 0;
  m_bRecentlyStuck = false;
  m_bCivilian = false;
  //m_ptransExecutionTarget	= nullptr;
  //	m_spriteWeapon.m_pthing	= this;
  m_ucSpecialBouy0ID = 0;
  m_ucSpecialBouy1ID = 0;
  m_bPanic = false;
  m_bRegisteredBirth = false;
  // Default to no fallback weapon.
  m_eFallbackWeaponType	= TotalIDs;
  m_sStuckCounter = 0;
  m_usBloodCounter = 0;
  m_siPlaying = 0;

  // explicitly initialize to stop Valgrind whining. --ryan.
  m_lIdleTimer = realm()->m_time.GetGameTime() + 2000 + GetRandom() % 2000;
  m_lStuckTimeout = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////
CDoofus::~CDoofus(void)
	{
	// If we've been born . . .
	if (m_bRegisteredBirth == true)
		{
		// See who killed us, if anyone
      bool bPlayerKill = false;
      if (m_killer && m_killer->type() == CDudeID)
				bPlayerKill = true; // Dude killed us.

		// Good bye.
      realm()->RegisterDeath(m_bCivilian, bPlayerKill);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CDoofus::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	int16_t sFileCount,										// In:  File count (unique per file, never 0)
	uint32_t	ulFileVersion)									// In:  Version of file format to load.
	{
	int16_t sResult = CCharacter::Load(pFile, bEditMode, sFileCount, ulFileVersion);

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
					pFile->Read(&ms_dAccUser);
					pFile->Read(&ms_dAccDrag);
					pFile->Read(&ms_dMaxVelFore);
					pFile->Read(&ms_dMaxVelBack);
					pFile->Read(&ms_dDegPerSec);
					// Clear the panic flag at the load of a level.
					m_bPanic = false;
					break;
			}
		}

		// Load object data

		switch (ulFileVersion)
		{
			default:
			case 43:
				pFile->Read(&m_ucSpecialBouy0ID);
				pFile->Read(&m_ucSpecialBouy1ID);

			case 42:
			case 41:
			case 40:
			case 39:
			case 38:
			case 37:
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
			case 3:
			case 2:
			case 1:
				// Get the instance ID for the NavNet
				uint16_t u16Data;
            pFile->Read(&u16Data);
            m_pNavNet = realm()->GetOrAddThingById<CNavigationNet>(u16Data);
				break;
		}
		
		// Make sure there were no file errors
		if (!pFile->Error())
		{
			// Get resources
		}
		else
		{
			sResult = FAILURE;
			TRACE("CDoofus::Load(): Error reading from file!\n");
		}
	}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CDoofus::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	int16_t sFileCount)										// In:  File count (unique per file, never 0)
	{
	// Call the base class save to save the u16InstanceID
	int16_t sResult = CCharacter::Save(pFile, sFileCount);
	if (sResult == SUCCESS)
	{
		// Save common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			// Save static data
			pFile->Write(&ms_dAccUser);
			pFile->Write(&ms_dAccDrag);
			pFile->Write(&ms_dMaxVelFore);
			pFile->Write(&ms_dMaxVelBack);
			pFile->Write(&ms_dDegPerSec);
			}

		// Save object data

		// Save the instance ID for the parent NavNet so it can be connected
		// again after load
		pFile->Write(&m_ucSpecialBouy0ID);
		pFile->Write(&m_ucSpecialBouy1ID);
      uint16_t u16Data = invalid_id;	// Safety.
		if (m_pNavNet)
			u16Data	= m_pNavNet->GetInstanceID();
		pFile->Write(&u16Data);

		sResult = pFile->Error();
	}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
void CDoofus::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	CCharacter::Startup();

	// If we don't have a pointer to the Nav Net yet, get it from the ID
   if (!m_pNavNet)
      {
     // Use the current NavNet.
     m_pNavNet	= realm()->NavNet();
			// Message depends on user mode.
         if (realm()->m_flags.bEditing)
				{
				rspMsgBox(
					RSP_MB_ICN_STOP | RSP_MB_BUT_OK,
					g_pszAppName,
					g_pszDoofusCannotFindNavNet_EditMode_hu_hu,
               GetInstanceID(),
               m_pNavNet->GetInstanceID());
				}
			else
				{
				rspMsgBox(
					RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
					g_pszAppName,
               g_pszDoofusCannotFindNavNet_PlayMode_hu_hu,
                  GetInstanceID(),
                  m_pNavNet->GetInstanceID());
				}

		}

	// Clear stuck counter
	m_sStuckCounter = 0;

	// Init other stuff
	m_dVel = 0.0;
   rotation.y = 0.0;
	m_dAnimRot = 0.0;
   m_sPrevHeight = (int16_t) position.y;
	m_sRotateDir = 0;

	// Set up the detection smash
   m_smashDetect.m_pThing = this;
	m_smashDetect.m_bits = 0;
	m_smashDetect.m_sphere.sphere.lRadius = ms_lDetectionRadius;

	// Set up the fire avoidance smash
   m_smashAvoid.m_sphere.sphere.X = position.x + (rspCos(rotation.y) * ms_lAvoidRadius);
   m_smashAvoid.m_sphere.sphere.Y = position.y;
   m_smashAvoid.m_sphere.sphere.Z = position.z - (rspSin(rotation.y) * ms_lAvoidRadius);
	m_smashAvoid.m_sphere.sphere.lRadius = ms_lAvoidRadius;
	m_smashAvoid.m_bits = 0;
   m_smashAvoid.m_pThing = this;

	// Setup weapon sprite.
   m_spriteWeapon.flags.Hidden = true;
   AddChild(&m_spriteWeapon);


	// Start in a neutral state
	m_state = State_Idle;
	// Hello, world.
   realm()->RegisterBirth(m_bCivilian);
	// Note that we've registered our birth so we know later that we need to
	// register our death.
	m_bRegisteredBirth	= true;

	// Set tunable values to their doofus defaults
   m_lShootTimeout = ms_lShootTimeoutMin + ((11-realm()->m_flags.sDifficulty) * ms_lShootTimeoutInc);
	m_lRunShootInterval = ms_lRunShootInterval;
	m_lShotReactionTimeout = ms_lShotTimeout;
}

#if !defined(EDITOR_REMOVED)
////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
int16_t CDoofus::EditNew(									// Returns 0 if successfull, non-zero otherwise
	int16_t sX,												// In:  New x coord
	int16_t sY,												// In:  New y coord
	int16_t sZ)												// In:  New z coord
{
   int16_t sResult = SUCCESS;

	CCharacter::EditNew(sX, sY, sZ);	

	// Since we were just created in the editor, set our nav net to the
	// current one for this realm.
   m_pNavNet = realm()->NavNet();

	return sResult;
}
#endif // !defined(EDITOR_REMOVED)

////////////////////////////////////////////////////////////////////////////////
// Render - This override version temporarily replaces the rotation.y with the
//				m_dAnimRot so that it draws the person in the desired facing 
//				direction.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Render(void)
{
   double dSaveRotation = rotation.y;
   rotation.y = m_dAnimRot;

	CCharacter::Render();
	
   rotation.y = dSaveRotation;

	CAnim3D*		panimWeapon	= nullptr;

	if (m_panimCur->m_pevent)
	{	
		// Get current event.
      uint8_t u8Event = m_panimCur->m_pevent->atTime(m_lAnimTime);

		// If gun not hidden by Randy . . .
		if (u8Event < 10)
		{
			panimWeapon	= GetWeaponAnim(m_eWeaponType);
			// If we got an anim . . .
			if (panimWeapon)
			{
				// If this anim is empty (and not intentional) . . .
				if (panimWeapon->m_pmeshes == nullptr && m_eWeaponType != TotalIDs)
				{
					// Get fallback weapon, if any.
					panimWeapon	= GetWeaponAnim(m_eFallbackWeaponType);
				}
			}
		}
	}

	// If we have a visible weapon . . .
	if (panimWeapon)
	{
		// If we have the necessary components . . .
		if (panimWeapon->m_pmeshes && m_panimCur->m_ptransWeapon)
		{
         // Show weapon sprite.
        m_spriteWeapon.flags.Hidden = false;

         m_spriteWeapon.m_pmesh		= &panimWeapon->m_pmeshes->atTime(m_lAnimTime);
         m_spriteWeapon.m_psop		= &panimWeapon->m_psops->atTime(m_lAnimTime);
         m_spriteWeapon.m_ptex		= &panimWeapon->m_ptextures->atTime(m_lAnimTime);
         m_spriteWeapon.m_psphere	= &panimWeapon->m_pbounds->atTime(m_lAnimTime);
         m_spriteWeapon.m_ptrans		= &m_panimCur->m_ptransWeapon->atTime(m_lAnimTime);
		}
		else
		{
         // Hide weapon sprite.
        m_spriteWeapon.flags.Hidden = true;
		}
	}
	else
	{
		// Hide weapon sprite.
      m_spriteWeapon.flags.Hidden = true;
	}
}

#if !defined(EDITOR_REMOVED)
////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CDoofus::EditRender(void)
{
	// In some cases, object's might need to do a special-case render in edit
	// mode because Startup() isn't called.  In this case it doesn't matter, so
	// we can call the normal Render().
	Render();
}
#endif // !defined(EDITOR_REMOVED)

////////////////////////////////////////////////////////////////////////////////
// SelectDude
////////////////////////////////////////////////////////////////////////////////

int16_t CDoofus::SelectDudeBouy(void)
{
   int16_t sResult = SUCCESS;
//	CDude* pDude;
//	CBouy* pBouytest;

	if (SelectDude() == SUCCESS)
      {
      if (m_dude)
			{
         m_ucDestBouyID = m_pNavNet->FindNearestBouy(m_dude->GetX(), m_dude->GetZ());
			}
		}
	else
      sResult = FAILURE;

/*
   if (realm()->m_asClassNumThings[CDudeID] > 0)
	{
      pDude = (CDude*) realm()->m_aclassHeads[CDudeID].GetNext();
		m_ucDestBouyID = m_pNavNet->FindNearestBouy(pDude->GetX(), pDude->GetZ());
	}
	else
	{
		if (m_pNavNet->GetNumNodes() < 1)
         sResult = FAILURE;
		else
		{
			pBouytest = nullptr;
			while (pBouytest == nullptr)		
			{
				m_ucDestBouyID = GetRandom() % m_pNavNet->GetNumNodes();
				pBouytest = m_pNavNet->GetBouy(m_ucDestBouyID);		
			}
		}
	}
*/
   return sResult;

}

////////////////////////////////////////////////////////////////////////////////
// SelectRandomBouy - make sure it exists before setting it
////////////////////////////////////////////////////////////////////////////////

uint8_t CDoofus::SelectRandomBouy(void)
{
	uint8_t ucSelect = 0;
   managed_ptr<CBouy> pBouy;

	if (m_pNavNet->GetNumNodes() <= 1)
		return 0;

   while(!pBouy)
	{
		ucSelect = GetRandom() % m_pNavNet->GetNumNodes();
		pBouy = m_pNavNet->GetBouy(ucSelect);
	}
	return ucSelect;
}

////////////////////////////////////////////////////////////////////////////////
// SelectDude - Picks the closest dude from the dude list and assignes it to
//					 this enemy's CDude pointer.
////////////////////////////////////////////////////////////////////////////////

int16_t CDoofus::SelectDude(void)
{
//	Things::iterator i;
//	Things* pDudes;

   m_dude.reset();
	uint32_t	ulSqrDistance;
   uint32_t	ulCurSqrDistance	= UINT32_MAX;
	uint32_t	ulDistX;
   uint32_t	ulDistZ;

   for(const managed_ptr<CThing>& pThing : realm()->GetThingsByType(CDudeID))
   {
     managed_ptr<CDude> pdude = pThing;
		// If this dude is not dead . . .
		if (pdude->m_state != State_Dead)
		{
         ulDistX	= pdude->position.x - position.x;
         ulDistZ	= pdude->position.z - position.z;
			ulSqrDistance	= ulDistX * ulDistX + ulDistZ * ulDistZ;
			if (ulSqrDistance < ulCurSqrDistance)
			{
				// This one is closer.
				ulCurSqrDistance	= ulSqrDistance;
            m_dude = pdude;
			}
      }
	}

   return m_dude ? SUCCESS : FAILURE;
}

////////////////////////////////////////////////////////////////////////////////
// FindDirection - gives the direction toward the cDude
////////////////////////////////////////////////////////////////////////////////

int16_t CDoofus::FindDirection()
{
	int16_t sAngle;
	double dDudeX;
	double dDudeZ;
	double dX;
	double dZ;

   if (!m_dude)
		SelectDude();

   if (m_dude)
   {
         dDudeX = m_dude->GetX();
         dDudeZ = m_dude->GetZ();
         dX = dDudeX - position.x;
         dZ = position.z - dDudeZ;
         sAngle = rspATan(dZ, dX);
	}
	else
      sAngle = rotation.y;

	return sAngle;
}

////////////////////////////////////////////////////////////////////////////////
// Return the square of the distance to the guy (to test for proximity
////////////////////////////////////////////////////////////////////////////////

double CDoofus::SQDistanceToDude()
{
	double dSquareDistance	= 0.0;
	double dX;
	double dZ;
		
   if (!m_dude)
		SelectDude();

   if (m_dude)
   {
         dX = m_dude->GetX() - position.x;
         dZ = m_dude->GetZ() - position.z;
         dSquareDistance = (dX * dX) + (dZ * dZ);
	}

	return dSquareDistance;
}


////////////////////////////////////////////////////////////////////////////////
// AlignToBouy - When timer is up, recalculate direction to bouy
////////////////////////////////////////////////////////////////////////////////

void CDoofus::AlignToBouy(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();
	if (lThisTime > m_lAlignTimer || m_bRecentlyStuck)
	{
		// If he got stuck and is now free, pick a new bouy
		if (m_bRecentlyStuck)
		{
			m_bRecentlyStuck = false;
         m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);

			if (m_ucNextBouyID > 0)
			{
				m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
            if (m_pNextBouy)
				{
					m_sNextX = m_pNextBouy->GetX();
					m_sNextZ = m_pNextBouy->GetZ();			
				}
			}
		}
		m_lAlignTimer = lThisTime + ms_lDefaultAlignTime;
      m_dAnimRot = rotation.y = rspATan(position.z - m_sNextZ, m_sNextX - position.x);
	}
}

////////////////////////////////////////////////////////////////////////////////
// TryClearDirection - Given an angle, and a variance, checks the given angle
//							  to see if it is clear.  If it is blocked by walls or fire
//							  it will try the max variance in either direction for a 
//							  clear path and set the angle if it finds one.  If these
//							  three angles fail, it will return false.
//
// Enemy guys can use this to attempt several paths before moving.  If it fails
// then they can fall back on using a random direction, or they could try again
//	with a completely different angle and variance.
////////////////////////////////////////////////////////////////////////////////

bool CDoofus::TryClearDirection(double* pdRot, int16_t sVariance)
{
	bool bFoundPath = false;
	int16_t sTries = 0;
	int16_t sX, sY, sZ;
	double dRotAttempt = *pdRot;
   managed_ptr<sprite_base_t> pthing;

	while (!bFoundPath && sTries < 2)
	{
		// Check clear path
		if (IsPathClear(							// Returns true, if the entire path is clear.
														// Returns false, if only a portion of the path is clear.
														// (see *psX, *psY, *psZ, *ppthing).
         position.x,										// In:  Starting X.
         position.y,										// In:  Starting Y.
         position.z,										// In:  Starting Z.
			dRotAttempt,							// In:  Rotation around y axis (direction on X/Z plane).
			10,										// In:  Rate at which to scan ('crawl') path in pixels per
														// iteration.
														// NOTE: We scan terrain using GetFloorAttributes()
														// so small values of sCrawl are not necessary.
														// NOTE: We could change this to a speed in pixels per second
														// where we'd assume a certain frame rate.
			200,										// In:  Range on X/Z plane.
			m_smash.m_sphere.sphere.lRadius,	// In:  Radius of path traverser.
			MaxStepUpThreshold,					// In:  Max traverser can step up.
			CSmash::Fire,							// In:  Mask of CSmash bits that would terminate path.
			CSmash::Bad | CSmash::Good,		// In:  Mask of CSmash bits that would not affect path.
			CSmash::Dead,							// In:  Mask of CSmash bits that cannot affect path.
			&sX,										// Out: Point of intercept, if any, on path.
			&sY,										// Out: Point of intercept, if any, on path.
			&sZ,										// Out: Point of intercept, if any, on path.
         pthing,									// Out: Thing that intercepted us or nullptr, if none.
			&m_smash)								// In:  Optional CSmash to exclude or nullptr, if none.
			== false)
			{
				switch (sTries)
				{
					case 0:
						dRotAttempt = rspMod360(dRotAttempt + (sVariance/2));
						break;
					case 1:
						dRotAttempt = rspMod360(dRotAttempt - sVariance);
						break;
				}
			}
			else
			{
				bFoundPath = true;
				*pdRot = dRotAttempt;
			}

			sTries++;
	}

	return bFoundPath;
}

////////////////////////////////////////////////////////////////////////////////
// TryClearShot - Works like the TryClearDirection, but doesn't check for 
//						fire, only walls.  So it knows if it could hit the target
//						from here.
////////////////////////////////////////////////////////////////////////////////

bool CDoofus::TryClearShot(double dRot, int16_t sVariance)
{
	bool bFoundPath = false;

	// If we have a weapon transform . . .
	if (m_panimCur->m_ptransRigid)
	{
		int16_t sTries = 0;
		int16_t sX, sY, sZ;
//		CThing* pthing = nullptr;
		double dRotAttempt = dRot;

		// Do a translation to the weapon position so that it is at the correct
		// height for checking

		double	dMuzzleX, dMuzzleY, dMuzzleZ;
		GetLinkPoint(														// Returns nothing.
         m_panimCur->m_ptransRigid->atTime(m_lAnimTime),	// In:  Transform specifying point.
         dMuzzleX,														// Out: Point speicfied.
         dMuzzleY,														// Out: Point speicfied.
         dMuzzleZ);														// Out: Point speicfied.			// Update execution point via link point.

		while (!bFoundPath && sTries < 2)
		{
			// Check clear path
/*
				// This one checks terrain and for people in the way
				if (IsPathClear(
               (short) (position.x + dMuzzleX),			// Start x position
               (short) (position.y + dMuzzleY),			// Start y position
               (short) (position.z + dMuzzleZ),			// Start z position
					(short) dRotAttempt,					// Angle of shot
					4,											// Crawl rate
					rspSqrt(CDoofus::SQDistanceToDude()),
					2,											// radius of traverser
					2,											// vertical tolerance
					CSmash::Bad,							// Bits that would terminate path
					0,											// Bits that would not affect path
					0,											// Bits that cannot affect path
					&sX,										// pointer to terminating point
					&sY,										// pointer to terminating point
					&sZ,										// pointer to terminating point
					&pthing,									// handle to terminating target
					&m_smash)								// exclude your own smash
					== false)
*/
				// This one only checks terrain
            if (realm()->IsPathClear(
                position.x + dMuzzleX,
                position.y + dMuzzleY,
                position.z + dMuzzleZ,
					 (int16_t) dRotAttempt,
					 3,
					 rspSqrt(CDoofus::SQDistanceToDude()),
					 0,
					 &sX,
					 &sY,
					 &sZ)
					 == false)

				{
					switch (sTries)
					{
						case 0:
							dRotAttempt = rspMod360(dRotAttempt + (sVariance/2));
							break;
						case 1:
							dRotAttempt = rspMod360(dRotAttempt - sVariance);
							break;
					}
				}
				else
				{
					bFoundPath = true;
				}

				sTries++;
		}
	}

	return bFoundPath;
}

////////////////////////////////////////////////////////////////////////////////
// Update - default implementation of logic for enemy characters
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Update(void)
{
	if (!m_sSuspend)
	{
      milliseconds_t lThisTime = realm()->m_time.GetGameTime();

		// Check for new messages that may change the state
		ProcessMessages();

		// If he has no network, then he should stick to Guard mode
//		if (!m_pNavNet)
//			m_state = State_Guard;

		// See if there are any pylons nearby that he wants to use.
		if (m_state == State_Idle || 
		    m_state == State_Wait ||
			 m_state == State_Hunt)
			Logic_PylonDetect();
	 
		switch (m_state)
		{
        UNHANDLED_SWITCH;
         case State_Guard:
				Logic_Guard();
				break;

			case State_PopBegin:
				Logic_PopBegin();
				break;

			case State_PopWait:
				Logic_PopWait();
				break;

			case State_Popout:
				Logic_Popout();
				break;

			case State_RunShootBegin:
				Logic_RunShootBegin();
				break;

			case State_RunShoot:
				Logic_RunShoot();
				break;

			case State_RunShootWait:
				Logic_RunShootWait();
				break;

			case State_Shoot:
				Logic_Shoot();
				break;

			case State_Hunt:
				Logic_Hunt();
				break;

			case State_HuntNext:
				Logic_MoveNext();
				break;

			case State_Shot:
				Logic_Shot();
				break;

			case State_BlownUp:
				Logic_BlownUp();
				break;

			case State_Burning:
				Logic_Burning();
				break;

			case State_Die:
				Logic_Die();
				break;

			case State_Dead:
				CCharacter::OnDead();
            Object::enqueue(SelfDestruct);
            return;
		}	

		// Determine appropriate position for main smash.
		PositionSmash();

		// Update the smash.
      realm()->m_smashatorium.Update(&m_smash);
		
		m_lPrevTime = lThisTime;

		// Call base class //////////////////////////////////////////////////////

		CCharacter::Update();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Shot - You got shot so check your damage
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Shot(void)
{
	if (m_stockpile.m_sHitPoints > NEAR_DEATH_HITPOINTS)
	// Restore previous state & go back to what you were doing
	{
		// If your shot animation is over...
      if (m_lAnimTime > m_panimCur->m_psops->totalTime)
		{
			// Add this check for low health -> retreat
			// if (m_stockpile.m_sHitPoints < DefHitPoints / 2)
			//{
			//	m_state = State_Retreat;
			//	m_panimCur = &m_animRun;
			//	m_lAnimTime = 0;
         //	rotation.y = rspMod360(CDoofus::FindDirection() + 180);
         //	if (CDoofus::TryClearDirection(&rotation.y, 90) == false)
         //		rotation.y = rspMod360(GetRandom());
			//}
			// 
			// else // go back to previous state
			m_state = m_ePreviousState;
			m_panimCur = m_panimPrev;
			m_lAnimTime = 0;
		}
		else
		// Else continue shot animation
		{
         m_lAnimTime += realm()->m_time.GetGameTime() - m_lPrevTime;
		}

		Cheater();
	}
	else
	// else you are dead or writhing so die
	{
		m_state = State_Die;
		m_panimCur = &m_animDie;
		m_lAnimTime = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_BlownUp - You were blown up so pop up into the air and come down again
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_BlownUp(void)
{
	// Make him animate
   m_lAnimTime += (realm()->m_time.GetGameTime() - m_lPrevTime);

	if (!WhileBlownUp())
		m_state = State_Dead;
	else
		UpdateFirePosition();

	Cheater();
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Burning - You are on fire, so run aroudn burning until you are dead
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Burning(void)
{
   m_lAnimTime += realm()->m_time.GetGameTime() - m_lPrevTime;

	if (!CCharacter::WhileBurning())
	{
		m_state = State_Die;
		m_lAnimTime = 0;
		m_panimCur = &m_animDie;
	}

	Cheater();
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Die - You are about to die, so look like it!
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Die(void)
{
	// Change our smashatorium bits
	// enable this if you want them to always writhe when dead - for testing
#if 0
	if ((m_smash.m_bits & CSmash::AlmostDead) == 0)
	{
		m_smash.m_bits |= CSmash::AlmostDead;
		m_stockpile.m_sHitPoints = 30;
	}
#else
	m_smash.m_bits |= CSmash::AlmostDead;
#endif

	// Send to back.
	m_sLayerOverride	= CRealm::LayerSprite1; 

	// When the die animation is finsihed, so are you.
   if (m_lAnimTime > m_panimCur->m_psops->totalTime)
	{
		if (m_stockpile.m_sHitPoints > 0)
		{
			m_state = State_Writhing;
			m_panimCur = &m_animWrithing;
			m_lAnimTime = 0;
			GameMessage msg;
			msg.msg_Writhing.eType = typeWrithing;
			msg.msg_Writhing.sPriority = 0;
         auto list = realm()->GetThingsByType(CDemonID);
         if (!list.empty())
            SendThingMessage(msg, list.front());
		}
		else
		{
			m_state = State_Dead;
		}
	}	
	else
   {
      if (m_weapon)
		{
			// It should drop like a rock if its a throwing weapon or just delete it if it
			// is a launched weapon
			if (m_eWeaponType == CHeatseekerID || m_eWeaponType == CRocketID || m_eWeaponType == CNapalmID)
			{
           Object::enqueue(m_weapon->SelfDestruct);
			}
			else
         {
            m_weapon->m_dHorizVel = (GetRandom() % (int16_t) CGrenade::ms_dThrowHorizVel);
            m_dShootAngle = m_weapon->rotation.y = GetRandom() % 360;
				ShootWeapon();
			}
		}
      m_lAnimTime += realm()->m_time.GetGameTime() - m_lPrevTime;

#if 0  // causes people to spin when they die. --ryan.
		// Rotate him so that he doesn't fall the same way
		// Don't rotate again if he was writhing and has been executed
		if (m_panimCur != &m_animExecuted)
		{
			short sRot = GetRandom() % 2;
			if (sRot & 0x01)
				sRot++;

         if (((short) rotation.y) & 0x01)
            m_dAnimRot = rotation.y = rspMod360(rotation.y + sRot);
			else
            m_dAnimRot = rotation.y = rspMod360(rotation.y - sRot);
		}
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Writhing - Rolling around on the ground in pain, waiting to be
//						  executed or until the pain is no longer tolerable.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Writhing(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();
   milliseconds_t lTimeDifference = lThisTime - m_lPrevTime;

	// Send to back.
	m_sLayerOverride	= CRealm::LayerSprite1; 

   if (m_lAnimTime > m_panimCur->m_psops->totalTime)
	{
		if (--m_stockpile.m_sHitPoints <= 0)
		{
			// Die. - Run the executed anim first to transition between the
			// writhing animation and being dead, otherwise some characters will
			// die on their knees.
			m_state = State_Die;
			m_panimCur = &m_animExecuted;
			m_lAnimTime = 0;
//			m_state = State_Dead;
		}
		else
		{
			// Whoa!  MOD EQUALS is deep, man!
         m_lAnimTime	%= m_panimCur->m_psops->totalTime;
			// Now you can tune the amount of blood if it gets too thick.
			if ((++m_usBloodCounter % 2) == 0)
				MakeBloodPool();

			if (lThisTime >= m_lTimer)
			{
				// Launch a sample and get the duration of the sample.
				int32_t	lSampleDuration;
				PlaySoundWrithing(&lSampleDuration);

				m_lTimer	= lThisTime	+ lSampleDuration + MS_BETWEEN_SAMPLES;
			}
		}
	}
	else
   {
      if (m_weapon)
		{
			// It should drop like a rock if its a throwing weapon or just delete it if it
			// is a launched weapon
			if (m_eWeaponType == CHeatseekerID || m_eWeaponType == CRocketID || m_eWeaponType == CNapalmID)
			{
            Object::enqueue(m_weapon->SelfDestruct);
			}
			else
         {
            m_weapon->m_dHorizVel = (GetRandom() % (int16_t) CGrenade::ms_dThrowHorizVel);
            m_dShootAngle = m_weapon->rotation.y = GetRandom() % 360;
				ShootWeapon();
			}
		}
		m_lAnimTime += lTimeDifference;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Guard - Guard the area until a CDude is nearby
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Guard(void)
{
   milliseconds_t lThisTime;
#ifdef UNUSED_VARIABLES
   milliseconds_t lTimeDifference;
	double dSeconds;
#endif

   m_eCurrentAction = Action_Guard;
	// Get new time
   lThisTime = realm()->m_time.GetGameTime();
#ifdef UNUSED_VARIABLES
   lTimeDifference = lThisTime - m_lPrevTime;

   // Calculate the elapsed time in seconds
	dSeconds = (double)(lThisTime - m_lPrevTime) / 1000.0;
#endif
//	if (m_panimCur != &m_animStand)
//	{
//		m_panimCur = &m_animStand;
//		m_lAnimTime = 0;
//	}
   rotation.y = m_dAnimRot = m_dShootAngle = FindDirection();
	RunIdleAnimation();

	// We assume at first that nothing much is going on in the area, so he
	// only evaluates CDude position every once in a while to cut down on
	// the processing.
	if (lThisTime > m_lTimer)
	{
		m_eCurrentAction = Action_Guard;
		m_lTimer = lThisTime + m_lGuardTimeout;
		if (SelectDude() == SUCCESS && SQDistanceToDude() < ms_dGuardDistance)
		{	
         if (CDoofus::TryClearShot(rotation.y, 20) == true)
			{
            PrepareWeapon();
            if (m_weapon)
				{
					// Keep it hidden, for now.
               m_weapon->flags.Hidden = true;
               m_weapon->SetRangeToTarget(rspSqrt(SQDistanceToDude()));
				}
				m_panimCur = &m_animShoot;
				m_lAnimTime = 0;
				SelectDude();
            m_dShootAngle = m_dAnimRot = rotation.y = FindDirection();
				m_state = State_Shoot;
				m_eNextState = State_Guard;
				m_lTimer = lThisTime + m_lShootTimeout;
			}
			else
			{
				m_lTimer = lThisTime + m_lShootTimeout;
			}
		}
	}
	else
		ReevaluateState();
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Patrol - Keep an eye on the CDude and look like you're doing something
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Patrol(void)
{
	// Turn to face dude but do not attack yet
   m_dShootAngle = m_dAnimRot = rotation.y = FindDirection();
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Hunt - Find a CDude, use the bouy network to get to him wherever he is
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Hunt(void)
{
	// Find the destination bouy (one closest to the Dude)
	m_eDestinationState = State_HuntHold;
	SelectDudeBouy();
   m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);
	if (m_ucNextBouyID > 0)
	{
		m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
      if (m_pNextBouy)
		{
			m_sNextX = m_pNextBouy->GetX();
			m_sNextZ = m_pNextBouy->GetZ();			
			m_state = State_HuntNext;
         m_lTimer = realm()->m_time.GetGameTime() + ms_lReseekTime;
		}
		else
		{
			// Couldn't get bouy position, so stay put
			m_state = State_Guard;
			m_eCurrentAction = Action_Guard;
		}	
	}
	else
	{
		// Couldn't find a bouy so stay put
		m_state = State_Guard;
		m_eCurrentAction = Action_Guard;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_HuntHold - This state means that you were trying to advance to the
//						  Dude and have got to the bouy closest to the Dude.  The
//						  Logic table can either break you out into Engage or 
//						  whatever, or you will stay in this state where you will 
//						  be in a guard mode, but always looking to see if you should
//						  move to a closer bouy once the Dude changes his position.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_HuntHold(void)
{
	// Evaluate the logic table suggestion to see if you should change states
	m_eCurrentAction = Action_AdvanceHold;
	if (!ReevaluateState())
	{
		// Check to see if the dude has moved and you should move	
      milliseconds_t lThisTime = realm()->m_time.GetGameTime();

		if (lThisTime > m_lTimer)
		{
			m_lTimer = lThisTime + 1000;
			SelectDudeBouy();
         m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);
			// See if he should move closer.
			if (m_ucNextBouyID != m_ucDestBouyID)
			{
				m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
            if (m_pNextBouy)
				{
					m_sNextX = m_pNextBouy->GetX();
					m_sNextZ = m_pNextBouy->GetZ();
					m_lTimer = lThisTime + ms_lReseekTime;
					m_state = State_HuntNext;
					m_eCurrentAction = Action_Advance;
				}
			}
			// Else see if he is close enough to take a shot.
			else
			{
				if (SelectDude() == SUCCESS && SQDistanceToDude() < ms_dGuardDistance)
				{	
               PrepareWeapon();
               if (m_weapon)
					{
						// Keep it hidden, for now.
                  m_weapon->flags.Hidden = true;
                  m_weapon->SetRangeToTarget(rspSqrt(SQDistanceToDude()));
					}
					m_panimCur = &m_animShoot;
					m_lAnimTime = 0;
					SelectDude();
               m_dShootAngle = m_dAnimRot = rotation.y = FindDirection();
					m_state = State_Shoot;
					m_eNextState = State_HuntHold;
					m_lTimer = lThisTime + m_lGuardTimeout;
				}
			}
		}

		// Check to see if the dude is nearby and you should shoot at him.
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_HuntNext - going to next bouy in network en route to final destination
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_MoveNext(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();
   milliseconds_t lElapsedTime = lThisTime - m_lPrevTime;
	double dSeconds = lElapsedTime / 1000.0;
   double dStartX = position.x;
   double dStartZ = position.z;

	if (!ReevaluateState())
	{
		// Make sure its using the correct animation
		if (m_state == State_MarchNext || m_state == State_WalkNext)
		{
			if (m_panimCur != &m_animWalk)
			{
				m_panimCur = &m_animWalk;
				m_lAnimTime = 0;
			}
			else
				m_lAnimTime += lElapsedTime;

			// Cheat and make him walk slower by continuously
			// resetting the velocity.
			m_dVel = ms_dMarchVelocity;
		}
		// Normally use run
		else
		{
			if (m_panimCur != &m_animRun)
			{
				m_panimCur = &m_animRun;
				m_lAnimTime = 0;
			}
			else
				m_lAnimTime += lElapsedTime;
		}

		AlignToBouy();
		m_dAcc = ms_dAccUser;

		DeluxeUpdatePosVel(dSeconds);

		// If not moving when you are trying to, rotate 
      if (position.x == dStartX && position.z == dStartZ)
		{
			if (m_sRotateDir)
            m_dAnimRot = rotation.y = rspMod360(rotation.y + 20);
			else
            m_dAnimRot = rotation.y = rspMod360(rotation.y - 20);
			// Reset alignment timer so he doesn't reseek the bouy direction immediately
			m_lAlignTimer = lThisTime + ms_lDefaultAlignTime;
			// Set a flag indicating that you were stuck, so we know what bouy to look
			// for once you get free
			m_bRecentlyStuck = true;
		}
		else
		{
			m_sRotateDir = GetRandom() % 2;
		}

		// See if we are at the next bouy yet
      double dX = position.x - m_sNextX;
      double dZ = position.z - m_sNextZ;
		double dsq = (dX * dX) + (dZ * dZ);
		if (dsq < 5*5) // Was 10*10 for a long time, trying smaller to see if it keeps guys from getting stuck
		{
			uint8_t ucNext = m_pNextBouy->NextRouteNode(m_ucDestBouyID);
			if (ucNext == 0 || ucNext == 255) // you are here or you are lost
			{
				m_state = m_eDestinationState;
				switch (m_state)
				{
              UNHANDLED_SWITCH;
               case State_Engage:
						m_eCurrentAction = Action_Engage;
						break;

					case State_Guard:
						m_eCurrentAction = Action_Guard;
						break;

					case State_March:
						m_eCurrentAction = Action_March;
						break;

					case State_Walk:
						m_eCurrentAction = Action_Walk;
						break;
				}
			}
			else
			{
				// If the reseek timer has expired, find the Dude bouy 
				// again since he may have moved
				if (lThisTime > m_lTimer || ucNext == 255)
				{
					if (m_state == State_HuntNext)
						m_state = State_Hunt;
					else
						m_lTimer = lThisTime + ms_lReseekTime;
				}
				else
				{
					m_ucNextBouyID = ucNext;
					m_pNextBouy = m_pNavNet->GetBouy(ucNext);
					m_sNextX = m_pNextBouy->GetX();
					m_sNextZ = m_pNextBouy->GetZ();
				}
			}
		}
	}
	// Check for fire in your path so you don't keep moving
	AvoidFire();
}

////////////////////////////////////////////////////////////////////////////////
// Logic_PositionSet - Get yourself set up to get into range for shooting
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_PositionSet(void)
{
	if (!ReevaluateState())
	{
      milliseconds_t lThisTime = realm()->m_time.GetGameTime();
		int16_t sVarRot = GetRandom() % 40;
		double dTargetDist = SQDistanceToDude();
		double dTestAngle;
		double dAngleTurn;

		// If he is out of range now, reset desired range to the middle value.
		if (dTargetDist < ms_dMinFightDistanceSQ || dTargetDist > ms_dMaxFightDistanceSQ)
			dTargetDist = ms_dMedFightDistanceSQ;

		bool bFoundDirection = false;
		int16_t sAttempts = 0;

		while (!bFoundDirection && sAttempts < 8)
		{
			switch (sAttempts)
			{
				case 0:
					if (sVarRot & 0x01)
                  dTestAngle = rspMod360(rotation.y + 40 + sVarRot);
					else
                  dTestAngle = rspMod360(rotation.y - 40 - sVarRot);
					dAngleTurn = 40 + sVarRot;
					break;
					
				case 1:
					if (sVarRot & 0x01)
                  dTestAngle = rspMod360(rotation.y - 40 - sVarRot);
					else
                  dTestAngle = rspMod360(rotation.y + 40 + sVarRot);
					// Same dAngleTurn as last time
					break;
					
				case 2:
					if (sVarRot & 0x01)
                  dTestAngle = rspMod360(rotation.y + 20 + sVarRot);
					else
                  dTestAngle = rspMod360(rotation.y - 20 - sVarRot);
					dAngleTurn = 20 + sVarRot;
					break;
					
				case 3:
					if (sVarRot & 0x01)
                  dTestAngle = rspMod360(rotation.y - 20 - sVarRot);
					else
                  dTestAngle = rspMod360(rotation.y + 20 + sVarRot);
					// Same dAngleTurn as last time
					break;
					
				case 4:
               dTestAngle = rspMod360(rotation.y + 95);
					break;
					
				case 5:
               dTestAngle = rspMod360(rotation.y - 95);
					break;

				case 6:
               dTestAngle = rotation.y;
					break;

				case 7:
               dTestAngle = rspMod360(rotation.y + 180);
					break;
			}

			if (CDoofus::TryClearDirection(&dTestAngle, 30) == true)
			{
				bFoundDirection = true;
				if (sAttempts < 4)
				{
               m_sDistRemaining = 2 * rspPI * rspSqrt(dTargetDist) * ((180.0-(2.0*(dAngleTurn)))/360.0);
					m_lTimer = lThisTime + (1000 * (m_sDistRemaining / ms_dMaxVelFore));		
				}
				else
				{
					m_lTimer = 800 + GetRandom() % 400;
				}
			}
			else
			{
				sAttempts++;
			}
		}

		// If you found a clear direction, take it
		if (bFoundDirection)
		{
            m_dAnimRot = rotation.y = dTestAngle;
				// Set new animation
				m_dAcc = ms_dAccUser;
				m_state = State_PositionMove;
				m_panimCur = &m_animRun;
				m_lAnimTime = 0;
		}
		else
		// else stay where you are.
		{
         m_dShootAngle = m_dAnimRot = rotation.y = FindDirection();
         if (m_dude)
			{
				m_state = State_DelayShoot;
				m_lTimer = lThisTime + ms_lDelayShootTimeout;
				m_eNextState = State_PositionSet;
				m_sStuckCounter++;
				m_lAnimTime = 0;
				if (m_sStuckCounter > ms_sStuckLimit)
				{
					m_sStuckCounter = 0;
					m_lStuckTimeout = lThisTime + ms_lStuckRecoveryTime;
					ReevaluateState();
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_DelayShoot - wait for timer to expire then shoot if clear
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_DelayShoot(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();

	// See if its time to shoot yet
	if (lThisTime > m_lTimer)
	{
      if (TryClearShot(rotation.y, 20) == true && SelectDude() == SUCCESS)
		{
         PrepareWeapon();
         if (m_weapon)
			{
            m_weapon->flags.Hidden = true;
            m_weapon->SetRangeToTarget(rspSqrt(int32_t(SQDistanceToDude())));
			}
			m_panimCur = &m_animShoot;
			m_lAnimTime = 0;
			m_state = State_Shoot;
		}
		else
		// Skip the shot and go back to the correct state
		{
			m_state = m_eNextState;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_PositionMove - Move into position then shoot
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_PositionMove(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();
   milliseconds_t lTimeDifference = lThisTime - m_lPrevTime;
	double dSeconds = lTimeDifference / 1000.0;

	if (lThisTime > m_lTimer)
	{
      m_dShootAngle = m_dAnimRot = rotation.y = FindDirection();
		// Check for clear shot before shooting.  If not clear, go back to moving
      if (CDoofus::TryClearShot(rotation.y, 20) == true && SelectDude() == SUCCESS)
		{
         PrepareWeapon();
         if (m_weapon)
			{
				// Keep it hidden, for now.
            m_weapon->flags.Hidden = true;
            m_weapon->SetRangeToTarget(rspSqrt(SQDistanceToDude()));
			}
			m_state = State_Shoot;
			m_eNextState = State_PositionSet;
			m_panimCur = &m_animShoot;
			m_lAnimTime = 0;
		}
		// If path is blocked, go on the move again and don't shoot
		else
		{
			m_state = State_PositionSet;	
		}
	}
	else
	{
      double dLastPosX = position.x;
      double dLastPosZ = position.z;

		DeluxeUpdatePosVel(dSeconds);
		m_lAnimTime += lTimeDifference;

		// If not moving when intending to, rotate one way or the other
		// to try to avoid the obstacle
      if (m_dVel != 0.0 && dLastPosX == position.x && dLastPosZ == position.z)
		{
         if (((int16_t) rotation.y) & 0x01)
            m_dAnimRot = rotation.y = rspMod360(rotation.y + 20);
			else
            m_dAnimRot = rotation.y = rspMod360(rotation.y - 20);
		}
		// Avoid fire in your path
		AvoidFire();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Engage - engage the Dude and start fighting at close range
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Engage(void)
{
   m_dShootAngle = m_dAnimRot = rotation.y = FindDirection();
	m_state = State_PositionSet;
	m_sStuckCounter = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Logic_BeSafe - Stay behind the SafeSpot bouy if the Dude is facing you
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_BeSafe(void)
{

}

////////////////////////////////////////////////////////////////////////////////
// Logic_Firefight - Shoot and move
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Firefight(void)
{

}

////////////////////////////////////////////////////////////////////////////////
// Logic_PylonDetect - If you detect that a pylon is near, find out what kind
//						     it is and decide if you will use it based on the
//							  suggested action.  This function is only called when
//							  the suggested action requires the use of a pylon.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_PylonDetect(void)
{
	CSmash* pSmashPylon;
   managed_ptr<CPylon> pPylon;
	double dSmashedX = 0.0;
	double dSmashedZ = 0.0;

	m_bPylonSafeAvailable = false;
	m_bPylonPopoutAvailable = false;
	m_bPylonRunShootAvailable = false;

	// Update the detection smash.
   m_smashDetect.m_sphere.sphere.X = position.x;
   m_smashDetect.m_sphere.sphere.Y = position.y;
   m_smashDetect.m_sphere.sphere.Z = position.z;

	// For now when we detect a pylon we are assuming that this
	// character should use that logic.  Later we will have to 
	// decide based on the personality numbers and current state.
   if (realm()->m_smashatorium.QuickCheckClosest(&m_smashDetect,
																  CSmash::Pylon,
																  0,
																  0,
																  &pSmashPylon))
	{
		ASSERT(pSmashPylon);
      pPylon = pSmashPylon->m_pThing;
		ASSERT(pPylon);

      if (pSmashPylon->m_pThing != this)
		{
			dSmashedX = pSmashPylon->m_sphere.sphere.X;
			dSmashedZ = pSmashPylon->m_sphere.sphere.Z;
			// Check IsPathClear to that thing
         if (realm()->IsPathClear(	// Returns true, if the entire path is clear.
													// Returns false, if only a portion of the path is clear.     
													// (see *psX, *psY, *psZ).                                    
               (int16_t) position.x, 				// In:  Starting X.
               (int16_t) position.y, 				// In:  Starting Y.
               (int16_t) position.z, 				// In:  Starting Z.
					3.0, 							// In:  Rate at which to scan ('crawl') path in pixels per    
													// iteration.                                                 
													// NOTE: Values less than 1.0 are inefficient.                
													// NOTE: We scan terrain using GetHeight()                    
													// at only one pixel.                                         
													// NOTE: We could change this to a speed in pixels per second 
													// where we'd assume a certain frame rate.                    
					(int16_t) dSmashedX,		// In:  Destination X.                                        
					(int16_t) dSmashedZ,		// In:  Destination Z.                                        
					0,								// In:  Max traverser can step up.                      
					nullptr,							// Out: If not nullptr, last clear point on path.                
					nullptr,							// Out: If not nullptr, last clear point on path.                
					nullptr,							// Out: If not nullptr, last clear point on path.                
					true) )						// In:  If true, will consider the edge of the realm a path
													// inhibitor.  If false, reaching the edge of the realm    
													// indicates a clear path. 
			{                                
				switch (pPylon->m_msg.msg_Generic.eType)
				{
					case typeSafeSpot:
						m_bPylonSafeAvailable = true;
						m_pPylonStart = pPylon;
						break;

					case typePopout:
						m_bPylonPopoutAvailable = true;
						m_pPylonStart = pPylon;
						break;

					case typeShootCycle:
						m_bPylonRunShootAvailable = true;
						m_pPylonStart = pPylon;
						break;

					default:
						break;
				}
			}
		}
	}
	else
	{
		// If no pylons were nearby where he started, and he was just idle,
		// then set him up as a stationary guard for now.
		if (m_state == State_Idle || m_state == State_Wait)
		{	
			// See if there are bouys, if there aren't any, then stay in Guard mode
			if (m_pNavNet->GetNumNodes() > 1)
			{
				m_state = State_Hunt;
				m_eCurrentAction = Action_Advance;
			}
			else
			{
				m_state = State_Guard;
				m_eCurrentAction = Action_Guard;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_HideBegin - Go to the hiding pylon and then wait once you get there
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_HideBegin(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();
	// If we'renot using the run animation yet then switch to it
	if (m_panimCur != &m_animRun)
	{
		m_panimCur = &m_animRun;
		m_lAnimTime = 0;
	}
	else
		m_lAnimTime += lThisTime - m_lPrevTime;

	// Set angle to pylon.
   m_dAnimRot = rotation.y = FindAngleTo(m_sNextX, m_sNextZ);
	m_dAcc = ms_dAccUser;

   int32_t lElapsedTime = realm()->m_time.GetGameTime() - m_lPrevTime;
	double dSeconds = lElapsedTime / 1000.0;
	DeluxeUpdatePosVel(dSeconds);

	//	if close to pylon, go to next state
	// for now just check the square distance, but later, probably use
	// QuickCheckCloses in smash to see if you are there yet.
   double dX = position.x - m_sNextX;
   double dZ = position.z - m_sNextZ;
	double dsq = (dX * dX) + (dZ * dZ);
	if (dsq < 300)
	{
		m_state = State_Hide;
	}
	// Check for fire in your path
	AvoidFire();
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Hide - Wait here and hope nobody sees you and blows you away.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Hide(void)
{
	ReevaluateState();
}

////////////////////////////////////////////////////////////////////////////////
// Logic_PopBegin - Once you have detected a nearby Pylon, begin the popout
//						  phase here by going to that pylon.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_PopBegin(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();
   double dStartX = position.x;
   double dStartZ = position.z;

	// If we're not using the run animation yet then switch to it
	if (m_panimCur != &m_animRun)
	{
		m_panimCur = &m_animRun;
		m_lAnimTime = 0;
	}
	else
		m_lAnimTime += lThisTime - m_lPrevTime;

	m_eCurrentAction = Action_Popout;
	// Set angle to pylon.
	if (lThisTime > m_lAlignTimer)
	{
      m_dAnimRot = rotation.y = FindAngleTo(m_sNextX, m_sNextZ);
		m_lAlignTimer = lThisTime + 100;
		m_sRotateDir = GetRandom() % 2;
	}
	m_dAcc = ms_dAccUser;

   int32_t lElapsedTime = realm()->m_time.GetGameTime() - m_lPrevTime;
	double dSeconds = lElapsedTime / 1000.0;
	DeluxeUpdatePosVel(dSeconds);

	// Check for fire in your path
	AvoidFire();

	// If not moving when meaning to, rotate to avoid obstacle
   if (position.x == dStartX && position.z == dStartZ)
	{
		if (m_sRotateDir)
         m_dAnimRot = rotation.y = rspMod360(rotation.y + 20);
		else
         m_dAnimRot = rotation.y = rspMod360(rotation.y - 20);
		m_lAlignTimer = lThisTime + 100;
	}

	//	if close to pylon, go to next state
	// for now just check the square distance, but later, probably use
	// QuickCheckCloses in smash to see if you are there yet.
   double dX = position.x - m_sNextX;
   double dZ = position.z - m_sNextZ;
	double dsq = (dX * dX) + (dZ * dZ);
	if (dsq < 300)
	{
		m_state = State_PopWait;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_PopWait - Wait at covered pylon until a CDude steps on the trigger
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_PopWait(void)
{
	// If the stand animation is not being used yet then switch to it.
//	if (m_panimCur != &m_animStand)
//	{
//		m_panimCur = &m_animStand;
//		m_lAnimTime = 0;
//	}
	RunIdleAnimation();

	if (!ReevaluateState())
	{
		m_eCurrentAction = Action_Popout;
      if (realm()->m_time.GetGameTime() > m_lTimer && m_pPylonStart->Triggered())
		{
			// Set next pylon to go to 
			m_sNextX = m_pPylonEnd->GetX();
			m_sNextZ = m_pPylonEnd->GetZ();
			m_state = State_Popout;

			m_panimCur = &m_animRun;
			m_lAnimTime = 0;
		}
	}
	else
	{
      m_pPylonStart.reset();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Popout - When at a popout bouy, if the guy is close, popout, shoot, 
//						and duck back behind the bouy.  If he is not in range, stay
//						behind the bouy and wait for some amount of time.  Reevaluate
//						logic choice based on enemy attributes.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Popout(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();
   milliseconds_t lElapsedTime = lThisTime - m_lPrevTime;
	double dSeconds = lElapsedTime / 1000.0;
	m_lAnimTime += lElapsedTime;
	m_eCurrentAction = Action_Popout;

	// Go to next pylon
   m_dAnimRot = rotation.y = FindAngleTo(m_sNextX, m_sNextZ);
	m_dAcc = ms_dAccUser;
	DeluxeUpdatePosVel(dSeconds);

	// Check for fire in your path
	AvoidFire();

	// If you are at the pylon, then go to the next state
	//	if close to pylon, go to next state
	// for now just check the square distance, but later, probably use
	// QuickCheckCloses in smash to see if you are there yet.
   double dX = position.x - m_sNextX;
   double dZ = position.z - m_sNextZ;
	double dsq;
	dsq = (dX * dX) + (dZ * dZ);
	if (dsq < 300)
	{
		m_state = State_Shoot;
		// After shooting, go back to beginning
		m_eNextState = State_PopBegin;
		m_lTimer = lThisTime + 2000 + GetRandom() % 2000;
		m_sNextX = m_pPylonStart->GetX();
		m_sNextZ = m_pPylonStart->GetZ();
      PrepareWeapon();
      if (m_weapon)
		{
			// Keep it hidden, for now.
         m_weapon->flags.Hidden = true;
		}
		m_panimCur = &m_animShoot;
		m_lAnimTime = 0;
		SelectDude();
      m_dShootAngle = m_dAnimRot = rotation.y = FindDirection();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Shoot - run the animation to the proper time and then shoot the weapon
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Shoot(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();
   milliseconds_t lTimeDifference = lThisTime - m_lPrevTime;

	// Switch behavior on weapon type.
	switch (m_eWeaponType)
	{
      case CFirestreamID:
			// If out of fire streams . . .
         if (!m_weapon)
				{
				// Must prepare more fire.
            PrepareWeapon();
            if(!m_weapon)
					break;
				}
			// Intentional fall through.
      case CShotGunID:
      case CMachineGunID:
      case CPistolID:
      case CAssaultWeaponID:
      case CDoubleBarrelID:
		case CUziID:
		case CAutoRifleID:
		case CSmallPistolID:
		{
			// Get event.
         uint8_t u8Event = m_panimCur->m_pevent->atTime(m_lAnimTime);
			// We don't care about show point for these non-object weapon types.
			// If it's time to fire the weapon . . .
			if (u8Event > 0 && lThisTime > m_lShootTimer)
			{
				// Fire!
				ShootWeapon();

				switch (m_eWeaponType)
				{
              UNHANDLED_SWITCH;
               case CShotGunID:
               case CDoubleBarrelID:
						m_lShootTimer = lThisTime + 2000;
						break;

					case CUziID:
					case CAutoRifleID:
               case CMachineGunID:
               case CAssaultWeaponID:
               case CFirestreamID:
						m_lShootTimer = lThisTime + 100;
						break;

					case CSmallPistolID:
               case CPistolID:
						m_lShootTimer = lThisTime + 300;
						break;
				}
			}
			break;
		}
		
		// This is the "I don't have a weapon" case (i.e., NoWeapon).
		case TotalIDs:
			break;

		default:
         if (m_weapon)
			{
				if (WhileHoldingWeapon(ms_u32CollideBitsInclude, ms_u32CollideBitsDontcare, ms_u32CollideBitsExclude) == true)
				{
					// Note we are done with the child.  Now we're just finishing the anim.
               ASSERT(!m_weapon);
				}
			}
			break;
	}

	// See if we are at the end of the animation or we have no weapon . . .
   if (m_lAnimTime > m_panimCur->m_psops->totalTime || m_eWeaponType == TotalIDs)
	{
		m_state = m_eNextState;
	}
	// Else advance the animation timer
	else
	{
		m_lAnimTime += lTimeDifference;					
	}
	
}

////////////////////////////////////////////////////////////////////////////////
// Logic_ShootRun - Shoot while running - use the Shoot logic to control
//						  the firing of the shot and the animation, and just add on
//						  the motion update
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_ShootRun(void)
{
   double dSeconds = (realm()->m_time.GetGameTime() - m_lPrevTime) / 1000.0;

	Logic_Shoot();

	DeluxeUpdatePosVel(dSeconds);

	// Check for fire in your path
	AvoidFire();
}

////////////////////////////////////////////////////////////////////////////////
// Logic_RunShootBegin - Set up the destination pylon and shooting interval
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_RunShootBegin(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();
   milliseconds_t lElapsedTime = lThisTime - m_lPrevTime;
   double dStartX = position.x;
   double dStartZ = position.z;

	// If we're not using the run animation yet, then switch to it
	if (m_panimCur != &m_animRun)
	{
		m_panimCur = &m_animRun;
		m_lAnimTime = 0;
	}						 
	else
		m_lAnimTime += lElapsedTime;

	// Set angle to pylon.
	if (lThisTime > m_lAlignTimer)
	{
      m_dAnimRot = rotation.y = FindAngleTo(m_sNextX, m_sNextZ);
		m_lAlignTimer = lThisTime + 100;
		m_sRotateDir = GetRandom() % 2;
	}
	m_dAcc = ms_dAccUser;

	double dSeconds = lElapsedTime / 1000.0;
	DeluxeUpdatePosVel(dSeconds);

	// Check for fire in your path
	AvoidFire();

	// If not moving when you meant to, start to rotate to avoid obstacles
   if (position.x == dStartX && position.z == dStartZ)
	{
		if (m_sRotateDir)
         m_dAnimRot = rotation.y = rspMod360(rotation.y + 20);
		else
         m_dAnimRot = rotation.y = rspMod360(rotation.y - 20);
		m_lAlignTimer = lThisTime + 100;
	}

	// If close to pylon, go to nex state
   double dX = position.x - m_sNextX;
   double dZ = position.z - m_sNextZ;
	double dsq = (dX * dX) + (dZ * dZ);
	if (dsq < 300)
	{
		// Find position of destinaiton pylon
		m_sNextX = m_pPylonEnd->GetX();
		m_sNextZ = m_pPylonEnd->GetZ();
      m_dAnimRot = rotation.y = FindAngleTo(m_sNextX, m_sNextZ);
		m_lTimer = lThisTime + m_lRunShootInterval;
//		m_state = State_RunShoot;
		m_state = State_RunShootWait;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_RunShoot - Run and Shoot logic.  When near a ShootCycle bouy, hide, 
//						  run out and shoot at a Dude, run towards other bouy, stop
//						  and shoot again, then duck behind the second bouy.  Repeat
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_RunShoot(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();
   milliseconds_t lElapsedTime = lThisTime - m_lPrevTime;

	// If its not using the run animation, then switch to it
	if (m_panimCur != &m_animRun)
	{
		m_panimCur = &m_animRun;
//		m_lAnimTime = 0;
      m_dAnimRot = rotation.y;
	}

	if (!ReevaluateState())
	{
		// Advance the run animation
		m_lAnimTime += lElapsedTime;

		// If its time to shoot then do it.
		if (lThisTime > m_lTimer)
		{
			m_state = State_ShootRun;
			// After shooting, go back to beginning
			m_eNextState = State_RunShoot;
         PrepareWeapon();
         if (m_weapon)
			{
				// Keep it hidden, for now.
            m_weapon->flags.Hidden = true;
			}
			SelectDude();

			int16_t sTargetAngle = FindDirection();
			m_dShootAngle = sTargetAngle;
         int16_t sAngleCCL = rspMod360(sTargetAngle - rotation.y);
         int16_t sAngleCL  = rspMod360((360 - sTargetAngle) + rotation.y);
			int16_t sAngleDistance = MIN(sAngleCCL, sAngleCL);
			if (sAngleCCL < sAngleCL)
			// Rotate Counter Clockwise - Use left animations
			{
				if (sAngleDistance > 150)
				{
					m_panimCur = &m_animShootRunBack;
               m_dAnimRot = rspMod360(rotation.y + (sAngleDistance - 180));
				}
				else
				{
					if (sAngleDistance > 68)
					{
						m_panimCur = &m_animShootRunL1;
                  m_dAnimRot = rspMod360(rotation.y + (sAngleDistance - 90));
					}
					else
					{
						if (sAngleDistance > 23)
						{
							m_panimCur = &m_animShootRunL0;
                     m_dAnimRot = rspMod360(rotation.y + (sAngleDistance - 45));
						}
						else
						{
							m_panimCur = &m_animShootRun;
                     m_dAnimRot = rspMod360(rotation.y + sAngleDistance);
						}
					}
				}
			}
			else
			// Rotate Clockwise - Use right animations
			{
				if (sAngleDistance > 150)
				{
					m_panimCur = &m_animShootRunBack;
               m_dAnimRot = rspMod360(rotation.y - (sAngleDistance - 180));
				}
				else
				{
					if (sAngleDistance > 68)
					{
						m_panimCur = &m_animShootRunR1;
                  m_dAnimRot = rspMod360(rotation.y - (sAngleDistance - 90));
					}
					else
					{
						if (sAngleDistance > 23)
						{
							m_panimCur = &m_animShootRunR0;
                     m_dAnimRot = rspMod360(rotation.y - (sAngleDistance - 45));
						}
						else
						{
							m_panimCur = &m_animShootRun;
                     m_dAnimRot = rspMod360(rotation.y - sAngleDistance);
						}
					}
				}
			}

			m_lTimer = lThisTime + m_lRunShootInterval;
			m_lAnimTime = 0;
		}
		else
		{
			// Set angle to pylon.
         m_dAnimRot = rotation.y = FindAngleTo(m_sNextX, m_sNextZ);
			m_dAcc = ms_dAccUser;

			double dSeconds = lElapsedTime / 1000.0;
			DeluxeUpdatePosVel(dSeconds);

			// Check for fire in your path
			AvoidFire();

			//	if close to pylon, go to next state
			// for now just check the square distance, but later, probably use
			// QuickCheckCloses in smash to see if you are there yet.
         double dX = position.x - m_sNextX;
         double dZ = position.z - m_sNextZ;
			double dsq = (dX * dX) + (dZ * dZ);
			if (dsq < 300)
			{
				m_state = State_RunShootWait;
				m_lTimer = lThisTime + m_lGuardTimeout;
				// Make him face the other way.
            m_dAnimRot = rotation.y = FindAngleTo(m_pPylonEnd->GetX(), m_pPylonEnd->GetZ());
			}
		}
	}
	else
	{
      m_pPylonStart.reset();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_RunShootWait - Wait at the endpoint before going again.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_RunShootWait(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();
	
	RunIdleAnimation();

	// If the waiting is over and its triggered
	if (lThisTime > m_lTimer && m_pPylonStart->Triggered())
	{
      managed_ptr<CPylon> pPylonTemp = m_pPylonStart;
		m_pPylonStart = m_pPylonEnd;
		m_pPylonEnd = pPylonTemp;
		m_state = State_RunShoot;
		m_sNextX = m_pPylonStart->GetX();
		m_sNextZ = m_pPylonStart->GetZ();
		m_dVel = 0.0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Retreat - Once you are low on health, you may choose this logic to
//						 run away from the Dude, taking shelter wherever you can.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Retreat(void)
{
	// Get your closest bouy, pick a random destination bouy, then
	// start traversing 3 away from your current one and set that
	// as the destination bouy, then switch to State_MoveNext

	m_eDestinationState = State_Guard;
	m_ucDestBouyID = SelectRandomBouy();
   m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);
	if (m_ucNextBouyID > 0)
	{
		m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
      if (m_pNextBouy)
		{
			m_sNextX = m_pNextBouy->GetX();
			m_sNextZ = m_pNextBouy->GetZ();
			m_state = State_MoveNext;
			m_eCurrentAction = Action_Retreat;
		}
		else
		{
			// Couldn't find bouy so stay engaged
			m_state = State_Engage;
			m_eCurrentAction = Action_Engage;
		}
	}
	else
	{
		// Couldn't find a bouy so just stay in engage.
		m_state = State_Engage;
		m_eCurrentAction = Action_Engage;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_PanicBegin - Pick a random bouy to run to and set animation
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_PanicBegin(void)
{
	// See if dude is still alive
	SelectDude();
	// Switch to correct animation if not already.
	if (m_panimCur != &m_animRun)
	{
		m_panimCur = &m_animRun;
		m_lAnimTime = 0;
	}

	m_eDestinationState = State_PanicContinue;
	m_bPanic = true;
	m_ucDestBouyID = SelectRandomBouy();
   m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);
	if (m_ucNextBouyID > 0)
	{
		m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
      if (m_pNextBouy)
		{
			m_sNextX = m_pNextBouy->GetX();
			m_sNextZ = m_pNextBouy->GetZ();
			m_state = State_MoveNext;
		}
		else
		{
			// Couldn't find bouy so stay engaged
			m_state = State_PanicBegin;
		}
	}
	else
	{
		// Couldn't find a bouy so just stay in engage.
		m_state = State_PanicBegin;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_PanicContinue - End of panic state - pick another bouy
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_PanicContinue(void)
{
	m_state = State_PanicBegin;
}

////////////////////////////////////////////////////////////////////////////////
// Logic_MarchBegin - Pick an endpoint and march to it.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_MarchBegin(void)
{
	// See if Dude is still alive
	SelectDude();

	// Switch to correct animation if not already.
	if (m_panimCur != &m_animWalk)
	{
		m_panimCur = &m_animWalk;
		m_lAnimTime = 0;
	}

	m_eDestinationState = State_March;
	m_eCurrentAction = Action_March;
	// Pick an endpoint that we are not already at.
	if (m_ucDestBouyID == m_ucSpecialBouy0ID)
		m_ucDestBouyID = m_ucSpecialBouy1ID;
	else
		m_ucDestBouyID = m_ucSpecialBouy0ID;

   m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);
	if (m_ucNextBouyID > 0)
	{
		m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
      if (m_pNextBouy)
		{
			m_sNextX = m_pNextBouy->GetX();
			m_sNextZ = m_pNextBouy->GetZ();
			m_state = State_MarchNext;
		}
		// Couldn't find a bouy - try guard mode
		else
		{
			m_state = State_Guard;
			m_eCurrentAction = Action_Guard;
		}
	}
	// Couldn't find a bouy - try guard mode
	else
	{
		m_state = State_Guard;
		m_eCurrentAction = Action_Guard;
	}

}

////////////////////////////////////////////////////////////////////////////////
// Logic_WalkBegin - Pick a random bouy to run to and set animation
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_WalkBegin(void)
{
	// See if dude is still alive
	SelectDude();
	m_eDestinationState = State_WalkContinue;
	m_ucDestBouyID = SelectRandomBouy();
   m_ucNextBouyID = m_pNavNet->FindNearestBouy(position.x, position.z);
	if (m_ucNextBouyID > 0)
	{
		m_pNextBouy = m_pNavNet->GetBouy(m_ucNextBouyID);
      if (m_pNextBouy)
		{
			m_sNextX = m_pNextBouy->GetX();
			m_sNextZ = m_pNextBouy->GetZ();
			m_state = State_WalkNext;
		}
		else
		{
			// Couldn't find bouy so stay engaged
			m_state = State_Walk;
		}
	}
	else
	{
		// Couldn't find a bouy so just stay in engage.
		m_state = State_Walk;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Logic_WalkContinue - End of walk state - pick another bouy
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_WalkContinue(void)
{
	m_state = State_WalkBegin;
}

////////////////////////////////////////////////////////////////////////////////
// Logic_Helping - Help out your buddies without moving around and screwing
//						 up your positioning
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_Helping(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();

	if (lThisTime > m_lTimer)
	{
		m_eCurrentAction = Action_Help;
		m_lTimer = lThisTime + ms_lHelpingTimeout;
		if (SelectDude() == SUCCESS && SQDistanceToDude() <= ms_lYellRadius)
		{	
         if (CDoofus::TryClearShot(rotation.y, 20) == true)
			{
            PrepareWeapon();
            if (m_weapon)
				{
					// Keep it hidden, for now.
               m_weapon->flags.Hidden = true;
               m_weapon->SetRangeToTarget(rspSqrt(SQDistanceToDude()));
				}
				m_panimCur = &m_animShoot;
				m_lAnimTime = 0;
				SelectDude();
            m_dShootAngle = m_dAnimRot = rotation.y = FindDirection();
				m_state = State_Shoot;
				m_eNextState = State_Helping;
				m_lTimer = lThisTime + ms_lHelpingTimeout;
			}
			else
			{
				m_lTimer = lThisTime + ms_lHelpingTimeout;
			}
		}
	}
	else
		ReevaluateState();
}

////////////////////////////////////////////////////////////////////////////////
// Logic_AvoidFire - Wait for fire danger to pass before going back to your
//						   previous state.  This state is set by AvoidFire function
//							when there is a fire in your path.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::Logic_AvoidFire(void)
{
	if (AvoidFire())
	{
		// Check for a different state
		ReevaluateState();
	}
	else
	// Go back to your previous state
	{
		m_state = m_ePreviousState;
		m_panimCur = m_panimPrev;
	}
}

////////////////////////////////////////////////////////////////////////////////
// AvoidFire - checks for fire in your path, returns true if there is a fire
//					danger.  If you are not already in the fire avoidance state,
//					it will save your previous state info and change your state.
////////////////////////////////////////////////////////////////////////////////

bool CDoofus::AvoidFire(void)
{
	bool bFireDanger = false;

	CSmash* pSmashed = nullptr;

	// Change this to quick check closest
   if (realm()->m_smashatorium.QuickCheck(&m_smashAvoid,
														 CSmash::Fire,	
														 CSmash::Good | CSmash::Bad, 
														 0, 
														 &pSmashed))
	{
		bFireDanger = true;

		// If its not already avoiding fire, switch to this safe state
		// and save the previous state
		if (m_state != State_AvoidFire)
		{
			m_ePreviousState = m_state;
			m_panimPrev = m_panimCur;
			m_state = State_AvoidFire;
			m_panimCur = &m_animStand;
		}
	}


	return bFireDanger;	
}

////////////////////////////////////////////////////////////////////////////////
// YellForHelp - Call this when you get shot and it will alert other enemies in
//					  the area within line of sight that you are in trouble.  Then 
//					  they can choose to react.
////////////////////////////////////////////////////////////////////////////////

void CDoofus::YellForHelp(void)
{
	CSmash* pSmashed = nullptr;
	CSmash smashYell;
	double dSmashedX;
	double dSmashedZ;
	GameMessage msg;
	msg.msg_Help.eType = typeHelp;
	msg.msg_Help.sPriority = 0;

	// Set up the yell smash
   smashYell.m_sphere.sphere.X = position.x;
   smashYell.m_sphere.sphere.Y = position.y;
   smashYell.m_sphere.sphere.Z = position.z;
	smashYell.m_sphere.sphere.lRadius = ms_lYellRadius;
	smashYell.m_bits = 0;
   smashYell.m_pThing = this;

   realm()->m_smashatorium.QuickCheckReset(&smashYell,
														  CSmash::Character | CSmash::Bad,
														  0,
														  CSmash::Good);
   while (realm()->m_smashatorium.QuickCheckNext(&pSmashed))
	{
		ASSERT(pSmashed->m_pThing);
      if (pSmashed->m_pThing != this)
		{
			dSmashedX = pSmashed->m_sphere.sphere.X;
			dSmashedZ = pSmashed->m_sphere.sphere.Z;
			// Check IsPathClear to that thing
         if (realm()->IsPathClear(	// Returns true, if the entire path is clear.
													// Returns false, if only a portion of the path is clear.     
													// (see *psX, *psY, *psZ).                                    
               (int16_t) position.x, 				// In:  Starting X.
               (int16_t) position.y, 				// In:  Starting Y.
               (int16_t) position.z, 				// In:  Starting Z.
					3.0, 							// In:  Rate at which to scan ('crawl') path in pixels per    
													// iteration.                                                 
													// NOTE: Values less than 1.0 are inefficient.                
													// NOTE: We scan terrain using GetHeight()                    
													// at only one pixel.                                         
													// NOTE: We could change this to a speed in pixels per second 
													// where we'd assume a certain frame rate.                    
					(int16_t) dSmashedX,		// In:  Destination X.                                        
					(int16_t) dSmashedZ,		// In:  Destination Z.                                        
					0,								// In:  Max traverser can step up.                      
					nullptr,							// Out: If not nullptr, last clear point on path.                
					nullptr,							// Out: If not nullptr, last clear point on path.                
					nullptr,							// Out: If not nullptr, last clear point on path.                
					true) )						// In:  If true, will consider the edge of the realm a path
													// inhibitor.  If false, reaching the edge of the realm    
													// indicates a clear path. 
			{                                
				SendThingMessage(msg, pSmashed->m_pThing);				
			}
		}
	}

}

////////////////////////////////////////////////////////////////////////////////
// ReevaluateState - Based on parameters, some randomness, current state etc,
//							come up with a new state.
//
//							Returns true if the state changed
////////////////////////////////////////////////////////////////////////////////

bool CDoofus::ReevaluateState(void)
{
	bool bChanged = false;

	// If its not already doing the suggested Action, then attempt to switch to it.
	// and if its not in one of the final death states
	if (m_eSuggestedAction != m_eCurrentAction &&
	    m_state != State_Burning &&
		 m_state != State_BlownUp &&
		 m_state != State_Die &&
		 m_state != State_Dead &&
		 m_state != State_Writhing)
	{
		switch (m_eSuggestedAction)
		{
        UNHANDLED_SWITCH;
         case Action_Guard:
				if (m_panimCur != &m_animStand &&
				    m_panimCur != &m_animCrouch &&
					 m_panimCur != &m_animSearch)
				{
					m_panimCur = &m_animStand;
					m_lAnimTime = 0;
				}
				m_state = State_Guard;
				m_eCurrentAction = Action_Guard;
				bChanged = true;
				break;

			case Action_Advance:
				if (m_eCurrentAction != Action_AdvanceHold)
				{
					m_state = State_Hunt;
					m_eCurrentAction = Action_Advance;
					bChanged = true;
				}
				break;

			case Action_Retreat:
				m_state = State_Retreat;
				m_eCurrentAction = Action_Retreat;
				bChanged = true;
				break;

			case Action_Engage:
				m_state = State_Engage;
				m_eCurrentAction = Action_Engage;
				bChanged = true;
				break;

			case Action_Walk:
				m_state = State_WalkBegin;
				m_eCurrentAction = Action_Walk;
				bChanged = true;
				break;

			case Action_March:
				m_state = State_March;
				m_eCurrentAction = Action_March;
				bChanged = true;
				break;

			case Action_Panic:
				m_state = State_PanicBegin;
				m_eCurrentAction = Action_Panic;
				bChanged = true;
				break;

			case Action_Help:
				m_state = State_Helping;
				m_eCurrentAction = Action_Help;
				bChanged = true;
				// Force it to shoot right away.
				m_lTimer = 0;
				break;

			// In these cases, if a pylon of their type is detected, it
			// will switch them to the correct state, otherwise it will leave
			// the action and the state unchanged.
			case Action_Popout:
				Logic_PylonDetect();
				if (m_bPylonPopoutAvailable)
				{
					m_pPylonEnd = m_pPylonStart->GetPylon(m_pPylonStart->m_msg.msg_Popout.ucIDNext);
               ASSERT(m_pPylonEnd);
					m_sNextX = m_pPylonStart->GetX();
					m_sNextZ = m_pPylonStart->GetZ();
					m_state = State_PopBegin;								
					m_eCurrentAction = Action_Popout;
					bChanged = true;
				}
				break;

			case Action_RunShoot:
				Logic_PylonDetect();
				if (m_bPylonRunShootAvailable)
				{
					m_pPylonEnd = m_pPylonStart->GetPylon(m_pPylonStart->m_msg.msg_ShootCycle.ucIDNext);
               ASSERT(m_pPylonEnd);
					m_sNextX = m_pPylonStart->GetX();
					m_sNextZ = m_pPylonStart->GetZ();
					m_state = State_RunShootBegin;
					m_eCurrentAction = Action_RunShoot;
					bChanged = true;
				}
				break;

			case Action_Hide:
				Logic_PylonDetect();
				if (m_bPylonSafeAvailable)
				{
					m_sNextX = m_pPylonStart->GetX();
					m_sNextZ = m_pPylonStart->GetZ();
					m_state = State_HideBegin;
					m_eCurrentAction = Action_Hide;
					bChanged = true;
				}
				break;
		}
	}
	return bChanged;
}

////////////////////////////////////////////////////////////////////////////////
// Message handlers
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Shot Message
////////////////////////////////////////////////////////////////////////////////

void CDoofus::OnShotMsg(Shot_Message* pMessage)
{
	// If he is already shot, just deduct hit points for
	// the additional bullets
	m_stockpile.m_sHitPoints -= pMessage->sDamage;

	if (m_state != State_Burning	&& 
	    m_state != State_BlownUp	&&
		 m_state != State_Die		&& 
		 m_state != State_Dead)
	{
		// Alert other in the area that you are being attacked.
		YellForHelp();

		// Set panic so that victims will panic after being shot
		m_bPanic = true;

		// If he got shot while he was writhing, then he was executed
		// so switch to that animation.
		if (m_state == State_Writhing)
		{
			m_stockpile.m_sHitPoints = 0;
			m_panimCur = &m_animExecuted;
			m_lAnimTime = 0;
			AbortSample(m_siPlaying);
		}
		else
		{
         milliseconds_t lThisTime = realm()->m_time.GetGameTime();
			if (lThisTime > m_lShotTimeout)
			{
				m_lShotTimeout = lThisTime + m_lShotReactionTimeout;

				PlaySoundShot();

				if (	m_state != State_Shot		&&
						m_state != State_Writhing)
				{
					m_ePreviousState = m_state;
					m_panimPrev = m_panimCur;
					m_state = State_Shot;
					m_panimCur = &m_animShot;
					m_lAnimTime = 0;
				}
			}
			// Give him time to escape and move without starting the full 
			// shot animation again
			else
			{
				PlaySoundShot();
			}
		}
	}

	// Let's have some blood no matter what!
	CCharacter::OnShotMsg(pMessage);
}

////////////////////////////////////////////////////////////////////////////////
// Explosion message
////////////////////////////////////////////////////////////////////////////////

void CDoofus::OnExplosionMsg(Explosion_Message* pMessage)
{
	if (
	    m_state != State_BlownUp	&&
		 m_state != State_Die		&& 
		 m_state != State_Dead)
	{
		CCharacter::OnExplosionMsg(pMessage);
		
//		PlaySample(g_smidBlownupYell);
		PlaySoundBlownup();
		m_ePreviousState = m_state;
		m_state = State_BlownUp;
		m_panimPrev = m_panimCur;
		m_panimCur = &m_animDie;
		m_lAnimTime = 0;
		m_stockpile.m_sHitPoints = 0;
      m_lTimer = realm()->m_time.GetGameTime() + 6000;
		GameMessage msg;
		msg.msg_Explosion.eType = typeExplosion;
		msg.msg_Explosion.sPriority = 0;
      auto list = realm()->GetThingsByType(CDemonID);
      if (!list.empty())
         SendThingMessage(msg, list.front());
		// If he has a weapon ready, either get rid of it or shoot it off randomly
      if (m_weapon)
		{
			// It should drop like a rock if its a throwing weapon or just delete it if it
			// is a launched weapon
			if (m_eWeaponType == CHeatseekerID || m_eWeaponType == CRocketID || m_eWeaponType == CNapalmID)
			{
            Object::enqueue(m_weapon->SelfDestruct);
			}
			else
         {
            m_weapon->m_dHorizVel = (GetRandom() % (int16_t) CGrenade::ms_dThrowHorizVel);
            m_dShootAngle = m_weapon->rotation.y = GetRandom() % 360;
				ShootWeapon();
			}
		}

	}
}

////////////////////////////////////////////////////////////////////////////////
// Burning message
////////////////////////////////////////////////////////////////////////////////

void CDoofus::OnBurnMsg(Burn_Message* pMessage)
{
	CCharacter::OnBurnMsg(pMessage);
	m_stockpile.m_sHitPoints -= pMessage->sDamage;

	if (m_state != State_Burning	&& 
	    m_state != State_BlownUp	&&
		 m_state != State_Die		&& 
		 m_state != State_Dead)
	{
		PlaySoundBurning();

		if (m_state == State_Writhing)
		{
			m_sBrightness = BURNT_BRIGHTNESS;
			// May also have to create a small fire at this position
			m_stockpile.m_sHitPoints = 0;
			m_state = State_Dead;
		}
		else
		{
			m_ePreviousState = m_state;
			m_state = State_Burning;

			// If he's not on fire already, switch to on fire animation
			if (m_panimCur != &m_animOnfire)
			{
				m_panimPrev = m_panimCur;
				m_panimCur = &m_animOnfire;
				m_lAnimTime = 0;
				GameMessage msg;
				msg.msg_Burn.eType = typeBurn;
				msg.msg_Burn.sPriority = 0;
            auto list = realm()->GetThingsByType(CDemonID);
            if (!list.empty())
               SendThingMessage(msg, list.front());
			}
		}
		// If he has a weapon ready, either get rid of it or shoot it off randomly.
      if (m_weapon)
		{
			// It should drop like a rock if its a throwing weapon or just delete it if it
			// is a launched weapon
			if (m_eWeaponType == CHeatseekerID || m_eWeaponType == CRocketID || m_eWeaponType == CNapalmID)
			{
            Object::enqueue(m_weapon->SelfDestruct);
			}
			else
         {
            m_weapon->m_dHorizVel = (GetRandom() % (int16_t) CGrenade::ms_dThrowHorizVel);
            m_dShootAngle = m_weapon->rotation.y = GetRandom() % 360;
				ShootWeapon();
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// Handles a Help_Message
// (virtual)
////////////////////////////////////////////////////////////////////////////////
void CDoofus::OnHelpMsg(					// Returns nothing
	Help_Message* phelpmsg)					// In:  Message to handle
	{
  UNUSED(phelpmsg);
   m_lLastHelpCallTime = realm()->m_time.GetGameTime();
	// Victium take the yell for help as a indication to panic.
	m_bPanic = true;							
						
	}

////////////////////////////////////////////////////////////////////////////////
// OnDead - override of CCharacter version
////////////////////////////////////////////////////////////////////////////////

void CDoofus::OnDead(void)
{
	CCharacter::OnDead();
}

////////////////////////////////////////////////////////////////////////////////
// Prepare current weapon (ammo).
// This should be done when the character starts its shoot animation.
// (virtual (overriden here) ).
////////////////////////////////////////////////////////////////////////////////
void CDoofus::PrepareWeapon(void)	// Returns the weapon ptr or nullptr.
{
	// Play sound even if we have no weapon...seems like they're threatening him.
	PlaySoundShooting();
   CCharacter::PrepareWeapon();
}

////////////////////////////////////////////////////////////////////////////////
// ShootWeapon - Overloaded version to use the Shooting Angle
////////////////////////////////////////////////////////////////////////////////

void CDoofus::ShootWeapon(CSmash::Bits bitsInclude,
										CSmash::Bits bitsDontcare,
										CSmash::Bits bitsExclude)
{
   double dSaveDirection = rotation.y;

	// Don't adjust shooting angle if shooting on the run
	if (m_state != State_ShootRun)
	{
		// Tuned for difficulty level - if the game is in the harder
		// levels, the enemies will tune their aim at the last second
		// before shooting, and then also depending on the difficulty,
		// they will add different amounts of random misses
		// Easy levels   = 1, 2, 3
		// Medium levels = 4, 5, 6
		// Hard levels   = 7, 8, 9, 10, 11
      switch (realm()->m_flags.sDifficulty)
		{
			case 0:
			case 1:
            rotation.y = rspMod360(m_dShootAngle - 8 + (GetRandom() % 17));
				break;

			case 2:
            rotation.y = rspMod360(m_dShootAngle - 7 + (GetRandom() % 15));
				break;

			case 3:
            rotation.y = rspMod360(m_dShootAngle - 6 + (GetRandom() % 13));
				break;

			case 4:
            rotation.y = m_dAnimRot = m_dShootAngle = dSaveDirection = rspMod360(FindDirection() - 6 + (GetRandom() % 13));
				break;

			case 5:
            rotation.y = m_dAnimRot = m_dShootAngle = dSaveDirection = rspMod360(FindDirection() - 5 + (GetRandom() % 11));
				break;

			case 6:
            rotation.y = m_dAnimRot = m_dShootAngle = dSaveDirection = rspMod360(FindDirection() - 4 + (GetRandom() % 9));
				break;

			case 7:
            rotation.y = m_dAnimRot = m_dShootAngle = dSaveDirection = rspMod360(FindDirection() - 6 + (GetRandom() % 13));
				break;

			case 8:
            rotation.y = m_dAnimRot = m_dShootAngle = dSaveDirection = rspMod360(FindDirection() - 5 + (GetRandom() % 11));
				break;

			case 9:
            rotation.y = m_dAnimRot = m_dShootAngle = dSaveDirection = rspMod360(FindDirection() - 4 + (GetRandom() % 9));
				break;

			case 10:
            rotation.y = m_dAnimRot = m_dShootAngle = dSaveDirection = rspMod360(FindDirection() - 3 + (GetRandom() % 7));
				break;

			case 11:
			default:
            rotation.y = m_dAnimRot = m_dShootAngle = dSaveDirection = FindDirection();
				break;
		}
	}
	else
	{
      rotation.y = m_dShootAngle;
	}
/*
	// Allow rockets to hit other enemies, but still not the special barrel type
	if (m_eWeaponType == CHeatseekerID || m_eWeaponType == CRocketID)
	{
		bitsExclude &= ~CSmash::Bad;
	}
*/
   CCharacter::ShootWeapon(bitsInclude, bitsDontcare, bitsExclude);
   if (m_weapon)
      m_weapon->SetDetectionBits(CSmash::Character | CSmash::Fire,
									  0,
									  CSmash::Bad | CSmash::Ducking | CSmash::AlmostDead);

   rotation.y = dSaveDirection;
}

////////////////////////////////////////////////////////////////////////////////
// RunIdleAnimation - Check the idle timer and pick an idle animation to run
////////////////////////////////////////////////////////////////////////////////

void CDoofus::RunIdleAnimation(void)
{
   milliseconds_t lThisTime = realm()->m_time.GetGameTime();

	if (m_panimCur == &m_animStand)
	{
		if (lThisTime > m_lIdleTimer)
		{
			if (GetRandom() % 3 == 0)
			{
				// Stay with current animation
				m_lIdleTimer = lThisTime + 2000 + GetRandom() % 2000;
			}
			else
			{
				// Go to the Crouch animaiton
				m_panimCur = &m_animCrouch;
				m_lAnimTime = 0;
				m_bAnimUp = true;
				m_lIdleTimer = lThisTime + 3000 + GetRandom() % 2000;
			}
		}
	}
	else
	{
		if (m_panimCur == &m_animCrouch)
		{
			// Animate forward or backward depending on if he is standing up or
			if (m_bAnimUp)
			{
				m_lAnimTime += lThisTime - m_lPrevTime;
				// See if it is time to switch yet
				if (lThisTime > m_lIdleTimer)
				{
					if ((GetRandom() % 3 ) == 0)
					// Start standing up to use the stand animation
					{
						// Start to stand up - run the animation backwards
                  m_lAnimTime = m_panimCur->m_psops->totalTime;
						m_bAnimUp = false;
					}
					else
					{
						// Pick either crouch still or search
						if (GetRandom() % 2)
						{
							// Start search animation
							m_panimCur = &m_animSearch;
							m_lAnimTime = 0;
							m_lIdleTimer = lThisTime + 1000 + GetRandom() % 1500;
						}
						else
						{
							// Stay with still crouch animation
							m_lIdleTimer = lThisTime + 2000 + GetRandom() % 2000;
						}
					}
				}
			}
			else
			{
				m_lAnimTime -= lThisTime - m_lPrevTime;
				// See if it is done standing yet.  If so, you have
				// already previously decided to end the crouch animation
				// so now pick the stand animation
            if (m_lAnimTime > INT32_MAX)
				{
					m_panimCur = &m_animStand;
					m_lAnimTime = 0;
					m_lIdleTimer = lThisTime + 3000 + GetRandom() % 2000;
				}
			}
		}
		else
		{
			if (m_panimCur == &m_animSearch)
			{
				// Animate the search 
				m_lAnimTime += lThisTime - m_lPrevTime;
				// See if its time to switch yet
				if (lThisTime > m_lIdleTimer)
				{
					switch (GetRandom() % 3)
					{
						case 0:	// Stay with current animation
							m_lIdleTimer = lThisTime + 1000 + GetRandom() % 1000;
							break;

						case 1:	// Go to Crouch animation
							m_panimCur = &m_animCrouch;
							m_bAnimUp = true;
							m_lIdleTimer = lThisTime + 3000 + GetRandom() % 2000;
							break;

						case 2:	// Go to stand - use Crouch backwards to get up
							m_panimCur = &m_animCrouch;
							m_bAnimUp = false;
                     m_lAnimTime = m_panimCur->m_psops->totalTime;
							m_lIdleTimer = lThisTime + 3000 + GetRandom() % 2500;
							break;
					}					
				}
			}
			// Start the stand animation since it isn't currently using any idle animations.
			else
			{
				m_panimCur = &m_animStand;
				m_lIdleTimer = lThisTime + 3000;
				m_lAnimTime = 0;			
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Position our smash approriately.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CDoofus::PositionSmash(void)
	{
	// If not in writhing state . . .
	if (m_state != State_Writhing)
		{
		// Update sphere.
      m_smash.m_sphere.sphere.X			= position.x;
      m_smash.m_sphere.sphere.Y			= position.y + m_sRadius;
      m_smash.m_sphere.sphere.Z			= position.z;
      m_smash.m_sphere.sphere.lRadius	= m_sRadius;
		}
	else
		{
		// If we have the execution points . . .
		if (m_ptransExecutionTarget)
			{
			// Compute link point for execution target.
			double	dVitalOrganX;
			double	dVitalOrganY;
			double	dVitalOrganZ;
			GetLinkPoint(														// Returns nothing.
            m_ptransExecutionTarget->atTime(m_lAnimTime),	// In:  Transform specifying point.
            dVitalOrganX,													// Out: Point speicfied.
            dVitalOrganY,													// Out: Point speicfied.
            dVitalOrganZ);												// Out: Point speicfied.			// Update execution point via link point.

			// Offset from hotspot to set collision sphere position.
         m_smash.m_sphere.sphere.X			= position.x + dVitalOrganX;
         m_smash.m_sphere.sphere.Y			= position.y + dVitalOrganY;
         m_smash.m_sphere.sphere.Z			= position.z + dVitalOrganZ;
         m_smash.m_sphere.sphere.lRadius	= m_sRadius;
			}
		else
			{
         ASSERT(rotation.y >= 0);
         ASSERT(rotation.y < 360);
			// Try to find center.
			// Let's go a radius up their torso.  Say... .
         // This only looks decent if rotation.y is the direction they fell which is
			// not always the case.
         int16_t	sPseudoCenter	= m_sRadius;
         m_smash.m_sphere.sphere.X			= position.x + COSQ[int16_t(rotation.y)] * sPseudoCenter;
         m_smash.m_sphere.sphere.Y			= position.y + m_sRadius;
         m_smash.m_sphere.sphere.Z			= position.z - SINQ[int16_t(rotation.y)] * sPseudoCenter;
         m_smash.m_sphere.sphere.lRadius	= m_sRadius;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
//	WhileHoldingWeapon
//
// Override for the character version which will just re-aim the guy during the
// shoot-prepare frames.  Otherwise, guys like the rocketman which has a long
// prepare animation, quickly flip around in ShootWeapon if the target has moved
// between PrepareWeapon and ShootWeapon.
//
////////////////////////////////////////////////////////////////////////////////

bool CDoofus::WhileHoldingWeapon(	// Returns true when weapon is released.
				uint32_t u32BitsInclude,		// In:  Collision bits passed to ShootWeapon
				uint32_t u32BitsDontcare,		// In:  Collision bits passed to ShootWeapon
				uint32_t u32BitsExclude)		// In:  Collision bits passed to ShootWeapon
{
	bool bResult = true;

	// If they haven't shot the weapon yet, adjust their aiming if the difficulty
	// level allows them to re-aim after preparing the weapon
	if (!(CCharacter::WhileHoldingWeapon(u32BitsInclude, u32BitsDontcare, u32BitsExclude)))
	{
		bResult = false;
		// Don't adjust shooting angle if shooting on the run
		if (m_state != State_ShootRun)
		{
			// Tuned for difficulty level - if the game is in the harder
			// levels, the enemies will tune their aim at the last second
			// before shooting, and then also depending on the difficulty,
			// they will add different amounts of random misses
			// Easy levels   = 1, 2, 3
			// Medium levels = 4, 5, 6
			// Hard levels   = 7, 8, 9, 10, 11
         switch (realm()->m_flags.sDifficulty)
			{
				// Easy levels don't re-aim after preparing weapon, so leve it alone
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
					break;

				// More difficult levels allow constant re-aiming.
				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
				default:
               rotation.y = m_dAnimRot = m_dShootAngle = rspMod360(FindDirection());
					break;
			}
		}
	}
	return bResult;
}


////////////////////////////////////////////////////////////////////////////////
// A way for the base class to get resources.  If you are going to use
// any of this class's resources (e.g., ms_aanimWeapons[]), call this
// when getting your resources.
////////////////////////////////////////////////////////////////////////////////
int16_t CDoofus::GetResources(void)
{
  int16_t sResult = SUCCESS;

  // If the ref count was 0 . . .
  if (ms_lWeaponResRefCount++ == 0)
  {
    // Get the actual resources.
    int16_t	i;
    int16_t	sLoadResult;
    for (i = 0; i < NumWeaponTypes; i++)
    {
      // If this weapon has a visible resource . . .
      if (ms_awdWeapons[i].pszResName)
      {
        sLoadResult	= ms_aanimWeapons[i].Get(ms_awdWeapons[i].pszResName) ? SUCCESS : FAILURE;
        if(sLoadResult == SUCCESS)
          ms_aanimWeapons[i].SetLooping(RChannel_LoopAtStart | RChannel_LoopAtEnd);
        else
        {
          TRACE("GetResources(): Failed to load weapon resource \"%s\".\n",
                ms_awdWeapons[i].pszResName);
          sResult = FAILURE;
        }
      }
    }
  }

  return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// A way for the base class to release resources.  If you are going to use
// any of this class's resources (e.g., ms_aanimWeapons[]), call this
// when releasing your resources.
////////////////////////////////////////////////////////////////////////////////
void CDoofus::ReleaseResources(void)
	{
	ASSERT(ms_lWeaponResRefCount > 0);

	// If the ref count hits zero with this release . . .
	if (--ms_lWeaponResRefCount == 0)
		{
		// Release the actual resources.
		int16_t	i;
		for (i = 0; i < NumWeaponTypes; i++)
			{
			if (ms_aanimWeapons[i].m_pmeshes)
				{
				ms_aanimWeapons[i].Release();
				}
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
