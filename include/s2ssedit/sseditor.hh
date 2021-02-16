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
#include <tuple>

#include <s2ssedit/abstractaction.hh>
#include <s2ssedit/object.hh>
#include <s2ssedit/ssobjfile.hh>

#include <gtkmm.h>

#define SIMAGE_SIZE 16
#define IMAGE_SIZE 16U
#define HALF_IMAGE_SIZE 8
#define QUARTER_IMAGE_SIZE 4

constexpr const int32_t center_x    = 0x40;
constexpr const int32_t right_angle = 0x80;

inline int32_t angle_simple(int angle) { return (angle + center_x) % 256; }

inline int32_t angle_normal(int angle) {
    return (angle + center_x + right_angle) % 256;
}

inline int angle_to_x(int angle) { return ((angle + center_x) % 256) * 2 + 4; }

inline uint8_t x_to_angle(int32_t x, bool constrain, int32_t snapoff = 0U) {
    int32_t angle = x + (x % 2) - 4;
    if (constrain) {
        angle += snapoff;
        angle -= (angle % (IMAGE_SIZE / 2U));
    }
    return static_cast<uint8_t>(angle / 2U) + center_x + right_angle;
}

inline int32_t x_constrained(int32_t x, int32_t constr = 2) {
    unsigned pos = x + (x % 2) - 4;
    return pos - (pos % (IMAGE_SIZE / constr)) + 4;
}

inline int32_t y_constrained(int32_t y, int32_t off) {
    unsigned ty = y + off;
    ty -= (ty % IMAGE_SIZE);
    ty -= off;
    return ty;
}

template <typename T>
T clamp(T val, T min, T max) {
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}

template <typename T>
T signum(T val) {
    return (T(0) < val) - (val < T(0));
}

template <typename T>
T sigplus(T val) {
    return (T(0) <= val) - (val < T(0));
}

class sseditor {
private:
    using ObjectTypes = sssegments::ObjectTypes;
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

    std::vector<int> segpos;

    int endpos;

    // GUI variables.
    Gtk::Window*                   main_win;
    Glib::RefPtr<Gtk::Application> kit;
    Gtk::MessageDialog*            helpdlg;
    Gtk::AboutDialog*              aboutdlg;
    Gtk::FileChooserDialog*        filedlg;
    Glib::RefPtr<Gtk::Builder>     builder;
    Glib::RefPtr<Gdk::Pixbuf>      ringimg, bombimg;

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
    Gtk::Scrollbar* pvscrollbar;
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
    Gtk::Grid*        psegment_grid;
    Gtk::RadioButton *pnormal_segment, *pring_message, *pcheckpoint,
        *pchaos_emerald, *psegment_turnthenrise, *psegment_turnthendrop,
        *psegment_turnthenstraight, *psegment_straight,
        *psegment_straightthenturn, *psegment_right, *psegment_left;
    // Object flags
    Gtk::Grid*        pobject_grid;
    Gtk::Button *     pmoveup, *pmovedown, *pmoveleft, *pmoveright;
    Gtk::RadioButton *pringtype, *pbombtype;

    bool move_object(int dx, int dy);
    void render() const { pspecialstageobjs->queue_draw(); }
    void show();

