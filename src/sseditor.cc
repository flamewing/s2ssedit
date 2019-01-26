/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * S2-SSEdit
 * Copyright (C) Flamewing 2011-2015 <flamewing.sonic@gmail.com>
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

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>

#include <s2ssedit/sseditor.hh>

#include <mdcomp/bigendian_io.hh>

#include <s2ssedit/ignore_unused_variable_warning.hh>
#include <s2ssedit/ssobjfile.hh>

using std::cout;
using std::endl;
using std::ios;
using std::make_shared;
using std::max;
using std::min;
using std::set;
using std::string;
using std::stringstream;
using std::swap;
using std::tie;
using std::tuple;
using std::vector;

size_t sseditor::get_current_segment() const {
    auto pos = size_t(pvscrollbar->get_value());
    return find_segment(pos);
}

size_t sseditor::find_segment(size_t pos) const {
    auto   it  = lower_bound(segpos.begin(), segpos.end(), pos);
    size_t seg = size_t(it - segpos.begin());
    if (*it == pos) {
        return seg;
    }
    return seg - 1;
}

void sseditor::update_segment_positions(bool setpos) {
    if (!specialstages) {
        disable_scroll();
        segpos.clear();
        endpos = 0;
        return;
    }
    constexpr const double range_offset = 9.0;
    constexpr const double step_size    = 4.0;
    constexpr const double page_incr    = 32.0;
    auto                   start_pos    = draw_height / IMAGE_SIZE;
    endpos = specialstages->get_stage(currstage)->fill_position_array(segpos);
    cout << endpos << "\t" << segpos.back() << "\t" << start_pos << endl;
    pvscrollbar->set_range(
        0.0, static_cast<double>(endpos) + range_offset - start_pos);
    pvscrollbar->set_increments(step_size, page_incr);
    if (setpos) {
        goto_segment(currsegment);
    }
}

bool sseditor::move_object(int dx, int dy) {
    if (!specialstages || selection.empty()) {
        return true;
    }

    set<object> temp;

    for (auto const& elem : selection) {
        int  oldseg = elem.get_segment();
        auto oldx   = elem.get_angle();
        auto oldy   = elem.get_pos();

        sssegments* currseg = get_segment(oldseg);
        if (currseg == nullptr || !currseg->exists(oldy, oldx)) {
            continue;
        }

        constexpr const int max_y = 0x40;

        int newx = static_cast<int8_t>(oldx + dx);
        int newy = (oldy + dy) % max_y;
        int cnt  = max_y;

        ObjectTypes type;
        while (cnt > 0 && currseg->exists(newy, newx, type) &&
               selection.find(object(oldseg, newx, newy, type)) ==
                   selection.end()) {
            newx = static_cast<int8_t>(newx + dx);
            newy += dy;
            newy %= max_y;
            cnt--;
        }

        int pos = (newy + segpos[oldseg]) * IMAGE_SIZE,
            loc = pvscrollbar->get_value() * IMAGE_SIZE;
        if (pos < loc || pos >= loc + draw_height) {
            pvscrollbar->set_value(pos / IMAGE_SIZE);
        }

        temp.emplace(elem.get_segment(), newx, newy, elem.get_type());
    }

    do_action<move_objects_action>(currstage, selection, temp);
    selection.swap(temp);
    render();
    update();
    return true;
}

