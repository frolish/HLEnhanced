/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Squadmonster  functions
//=========================================================
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes/Nodes.h"
#include "entities/NPCs/Monsters.h"
#include "animation.h"
#include "SaveRestore.h"
#include "CSquadMonster.h"
#include "CPlane.h"

//=========================================================
// Save/Restore
//=========================================================
BEGIN_DATADESC(	CSquadMonster )
	DEFINE_FIELD( m_hSquadLeader, FIELD_EHANDLE ),
	DEFINE_ARRAY( m_hSquadMember, FIELD_EHANDLE, MAX_SQUAD_MEMBERS - 1 ),

	// DEFINE_FIELD( m_afSquadSlots, FIELD_INTEGER ), // these need to be reset after transitions!
	DEFINE_FIELD( m_fEnemyEluded, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flLastEnemySightTime, FIELD_TIME ),

	DEFINE_FIELD( m_iMySlot, FIELD_INTEGER ),
END_DATADESC()

//=========================================================
// OccupySlot - if any slots of the passed slots are 
// available, the monster will be assigned to one.
//=========================================================
bool CSquadMonster::OccupySlot( int iDesiredSlots )
{
	int i;
	int iMask;
	int iSquadSlots;

	if ( !InSquad() )
	{
		return true;
	}

	if ( SquadEnemySplit() )
	{
		// if the squad members aren't all fighting the same enemy, slots are disabled
		// so that a squad member doesn't get stranded unable to engage his enemy because
		// all of the attack slots are taken by squad members fighting other enemies.
		m_iMySlot = bits_SLOT_SQUAD_SPLIT;
		return true;
	}

	CSquadMonster *pSquadLeader = MySquadLeader();

	if ( !( iDesiredSlots ^ pSquadLeader->m_afSquadSlots ) )
	{
		// none of the desired slots are available. 
		return false;
	}

	iSquadSlots = pSquadLeader->m_afSquadSlots;

	for ( i = 0; i < NUM_SLOTS; i++ )
	{
		iMask = 1<<i;
		if ( iDesiredSlots & iMask ) // am I looking for this bit?
		{
			if ( !(iSquadSlots & iMask) )	// Is it already taken?
			{
				// No, use this bit
				pSquadLeader->m_afSquadSlots |= iMask;
				m_iMySlot = iMask;
//				ALERT ( at_aiconsole, "Took slot %d - %d\n", i, m_hSquadLeader->m_afSquadSlots );
				return true;
			}
		}
	}

	return false;
}

//=========================================================
// VacateSlot 
//=========================================================
void CSquadMonster :: VacateSlot()
{
	if ( m_iMySlot != bits_NO_SLOT && InSquad() )
	{
//		ALERT ( at_aiconsole, "Vacated Slot %d - %d\n", m_iMySlot, m_hSquadLeader->m_afSquadSlots );
		MySquadLeader()->m_afSquadSlots &= ~m_iMySlot;
		m_iMySlot = bits_NO_SLOT;
	}
}

//=========================================================
// ScheduleChange
//=========================================================
void CSquadMonster :: ScheduleChange ( void )
{
	VacateSlot();
}

//=========================================================
// Killed
//=========================================================
void CSquadMonster::Killed( const CTakeDamageInfo& info, GibAction gibAction )
{
	VacateSlot();

	if ( InSquad() )
	{
		MySquadLeader()->SquadRemove( this );
	}

	CBaseMonster::Killed( info, gibAction );
}

// These functions are still awaiting conversion to CSquadMonster 


//=========================================================
//
// SquadRemove(), remove pRemove from my squad.
// If I am pRemove, promote m_pSquadNext to leader
//
//=========================================================
void CSquadMonster :: SquadRemove( CSquadMonster *pRemove )
{
	ASSERT( pRemove!=NULL );
	ASSERT( this->IsLeader() );
	ASSERT( pRemove->m_hSquadLeader == this );

	// If I'm the leader, get rid of my squad
	if (pRemove == MySquadLeader())
	{
		for (int i = 0; i < MAX_SQUAD_MEMBERS-1;i++)
		{
			CSquadMonster *pMember = MySquadMember(i);
			if (pMember)
			{
				pMember->m_hSquadLeader = NULL;
				m_hSquadMember[i] = NULL;
			}
		}
	}
	else
	{
		CSquadMonster *pSquadLeader = MySquadLeader();
		if (pSquadLeader)
		{
			for (int i = 0; i < MAX_SQUAD_MEMBERS-1;i++)
			{
				if (pSquadLeader->m_hSquadMember[i] == this)
				{
					pSquadLeader->m_hSquadMember[i] = NULL;
					break;
				}
			}
		}
	}

	pRemove->m_hSquadLeader = NULL;
}

