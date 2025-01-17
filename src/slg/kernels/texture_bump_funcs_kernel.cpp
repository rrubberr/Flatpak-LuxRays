#include <string>
namespace slg { namespace ocl {
std::string KernelSource_texture_bump_funcs = 
"#line 2 \"texture_bump_funcs.cl\"\n"
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
"//------------------------------------------------------------------------------\n"
"// Texture bump/normal mapping\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_BUMPMAPS)\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Generic texture bump mapping\n"
"//------------------------------------------------------------------------------\n"
"\n"
"float3 GenericTexture_Bump(\n"
"		const uint texIndex,\n"
"		__global HitPoint *hitPoint,\n"
"		const float sampleDistance\n"
"		TEXTURES_PARAM_DECL) {\n"
"	const float3 dpdu = VLOAD3F(&hitPoint->dpdu.x);\n"
"	const float3 dpdv = VLOAD3F(&hitPoint->dpdv.x);\n"
"	const float3 dndu = VLOAD3F(&hitPoint->dndu.x);\n"
"	const float3 dndv = VLOAD3F(&hitPoint->dndv.x);\n"
"\n"
"	// Calculate bump map value at intersection point\n"
"	const float base = Texture_GetFloatValue(texIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"\n"
"	// Compute offset positions and evaluate displacement texIndex\n"
"	const float3 origP = VLOAD3F(&hitPoint->p.x);\n"
"	const float3 origShadeN = VLOAD3F(&hitPoint->shadeN.x);\n"
"	const float2 origUV = VLOAD2F(&hitPoint->uv.u);\n"
"\n"
"	float2 duv;\n"
"\n"
"	// Shift hitPointTmp.du in the u direction and calculate value\n"
"	const float uu = sampleDistance / length(dpdu);\n"
"	VSTORE3F(origP + uu * dpdu, &hitPoint->p.x);\n"
"	hitPoint->uv.u += uu;\n"
"	VSTORE3F(normalize(origShadeN + uu * dndu), &hitPoint->shadeN.x);\n"
"	const float duValue = Texture_GetFloatValue(texIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"	duv.s0 = (duValue - base) / uu;\n"
"\n"
"	// Shift hitPointTmp.dv in the v direction and calculate value\n"
"	const float vv = sampleDistance / length(dpdv);\n"
"	VSTORE3F(origP + vv * dpdv, &hitPoint->p.x);\n"
"	hitPoint->uv.u = origUV.s0;\n"
"	hitPoint->uv.v += vv;\n"
"	VSTORE3F(normalize(origShadeN + vv * dndv), &hitPoint->shadeN.x);\n"
"	const float dvValue = Texture_GetFloatValue(texIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"	duv.s1 = (dvValue - base) / vv;\n"
"\n"
"	// Restore HitPoint\n"
"	VSTORE3F(origP, &hitPoint->p.x);\n"
"	VSTORE2F(origUV, &hitPoint->uv.u);\n"
"\n"
"	// Compute the new dpdu and dpdv\n"
"	const float3 bumpDpdu = dpdu + duv.s0 * origShadeN;\n"
"	const float3 bumpDpdv = dpdv + duv.s1 * origShadeN;\n"
"	float3 newShadeN = normalize(cross(bumpDpdu, bumpDpdv));\n"
"\n"
"	// The above transform keeps the normal in the original normal\n"
"	// hemisphere. If they are opposed, it means UVN was indirect and\n"
"	// the normal needs to be reversed\n"
"	newShadeN *= (dot(origShadeN, newShadeN) < 0.f) ? -1.f : 1.f;\n"
"\n"
"	return newShadeN;\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// ConstFloatTexture\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_CONST_FLOAT)\n"
"float3 ConstFloatTexture_Bump(__global HitPoint *hitPoint) {\n"
"	return VLOAD3F(&hitPoint->shadeN.x);\n"
"}\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// ConstFloat3Texture\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_CONST_FLOAT3)\n"
"float3 ConstFloat3Texture_Bump(__global HitPoint *hitPoint) {\n"
"	return VLOAD3F(&hitPoint->shadeN.x);\n"
"}\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// FresnelConstTexture\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_FRESNELCONST)\n"
"float3 FresnelConstTexture_Bump(__global HitPoint *hitPoint) {\n"
"	return VLOAD3F(&hitPoint->shadeN.x);\n"
"}\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// FresnelColorTexture\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_FRESNELCOLOR)\n"
"float3 FresnelColorTexture_Bump(__global HitPoint *hitPoint) {\n"
"	return VLOAD3F(&hitPoint->shadeN.x);\n"
"}\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// ImageMapTexture\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_IMAGEMAP) && defined(PARAM_HAS_IMAGEMAPS)\n"
"float3 ImageMapTexture_Bump(__global const Texture *tex, __global HitPoint *hitPoint,\n"
"		const float sampleDistance\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	float2 du, dv;\n"
"	const float2 uv = TextureMapping2D_MapDuv(&tex->imageMapTex.mapping, hitPoint, &du, &dv);\n"
"	__global const ImageMap *imageMap = &imageMapDescs[tex->imageMapTex.imageMapIndex];\n"
"	const float2 dst = ImageMap_GetDuv(imageMap, uv.x, uv.y IMAGEMAPS_PARAM);\n"
"	const float2 duv = tex->imageMapTex.gain * (float2)(dot(dst, du), dot(dst, dv));\n"
"	const float3 shadeN = VLOAD3F(&hitPoint->shadeN.x);\n"
"	const float3 n = normalize(cross(VLOAD3F(&hitPoint->dpdu.x) + duv.x * shadeN, VLOAD3F(&hitPoint->dpdv.x) + duv.y * shadeN));\n"
"	if (dot(n, shadeN) < 0.f)\n"
"		return -n;\n"
"	else\n"
"		return n;\n"
"}\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// NormalMapTexture\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_NORMALMAP)\n"
"float3 NormalMapTexture_Bump(\n"
"		__global const Texture *tex,\n"
"		__global HitPoint *hitPoint,\n"
"		const float sampleDistance\n"
"		TEXTURES_PARAM_DECL) {\n"
"	// Normal from normal map\n"
"	float3 rgb = Texture_GetSpectrumValue(tex->normalMap.texIndex, hitPoint\n"
"			TEXTURES_PARAM);\n"
"	rgb = clamp(rgb, -1.f, 1.f);\n"
"\n"
"	// Normal from normal map\n"
"	float3 n = 2.f * rgb - (float3)(1.f, 1.f, 1.f);\n"
"	const float scale = tex->normalMap.scale;\n"
"	n.x *= scale;\n"
"	n.y *= scale;\n"
"\n"
"	const float3 oldShadeN = VLOAD3F(&hitPoint->shadeN.x);\n"
"	float3 dpdu = VLOAD3F(&hitPoint->dpdu.x);\n"
"	float3 dpdv = VLOAD3F(&hitPoint->dpdv.x);\n"
"	\n"
"	Frame frame;\n"
"	Frame_Set_Private(&frame, dpdu, dpdv, oldShadeN);\n"
"\n"
"	// Transform n from tangent to object space\n"
"	float3 shadeN = normalize(Frame_ToWorld_Private(&frame, n));\n"
"	shadeN *= (dot(oldShadeN, shadeN) < 0.f) ? -1.f : 1.f;\n"
"\n"
"	return shadeN;\n"
"}\n"
"#endif\n"
"\n"
"#endif\n"
; } }
