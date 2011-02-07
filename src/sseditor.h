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

class ssobj_file;

class sseditor
{
private:
	static sseditor *instance;
	bool update_in_progress;
	bool dragging, drop_enabled, can_drag;

	// State variables.
	ssobj_file *specialstages;
	int currstage, currsegment;
	int first_line;
	int draw_width, draw_height;
	int mouse_x, mouse_y;
	enum EditModes
	{
		eSelectMode = 0,
		eInsertRingMode,
		eInsertBombMode,
		eDeleteMode,
		eNumModes
	} mode;
	int selseg, selx, sely, seltype;
	int hotseg, hotx, hoty, hottype;

	// GUI variables.
	Gtk::Window *main_win;
	Gtk::Main *kit;
	Gtk::AboutDialog *aboutdlg;
	Gtk::FileChooserDialog *filedlg;
	Glib::RefPtr<Gtk::Builder> builder;
	Glib::RefPtr<Gdk::Pixbuf> ringimg, bombimg;

	Glib::RefPtr<Gtk::FileFilter> pfilefilter;
	Gtk::DrawingArea *pspecialstageobjs;
	Gtk::Ruler *phruler;
	// Labels
	Gtk::Label *plabelcurrentstage, *plabeltotalstages, *plabelcurrentsegment, *plabeltotalsegments;
	// Scrollbar
	Gtk::VScrollbar *pvscrollbar;
	// Main toolbar
	Gtk::ToolButton *popenfilebutton, *psavefilebutton, *prevertfilebutton, *paboutbutton, *pquitbutton;
	Gtk::RadioToolButton *pselectmodebutton, *pinsertringbutton, *pinsertbombbutton,
	    *pdeletemodebutton;
	// Special stage toolbar
	Gtk::Toolbar	*pstage_toolbar;
	Gtk::ToolButton *pfirst_stage_button, *pprevious_stage_button, *pnext_stage_button,
		*plast_stage_button, *pinsert_stage_before_button, *pappend_stage_button,
		*pdelete_stage_button, *pswap_stage_prev_button, *pswap_stage_next_button;
	// Segment toolbar
	Gtk::Toolbar	*psegment_toolbar;
	Gtk::ToolButton *pfirst_segment_button, *pprevious_segment_button, *pnext_segment_button,
		*plast_segment_button, *pinsert_segment_before_button, *pappend_segment_button,
		*pdelete_segment_button, *pswap_segment_prev_button, *pswap_segment_next_button;
	// Segment flags
	Gtk::Expander	*psegment_expander;
	Gtk::RadioButton *pnormal_segment, *pring_message, *pcheckpoint, *pchaos_emerald,
		*psegment_turnthenrise, *psegment_turnthendrop,* psegment_turnthenstraight,
		*psegment_straight, *psegment_straightthenturn, *psegment_right, *psegment_left;
	// Object flags
	Gtk::Expander	*pobject_expander;
	Gtk::Button		*pmoveup, *pmovedown, *pmoveleft, *pmoveright;
	Gtk::RadioButton *pringtype, *pbombtype;
	
	sseditor();
	sseditor(sseditor const& other);
	sseditor(int argc, char *argv[], char const *uifile);

	bool move_object(int dx, int dy);
public:
	static sseditor * create_instance(int argc, char *argv[], char const *uifile)
	{
		if (!instance)
			instance = new sseditor(argc, argv, uifile);
		return instance;
	}
	static sseditor * get_instance()
	{	return instance;	}
	void run();

	bool on_specialstageobjs_configure_event(GdkEventConfigure *event);
	void on_specialstageobjs_drag_data_received(Glib::RefPtr<Gdk::DragContext> const& context,
	                                            int x, int y,
	                                            Gtk::SelectionData const& selection_data,
	                                            guint info, guint time);
	bool on_specialstageobjs_expose_event(GdkEventExpose *event);
	bool on_specialstageobjs_key_press_event(GdkEventKey *event);
	bool on_specialstageobjs_button_press_event(GdkEventButton *event);
	bool on_specialstageobjs_button_release_event(GdkEventButton *event);
	bool on_specialstageobjs_scroll_event(GdkEventScroll *event);
	void on_specialstageobjs_drag_begin(Glib::RefPtr<Gdk::DragContext> const& targets);
	bool on_specialstageobjs_motion_notify_event(GdkEventMotion *event);
	void on_specialstageobjs_drag_data_get(Glib::RefPtr<Gdk::DragContext> const& targets,
	                                       Gtk::SelectionData& selection_data,
	                                       guint info, guint time);
	bool on_specialstageobjs_selection_clear_event(GdkEventSelection *event);
	// Scrollbar
	void on_vscrollbar_value_changed();
	// Main toolbar
	void on_filedialog_response(int response_id);
	void on_openfilebutton_clicked();
	void on_savefilebutton_clicked();
	void on_revertfilebutton_clicked();
	void on_selectmodebutton_toggled();
	void on_insertringbutton_toggled();
	void on_insertbombbutton_toggled();
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
	// Object flags
	void on_moveup_clicked();
	void on_movedown_clicked();
	void on_moveleft_clicked();
	void on_moveright_clicked();
	void on_ringtype_toggled();
	void on_bombtype_toggled();
protected:
	void update();
};

#endif // _SSEDITOR_H_