//=========================================================
//
// SquadAdd(), add pAdd to my squad
//
//=========================================================
bool CSquadMonster::SquadAdd( CSquadMonster *pAdd )
{
	ASSERT( pAdd!=NULL );
	ASSERT( !pAdd->InSquad() );
	ASSERT( this->IsLeader() );

	for (int i = 0; i < MAX_SQUAD_MEMBERS-1; i++)
	{
		if (m_hSquadMember[i] == NULL)
		{
			m_hSquadMember[i] = pAdd;
			pAdd->m_hSquadLeader = this;
			return true;
		}
	}
	return false;
	// should complain here
}


//=========================================================
// 
// SquadPasteEnemyInfo - called by squad members that have
// current info on the enemy so that it can be stored for 
// members who don't have current info.
//
//=========================================================
void CSquadMonster :: SquadPasteEnemyInfo ( void )
{
	CSquadMonster *pSquadLeader = MySquadLeader( );
	if (pSquadLeader)
		pSquadLeader->m_vecEnemyLKP = m_vecEnemyLKP;
}

//=========================================================
//
// SquadCopyEnemyInfo - called by squad members who don't
// have current info on the enemy. Reads from the same fields
// in the leader's data that other squad members write to,
// so the most recent data is always available here.
//
//=========================================================
void CSquadMonster :: SquadCopyEnemyInfo ( void )
{
	CSquadMonster *pSquadLeader = MySquadLeader( );
	if (pSquadLeader)
		m_vecEnemyLKP = pSquadLeader->m_vecEnemyLKP;
}

//=========================================================
// 
// SquadMakeEnemy - makes everyone in the squad angry at
// the same entity.
//
//=========================================================
void CSquadMonster :: SquadMakeEnemy ( CBaseEntity *pEnemy )
{
	if (!InSquad())
		return;

	if ( !pEnemy )
	{
		ALERT ( at_console, "ERROR: SquadMakeEnemy() - pEnemy is NULL!\n" );
		return;
	}

	CSquadMonster *pSquadLeader = MySquadLeader( );
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
		if (pMember)
		{
			// reset members who aren't activly engaged in fighting
			if (pMember->m_hEnemy != pEnemy && !pMember->HasConditions( bits_COND_SEE_ENEMY))
			{
				if ( pMember->m_hEnemy != NULL) 
				{
					// remember their current enemy
					pMember->PushEnemy( pMember->m_hEnemy, pMember->m_vecEnemyLKP );
				}
				// give them a new enemy
				pMember->m_hEnemy = pEnemy;
				pMember->m_vecEnemyLKP = pEnemy->GetAbsOrigin();
				pMember->SetConditions ( bits_COND_NEW_ENEMY );
			}
		}
	}
}


//=========================================================
//
// SquadCount(), return the number of members of this squad
// callable from leaders & followers
//
//=========================================================
int CSquadMonster :: SquadCount( void )
{
	if (!InSquad())
		return 0;

	CSquadMonster *pSquadLeader = MySquadLeader();
	int squadCount = 0;
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		if (pSquadLeader->MySquadMember(i) != NULL)
			squadCount++;
	}

	return squadCount;
}


