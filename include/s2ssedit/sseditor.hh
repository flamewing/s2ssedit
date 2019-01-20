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

#ifndef SSEDITOR_H
#define SSEDITOR_H

#include <array>
#include <deque>
#include <memory>
#include <set>

#include <s2ssedit/abstractaction.hh>
#include <s2ssedit/object.hh>
#include <s2ssedit/ssobjfile.hh>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Winline"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wextra-semi"
#ifndef __clang__
#    pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <gtkmm.h>
#pragma GCC diagnostic pop

#define IMAGE_SIZE 16U
#define HALF_IMAGE_SIZE 8
#define QUARTER_IMAGE_SIZE 4

constexpr const unsigned center_x    = 0x40U;
constexpr const unsigned right_angle = 0x80U;

inline uint8_t angle_simple(uint8_t angle) { return angle + center_x; }

inline uint8_t angle_normal(uint8_t angle) {
    return angle + center_x + right_angle;
}

inline uint8_t angle_to_x(uint8_t angle) {
    return (angle + center_x) * 2U + 4U;
}

inline uint8_t x_to_angle(uint8_t x) {
    return ((x + (x & 1U) - 4U) / 2U) + center_x + right_angle;
}

inline uint8_t x_to_angle_constrained(uint8_t x, uint8_t constr = 2U) {
    uint8_t angle = x + (x & 1U) - 4U;
    angle -= (angle % (IMAGE_SIZE / constr));
    return (angle / 2U) + center_x + right_angle;
}

inline unsigned x_constrained(unsigned x, unsigned constr = 2) {
    unsigned pos = x + (x & 1U) - 4U;
    return pos - (pos % (IMAGE_SIZE / constr)) + 4U;
}

inline unsigned y_constrained(unsigned y, unsigned off) {
    unsigned ty = y + off;
    ty -= (ty % IMAGE_SIZE);
    ty -= off;
    return ty;
}

class sseditor {
private:
    bool update_in_progress;
    bool dragging, drop_enabled;

    // State variables.
    std::shared_ptr<ssobj_file> specialstages;

    unsigned currstage, currsegment;
    int      draw_width, draw_height;
    int      mouse_x, mouse_y;
    guint    state;

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

    std::shared_ptr<sslevels>   copylevel;
    std::shared_ptr<sssegments> copyseg;

    int  copypos;
    bool drawbox;
    bool snaptogrid;

    std::deque<std::shared_ptr<abstract_action>> undostack, redostack;

    std::vector<size_t> segpos;

    size_t endpos;

    // GUI variables.
    Gtk::Window*               main_win;
    std::shared_ptr<Gtk::Main> kit;
    Gtk::MessageDialog*        helpdlg;
    Gtk::AboutDialog*          aboutdlg;
    Gtk::FileChooserDialog*    filedlg;
    Glib::RefPtr<Gtk::Builder> builder;
    Glib::RefPtr<Gdk::Pixbuf>  ringimg, bombimg;

    Cairo::RefPtr<Cairo::Pattern> drawimg;

    Glib::RefPtr<Gtk::FileFilter> pfilefilter;
    Gtk::DrawingArea*             pspecialstageobjs;
    Gtk::Notebook*                pmodenotebook;
    // Labels
    Gtk::Label *plabelcurrentstage, *plabeltotalstages, *plabelcurrentsegment,
        *plabeltotalsegments, *plabelcurrsegrings, *plabelcurrsegbombs,
        *plabelcurrsegshadows, *plabelcurrsegtotal;
    Gtk::Image* pimagecurrsegwarn;
    // Scrollbar
    Gtk::VScrollbar* pvscrollbar;
    // Main toolbar
    Gtk::ToolButton *popenfilebutton, *psavefilebutton, *prevertfilebutton,
        *pundobutton, *predobutton, *phelpbutton, *paboutbutton, *pquitbutton;
    std::array<Gtk::RadioToolButton*, eNumModes> pmodebuttons;
    Gtk::ToggleToolButton*                       psnapgridbutton;
    // Selection toolbar
    Gtk::ToolButton *pcutbutton, *pcopybutton, *ppastebutton, *pdeletebutton;
    // Insert ring toolbar
    std::array<Gtk::RadioToolButton*, eNumInsertModes> pringmodebuttons;
    // Insert bomb toolbar
    std::array<Gtk::RadioToolButton*, eNumInsertModes> pbombmodebuttons;
    // Special stage toolbar
    Gtk::Toolbar*    pstage_toolbar;
    Gtk::ToolButton *pfirst_stage_button, *pprevious_stage_button,
        *pnext_stage_button, *plast_stage_button, *pinsert_stage_before_button,
        *pappend_stage_button, *pcut_stage_button, *pcopy_stage_button,
        *ppaste_stage_button, *pdelete_stage_button, *pswap_stage_prev_button,
        *pswap_stage_next_button;
    // Segment toolbar
    Gtk::Toolbar*    psegment_toolbar;
    Gtk::ToolButton *pfirst_segment_button, *pprevious_segment_button,
        *pnext_segment_button, *plast_segment_button,
        *pinsert_segment_before_button, *pappend_segment_button,
        *pcut_segment_button, *pcopy_segment_button, *ppaste_segment_button,
        *pdelete_segment_button, *pswap_segment_prev_button,
        *pswap_segment_next_button;
    // Segment flags
    Gtk::Expander*    psegment_expander;
    Gtk::RadioButton *pnormal_segment, *pring_message, *pcheckpoint,
        *pchaos_emerald, *psegment_turnthenrise, *psegment_turnthendrop,
        *psegment_turnthenstraight, *psegment_straight,
        *psegment_straightthenturn, *psegment_right, *psegment_left;
    // Object flags
    Gtk::Expander*    pobject_expander;
    Gtk::Button *     pmoveup, *pmovedown, *pmoveleft, *pmoveright;
    Gtk::RadioButton *pringtype, *pbombtype;

