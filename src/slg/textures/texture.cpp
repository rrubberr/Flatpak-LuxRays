/***************************************************************************
 * Copyright 1998-2013 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#include <algorithm>
#include <numeric>
#include <memory>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/dassert.h>

#include "slg/sdl/sdl.h"
#include "slg/sdl/bsdf.h"
#include "slg/textures/texture.h"
#include "slg/textures/blender_texture.h"

using namespace std;
using namespace luxrays;
using namespace slg;

//------------------------------------------------------------------------------
// Texture
//------------------------------------------------------------------------------

// The generic implementation
UV Texture::GetDuv(const HitPoint &hitPoint,
        const Vector &dpdu, const Vector &dpdv,
        const Normal &dndu, const Normal &dndv,
        const float sampleDistance) const {
    // Calculate bump map value at intersection point
    const float base = GetFloatValue(hitPoint);

    // Compute offset positions and evaluate displacement texture
    const Point origP = hitPoint.p;
    const Normal origShadeN = hitPoint.shadeN;
    const float origU = hitPoint.uv.u;

    UV duv;
    HitPoint hitPointTmp = hitPoint;

    // Shift hitPointTmp.du in the u direction and calculate value
    const float uu = sampleDistance / dpdu.Length();
    hitPointTmp.p += uu * dpdu;
    hitPointTmp.uv.u += uu;
    hitPointTmp.shadeN = Normalize(origShadeN + uu * dndu);
    duv.u = (GetFloatValue(hitPointTmp) - base) / uu;

    // Shift hitPointTmp.dv in the v direction and calculate value
    const float vv = sampleDistance / dpdv.Length();
    hitPointTmp.p = origP + vv * dpdv;
    hitPointTmp.uv.u = origU;
    hitPointTmp.uv.v += vv;
    hitPointTmp.shadeN = Normalize(origShadeN + vv * dndv);
    duv.v = (GetFloatValue(hitPointTmp) - base) / vv;

    return duv;
}

//------------------------------------------------------------------------------
// Texture utility functions
//------------------------------------------------------------------------------

// Perlin Noise Data
#define NOISE_PERM_SIZE 256
static const int NoisePerm[2 * NOISE_PERM_SIZE] = {
	151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96,
	53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
	// Rest of noise permutation table
	8, 99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
	151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

static float Grad(int x, int y, int z, float dx, float dy, float dz) {
	const int h = NoisePerm[NoisePerm[NoisePerm[x] + y] + z] & 15;
	const float u = h < 8 || h == 12 || h == 13 ? dx : dy;
	const float v = h < 4 || h == 12 || h == 13 ? dy : dz;
	return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

static float NoiseWeight(float t) {
	const float t3 = t * t * t;
	const float t4 = t3 * t;
	return 6.f * t4 * t - 15.f * t4 + 10.f * t3;
}

float slg::Noise(float x, float y, float z) {
	// Compute noise cell coordinates and offsets
	int ix = Floor2Int(x);
	int iy = Floor2Int(y);
	int iz = Floor2Int(z);
	const float dx = x - ix, dy = y - iy, dz = z - iz;
	// Compute gradient weights
	ix &= (NOISE_PERM_SIZE - 1);
	iy &= (NOISE_PERM_SIZE - 1);
	iz &= (NOISE_PERM_SIZE - 1);
	const float w000 = Grad(ix, iy, iz, dx, dy, dz);
	const float w100 = Grad(ix + 1, iy, iz, dx - 1, dy, dz);
	const float w010 = Grad(ix, iy + 1, iz, dx, dy - 1, dz);
	const float w110 = Grad(ix + 1, iy + 1, iz, dx - 1, dy - 1, dz);
	const float w001 = Grad(ix, iy, iz + 1, dx, dy, dz - 1);
	const float w101 = Grad(ix + 1, iy, iz + 1, dx - 1, dy, dz - 1);
	const float w011 = Grad(ix, iy + 1, iz + 1, dx, dy - 1, dz - 1);
	const float w111 = Grad(ix + 1, iy + 1, iz + 1, dx - 1, dy - 1, dz - 1);
	// Compute trilinear interpolation of weights
	const float wx = NoiseWeight(dx);
	const float wy = NoiseWeight(dy);
	const float wz = NoiseWeight(dz);
	const float x00 = Lerp(wx, w000, w100);
	const float x10 = Lerp(wx, w010, w110);
	const float x01 = Lerp(wx, w001, w101);
	const float x11 = Lerp(wx, w011, w111);
	const float y0 = Lerp(wy, x00, x10);
	const float y1 = Lerp(wy, x01, x11);
	return Lerp(wz, y0, y1);
}

static float FBm(const Point &P, const float omega, const int maxOctaves) {
	// Compute number of octaves for anti-aliased FBm
	const float foctaves = static_cast<float>(maxOctaves);
	const int octaves = Floor2Int(foctaves);
	// Compute sum of octaves of noise for FBm
	float sum = 0.f, lambda = 1.f, o = 1.f;
	for (int i = 0; i < octaves; ++i) {
		sum += o * Noise(lambda * P);
		lambda *= 1.99f;
		o *= omega;
	}
	const float partialOctave = foctaves - static_cast<float>(octaves);
	sum += o * SmoothStep(.3f, .7f, partialOctave) *
			Noise(lambda * P);
	return sum;
}

float slg::Turbulence(const Point &P, const float omega, const int maxOctaves) {
	// Compute number of octaves for anti-aliased FBm
	const float foctaves = static_cast<float>(maxOctaves);
	const int octaves = Floor2Int(foctaves);
	// Compute sum of octaves of noise for turbulence
	float sum = 0.f, lambda = 1.f, o = 1.f;
	for (int i = 0; i < octaves; ++i) {
		sum += o * fabsf(Noise(lambda * P));
		lambda *= 1.99f;
		o *= omega;
	}
	const float partialOctave = foctaves - static_cast<float>(octaves);
	sum += o * SmoothStep(.3f, .7f, partialOctave) *
	       fabsf(Noise(lambda * P));

	// finally, add in value to account for average value of fabsf(Noise())
	// (~0.2) for the remaining octaves...
	sum += (maxOctaves - foctaves) * 0.2f;

	return sum;
}

/* creates a sine wave */
float slg::tex_sin(float a) {
    a = 0.5f + 0.5f * sinf(a);

    return a;
}

