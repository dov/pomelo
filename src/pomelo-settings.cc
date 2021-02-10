// File to handle the pomelo settings through an glib ini file
#include <stdlib.h>
#include "pomelo-settings.h"
#include <glib/gstdio.h>

using namespace std;

const char *GROUP_NAME = "pomelo";

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
  try {
    load();
  }
  catch(...) {
    save();
  }
}

void PomeloSettings::save()
{
  save_to_file(m_settings_file);
}

void PomeloSettings::load()
{
  load_from_file(m_settings_file);
}

// Convenience functions as we have only one group "pomelo"
std::string PomeloSettings::get_string_default(const gchar *key, const char *default_value)
{
  if (!has_key(GROUP_NAME, key))
    return default_value;
  return get_string(GROUP_NAME, key);
}

int PomeloSettings::get_int_default(const gchar *key, int default_value)
{
  if (!has_key(GROUP_NAME, key))
    return default_value;
  return get_integer(GROUP_NAME, key);
}
int PomeloSettings::get_double_default(const gchar *key, double default_value)
{
  if (!has_key(GROUP_NAME, key))
    return default_value;
  return get_double(GROUP_NAME, key);
}

void PomeloSettings::set_int(const char *key, int value)
{
  set_integer(GROUP_NAME, key, value);
}

void PomeloSettings::set_double(const char *key, double value)
{
  KeyFile::set_double(GROUP_NAME, key, value);
}

