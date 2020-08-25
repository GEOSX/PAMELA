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

#include "Mesh/MeshFactory.hpp"
#include "Import/INRIA_mesh.hpp"
#include "Import/Eclipse_mesh.hpp"
#include "Utils/Logger.hpp"
#include "Mesh/CartesianMesh.hpp"
#include "Import/Gmsh_mesh.hpp"
#include "Utils/File.hpp"

namespace PAMELA
{
	/**
	 * \brief
	 * \param file_path
	 * \return
	 */
	Mesh* MeshFactory::makeMesh(std::string file_path)
	{
		LOGINFO("**********************************************************************");
		LOGINFO("                         PAMELA Library Import tool                   ");
		LOGINFO("**********************************************************************");


		std::string file_extension;
		file_extension = file_path.substr(file_path.find_last_of(".") + 1);
		File file = File(file_path);
		if(!file.exists())
		{
			LOGERROR("File does not exist.");
		}

		if ((file_extension == "mesh") || (file_extension == "MESH"))
		{
			LOGINFO("INRIA MESH FORMAT IDENTIFIED");
                        INRIA_mesh meshBuilder;
			return meshBuilder.CreateMesh(file_path);
		}
		if ((file_extension == "msh") || (file_extension == "MSH"))
		{
			LOGINFO("GMSH FORMAT IDENTIFIED");
                        Gmsh_mesh meshBuilder;
			return meshBuilder.CreateMesh(file_path);
		}
		if ((file_extension == "grdecl") || (file_extension == "GRDECL"))
		{
			LOGINFO("ECLIPSE GRDECL FORMAT IDENTIFIED");
                        Eclipse_mesh meshBuilder;
			return meshBuilder.CreateMeshFromGRDECL(file);
		}
		if ((file_extension == "EGRID") || (file_extension == "egrid"))
		{
			LOGINFO("ECLIPSE GRDECL FORMAT IDENTIFIED");
                        Eclipse_mesh meshBuilder;
			return meshBuilder.CreateMeshFromEclipseBinaryFiles(file);
		}

		LOGERROR("Mesh file format ." + file_extension + " not supported");
		return nullptr;

	}

	/**
	 * \brief
	 * \param nx
	 * \param ny
	 * \param nz
	 * \param dx
	 * \param dy
	 * \param dz
	 * \return
	 */
	Mesh* MeshFactory::makeMesh(int nx, int ny, int nz, double dx, double dy, double dz)
	{

		std::vector<double> vdx, vdy, vdz;
		for (auto i = 0; i < nx; ++i)
		{
			vdx.push_back(dx);
		}
		for (auto i = 0; i < ny; ++i)
		{
			vdy.push_back(dy);
		}
		for (auto i = 0; i < nz; ++i)
		{
			vdz.push_back(dz);
		}

		return new CartesianMesh(vdx, vdy, vdz);

	}
}
