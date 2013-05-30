/*
 * IBTApp.cc
 *
 *  Created on: Dec 15, 2008
 *      Author: imran
 */

#include "IBTApp.h"

Charset IBTApp::currentCharset = Charset::defaultCharset();

IBTApp::IBTApp(IOEventParser& my_iep, const std::string& path_to_mapping_file) :
  iep(my_iep), teacher_voice("./resources/Voice/teacher/"), student_voice("./resources/Voice/student/")
{
  if( path_to_mapping_file == "" || path_to_mapping_file.size() == 0 )
    loadDefaultCharset();
  else
    loadLanguageCharset(path_to_mapping_file);
}

IBTApp::~IBTApp()
{
  loadDefaultCharset(); //Before we destroy the app, we revert the BT back to English
  //Voice::stopAllPlaying(); //XXX: Not sure if this is making a difference
}

void IBTApp::loadDefaultCharset()
{
  currentCharset = Charset::defaultCharset();
}

void IBTApp::loadLanguageCharset(const std::string& path_to_mapping_file)
{
  //std::cout << "    (DEBUG)Setting Charset to:" << path_to_mapping_file << std::endl;
  currentCharset = Charset::fromFile(path_to_mapping_file);
  iep.setCharset(currentCharset);
}

const Charset& IBTApp::getCurrentCharset()
{
  //std::cout << "		(DEBUG)Inside getCurrentCharset" << std::endl;
  return currentCharset;
}

const Voice& IBTApp::getTeacherVoice() const
{
  return teacher_voice;
}
const Voice& IBTApp::getStudentVoice() const
{
  return student_voice;
}