/* creates a saw wave */
float slg::tex_saw(float a) {
    const float b = 2.0f * M_PI;

    int n = (int) (a / b);
    a -= n*b;
    if (a < 0) a += b;
    return a / b;
}

/* creates a triangle wave */
float slg::tex_tri(float a) {
    const float b = 2.0f * M_PI;
    const float rmax = 1.0f;

    a = rmax - 2.0f * fabs(floor((a * (1.0f / b)) + 0.5f) - (a * (1.0f / b)));

    return a;
}

//------------------------------------------------------------------------------
// Scale texture
//------------------------------------------------------------------------------

float ScaleTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return tex1->GetFloatValue(hitPoint) * tex2->GetFloatValue(hitPoint);
}

Spectrum ScaleTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return tex1->GetSpectrumValue(hitPoint) * tex2->GetSpectrumValue(hitPoint);
}

Properties ScaleTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("scale"));
	props.Set(Property("scene.textures." + name + ".texture1")(tex1->GetName()));
	props.Set(Property("scene.textures." + name + ".texture2")(tex2->GetName()));

	return props;
}

//------------------------------------------------------------------------------
// FresnelApproxN & FresnelApproxK texture
//------------------------------------------------------------------------------

float FresnelApproxN(const float Fr) {
	const float sqrtReflectance = sqrtf(Clamp(Fr, 0.f, .999f));

	return (1.f + sqrtReflectance) /
		(1.f - sqrtReflectance);
}

Spectrum FresnelApproxN(const Spectrum &Fr) {
	const Spectrum sqrtReflectance = Fr.Clamp(0.f, .999f).Sqrt();

	return (Spectrum(1.f) + sqrtReflectance) /
		(Spectrum(1.f) - sqrtReflectance);
}

float FresnelApproxK(const float Fr) {
	const float reflectance = Clamp(Fr, 0.f, .999f);

	return 2.f * sqrtf(reflectance /
		(1.f - reflectance));
}

Spectrum FresnelApproxK(const Spectrum &Fr) {
	const Spectrum reflectance = Fr.Clamp(0.f, .999f);

	return 2.f * Sqrt(reflectance /
		(Spectrum(1.f) - reflectance));
}

float FresnelApproxNTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return FresnelApproxN(tex->GetFloatValue(hitPoint));
}

