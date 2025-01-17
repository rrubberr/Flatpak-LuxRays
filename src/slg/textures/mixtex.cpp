/***************************************************************************
 * Copyright 1998-2017 by authors (see AUTHORS.txt)                        *
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

#include "slg/textures/mix.h"

using namespace std;
using namespace luxrays;
using namespace slg;

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

Normal MixTexture::Bump(const HitPoint &hitPoint, const float sampleDistance) const {
	const Vector u = Normalize(hitPoint.dpdu);
	const Vector v = Normalize(Cross(Vector(hitPoint.shadeN), hitPoint.dpdu));
	Normal n = tex1->Bump(hitPoint, sampleDistance);
	float nn = Dot(n, hitPoint.shadeN);
	const float du1 = Dot(n, u) / nn;
	const float dv1 = Dot(n, v) / nn;

	n = tex2->Bump(hitPoint, sampleDistance);
	nn = Dot(n, hitPoint.shadeN);
	const float du2 = Dot(n, u) / nn;
	const float dv2 = Dot(n, v) / nn;

	n = amount->Bump(hitPoint, sampleDistance);
	nn = Dot(n, hitPoint.shadeN);
	const float dua = Dot(n, u) / nn;
	const float dva = Dot(n, v) / nn;

	const float t1 = tex1->GetFloatValue(hitPoint);
	const float t2 = tex2->GetFloatValue(hitPoint);
	const float amt = Clamp(amount->GetFloatValue(hitPoint), 0.f, 1.f);

	const float du = Lerp(amt, du1, du2) + dua * (t2 - t1);
	const float dv = Lerp(amt, dv1, dv2) + dva * (t2 - t1);

	return Normal(Normalize(Vector(hitPoint.shadeN) + du * u + dv * v));
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
