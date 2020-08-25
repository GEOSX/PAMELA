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
#include "Logger.hpp"
#include "Utils.hpp"
#include <iomanip>

#define LEVEL_LOG_FILE "DEBUG"
#define LEVEL_LOG_SCREEN "BRIEF"
#define FILE_NAME "simulator.log"

namespace PAMELA
{
  Logger* Logger::instance()
  {
    static Logger s_instance(LEVEL_LOG_FILE, FILE_NAME, LEVEL_LOG_SCREEN);
    return &s_instance;
  }

  Logger::Logger(std::string LevelLogFile, std::string file_name, std::string LevelScreen)
  {

    //Log file
    if (LevelLogFile == "ERROR")
      m_log_level = VerbosityLevelLogFile::ERROR;
    if (LevelLogFile == "WARNING")
      m_log_level = VerbosityLevelLogFile::WARNING;
    if (LevelLogFile == "INFO")
      m_log_level = VerbosityLevelLogFile::INFO;
    if (LevelLogFile == "DEBUG")
      m_log_level = VerbosityLevelLogFile::DEBUG;

    m_logfile.open(file_name);

    //Screen
    if (LevelScreen == "SILENT")
      m_screen_level = VerbosityLevelScreen::SILENT;
    if (LevelScreen == "BRIEF")
      m_screen_level = VerbosityLevelScreen::BRIEF;
    if (LevelScreen == "ALL")
      m_screen_level = VerbosityLevelScreen::ALL;
  }


  void Logger::LogERROR(std::string msg)
  {

    std::string message = "[" + get_time() + "] ERROR:   " + msg.c_str();
    write_file(message);
    if (m_screen_level >= VerbosityLevelScreen::BRIEF)
      write_screen("ERROR:   " + msg + " in file " + __FILE__ + " line " + std::to_string(__LINE__));
                abort();
  }

  void Logger::LogWARNING(std::string msg)
  {
    std::string message = "[" + get_time() + "] WARNING: " + msg.c_str();
    if (m_log_level >= VerbosityLevelLogFile::WARNING)
      write_file(message);
    if (m_screen_level >= VerbosityLevelScreen::BRIEF)
      write_screen("WARNING: " + msg);
  }

  void Logger::LogINFO(std::string msg)
  {
    std::string message = "[" + get_time() + "] INFO:    " + msg.c_str();
    if (m_log_level >= VerbosityLevelLogFile::INFO)
      write_file(message);
    if (m_screen_level >= VerbosityLevelScreen::BRIEF)
      write_screen(msg);
  }

  void Logger::LogDEBUG(std::string msg)
  {
    std::string message = "[" + get_time() + "] DEBUG:   " + msg.c_str();
    if (m_log_level >= VerbosityLevelLogFile::DEBUG)
      write_file(message);
    if (m_screen_level >= VerbosityLevelScreen::ALL)
      write_screen(msg);
  }


  void Logger::Log(LogMessage& msg)
  {
    std::string str_msg = msg.getMessage();
    VerbosityLevelLogFile MessageLevel = msg.getMessageLevel();
    if (MessageLevel == VerbosityLevelLogFile::ERROR)
    {
      LogERROR(str_msg);
    }
    else if (MessageLevel == VerbosityLevelLogFile::WARNING)
    {
      LogWARNING(str_msg);
    }
    else if (MessageLevel == VerbosityLevelLogFile::DEBUG)
    {
      LogDEBUG(str_msg);
    }
    else if (MessageLevel == VerbosityLevelLogFile::INFO)
    {
      LogINFO(str_msg);
    }
  }



  std::string Logger::get_time()
  {
    std::stringstream res;
    time_t t = time(nullptr);
    struct tm *now;
#ifdef WIN32
    struct tm now_st;
    now = &now_st;
    localtime_s(now, &t);
#else
    now = localtime(&t);
#endif
    // res << std::put_time(now, "%Y-%m-%d %H:%M:%S");
    res << (now->tm_year + 1900) << "-" << (now->tm_mon + 1) << "-" << (now->tm_mday) << "-" << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec;
    return res.str();
  }

}