    bool   move_object(int dx, int dy);
    void   render();
    void   show();
    double get_obj_x(const object& obj) {
        return static_cast<double>(
            angle_to_x(obj.get_angle()) - HALF_IMAGE_SIZE);
    }
    double get_obj_y(const object& obj) {
        return (static_cast<double>(segpos[obj.get_segment()] + obj.get_pos()) -
                pvscrollbar->get_value()) *
               IMAGE_SIZE;
    }
    void draw_outlines(
        std::set<object>& col, Cairo::RefPtr<Cairo::Context> const& cr) {
        for (auto const& elem : col) {
            auto tx = get_obj_x(elem);
            auto ty = get_obj_y(elem);
            cr->rectangle(tx, ty, IMAGE_SIZE, IMAGE_SIZE);
            cr->stroke();
        }
    }
    void draw_outlines(
        std::set<object>& col1, std::set<object>& col2,
        Cairo::RefPtr<Cairo::Context> const& cr) {
        for (auto const& elem : col1) {
            if (col2.find(elem) != col2.end()) {
                continue;
            }
            auto tx = get_obj_x(elem);
            auto ty = get_obj_y(elem);
            cr->rectangle(tx, ty, IMAGE_SIZE, IMAGE_SIZE);
            cr->stroke();
        }
    }
    void
    draw_x(std::set<object>& col1, Cairo::RefPtr<Cairo::Context> const& cr) {
        for (auto const& elem : col1) {
            auto tx = get_obj_x(elem);
            auto ty = get_obj_y(elem);
            cr->rectangle(tx, ty, IMAGE_SIZE, IMAGE_SIZE);
            cr->move_to(tx, ty);
            cr->line_to(tx + IMAGE_SIZE, ty + IMAGE_SIZE);
            cr->move_to(tx + IMAGE_SIZE, ty);
            cr->line_to(tx, ty + IMAGE_SIZE);
            cr->stroke();
        }
    }
    void draw_objects(
        std::set<object>& col, Cairo::RefPtr<Cairo::Context> const& cr) {
        for (auto const& elem : col) {
            Glib::RefPtr<Gdk::Pixbuf> image =
                (elem.get_type() == sssegments::eBomb) ? bombimg : ringimg;
            auto tx = get_obj_x(elem);
            auto ty = get_obj_y(elem);
            Gdk::Cairo::set_source_pixbuf(cr, image, tx, ty);
            constexpr const double half_transparency = 0.5;
            cr->paint_with_alpha(half_transparency);

            constexpr const double thick_line = 2.0;
            cr->set_line_width(thick_line);
            cr->set_source_rgb(1.0, 1.0, 0.0);
            cr->rectangle(tx, ty, image->get_width(), image->get_height());
            cr->stroke();
        }
    }
    void object_triangle(
        int x, int y, int dx, int dy, int h, sssegments::ObjectTypes type,
        bool fill, std::set<object>& col);
    void   update_segment_positions(bool setpos);
    size_t get_current_segment() const;
    size_t get_segment(size_t pos) const;
    void   goto_segment(unsigned seg) {
        currsegment = seg;
        pvscrollbar->set_value(segpos[seg]);
    }
    void do_action(std::shared_ptr<abstract_action> const& act) {
        redostack.clear();
        abstract_action::MergeResult ret;
        if (undostack.empty() || (ret = undostack.front()->merge(act)) ==
                                     abstract_action::eNoMerge) {
            undostack.push_front(act);
        } else if (ret == abstract_action::eDeleteAction) {
            undostack.pop_front();
        }
        act->apply(specialstages, static_cast<std::set<object>*>(nullptr));
    }

public:
    sseditor(int argc, char* argv[], char const* uifile);

