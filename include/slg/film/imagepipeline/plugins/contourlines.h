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

#ifndef _SLG_CONTOURLINES_PLUGIN_H
#define	_SLG_CONTOURLINES_PLUGIN_H

#include <vector>
#include <memory>
#include <typeinfo> 
#include <boost/serialization/version.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/vector.hpp>

#include "eos/portable_oarchive.hpp"
#include "eos/portable_iarchive.hpp"

#include "luxrays/luxrays.h"
#include "luxrays/core/color/color.h"
#include "slg/film/imagepipeline/imagepipeline.h"

namespace slg {

//------------------------------------------------------------------------------
// Contour lines plugin
//------------------------------------------------------------------------------

class ContourLinesPlugin : public ImagePipelinePlugin {
public:
	ContourLinesPlugin(const float scale, const float range, const u_int steps,
			const int zeroGridSize);
	virtual ~ContourLinesPlugin() { }

	virtual ImagePipelinePlugin *Copy() const;

	virtual void Apply(Film &film, const u_int index);

	friend class boost::serialization::access;

	float scale, range;
	u_int steps;
	int zeroGridSize;

private:
	// Used by Copy() and serialization
	ContourLinesPlugin() { }

	template<class Archive> void serialize(Archive &ar, const u_int version) {
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ImagePipelinePlugin);
		ar & scale;
		ar & range;
		ar & steps;
		ar & zeroGridSize;
	}

	float GetLuminance(const Film &film, const u_int x, const u_int y) const;
	int GetStep(const Film &film, const int x, const int y, const int defaultValue,
			float *normalizedValue = NULL) const;
};

}

BOOST_CLASS_VERSION(slg::ContourLinesPlugin, 1)

BOOST_CLASS_EXPORT_KEY(slg::ContourLinesPlugin)

#endif	/*  _SLG_CONTOURLINES_PLUGIN_H */