Spectrum FresnelApproxNTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return FresnelApproxN(tex->GetSpectrumValue(hitPoint));
}

float FresnelApproxNTexture::Y() const {
	return FresnelApproxN(tex->Y());
}

float FresnelApproxNTexture::Filter() const {
	return FresnelApproxN(tex->Filter());
}

float FresnelApproxKTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return FresnelApproxK(tex->GetFloatValue(hitPoint));
}

Spectrum FresnelApproxKTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return FresnelApproxK(tex->GetSpectrumValue(hitPoint));
}

float FresnelApproxKTexture::Y() const {
	return FresnelApproxK(tex->Y());
}

float FresnelApproxKTexture::Filter() const {
	return FresnelApproxK(tex->Filter());
}

Properties FresnelApproxNTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("fresnelapproxn"));
	props.Set(Property("scene.textures." + name + ".texture")(tex->GetName()));

	return props;
}

Properties FresnelApproxKTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("fresnelapproxk"));
	props.Set(Property("scene.textures." + name + ".texture")(tex->GetName()));

	return props;
}

//------------------------------------------------------------------------------
// CheckerBoard 2D & 3D texture
//------------------------------------------------------------------------------

float CheckerBoard2DTexture::GetFloatValue(const HitPoint &hitPoint) const {
	const UV uv = mapping->Map(hitPoint);
	if ((Floor2Int(uv.u) + Floor2Int(uv.v)) % 2 == 0)
		return tex1->GetFloatValue(hitPoint);
	else
		return tex2->GetFloatValue(hitPoint);
}

Spectrum CheckerBoard2DTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	const UV uv = mapping->Map(hitPoint);
	if ((Floor2Int(uv.u) + Floor2Int(uv.v)) % 2 == 0)
		return tex1->GetSpectrumValue(hitPoint);
	else
		return tex2->GetSpectrumValue(hitPoint);
}

Properties CheckerBoard2DTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("checkerboard2d"));
	props.Set(Property("scene.textures." + name + ".texture1")(tex1->GetName()));
	props.Set(Property("scene.textures." + name + ".texture2")(tex2->GetName()));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

float CheckerBoard3DTexture::GetFloatValue(const HitPoint &hitPoint) const {
	const Point p = mapping->Map(hitPoint);
	if ((Floor2Int(p.x) + Floor2Int(p.y) + Floor2Int(p.z)) % 2 == 0)
		return tex1->GetFloatValue(hitPoint);
	else
		return tex2->GetFloatValue(hitPoint);
}

Spectrum CheckerBoard3DTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	const Point p = mapping->Map(hitPoint);
	if ((Floor2Int(p.x) + Floor2Int(p.y) + Floor2Int(p.z)) % 2 == 0)
		return tex1->GetSpectrumValue(hitPoint);
	else
		return tex2->GetSpectrumValue(hitPoint);
}

Properties CheckerBoard3DTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("checkerboard3d"));
	props.Set(Property("scene.textures." + name + ".texture1")(tex1->GetName()));
	props.Set(Property("scene.textures." + name + ".texture2")(tex2->GetName()));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

//------------------------------------------------------------------------------
// Mix texture
//------------------------------------------------------------------------------

float MixTexture::Y() const {
	return Lerp(amount->Y(), tex1->Y(), tex2->Y());
}

float MixTexture::Filter() const {
	return Lerp(amount->Filter(), tex1->Filter(), tex2->Filter());
}

float MixTexture::GetFloatValue(const HitPoint &hitPoint) const {
	const float amt = Clamp(amount->GetFloatValue(hitPoint), 0.f, 1.f);
	const float value1 = tex1->GetFloatValue(hitPoint);
	const float value2 = tex2->GetFloatValue(hitPoint);

	return Lerp(amt, value1, value2);
}

Spectrum MixTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	const float amt = Clamp(amount->GetFloatValue(hitPoint), 0.f, 1.f);
	const Spectrum value1 = tex1->GetSpectrumValue(hitPoint);
	const Spectrum value2 = tex2->GetSpectrumValue(hitPoint);

	return Lerp(amt, value1, value2);
}

Properties MixTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("mix"));
	props.Set(Property("scene.textures." + name + ".amount")(amount->GetName()));
	props.Set(Property("scene.textures." + name + ".texture1")(tex1->GetName()));
	props.Set(Property("scene.textures." + name + ".texture2")(tex2->GetName()));

	return props;
}

