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

namespace PAMELA
{

	struct IndexData
	{
		IndexData() :Local(-1), Global(-1) {}
		IndexData(int id) :Local(id), Global(id) {}
		IndexData(int idl, int idg) :Local(idl), Global(idg) {}
		int Local;
		int Global;
                int Init;
	};

}