//=========================================================
//
// SquadRecruit(), get some monsters of my classification and
// link them as a group.  returns the group size
//
//=========================================================
int CSquadMonster :: SquadRecruit( int searchRadius, int maxMembers )
{
	int squadCount;
	EntityClassification_t iMyClass = Classify();// cache this monster's class


	// Don't recruit if I'm already in a group
	if ( InSquad() )
		return 0;

	if ( maxMembers < 2 )
		return 0;

	// I am my own leader
	m_hSquadLeader = this;
	squadCount = 1;

	CBaseEntity *pEntity = NULL;

	if ( HasNetName() )
	{
		// I have a netname, so unconditionally recruit everyone else with that name.
		pEntity = UTIL_FindEntityByString( pEntity, "netname", GetNetName() );
		while ( pEntity )
		{
			CSquadMonster *pRecruit = pEntity->MySquadMonsterPointer();

			if ( pRecruit )
			{
				if ( !pRecruit->InSquad() && pRecruit->Classify() == iMyClass && pRecruit != this )
				{
					// minimum protection here against user error.in worldcraft. 
					if (!SquadAdd( pRecruit ))
						break;
					squadCount++;
				}
			}
	
			pEntity = UTIL_FindEntityByString( pEntity, "netname", GetNetName() );
		}
	}
	else 
	{
		while ((pEntity = UTIL_FindEntityInSphere( pEntity, GetAbsOrigin(), searchRadius )) != NULL)
		{
			CSquadMonster *pRecruit = pEntity->MySquadMonsterPointer( );

			if ( pRecruit && pRecruit != this && pRecruit->IsAlive() && !pRecruit->m_pCine )
			{
				// Can we recruit this guy?
				if ( !pRecruit->InSquad() && pRecruit->Classify() == iMyClass &&
				   ( (iMyClass != EntityClassifications().GetClassificationId( classify::ALIEN_MONSTER )) || FStrEq( GetClassname(), pRecruit->GetClassname() )) &&
				    !pRecruit->HasNetName() )
				{
					TraceResult tr;
					UTIL_TraceLine( GetAbsOrigin() + GetViewOffset(), pRecruit->GetAbsOrigin() + GetViewOffset(), ignore_monsters, pRecruit->edict(), &tr );// try to hit recruit with a traceline.
					if ( tr.flFraction == 1.0 )
					{
						if (!SquadAdd( pRecruit ))
							break;

						squadCount++;
					}
				}
			}
		}
	}

	// no single member squads
	if (squadCount == 1)
	{
		m_hSquadLeader = NULL;
	}

	return squadCount;
}

//=========================================================
// CheckEnemy
//=========================================================
bool CSquadMonster::CheckEnemy( CBaseEntity *pEnemy )
{
	bool bUpdatedLKP = CBaseMonster::CheckEnemy( m_hEnemy );
	
	// communicate with squad members about the enemy IF this individual has the same enemy as the squad leader.
	if ( InSquad() && (CBaseEntity *)m_hEnemy == MySquadLeader()->m_hEnemy )
	{
		if ( bUpdatedLKP )
		{
			// have new enemy information, so paste to the squad.
			SquadPasteEnemyInfo();
		}
		else
		{
			// enemy unseen, copy from the squad knowledge.
			SquadCopyEnemyInfo();
		}
	}

	return bUpdatedLKP;
}

//=========================================================
// StartMonster
//=========================================================
void CSquadMonster :: StartMonster( void )
{
	CBaseMonster :: StartMonster();

	if ( ( m_afCapability & bits_CAP_SQUAD ) && !InSquad() )
	{
		if ( HasNetName() )
		{
			// if I have a groupname, I can only recruit if I'm flagged as leader
			if ( !( pev->spawnflags & SF_SQUADMONSTER_LEADER ) )
			{
				return;
			}
		}

		// try to form squads now.
		int iSquadSize = SquadRecruit( 1024, 4 );

		if ( iSquadSize )
		{
		  ALERT ( at_aiconsole, "Squad of %d %s formed\n", iSquadSize, GetClassname() );
		}

		if ( IsLeader() && ClassnameIs( "monster_human_grunt" ) )
		{
			SetBodygroup( 1, 1 ); // UNDONE: truly ugly hack
			pev->skin = 0;
		}

	}
}

