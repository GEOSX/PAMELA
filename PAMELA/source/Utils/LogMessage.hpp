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

// Std library includes
#include <sstream>

namespace PAMELA
{

	enum class VerbosityLevelLogFile
	{
		ERROR, WARNING, DEBUG, INFO
	};


	class LogMessage
	{

	public:

		static LogMessage* instance();
		static LogMessage* instance(VerbosityLevelLogFile lvl);

		std::string getMessage() const { return m_message.str(); }
		VerbosityLevelLogFile& getMessageLevel() { return m_message_level; }


		template<typename T>
		LogMessage& operator<<(const T& msg)
		{
			m_message << msg;
			return *this;
		}

	private:

		VerbosityLevelLogFile m_message_level;

		std::stringstream m_message;

		static LogMessage* s_instance;

		LogMessage();
		explicit LogMessage(VerbosityLevelLogFile lvl);


	};

}
