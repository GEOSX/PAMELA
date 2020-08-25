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

#include "MeshDataWriters/MeshDataWriter.hpp"
#include "Mesh/Mesh.hpp"
#include "MeshDataWriters/MeshParts.hpp"
#include "Elements/ElementFactory.hpp"

namespace PAMELA
{
	MeshDataWriter::MeshDataWriter(Mesh * mesh, std::string name) : m_mesh(mesh), m_name(name), m_partition(Communicator::worldRank()), m_nPartition(Communicator::worldSize())
	{
		LOGINFO("***Mesh Data Writer Construction");

		//////Misc
		//Number of digits for file extension
		m_nDigitsExtensionPartition = 5;
		m_nDigitsExtensionTime = 5;

		//Time
		m_currentTime = 0.0;
		m_currentTimeStep = 0;

		//////Initialize Parts

		//Part Index
		int partIndex = 1;
                std::tie(m_PointParts, partIndex) = getPointPartMap(m_mesh,partIndex);
                std::tie(m_LineParts, partIndex) = getLinePartMap(m_mesh,partIndex);
                std::tie(m_PolygonParts, partIndex) = getPolygonPartMap(m_mesh,partIndex);
                std::tie(m_PolyhedronParts, partIndex) = getPolyhedronPartMap(m_mesh,partIndex);

		LOGINFO("*** Done");
	}


	void MeshDataWriter::DeclareVariable(FAMILY family, VARIABLE_DIMENSION dim, VARIABLE_LOCATION dloc, std::string name, std::string part)
	{

		switch (family)
		{
		case FAMILY::POINT:
			m_Variable[VariableKey(name, part)] = m_PointParts[part]->AddVariable(dim, dloc, name);
			break;
		case FAMILY::LINE:
			m_Variable[VariableKey(name, part)] = m_LineParts[part]->AddVariable(dim, dloc, name);
			break;
		case FAMILY::POLYGON:
			m_Variable[VariableKey(name, part)] = m_PolygonParts[part]->AddVariable(dim, dloc, name);
			break;
		case FAMILY::POLYHEDRON:
			m_Variable[VariableKey(name, part)] = m_PolyhedronParts[part]->AddVariable(dim, dloc, name);
			break;
		default:;
		}

	}

	void MeshDataWriter::DeclareVariable(FAMILY family, VARIABLE_DIMENSION dim, VARIABLE_LOCATION dloc, std::string name)
	{

		switch (family)
		{
		case FAMILY::POINT:
			for (auto const& part : m_PointParts)
			{
				DeclareVariable(family, dim, dloc, name, part.first);
			}
			break;
		case FAMILY::LINE:
			for (auto const& part : m_LineParts)
			{
				DeclareVariable(family, dim, dloc, name, part.first);
			}
			break;
		case FAMILY::POLYGON:
			for (auto const& part : m_PolygonParts)
			{
				DeclareVariable(family, dim, dloc, name, part.first);
			}
			break;
		case FAMILY::POLYHEDRON:
			for (auto const& part : m_PolyhedronParts)
			{
				DeclareVariable(family, dim, dloc, name, part.first);
			}
			break;
		default:;
		}


	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//------------------------------------------ Utils

	/**
	* \brief
	* \return
	*/
	std::string MeshDataWriter::TimeStepNumberForExtension()
	{
		std::string Ext = std::to_string(m_currentTimeStep);
		int nExt = static_cast<int>(Ext.size());
		for (auto i = 0; i < (m_nDigitsExtensionTime - nExt); ++i)
		{
			Ext.insert(0, "0");
		}

		return  Ext;
	}


	//void MeshDataWriter::DeclareAndSetAdjacency(std::string label, Adjacency* adjacency)
	//{

	//	//Reference
	//	m_Adjacency.emplace_back(label);
	//	auto& adj_data = m_Adjacency.back();

	//	//CSR Matrix
	//	auto csr_matrix = adjacency->get_adjacencySparseMatrix();
	//	auto dimRow = csr_matrix->dimRow;
	//	auto nnz = csr_matrix->nnz;
	//	auto columIndex = csr_matrix->columnIndex;
	//	auto rowPtr = csr_matrix->rowPtr;


	//	if ((adjacency->get_sourceFamily() == ELEMENTS::FAMILY::POLYHEDRON) && (adjacency->get_targetFamily() == ELEMENTS::FAMILY::POLYHEDRON))
	//	{
	//		auto sourcetarget = static_cast<PolyhedronCollection*>(adjacency->get_sourceElementCollection());

	//		//Compute Node coordinates
	//		int isource = 0, itarget = 0;
	//		int cpt = 0;
	//		for (auto irow = 0; irow != dimRow; ++irow)
	//		{
	//			auto it = m_mesh->get_PolyhedronCollection()->begin_owned() + irow;
	//			auto xyz = (*it)->get_centroidCoordinates();
	//			Point source_point = Point(isource, xyz[0], xyz[1], xyz[2]);
	//			adj_data.NodesVector.push_back(source_point);
	//			itarget = isource;
	//			for (auto icol = rowPtr[irow]; icol != rowPtr[irow + 1]; ++icol)
	//			{
	//				itarget = itarget + 1;
	//				auto it = m_mesh->get_PolyhedronCollection()->begin_owned() + columIndex[icol];
	//				auto xyz = (*it)->get_centroidCoordinates();
	//				Point target_point = Point(itarget, xyz[0], xyz[1], xyz[2]);
	//				adj_data.NodesVector.push_back(target_point);
	//				adj_data.iSource.push_back(isource);
	//				adj_data.iTarget.push_back(itarget);
	//			}
	//			isource = itarget + 1;
	//			cpt++;
	//		}

	//	}

	//}

	/**
	* \brief
	* \return
	*/
	std::string MeshDataWriter::PartitionNumberForExtension()
	{
		std::string Ext = std::to_string(m_partition + 1);
		int nExt = static_cast<int>(Ext.size());
		for (auto i = 0; i < (m_nDigitsExtensionPartition - nExt); ++i)
		{
			Ext.insert(0, "0");
		}

		return  Ext;
	}



}
