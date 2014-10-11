/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "physics/Physics.h"

#include <stddef.h>

#include "graphics/GraphicsTypes.h"
#include "graphics/data/Mesh.h"
#include "math/Vector.h"

#include "core/GameTime.h"

#include "game/NPC.h"
#include "game/EntityManager.h"
#include "game/magic/spells/SpellsLvl06.h"

#include "scene/Interactive.h"

#include "physics/Box.h"
#include "physics/Collisions.h"

extern Material CUR_COLLISION_MATERIAL;

static const float VELOCITY_THRESHOLD = 400.f;

void ARX_ApplySpring(PHYSVERT * phys, long k, long l, float PHYSICS_constant,
					 float PHYSICS_Damp) {

	Vec3f deltaP, deltaV, springforce;
	PHYSVERT * pv_k = &phys[k];
	PHYSVERT * pv_l = &phys[l];
	float Dterm, Hterm;

	float restlength = glm::distance(pv_k->initpos, pv_l->initpos);
	// Computes Spring Magnitude
	deltaP = pv_k->pos - pv_l->pos;
	float dist = glm::length(deltaP); // Magnitude of delta
	dist = std::max(dist, 0.000001f); //TODO workaround for division by zero
	float divdist = 1.f / dist;
	Hterm = (dist - restlength) * PHYSICS_constant;

	deltaV = pv_k->velocity - pv_l->velocity; // Delta Velocity Vector
	Dterm = glm::dot(deltaV, deltaP) * PHYSICS_Damp * divdist; // Damping Term
	Dterm = (-(Hterm + Dterm));
	divdist *= Dterm;
	springforce = deltaP * divdist; // Normalize Distance Vector & Calc Force

	pv_k->force += springforce; // + force on particle 1

	pv_l->force -= springforce; // - force on particle 2
}

void ComputeForces(PHYSVERT * phys, long nb) {

	const Vec3f PHYSICS_Gravity(0.f, 65.f, 0.f);
	const float PHYSICS_Damping = 0.5f;

	float lastmass = 1.f;
	float div = 1.f;

	for(long k = 0; k < nb; k++) {

		PHYSVERT * pv = &phys[k];

		// Reset Force
		pv->force = pv->inertia;

		// Apply Gravity
		if(pv->mass > 0.f) {

			// need to be precomputed...
			if(lastmass != pv->mass) {
				div = 1.f / pv->mass;
				lastmass = pv->mass;
			}

			pv->force += (PHYSICS_Gravity * div);
		}

		// Apply Damping
		pv->force += pv->velocity * -PHYSICS_Damping;
	}

	for(int k = 0; k < nb; k++) {
		// Now Resolves Spring System
		for(long l = 0; l < nb; l++) {
			if(l != k) {
				ARX_ApplySpring(phys, l, k, 15.f, 0.99f);
			}
		}
	}
}

bool ARX_INTERACTIVE_CheckFULLCollision(PHYSICS_BOX_DATA * obj, EntityHandle source);

