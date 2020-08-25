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
// Library includes
#include <unordered_map>
#include "Utils/Assert.hpp"

#if defined( _WIN32)
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace PAMELA
{

	//////VARIABLES
	//Variable dimension
	enum class VARIABLE_DIMENSION { UNKNOWN = -1, SCALAR = 1, VECTOR = 3, TENSOR_SYMM = 6 };
	//Variable type
	enum class VARIABLE_TYPE { UNKNOWN = -1, DOUBLE = 1, INTEGER = 2 };
	//Variable locations
	enum class VARIABLE_LOCATION { UNKNOWN = -1, PER_NODE = 1, PER_CELL = 2 };
	//Variable size
	const std::unordered_map<int, int> VariableDimensionToSize =
	{
		{ static_cast<int>(VARIABLE_DIMENSION::SCALAR), 1 },
		{ static_cast<int>(VARIABLE_DIMENSION::VECTOR), 3 },
		{ static_cast<int>(VARIABLE_DIMENSION::TENSOR_SYMM), 6 },
	};



	template<class T>
	struct Variable
	{
		Variable(VARIABLE_DIMENSION dim, VARIABLE_TYPE type, std::string label, size_t size) : Label(label), Dimension(dim), Type(type)
		{
			offset = VariableDimensionToSize.at(static_cast<int>(dim));
			Data = std::vector<T>(size*offset);
		}

		std::string Label;
		size_t offset;
		VARIABLE_DIMENSION Dimension;
		VARIABLE_TYPE Type;

		size_t size() { return Data.size(); }

		void set_data(T cst)
		{
			std::fill(Data.begin(), Data.end(), cst);
		}

		void set_data(typename std::vector<T>::iterator it_begin_vec, typename std::vector<T>::iterator it_end_vec)
		{
			ASSERT(static_cast<size_t>(it_end_vec - it_begin_vec) == Data.size(), "Mismatch sizes");
			Data.assign(it_begin_vec, it_end_vec);
		}


		std::vector<T> get_data(int i)
		{
			if (offset == 1)
				return{ Data[i] };
			std::vector<T> vec(&Data[i*offset], &Data[(i + 1)*offset]);
			return vec;
		}

	private:
		std::vector<T> Data;

	};


	struct VariableDouble : public Variable<double>
	{
		VariableDouble(VARIABLE_DIMENSION dim, std::string label, size_t size) :Variable<double>(dim, VARIABLE_TYPE::DOUBLE, label, size) {}
	};


	struct VariableInt : public Variable<int>
	{
		VariableInt(VARIABLE_DIMENSION dim, std::string label, size_t size) :Variable<int>(dim, VARIABLE_TYPE::INTEGER, label, size) {}
	};





	struct VariableKey
	{
		std::string first;
		std::string second;

		VariableKey(std::string First, std::string Second)
		{
			this->first = First;
			this->second = Second;
		}

		bool operator==(const VariableKey &other) const
		{
			return (first == other.first
				&& second == other.second);
		}
	};

	struct VariableKeyHash
	{
		std::size_t operator()(const VariableKey& k) const
		{
			auto hfirst = std::hash<std::string>()(k.first);
			auto hsecond = std::hash<std::string>()(k.second);
			return hfirst ^ hsecond;
		}
	};

}
