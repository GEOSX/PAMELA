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
#include "MeshDataWriters/MeshDataWriter.hpp"
#include "Mesh/Mesh.hpp"
#include <iterator> 

#if defined( _WIN32)
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace PAMELA
{

	//TODO variables in parallel, vector and tensors data

	enum class ENSIGHT_GOLD_TYPE { UNKNOWN = -1, ESG_POINT = 1, ESG_BAR2 = 2, ESG_TRIA3 = 4, ENS_QUAD4 = 6, ENS_TETRA4 = 8, ENS_PYRAMID5 = 10, ENS_PENTA6 = 12, ENS_HEXA8 = 14 };

	class EnsightGoldWriter : public MeshDataWriter
	{

	public:

		EnsightGoldWriter(Mesh * mesh, std::string name) : MeshDataWriter(mesh, name) {}

		virtual void Init() final;
		virtual void Dump() final;

	private:

		void MakeCaseFile();
		void MakeGeoFile();

		//Geo
		void MakeGeoFile_Header();

		template<typename T>
		void MakeGeoFile_AddParts(const PartMap<T>* parts);

		template<typename T>
		void DumpVariables_Parts(const PartMap<T>* parts);

		const std::unordered_map<int, std::string> ElementToLabel
			=
		{
			{ static_cast<int>(ELEMENTS::TYPE::VTK_VERTEX), "point" },
			{ static_cast<int>(ELEMENTS::TYPE::VTK_LINE),"bar2" },
			{ static_cast<int>(ELEMENTS::TYPE::VTK_TRIANGLE),"tria3" },
			{ static_cast<int>(ELEMENTS::TYPE::VTK_QUAD) ,"quad4" },
			{ static_cast<int>(ELEMENTS::TYPE::VTK_TETRA),"tetra4" },
			{ static_cast<int>(ELEMENTS::TYPE::VTK_HEXAHEDRON) ,"hexa8" },
			{ static_cast<int>(ELEMENTS::TYPE::VTK_WEDGE),"penta6" },
			{ static_cast<int>(ELEMENTS::TYPE::VTK_PYRAMID),"pyramid5" }
		};


		const std::unordered_map<int, FAMILY> TypeToFamily =
		{
			{ static_cast<int>(ENSIGHT_GOLD_TYPE::ESG_POINT),FAMILY::POINT },
			{ static_cast<int>(ENSIGHT_GOLD_TYPE::ESG_BAR2),FAMILY::LINE },
			{ static_cast<int>(ENSIGHT_GOLD_TYPE::ESG_TRIA3),FAMILY::POLYGON },
			{ static_cast<int>(ENSIGHT_GOLD_TYPE::ENS_QUAD4),FAMILY::POLYGON },
			{ static_cast<int>(ENSIGHT_GOLD_TYPE::ENS_TETRA4),FAMILY::POLYHEDRON },
			{ static_cast<int>(ENSIGHT_GOLD_TYPE::ENS_HEXA8),FAMILY::POLYHEDRON },
			{ static_cast<int>(ENSIGHT_GOLD_TYPE::ENS_PENTA6),FAMILY::POLYHEDRON },
			{ static_cast<int>(ENSIGHT_GOLD_TYPE::ENS_PYRAMID5),FAMILY::POLYHEDRON }
		};

		enum class ENSIGHT_GOLD_VARIABLE_DIMENSION { UNKNOWN = -1, SCALAR = 1, VECTOR = 3, TENSOR_SYMM = 6 };
		enum class ENSIGHT_GOLD_VARIABLE_LOCATION { UNKNOWN = -1, PER_NODE = 1, PER_CELL = 2 };

		std::ofstream m_caseFile;
		std::ofstream m_geoFile;


	};



	/**
	 * \brief
	 * \tparam T
	 * \param ipart
	 * \param parts
	 * \param prefixLabel
	 */
	template <typename T>
	void EnsightGoldWriter::MakeGeoFile_AddParts(const PartMap<T>* parts)
	{

		for (auto it = parts->begin(); it != parts->end(); ++it)			//Loop over Parts that are defined from the mesh input
		{

			auto partptr = it->second;

			m_geoFile << "part" << std::endl;
			m_geoFile << std::setw(10);
			m_geoFile << partptr->Index << std::endl;
			m_geoFile << partptr->Label << std::endl;

			//Coordinates
			m_geoFile << "coordinates" << std::endl;
			m_geoFile << std::setw(10);
			m_geoFile << partptr->Points.size() << std::endl;


			//--Id
			for (auto it2 = partptr->Points.begin(); it2 != partptr->Points.end(); ++it2)
			{
				m_geoFile << std::setw(10);
				m_geoFile << (*it2)->get_globalIndex() << std::endl;
			}

			//--x
			for (auto it2 = partptr->Points.begin(); it2 != partptr->Points.end(); ++it2)		//Loop over Points 
			{
				m_geoFile << std::setw(12);
				m_geoFile << (*it2)->get_coordinates().x << std::endl;
			}

			//--y
			for (auto it2 = partptr->Points.begin(); it2 != partptr->Points.end(); ++it2)		//Loop over Points 
			{
				m_geoFile << std::setw(12);
				m_geoFile << (*it2)->get_coordinates().y << std::endl;
			}

			//--z
			for (auto it2 = partptr->Points.begin(); it2 != partptr->Points.end(); ++it2)		//Loop over Points 
			{
				m_geoFile << std::setw(12);
				m_geoFile << (*it2)->get_coordinates().z << std::endl;
			}

			//Elements

			for (auto it2 = partptr->SubParts.begin(); it2 != partptr->SubParts.end(); ++it2)
			{
				if (it2->second->SubCollection.size_owned() > 0)
				{
					auto subpart = it2->second;
					auto elementType = subpart->ElementType;
					std::string ENSTypeLabel = ElementToLabel.at(static_cast<int>(elementType));
					m_geoFile << ENSTypeLabel << std::endl;
					m_geoFile << std::setw(10);
					//----dimension
					m_geoFile << subpart->SubCollection.size_owned() << std::endl;
					//----Id
					for (auto it3 = subpart->SubCollection.begin_owned(); it3 != subpart->SubCollection.end_owned(); ++it3)
					{
						m_geoFile << std::setw(10) << (*it3)->get_globalIndex() << std::endl;
					}
					//----Connectivity
					for (auto it3 = subpart->SubCollection.begin_owned(); it3 != subpart->SubCollection.end_owned(); ++it3)
					{
						auto VertexList = (*it3)->get_vertexList();
						auto nVertex = VertexList.size();
						for (unsigned int j = 0; j != nVertex; ++j)
						{
							m_geoFile << std::setw(10) << partptr->GlobalToLocalPointMapping.at(VertexList[j]->get_globalIndex()) + 1;
						}
						m_geoFile << std::endl;
					}
				}
			}
		}

	}

	template <typename T>
	void EnsightGoldWriter::DumpVariables_Parts(const PartMap<T>* parts)
	{

		//Polyhedron
		for (auto it = parts->begin(); it != parts->end(); ++it)			//Loop over Parts that are defined from the mesh input
		{
			auto partptr = it->second;


			//PerElementVariables
			for (auto it2 = partptr->PerElementVariable.begin(); it2 != partptr->PerElementVariable.end(); ++it2)
			{

				auto variableptr = (*it2);

				//Create file
				std::ofstream variableFile;
				variableFile.open(m_name + "_" + partptr->Label + "_" + variableptr->Label + "_" + TimeStepNumberForExtension(), std::fstream::in | std::fstream::out | std::fstream::trunc);

				//write
				variableFile << variableptr->Label << std::endl;
				variableFile << "part" << std::endl;
				variableFile << std::setw(10);
				variableFile << partptr->Index << std::endl;


				for (auto it3 = partptr->SubParts.begin(); it3 != partptr->SubParts.end(); ++it3)
				{
					if (it3->second->SubCollection.size_owned() > 0)
					{
						auto subpart = it3->second;
						auto elementType = subpart->ElementType;
						std::string ENSTypeLabel = ElementToLabel.at(static_cast<int>(elementType));
						variableFile << ENSTypeLabel << std::endl;
						variableFile << std::setw(10);
						for (auto it4 = subpart->SubCollection.begin_owned(); it4 != subpart->SubCollection.end_owned(); ++it4)
						{
							auto collectionIndex = it4 - subpart->SubCollection.begin_owned();
							auto variableIndex = subpart->IndexMapping[collectionIndex];
							auto variableData = variableptr->get_data(variableIndex);
							for (auto it5 = variableData.begin(); it5 != variableData.end(); ++it5)
							{
								variableFile << std::setw(12) << (*it5) << std::endl;
							}
						}

					}
				}

			}

			//PerNodeVariables
			for (auto it2 = partptr->PerNodeVariable.begin(); it2 != partptr->PerNodeVariable.end(); ++it2)
			{

				auto variableptr = (*it2);

				//Create file
				std::ofstream variableFile;
				variableFile.open("./" + partptr->Label + "_" + variableptr->Label + "_" + TimeStepNumberForExtension(), std::fstream::in | std::fstream::out | std::fstream::trunc);

				//write
				variableFile << variableptr->Label << std::endl;
				variableFile << "part" << std::endl;
				variableFile << std::setw(10);
				variableFile << partptr->Index << std::endl;
				variableFile << "coordinates" << std::endl;
				for (auto it3 = partptr->Points.begin(); it3 != partptr->Points.end(); ++it3)
				{

					//TODO does not work
					auto collectionIndex = std::distance(partptr->Points.begin(), it3);
					auto variableData = variableptr->get_data(static_cast<int>(collectionIndex));
					for (auto it5 = variableData.begin(); it5 != variableData.end(); ++it5)
					{
						variableFile << std::setw(12) << (*it5) << std::endl;
					}
					////y
					//for (auto it5 = variableData.begin(); it5 != variableData.end(); ++it5)
					//{
					//	variableFile << std::setw(12) << (*it5) << std::endl;
					//}
					////z
					//	for (auto it5 = variableData.begin(); it5 != variableData.end(); ++it5)
					//{
					//	variableFile << std::setw(12) << (*it5) << std::endl;
					//}
				}


				/*for (auto it3 = partptr->SubParts.begin(); it3 != partptr->SubParts.end(); ++it3)
				{
					if (it3->second->SubCollection.size_owned() > 0)
					{
						auto subpart = it3->second;
						auto elementType = subpart->ElementType;
						std::string ENSTypeLabel = EnsightGold::ElementToLabel.at(elementType);
						variableFile << ENSTypeLabel << std::endl;
						variableFile << std::setw(10);
						for (auto it4 = subpart->SubCollection.begin_owned(); it4 != subpart->SubCollection.end_owned(); ++it4)
						{
							auto collectionIndex = it4 - subpart->SubCollection.begin_owned();
							auto variableIndex = subpart->IndexMapping[collectionIndex];
							auto variableData = variableptr->get_data(static_cast<int>(collectionIndex));
							for (auto it5 = variableData.begin(); it5 != variableData.end(); ++it5)
							{
								variableFile << std::setw(12) << (*it5) << std::endl;
							}

						}

					}
				}*/

			}

		}

	}
}


