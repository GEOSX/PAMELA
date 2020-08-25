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

#pragma once

#include <cmath>

// Project includes
#include "Utils/Types.hpp"

// Std library includes
#include <algorithm>
#include <utility>
#include <string>
#include <sstream>
#include <array>

namespace PAMELA
{

    namespace utils
    {
      template < typename T >
      void pamela_unused( const T& /*unused*/ )
      {
      }

        template< typename T>
            bool nearlyEqual(T a, T b, T epsilon = 1e-6) {
                return std::fabs(a - b) < epsilon;
            }

		/* CUSTOM COPY FUNCTIONS */
		// Call std::copy when iterator value type is the same for input and output and std::transform otherwise

		template <typename InputIt, typename OutputIt>
		OutputIt copy(InputIt first, InputIt last, OutputIt dest,
			typename std::enable_if<std::is_same<typename std::iterator_traits<InputIt>::value_type,
			typename std::iterator_traits<OutputIt>::value_type>::value>::type* = nullptr)
		{
			return std::copy(first, last, dest);
		}

		template <typename InputIt, typename OutputIt>
		OutputIt copy(InputIt first, InputIt last, OutputIt dest,
			typename std::enable_if<!std::is_same<typename std::iterator_traits<InputIt>::value_type,
			typename std::iterator_traits<OutputIt>::value_type>::value>::type* = nullptr)
		{
			using input_type = typename std::iterator_traits<InputIt>::value_type;
			using output_type = typename std::iterator_traits<OutputIt>::value_type;
			return std::transform(first, last, dest, [](const input_type& a) { return static_cast<output_type>(a); });
		}
	}

}
