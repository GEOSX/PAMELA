/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2019 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2019 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2019 Total, S.A
 * Copyright (c) 2020-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

#pragma once
#include <vector>
#include <cmath>

namespace PAMELA
{

	struct Coordinates
	{
		Coordinates() :x(0), y(0), z(0) {}
		Coordinates(double xx, double yy, double zz) :x(xx), y(yy), z(zz) {}
		double x, y, z;
	};



	inline int CoinToss(int i0, int i1)
	{
		if (i0 < i1)
			return i0;
		return i1;
	}


	inline std::vector <double> cross_product(const std::vector <double>& vec1, const std::vector <double>& vec2)
	{
		std::vector <double> val(3);
		val[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
		val[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
		val[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
		return val;
	}

	inline double norm(const std::vector <double>& vec)
	{
		return std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	}

}
