// File to handle the pomelo settings through an glib ini file
#include <stdlib.h>
#include "pomelo-settings.h"
#include <glib/gstdio.h>
#include <fmt/core.h>

using namespace std;
using namespace fmt;

static void rec_mkdir(std::string path)
{
  size_t pos = 0;
  int mode = 0;
#ifndef _WIN32
  mode = S_IRWXU | S_IRWXG | S_IRWXO;
#endif

  if (path[path.size()-1]!= '/')
      path += '/';

  while((pos = path.find_first_of('/', pos+1)) != std::string::npos)
    g_mkdir(path.substr(0, pos).c_str(), mode);
}

PomeloSettings::PomeloSettings()
{
  string config_dir;
  const char *s;
  if ((s = getenv("LOCALAPPDATA"))) // Really only for windows
    config_dir = string(s);
  else
    config_dir = string(getenv("HOME"))+"/.config";

  config_dir += "/pomelo";
  rec_mkdir(config_dir);
  m_settings_file = config_dir + "/pomelo.conf";
  m_group_name = "pomelo";

  try {
    load();
  }
  catch(...) {
  }
  set_string("config_dir",config_dir);
  save();
}

void PomeloSettings::save()
{
  try
    {
      save_to_file(m_settings_file);
    }
  catch(Glib::KeyFileError& err)
    {
      print("Failed saving to file {}: code={}\n", m_settings_file, err.code());
    }
}

void PomeloSettings::load()
{
  try
    {
      load_from_file(m_settings_file);
    }
  catch(Glib::KeyFileError& err)
    {
      print("Failed saving to file {}: code={}\n", m_settings_file, err.code());
    }
}

// Convenience functions as we have only one group "pomelo"
std::string PomeloSettings::get_string_default(const string& key, const char *default_value)
{
  if (!has_key(m_group_name, key))
    return default_value;
  return get_string(m_group_name, key);
}

int PomeloSettings::get_int_default(const string& key, int default_value)
{
  print("Getting {}\n", key);
  if (!has_group(m_group_name) || !has_key(m_group_name, key))
    return default_value;
  return get_integer(m_group_name, key);
}

int PomeloSettings::get_double_default(const string& key, double default_value)
{
  print("Getting {}\n", key);
  if (!has_group(m_group_name) || !has_key(m_group_name, key))
    return default_value;

  return get_double(m_group_name, key);
}

void PomeloSettings::set_int(const string& key, int value)
{
  print("Setting {} to {}\n", key, value);
  set_integer(m_group_name, key, value);
}

void PomeloSettings::set_double(const string& key, double value)
{
  print("Setting {} to {}\n", key, value);
  KeyFile::set_double(m_group_name, key, value);
}

void PomeloSettings::set_string(const string& key, const string& value)
{
  print("Setting {} to {}\n", key, value);
  KeyFile::set_string(m_group_name, key, value);
}