//------------------------------------------------------------------------------
// FBM texture
//------------------------------------------------------------------------------

float FBMTexture::GetFloatValue(const HitPoint &hitPoint) const {
	const Point p(mapping->Map(hitPoint));
	const float value = FBm(p, omega, octaves);
	
	return value;
}

Spectrum FBMTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return Spectrum(GetFloatValue(hitPoint));
}

Properties FBMTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("fbm"));
	props.Set(Property("scene.textures." + name + ".octaves")(octaves));
	props.Set(Property("scene.textures." + name + ".roughness")(omega));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

//------------------------------------------------------------------------------
// Marble texture
//------------------------------------------------------------------------------

Spectrum MarbleTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	Point P(mapping->Map(hitPoint));
	P *= scale;

	float marble = P.y + variation * FBm(P, omega, octaves);
	float t = .5f + .5f * sinf(marble);
	// Evaluate marble spline at _t_
	static const float c[9][3] = {
		{ .58f, .58f, .6f},
		{ .58f, .58f, .6f},
		{ .58f, .58f, .6f},
		{ .5f, .5f, .5f},
		{ .6f, .59f, .58f},
		{ .58f, .58f, .6f},
		{ .58f, .58f, .6f},
		{.2f, .2f, .33f},
		{ .58f, .58f, .6f}
	};
#define NC  sizeof(c) / sizeof(c[0])
#define NSEG (NC-3)
	int first = Floor2Int(t * NSEG);
	t = (t * NSEG - first);
#undef NC
#undef NSEG
	Spectrum c0(c[first]), c1(c[first + 1]), c2(c[first + 2]), c3(c[first + 3]);
	// Bezier spline evaluated with de Castilejau's algorithm
	Spectrum s0(Lerp(t, c0, c1));
	Spectrum s1(Lerp(t, c1, c2));
	Spectrum s2(Lerp(t, c2, c3));
	s0 = Lerp(t, s0, s1);
	s1 = Lerp(t, s1, s2);
	// Extra scale of 1.5 to increase variation among colors
	return 1.5f * Lerp(t, s0, s1);
}

float MarbleTexture::Y() const {
	static float c[][3] = { { .58f, .58f, .6f }, { .58f, .58f, .6f }, { .58f, .58f, .6f },
		{ .5f, .5f, .5f }, { .6f, .59f, .58f }, { .58f, .58f, .6f },
		{ .58f, .58f, .6f }, {.2f, .2f, .33f }, { .58f, .58f, .6f }, };
	Spectrum cs;
#define NC  sizeof(c) / sizeof(c[0])
	for (u_int i = 0; i < NC; ++i)
		cs += Spectrum(c[i]);
	return cs.Y() / NC;
#undef NC
}

float MarbleTexture::Filter() const {
	static float c[][3] = { { .58f, .58f, .6f }, { .58f, .58f, .6f }, { .58f, .58f, .6f },
		{ .5f, .5f, .5f }, { .6f, .59f, .58f }, { .58f, .58f, .6f },
		{ .58f, .58f, .6f }, {.2f, .2f, .33f }, { .58f, .58f, .6f }, };
	Spectrum cs;
#define NC  sizeof(c) / sizeof(c[0])
	for (u_int i = 0; i < NC; ++i)
		cs += Spectrum(c[i]);
	return cs.Filter() / NC;
#undef NC
}

float MarbleTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return GetSpectrumValue(hitPoint).Y();
}

Properties MarbleTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("marble"));
	props.Set(Property("scene.textures." + name + ".octaves")(octaves));
	props.Set(Property("scene.textures." + name + ".roughness")(omega));
	props.Set(Property("scene.textures." + name + ".scale")(scale));
	props.Set(Property("scene.textures." + name + ".variation")(variation));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

//------------------------------------------------------------------------------
// Dots texture
//------------------------------------------------------------------------------

