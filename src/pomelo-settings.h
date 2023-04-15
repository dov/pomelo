//======================================================================
//  pomelo-settings.h - Settings in ~/.config/pomelo
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Feb  8 22:25:40 2021
//----------------------------------------------------------------------
#ifndef POMELO_SETTINGS_H
#define POMELO_SETTINGS_H

#include <glibmm/keyfile.h>
#include <string>

class PomeloSettings : public Glib::KeyFile
{
  public:
  PomeloSettings();
  void save();
  void load();
  std::string get_string_default(const std::string& key, const char *default_value="");
  int get_int_default(const std::string& key, int default_value=0);
  int get_double_default(const std::string& key, double default_value=0);
  void set_int(const std::string& key, int value);
  void set_double(const std::string& key, double val);
  void set_string(const std::string& key, const std::string& val);

  private:
  std::string m_settings_file;
  std::string m_group_name;
};

#endif /* POMELO-SETTINGS */