    static double get_obj_x(const object& obj) {
        return static_cast<double>(
            angle_to_x(obj.get_angle()) - HALF_IMAGE_SIZE);
    }
    double get_obj_y(const object& obj) {
        return (get_obj_pos<double>(obj) - pvscrollbar->get_value()) *
               SIMAGE_SIZE;
    }
    void draw_outlines(
        std::set<object>& col, Cairo::RefPtr<Cairo::Context> const& cr) {
        for (auto const& elem : col) {
            auto tx = get_obj_x(elem);
            auto ty = get_obj_y(elem);
            cr->rectangle(tx, ty, SIMAGE_SIZE, SIMAGE_SIZE);
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
            cr->rectangle(tx, ty, SIMAGE_SIZE, SIMAGE_SIZE);
            cr->stroke();
        }
    }
    void
    draw_x(std::set<object>& col1, Cairo::RefPtr<Cairo::Context> const& cr) {
        for (auto const& elem : col1) {
            auto tx = get_obj_x(elem);
            auto ty = get_obj_y(elem);
            cr->rectangle(tx, ty, SIMAGE_SIZE, SIMAGE_SIZE);
            cr->move_to(tx, ty);
            cr->line_to(tx + SIMAGE_SIZE, ty + SIMAGE_SIZE);
            cr->move_to(tx + SIMAGE_SIZE, ty);
            cr->line_to(tx, ty + SIMAGE_SIZE);
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
    size_t find_segment(int pos) const;
    void   goto_segment(unsigned seg) {
        currsegment = seg;
        pvscrollbar->set_value(segpos[seg]);
    }
    template <typename Act, typename... Args>
    void do_action(Args&&... args) {
        using MergeResult = abstract_action::MergeResult;
        redostack.clear();
        auto act = std::make_shared<Act>(std::forward<Args>(args)...);
        if (undostack.empty()) {
            undostack.push_front(act);
        } else {
            const MergeResult ret = undostack.front()->merge(act);
            if (ret == abstract_action::eNoMerge) {
                undostack.push_front(act);
            } else if (ret == abstract_action::eDeleteAction) {
                undostack.pop_front();
            }
        }
        act->apply(specialstages, static_cast<std::set<object>*>(nullptr));
    }
    int get_scroll() const {
        return static_cast<int>(pvscrollbar->get_value());
    }

public:
    sseditor(Glib::RefPtr<Gtk::Application> application, char const* uifile);

    void run();

    bool on_specialstageobjs_configure_event(GdkEventConfigure* event);
    void on_specialstageobjs_drag_data_received(
        Glib::RefPtr<Gdk::DragContext> const& context, int x, int y,
        Gtk::SelectionData const& selection_data, guint info, guint time);
    bool
    on_specialstageobjs_expose_event(const Cairo::RefPtr<Cairo::Context>& cr);
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
    bool want_snap_to_grid(guint istate) const noexcept {
        return static_cast<unsigned int>(snaptogrid) !=
               (istate & GDK_CONTROL_MASK);
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

        do_action<alter_segment_action>(
            currstage, currsegment, *currseg, currseg->get_direction(), N,
            currseg->get_geometry());
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

        do_action<alter_segment_action>(
            currstage, currsegment, *currseg, currseg->get_direction(),
            currseg->get_type(), N);
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

        do_action<alter_segment_action>(
            currstage, currsegment, *currseg, tf, currseg->get_type(),
            currseg->get_geometry());
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
        do_action<alter_selection_action>(currstage, N, selection);
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

private:
    template <typename T>
    T get_obj_pos(object obj) const {
        return static_cast<T>(segpos[obj.get_segment()] + obj.get_pos());
    }
    void delete_set(std::set<object>& toDel);
    void delete_set(std::set<object>&& toDel);
    void delete_existing_object(int seg, unsigned pos, unsigned angle);
    void delete_object(int seg, unsigned x, unsigned y, ObjectTypes t) {
        delete_set({object(seg, x, y, t)});
    }
    void cycle_object_type(int seg, unsigned pos, unsigned angle);
    void finalize_selection();
    void insert_set();
    void finish_drag_box(GdkEventButton* event) {
        if (event->button == GDK_BUTTON_PRIMARY && drawbox) {
            drawbox = false;
        }
    }
    sssegments* get_segment(int seg);

    std::tuple<int, int, int> get_mouseup_loc(GdkEventButton* event);

    void increment_mode(bool ctrl);
    void decrement_mode(bool ctrl);
    void increment_editmode() {
        switch (mode) {
        case eSelectMode:
            mode = eInsertRingMode;
            break;
        case eInsertRingMode:
            mode = eInsertBombMode;
            break;
        case eInsertBombMode:
            mode = eDeleteMode;
            break;
        case eDeleteMode:
            mode = eSelectMode;
            break;
        case eNumModes:
            __builtin_unreachable();
        }
    }
    void decrement_editmode() {
        switch (mode) {
        case eSelectMode:
            mode = eDeleteMode;
            break;
        case eInsertRingMode:
            mode = eSelectMode;
            break;
        case eInsertBombMode:
            mode = eInsertRingMode;
            break;
        case eDeleteMode:
            mode = eInsertBombMode;
            break;
        case eNumModes:
            __builtin_unreachable();
        }
    }

    static void increment_insertmode(InsertModes& ins) {
        switch (ins) {
        case eSingle:
            ins = eLine;
            break;
        case eLine:
            ins = eLoop;
            break;
        case eLoop:
            ins = eZigzag;
            break;
        case eZigzag:
            ins = eDiamond;
            break;
        case eDiamond:
            ins = eLozenge;
            break;
        case eLozenge:
            ins = eStar;
            break;
        case eStar:
            ins = eTriangle;
            break;
        case eTriangle:
            ins = eSingle;
            break;
        case eNumInsertModes:
            __builtin_unreachable();
        }
    }
    static void decrement_insertmode(InsertModes& ins) {
        switch (ins) {
        case eSingle:
            ins = eTriangle;
            break;
        case eLine:
            ins = eSingle;
            break;
        case eLoop:
            ins = eLine;
            break;
        case eZigzag:
            ins = eLoop;
            break;
        case eDiamond:
            ins = eZigzag;
            break;
        case eLozenge:
            ins = eDiamond;
            break;
        case eStar:
            ins = eLozenge;
            break;
        case eTriangle:
            ins = eStar;
            break;
        case eNumInsertModes:
            __builtin_unreachable();
        }
    }

    static void draw_balls(Cairo::RefPtr<Cairo::Context> const& cr, int ty);

    void cleanup_render(Cairo::RefPtr<Cairo::Context> const& cr);
    void
    draw_objects(Cairo::RefPtr<Cairo::Context> const& cr, int start, int end);
    bool want_checkerboard(int row, int seg, sssegments* currseg);
    void draw_box(Cairo::RefPtr<Cairo::Context> const& cr);
    void select_hotspot();
    void fix_stage(unsigned numstages) {
        if (numstages == 0) {
            currstage = 0;
        } else if (currstage >= numstages) {
            currstage = numstages - 1;
        }
    }
    void fix_segment(unsigned numsegments) {
        if (numsegments == 0) {
            currsegment = 0;
        } else if (currstage >= numsegments) {
            currsegment = numsegments - 1;
        }
    }
    Gtk::RadioButton* segment_type_button(sssegments::SegmentTypes type) {
        switch (type) {
        case sssegments::eRingsMessage:
            return pring_message;
        case sssegments::eCheckpoint:
            return pcheckpoint;
        case sssegments::eChaosEmerald:
            return pchaos_emerald;
        case sssegments::eNormalSegment:
            return pnormal_segment;
        }
        __builtin_unreachable();
    }
    Gtk::RadioButton* geometry_button(sssegments::SegmentGeometry type) {
        switch (type) {
        case sssegments::eTurnThenDrop:
            return psegment_turnthendrop;
        case sssegments::eTurnThenStraight:
            return psegment_turnthenstraight;
        case sssegments::eStraight:
            return psegment_straight;
        case sssegments::eStraightThenTurn:
            return psegment_straightthenturn;
        case sssegments::eTurnThenRise:
            return psegment_turnthenrise;
        }
        __builtin_unreachable();
    }
    Gtk::RadioButton* direction_button(bool dir) {
        return dir ? psegment_left : psegment_right;
    }
    static std::pair<size_t, size_t> count_objects(std::set<object>& objs) {
        size_t nrings = 0;
        size_t nbombs = 0;
        for (auto const& elem : objs) {
            if (elem.get_type() == sssegments::eBomb) {
                ++nbombs;
            } else {
                ++nrings;
            }
        }
        return std::pair<size_t, size_t>{nrings, nbombs};
    }
    std::pair<int, int> get_motion_loc(GdkEventMotion* event) {
        int angle;
        int pos;
        if (!hotspot.valid()) {
            angle = x_to_angle(event->x, want_snap_to_grid(state), 4U);
            pos   = static_cast<int>(
                event->y / SIMAGE_SIZE + pvscrollbar->get_value());
        } else {
            angle = hotspot.get_angle();
            pos   = get_obj_pos<int>(hotspot);
        }
        return std::pair<int, int>{angle_simple(angle), pos};
    }
    std::pair<ObjectTypes, InsertModes> get_obj_type() const noexcept {
        if (mode == eInsertBombMode) {
            return std::pair<ObjectTypes, InsertModes>{
                sssegments::eBomb, bombmode};
        }
        return std::pair<ObjectTypes, InsertModes>{sssegments::eRing, ringmode};
    }

    void motion_update_select_insert(GdkEventMotion* event);
    void motion_update_selection(
        int dangle, int dpos, int pos0, int pos1, int angle0, int angle1);
    void motion_update_insertion(
        int dangle, int dpos, int pos0, int pos1, int angle0, int angle1,
        bool grid, bool lbutton_pressed);
    void motion_update_line(
        int dpos, int pos0, int pos1, int angle0, ObjectTypes type,
        int angledelta);
    void motion_update_loop(
        int dpos, int pos0, int angle0, ObjectTypes type, int angledelta,
        bool grid);
    void motion_update_zigzag(
        int dpos, int pos0, int pos1, int angle0, ObjectTypes type,
        int angledelta);
    void motion_update_diamond(
        int dpos, int pos0, int pos1, int angle0, ObjectTypes type,
        int angledelta);
    void motion_update_star_lozenge(
        int dpos, int pos0, int pos1, int angle0, ObjectTypes type,
        int angledelta, bool fill);
    void scroll_into_view(GdkEventMotion* event);

    static int motion_compute_angledelta(
        int dpos, InsertModes submode, bool grid, int angledelta);

protected:
    void update();
    void disable_scroll() {
        constexpr const double no_scroll = 0.1;
        pvscrollbar->set_value(0.0);
        pvscrollbar->set_range(0.0, no_scroll);
    }
};

#endif // SSEDITOR_H
