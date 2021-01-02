//======================================================================
//  progress-dialog.h - A progress dialog with a message,
//  a progress bar, and a cancel button.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sat Jan  2 06:50:51 2021
//----------------------------------------------------------------------
#ifndef PROGRESS_DIALOG_H
#define PROGRESS_DIALOG_H

#include <gtkmm.h>

class ProgressDialog : public Gtk::Dialog {
  public:
  // contructor
  ProgressDialog(Gtk::Window& parent,
                 const Glib::ustring& title);

  void update(const Glib::ustring& text_markup,
              double progress);
              
  using type_signal_cancel = sigc::signal<void()>;
  type_signal_cancel signal_cancel();
  
  private:
  // Signals
  type_signal_cancel m_signal_cancel;
  Gtk::Label m_label;
  Gtk::ProgressBar m_progress_bar;

  // Slots
  void on_response(int response_id) override;
};

#endif /* PROGRESS-DIALOG */
