#include <string>
namespace slg { namespace ocl {
std::string KernelSource_materialdefs_funcs_archglass = 
"#line 2 \"materialdefs_funcs_archglass.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2017 by authors (see AUTHORS.txt)                        *\n"
" *                                                                         *\n"
" *   This file is part of LuxRender.                                       *\n"
" *                                                                         *\n"
" * Licensed under the Apache License, Version 2.0 (the \"License\");         *\n"
" * you may not use this file except in compliance with the License.        *\n"
" * You may obtain a copy of the License at                                 *\n"
" *                                                                         *\n"
" *     http://www.apache.org/licenses/LICENSE-2.0                          *\n"
" *                                                                         *\n"
" * Unless required by applicable law or agreed to in writing, software     *\n"
" * distributed under the License is distributed on an \"AS IS\" BASIS,       *\n"
" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*\n"
" * See the License for the specific language governing permissions and     *\n"
" * limitations under the License.                                          *\n"
" ***************************************************************************/\n"
"\n"
"#if defined(PARAM_HAS_VOLUMES)\n"
"float ExtractExteriorIors(__global HitPoint *hitPoint, const uint exteriorIorTexIndex\n"
"		TEXTURES_PARAM_DECL) {\n"
"	uint extIndex = NULL_INDEX;\n"
"	if (exteriorIorTexIndex != NULL_INDEX)\n"
"		extIndex = exteriorIorTexIndex;\n"
"	else {\n"
"		const uint hitPointExteriorIorTexIndex = hitPoint->exteriorIorTexIndex;\n"
"		if (hitPointExteriorIorTexIndex != NULL_INDEX)\n"
"			extIndex = hitPointExteriorIorTexIndex;\n"
"	}\n"
"	return (extIndex == NULL_INDEX) ? 1.f : Texture_GetFloatValue(extIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"}\n"
"\n"
"float ExtractInteriorIors(__global HitPoint *hitPoint, const uint interiorIorTexIndex\n"
"		TEXTURES_PARAM_DECL) {\n"
"	uint intIndex = NULL_INDEX;\n"
"	if (interiorIorTexIndex != NULL_INDEX)\n"
"		intIndex = interiorIorTexIndex;\n"
"	else {\n"
"		const uint hitPointInteriorIorTexIndex = hitPoint->interiorIorTexIndex;\n"
"		if (hitPointInteriorIorTexIndex != NULL_INDEX)\n"
"			intIndex = hitPointInteriorIorTexIndex;\n"
"	}\n"
"	return (intIndex == NULL_INDEX) ? 1.f : Texture_GetFloatValue(intIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"}\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// ArchGlass material\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_ARCHGLASS)\n"
"\n"
"BSDFEvent ArchGlassMaterial_GetEventTypes() {\n"
"	return SPECULAR | REFLECT | TRANSMIT;\n"
"}\n"
"\n"
"bool ArchGlassMaterial_IsDelta() {\n"
"	return true;\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"float3 ArchGlassMaterial_GetPassThroughTransparency(__global const Material *material,\n"
"		__global HitPoint *hitPoint, const float3 localFixedDir, const float passThroughEvent\n"
"		TEXTURES_PARAM_DECL) {\n"
"	const float3 kt = Spectrum_Clamp(Texture_GetSpectrumValue(material->archglass.ktTexIndex, hitPoint\n"
"		TEXTURES_PARAM));\n"
"	const float3 kr = Spectrum_Clamp(Texture_GetSpectrumValue(material->archglass.krTexIndex, hitPoint\n"
"		TEXTURES_PARAM));\n"
"\n"
"	const bool isKtBlack = Spectrum_IsBlack(kt);\n"
"	const bool isKrBlack = Spectrum_IsBlack(kr);\n"
"	if (isKtBlack && isKrBlack)\n"
"		return BLACK;\n"
"\n"
"	const bool entering = (CosTheta(localFixedDir) > 0.f);\n"
"	\n"
"	const float nc = ExtractExteriorIors(hitPoint,\n"
"			material->archglass.exteriorIorTexIndex\n"
"			TEXTURES_PARAM);\n"
"	const float nt = ExtractInteriorIors(hitPoint,\n"
"			material->archglass.interiorIorTexIndex\n"
"			TEXTURES_PARAM);\n"
"	const float ntc = nt / nc;\n"
"	const float costheta = CosTheta(localFixedDir);\n"
"\n"
"	// Decide to transmit or reflect\n"
"	const float threshold = isKrBlack ? 1.f : (isKtBlack ? 0.f : .5f);\n"
"	if (passThroughEvent < threshold) {\n"
"		// Transmit\n"
"\n"
"		// Compute transmitted ray direction\n"
"		const float sini2 = SinTheta2(localFixedDir);\n"
"		const float eta = nc / nt;\n"
"		const float eta2 = eta * eta;\n"
"		const float sint2 = eta2 * sini2;\n"
"\n"
"		// Handle total internal reflection for transmission\n"
"		if (sint2 >= 1.f)\n"
"			return BLACK;\n"
"\n"
"		float3 result;\n"
"		//if (!hitPoint.fromLight) {\n"
"			if (entering)\n"
"				result = BLACK;\n"
"			else\n"
"				result = FresnelCauchy_Evaluate(ntc, -costheta);\n"
"		//} else {\n"
"		//	if (entering)\n"
"		//		result = FresnelCauchy_Evaluate(ntc, costheta);\n"
"		//	else\n"
"		//		result = BLACK;\n"
"		//}\n"
"		result *= 1.f + (1.f - result) * (1.f - result);\n"
"		result = 1.f - result;\n"
"\n"
"		// The \"2.f*\" is there in place of \"/threshold\" (aka \"/pdf\")\n"
"		return 2.f * kt * result;\n"
"	} else\n"
"		return BLACK;\n"
"}\n"
"#endif\n"
"\n"
"float3 ArchGlassMaterial_Evaluate(\n"
"		__global HitPoint *hitPoint, const float3 lightDir, const float3 eyeDir,\n"
"		BSDFEvent *event, float *directPdfW,\n"
"		const float3 ktTexVal, const float3 krTexVal,\n"
"		const float nc, const float nt) {\n"
"	return BLACK;\n"
"}\n"
"\n"
"float3 ArchGlassMaterial_Sample(\n"
"		__global HitPoint *hitPoint, const float3 localFixedDir, float3 *localSampledDir,\n"
"		const float u0, const float u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float *pdfW, float *absCosSampledDir, BSDFEvent *event,\n"
"		const BSDFEvent requestedEvent,\n"
"		const float3 ktTexVal, const float3 krTexVal,\n"
"		const float nc, const float nt) {\n"
"	if (!(requestedEvent & SPECULAR))\n"
"		return BLACK;\n"
"\n"
"	const float3 kt = Spectrum_Clamp(ktTexVal);\n"
"	const float3 kr = Spectrum_Clamp(krTexVal);\n"
"\n"
"	const bool isKtBlack = Spectrum_IsBlack(kt);\n"
"	const bool isKrBlack = Spectrum_IsBlack(kr);\n"
"	if (isKtBlack && isKrBlack)\n"
"		return BLACK;\n"
"\n"
"	const bool entering = (CosTheta(localFixedDir) > 0.f);\n"
"	const float ntc = nt / nc;\n"
"	const float eta = nc / nt;\n"
"	const float costheta = CosTheta(localFixedDir);\n"
"\n"
"	// Decide to transmit or reflect\n"
"	float threshold;\n"
"	if ((requestedEvent & REFLECT) && !isKrBlack) {\n"
"		if ((requestedEvent & TRANSMIT) && !isKtBlack)\n"
"			threshold = .5f;\n"
"		else\n"
"			threshold = 0.f;\n"
"	} else {\n"
"		if ((requestedEvent & TRANSMIT) && !isKtBlack)\n"
"			threshold = 1.f;\n"
"		else\n"
"			return BLACK;\n"
"	}\n"
"\n"
"	float3 result;\n"
"	if (passThroughEvent < threshold) {\n"
"		// Transmit\n"
"\n"
"		// Compute transmitted ray direction\n"
"		const float sini2 = SinTheta2(localFixedDir);\n"
"		const float eta2 = eta * eta;\n"
"		const float sint2 = eta2 * sini2;\n"
"\n"
"		// Handle total internal reflection for transmission\n"
"		if (sint2 >= 1.f)\n"
"			return BLACK;\n"
"\n"
"		*localSampledDir = -localFixedDir;\n"
"		*absCosSampledDir = fabs(CosTheta(*localSampledDir));\n"
"\n"
"		*event = SPECULAR | TRANSMIT;\n"
"		*pdfW = threshold;\n"
"\n"
"		//if (!hitPoint.fromLight) {\n"
"			if (entering)\n"
"				result = BLACK;\n"
"			else\n"
"				result = FresnelCauchy_Evaluate(ntc, -costheta);\n"
"		//} else {\n"
"		//	if (entering)\n"
"		//		result = FresnelCauchy_Evaluate(ntc, costheta);\n"
"		//	else\n"
"		//		result = BLACK;\n"
"		//}\n"
"		result *= 1.f + (1.f - result) * (1.f - result);\n"
"		result = 1.f - result;\n"
"\n"
"		result *= kt;\n"
"	} else {\n"
"		// Reflect\n"
"		if (costheta <= 0.f)\n"
"			return BLACK;\n"
"\n"
"		*localSampledDir = (float3)(-localFixedDir.x, -localFixedDir.y, localFixedDir.z);\n"
"		*absCosSampledDir = fabs(CosTheta(*localSampledDir));\n"
"\n"
"		*event = SPECULAR | REFLECT;\n"
"		*pdfW = 1.f - threshold;\n"
"\n"
"		result = kr * FresnelCauchy_Evaluate(ntc, costheta);\n"
"	}\n"
"\n"
"	return result / *pdfW;\n"
"}\n"
"\n"
"#endif\n"
; } }
