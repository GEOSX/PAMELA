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
#include "Adjacency/CSRMatrix.hpp"
#include "Mesh/Mesh.hpp"

namespace PAMELA
{

	class Adjacency
	{

		friend class Mesh;

	public:

		Adjacency(ELEMENTS::FAMILY sourceFamily, ELEMENTS::FAMILY targetFamily, ELEMENTS::FAMILY baseFamily, ParallelEnsembleBase* source, ParallelEnsembleBase* target, ParallelEnsembleBase* base, CSRMatrix* csr_mat) :
			m_sourceElementCollection(source), m_targetElementCollection(target), m_baseElementCollection(base),
			m_sourceFamily(sourceFamily), m_targetFamily(targetFamily), m_baseFamily(baseFamily),
			m_adjacencySparseMatrix(csr_mat)
		{
		}

		Adjacency(ELEMENTS::FAMILY sourceFamily, ELEMENTS::FAMILY targetFamily, ELEMENTS::FAMILY baseFamily, ParallelEnsembleBase* source, ParallelEnsembleBase* target, ParallelEnsembleBase* base) 
		: Adjacency(sourceFamily, targetFamily, baseFamily, source, target, base, new CSRMatrix(static_cast<int>(source->size_all()), static_cast<int>(target->size_all())))
		{
		}

		~Adjacency();

		CSRMatrix* get_adjacencySparseMatrix() { return m_adjacencySparseMatrix; }

		//Utils
		static Adjacency* transposed(Adjacency* input);
		static Adjacency* multiply(Adjacency* input_lhs, Adjacency* input_rhs);
		
		ParallelEnsembleBase* get_sourceElementCollection() const { return m_sourceElementCollection; }
		ParallelEnsembleBase* get_targetElementCollection() const { return m_targetElementCollection; }
		ParallelEnsembleBase* get_baseElementCollection() const { return m_baseElementCollection; }

		ELEMENTS::FAMILY get_sourceFamily() const { return m_sourceFamily; }
		ELEMENTS::FAMILY get_targetFamily() const { return m_targetFamily; }
		ELEMENTS::FAMILY get_baseFamily() const { return m_baseFamily; }

		std::pair<std::vector<int>, std::vector<int>> get_SingleElementAdjacency(int i) const;

	private:



		//Components
		ParallelEnsembleBase* m_sourceElementCollection;
		ParallelEnsembleBase* m_targetElementCollection;
		ParallelEnsembleBase* m_baseElementCollection;

		ELEMENTS::FAMILY m_sourceFamily;
		ELEMENTS::FAMILY m_targetFamily;
		ELEMENTS::FAMILY m_baseFamily;

		//data
		CSRMatrix* m_adjacencySparseMatrix;
	};


}










