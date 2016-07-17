#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "CItemSoda.h"

LINK_ENTITY_TO_CLASS( item_sodacan, CItemSoda );

void CItemSoda::Spawn( void )
{
	Precache();
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_TOSS;

	SET_MODEL( ENT( pev ), "models/can.mdl" );
	UTIL_SetSize( this, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

	SetThink( &CItemSoda::CanThink );
	pev->nextthink = gpGlobals->time + 0.5;
}

void CItemSoda::Precache( void )
{
}

void CItemSoda::CanThink( void )
{
	EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/g_bounce3.wav", 1, ATTN_NORM );

	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize( this, Vector( -8, -8, 0 ), Vector( 8, 8, 8 ) );
	SetThink( NULL );
	SetTouch( &CItemSoda::CanTouch );
}

void CItemSoda::CanTouch( CBaseEntity *pOther )
{
	if( !pOther->IsPlayer() )
	{
		return;
	}

	// spoit sound here

	pOther->GiveHealth( 1, DMG_GENERIC );// a bit of health.

	if( !FNullEnt( pev->owner ) )
	{
		// tell the machine the can was taken
		pev->owner->v.frags = 0;
	}

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = EF_NODRAW;
	SetTouch( NULL );
	SetThink( &CItemSoda::SUB_Remove );
	pev->nextthink = gpGlobals->time;
}