bool DotsTexture::Evaluate(const HitPoint &hitPoint) const {
	const UV uv = mapping->Map(hitPoint);

	const int sCell = Floor2Int(uv.u + .5f);
	const int tCell = Floor2Int(uv.v + .5f);
	// Return _insideDot_ result if point is inside dot
	if (Noise(sCell + .5f, tCell + .5f) > 0.f) {
		const float radius = .35f;
		const float maxShift = 0.5f - radius;
		const float sCenter = sCell + maxShift *
			Noise(sCell + 1.5f, tCell + 2.8f);
		const float tCenter = tCell + maxShift *
			Noise(sCell + 4.5f, tCell + 9.8f);
		const float ds = uv.u - sCenter, dt = uv.v - tCenter;
		if (ds * ds + dt * dt < radius * radius)
			return true;
	}

	return false;
}

float DotsTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return Evaluate(hitPoint) ? insideTex->GetFloatValue(hitPoint) :
		outsideTex->GetFloatValue(hitPoint);
}

Spectrum DotsTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return Evaluate(hitPoint) ? insideTex->GetSpectrumValue(hitPoint) :
		outsideTex->GetSpectrumValue(hitPoint);
}

Properties DotsTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("dots"));
	props.Set(Property("scene.textures." + name + ".inside")(insideTex->GetName()));
	props.Set(Property("scene.textures." + name + ".outside")(outsideTex->GetName()));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

//------------------------------------------------------------------------------
// Brick texture
//------------------------------------------------------------------------------

BrickTexture::BrickTexture(const TextureMapping3D *mp, const Texture *t1,
		const Texture *t2, const Texture *t3,
		float brickw, float brickh, float brickd, float mortar,
		float r, float bev, const string &b) :
		mapping(mp), tex1(t1), tex2(t2), tex3(t3),
		brickwidth(brickw), brickheight(brickh), brickdepth(brickd), mortarsize(mortar),
		run(r), initialbrickwidth(brickw), initialbrickheight(brickh), initialbrickdepth(brickd) {
	if (b == "stacked") {
		bond = RUNNING;
		run = 0.f;
	} else if (b == "flemish")
		bond = FLEMISH;
	else if (b == "english") {
		bond = ENGLISH;
		run = 0.25f;
	} else if (b == "herringbone")
		bond = HERRINGBONE;
	else if (b == "basket")
		bond = BASKET;
	else if (b == "chain link") {
		bond = KETTING;
		run = 1.25f;
		offset = Point(.25f, -1.f, 0.f);
	} else {
		bond = RUNNING;
		offset = Point(0.f, -.5f , 0.f);
	}
	if (bond == HERRINGBONE || bond == BASKET) {
		proportion = floorf(brickwidth / brickheight);
		brickdepth = brickheight = brickwidth;
		invproportion = 1.f / proportion;
	}
	mortarwidth = mortarsize / brickwidth;
	mortarheight = mortarsize / brickheight;
	mortardepth = mortarsize / brickdepth;
	bevelwidth = bev / brickwidth;
	bevelheight = bev / brickheight;
	beveldepth = bev / brickdepth;
	usebevel = bev > 0.f;
}

float BrickTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return GetSpectrumValue(hitPoint).Y();
}

bool BrickTexture::RunningAlternate(const Point &p, Point &i, Point &b,
		int nWhole) const {
	const float sub = nWhole + 0.5f;
	const float rsub = ceilf(sub);
	i.z = floorf(p.z);
	b.x = (p.x + i.z * run) / sub;
	b.y = (p.y + i.z * run) / sub;
	i.x = floorf(b.x);
	i.y = floorf(b.y);
	b.x = (b.x - i.x) * sub;
	b.y = (b.y - i.y) * sub;
	b.z = (p.z - i.z) * sub;
	i.x += floor(b.x) / rsub;
	i.y += floor(b.y) / rsub;
	b.x -= floor(b.x);
	b.y -= floor(b.y);
	return b.z > mortarheight && b.y > mortardepth &&
		b.x > mortarwidth;
}

bool BrickTexture::Basket(const Point &p, Point &i) const {
	i.x = floorf(p.x);
	i.y = floorf(p.y);
	float bx = p.x - i.x;
	float by = p.y - i.y;
	i.x += i.y - 2.f * floorf(0.5f * i.y);
	const bool split = (i.x - 2.f * floor(0.5f * i.x)) < 1.f;
	if (split) {
		bx = fmodf(bx, invproportion);
		i.x = floorf(proportion * p.x) * invproportion;
	} else {
		by = fmodf(by, invproportion);
		i.y = floorf(proportion * p.y) * invproportion;
	}
	return by > mortardepth && bx > mortarwidth;
}

