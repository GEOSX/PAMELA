/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2020 Total, S.A
 * Copyright (c) 2020-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

#pragma once

// Project includes
#include "Utils/Utils.hpp"

// Std library includes
#include <vector>
#include <algorithm>

namespace PAMELA
{

	template <typename SizeType, typename IndexType, typename DataType>
	class CSRGraph
	{
	public:

		using size_type = SizeType;
		using index_type = IndexType;
		using data_type = DataType;

		using row_start_vec = std::vector<size_type>;
		using adj_index_vec = std::vector<index_type>;
		using adj_data_vec = std::vector<data_type>;

		CSRGraph();

		CSRGraph(size_type nvert, size_type nedge);

		template<typename SizeType2, typename IndexType2, typename DataType2>
		explicit CSRGraph(const CSRGraph<SizeType2, IndexType2, DataType2>& other);

		virtual ~CSRGraph();

		void resize(size_type nvert, size_type nedge);

		size_type getNumNodes() const { return m_row_start.size() - 1; }
		size_type getNumEdges() const { return m_adj_index.size(); }

		row_start_vec& getRowStart() { return m_row_start; }
		adj_index_vec& getAdjIndex() { return m_adj_index; }
		adj_data_vec&  getAdjData() { return m_adj_data; }

		const row_start_vec& getRowStart() const { return m_row_start; }
		const adj_index_vec& getAdjIndex() const { return m_adj_index; }
		const adj_data_vec&  getAdjData()  const { return m_adj_data; }

	private:

		row_start_vec m_row_start;
		adj_index_vec m_adj_index;
		adj_data_vec  m_adj_data;

	};

	template <typename SizeType, typename IndexType, typename DataType>
	CSRGraph<SizeType, IndexType, DataType>::CSRGraph()
	{
	}

	template <typename SizeType, typename IndexType, typename DataType>
	CSRGraph<SizeType, IndexType, DataType>::CSRGraph(size_type nvert, size_type nedge)
	{
		resize(nvert, nedge);
	}

	template <typename SizeType, typename IndexType, typename DataType>
	template<typename SizeType2, typename IndexType2, typename DataType2>
	CSRGraph<SizeType, IndexType, DataType>::CSRGraph(const CSRGraph<SizeType2, IndexType2, DataType2>& other)
	{
		m_row_start.resize(other.getNumNodes() + 1);
		m_adj_index.resize(other.getNumEdges());
		m_adj_data.resize(other.getNumEdges());

		utils::copy(other.getRowStart().begin(), other.getRowStart().end(), m_row_start.begin());
		utils::copy(other.getAdjIndex().begin(), other.getAdjIndex().end(), m_adj_index.begin());
		utils::copy(other.getAdjData().begin(), other.getAdjData().end(), m_adj_data.begin());
	}

	template <typename SizeType, typename IndexType, typename DataType>
	CSRGraph<SizeType, IndexType, DataType>::~CSRGraph()
	{
	}

	template <typename SizeType, typename IndexType, typename DataType>
	void CSRGraph<SizeType, IndexType, DataType>::resize(size_type nvert, size_type nedge)
	{
		m_row_start.resize(nvert + 1);
		m_adj_index.resize(nedge);
		m_adj_data.resize(nedge);
	}

}

