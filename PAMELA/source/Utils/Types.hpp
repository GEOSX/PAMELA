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



// Std library includes
#include <cstddef>


namespace PAMELA
{

	namespace Types
	{

		// general types
		using int_t = int;
		using uint_t = unsigned int;
		using size_type = std::size_t;

		// for unpartitioned entities, like groups
		using idx_t = unsigned int;

		// for partitioned entities, like cells
		using local_idx_t = unsigned int;
		using global_idx_t = unsigned long long;

		// for partitioning
		using proc_id_t = unsigned int;
		using weight_t = double;

	}
}