bool BrickTexture::Herringbone(const Point &p, Point &i) const {
	i.y = floorf(proportion * p.y);
	const float px = p.x + i.y * invproportion;
	i.x = floorf(px);
	float bx = 0.5f * px - floorf(px * 0.5f);
	bx *= 2.f;
	float by = proportion * p.y - floorf(proportion * p.y);
	by *= invproportion;
	if (bx > 1.f + invproportion) {
		bx = proportion * (bx - 1.f);
		i.y -= floorf(bx - 1.f);
		bx -= floorf(bx);
		bx *= invproportion;
		by = 1.f;
	} else if (bx > 1.f) {
		bx = proportion * (bx - 1.f);
		i.y -= floorf(bx - 1.f);
		bx -= floorf(bx);
		bx *= invproportion;
	}
	return by > mortarheight && bx > mortarwidth;
}

bool BrickTexture::Running(const Point &p, Point &i, Point &b) const {
	i.z = floorf(p.z);
	b.x = p.x + i.z * run;
	b.y = p.y - i.z * run;
	i.x = floorf(b.x);
	i.y = floorf(b.y);
	b.z = p.z - i.z;
	b.x -= i.x;
	b.y -= i.y;
	return b.z > mortarheight && b.y > mortardepth &&
		b.x > mortarwidth;
}

bool BrickTexture::English(const Point &p, Point &i, Point &b) const {
	i.z = floorf(p.z);
	b.x = p.x + i.z * run;
	b.y = p.y - i.z * run;
	i.x = floorf(b.x);
	i.y = floorf(b.y);
	b.z = p.z - i.z;
	const float divider = floorf(fmodf(fabsf(i.z), 2.f)) + 1.f;
	b.x = (divider * b.x - floorf(divider * b.x)) / divider;
	b.y = (divider * b.y - floorf(divider * b.y)) / divider;
	return b.z > mortarheight && b.y > mortardepth &&
		b.x > mortarwidth;
}

Spectrum BrickTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
#define BRICK_EPSILON 1e-3f
	const Point P(mapping->Map(hitPoint));

	const float offs = BRICK_EPSILON + mortarsize;
	Point bP(P + Point(offs, offs, offs));

	// Normalize coordinates according to brick dimensions
	bP.x /= brickwidth;
	bP.y /= brickdepth;
	bP.z /= brickheight;

	bP += offset;

	Point brickIndex;
	Point bevel;
	bool b;
	switch (bond) {
		case FLEMISH:
			b = RunningAlternate(bP, brickIndex, bevel, 1);
			break;
		case RUNNING:
			b = Running(bP, brickIndex, bevel);
			break;
		case ENGLISH:
			b = English(bP, brickIndex, bevel);
			break;
		case HERRINGBONE:
			b = Herringbone(bP, brickIndex);
			break;
		case BASKET:
			b = Basket(bP, brickIndex);
			break;
		case KETTING:
			b = RunningAlternate(bP, brickIndex, bevel, 2);
			break; 
		default:
			b = true;
			break;
	}

	if (b) {
		// Brick texture * Modulation texture
		return tex1->GetSpectrumValue(hitPoint) * tex3->GetSpectrumValue(hitPoint);
	} else {
		// Mortar texture
		return tex2->GetSpectrumValue(hitPoint);
	}
#undef BRICK_EPSILON
}

Properties BrickTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("brick"));
	props.Set(Property("scene.textures." + name + ".bricktex")(tex1->GetName()));
	props.Set(Property("scene.textures." + name + ".mortartex")(tex2->GetName()));
	props.Set(Property("scene.textures." + name + ".brickmodtex")(tex3->GetName()));
	props.Set(Property("scene.textures." + name + ".brickwidth")(brickwidth));
	props.Set(Property("scene.textures." + name + ".brickheight")(brickheight));
	props.Set(Property("scene.textures." + name + ".brickdepth")(brickdepth));
	props.Set(Property("scene.textures." + name + ".mortarsize")(mortarsize));
	props.Set(Property("scene.textures." + name + ".brickrun")(run));
	props.Set(Property("scene.textures." + name + ".brickbevel")(bevelwidth * brickwidth));

	string brickBondValue;
	switch (bond) {
		case FLEMISH:
			brickBondValue = "flemish";
			break;
		case ENGLISH:
			brickBondValue = "english";
			break;
		case HERRINGBONE:
			brickBondValue = "herringbone";
			break;
		case BASKET:
			brickBondValue = "basket";
			break;
		case KETTING:
			brickBondValue = "chain link";
			break;
		default:
		case RUNNING:
			brickBondValue = "stacked";
			break;
	}
	props.Set(Property("scene.textures." + name + ".brickbond")(brickBondValue));

	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

