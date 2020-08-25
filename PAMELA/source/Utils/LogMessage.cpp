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
#include "Utils/LogMessage.hpp"

namespace PAMELA
{

	LogMessage* LogMessage::s_instance = nullptr;

	LogMessage* LogMessage::instance()
	{
		return instance(VerbosityLevelLogFile::INFO);
	}

	LogMessage* LogMessage::instance(VerbosityLevelLogFile lvl)
	{
		static LogMessage instance(lvl);
		return &instance;
	}

	LogMessage::LogMessage()
	{
		m_message_level = VerbosityLevelLogFile::INFO;
		m_message << "";
	}

	LogMessage::LogMessage(VerbosityLevelLogFile lvl)
	{
		m_message_level = lvl;
		m_message << "";
	}

}

