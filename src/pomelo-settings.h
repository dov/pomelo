//======================================================================
//  pomelo-settings.h - Settings in ~/.config/pomelo
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Feb  8 22:25:40 2021
//----------------------------------------------------------------------
#ifndef POMELO_SETTINGS_H
#define POMELO_SETTINGS_H

#include <gtkmm.h>
#include <string>

class PomeloSettings : public Glib::KeyFile
{
  public:
  PomeloSettings();
  void save();
  void load();
  std::string get_string_default(const gchar *key, const char *default_value="");
  int get_int_default(const gchar *key, int default_value=0);
  int get_double_default(const gchar *key, double default_value=0);
  void set_int(const char *key, int value);
  void set_double(const char *key, double val);

  private:
  std::string m_settings_file;
};

#endif /* POMELO-SETTINGS */
