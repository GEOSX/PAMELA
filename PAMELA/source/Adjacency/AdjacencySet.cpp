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

#include "Adjacency/AdjacencySet.hpp"
#include "Utils/Logger.hpp"
#include "Elements/Polyhedron.hpp"
#include "Adjacency/Adjacency.hpp"

namespace PAMELA
{
	

	Adjacency* AdjacencySet::get_TopologicalAdjacency(ELEMENTS::FAMILY source, ELEMENTS::FAMILY target, ELEMENTS::FAMILY base)
	{
		if (adjacencyExist(source, target, base) != nullptr)
		{
			return adjacencyExist(source, target, base);
		}

		// SOURCE = POLYHEDRON ; TARGET = POINT ; BASE = POLYHEDRON
		if ((source == ELEMENTS::FAMILY::POLYHEDRON) && (target == ELEMENTS::FAMILY::POINT) && (base == ELEMENTS::FAMILY::POLYHEDRON))
		{
			return get_TopologicalAdjacency(m_mesh->get_PolyhedronCollection(), m_mesh->get_PointCollection(), m_mesh->get_PolyhedronCollection());
		}

		// SOURCE = POINT ; TARGET = POLYHEDRON ; BASE = POLYHEDRON
		if ((source == ELEMENTS::FAMILY::POINT) && (target == ELEMENTS::FAMILY::POLYHEDRON) && (base == ELEMENTS::FAMILY::POLYHEDRON))
		{
			return get_TopologicalAdjacency(m_mesh->get_PointCollection(), m_mesh->get_PolyhedronCollection(), m_mesh->get_PolyhedronCollection());
		}


		// SOURCE = POLYHEDRON ; TARGET = POLYGON ; BASE = POLYHEDRON
		if ((source == ELEMENTS::FAMILY::POLYHEDRON) && (target == ELEMENTS::FAMILY::POLYGON) && (base == ELEMENTS::FAMILY::POLYHEDRON))
		{
			return get_TopologicalAdjacency(m_mesh->get_PolyhedronCollection(), m_mesh->get_PolygonCollection(), m_mesh->get_PolyhedronCollection());
		}

		// SOURCE = POLYGON ; TARGET = POLYHEDRON ; BASE = POLYHEDRON
		if ((source == ELEMENTS::FAMILY::POLYGON) && (target == ELEMENTS::FAMILY::POLYHEDRON) && (base == ELEMENTS::FAMILY::POLYHEDRON))
		{
			return get_TopologicalAdjacency(m_mesh->get_PolygonCollection(), m_mesh->get_PolyhedronCollection(), m_mesh->get_PolyhedronCollection());
		}

		// SOURCE = POLYHEDRON ; TARGET = POLYHEDRON ; BASE = POLYGON
		if ((source == ELEMENTS::FAMILY::POLYHEDRON) && (target == ELEMENTS::FAMILY::POLYHEDRON) && (base == ELEMENTS::FAMILY::POLYGON))
		{
			return get_TopologicalAdjacency(m_mesh->get_PolyhedronCollection(), m_mesh->get_PolyhedronCollection(), m_mesh->get_PolygonCollection());
		}

		LOGERROR("Adjacency not implemented yet");
		return nullptr;


	}

	void AdjacencySet::ClearAfterPartitioning(std::set<int>& PolyhedronOwned, std::set<int>& PolyheronGhost,std::set<int>& PolygonOwned, std::set<int>& PolygonGhost)
	{

                utils::pamela_unused(PolyhedronOwned);
                utils::pamela_unused(PolyheronGhost);
		//Topological
		auto adjacency = get_TopologicalAdjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYHEDRON);
		auto new_adjacency = ClearAfterPartitioning_Topological(adjacency, PolygonOwned, PolygonGhost);
		delete adjacency;
		TopologicalAdjacencyMap.clear();
		TopologicalAdjacencyMap[std::make_tuple(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYHEDRON)] = new_adjacency;

		//Others

