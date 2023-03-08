//======================================================================
//  progress-dialog.h - A progress dialog with a message,
//  a progress bar, and a cancel button.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sat Jan  2 06:50:51 2021
//----------------------------------------------------------------------

#include "progress-dialog.h"
#include "dov-mm-macros.h"
#include <fmt/core.h>
#include <spdlog/spdlog.h>

using namespace fmt;

// Constructor
ProgressDialog::ProgressDialog(Gtk::Window& parent,
                               const Glib::ustring& title)
  : Gtk::Dialog(title,
                parent,
                true // model
                )
{
  spdlog::info("Creating the ProgressDialog");

  set_border_width(0);
  m_progress_bar.set_show_text(true);
  m_label.set_text("Message");
  get_content_area()->pack_start(m_label);
  get_content_area()->pack_start(m_progress_bar);
  get_content_area()->show_all();
  add_button("Cancel",0);

  spdlog::info("Done creating the ProgressDialog");
}

ProgressDialog::type_signal_cancel ProgressDialog::signal_cancel()
{
  return m_signal_cancel;
}

// Slots
void ProgressDialog::on_response(int response_id)
{
  print("on_response. response_id={}\n", response_id);
  this->m_signal_cancel();
}

void ProgressDialog::update(const Glib::ustring& text_markup,
                            double progress)
{
  m_label.set_markup(text_markup);
  m_progress_bar.set_text(text_markup);
  m_progress_bar.set_fraction(progress);
}

