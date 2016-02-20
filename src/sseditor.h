/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Copyright (C) Flamewing 2011-2015 <flamewing.sonic@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SSEDITOR_H
#define __SSEDITOR_H

#include <memory>

#include <deque>
#include <set>
#include "abstractaction.h"
#include "object.h"
#include "ssobjfile.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Winline"
#pragma GCC diagnostic ignored "-Wcast-qual"
#include <gtkmm.h>
#pragma GCC diagnostic pop

#define IMAGE_SIZE 16

static inline int angle_simple(int angle) {
	return ((angle + 0x40) & 0xff);
}

static inline int angle_normal(int angle) {
	return ((angle + 0xc0) & 0xff);
}

static inline int angle_to_x(int angle) {
	return ((angle + 0x40) & 0xff) * 2 + 4;
}

static inline int x_to_angle(int x) {
	return (((x + (x & 1) - 4) / 2) + 0xc0) & 0xff;
}

static inline int x_to_angle_constrained(int x, int constr = 2) {
	int angle = x + (x & 1) - 4;
	angle -= (angle % (IMAGE_SIZE / constr));
	return ((angle / 2) + 0xc0) & 0xff;
}

static inline int x_constrained(int x, int constr = 2) {
	int pos = x + (x & 1) - 4;
	return pos - (pos % (IMAGE_SIZE / constr)) + 4;
}

static inline int y_constrained(int y, int off) {
	int ty = y + off;
	ty -= (ty % IMAGE_SIZE);
	ty -= off;
	return ty;
}

class sseditor {
private:
	static sseditor *instance;
	bool update_in_progress;
	bool dragging, drop_enabled;

	// State variables.
	std::shared_ptr<ssobj_file> specialstages;
	unsigned currstage, currsegment;
	int first_line;
	int draw_width, draw_height;
	int mouse_x, mouse_y;
	guint state;
	enum EditModes {
		eSelectMode = 0,
		eInsertRingMode,
		eInsertBombMode,
		eDeleteMode,
		eNumModes
	} mode;
	enum InsertModes {
		eSingle = 0,
		eLine,
		eLoop,
		eZigzag,
		eDiamond,
		eLozenge,
		eStar,
		eTriangle,
		eNumInsertModes
	};
	InsertModes ringmode, bombmode;
	std::set<object> selection, hotstack, insertstack, sourcestack, copystack;
	object hotspot, lastclick, selclear, boxcorner;
	std::shared_ptr<sslevels> copylevel;
	std::shared_ptr<sssegments> copyseg;
	int copypos;
	bool drawbox;
	bool snaptogrid;

	std::deque<std::shared_ptr<abstract_action> > undostack, redostack;
	std::vector<size_t> segpos;
	size_t endpos;

	// GUI variables.
	Gtk::Window *main_win;
	std::shared_ptr<Gtk::Main> kit;
	Gtk::MessageDialog *helpdlg;
	Gtk::AboutDialog *aboutdlg;
	Gtk::FileChooserDialog *filedlg;
	Glib::RefPtr<Gtk::Builder> builder;
	Glib::RefPtr<Gdk::Pixbuf> ringimg, bombimg;

	Cairo::RefPtr<Cairo::Pattern> drawimg;