bool sseditor::on_specialstageobjs_configure_event(GdkEventConfigure* event) {
    draw_width  = event->width;
    draw_height = event->height;
    pvscrollbar->set_range(0.0, endpos + 9 - (draw_height / IMAGE_SIZE));
    if (!drop_enabled) {
        drop_enabled = true;
        vector<Gtk::TargetEntry> vec;
        vec.emplace_back("SpecialStageObjects", Gtk::TargetFlags(0), 1);
        pspecialstageobjs->drag_dest_set(
            vec, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
        pspecialstageobjs->signal_drag_data_received().connect(sigc::mem_fun(
            this, &sseditor::on_specialstageobjs_drag_data_received));
        pspecialstageobjs->signal_drag_motion().connect(
            sigc::mem_fun(this, &sseditor::on_drag_motion));
    }
    render();
    update();
    return true;
}

void sseditor::cleanup_render(Cairo::RefPtr<Cairo::Context> cr) {
    if (drawimg) {
        drawimg->unreference();
    }
    drawimg = cr->pop_group();
}

void sseditor::draw_objects(
    Cairo::RefPtr<Cairo::Context> cr, int start, int end) {
    for (int i = start; i <= end; i++) {
        int         seg     = find_segment(i);
        sssegments* currseg = get_segment(seg);
        if (currseg == nullptr) {
            return;
        }

        auto const& row = currseg->get_row(i - segpos[seg]);
        for (auto const& elem : row) {
            Glib::RefPtr<Gdk::Pixbuf> image =
                (elem.second == sssegments::eBomb) ? bombimg : ringimg;

            int ty = (i - pvscrollbar->get_value()) * IMAGE_SIZE,
                tx = angle_to_x(elem.first) - image->get_width() / 2;
            Gdk::Cairo::set_source_pixbuf(cr, image, tx, ty);
            cr->paint();
        }
    }
}

bool sseditor::want_checkerboard(int row, int seg, sssegments* currseg) {
    return (seg == 2 &&
            (row - segpos[seg] == (currseg->get_length() - 1) / 2)) ||
           ((currseg->get_type() == sssegments::eCheckpoint ||
             currseg->get_type() == sssegments::eChaosEmerald) &&
            (row - segpos[seg]) == currseg->get_length() - 1);
}

void sseditor::draw_balls(Cairo::RefPtr<Cairo::Context> cr, int ty) {
    for (int iangle = 0; iangle < 3; iangle++) {
        double angle = (iangle * 64.0) / 3.0;
        cr->set_source_rgb(180.0 / 256.0, 108.0 / 256.0, 36.0 / 256.0);
        cr->arc(
            angle_to_x(angle + 0x80) + HALF_IMAGE_SIZE, ty, HALF_IMAGE_SIZE,
            0.0, 2.0 * G_PI);
        cr->begin_new_sub_path();
        cr->arc(
            angle_to_x(0x00 - angle) - HALF_IMAGE_SIZE, ty, HALF_IMAGE_SIZE,
            0.0, 2.0 * G_PI);
        cr->fill();
        cr->set_source_rgb(216.0 / 256.0, 144.0 / 256.0, 36.0 / 256.00);
        cr->arc(
            angle_to_x(angle + 0x80) + HALF_IMAGE_SIZE, ty - 1.5,
            HALF_IMAGE_SIZE - 2, 0.0, 2.0 * G_PI);
        cr->begin_new_sub_path();
        cr->arc(
            angle_to_x(0x00 - angle) - HALF_IMAGE_SIZE, ty - 1.5,
            HALF_IMAGE_SIZE - 2, 0.0, 2.0 * G_PI);
        cr->fill();
        cr->set_source_rgb(1.0, 180.0 / 256.0, 36.0 / 256.0);
        cr->arc(
            angle_to_x(angle + 0x80) + HALF_IMAGE_SIZE, ty - 3.0,
            HALF_IMAGE_SIZE - 4, 0.0, 2.0 * G_PI);
        cr->begin_new_sub_path();
        cr->arc(
            angle_to_x(0x00 - angle) - HALF_IMAGE_SIZE, ty - 3.0,
            HALF_IMAGE_SIZE - 4, 0.0, 2.0 * G_PI);
        cr->fill();
    }
}

void sseditor::render() {
    Glib::RefPtr<Gdk::Window> window = pspecialstageobjs->get_window();

    if (!window) {
        return;
    }

    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

    cr->push_group();
    cr->set_source_rgb(0.0, 180.0 / 256.0, 216.0 / 256.0);
    cr->paint();

    if (!specialstages) {
        cleanup_render(cr);
        return;
    }

    int start    = pvscrollbar->get_value(),
        end      = start + (draw_height + IMAGE_SIZE - 1) / IMAGE_SIZE;
    int last_seg = -1;

    cr->set_source_rgb(36.0 / 256.0, 108.0 / 256.0, 144.0 / 256.0);
    cr->rectangle(0.0, 0.0, angle_to_x(0x00), draw_height);
    cr->fill();

    cr->rectangle(
        angle_to_x(0x80), 0.0, draw_width - angle_to_x(0x80), draw_height);
    cr->fill();

    cr->set_line_width(8.0);
    cr->set_source_rgb(1.0, 180.0 / 256.0, 36.0 / 256.0);
    cr->move_to(angle_to_x(0x00 - 2), 0.0);
    cr->line_to(angle_to_x(0x00 - 2), draw_height);
    cr->move_to(angle_to_x(0x80 + 2), 0.0);
    cr->line_to(angle_to_x(0x80 + 2), draw_height);
    cr->stroke();

    cr->set_line_width(16.0);
    cr->move_to(angle_to_x(0x30 - 4), 0.0);
    cr->line_to(angle_to_x(0x30 - 4), draw_height);
    cr->move_to(angle_to_x(0x50 + 4), 0.0);
    cr->line_to(angle_to_x(0x50 + 4), draw_height);
    cr->stroke();

    for (int ii = start; ii <= end; ii++) {
        int         seg     = find_segment(ii);
        sssegments* currseg = get_segment(seg);
        if (currseg == nullptr) {
            break;
        }

        int ty = (ii - pvscrollbar->get_value()) * IMAGE_SIZE;
        if ((ii + 1) % 4 == 0) {
            cr->set_line_width(HALF_IMAGE_SIZE);
            cr->set_source_rgb(1.0, 180.0 / 256.0, 36.0 / 256.0);
            // Horizontal beams.
            cr->move_to(angle_to_x(0x00), ty);
            cr->line_to(angle_to_x(0x80), ty);
            cr->stroke();
            // Horizontal balls.
            draw_balls(cr, ty);
            // Yellow beams.
            cr->set_line_width(QUARTER_IMAGE_SIZE);
            cr->set_source_rgb(1.0, 1.0, 0.0);
            cr->move_to(angle_to_x(0x30 - 4), ty - IMAGE_SIZE);
            cr->line_to(angle_to_x(0x30 - 4), ty + IMAGE_SIZE);
            cr->move_to(angle_to_x(0x50 + 4), ty - IMAGE_SIZE);
            cr->line_to(angle_to_x(0x50 + 4), ty + IMAGE_SIZE);
            cr->move_to(angle_to_x(0x00 - 4), ty - 3 * IMAGE_SIZE);
            cr->line_to(angle_to_x(0x00 - 4), ty - IMAGE_SIZE);
            cr->move_to(angle_to_x(0x80 + 4), ty - 3 * IMAGE_SIZE);
            cr->line_to(angle_to_x(0x80 + 4), ty - IMAGE_SIZE);
            cr->stroke();
        }

        if (want_checkerboard(ii, seg, currseg)) {
            cr->save();
            cr->set_dash(vector<double>{8.0}, 0.0);
            cr->set_source_rgb(1.0, 1.0, 1.0);
            cr->set_line_width(HALF_IMAGE_SIZE);
            cr->move_to(angle_to_x(0x00), ty - HALF_IMAGE_SIZE);
            cr->line_to(angle_to_x(0x80), ty - HALF_IMAGE_SIZE);
            cr->move_to(angle_to_x(0x80), ty);
            cr->line_to(angle_to_x(0x00), ty);
            cr->move_to(angle_to_x(0x00), ty + HALF_IMAGE_SIZE);
            cr->line_to(angle_to_x(0x80), ty + HALF_IMAGE_SIZE);
            cr->stroke();
            cr->set_source_rgb(0.0, 0.0, 0.0);
            cr->set_line_width(HALF_IMAGE_SIZE);
            cr->move_to(angle_to_x(0x80), ty - HALF_IMAGE_SIZE);
            cr->line_to(angle_to_x(0x00), ty - HALF_IMAGE_SIZE);
            cr->move_to(angle_to_x(0x00), ty);
            cr->line_to(angle_to_x(0x80), ty);
            cr->move_to(angle_to_x(0x80), ty + HALF_IMAGE_SIZE);
            cr->line_to(angle_to_x(0x00), ty + HALF_IMAGE_SIZE);
            cr->stroke();
            cr->restore();
        }

        if (last_seg != seg) {
            last_seg = seg;
            cr->set_line_width(4.0);
            cr->set_source_rgb(1.0, 1.0, 1.0);
            int ty = (segpos[seg] - pvscrollbar->get_value()) * IMAGE_SIZE;
            cr->move_to(0, ty);
            cr->line_to(draw_width, ty);
            cr->stroke();
        }
    }

    draw_objects(cr, start, end);
    cleanup_render(cr);
}

void sseditor::draw_box(Cairo::RefPtr<Cairo::Context> cr) {
    if (mode == eDeleteMode) {
        cr->set_line_width(2.0);
        cr->set_source_rgb(1.0, 0.0, 0.0);
        draw_x(hotstack, cr);
    }
    int tx0 = angle_to_x(lastclick.get_angle()),
        ty0 = get_obj_pos<int>(lastclick);
    int tx1 = angle_to_x(boxcorner.get_angle()),
        ty1 = get_obj_pos<int>(boxcorner);
    int dx = tx1 - tx0, dy = ty1 - ty0;
    if (dx < 0) {
        swap(tx0, tx1);
    }
    if (dy < 0) {
        swap(ty0, ty1);
    }
    ty0 -= pvscrollbar->get_value();
    ty1 -= pvscrollbar->get_value();
    ty0 *= IMAGE_SIZE;
    ty1 *= IMAGE_SIZE;
    ty1 += IMAGE_SIZE;
    if (mode == eSelectMode) {
        cr->set_source_rgba(0.0, 1.0, 0.0, 0.25);
    } else {
        cr->set_source_rgba(1.0, 0.0, 0.0, 0.25);
    }
    cr->rectangle(tx0, ty0, tx1 - tx0, ty1 - ty0);
    cr->fill_preserve();
    if (mode == eSelectMode) {
        cr->set_source_rgb(0.0, 1.0, 0.0);
    } else {
        cr->set_source_rgb(1.0, 0.0, 0.0);
    }
    cr->set_line_width(1.0);
    cr->stroke();
}

void sseditor::select_hotspot() {
    hotspot.reset();
    int start = pvscrollbar->get_value(),
        end   = start + (draw_height + IMAGE_SIZE - 1) / IMAGE_SIZE;

    for (int i = start; i <= end && !hotspot.valid(); i++) {
        int         seg     = find_segment(i);
        sssegments* currseg = get_segment(seg);
        if (currseg == nullptr) {
            break;
        }

        auto const& row = currseg->get_row(i - segpos[seg]);
        for (auto const& elem : row) {
            int ty = (i - pvscrollbar->get_value()) * IMAGE_SIZE,
                tx = angle_to_x(static_cast<int8_t>(elem.first)) -
                     HALF_IMAGE_SIZE;

            if (mouse_x >= tx && mouse_y >= ty &&
                mouse_x < tx + static_cast<int>(IMAGE_SIZE) &&
                mouse_y < ty + static_cast<int>(IMAGE_SIZE)) {
                hotspot.set(seg, elem.first, i - segpos[seg], elem.second);
                break;
            }
        }
    }
}

void sseditor::show() {
    if (!drawimg) {
        render();
    }

    Glib::RefPtr<Gdk::Window> window = pspecialstageobjs->get_window();

    if (!window) {
        return;
    }

    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

    cr->set_source(drawimg);
    cr->paint();

    if (!specialstages) {
        return;
    }

    select_hotspot();

    if (mode == eSelectMode) {
        cr->set_source_rgb(0.0, 0.0, 0.0);
        cr->set_line_width(2.0);
        draw_outlines(selection, hotstack, cr);
        draw_outlines(hotstack, selection, cr);
        if (hotspot.valid()) {
            cr->set_source_rgb(1.0, 1.0, 0.0);
            cr->set_line_width(2.0);
            int tx = angle_to_x(hotspot.get_angle()) - HALF_IMAGE_SIZE;
            int ty = (get_obj_pos<int>(hotspot) - pvscrollbar->get_value()) *
                     IMAGE_SIZE;
            cr->rectangle(tx, ty, IMAGE_SIZE, IMAGE_SIZE);
            cr->stroke();
        }
        if (dragging) {
            draw_objects(insertstack, cr);
        }
        if (drawbox) {
            draw_box(cr);
        }
    } else if (mode == eDeleteMode) {
        if (drawbox) {
            draw_box(cr);
        } else if (hotspot.valid()) {
            int tx = angle_to_x(hotspot.get_angle()) - HALF_IMAGE_SIZE;
            int ty = (get_obj_pos<int>(hotspot) - pvscrollbar->get_value()) *
                     IMAGE_SIZE;
            cr->set_line_width(2.0);
            cr->set_source_rgb(1.0, 0.0, 0.0);
            cr->rectangle(tx, ty, IMAGE_SIZE, IMAGE_SIZE);
            cr->stroke();
            cr->move_to(tx, ty);
            cr->line_to(tx + IMAGE_SIZE, ty + IMAGE_SIZE);
            cr->move_to(tx + IMAGE_SIZE, ty);
            cr->line_to(tx, ty + IMAGE_SIZE);
            cr->stroke();
        }
    } else if (mode == eInsertRingMode || mode == eInsertBombMode) {
        cr->set_source_rgb(1.0, 1.0, 0.0);
        cr->set_line_width(2.0);
        draw_objects(insertstack, cr);
    }
}

bool sseditor::on_specialstageobjs_expose_event(GdkEventExpose* event) {
    ignore_unused_variable_warning(event);
    show();
    return true;
}

static inline int get_angle_delta(bool grid) { return grid ? 4 : 1; }

static inline double get_scroll_delta(guint state) {
    bool fast = (state & GDK_SHIFT_MASK) != 0;
    return fast ? 32.0 : 4.0;
}

bool sseditor::on_specialstageobjs_key_press_event(GdkEventKey* event) {
    if (!specialstages) {
        return true;
    }

    switch (event->keyval) {
    case GDK_KEY_Up:
    case GDK_KEY_KP_Up:
        return move_object(0, -1);

    case GDK_KEY_Down:
    case GDK_KEY_KP_Down:
        return move_object(0, 1);

    case GDK_KEY_Left:
    case GDK_KEY_KP_Left: {
        bool grid = want_snap_to_grid(event->state);
        return move_object(-get_angle_delta(grid), 0);
    }

    case GDK_KEY_Right:
    case GDK_KEY_KP_Right: {
        bool grid = want_snap_to_grid(event->state);
        return move_object(get_angle_delta(grid), 0);
    }

    case GDK_KEY_Page_Up:
    case GDK_KEY_KP_Page_Up: {
        double delta = get_scroll_delta(event->state);
        pvscrollbar->set_value(pvscrollbar->get_value() - delta);
        render();
        show();
        break;
    }

    case GDK_KEY_Page_Down:
    case GDK_KEY_KP_Page_Down: {
        double delta = get_scroll_delta(event->state);
        pvscrollbar->set_value(pvscrollbar->get_value() + delta);
        render();
        show();
        break;
    }
    }
    return true;
}

bool sseditor::on_specialstageobjs_button_press_event(GdkEventButton* event) {
    state = event->state;

    if (!specialstages) {
        return true;
    }

    if (event->button != GDK_BUTTON_LEFT) {
        return true;
    }
    int pos = event->y / IMAGE_SIZE + pvscrollbar->get_value(),
        seg = find_segment(pos);
    lastclick.set(
        seg, x_to_angle(event->x, true), pos - segpos[seg], sssegments::eRing);

    selclear.reset();
    if (mode == eSelectMode) {
        if ((event->state & GDK_CONTROL_MASK) == 0 &&
            (!hotspot.valid() || selection.find(hotspot) == selection.end())) {
            selection.clear();
        }
        if (hotspot.valid()) {
            if (selection.find(hotspot) == selection.end()) {
                selection.insert(hotspot);
            } else {
                selclear = hotspot;
            }
        }
        update();
        return true;
    }

    selection.clear();
    update();
    return true;
}

void sseditor::delete_set(set<object>& toDel) {
    do_action<delete_selection_action>(currstage, toDel);
    toDel.clear();
}

void sseditor::delete_set(set<object>&& toDel) {
    do_action<delete_selection_action>(currstage, std::move(toDel));
}

void sseditor::delete_existing_object(int seg, unsigned pos, unsigned angle) {
    sssegments* currseg = get_segment(seg);
    ObjectTypes type;
    if (currseg != nullptr && currseg->exists(pos, angle, type)) {
        delete_object(seg, angle, pos, type);
    }
}

void sseditor::cycle_object_type(int seg, unsigned pos, unsigned angle) {
    ObjectTypes type;
    sssegments* currseg = get_segment(seg);
    if (currseg == nullptr) {
        return;
    }

    selection.clear();
    if (currseg->exists(pos, angle, type)) {
        if (type == sssegments::eBomb) {
            delete_object(seg, angle, pos, type);
        } else {
            selection.emplace(seg, angle, pos, type);
            do_action<alter_selection_action>(
                currstage, sssegments::eBomb, selection);
        }
    } else {
        selection.emplace(seg, angle, pos, sssegments::eRing);
        do_action<insert_objects_action>(currstage, selection);
    }
}

sssegments* sseditor::get_segment(int seg) {
    sslevels* currlvl = specialstages->get_stage(currstage);
    if (seg >= static_cast<int>(segpos.size())) {
        return nullptr;
    }
    return currlvl->get_segment(seg);
}

void sseditor::finalize_selection() {
    for (auto const& elem : hotstack) {
        auto it2 = selection.find(elem);
        if (it2 == selection.end()) {
            selection.insert(elem);
        } else {
            selection.erase(it2);
        }
    }
    hotstack.clear();
}

void sseditor::insert_set() {
    if (!insertstack.empty()) {
        set<object> delstack;
        for (auto const& elem : insertstack) {
            int         seg     = elem.get_segment();
            sssegments* currseg = get_segment(seg);
            ObjectTypes type;
            if (currseg != nullptr &&
                currseg->exists(elem.get_pos(), elem.get_angle(), type)) {
                delstack.emplace(seg, elem.get_angle(), elem.get_pos(), type);
            }
        }
        do_action<insert_objects_ex_action>(currstage, delstack, insertstack);
        insertstack.clear();
    }
}

tuple<int, int, int> sseditor::get_mouseup_loc(GdkEventButton* event) {
    int angle, pos, seg;
    if (!hotspot.valid()) {
        angle = x_to_angle(event->x, want_snap_to_grid(state), 4U);
        pos   = event->y / IMAGE_SIZE + pvscrollbar->get_value();
        seg   = find_segment(pos);
        pos -= segpos[seg];
    } else {
        angle = hotspot.get_angle();
        pos   = hotspot.get_pos();
        seg   = hotspot.get_segment();
    }
    return tuple<int, int, int>{angle, pos, seg};
}

bool sseditor::on_specialstageobjs_button_release_event(GdkEventButton* event) {
    state = event->state;
    if (!specialstages) {
        return true;
    }

    finish_drag_box(event);

    int angle, pos, seg;
    tie(angle, pos, seg) = get_mouseup_loc(event);

    sssegments* currseg = get_segment(seg);
    if (currseg == nullptr) {
        return true;
    }

    switch (mode) {
    case eSelectMode:
        if (event->button == GDK_BUTTON_LEFT) {
            finalize_selection();
        } else if (event->button == GDK_BUTTON_RIGHT) {
            cycle_object_type(seg, pos, angle);
        }
        break;

    case eDeleteMode: {
        if (!hotstack.empty()) {
            delete_set(hotstack);
        } else {
            delete_existing_object(seg, pos, angle);
        }
        break;
    }
    case eInsertBombMode:
    case eInsertRingMode: {
        if (event->button == GDK_BUTTON_LEFT) {
            insert_set();
        } else if (event->button == GDK_BUTTON_RIGHT) {
            delete_existing_object(seg, pos, angle);
        }
        break;
    }
    default:
        __builtin_unreachable();
    }
    render();
    update();
    return true;
}

void sseditor::increment_mode(bool ctrl) {
    if (ctrl) {
        if (mode == eInsertRingMode) {
            increment_insertmode(ringmode);
            pringmodebuttons[ringmode]->set_active(true);
        } else if (mode == eInsertBombMode) {
            increment_insertmode(bombmode);
            pbombmodebuttons[bombmode]->set_active(true);
        }
    } else {
        increment_editmode();
        pmodebuttons[mode]->set_active(true);
    }
}

void sseditor::decrement_mode(bool ctrl) {
    if (ctrl) {
        if (mode == eInsertRingMode) {
            decrement_insertmode(ringmode);
            pringmodebuttons[ringmode]->set_active(true);
        } else if (mode == eInsertBombMode) {
            decrement_insertmode(bombmode);
            pbombmodebuttons[bombmode]->set_active(true);
        }
    } else {
        decrement_editmode();
        pmodebuttons[mode]->set_active(true);
    }
}

bool sseditor::on_specialstageobjs_scroll_event(GdkEventScroll* event) {
    state = event->state;
    if (!specialstages) {
        return false;
    }

    bool   ctrl  = (event->state & GDK_CONTROL_MASK) != 0;
    double delta = ctrl ? 32 : 4;

    switch (event->direction) {
    case GDK_SCROLL_UP:
        pvscrollbar->set_value(pvscrollbar->get_value() - delta);
        break;

    case GDK_SCROLL_DOWN:
        pvscrollbar->set_value(pvscrollbar->get_value() + delta);
        break;

    case GDK_SCROLL_LEFT:
        decrement_mode(ctrl);
        break;

    case GDK_SCROLL_RIGHT:
        increment_mode(ctrl);
        break;
    }

    return true;
}

void sseditor::object_triangle(
    int x, int y, int dx, int dy, int h, ObjectTypes type, bool fill,
    set<object>& col) {
    ignore_unused_variable_warning(col);
    int numsegments = segpos.size();
    int angle       = x;
    int seg = find_segment(y), pos = y - segpos[seg];
    if (seg < numsegments) {
        insertstack.emplace(seg, angle_normal(angle), pos, type);
    }
    int delta;
    int i, last;
    if (h > 0) {
        delta = dx;
        i     = y + dy;
        last  = y + h;
    } else {
        delta = -h * dx;
        i     = y + h;
        last  = y;
        dx    = -dx;
        dy    = -dy;
    }

    while (i < last) {
        seg = find_segment(i);
        pos = i - segpos[seg];
        i += dy;
        if (seg >= numsegments) {
            continue;
        }

        int tx;
        if (fill) {
            int const cnt    = (2 * delta) / HALF_IMAGE_SIZE,
                      middle = (cnt & 1) == 0;
            int const dj     = 8 * delta;
            int const min    = middle ? cnt * IMAGE_SIZE : 0;
            if (middle) {
                insertstack.emplace(seg, angle_normal(angle), pos, type);
            }

            for (int j = 4 * delta * cnt; j >= min; j -= dj) {
                double jang = j / (4.0 * cnt);
                tx          = angle_normal(static_cast<int8_t>(angle - jang));
                insertstack.emplace(seg, tx, pos, type);
                tx = angle_normal(static_cast<int8_t>(angle + jang));
                insertstack.emplace(seg, tx, pos, type);
            }
        } else {
            tx = angle_normal(static_cast<int8_t>(angle - delta));
            insertstack.emplace(seg, tx, pos, type);
            tx = angle_normal(static_cast<int8_t>(angle + delta));
            insertstack.emplace(seg, tx, pos, type);
        }
        delta += dx;
    }
}

void sseditor::scroll_into_view(GdkEventMotion* event) {
    if ((event->state & GDK_BUTTON1_MASK) == 0) {
        return;
    }
    if (event->y < 5) {
        pvscrollbar->set_value(pvscrollbar->get_value() - 4.0);
    } else if (draw_height - event->y < 5) {
        pvscrollbar->set_value(pvscrollbar->get_value() + 4.0);
    }
}

void sseditor::motion_update_selection(
    int dangle, int dpos, int pos0, int pos1, int angle0, int angle1) {
    hotstack.clear();
    if (dangle == 0 && dpos == 0) {
        return;
    }
    int seg1 = find_segment(pos1);
    boxcorner.set(
        seg1, static_cast<int8_t>(angle1 + 0xc0), pos1 - segpos[seg1],
        sssegments::eRing);
    drawbox = true;
    for (int i = min(pos0, pos1); i <= max(pos0, pos1); i++) {
        int         seg = find_segment(i), pos = i - segpos[seg];
        sssegments* currseg = get_segment(seg);
        if (currseg == nullptr) {
            continue;
        }
        for (int j = min(angle0, angle1); j <= max(angle0, angle1); j++) {
            int         angle = static_cast<int8_t>(j + 0xc0);
            ObjectTypes type;
            if (currseg->exists(pos, angle, type)) {
                hotstack.emplace(seg, angle, pos, type);
            }
        }
    }
}

void sseditor::motion_update_line(
    int dpos, int pos0, int pos1, int angle0, ObjectTypes type,
    int angledelta) {
    int const numsegments = segpos.size();
    int const delta       = sigplus(dpos);
    do {
        int seg = find_segment(pos0), pos = pos0 - segpos[seg];
        pos0 += delta;
        if (seg < numsegments) {
            insertstack.emplace(
                seg, angle_normal(static_cast<int8_t>(angle0)), pos, type);
            angle0 = static_cast<int8_t>(angle0 + angledelta);
        }
    } while (pos0 != pos1 + delta);
}

void sseditor::motion_update_loop(
    int dpos, int pos0, int angle0, ObjectTypes type, int angledelta,
    bool grid) {
    int    dy = signum(dpos);
    int    nobj;
    double delta;
    if (dy != 0) {
        if (angledelta == 0) {
            angledelta = get_angle_delta(grid);
        }
        int sign = sigplus(angledelta);
        int dx   = abs(angledelta);
        nobj     = 0x100 / dx;
        delta    = sign * (double(0x100) / nobj);
    } else {
        angledelta = clamp(
            abs(angledelta), HALF_IMAGE_SIZE, std::numeric_limits<int>::max());
        nobj  = (0x100 - angledelta) / angledelta;
        delta = double(0x100 - angledelta) / nobj;
    }

    int const numsegments = segpos.size();
    for (int ii = 0; ii <= nobj; ii++, pos0 += dy) {
        int seg = find_segment(pos0), pos = pos0 - segpos[seg];
        if (seg < numsegments) {
            insertstack.emplace(
                seg, angle_normal(static_cast<int8_t>(angle0)), pos, type);
            angle0 += delta;
        }
    }
}

void sseditor::motion_update_zigzag(
    int dpos, int pos0, int pos1, int angle0, ObjectTypes type,
    int angledelta) {
    int const delta = sigplus(dpos);
    angledelta      = clamp(angledelta, -HALF_IMAGE_SIZE, HALF_IMAGE_SIZE);
    int const numsegments = segpos.size();
    do {
        int seg = find_segment(pos0), pos = pos0 - segpos[seg];
        pos0 += delta;
        if (seg < numsegments) {
            insertstack.emplace(seg, angle_normal(angle0), pos, type);
            angle0     = static_cast<int8_t>(angle0 + angledelta);
            angledelta = -angledelta;
        }
    } while (pos0 != pos1 + delta);
}

void sseditor::motion_update_diamond(
    int dpos, int pos0, int pos1, int angle0, ObjectTypes type,
    int angledelta) {
    int const delta       = sigplus(dpos);
    int const numsegments = segpos.size();
    angledelta = clamp(abs(angledelta), QUARTER_IMAGE_SIZE, HALF_IMAGE_SIZE);
    int angle  = angle0;
    int ii     = pos0 + delta;
    int seg = find_segment(pos0), pos = pos0 - segpos[seg];
    if (seg < numsegments) {
        insertstack.emplace(seg, angle_normal(angle), pos, type);
    }
    seg = find_segment(pos1);
    pos = pos1 - segpos[seg];
    if (seg < numsegments) {
        insertstack.emplace(seg, angle_normal(angle), pos, type);
    }
    while (ii != pos1) {
        seg = find_segment(ii);
        pos = ii - segpos[seg];
        ii += delta;
        if (seg < numsegments) {
            insertstack.emplace(
                seg, angle_normal(static_cast<int8_t>(angle - angledelta)), pos,
                type);
            insertstack.emplace(
                seg, angle_normal(static_cast<int8_t>(angle + angledelta)), pos,
                type);
        }
    }
}

void sseditor::motion_update_insertion(
    int dangle, int dpos, int pos0, int pos1, int angle0, int angle1, bool grid,
    bool lbutton_pressed) {
    insertstack.clear();
    ObjectTypes type;
    InsertModes submode;
    tie(type, submode) = get_obj_type();

    if (submode == eSingle || (submode != eLoop && !dpos) || !lbutton_pressed) {
        int seg1 = find_segment(pos1);
        insertstack.emplace(
            seg1, angle_normal(angle1), pos1 - segpos[seg1], type);
        return;
    }

    int angledelta = dangle;
    if (submode == eLine) {
        angledelta /= abs(dpos);
    }
    if (grid) {
        angledelta += 3 * sigplus(angledelta);
        angledelta /= 4;
        angledelta *= 4;
    }

    switch (submode) {
    case eLine:
        motion_update_line(dpos, pos0, pos1, angle0, type, angledelta);
        break;
    case eLoop:
        motion_update_loop(dpos, pos0, angle0, type, angledelta, grid);
        break;
    case eZigzag:
        motion_update_zigzag(dpos, pos0, pos1, angle0, type, angledelta);
        break;
    case eDiamond:
        motion_update_diamond(dpos, pos0, pos1, angle0, type, angledelta);
        break;
    case eLozenge:
    case eStar: {
        bool fill = submode == eLozenge;
        angledelta =
            clamp(abs(angledelta), QUARTER_IMAGE_SIZE, HALF_IMAGE_SIZE);
        int off0 = dpos >= 0;
        int off1 = dpos < 0;
        object_triangle(
            angle0, pos0, angledelta, sigplus(dpos), (dpos + off0) / 2, type,
            fill, insertstack);
        object_triangle(
            angle0, pos1, angledelta, -sigplus(dpos), (-dpos + off1) / 2, type,
            fill, insertstack);
        break;
    }
    case eTriangle:
        angledelta =
            clamp(abs(angledelta), QUARTER_IMAGE_SIZE, HALF_IMAGE_SIZE);
        object_triangle(
            angle0, pos1, angledelta, -sigplus(dpos), -dpos, type, true,
            insertstack);
        break;
    default:
        __builtin_unreachable();
    }
}

void sseditor::motion_update_select_insert(GdkEventMotion* event) {
    bool lbutton_pressed = (event->state & GDK_BUTTON1_MASK) != 0;
    bool want_update     = drawbox || mode == eInsertRingMode ||
                       mode == eInsertBombMode ||
                       (lbutton_pressed && (!dragging));
    if (want_update) {
        scroll_into_view(event);
        int angle1, pos1;
        tie(angle1, pos1) = get_motion_loc(event);

        int angle0 = angle_simple(lastclick.get_angle());
        int pos0   = get_obj_pos<int>(lastclick);
        int dangle = angle1 - angle0;
        int dpos   = pos1 - pos0;

        switch (mode) {
        case eSelectMode:
        case eDeleteMode:
            motion_update_selection(dangle, dpos, pos0, pos1, angle0, angle1);
            break;

        case eInsertRingMode:
        case eInsertBombMode:
            motion_update_insertion(
                dangle, dpos, pos0, pos1, angle0, angle1,
                want_snap_to_grid(event->state), lbutton_pressed);
            break;

        default:
            __builtin_unreachable();
        }
    }
}

bool sseditor::on_specialstageobjs_motion_notify_event(GdkEventMotion* event) {
    if (!specialstages) {
        return true;
    }

    bool lbutton_pressed = (event->state & GDK_BUTTON1_MASK) != 0;
    bool dragging =
        hotspot.valid() && selection.find(hotspot) != selection.end();

    state   = event->state;
    mouse_x = event->x;
    mouse_y = event->y;
    drawbox = drawbox && lbutton_pressed;

    update();

    bool no_dragdrop =
        (mode != eSelectMode || drawbox || !lbutton_pressed || !dragging);
    if (no_dragdrop) {
        return true;
    }

    dragging = true;
    vector<Gtk::TargetEntry> vec;
    vec.emplace_back("SpecialStageObjects", Gtk::TargetFlags(0), 1);
    Glib::RefPtr<Gtk::TargetList> lst = Gtk::TargetList::create(vec);
    pspecialstageobjs->drag_begin(
        lst, Gdk::ACTION_MOVE, 1, reinterpret_cast<GdkEvent*>(event));
    return true;
}

void sseditor::on_specialstageobjs_drag_begin(
    Glib::RefPtr<Gdk::DragContext> const& targets) {
    ignore_unused_variable_warning(targets);
    if (selection.empty()) {
        return;
    }

    sourcestack = selection;

    // Glib::RefPtr<Gdk::Pixbuf> image = seltype == sssegments::eBomb ? bombimg
    // : ringimg; targets->set_icon(image, 0, 0);
}

bool sseditor::on_drag_motion(
    Glib::RefPtr<Gdk::DragContext> const& context, int x, int y, guint time) {
    ignore_unused_variable_warning(context, time);
    constexpr const double delta       = 4.0;
    constexpr const int    threshold_y = 5;
    if (y < threshold_y) {
        pvscrollbar->set_value(pvscrollbar->get_value() - delta);
    } else if (draw_height - y < threshold_y) {
        pvscrollbar->set_value(pvscrollbar->get_value() + delta);
    }
    int dangle;
    int dpos;
    if (hotspot.valid()) {
        dangle = hotspot.get_angle();
        dpos   = get_obj_pos<int>(hotspot);
    } else {
        dangle = x_to_angle(x, snaptogrid);
        dpos   = static_cast<int>(y / IMAGE_SIZE) +
               static_cast<int>(pvscrollbar->get_value());
    }
    dangle = static_cast<int8_t>(dangle - lastclick.get_angle());
    dpos -= get_obj_pos<int>(lastclick);

    insertstack.clear();

    if (dangle == 0 && dpos == 0) {
        insertstack = sourcestack;
    } else {
        int maxpos = static_cast<int>(endpos) - 1;
        for (auto obj : sourcestack) {
            obj.set_angle(static_cast<int8_t>(obj.get_angle() + dangle));
            int pos = clamp(get_obj_pos<int>(obj) + dpos, 0, maxpos);
            int seg = find_segment(pos);
            obj.set_pos(pos - segpos[seg]);
            obj.set_segment(seg);
            insertstack.insert(obj);
        }
    }

    mouse_x = x;
    mouse_y = y;
    render();
    update();
    return true;
}

void sseditor::on_specialstageobjs_drag_data_get(
    Glib::RefPtr<Gdk::DragContext> const& targets,
    Gtk::SelectionData& selection_data, guint info, guint time) {
    ignore_unused_variable_warning(targets, info, time);
    if (insertstack.empty()) {
        return;
    }

    stringstream data(ios::in | ios::out | ios::binary);
    for (auto const& elem : insertstack) {
        Write1(data, elem.get_segment());
        Write1(data, elem.get_angle());
        Write1(data, elem.get_pos());
        Write1(data, uint8_t(elem.get_type()));
    }
    selection_data.set("SpecialStageObjects", data.str());
}

void sseditor::on_specialstageobjs_drag_end(
    Glib::RefPtr<Gdk::DragContext> const& context) {
    ignore_unused_variable_warning(context);
    dragging = false;
}

void sseditor::on_specialstageobjs_drag_data_received(
    Glib::RefPtr<Gdk::DragContext> const& context, int x, int y,
    Gtk::SelectionData const& selection_data, guint info, guint time) {
    ignore_unused_variable_warning(x, y, info);
    if (selection_data.get_data_type() == "SpecialStageObjects" &&
        selection_data.get_length() > 0) {
        // set<object> temp = sourcestack;
        context->drag_finish(true, false, time);

        size_t        len  = selection_data.get_length();
        guchar const* data = selection_data.get_data();
        guchar const* ptr  = data;
        selection.clear();

        while (len > 0) {
            int  newseg   = Read1(ptr);
            int  newangle = Read1(ptr);
            int  newpos   = Read1(ptr);
            auto newtype  = ObjectTypes(Read1(ptr));
            len -= 4;
            selection.emplace(newseg, newangle, newpos, newtype);
        }

        if (!equal(
                sourcestack.begin(), sourcestack.end(), selection.begin(),
                ObjectMatchFunctor())) {
            do_action<move_objects_action>(currstage, sourcestack, selection);
        }
        sourcestack.clear();
    } else {
        context->drag_finish(false, false, time);
    }

    render();
    update();
}
