/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2019 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2019 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2019 Total, S.A
 * Copyright (c) 2020-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

#pragma once

#define LEVEL_LOG_FILE "DEBUG"
#define LEVEL_LOG_SCREEN "BRIEF"

//#define IS_BIG_ENDIAN() ( (*(char*)&i) == 0 )

namespace PAMELA
{

  namespace utils
  {
    
    template <class T>
    static void bites_swap(T *objp)
    {
      if (true)
      {
        unsigned char *memp = reinterpret_cast<unsigned char*>(objp);
        std::reverse(memp, memp + sizeof(T));
      }
    }

  }

}
