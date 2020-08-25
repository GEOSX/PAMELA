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

// Header include
#include "File.hpp"

// Std library includes
#include <algorithm>


namespace PAMELA {
File::File(std::string fileName) :
  m_FullName(fileName),
  m_Directory(""),
  m_ShortName(""),
  m_Extension(""),
  m_NameWithoutExtension("")
{
  // replace '\' with '/' (all file APIs accept '/' on Windows)
  std::replace(m_FullName.begin(), m_FullName.end(), '\\', '/');

  //Directory
  std::size_t last_slash_idx = m_FullName.rfind('/');
  if (std::string::npos != last_slash_idx)
  {
    m_Directory = m_FullName.substr(0, last_slash_idx);
    m_ShortName = m_FullName.substr(last_slash_idx + 1);
  }
  else
  {
    m_ShortName = m_FullName;
  }

  //Extension
  std::size_t last_dot_idx = m_ShortName.rfind('.');
  if (std::string::npos != last_dot_idx)
  {
    m_Extension = m_ShortName.substr(last_dot_idx + 1);
    m_NameWithoutExtension = m_ShortName.substr(0, last_dot_idx);
  }
  else
  {
    m_NameWithoutExtension = m_ShortName;
  }

}
} // namespace
