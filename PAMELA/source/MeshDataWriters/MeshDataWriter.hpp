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
// Library includes
#include "MeshDataWriters/Part.hpp"
#include "MeshDataWriters/Variable.hpp"
#include "Parallel/Communicator.hpp"
#include "Adjacency/Adjacency.hpp"
#include "Adjacency/CSRGraph.hpp"
#if defined( _WIN32)
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace PAMELA
{

	enum class FAMILY { POLYHEDRON = 3, POLYGON = 2, LINE = 1, POINT = 0, UNKNOWN = -1 };

	class Mesh;


	struct AdjacencyData
	{
		AdjacencyData(std::string lab) :label(lab) {}
		std::string label;
		std::vector<Point> NodesVector;
		std::vector<int> iSource;
		std::vector<int> iTarget;
	};


	class MeshDataWriter
	{
	public:

		template <typename T>
		using PartMap = std::unordered_map<std::string, Part<T>*>;
		virtual ~MeshDataWriter() = default;

		MeshDataWriter(Mesh * mesh, std::string name);

		virtual void Init() = 0;

		void DeclareVariable(FAMILY family, VARIABLE_DIMENSION dim, VARIABLE_LOCATION dloc, std::string name, std::string part);

		void DeclareVariable(FAMILY family, VARIABLE_DIMENSION dim, VARIABLE_LOCATION dloc, std::string name);

		template<class T>
		void SetVariableOnAllParts(std::string label, T univalue)
		{
			for (auto const& part : m_PolyhedronParts)
			{
				if (m_Variable.find(VariableKey(label, part.first)) != m_Variable.end())
				{
					auto var = m_Variable.at(VariableKey(label, part.first));
					var->set_data(univalue);
				}
			}

			for (auto const& part : m_PolygonParts)
			{
				if (m_Variable.find(VariableKey(label, part.first)) != m_Variable.end())
				{
					auto var = m_Variable.at(VariableKey(label, part.first));
					var->set_data(univalue);
				}
			}

			for (auto const& part : m_LineParts)
			{
				if (m_Variable.find(VariableKey(label, part.first)) != m_Variable.end())
				{
					auto var = m_Variable.at(VariableKey(label, part.first));
					var->set_data(univalue);
				}
			}

			for (auto const& part : m_PointParts)
			{
				if (m_Variable.find(VariableKey(label, part.first)) != m_Variable.end())
				{
					auto var = m_Variable.at(VariableKey(label, part.first));
					var->set_data(univalue);
				}
			}

		}

		template<class T>
		void SetVariable(std::string label, std::string part, T univalue)
		{
			auto var = m_Variable.at(VariableKey(label, part));
			var->set_data(univalue);
		}

		template<class T>
		void SetVariable(std::string label, std::string part, ParallelEnsemble<T>& values)
		{
			auto var = m_Variable.at(VariableKey(label, part));
			var->set_data(values.begin_owned(), values.end_owned());
		}

		template<class T>
		void SetVariableOnPolyhedron(std::string label, ParallelEnsemble<T>& values)
		{
                  for( auto polyhedronPartItr = m_PolyhedronParts.begin();
                      polyhedronPartItr != m_PolyhedronParts.end();
                      ++polyhedronPartItr)
                  {
                    auto var = m_Variable.at(VariableKey(label, polyhedronPartItr->first));
                    auto polyhedronPartPtr = polyhedronPartItr->second;
                    auto nbElements = polyhedronPartPtr->Collection->size_owned();
                    std::vector< T > values_in_part(nbElements);
                    for(auto cellBlockItr = polyhedronPartPtr->SubParts.begin();
                        cellBlockItr != polyhedronPartPtr->SubParts.end();
                        cellBlockItr++)
                    {
                      auto cellBlockPtr = cellBlockItr->second;
                      for(auto cellItr = cellBlockPtr->SubCollection.begin_owned();
                          cellItr != cellBlockPtr->SubCollection.end_owned();
                          cellItr++)
                      {
                        auto cellPtr = *(cellItr);
                        auto localIndex = cellPtr->get_localIndex();
                        values_in_part[localIndex] = values[localIndex];
                      }

                      /*
                         auto globalIndex = elemItr->first;
                         auto localIndex = elemItr->second;
                         values_in_part[localIndex]= values[globalIndex];
                         */
                    }
                    var->set_data(values_in_part.begin(), values_in_part.end());
                  }
		}

		void DeclareAndSetPartitionNumber()
		{
			DeclareVariable(
				FAMILY::POLYHEDRON, VARIABLE_DIMENSION::SCALAR,
				VARIABLE_LOCATION::PER_CELL, "Partition");
			DeclareVariable(FAMILY::POLYGON, VARIABLE_DIMENSION::SCALAR,
				VARIABLE_LOCATION::PER_CELL, "Partition");
			DeclareVariable(FAMILY::LINE, VARIABLE_DIMENSION::SCALAR,
				VARIABLE_LOCATION::PER_CELL, "Partition");
			DeclareVariable(FAMILY::POINT, VARIABLE_DIMENSION::SCALAR,
				VARIABLE_LOCATION::PER_CELL, "Partition");
			SetVariableOnAllParts("Partition", Communicator::worldRank());
		}

		void DeclareAndSetElementGlobalIndex()
		{
			//DeclareVariable(FAMILY::POLYHEDRON, VARIABLE_DIMENSION::SCALAR,VARIABLE_LOCATION::PER_CELL, "globalIndex_Polyhedron");
			//DeclareVariable(FAMILY::POLYGON, VARIABLE_DIMENSION::SCALAR,VARIABLE_LOCATION::PER_CELL, "globalIndex_Polygon");
			//DeclareVariable(FAMILY::LINE, VARIABLE_DIMENSION::SCALAR,VARIABLE_LOCATION::PER_CELL, "globalIndex_Line");
			//DeclareVariable(FAMILY::POINT, VARIABLE_DIMENSION::SCALAR,VARIABLE_LOCATION::PER_CELL, "globalIndex_Point");
			//SetElementGlobalIndexOnPart(&m_PointParts);
			//SetElementGlobalIndexOnPart(&m_LineParts);
			//SetElementGlobalIndexOnPart(&m_PolyhedronParts);
			//SetElementGlobalIndexOnPart(&m_PolygonParts);


			//TODO: Need to be fixed as globalIndex cannot be defined on all types of elements
		}

		virtual void Dump() = 0;


	private:

		template<typename T>
		void SetElementGlobalIndexOnPart(PartMap<T>* partMap) {
			for (auto it = partMap->begin(); it != partMap->end(); ++it)
			{
				ParallelEnsemble< double > globalIndex;
				auto partptr = it->second;
				for (auto it2 = partptr->SubParts.begin(); it2 != partptr->SubParts.end(); ++it2)
				{
					if (it2->second->SubCollection.size_owned() > 0)
					{
						auto subpart = it2->second;
						for (auto it3 = subpart->SubCollection.begin_owned(); it3 != subpart->SubCollection.end_owned(); ++it3)
						{
							globalIndex.push_back_owned((*it3)->get_globalIndex());
						}
					}
				}
				SetVariable("globalIndex", it->first, globalIndex);
			}
		}

	protected:

		std::string PartitionNumberForExtension();
		std::string TimeStepNumberForExtension();

		Mesh* m_mesh;

		std::string m_name;

		Types::uint_t m_partition;
		Types::uint_t m_nPartition;

		double m_currentTime;
		int m_currentTimeStep;

		int m_nDigitsExtensionPartition;
		int m_nDigitsExtensionTime;

		//Parts
		PartMap<Point*>   m_PointParts;
		PartMap<Line*>  m_LineParts;
		PartMap<Polygon*>  m_PolygonParts;
		PartMap<Polyhedron*>  m_PolyhedronParts;

		//Property
		std::unordered_map<VariableKey, VariableDouble*, VariableKeyHash> m_Variable;

		//Adjacency
		std::vector<AdjacencyData> m_Adjacency;

	};
}
