/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2019 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2019 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2019 Total, S.A
 * Copyright (c) 2019-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

#include "Utils/VectorUtils.hpp"
#include <algorithm>

namespace PAMELA
{

	int vectorUtils::MostOccuringValue(std::vector<int> vec)
	{

		std::sort(std::begin(vec), std::end(vec));
		int curr_freq = 0;
		int max_freq = 0;
		int most_frequent_value = vec.front();
		int last_seen_value = vec.front();

		for (int value : vec)
		{
			if (value == last_seen_value) ++curr_freq;
			else
			{
				if (curr_freq > max_freq)
				{
					max_freq = curr_freq;
					most_frequent_value = last_seen_value;
				}

				last_seen_value = value;
				curr_freq = 1;
			}
		}
		return most_frequent_value;
	}

	std::vector<int> vectorUtils::Vector2VectorMapping(const std::vector<int>& map_vec, const std::vector<int>& source_vec)
	{

		int size = static_cast<int>(map_vec.size());
		std::vector<int> res(size);
		for (auto i = 0; i < size; ++i)
		{
			res[i] = source_vec[map_vec[i]];
		}

		return res;
	}

}