		/*for (auto it = NonTopologicalAdjacencyMap.begin();it!= NonTopologicalAdjacencyMap.end();++it)
		{
			auto adj = it->second;
			auto label = it->first;
			if((adj->get_sourceFamily()==ELEMENTS::FAMILY::POLYHEDRON)&& (adj->get_targetFamily() == ELEMENTS::FAMILY::POLYHEDRON)&& (adj->get_baseFamily() == ELEMENTS::FAMILY::UNKNOWN))
			{
				ClearAfterPartitioning_NonTopological(adj, PolyhedronOwned, PolyheronGhost);
				NonTopologicalAdjacencyMap[label] = adj;
			}
		}*/

	}



	Adjacency* AdjacencySet::ClearAfterPartitioning_Topological(Adjacency* adjacency, std::set<int>& PolygonOwned, std::set<int>& PolygonGhost)
	{
		auto polyhedra = static_cast<PolyhedronCollection*>(adjacency->get_sourceElementCollection());
		auto polygons = static_cast<PolygonCollection*>(adjacency->get_targetElementCollection());
		auto csr_matrix = adjacency->get_adjacencySparseMatrix();

		auto& columIndex = csr_matrix->columnIndex;
		auto& rowPtr = csr_matrix->rowPtr;
		auto& val = csr_matrix->values;

		auto new_csr_matrix = new CSRMatrix;
		auto& new_nnz = new_csr_matrix->nnz = 0;
		auto& new_columIndex = new_csr_matrix->columnIndex;
		auto& new_rowPtr = new_csr_matrix->rowPtr;
		auto& new_val = new_csr_matrix->values;

		for (auto it = polyhedra->begin(); it != polyhedra->end(); ++it)
		{
			auto polyhedron_global_index = (*it)->get_globalIndex();
			auto polyhedron_local_index = (*it)->get_localIndex();
			auto i0 = rowPtr[polyhedron_global_index];
			auto i1 = rowPtr[polyhedron_global_index + 1];
			std::vector<int> sub_columnIndex(columIndex.begin() + i0, columIndex.begin() + i1);
			std::vector<int> sub_val(val.begin() + i0, val.begin() + i1);
			std::vector<int> temp;
			for (size_t i = 0; i != sub_columnIndex.size(); ++i)
			{
				auto polygon_global_index = sub_columnIndex[i];

				if ((PolygonOwned.count(polygon_global_index) == 1) || (PolygonGhost.count(polygon_global_index) == 1))	//face is in the partition
				{
					auto polygon_local_index = polygons->get_GlobalToLocalIndex().at(polygon_global_index);
					temp.push_back(polygon_local_index);
					new_val.push_back(polyhedron_local_index);
					++new_nnz;
				}
			}
			std::sort(temp.begin(), temp.end());
			new_columIndex.insert(new_columIndex.end(), temp.begin(), temp.end());
			new_rowPtr.push_back(new_nnz);
		}
		return new Adjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYHEDRON, polyhedra, polyhedra, polygons, new_csr_matrix);
	}


	Adjacency* AdjacencySet::ClearAfterPartitioning_NonTopological(Adjacency* adjacency, std::set<int>& Polyhedron_owned, std::set<int>& Polyhedron_ghost)
	{
		auto polyhedra = static_cast<PolyhedronCollection*>(adjacency->get_sourceElementCollection());
		auto csr_matrix = adjacency->get_adjacencySparseMatrix();

		auto& columIndex = csr_matrix->columnIndex;
		auto& rowPtr = csr_matrix->rowPtr;
		auto& val = csr_matrix->values;

		auto new_csr_matrix = new CSRMatrix;
		auto& new_nnz = new_csr_matrix->nnz = 0;
		auto& new_columIndex = new_csr_matrix->columnIndex;
		auto& new_rowPtr = new_csr_matrix->rowPtr;
		auto& new_val = new_csr_matrix->values;

		for (auto it = polyhedra->begin(); it != polyhedra->end(); ++it)
		{
			auto polyhedron_global_index = (*it)->get_globalIndex();
			auto polyhedron_local_index = (*it)->get_localIndex();
			auto i0 = rowPtr[polyhedron_global_index];
			auto i1 = rowPtr[polyhedron_global_index + 1];
			std::vector<int> sub_columnIndex(columIndex.begin() + i0, columIndex.begin() + i1);
			std::vector<int> sub_val(val.begin() + i0, val.begin() + i1);
			std::vector<int> temp;
			for (size_t i = 0; i != sub_columnIndex.size(); ++i)
			{
				auto polyhedron2_global_index = sub_columnIndex[i];

				if ((Polyhedron_owned.count(polyhedron2_global_index) == 1) || (Polyhedron_ghost.count(polyhedron2_global_index) == 1))	//face is in the partition
				{
					auto polyhedron2_local_index = polyhedra->get_GlobalToLocalIndex().at(polyhedron2_global_index);
					temp.push_back(polyhedron2_local_index);
					new_val.push_back(polyhedron_local_index);
					++new_nnz;
				}
			}
			std::sort(temp.begin(), temp.end());
			new_columIndex.insert(new_columIndex.end(), temp.begin(), temp.end());
			new_rowPtr.push_back(new_nnz);
		}
		return new Adjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYHEDRON, polyhedra, polyhedra, polyhedra, new_csr_matrix);
	}

	void AdjacencySet::Add_NonTopologicalAdjacencySum(std::string label, std::vector<Adjacency*> sumAdj)
	{
		auto adjacency1 = sumAdj[0];
		auto adjacency2 = sumAdj[1];  //TODO: this is special case

		auto csr1 = adjacency1->get_adjacencySparseMatrix();
		auto csr2 = adjacency2->get_adjacencySparseMatrix();
		auto csr_sum = CSRMatrix::sum(csr1, csr2);
		auto adj1_sourceFamily = adjacency1->get_sourceFamily();
		auto adj1_targetFamily = adjacency1->get_targetFamily();
		auto adj1_baseFamily = adjacency1->get_baseFamily();
		auto adj1_source = adjacency1->get_sourceElementCollection();
		auto adj1_target = adjacency1->get_targetElementCollection();
		auto adj1_base = adjacency1->get_baseElementCollection();
		ASSERT(adj1_sourceFamily == adjacency2->get_sourceFamily(), "Non matching source elements");
		ASSERT(adj1_baseFamily == adjacency2->get_baseFamily(), "Non matching base elements");
		ASSERT(adj1_targetFamily == adjacency2->get_targetFamily(), "Non matching target elements");
		auto adjacency_sum = new Adjacency(adj1_sourceFamily, adj1_targetFamily, adj1_baseFamily, adj1_source, adj1_target, adj1_base, csr_sum);
		NonTopologicalAdjacencyMap[label] = adjacency_sum;
	}


	// Polyhedra to Points adjacency 
	Adjacency* AdjacencySet::get_TopologicalAdjacency(PolyhedronCollection* source, PointCollection* target, PolyhedronCollection* base)
	{

		Adjacency* adj = new Adjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POINT, ELEMENTS::FAMILY::POLYHEDRON, source, target, base);
		auto collectionSize = source->size_all();
		int nbVertex = 0;
		int vertexIndex = 0;
		int PolyhedronIndex = 0;
		int nval = 0;
		int nrow = 0;

		auto csr_mat = adj->get_adjacencySparseMatrix();

		csr_mat->rowPtr[0];
		for (size_t i = 0; i != collectionSize; i++)
		{
			Polyhedron* polyhedron = source->operator[](static_cast<int>(i));
			PolyhedronIndex = polyhedron->get_localIndex();
			const std::vector<Point*>& vertexList = polyhedron->get_vertexList();
			nbVertex = static_cast<int>(vertexList.size());
			for (auto j = 0; j < nbVertex; j++)
			{
				vertexIndex = vertexList[j]->get_localIndex();
				csr_mat->columnIndex.push_back(vertexIndex);
				csr_mat->values.push_back(PolyhedronIndex);
				nval++;
			}
			nrow++;
			csr_mat->rowPtr[nrow] = nval;
		}
		csr_mat->nnz = nval;
		csr_mat->sortRowIndexAndMoveValues();
		csr_mat->checkMatrix();

		//Add to map
		TopologicalAdjacencyMap[std::make_tuple(source->get_family(), target->get_family(), base->get_family())] = adj;

		return adj;
	}

	// Points to Polyhedra adjacency 
	Adjacency* AdjacencySet::get_TopologicalAdjacency(PointCollection* source, PolyhedronCollection* target, PolyhedronCollection* base)
	{
                utils::pamela_unused(source);
                utils::pamela_unused(target);
                utils::pamela_unused(base);
		Adjacency* adj = Adjacency::transposed(get_TopologicalAdjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POINT, ELEMENTS::FAMILY::POLYHEDRON));
		return adj;
	}


	// Polyhedra to Face adjacency
	Adjacency* AdjacencySet::get_TopologicalAdjacency(PolyhedronCollection* source, PolygonCollection* target, PolyhedronCollection* base)
	{

                utils::pamela_unused(source);
                utils::pamela_unused(target);
                utils::pamela_unused(base);
		LOGERROR("This adjacency must be created while creating Polygons from Polyhedra");
		return nullptr;
	}

	Adjacency* AdjacencySet::get_TopologicalAdjacency(PolygonCollection* source, PolyhedronCollection* target, PolyhedronCollection* base)
	{

                utils::pamela_unused(source);
                utils::pamela_unused(target);
                utils::pamela_unused(base);
		Adjacency* adj = Adjacency::transposed(get_TopologicalAdjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYHEDRON));
		return adj;
	}



	Adjacency* AdjacencySet::get_TopologicalAdjacency(PolyhedronCollection* source, PolyhedronCollection* target, PolygonCollection* base)
	{
                utils::pamela_unused(source);
                utils::pamela_unused(target);
                utils::pamela_unused(base);
		Adjacency* adj1 = get_TopologicalAdjacency(ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYHEDRON);
		Adjacency* adj2 = get_TopologicalAdjacency(ELEMENTS::FAMILY::POLYGON, ELEMENTS::FAMILY::POLYHEDRON, ELEMENTS::FAMILY::POLYHEDRON);
		Adjacency* adj = Adjacency::multiply(adj1, adj2);
		return adj;
	}


	Adjacency* AdjacencySet::adjacencyExist(ELEMENTS::FAMILY source, ELEMENTS::FAMILY target, ELEMENTS::FAMILY base)
	{
		familyTriplet tri = std::make_tuple(source, target, base);
		if (TopologicalAdjacencyMap.find(tri) != TopologicalAdjacencyMap.end())
		{
			return TopologicalAdjacencyMap.at(tri);
		}
		return nullptr;
	}


}