	Glib::RefPtr<Gtk::FileFilter> pfilefilter;
	Gtk::DrawingArea *pspecialstageobjs;
	Gtk::Notebook *pmodenotebook;
	// Labels
	Gtk::Label *plabelcurrentstage, *plabeltotalstages, *plabelcurrentsegment,
	    *plabeltotalsegments,  *plabelcurrsegrings,   *plabelcurrsegbombs,
	    *plabelcurrsegshadows, *plabelcurrsegtotal;
	Gtk::Image *pimagecurrsegwarn;
	// Scrollbar
	Gtk::VScrollbar *pvscrollbar;
	// Main toolbar
	Gtk::ToolButton *popenfilebutton, *psavefilebutton, *prevertfilebutton,
	    *pundobutton, *predobutton, *phelpbutton, *paboutbutton, *pquitbutton;
	Gtk::RadioToolButton *pmodebuttons[eNumModes];
	Gtk::ToggleToolButton *psnapgridbutton;
	// Selection toolbar
	Gtk::ToolButton *pcutbutton, *pcopybutton, *ppastebutton, *pdeletebutton;
	// Insert ring toolbar
	Gtk::RadioToolButton *pringmodebuttons[eNumInsertModes];
	// Insert bomb toolbar
	Gtk::RadioToolButton *pbombmodebuttons[eNumInsertModes];
	// Special stage toolbar
	Gtk::Toolbar    *pstage_toolbar;
	Gtk::ToolButton *pfirst_stage_button, *pprevious_stage_button, *pnext_stage_button,
	    *plast_stage_button, *pinsert_stage_before_button, *pappend_stage_button,
	    *pcut_stage_button, *pcopy_stage_button, *ppaste_stage_button,
	    *pdelete_stage_button, *pswap_stage_prev_button, *pswap_stage_next_button;
	// Segment toolbar
	Gtk::Toolbar    *psegment_toolbar;
	Gtk::ToolButton *pfirst_segment_button, *pprevious_segment_button, *pnext_segment_button,
	    *plast_segment_button, *pinsert_segment_before_button, *pappend_segment_button,
	    *pcut_segment_button, *pcopy_segment_button, *ppaste_segment_button,
	    *pdelete_segment_button, *pswap_segment_prev_button, *pswap_segment_next_button;
	// Segment flags
	Gtk::Expander   *psegment_expander;
	Gtk::RadioButton *pnormal_segment, *pring_message, *pcheckpoint, *pchaos_emerald,
	    *psegment_turnthenrise, *psegment_turnthendrop, * psegment_turnthenstraight,
	    *psegment_straight, *psegment_straightthenturn, *psegment_right, *psegment_left;
	// Object flags
	Gtk::Expander   *pobject_expander;
	Gtk::Button     *pmoveup, *pmovedown, *pmoveleft, *pmoveright;
	Gtk::RadioButton *pringtype, *pbombtype;

	sseditor();
	sseditor(sseditor const &other);
	sseditor(int argc, char *argv[], char const *uifile);

	bool move_object(int dx, int dy);
	void render();
	void show();
	void draw_outlines(std::set<object> &col, Cairo::RefPtr<Cairo::Context> cr) {
		for (const auto & elem : col) {
			int tx = angle_to_x(elem.get_angle()) - IMAGE_SIZE / 2;
			int ty = (segpos[elem.get_segment()] +
			          elem.get_pos() - pvscrollbar->get_value()) * IMAGE_SIZE;
			cr->rectangle(tx, ty, IMAGE_SIZE, IMAGE_SIZE);
			cr->stroke();
		}
	}
	void draw_outlines(std::set<object> &col1, std::set<object> &col2,
	                   Cairo::RefPtr<Cairo::Context> cr) {
		for (const auto & elem : col1) {
			if (col2.find(elem) != col2.end()) {
				continue;
			}
			int tx = angle_to_x(elem.get_angle()) - IMAGE_SIZE / 2;
			int ty = (segpos[elem.get_segment()] +
			          elem.get_pos() - pvscrollbar->get_value()) * IMAGE_SIZE;
			cr->rectangle(tx, ty, IMAGE_SIZE, IMAGE_SIZE);
			cr->stroke();
		}
	}
	void draw_x(std::set<object> &col1, Cairo::RefPtr<Cairo::Context> cr) {
		for (const auto & elem : col1) {
			int tx = angle_to_x(elem.get_angle()) - IMAGE_SIZE / 2;
			int ty = (segpos[elem.get_segment()] +
			          elem.get_pos() - pvscrollbar->get_value()) * IMAGE_SIZE;
			cr->rectangle(tx, ty, IMAGE_SIZE, IMAGE_SIZE);
			cr->move_to(tx, ty);
			cr->line_to(tx + IMAGE_SIZE, ty + IMAGE_SIZE);
			cr->move_to(tx + IMAGE_SIZE, ty);
			cr->line_to(tx, ty + IMAGE_SIZE);
			cr->stroke();
		}
	}
	void draw_objects(std::set<object> &col, Cairo::RefPtr<Cairo::Context> cr) {
		for (const auto & elem : col) {
			Glib::RefPtr<Gdk::Pixbuf> image = (elem.get_type() == sssegments::eBomb)
			                                  ? bombimg : ringimg;
			int tx = angle_to_x(elem.get_angle()) - IMAGE_SIZE / 2;
			int ty = (segpos[elem.get_segment()] +
			          elem.get_pos() - pvscrollbar->get_value()) * IMAGE_SIZE;
			Gdk::Cairo::set_source_pixbuf(cr, image, tx, ty);
			cr->paint_with_alpha(0.5);

			cr->set_line_width(2.0);
			cr->set_source_rgb(1.0, 1.0, 0.0);
			cr->rectangle(tx, ty, image->get_width(), image->get_height());
			cr->stroke();
		}
		if (col.size()) {
		}
	}
	void object_triangle(int x, int y, int dx, int dy, int h,
	                     sssegments::ObjectTypes type, bool fill,
	                     std::set<object> &col);
	void update_segment_positions(bool setpos);
	size_t get_current_segment() const;
	size_t get_segment(size_t pos) const;
	void goto_segment(unsigned seg) {
		currsegment = seg;
		pvscrollbar->set_value(segpos[seg]);
	}
	void do_action(std::shared_ptr<abstract_action> act) {
		redostack.clear();
		abstract_action::MergeResult ret;
		if (!undostack.size()
		        || (ret = undostack.front()->merge(act)) == abstract_action::eNoMerge) {
			if (undostack.size() == 100) {
				undostack.pop_back();
			}
			undostack.push_front(act);
		} else if (ret == abstract_action::eDeleteAction) {
			undostack.pop_front();
		}
		act->apply(specialstages, static_cast<std::set<object> *>(nullptr));
	}
public:
	static sseditor *create_instance(int argc, char *argv[], char const *uifile) {
		if (!instance) {
			instance = new sseditor(argc, argv, uifile);
		}
		return instance;
	}
	static sseditor *get_instance() {
		return instance;
	}
	void run();