//! Calculate new Positions and Velocities given a deltatime
//! \param DeltaTime that has passed since last iteration
void RK4Integrate(PHYSICS_BOX_DATA * pbox, float DeltaTime) {

	PHYSVERT * source, * target, * accum1, * accum2, * accum3, * accum4;
	float halfDeltaT, sixthDeltaT;
	halfDeltaT = DeltaTime * .5f; // some time values i will need
	sixthDeltaT = ( 1.0f / 6 );

	PHYSVERT m_TempSys[5][32];

	for(long jj = 0; jj < 4; jj++) {

		arx_assert(size_t(pbox->nb_physvert) <= ARRAY_SIZE(m_TempSys[jj + 1]));
		memcpy(m_TempSys[jj + 1], pbox->vert, sizeof(PHYSVERT) * pbox->nb_physvert);

		if(jj == 3) {
			halfDeltaT = DeltaTime;
		}

		for(long kk = 0; kk < pbox->nb_physvert; kk++) {

			source = &pbox->vert[kk];
			accum1 = &m_TempSys[jj + 1][kk];
			target = &m_TempSys[0][kk];

			accum1->force = source->force * (source->mass * halfDeltaT);
			accum1->velocity = source->velocity * halfDeltaT;

			// determine the new velocity for the particle over 1/2 time
			target->velocity = source->velocity + accum1->force;
			target->mass = source->mass;

			// set the new position
			target->pos = source->pos + accum1->velocity;
		}

		ComputeForces(m_TempSys[0], pbox->nb_physvert); // compute the new forces
	}

	for(long kk = 0; kk < pbox->nb_physvert; kk++) {

		source = &pbox->vert[kk]; // current state of particle
		target = &pbox->vert[kk];
		accum1 = &m_TempSys[1][kk];
		accum2 = &m_TempSys[2][kk];
		accum3 = &m_TempSys[3][kk];
		accum4 = &m_TempSys[4][kk];

		// determine the new velocity for the particle using rk4 formula
		Vec3f dv = accum1->force + ((accum2->force + accum3->force) * 2.f) + accum4->force;
		target->velocity = source->velocity + (dv * sixthDeltaT);
		// determine the new position for the particle using rk4 formula
		Vec3f dp = accum1->velocity + ((accum2->velocity + accum3->velocity) * 2.f)
				   + accum4->velocity;
		target->pos = source->pos + (dp * sixthDeltaT * 1.2f);
	}

}