//=========================================================
// NoFriendlyFire - checks for possibility of friendly fire
//
// Builds a large box in front of the grunt and checks to see 
// if any squad members are in that box. 
//=========================================================
bool CSquadMonster::NoFriendlyFire()
{
	if ( !InSquad() )
	{
		return true;
	}

	CPlane	backPlane;
	CPlane  leftPlane;
	CPlane	rightPlane;

	Vector	vecLeftSide;
	Vector	vecRightSide;
	Vector	v_left;

	//!!!BUGBUG - to fix this, the planes must be aligned to where the monster will be firing its gun, not the direction it is facing!!!

	if ( m_hEnemy != NULL )
	{
		UTIL_MakeVectors ( UTIL_VecToAngles( m_hEnemy->Center() - GetAbsOrigin() ) );
	}
	else
	{
		// if there's no enemy, pretend there's a friendly in the way, so the grunt won't shoot.
		return false;
	}

	//UTIL_MakeVectors ( GetAbsAngles() );
	
	vecLeftSide = GetAbsOrigin() - ( gpGlobals->v_right * ( GetBounds().x * 1.5 ) );
	vecRightSide = GetAbsOrigin() + ( gpGlobals->v_right * ( GetBounds().x * 1.5 ) );
	v_left = gpGlobals->v_right * -1;

	leftPlane.InitializePlane ( gpGlobals->v_right, vecLeftSide );
	rightPlane.InitializePlane ( v_left, vecRightSide );
	backPlane.InitializePlane ( gpGlobals->v_forward, GetAbsOrigin() );

/*
	ALERT ( at_console, "LeftPlane: %f %f %f : %f\n", leftPlane.m_vecNormal.x, leftPlane.m_vecNormal.y, leftPlane.m_vecNormal.z, leftPlane.m_flDist );
	ALERT ( at_console, "RightPlane: %f %f %f : %f\n", rightPlane.m_vecNormal.x, rightPlane.m_vecNormal.y, rightPlane.m_vecNormal.z, rightPlane.m_flDist );
	ALERT ( at_console, "BackPlane: %f %f %f : %f\n", backPlane.m_vecNormal.x, backPlane.m_vecNormal.y, backPlane.m_vecNormal.z, backPlane.m_flDist );
*/

	CSquadMonster *pSquadLeader = MySquadLeader();
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
		if (pMember && pMember != this)
		{

			if ( backPlane.PointInFront  ( pMember->GetAbsOrigin() ) &&
				 leftPlane.PointInFront  ( pMember->GetAbsOrigin() ) && 
				 rightPlane.PointInFront ( pMember->GetAbsOrigin()) )
			{
				// this guy is in the check volume! Don't shoot!
				return false;
			}
		}
	}

	return true;
}

//=========================================================
// GetIdealState - surveys the Conditions information available
// and finds the best new state for a monster.
//=========================================================
MONSTERSTATE CSquadMonster :: GetIdealState ( void )
{
	//int	iConditions = IScheduleFlags();
	
	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch ( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
	case MONSTERSTATE_ALERT:
		if ( HasConditions ( bits_COND_NEW_ENEMY ) && InSquad() )
		{
			SquadMakeEnemy ( m_hEnemy );
		}
		break;

	default: break;
	}

	return CBaseMonster :: GetIdealState();
}

//=========================================================
// FValidateCover - determines whether or not the chosen
// cover location is a good one to move to. (currently based
// on proximity to others in the squad)
//=========================================================
bool CSquadMonster::FValidateCover( const Vector &vecCoverLocation )
{
	if ( !InSquad() )
	{
		return true;
	}

	if (SquadMemberInRange( vecCoverLocation, 128 ))
	{
		// another squad member is too close to this piece of cover.
		return false;
	}

	return true;
}

//=========================================================
// SquadEnemySplit- returns true if not all squad members
// are fighting the same enemy. 
//=========================================================
bool CSquadMonster::SquadEnemySplit()
{
	if (!InSquad())
		return false;

	CSquadMonster	*pSquadLeader = MySquadLeader();
	CBaseEntity		*pEnemy	= pSquadLeader->m_hEnemy;

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
		if (pMember != NULL && pMember->m_hEnemy != NULL && pMember->m_hEnemy != pEnemy)
		{
			return true;
		}
	}
	return false;
}

//=========================================================
// FValidateCover - determines whether or not the chosen
// cover location is a good one to move to. (currently based
// on proximity to others in the squad)
//=========================================================
bool CSquadMonster::SquadMemberInRange( const Vector &vecLocation, float flDist )
{
	if (!InSquad())
		return false;

	CSquadMonster *pSquadLeader = MySquadLeader();

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pSquadMember = pSquadLeader->MySquadMember(i);
		if (pSquadMember && (vecLocation - pSquadMember->GetAbsOrigin() ).Length2D() <= flDist)
			return true;
	}
	return false;
}


extern Schedule_t	slChaseEnemyFailed[];

Schedule_t *CSquadMonster::GetScheduleOfType( int iType )
{
	switch ( iType )
	{

	case SCHED_CHASE_ENEMY_FAILED:
		{
			return &slChaseEnemyFailed[ 0 ];
		}
	
	default:
		return CBaseMonster::GetScheduleOfType( iType );
	}
}