	bool on_specialstageobjs_configure_event(GdkEventConfigure *event);
	void on_specialstageobjs_drag_data_received(Glib::RefPtr<Gdk::DragContext> const &context,
	        int x, int y,
	        Gtk::SelectionData const &selection_data,
	        guint info, guint time);
	bool on_specialstageobjs_expose_event(GdkEventExpose *event);
	bool on_specialstageobjs_key_press_event(GdkEventKey *event);
	bool on_specialstageobjs_button_press_event(GdkEventButton *event);
	bool on_specialstageobjs_button_release_event(GdkEventButton *event);
	bool on_specialstageobjs_scroll_event(GdkEventScroll *event);
	void on_specialstageobjs_drag_begin(Glib::RefPtr<Gdk::DragContext> const &targets);
	bool on_specialstageobjs_motion_notify_event(GdkEventMotion *event);
	void on_specialstageobjs_drag_data_get(Glib::RefPtr<Gdk::DragContext> const &targets,
	                                       Gtk::SelectionData &selection_data,
	                                       guint info, guint time);
	void on_specialstageobjs_drag_data_delete(Glib::RefPtr<Gdk::DragContext> const &context);
	void on_specialstageobjs_drag_end(Glib::RefPtr<Gdk::DragContext> const &context);
	bool on_drag_motion(Glib::RefPtr<Gdk::DragContext> const &context,
	                    int x, int y, guint time);
	// Scrollbar
	void on_vscrollbar_value_changed();
	// Main toolbar
	void on_filedialog_response(int response_id);
	void on_openfilebutton_clicked();
	void on_savefilebutton_clicked();
	void on_revertfilebutton_clicked();
	void on_undobutton_clicked();
	void on_redobutton_clicked();
	template<EditModes N>
	void on_modebutton_toggled() {
		pmodenotebook->set_current_page(int(N));
		mode = N;
		selection.clear();
		hotstack.clear();
		insertstack.clear();
		sourcestack.clear();
		update();
	}
	void on_snapgridbutton_toggled() {
		snaptogrid = psnapgridbutton->get_active();
	}
	void on_helpdialog_response(int response_id);
	void on_helpbutton_clicked();
	void on_aboutdialog_response(int response_id);
	void on_aboutbutton_clicked();
	void on_quitbutton_clicked();
	// Selection toolbar
	void on_cutbutton_clicked();
	void on_copybutton_clicked();
	void on_pastebutton_clicked();
	void on_deletebutton_clicked();
	// Insert ring toolbar
	template <InsertModes N>
	void on_ringmode_toggled() {
		ringmode = N;
		update();
	}
	// Insert bomb toolbar
	template <InsertModes N>
	void on_bombmode_toggled() {
		bombmode = N;
		update();
	}
	// Special stage toolbar
	void on_first_stage_button_clicked();
	void on_previous_stage_button_clicked();
	void on_next_stage_button_clicked();
	void on_last_stage_button_clicked();
	void on_insert_stage_before_button_clicked();
	void on_append_stage_button_clicked();
	void on_cut_stage_button_clicked();
	void on_copy_stage_button_clicked();
	void on_paste_stage_button_clicked();
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
	void on_cut_segment_button_clicked();
	void on_copy_segment_button_clicked();
	void on_paste_segment_button_clicked();
	void on_delete_segment_button_clicked();
	void on_swap_segment_prev_button_clicked();
	void on_swap_segment_next_button_clicked();
	// Segment flags
	template <sssegments::SegmentTypes N, Gtk::RadioButton *sseditor::*btn>
	void on_segmenttype_toggled() {
		if (!specialstages || update_in_progress) {
			return;
		}
		if (!(this->*btn)->get_active()) {
			return;
		}
		sslevels *currlvl = specialstages->get_stage(currstage);
		//size_t numsegments = currlvl->num_segments();
		sssegments *currseg = currlvl->get_segment(currsegment);

		std::shared_ptr<abstract_action>
		act(new alter_segment_action(currstage, currsegment, *currseg,
		                             currseg->get_direction(), N,
		                             currseg->get_geometry()));
		do_action(act);
	}
	template <sssegments::SegmentGeometry N, Gtk::RadioButton *sseditor::*btn>
	void on_segment_segmentgeometry_toggled() {
		if (!specialstages || update_in_progress) {
			return;
		}
		if (!(this->*btn)->get_active()) {
			return;
		}
		sslevels *currlvl = specialstages->get_stage(currstage);
		//size_t numsegments = currlvl->num_segments();
		sssegments *currseg = currlvl->get_segment(currsegment);

		std::shared_ptr<abstract_action>
		act(new alter_segment_action(currstage, currsegment, *currseg,
		                             currseg->get_direction(), currseg->get_type(),
		                             N));
		do_action(act);
		update_segment_positions(false);
	}
	template <bool tf, Gtk::RadioButton *sseditor::*btn>
	void on_segmentdirection_toggled() {
		if (!specialstages || update_in_progress) {
			return;
		}
		if (!(this->*btn)->get_active()) {
			return;
		}
		sslevels *currlvl = specialstages->get_stage(currstage);
		//size_t numsegments = currlvl->num_segments();
		sssegments *currseg = currlvl->get_segment(currsegment);

		std::shared_ptr<abstract_action>
		act(new alter_segment_action(currstage, currsegment, *currseg,
		                             tf, currseg->get_type(),
		                             currseg->get_geometry()));
		do_action(act);
	}
	// Object flags
	template <int dx, int dy>
	void on_movebtn_clicked() {
		move_object(dx, dy);
	}
	template <sssegments::ObjectTypes N, Gtk::RadioButton *sseditor::*btn>
	void on_objecttype_toggled() {
		if (!specialstages || selection.empty() || update_in_progress) {
			return;
		}
		if (!(this->*btn)->get_active()) {
			return;
		}
		std::shared_ptr<abstract_action>
		act(new alter_selection_action(currstage, N, selection));
		do_action(act);

		std::set<object> temp;
		//sslevels *currlvl = specialstages->get_stage(currstage);
		for (const auto & elem : selection) {
			//sssegments *currseg = currlvl->get_segment(it->get_segment());
			temp.insert(object(elem.get_segment(), elem.get_angle(),
			                   elem.get_pos(), N));
		}
		selection.swap(temp);

		render();
		update();
		show();
	}
protected:
	void update();
};

#endif // __SSEDITOR_H