    void run();

    bool on_specialstageobjs_configure_event(GdkEventConfigure* event);
    void on_specialstageobjs_drag_data_received(
        Glib::RefPtr<Gdk::DragContext> const& context, int x, int y,
        Gtk::SelectionData const& selection_data, guint info, guint time);
    bool on_specialstageobjs_expose_event(GdkEventExpose* event);
    bool on_specialstageobjs_key_press_event(GdkEventKey* event);
    bool on_specialstageobjs_button_press_event(GdkEventButton* event);
    bool on_specialstageobjs_button_release_event(GdkEventButton* event);
    bool on_specialstageobjs_scroll_event(GdkEventScroll* event);
    void on_specialstageobjs_drag_begin(
        Glib::RefPtr<Gdk::DragContext> const& targets);
    bool on_specialstageobjs_motion_notify_event(GdkEventMotion* event);
    void on_specialstageobjs_drag_data_get(
        Glib::RefPtr<Gdk::DragContext> const& targets,
        Gtk::SelectionData& selection_data, guint info, guint time);
    void on_specialstageobjs_drag_data_delete(
        Glib::RefPtr<Gdk::DragContext> const& context);
    void
         on_specialstageobjs_drag_end(Glib::RefPtr<Gdk::DragContext> const& context);
    bool on_drag_motion(
        Glib::RefPtr<Gdk::DragContext> const& context, int x, int y,
        guint time);
    // Scrollbar
    void on_vscrollbar_value_changed();
    // Main toolbar
    void on_filedialog_response(int response_id);
    void on_openfilebutton_clicked();
    void on_savefilebutton_clicked();
    void on_revertfilebutton_clicked();
    void on_undobutton_clicked();
    void on_redobutton_clicked();
    template <EditModes N>
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
    template <sssegments::SegmentTypes N, Gtk::RadioButton* sseditor::*btn>
    void on_segmenttype_toggled() {
        if (!specialstages || update_in_progress) {
            return;
        }
        if (!(this->*btn)->get_active()) {
            return;
        }
        sslevels* currlvl = specialstages->get_stage(currstage);
        // size_t numsegments = currlvl->num_segments();
        sssegments* currseg = currlvl->get_segment(currsegment);

        auto act = std::make_shared<alter_segment_action>(
            currstage, currsegment, *currseg, currseg->get_direction(), N,
            currseg->get_geometry());
        do_action(act);
    }
    template <sssegments::SegmentGeometry N, Gtk::RadioButton* sseditor::*btn>
    void on_segment_segmentgeometry_toggled() {
        if (!specialstages || update_in_progress) {
            return;
        }
        if (!(this->*btn)->get_active()) {
            return;
        }
        sslevels* currlvl = specialstages->get_stage(currstage);
        // size_t numsegments = currlvl->num_segments();
        sssegments* currseg = currlvl->get_segment(currsegment);

        auto act = std::make_shared<alter_segment_action>(
            currstage, currsegment, *currseg, currseg->get_direction(),
            currseg->get_type(), N);
        do_action(act);
        update_segment_positions(false);
    }
    template <bool tf, Gtk::RadioButton* sseditor::*btn>
    void on_segmentdirection_toggled() {
        if (!specialstages || update_in_progress) {
            return;
        }
        if (!(this->*btn)->get_active()) {
            return;
        }
        sslevels* currlvl = specialstages->get_stage(currstage);
        // size_t numsegments = currlvl->num_segments();
        sssegments* currseg = currlvl->get_segment(currsegment);

        auto act = std::make_shared<alter_segment_action>(
            currstage, currsegment, *currseg, tf, currseg->get_type(),
            currseg->get_geometry());
        do_action(act);
    }
    // Object flags
    template <int dx, int dy>
    void on_movebtn_clicked() {
        move_object(dx, dy);
    }
    template <sssegments::ObjectTypes N, Gtk::RadioButton* sseditor::*btn>
    void on_objecttype_toggled() {
        if (!specialstages || selection.empty() || update_in_progress) {
            return;
        }
        if (!(this->*btn)->get_active()) {
            return;
        }
        auto act =
            std::make_shared<alter_selection_action>(currstage, N, selection);
        do_action(act);

        std::set<object> temp;
        // sslevels *currlvl = specialstages->get_stage(currstage);
        for (auto const& elem : selection) {
            // sssegments *currseg = currlvl->get_segment(it->get_segment());
            temp.emplace(
                elem.get_segment(), elem.get_angle(), elem.get_pos(), N);
        }
        selection.swap(temp);

        render();
        update();
        show();
    }

protected:
    void update();
    void disable_scroll() {
        constexpr const double no_scroll = 0.1;
        pvscrollbar->set_value(0.0);
        pvscrollbar->set_range(0.0, no_scroll);
    }
};

#endif // SSEDITOR_H