//------------------------------------------------------------------------------
// Add texture
//------------------------------------------------------------------------------

float AddTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return tex1->GetFloatValue(hitPoint) + tex2->GetFloatValue(hitPoint);
}

Spectrum AddTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return tex1->GetSpectrumValue(hitPoint) + tex2->GetSpectrumValue(hitPoint);
}

Properties AddTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("add"));
	props.Set(Property("scene.textures." + name + ".texture1")(tex1->GetName()));
	props.Set(Property("scene.textures." + name + ".texture2")(tex2->GetName()));

	return props;
}

//------------------------------------------------------------------------------
// Subtract texture
//------------------------------------------------------------------------------

float SubtractTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return tex1->GetFloatValue(hitPoint) - tex2->GetFloatValue(hitPoint);
}

Spectrum SubtractTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return tex1->GetSpectrumValue(hitPoint) - tex2->GetSpectrumValue(hitPoint);
}

Properties SubtractTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;
	
	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("substract"));
	props.Set(Property("scene.textures." + name + ".texture1")(tex1->GetName()));
	props.Set(Property("scene.textures." + name + ".texture2")(tex2->GetName()));
	
	return props;
}

//------------------------------------------------------------------------------
// Windy texture
//------------------------------------------------------------------------------

float WindyTexture::GetFloatValue(const HitPoint &hitPoint) const {
	const Point P(mapping->Map(hitPoint));
	const float windStrength = FBm(.1f * P, .5f, 3);
	const float waveHeight = FBm(P, .5f, 6);

	return fabsf(windStrength) * waveHeight;
}

Spectrum WindyTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return Spectrum(GetFloatValue(hitPoint));
}

Properties WindyTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("windy"));

	return props;
}

//------------------------------------------------------------------------------
// Wrinkled texture
//------------------------------------------------------------------------------

float WrinkledTexture::GetFloatValue(const HitPoint &hitPoint) const {
	const Point p(mapping->Map(hitPoint));
	return Turbulence(p, omega, octaves);
}

Spectrum WrinkledTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return Spectrum(GetFloatValue(hitPoint));
}

Properties WrinkledTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("wrinkled"));
	props.Set(Property("scene.textures." + name + ".octaves")(octaves));
	props.Set(Property("scene.textures." + name + ".roughness")(omega));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

//------------------------------------------------------------------------------
// UV texture
//------------------------------------------------------------------------------

float UVTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return GetSpectrumValue(hitPoint).Y();
}

Spectrum UVTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	const UV uv = mapping->Map(hitPoint);
	
	return Spectrum(uv.u - Floor2Int(uv.u), uv.v - Floor2Int(uv.v), 0.f);
}

Properties UVTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("uv"));
	props.Set(mapping->ToProperties("scene.textures." + name + ".mapping"));

	return props;
}

//------------------------------------------------------------------------------
// Band texture
//------------------------------------------------------------------------------

float BandTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return GetSpectrumValue(hitPoint).Y();
}

Spectrum BandTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	const float a = Clamp(amount->GetFloatValue(hitPoint), 0.f, 1.f);

	if (a < offsets.front())
		return values.front();
	if (a >= offsets.back())
		return values.back();
	// upper_bound is not available on OpenCL
	//const u_int p = upper_bound(offsets.begin(), offsets.end(), a) - offsets.begin();
	u_int p = 0;
	for (; p < offsets.size(); ++p) {
		if (a < offsets[p])
			break;
	}

	return Lerp((a - offsets[p - 1]) / (offsets[p] - offsets[p - 1]),
			values[p - 1], values[p]);
}

Properties BandTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("band"));
	props.Set(Property("scene.textures." + name + ".amount")(amount->GetName()));

	for (u_int i = 0; i < offsets.size(); ++i) {
		props.Set(Property("scene.textures." + name + ".offset" + ToString(i))(offsets[i]));
		props.Set(Property("scene.textures." + name + ".value" + ToString(i))(values[i]));
	}

	return props;
}

