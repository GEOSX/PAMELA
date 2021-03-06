/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2020 Total, S.A
 * Copyright (c) 2020-     GEOSX Contributors
 * All right reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

#include "MeshDataWriters/EnsightGoldWriter.hpp"
#include "Utils/Logger.hpp"
#include <iomanip>
namespace PAMELA
{



	/**
	 * \brief
	 */
	void EnsightGoldWriter::MakeCaseFile()
	{

		LOGINFO("*** Make Ensight Gold .case file");

		//Create file
		m_caseFile.open(m_name + "_" + PartitionNumberForExtension() + ".case", std::fstream::in | std::fstream::out | std::fstream::trunc);


		//Header
		m_caseFile << "# Generated by PAMELA" << std::endl;
		m_caseFile << "FORMAT" << std::endl;
		m_caseFile << "type: ensight gold" << std::endl;
		m_caseFile << std::endl;
		m_caseFile << "GEOMETRY" << std::endl;
		m_caseFile << "model: " << m_name + "_" + PartitionNumberForExtension() + ".geo" << std::endl;
		m_caseFile << std::endl;

		std::string stars(m_nDigitsExtensionTime, '*');


		m_caseFile << "VARIABLE" << std::endl;

		//--Variables
		if (m_Variable.size() != 0)
		{

			//Point variables
			for (auto it = m_PointParts.begin(); it != m_PointParts.end(); ++it)
			{
				auto part = it->second;

				//Per Element Variable
				for (auto it2 = part->PerElementVariable.begin(); it2 != part->PerElementVariable.end(); ++it2)
				{
					m_caseFile << "scalar per element:" << "     " << 1 << std::setw(5);
					m_caseFile << "     " << (*it2)->Label << " " << m_name << "_" << part->Label << "_" + (*it2)->Label << "_" << stars << std::endl;
				}

				//Per Node Variable
				for (auto it2 = part->PerNodeVariable.begin(); it2 != part->PerNodeVariable.end(); ++it2)
				{
					m_caseFile << "scalar per node:" << "     " << 1 << std::setw(5);
					m_caseFile << "     " << (*it2)->Label << " " << m_name << "_" << part->Label << "_" + (*it2)->Label << "_" << stars << std::endl;
				}
			}

			//Line variables
			for (auto it = m_LineParts.begin(); it != m_LineParts.end(); ++it)
			{
				auto part = it->second;

				//Per Element Variable
				for (auto it2 = part->PerElementVariable.begin(); it2 != part->PerElementVariable.end(); ++it2)
				{
					m_caseFile << "scalar per element:" << "     " << 1 << std::setw(5);
					m_caseFile << "     " << (*it2)->Label << " " << m_name << "_" << part->Label << "_" + (*it2)->Label << "_" << stars << std::endl;
				}

				//Per Node Variable
				for (auto it2 = part->PerNodeVariable.begin(); it2 != part->PerNodeVariable.end(); ++it2)
				{
					m_caseFile << "scalar per node:" << "     " << 1 << std::setw(5);
					m_caseFile << "     " << (*it2)->Label << " " << m_name << "_" << part->Label << "_" + (*it2)->Label << "_" << stars << std::endl;
				}
			}


			//Polygon variables
			for (auto it = m_PolygonParts.begin(); it != m_PolygonParts.end(); ++it)
			{
				auto part = it->second;

				//Per Element Variable
				for (auto it2 = part->PerElementVariable.begin(); it2 != part->PerElementVariable.end(); ++it2)
				{
					m_caseFile << "scalar per element:" << "     " << 1 << std::setw(5);
					m_caseFile << "     " << (*it2)->Label << " " << m_name << "_" << part->Label << "_" + (*it2)->Label << "_" << stars << std::endl;
				}

				//Per Node Variable
				for (auto it2 = part->PerNodeVariable.begin(); it2 != part->PerNodeVariable.end(); ++it2)
				{
					m_caseFile << "scalar per node:" << "     " << 1 << std::setw(5);
					m_caseFile << "     " << (*it2)->Label << " " << m_name << "_" << part->Label << "_" + (*it2)->Label << "_" << stars << std::endl;
				}
			}


			//Polyhedron variables
			for (auto it = m_PolyhedronParts.begin(); it != m_PolyhedronParts.end(); ++it)
			{
				auto part = it->second;

				//Per Element Variable
				for (auto it2 = part->PerElementVariable.begin(); it2 != part->PerElementVariable.end(); ++it2)
				{
					m_caseFile << "scalar per element:" << "     " << 1 << std::setw(5);
					m_caseFile << "     " << (*it2)->Label << " " << m_name << "_" << part->Label << "_" + (*it2)->Label << "_" << stars << std::endl;
				}

				//Per Node Variable
				for (auto it2 = part->PerNodeVariable.begin(); it2 != part->PerNodeVariable.end(); ++it2)
				{
					m_caseFile << "scalar per node:" << "     " << 1 << std::setw(5);
					m_caseFile << "     " << (*it2)->Label << " " << m_name << "_" << part->Label << "_" + (*it2)->Label << "_" << stars << std::endl;
				}
			}
		}
		else
		{
			LOGWARNING("No variables found. Add variable before making case file");
		}


		//--Time
		m_caseFile << std::endl;
		m_caseFile << "TIME" << std::endl;
		m_caseFile << "time set: 1" << std::endl;
		m_caseFile << "number of steps: 1" << std::endl;
		m_caseFile << "filename start number: 0" << std::endl;
		m_caseFile << "filename increment: 1" << std::endl;
		m_caseFile << "time values:" << std::endl;
		m_caseFile << 0 << std::endl;

		LOGINFO("*** Done");
	}

	/**
	 * \brief
	 */
	void EnsightGoldWriter::MakeGeoFile()
	{
		LOGINFO("*** Init Ensight Gold .geo file");

		//Create file
		m_geoFile.open( m_name + "_" + PartitionNumberForExtension() + ".geo", std::fstream::in | std::fstream::out | std::fstream::trunc);

		//Header
		MakeGeoFile_Header();

		//-- Point
		MakeGeoFile_AddParts(&m_PointParts);

		//-- Line
		MakeGeoFile_AddParts(&m_LineParts);

		//-- Polygon
		MakeGeoFile_AddParts(&m_PolygonParts);

		//-- Polyhedron
		MakeGeoFile_AddParts(&m_PolyhedronParts);

		LOGINFO("*** Done");
	}



	void EnsightGoldWriter::MakeGeoFile_Header()
	{

		//Header
		m_geoFile << "EnSight Model Geometry File" << std::endl;
		m_geoFile << "EnSight 7.1.0" << std::endl;

		//Node id
		m_geoFile << "node id given" << std::endl;

		//Element id
		m_geoFile << "element id given" << std::endl;

	}


	void EnsightGoldWriter::Init()
	{
		//To be launched after variables declaration
		MakeCaseFile();
		MakeGeoFile();
	}

	/**
	* \brief
	*/
	void EnsightGoldWriter::Dump()
	{

		//Point
		DumpVariables_Parts(&m_PointParts);

		//Line
		DumpVariables_Parts(&m_LineParts);

		//Polygon
		DumpVariables_Parts(&m_PolygonParts);

		//Polyhedron
		DumpVariables_Parts(&m_PolyhedronParts);

	}
}
