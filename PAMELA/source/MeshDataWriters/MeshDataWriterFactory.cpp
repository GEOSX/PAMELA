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

#include "MeshDataWriters/MeshDataWriterFactory.hpp"
#include "MeshDataWriters/EnsightGoldWriter.hpp"
#include "MeshDataWriters/VTKWriter.hpp"
#include "Utils/Logger.hpp"

#include <string>

namespace PAMELA
{
	MeshDataWriter* MeshDataWriterFactory::makeWriter(
                Mesh * mesh,
                const std::string& file_path)
	{
		LOGINFO("**********************************************************************");
		LOGINFO("                         PAMELA Library Export tool                   ");
		LOGINFO("**********************************************************************");


                std::string filename = file_path.substr(file_path.find_last_of("/") + 1);
		std::string file_extension = filename.substr(filename.find_last_of(".") + 1);
                std::string file_wo_extension
                    = file_path.substr(0,file_path.find_last_of("."));
#ifdef WITH_VTK
		if ((file_extension == "vtm") || (file_extension == "VTM"))
		{

			LOGINFO("VTM OUTPUT MESH FORMAT IDENTIFIED");
			VTKWriter* writer = new VTKWriter(mesh, file_wo_extension);
			return  writer;

		}
#endif // WITH_VTK

		if ((file_extension == "case") || (file_extension == "CASE"))
		{
			LOGINFO("ENSIGHT GOLD OUTPUT MESH FORMAT IDENTIFIED");
                        EnsightGoldWriter * writer =
                            new EnsightGoldWriter( mesh,file_wo_extension);
			return  writer;
		}
		else
		{
			LOGERROR("Mesh file format ." + file_extension + " not supported");
			return nullptr;
		}
	}
}
