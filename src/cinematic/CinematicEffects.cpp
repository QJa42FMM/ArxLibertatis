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

#include "cinematic/CinematicEffects.h"

#include <cstring>

#include "cinematic/Cinematic.h"
#include "cinematic/CinematicKeyframer.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "cinematic/CinematicTexture.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/TextureStage.h"

/*---------------------------------------------------------------------------------*/
#define		NBOLDPOS	10
/*---------------------------------------------------------------------------------*/
float		FlashAlpha;
int			TotOldPos;
static Vec3f OldPos[NBOLDPOS];
static float OldAz[NBOLDPOS];

/*---------------------------------------------------------------------------------*/
Color FX_FadeIN(float a, Color color, Color colord)
{
	float	r, g, b;
	float	rd, gd, bd;
	
	r = color.r;
	g = color.g;
	b = color.b;
	
	rd = colord.r;
	gd = colord.g;
	bd = colord.b;

	r = (r - rd) * a + rd;
	g = (g - gd) * a + gd;
	b = (b - bd) * a + bd;
	
	return Color((int)r, (int)g, (int)b, 0);

}
/*---------------------------------------------------------------------------------*/
Color FX_FadeOUT(float a, Color color, Color colord)
{
	float	r, g, b;
	float	rd, gd, bd;

	a = 1.f - a;
	
	r = color.r;
	g = color.g;
	b = color.b;
	
	rd = colord.r;
	gd = colord.g;
	bd = colord.b;

	r = (r - rd) * a + rd;
	g = (g - gd) * a + gd;
	b = (b - bd) * a + bd;

	return Color((int)r, (int)g, (int)b, 0);
}

static float LastTime;

extern Rect g_size;

bool FX_Blur(Cinematic *c, CinematicBitmap *tb, EERIE_CAMERA &camera)
{
	if(c->numbitmap < 0 || !tb)
		return false;

	if(TotOldPos == NBOLDPOS) {
		TotOldPos--;
		std::copy(OldPos + 1, OldPos + 1 + TotOldPos, OldPos);
		memmove(OldAz, OldAz + 1, TotOldPos * 4);
	}

	if((GetTimeKeyFramer() - LastTime) < 0.40f) {
		LastTime = GetTimeKeyFramer();
		OldPos[TotOldPos] = c->pos;
		OldAz[TotOldPos] = c->angz;
		TotOldPos++;
	}

	float alpha = 32.f;
	float dalpha = (127.f / NBOLDPOS);
	Vec3f *pos = OldPos;
	float *az = OldAz;
	int nb = TotOldPos;

	while(nb) {
		camera.orgTrans.pos = *pos;
		camera.setTargetCamera(camera.orgTrans.pos.x, camera.orgTrans.pos.y, 0.f);
		camera.angle.setPitch(0);
		camera.angle.setRoll(*az);
		PrepareCamera(&camera, g_size);
		
		Color col = Color(255, 255, 255, int(alpha));
		DrawGrille(&tb->grid, col, 0, NULL, &c->posgrille, c->angzgrille);

		alpha += dalpha;
		pos++;
		az++;
		nb--;
	}

	return true;
}

//POST FX
bool FX_FlashBlanc(Vec2f size, float speed, Color color, float fps, float currfps) {
	
	if(FlashAlpha < 0.f)
		return false;
	
	if(FlashAlpha == 0.f)
		FlashAlpha = 1.f;
	
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::ArgDiffuse);
	GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::ArgDiffuse);
	GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendOne);
	
	color.a = 255.f * FlashAlpha;
	ColorRGBA col = color.toRGBA();
	
	TexturedVertex v[4];
	v[0].p = Vec3f(0.f, 0.f, 0.01f);
	v[0].rhw = 1.f;
	v[0].color = col;
	v[1].p = Vec3f(size.x - 1.f, 0.f, 0.01f);
	v[1].rhw = 1.f;
	v[1].color = col;
	v[2].p = Vec3f(0.f, size.y - 1.f, 0.01f);
	v[2].rhw = 1.f;
	v[2].color = col;
	v[3].p = Vec3f(size.x - 1.f, size.y - 1.f, 0.01f);
	v[3].rhw = 1.f;
	v[3].color = col;
	
	FlashAlpha -= speed * fps / currfps;
	
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	
	return true;
}

/*---------------------------------------------------------------------------------*/
float	DreamAng, DreamAng2;
float	DreamTable[64*64*2];
/*---------------------------------------------------------------------------------*/
void FX_DreamPrecalc(CinematicBitmap * bi, float amp, float fps) {
	
	float a = DreamAng;
	float a2 = DreamAng2;
	
	float s1 = bi->m_count.x * std::cos(glm::radians(0.f));
	float s2 = bi->m_count.y * std::cos(glm::radians(0.f));
	int nx = (bi->m_count.x + 1) << 1;
	int ny = (bi->m_count.y + 1) << 1;
	float nnx = ((float)nx) + s1;
	float nny = ((float)ny) + s2;
	
	float ox, oy;
	
	ox = amp * ((2 * (std::sin(nnx / 20) + std::sin(nnx * nny / 2000)
	                  + std::sin((nnx + nny) / 100) + std::sin((nny - nnx) / 70) + std::sin((nnx + 4 * nny) / 70)
	                  + 2 * std::sin(hypot(256 - nnx, (150 - nny / 8)) / 40))));
	oy = amp * (((std::cos(nnx / 31) + std::cos(nnx * nny / 1783) +
	              + 2 * std::cos((nnx + nny) / 137) + std::cos((nny - nnx) / 55) + 2 * std::cos((nnx + 8 * nny) / 57)
	              + std::cos(hypot(384 - nnx, (274 - nny / 9)) / 51))));
	
	float * t = DreamTable;
	ny = ((bi->m_count.y * bi->grid.m_scale) + 1);
	
	while(ny) {
		nx = ((bi->m_count.x * bi->grid.m_scale) + 1);
		while(nx) {
			s1 = bi->m_count.x * std::cos(glm::radians(a));
			s2 = bi->m_count.y * std::cos(glm::radians(a2));
			a -= 15.f;
			a2 += 8.f;
			
			nnx = ((float)nx) + s1;
			nny = ((float)ny) + s2;
			
			*t++ = (float)(-ox + amp * ((2 * (std::sin(nnx / 20) + std::sin(nnx * nny / 2000)
			                                  + std::sin((nnx + nny) / 100) + std::sin((nny - nnx) / 70) + std::sin((nnx + 4 * nny) / 70)
			                                  + 2 * std::sin(hypot(256 - nnx, (150 - nny / 8)) / 40)))));
			*t++ = (float)(-oy + amp * (((std::cos(nnx / 31) + std::cos(nnx * nny / 1783) +
			                              + 2 * std::cos((nnx + nny) / 137) + std::cos((nny - nnx) / 55) + 2 * std::cos((nnx + 8 * nny) / 57)
			                              + std::cos(hypot(384 - nnx, (274 - nny / 9)) / 51)))));
			
			nx--;
		}
		ny--;
	}
	
	DreamAng += 4.f * fps;
	DreamAng2 -= 2.f * fps;
}
