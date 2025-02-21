#include "../qcommon/qcommon.h"
#include "g_shared.h"

#ifdef TESTING_LIBRARY
#define g_entities ((gentity_t*)( 0x08665480 ))
#else
extern gentity_t g_entities[];
#endif

#ifdef TESTING_LIBRARY
#define level (*((level_locals_t*)( 0x0859B400 )))
#else
extern level_locals_t level;
#endif

char riflePriorityMap[] =
{
	1,
	9,
	9,
	9,
	8,
	7,
	6,
	6,
	6,
	6,
	5,
	5,
	4,
	4,
	4,
	4,
	3,
	3,
	0,
};

char bulletPriorityMap[] =
{
	1,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	0,
};

int Bullet_CalcDamageRange(const weaponParms *wp, float dist)
{
	float damage;
	float minDamage;
	float maxDamage;
	float range;

	if ( wp->weapDef->maxDamageRange > dist )
		return wp->weapDef->damage;

	if ( wp->weapDef->minDamageRange <= dist )
		return wp->weapDef->minDamage;

	range = wp->weapDef->minDamageRange - wp->weapDef->maxDamageRange;

	if ( range == 0.0 )
		return wp->weapDef->damage;

	maxDamage = (dist - wp->weapDef->maxDamageRange) / range;
	minDamage = (float)wp->weapDef->minDamage;
	damage = (float)wp->weapDef->damage;

	return (int)lerp(damage, minDamage, maxDamage);
}

void Bullet_Fire_Extended(const gentity_s *inflictor, gentity_s *attacker, float *start, float *end, float dmgScale, int callCount, const weaponParms *wp, const gentity_s *weaponEnt, int gameTime)
{
	float scale;
	float scaleSquared;
	float dirScale;
	int surfaceType;
	int event;
	float lengthSquared;
	vec3_t dir;
	vec3_t origin;
	int meansOfDeath;
	int dflags;
	int damage;
	float dist;
	vec3_t temp;
	gentity_s *self;
	gentity_s *tempEnt;
	trace_t trace;

	dflags = 0;

	if ( callCount <= 12 )
	{
		if ( wp->weapDef->rifleBullet )
		{
			meansOfDeath = MOD_RIFLE_BULLET;
			dflags = 32;
		}
		else
		{
			meansOfDeath = MOD_PISTOL_BULLET;
		}

		if ( wp->weapDef->armorPiercing )
			dflags |= 2u;

		if ( wp->weapDef->rifleBullet )
			G_LocationalTrace(&trace, start, end, inflictor->s.number, 41953329, riflePriorityMap);
		else
			G_LocationalTrace(&trace, start, end, inflictor->s.number, 41953329, bulletPriorityMap);

		Vec3Lerp(start, end, trace.fraction, origin);
		G_CheckHitTriggerDamage(attacker, start, origin, wp->weapDef->damage, meansOfDeath);
		self = &g_entities[trace.hitId];
		VectorSubtract(end, start, dir);
		Vec3Normalize(dir);
		lengthSquared = VectorsLengthSquared(dir, trace.normal) * -2.0;
		VectorMA(dir, lengthSquared, trace.normal, dir);

		if ( (trace.surfaceFlags & 4) == 0 && !self->client && trace.fraction < 1.0 )
		{
			if ( wp->weapDef->weaponClass == WEAPCLASS_SPREAD )
			{
				event = EV_BULLET_HIT_SMALL;
			}
			else if ( wp->weapDef->rifleBullet )
			{
				event = EV_SHOTGUN_HIT;
			}
			else
			{
				event = EV_BULLET_HIT_LARGE;
			}

			if ( wp->weapDef->rifleBullet )
				event = EV_SHOTGUN_HIT;
			else
				event = EV_BULLET_HIT_LARGE;

			tempEnt = G_TempEntity(origin, event);
			tempEnt->s.eventParm = DirToByte(trace.normal);
			tempEnt->s.scale = DirToByte(dir);

			if ( self->s.eType == ET_PLAYER_CORPSE )
				surfaceType = 7;
			else
				surfaceType = (trace.surfaceFlags & 0x1F00000) >> 20;

			tempEnt->s.surfType = surfaceType;
			tempEnt->s.otherEntityNum = weaponEnt->s.number;
		}

		if ( (trace.contents & 0x10) != 0 )
		{
			VectorSubtract(end, start, dir);
			Vec3Normalize(dir);
			scaleSquared = VectorsLengthSquared(trace.normal, dir);

			if ( -scaleSquared < 0.125 )
				dirScale = 0.0;
			else
				dirScale = 0.25 / -scaleSquared;

			VectorMA(origin, dirScale, dir, start);
			Bullet_Fire_Extended(inflictor, attacker, start, end, dmgScale, callCount + 1, wp, weaponEnt, gameTime);
		}
		else if ( self->takedamage )
		{
			if ( self != attacker )
			{
				VectorSubtract(start, origin, temp);
				dist = VectorLength(temp);
				damage = (int)((float)Bullet_CalcDamageRange(wp, dist) * dmgScale);

				G_Damage(self, attacker, attacker, wp->forward, origin, damage, dflags, meansOfDeath, trace.partName, level.time - gameTime);

				if ( self->client )
				{
					if ( (dflags & 0x20) != 0 && (Dvar_GetInt("scr_friendlyfire") || !OnSameTeam(self, attacker)) )
					{
						scale = dmgScale * 0.5;
						Bullet_Fire_Extended(self, attacker, origin, end, scale, callCount + 1, wp, weaponEnt, gameTime);
					}
				}
			}
		}
	}
	else
	{
		Com_DPrintf("Bullet_Fire_Extended: Too many resursions, bullet aborted\n");
	}
}

