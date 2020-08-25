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
#include "Mesh/Mesh.hpp"

namespace PAMELA
{

	class CartesianMesh : public Mesh
	{

	public:

		CartesianMesh(const std::vector<double>& dx, const std::vector<double>& dy, const std::vector<double>& dz);
		void Distort(double alpha);


	private:

		std::vector<double> m_dx, m_dy, m_dz;
		double m_Lxmin, m_Lymin, m_Lzmin, m_Lxmax, m_Lymax, m_Lzmax;

	};

}
