/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2019 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2019 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2019 Total, S.A
 * Copyright (c) 2020-     GEOSX Contributors
 * All right reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

#pragma once
#include <vector> 
#include "MeshDataWriters/MeshDataWriter.hpp"

namespace PAMELA
{
	class MeshDataWriterFactory
	{

	public:

		static MeshDataWriter* makeWriter(Mesh * mesh,
                        const std::string& file_path);

	private:
		MeshDataWriterFactory() = delete;

	};

}