static bool IsObjectInField(PHYSICS_BOX_DATA * pbox) {

	for(size_t i = 0; i < MAX_SPELLS; i++) {
		const SpellBase * spell = spells[SpellHandle(i)];

		if(spell && spell->m_type == SPELL_CREATE_FIELD) {
			const CreateFieldSpell * sp = static_cast<const CreateFieldSpell *>(spell);
			
			if(ValidIONum(sp->m_entity)) {
				Entity * pfrm = entities[sp->m_entity];
				
				Cylinder cyl;
				cyl.height = -35.f;
				cyl.radius = 35.f;

				for(long k = 0; k < pbox->nb_physvert; k++) {
					PHYSVERT * pv = &pbox->vert[k];
					cyl.origin = pv->pos + Vec3f(0.f, 17.5f, 0.f);
					if(CylinderPlatformCollide(&cyl, pfrm) != 0.f) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

static bool IsObjectVertexCollidingPoly(PHYSICS_BOX_DATA * pbox, const EERIEPOLY & ep) {

	Vec3f pol[3];
	pol[0] = ep.v[0].p;
	pol[1] = ep.v[1].p;
	pol[2] = ep.v[2].p;

	if(ep.type & POLY_QUAD) {

		if(IsObjectVertexCollidingTriangle(pbox, pol)) {
			return true;
		}

		pol[1] = ep.v[2].p;
		pol[2] = ep.v[3].p;

		if(IsObjectVertexCollidingTriangle(pbox, pol)) {
			return true;
		}

		return false;
	}

	if(IsObjectVertexCollidingTriangle(pbox, pol)) {
		return true;
	}

	return false;
}

void polyTypeToCollisionMaterial(const EERIEPOLY & ep) {
	if (ep.type & POLY_METAL) CUR_COLLISION_MATERIAL = MATERIAL_METAL;
	else if (ep.type & POLY_WOOD) CUR_COLLISION_MATERIAL = MATERIAL_WOOD;
	else if (ep.type & POLY_STONE) CUR_COLLISION_MATERIAL = MATERIAL_STONE;
	else if (ep.type & POLY_GRAVEL) CUR_COLLISION_MATERIAL = MATERIAL_GRAVEL;
	else if (ep.type & POLY_WATER) CUR_COLLISION_MATERIAL = MATERIAL_WATER;
	else if (ep.type & POLY_EARTH) CUR_COLLISION_MATERIAL = MATERIAL_EARTH;
	else CUR_COLLISION_MATERIAL = MATERIAL_STONE;
}

static bool IsFULLObjectVertexInValidPosition(PHYSICS_BOX_DATA * pbox, EERIEPOLY *& collisionPoly) {

	bool ret = true;
	float rad = pbox->radius;
	
	// TODO copy-paste background tiles
	short tilex = pbox->vert[0].pos.x * ACTIVEBKG->Xmul;
	short tilez = pbox->vert[0].pos.z * ACTIVEBKG->Zmul;
	short radius = std::min(1, short(rad * (1.0f/100)) + 1);
	
	short minx = std::max(tilex - radius, 0);
	short maxx = std::min(tilex + radius, ACTIVEBKG->Xsize - 1);
	short minz = std::max(tilez - radius, 0);
	short maxz = std::min(tilez + radius, ACTIVEBKG->Zsize - 1);
	
	for(short z = minz; z <= maxz; z++)
	for(short x = minx; x <= maxx; x++) {
		EERIE_BKG_INFO & eg = ACTIVEBKG->fastdata[x][z];
		
		for(long k = 0; k < eg.nbpoly; k++) {
			EERIEPOLY & ep = eg.polydata[k];
			
			if(ep.area > 190.f
			   && !(ep.type & POLY_WATER)
			   && !(ep.type & POLY_TRANS)
			   && !(ep.type & POLY_NOCOL)
			) {
				if(fartherThan(ep.center, pbox->vert[0].pos, rad + 75.f))
					continue;
				
				for(long kk = 0; kk < pbox->nb_physvert; kk++) {
					float radd = 4.f;

					if(!fartherThan(pbox->vert[kk].pos, ep.center, radd)
					   || !fartherThan(pbox->vert[kk].pos, ep.v[0].p, radd)
					   || !fartherThan(pbox->vert[kk].pos, ep.v[1].p, radd)
					   || !fartherThan(pbox->vert[kk].pos, ep.v[2].p, radd)
					   || !fartherThan(pbox->vert[kk].pos, (ep.v[0].p + ep.v[1].p) * .5f, radd)
					   || !fartherThan(pbox->vert[kk].pos, (ep.v[2].p + ep.v[1].p) * .5f, radd)
					   || !fartherThan(pbox->vert[kk].pos, (ep.v[0].p + ep.v[2].p) * .5f, radd)
					) {
						collisionPoly = &ep;
						
						polyTypeToCollisionMaterial(ep);
						
						return false;
					}
					
					// Last addon
					for(long kl = 1; kl < pbox->nb_physvert; kl++) {
						if(kl != kk) {
							Vec3f pos = (pbox->vert[kk].pos + pbox->vert[kl].pos) * .5f;
							
							if(!fartherThan(pos, ep.center, radd)
							   || !fartherThan(pos, ep.v[0].p, radd)
							   || !fartherThan(pos, ep.v[1].p, radd)
							   || !fartherThan(pos, ep.v[2].p, radd)
							   || !fartherThan(pos, (ep.v[0].p + ep.v[1].p) * .5f, radd)
							   || !fartherThan(pos, (ep.v[2].p + ep.v[1].p) * .5f, radd)
							   || !fartherThan(pos, (ep.v[0].p + ep.v[2].p) * .5f, radd)
							) {
								collisionPoly = &ep;

								polyTypeToCollisionMaterial(ep);
								
								return false;
							}
						}
					}
				}
				
				if(IsObjectVertexCollidingPoly(pbox, ep)) {
					
					collisionPoly = &ep;
					
					polyTypeToCollisionMaterial(ep);
					
					return false;
				}
			}
		}
	}

	return ret;
}

static bool ARX_EERIE_PHYSICS_BOX_Compute(PHYSICS_BOX_DATA * pbox, float framediff, EntityHandle source) {

	Vec3f oldpos[32];
	long COUNT = 0;
	COUNT++;

	for(long kk = 0; kk < pbox->nb_physvert; kk++) {
		PHYSVERT *pv = &pbox->vert[kk];
		oldpos[kk] = pv->pos;
		pv->inertia = Vec3f_ZERO;

		pv->velocity.x = glm::clamp(pv->velocity.x, -VELOCITY_THRESHOLD, VELOCITY_THRESHOLD);
		pv->velocity.y = glm::clamp(pv->velocity.y, -VELOCITY_THRESHOLD, VELOCITY_THRESHOLD);
		pv->velocity.z = glm::clamp(pv->velocity.z, -VELOCITY_THRESHOLD, VELOCITY_THRESHOLD);
	}

	CUR_COLLISION_MATERIAL = MATERIAL_STONE;

	RK4Integrate(pbox, framediff);

	Sphere sphere;
	PHYSVERT *pv = &pbox->vert[0];
	sphere.origin = pv->pos;
	sphere.radius = pbox->radius;
	long colidd = 0;
	EERIEPOLY * collisionPoly = NULL;
	
	if(   !IsFULLObjectVertexInValidPosition(pbox, collisionPoly)
	   || ARX_INTERACTIVE_CheckFULLCollision(pbox, source)
	   || colidd
	   || IsObjectInField(pbox)
	) {
		colidd = 1;
		float power = (glm::abs(pbox->vert[0].velocity.x)
					   + glm::abs(pbox->vert[0].velocity.y)
					   + glm::abs(pbox->vert[0].velocity.z)) * .01f;


		if(!(ValidIONum(source) && (entities[source]->ioflags & IO_BODY_CHUNK)))
			ARX_TEMPORARY_TrySound(0.4f + power);

		if(!collisionPoly) {
			for(long k = 0; k < pbox->nb_physvert; k++) {
				pv = &pbox->vert[k];

				pv->velocity.x *= -0.3f;
				pv->velocity.z *= -0.3f;
				pv->velocity.y *= -0.4f;

				pv->pos = oldpos[k];
			}
		} else {
			for(long k = 0; k < pbox->nb_physvert; k++) {
				pv = &pbox->vert[k];

				float t = glm::dot(collisionPoly->norm, pv->velocity);
				pv->velocity -= collisionPoly->norm * (2.f * t);

				pv->velocity.x *= 0.3f;
				pv->velocity.z *= 0.3f;
				pv->velocity.y *= 0.4f;

				pv->pos = oldpos[k];
			}
		}
	}

	if(colidd) {
		pbox->stopcount += 1;
	} else {
		pbox->stopcount -= 2;

		if(pbox->stopcount < 0)
			pbox->stopcount = 0;
	}

	return true;
}

long ARX_PHYSICS_BOX_ApplyModel(PHYSICS_BOX_DATA * pbox, float framediff, float rubber, EntityHandle source) {

	long ret = 0;

	if(!pbox)
		return ret;

	if(pbox->active == 2)
		return ret;

	if(framediff == 0.f)
		return ret;
	
	// Memorizes initpos
	for(long k = 0; k < pbox->nb_physvert; k++) {
		PHYSVERT & pv = pbox->vert[k];
		pv.temp = pv.pos;
	}

	float timing = pbox->storedtiming + framediff * rubber * 0.0055f;
	float t_threshold = 0.18f;

	if(timing < t_threshold) {
		pbox->storedtiming = timing;
		return 1;
	} else {
		while(timing >= t_threshold) {
			ComputeForces(pbox->vert, pbox->nb_physvert);

			if(!ARX_EERIE_PHYSICS_BOX_Compute(pbox, std::min(0.11f, timing * 10), source))
				ret = 1;

			timing -= t_threshold;
		}

		pbox->storedtiming = timing;
	}

	if(pbox->stopcount < 16)
		return ret;

	pbox->active = 2;
	pbox->stopcount = 0;

	if(ValidIONum(source)) {
		entities[source]->soundcount = 0;
		entities[source]->soundtime = (unsigned long)(arxtime) + 2000;
	}

	return ret;
}

