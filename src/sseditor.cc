/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Copyright (C) Flamewing 2011-2019 <flamewing.sonic@gmail.com>
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

#include <iostream>
#include <set>
#include <vector>

#include <s2ssedit/sseditor.hh>

#include <s2ssedit/ignore_unused_variable_warning.hh>

using std::cout;
using std::endl;
using std::set;
using std::swap;
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
