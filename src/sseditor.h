/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * S2-SSEdit
 * Copyright (C) Flamewing 2011 <flamewing.sonic@gmail.com>
 * 
 * S2-SSEdit is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * S2-SSEdit is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SSEDITOR_H_
#define _SSEDITOR_H_

class sseditor
{
private:
	static sseditor *instance;
	Gtk::Window *main_win;
	Gtk::Main *kit;
	Gtk::AboutDialog *aboutdlg;
	Gtk::FileChooserDialog *filedlg;
	Glib::RefPtr<Gtk::Builder> builder;

	Glib::RefPtr<Gtk::FileFilter> pfilefilter;
	// Labels
	Gtk::Label *plabelcurrentstage;
	Gtk::Label *plabeltotalstages;
	Gtk::Label *plabelcurrentsegment;
	Gtk::Label *plabeltotalsegments;
	// Scrollbar
	Gtk::VScrollbar *pvscrollbar;
	// Main toolbar
	Gtk::ToolButton *popenfilebutton;
	Gtk::ToolButton *psavefilebutton;
	Gtk::ToolButton *prevertfilebutton;
	Gtk::RadioToolButton *pselectmodebutton;
	Gtk::RadioToolButton *pinsertmodebutton;
	Gtk::RadioToolButton *pdeletemodebutton;
	Gtk::ToolButton *paboutbutton;
	Gtk::ToolButton *pquitbutton;
	// Special stage toolbar
	Gtk::ToolButton *pfirst_stage_button;
	Gtk::ToolButton *pprevious_stage_button;
	Gtk::ToolButton *pnext_stage_button;
	Gtk::ToolButton *plast_stage_button;
	Gtk::ToolButton *pinsert_stage_before_button;
	Gtk::ToolButton *pappend_stage_button;
	Gtk::ToolButton *pdelete_stage_button;
	Gtk::ToolButton *pswap_stage_prev_button;
	Gtk::ToolButton *pswap_stage_next_button;
	// Segment toolbar
	Gtk::ToolButton *pfirst_segment_button;
	Gtk::ToolButton *pprevious_segment_button;
	Gtk::ToolButton *pnext_segment_button;
	Gtk::ToolButton *plast_segment_button;
	Gtk::ToolButton *pinsert_segment_before_button;
	Gtk::ToolButton *pappend_segment_button;
	Gtk::ToolButton *pdelete_segment_button;
	Gtk::ToolButton *pswap_segment_prev_button;
	Gtk::ToolButton *pswap_segment_next_button;
	// Segment flags
	Gtk::RadioButton *pnormal_segment;
	Gtk::RadioButton *pring_message;
	Gtk::RadioButton *pcheckpoint;
	Gtk::RadioButton *pchaos_emerald;
	Gtk::RadioButton *psegment_turnthenrise;
	Gtk::RadioButton *psegment_turnthendrop;
	Gtk::RadioButton *psegment_turnthenstraight;
	Gtk::RadioButton *psegment_straight;
	Gtk::RadioButton *psegment_straightthenturn;
	Gtk::RadioButton *psegment_right;
	Gtk::RadioButton *psegment_left;

	sseditor();
	sseditor(sseditor const& other);
	sseditor(int argc, char *argv[], char const *uifile);
public:
	static sseditor * create_instance(int argc, char *argv[], char const *uifile)
	{
		if (!instance)
			instance = new sseditor(argc, argv, uifile);
		return instance;
	}
	static sseditor * get_instance()
	{
		return instance;
	}
	void run();

	// Scrollbar
	void on_vscrollbar1_value_changed();
	// Main toolbar
	void on_filedialog_response(int response_id);
	void on_openfilebutton_clicked();
	void on_savefilebutton_clicked();
	void on_revertfilebutton_clicked();
	void on_selectmodebutton_toggled();
	void on_insertmodebutton_toggled();
	void on_deletemodebutton_toggled();
	void on_aboutdialog_response(int response_id);
	void on_aboutbutton_clicked();
	void on_quitbutton_clicked();
	// Special stage toolbar
	void on_first_stage_button_clicked();
	void on_previous_stage_button_clicked();
	void on_next_stage_button_clicked();
	void on_last_stage_button_clicked();
	void on_insert_stage_before_button_clicked();
	void on_append_stage_button_clicked();
	void on_delete_stage_button_clicked();
	void on_swap_stage_prev_button_clicked();
	void on_swap_stage_next_button_clicked();
	// Segment toolbar
	void on_first_segment_button_clicked();
	void on_previous_segment_button_clicked();
	void on_next_segment_button_clicked();
	void on_last_segment_button_clicked();
	void on_insert_segment_before_button_clicked();
	void on_append_segment_button_clicked();
	void on_delete_segment_button_clicked();
	void on_swap_segment_prev_button_clicked();
	void on_swap_segment_next_button_clicked();
	// Segment flags
	void on_normal_segment_toggled();
	void on_ring_message_toggled();
	void on_checkpoint_toggled();
	void on_chaos_emerald_toggled();
	void on_segment_turnthenrise_toggled();
	void on_segment_turnthendrop_toggled();
	void on_segment_turnthenstraight_toggled();
	void on_segment_straight_toggled();
	void on_segment_straightthenturn_toggled();
	void on_segment_right_toggled();
	void on_segment_left_toggled();
protected:

};

#endif // _SSEDITOR_H_
