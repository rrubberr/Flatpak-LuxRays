#include <string>
namespace slg { namespace ocl {
std::string KernelSource_tonemap_reinhard02_funcs = 
"#line 2 \"tonemap_reinhard02_funcs.cl\"\n"
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
"// Reinhard02ToneMap_Apply\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(256, 1, 1))) void Reinhard02ToneMap_Apply(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global float *channel_IMAGEPIPELINE,\n"
"		__global uint *channel_FRAMEBUFFER_MASK,\n"
"		const float gamma,\n"
"		const float preScale,\n"
"		const float postScale,\n"
"		const float burn,\n"
"		__global float *totalRGB) {\n"
"	const size_t gid = get_global_id(0);\n"
"	const uint pixelCount = filmWidth * filmHeight;\n"
"	if (gid >= pixelCount)\n"
"		return;\n"
"\n"
"	const uint maskValue = channel_FRAMEBUFFER_MASK[gid];\n"
"	if (maskValue) {\n"
"		float Ywa = native_exp(totalRGB[0] / pixelCount);\n"
"\n"
"		// Avoid division by zero\n"
"		if (Ywa == 0.f)\n"
"			Ywa = 1.f;\n"
"\n"
"		const float alpha = .1f;\n"
"		const float invB2 = (burn > 0.f) ? 1.f / (burn * burn) : 1e5f;\n"
"		const float scale = alpha / Ywa;\n"
"		const float preS = scale / preScale;\n"
"		const float postS = scale * postScale;\n"
"\n"
"		__global float *pixel = &channel_IMAGEPIPELINE[gid * 3];\n"
"		float3 pixelValue = VLOAD3F(&channel_IMAGEPIPELINE[gid * 3]);\n"
"\n"
"		const float ys = Spectrum_Y(pixelValue) * preS;\n"
"		// Note: I don't need to convert to XYZ and back because I'm only\n"
"		// scaling the value.\n"
"		pixelValue *= postS * (1.f + ys * invB2) / (1.f + ys);\n"
"\n"
"		VSTORE3F(pixelValue, pixel);\n"
"	}\n"
"}\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// REDUCE_OP & ACCUM_OP (used by tonemap_reduce_funcs.cl)\n"
"//------------------------------------------------------------------------------\n"
"\n"
"float3 REDUCE_OP(const float3 a, const float3 b) {\n"
"	if (isinf(b.s0) || isinf(b.s1) || isinf(b.s2))\n"
"		return a;\n"
"	else {\n"
"		const float y = fmax(Spectrum_Y(b), 1e-6f);\n"
"		return a + native_log(y);\n"
"	}\n"
"}\n"
"\n"
"float3 ACCUM_OP(const float3 a, const float3 b) {\n"
"	return a + b;\n"
"}\n"
; } }