void gunrandom(float *x, float *y)
{
	long double v2;
	float sinT;
	float theta;
	float r;
	float cosT;

	theta = G_random() * 360.0;
	r = G_random();
	v2 = (float)(theta * 0.017453292);
	cosT = cos(v2);
	sinT = sin(v2);
	*x = r * cosT;
	*y = r * sinT;
}

void Bullet_RandomSpread(float spread, float *end, const weaponParms *wp, float maxRange)
{
	float r;
	float aimOffset;
	float up;
	float right;

	r = tan(spread * 0.017453292);
	aimOffset = r * maxRange;

	gunrandom(&right, &up);

	right = right * aimOffset;
	up = up * aimOffset;

	VectorMA(wp->muzzleTrace, maxRange, wp->forward, end);
	VectorMA(end, right, wp->right, end);
	VectorMA(end, up, wp->up, end);
}

void Bullet_Fire_Spread(const gentity_s *weaponEnt, gentity_s *attacker, const weaponParms *wp, int gameTime, float spread)
{
	int i;
	vec3_t start;
	vec3_t end;

	VectorCopy(wp->muzzleTrace, start);

	for ( i = 0; i < wp->weapDef->shotCount; ++i )
	{
		Bullet_RandomSpread(spread, end, wp, wp->weapDef->minDamageRange);
		Bullet_Fire_Extended(weaponEnt, attacker, start, end, 1.0, 0, wp, weaponEnt, gameTime);
	}
}

void Bullet_Fire(gentity_s *attacker, float spread, weaponParms *wp, const gentity_s *ent, int gameTime)
{
	AntilagClientStore store;
	vec3_t endpos;

	G_AntiLagRewindClientPos(gameTime, &store);

	if ( wp->weapDef->weaponClass == WEAPCLASS_SPREAD )
	{
		Bullet_Fire_Spread(ent, attacker, wp, gameTime, spread);
	}
	else
	{
		Bullet_RandomSpread(spread, endpos, wp, 8192.0);
		Bullet_Fire_Extended(ent, attacker, wp->muzzleTrace, endpos, 1.0, 0, wp, ent, gameTime);
	}

	G_AntiLag_RestoreClientPos(&store);
}