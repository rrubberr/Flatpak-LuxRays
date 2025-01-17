#include <string>
namespace slg { namespace ocl {
std::string KernelSource_tonemap_luxlinear_funcs = 
"#line 2 \"tonemap_luxlinear_funcs.cl\"\n"
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
"// LuxLinearToneMap_Apply\n"
"//------------------------------------------------------------------------------\n"
"\n"
"__kernel __attribute__((work_group_size_hint(256, 1, 1))) void LuxLinearToneMap_Apply(\n"
"		const uint filmWidth, const uint filmHeight,\n"
"		__global float *channel_IMAGEPIPELINE,\n"
"		__global uint *channel_FRAMEBUFFER_MASK,\n"
"		const float scale) {\n"
"	const size_t gid = get_global_id(0);\n"
"	if (gid >= filmWidth * filmHeight)\n"
"		return;\n"
"\n"
"	const uint maskValue = channel_FRAMEBUFFER_MASK[gid];\n"
"	if (maskValue) {\n"
"		__global float *pixel = &channel_IMAGEPIPELINE[gid * 3];\n"
"\n"
"		pixel[0] *= scale;\n"
"		pixel[1] *= scale;\n"
"		pixel[2] *= scale;\n"
"	}\n"
"}\n"
; } }
