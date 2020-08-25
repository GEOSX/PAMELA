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

// Std library includes
#include <type_traits>
#include <unordered_map>

namespace PAMELA
{
	struct EnumHash
	{
		template <typename T>
		std::size_t operator()(T t) const
		{
			return static_cast<std::size_t>(t);
		}
	};

	template <typename Key>
	using HashType = typename std::conditional<std::is_enum<Key>::value, EnumHash, std::hash<Key>>::type;

	template <typename Key, typename T>
	using HashMap = std::unordered_map<Key, T, HashType<Key>>;

	template <typename Key, typename T>
	using HashMultiMap = std::unordered_multimap<Key, T, HashType<Key>>;
}
