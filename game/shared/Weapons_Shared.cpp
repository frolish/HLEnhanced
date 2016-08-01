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

#include "entities/CEntityDictionary.h"

#ifdef CLIENT_DLL
#include "hl/hl_weapons.h"
#endif

#include "Weapons.h"

ItemInfo CBasePlayerItem::ItemInfoArray[ MAX_WEAPONS ];

// Precaches the weapon and queues the weapon info for sending to clients
void UTIL_PrecacheOtherWeapon( const char* const pszClassname )
{
#ifdef SERVER_DLL
	CBaseEntity* pEntity = UTIL_CreateNamedEntity( pszClassname );

	if( !pEntity )
	{
		ALERT( at_console, "NULL Ent \"%s\" in UTIL_PrecacheOtherWeapon\n", pszClassname );
		return;
	}

	ItemInfo II;
	pEntity->Precache();
	memset( &II, 0, sizeof II );
	if( ( ( CBasePlayerItem* ) pEntity )->GetItemInfo( &II ) )
	{
		CBasePlayerItem::ItemInfoArray[ II.iId ] = II;
	}

	UTIL_RemoveNow( pEntity );
#else
	auto pReg = g_EntityDict.FindEntityClassByEntityName( pszClassname );

	if( !pReg )
	{
		ALERT( at_console, "NULL Ent \"%s\" in UTIL_PrecacheOtherWeapon\n", pszClassname );
		return;
	}

	auto pev = HUD_AllocEntity();

	CBaseEntity* pEntity = pReg->CreateInstance( pev );

	if( !pEntity )
	{
		//We're in deep shit if this happens because we can't free the entvars instance. - Solokiller
		ALERT( at_console, "NULL Ent \"%s\" in UTIL_PrecacheOtherWeapon\n", pszClassname );
		return;
	}

	CBasePlayerWeapon* pWeapon = dynamic_cast<CBasePlayerWeapon*>( pEntity );

	if( !pWeapon )
	{
		ASSERTSZ( false, "Check your class inheritance!" );
		ALERT( at_console, "UTIL_PrecacheOtherWeapon: \"%s\" is not a weapon!\n", pszClassname );
		return;
	}

	ItemInfo II;
	pEntity->Precache();
	memset( &II, 0, sizeof II );
	if( pWeapon->GetItemInfo( &II ) )
	{
		CBasePlayerItem::ItemInfoArray[ II.iId ] = II;
	}

	HUD_AddWeapon( pWeapon );

#endif
}

void PrecacheWeapons()
{
	memset( CBasePlayerItem::ItemInfoArray, 0, sizeof( CBasePlayerItem::ItemInfoArray ) );

	//TODO: weapons and ammo could be added to a list that is enumerated. Would make things like cheats and map config parsers easier to write - Solokiller

	// shotgun
	UTIL_PrecacheOtherWeapon( "weapon_shotgun" );

	// crowbar
	UTIL_PrecacheOtherWeapon( "weapon_crowbar" );

	// glock
	UTIL_PrecacheOtherWeapon( "weapon_9mmhandgun" );

	// mp5
	UTIL_PrecacheOtherWeapon( "weapon_9mmAR" );

	// python
	UTIL_PrecacheOtherWeapon( "weapon_357" );

	// gauss
	UTIL_PrecacheOtherWeapon( "weapon_gauss" );

	// rpg
	UTIL_PrecacheOtherWeapon( "weapon_rpg" );

	// crossbow
	UTIL_PrecacheOtherWeapon( "weapon_crossbow" );

	// egon
	UTIL_PrecacheOtherWeapon( "weapon_egon" );

	// tripmine
	UTIL_PrecacheOtherWeapon( "weapon_tripmine" );

	// satchel charge
	UTIL_PrecacheOtherWeapon( "weapon_satchel" );

	// hand grenade
	UTIL_PrecacheOtherWeapon( "weapon_handgrenade" );

	// squeak grenade
	UTIL_PrecacheOtherWeapon( "weapon_snark" );

	// hornetgun
	UTIL_PrecacheOtherWeapon( "weapon_hornetgun" );

	//Sniper rifle
	UTIL_PrecacheOtherWeapon( "weapon_sniperrifle" );
}