/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "Effects.h"

#include "CXenPLight.h"

BEGIN_DATADESC( CXenPLight )
	DEFINE_FIELD( m_pGlow, FIELD_CLASSPTR ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( xen_plantlight, CXenPLight );

void CXenPLight::Spawn( void )
{
	Precache();

	SET_MODEL( ENT( pev ), "models/light.mdl" );
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_TRIGGER;

	UTIL_SetSize( this, Vector( -80, -80, 0 ), Vector( 80, 80, 32 ) );
	SetActivity( ACT_IDLE );
	pev->nextthink = gpGlobals->time + 0.1;
	pev->frame = RANDOM_FLOAT( 0, 255 );

	m_pGlow = CSprite::SpriteCreate( XEN_PLANT_GLOW_SPRITE, pev->origin + Vector( 0, 0, ( pev->mins.z + pev->maxs.z )*0.5 ), false );
	m_pGlow->SetTransparency( kRenderGlow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx );
	m_pGlow->SetAttachment( edict(), 1 );
}

void CXenPLight::Precache( void )
{
	PRECACHE_MODEL( "models/light.mdl" );
	PRECACHE_MODEL( XEN_PLANT_GLOW_SPRITE );
}

void CXenPLight::Touch( CBaseEntity *pOther )
{
	if( pOther->IsPlayer() )
	{
		pev->dmgtime = gpGlobals->time + XEN_PLANT_HIDE_TIME;
		if( GetActivity() == ACT_IDLE || GetActivity() == ACT_STAND )
		{
			SetActivity( ACT_CROUCH );
		}
	}
}

void CXenPLight::Think( void )
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	switch( GetActivity() )
	{
	case ACT_CROUCH:
		if( m_fSequenceFinished )
		{
			SetActivity( ACT_CROUCHIDLE );
			LightOff();
		}
		break;

	case ACT_CROUCHIDLE:
		if( gpGlobals->time > pev->dmgtime )
		{
			SetActivity( ACT_STAND );
			LightOn();
		}
		break;

	case ACT_STAND:
		if( m_fSequenceFinished )
			SetActivity( ACT_IDLE );
		break;

	case ACT_IDLE:
	default:
		break;
	}
}

void CXenPLight::LightOn( void )
{
	SUB_UseTargets( this, USE_ON, 0 );
	if( m_pGlow )
		m_pGlow->pev->effects &= ~EF_NODRAW;
}

void CXenPLight::LightOff( void )
{
	SUB_UseTargets( this, USE_OFF, 0 );
	if( m_pGlow )
		m_pGlow->pev->effects |= EF_NODRAW;
}