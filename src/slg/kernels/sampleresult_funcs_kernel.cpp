#include <string>
namespace slg { namespace ocl {
std::string KernelSource_sampleresult_funcs = 
"#line 2 \"sampleresult_funcs.cl\"\n"
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
"void SampleResult_Init(__global SampleResult *sampleResult) {\n"
"	// Initialize only Spectrum fields\n"
"\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"	VSTORE3F(BLACK, sampleResult->radiancePerPixelNormalized[0].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"	VSTORE3F(BLACK, sampleResult->radiancePerPixelNormalized[1].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"	VSTORE3F(BLACK, sampleResult->radiancePerPixelNormalized[2].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"	VSTORE3F(BLACK, sampleResult->radiancePerPixelNormalized[3].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"	VSTORE3F(BLACK, sampleResult->radiancePerPixelNormalized[4].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"	VSTORE3F(BLACK, sampleResult->radiancePerPixelNormalized[5].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"	VSTORE3F(BLACK, sampleResult->radiancePerPixelNormalized[6].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"	VSTORE3F(BLACK, sampleResult->radiancePerPixelNormalized[7].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)\n"
"	VSTORE3F(BLACK, sampleResult->directDiffuse.c);\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)\n"
"	VSTORE3F(BLACK, sampleResult->directGlossy.c);\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)\n"
"	VSTORE3F(BLACK, sampleResult->emission.c);\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"	VSTORE3F(BLACK, sampleResult->indirectDiffuse.c);\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"	VSTORE3F(BLACK, sampleResult->indirectGlossy.c);\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"	VSTORE3F(BLACK, sampleResult->indirectSpecular.c);\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_RAYCOUNT)\n"
"	sampleResult->rayCount = 0.f;\n"
"#endif\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_IRRADIANCE)\n"
"	VSTORE3F(BLACK, sampleResult->irradiance.c);\n"
"#endif\n"
"\n"
"	sampleResult->firstPathVertexEvent = NONE;\n"
"	sampleResult->firstPathVertex = true;\n"
"	// sampleResult->lastPathVertex can not be really initialized here without knowing\n"
"	// the max. path depth.\n"
"	sampleResult->lastPathVertex = true;\n"
"	sampleResult->passThroughPath = true;\n"
"}\n"
"\n"
"void SampleResult_AddEmission(__global SampleResult *sampleResult, const uint lightID,\n"
"		const float3 pathThroughput, const float3 incomingRadiance) {\n"
"	const float3 radiance = pathThroughput * incomingRadiance;\n"
"\n"
"	// Avoid out of bound access if the light group doesn't exist. This can happen\n"
"	// with RT modes.\n"
"	const uint id = min(lightID, PARAM_FILM_RADIANCE_GROUP_COUNT - 1u);\n"
"	VADD3F(sampleResult->radiancePerPixelNormalized[id].c, radiance);\n"
"\n"
"	if (sampleResult->firstPathVertex) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)\n"
"		VADD3F(sampleResult->emission.c, radiance);\n"
"#endif\n"
"	} else {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"		sampleResult->indirectShadowMask = 0.f;\n"
"#endif\n"
"		const BSDFEvent firstPathVertexEvent = sampleResult->firstPathVertexEvent;\n"
"		if (firstPathVertexEvent & DIFFUSE) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"			VADD3F(sampleResult->indirectDiffuse.c, radiance);\n"
"#endif\n"
"		} else if (firstPathVertexEvent & GLOSSY) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"			VADD3F(sampleResult->indirectGlossy.c, radiance);\n"
"#endif\n"
"		} else if (firstPathVertexEvent & SPECULAR) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"			VADD3F(sampleResult->indirectSpecular.c, radiance);\n"
"#endif\n"
"		}\n"
"	}\n"
"}\n"
"\n"
"void SampleResult_AddDirectLight(__global SampleResult *sampleResult, const uint lightID,\n"
"		const BSDFEvent bsdfEvent, const float3 pathThroughput, const float3 incomingRadiance,\n"
"		const float lightScale) {\n"
"	const float3 radiance = pathThroughput * incomingRadiance;\n"
"\n"
"	// Avoid out of bound access if the light group doesn't exist. This can happen\n"
"	// with RT modes.\n"
"	const uint id = min(lightID, PARAM_FILM_RADIANCE_GROUP_COUNT - 1u);\n"
"	VADD3F(sampleResult->radiancePerPixelNormalized[id].c, radiance);\n"
"\n"
"	if (sampleResult->firstPathVertex) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK)\n"
"		sampleResult->directShadowMask = fmax(0.f, sampleResult->directShadowMask - lightScale);\n"
"#endif\n"
"\n"
"		if (bsdfEvent & DIFFUSE) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE)\n"
"			VADD3F(sampleResult->directDiffuse.c, radiance);\n"
"#endif\n"
"		} else {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY)\n"
"			VADD3F(sampleResult->directGlossy.c, radiance);\n"
"#endif\n"
"		}\n"
"	} else {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"		sampleResult->indirectShadowMask = fmax(0.f, sampleResult->indirectShadowMask - lightScale);\n"
"#endif\n"
"\n"
"		const BSDFEvent firstPathVertexEvent = sampleResult->firstPathVertexEvent;\n"
"		if (firstPathVertexEvent & DIFFUSE) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"			VADD3F(sampleResult->indirectDiffuse.c, radiance);\n"
"#endif\n"
"		} else if (firstPathVertexEvent & GLOSSY) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"			VADD3F(sampleResult->indirectGlossy.c, radiance);\n"
"#endif\n"
"		} else if (firstPathVertexEvent & SPECULAR) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"			VADD3F(sampleResult->indirectSpecular.c, radiance);\n"
"#endif\n"
"		}\n"
"\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_IRRADIANCE)\n"
"		VADD3F(sampleResult->irradiance.c, VLOAD3F(sampleResult->irradiancePathThroughput.c) * incomingRadiance);\n"
"#endif\n"
"	}\n"
"}\n"
"\n"
"float SampleResult_Radiance_Y(__global SampleResult *sampleResult) {\n"
"	float y = 0.f;\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"	y += Spectrum_Y(VLOAD3F(sampleResult->radiancePerPixelNormalized[0].c));\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"	y += Spectrum_Y(VLOAD3F(sampleResult->radiancePerPixelNormalized[1].c));\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"	y += Spectrum_Y(VLOAD3F(sampleResult->radiancePerPixelNormalized[2].c));\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"	y += Spectrum_Y(VLOAD3F(sampleResult->radiancePerPixelNormalized[3].c));\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"	y += Spectrum_Y(VLOAD3F(sampleResult->radiancePerPixelNormalized[4].c));\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"	y += Spectrum_Y(VLOAD3F(sampleResult->radiancePerPixelNormalized[5].c));\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"	y += Spectrum_Y(VLOAD3F(sampleResult->radiancePerPixelNormalized[6].c));\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"	y += Spectrum_Y(VLOAD3F(sampleResult->radiancePerPixelNormalized[7].c));\n"
"#endif\n"
"\n"
"	return y;\n"
"}\n"
"\n"
"void SampleResult_ClampRadiance(__global SampleResult *sampleResult,\n"
"		const float minRadiance, const float maxRadiance) {\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n"
"	VSTORE3F(\n"
"			Spectrum_ScaledClamp(VLOAD3F(sampleResult->radiancePerPixelNormalized[0].c), minRadiance, maxRadiance),\n"
"			sampleResult->radiancePerPixelNormalized[0].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n"
"	VSTORE3F(\n"
"			Spectrum_ScaledClamp(VLOAD3F(sampleResult->radiancePerPixelNormalized[1].c), minRadiance, maxRadiance),\n"
"			sampleResult->radiancePerPixelNormalized[0].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n"
"	VSTORE3F(\n"
"			Spectrum_ScaledClamp(VLOAD3F(sampleResult->radiancePerPixelNormalized[2].c), minRadiance, maxRadiance),\n"
"			sampleResult->radiancePerPixelNormalized[0].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n"
"	VSTORE3F(\n"
"			Spectrum_ScaledClamp(VLOAD3F(sampleResult->radiancePerPixelNormalized[3].c), minRadiance, maxRadiance),\n"
"			sampleResult->radiancePerPixelNormalized[0].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n"
"	VSTORE3F(\n"
"			Spectrum_ScaledClamp(VLOAD3F(sampleResult->radiancePerPixelNormalized[4].c), minRadiance, maxRadiance),\n"
"			sampleResult->radiancePerPixelNormalized[0].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n"
"	VSTORE3F(\n"
"			Spectrum_ScaledClamp(VLOAD3F(sampleResult->radiancePerPixelNormalized[5].c), minRadiance, maxRadiance),\n"
"			sampleResult->radiancePerPixelNormalized[0].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n"
"	VSTORE3F(\n"
"			Spectrum_ScaledClamp(VLOAD3F(sampleResult->radiancePerPixelNormalized[6].c), minRadiance, maxRadiance),\n"
"			sampleResult->radiancePerPixelNormalized[0].c);\n"
"#endif\n"
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n"
"	VSTORE3F(\n"
"			Spectrum_ScaledClamp(VLOAD3F(sampleResult->radiancePerPixelNormalized[7].c), minRadiance, maxRadiance),\n"
"			sampleResult->radiancePerPixelNormalized[0].c);\n"
"#endif\n"
"}\n"
; } }