//------------------------------------------------------------------------------
// HitPointColor texture
//------------------------------------------------------------------------------

float HitPointColorTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return hitPoint.color.Y();
}

Spectrum HitPointColorTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return hitPoint.color;
}

Properties HitPointColorTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("hitpointcolor"));

	return props;
}

//------------------------------------------------------------------------------
// HitPointAlpha texture
//------------------------------------------------------------------------------

float HitPointAlphaTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return hitPoint.alpha;
}

Spectrum HitPointAlphaTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	return Spectrum(hitPoint.alpha);
}

Properties HitPointAlphaTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("hitpointalpha"));

	return props;
}

//------------------------------------------------------------------------------
// HitPointGrey texture
//------------------------------------------------------------------------------

float HitPointGreyTexture::GetFloatValue(const HitPoint &hitPoint) const {
	return (channel > 2) ? hitPoint.color.Y() : hitPoint.color.c[channel];
}

Spectrum HitPointGreyTexture::GetSpectrumValue(const HitPoint &hitPoint) const {
	const float v = (channel > 2) ? hitPoint.color.Y() : hitPoint.color.c[channel];
	return Spectrum(v);
}

Properties HitPointGreyTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("hitpointgrey"));
	props.Set(Property("scene.textures." + name + ".channel")(
		((channel != 0) && (channel != 1) && (channel != 2)) ? -1 : ((int)channel)));

	return props;
}

//------------------------------------------------------------------------------
// NormalMap textures
//------------------------------------------------------------------------------

UV NormalMapTexture::GetDuv(const HitPoint &hitPoint,
        const Vector &dpdu, const Vector &dpdv,
        const Normal &dndu, const Normal &dndv,
        const float sampleDistance) const {
    Spectrum rgb = tex->GetSpectrumValue(hitPoint);
    rgb.Clamp(-1.f, 1.f);

	// Normal from normal map
	Vector n(rgb.c[0], rgb.c[1], rgb.c[2]);
	n = 2.f * n - Vector(1.f, 1.f, 1.f);

	const Vector k = Vector(hitPoint.shadeN);

	// Transform n from tangent to object space
	const Vector &t1 = dpdu;
	const Vector &t2 = dpdv;
    const float btsign = (Dot(dpdv, hitPoint.shadeN) > 0.f) ? 1.f : -1.f;

	// Magnitude of btsign is the magnitude of the interpolated normal
	const Vector kk = k * fabsf(btsign);

	// tangent -> object
	n = Normalize(n.x * t1 + n.y * btsign * t2 + n.z * kk);	

	// Since n is stored normalized in the normal map
	// we need to recover the original length (lambda).
	// We do this by solving 
	//   lambda*n = dp/du x dp/dv
	// where 
	//   p(u,v) = base(u,v) + h(u,v) * k
	// and
	//   k = dbase/du x dbase/dv
	//
	// We recover lambda by dotting the above with k
	//   k . lambda*n = k . (dp/du x dp/dv)
	//   lambda = (k . k) / (k . n)
	// 
	// We then recover dh/du by dotting the first eq by dp/du
	//   dp/du . lambda*n = dp/du . (dp/du x dp/dv)
	//   dp/du . lambda*n = dh/du * [dbase/du . (k x dbase/dv)]
	//
	// The term "dbase/du . (k x dbase/dv)" reduces to "-(k . k)", so we get
	//   dp/du . lambda*n = dh/du * -(k . k)
	//   dp/du . [(k . k) / (k . n)*n] = dh/du * -(k . k)
	//   dp/du . [-n / (k . n)] = dh/du
	// and similar for dh/dv
	// 
	// Since the recovered dh/du will be in units of ||k||, we must divide
	// by ||k|| to get normalized results. Using dg.nn as k in the last eq 
	// yields the same result.
	const Vector nn = (-1.f / Dot(k, n)) * n;

	return UV(Dot(dpdu, nn), Dot(dpdv, nn));
}

Properties NormalMapTexture::ToProperties(const ImageMapCache &imgMapCache) const {
	Properties props;

	const string name = GetName();
	props.Set(Property("scene.textures." + name + ".type")("normalmap"));
	props.Set(Property("scene.textures." + name + ".texture")(tex->GetName()));

	return props;
}