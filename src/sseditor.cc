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

#include <gdkmm/rgba.h>
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

size_t sseditor::find_segment(int pos) const {
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

        int pos = (newy + segpos[oldseg]) * SIMAGE_SIZE;
        int loc = get_scroll() * SIMAGE_SIZE;
        if (pos < loc || pos >= loc + draw_height) {
            int newpos = pos / SIMAGE_SIZE;
            pvscrollbar->set_value(newpos);
        }

        temp.emplace(elem.get_segment(), newx, newy, elem.get_type());
    }

    do_action<move_objects_action>(currstage, selection, temp);
    selection.swap(temp);
    update();
    return true;
}

void sseditor::cleanup_render(Cairo::RefPtr<Cairo::Context> const& cr) {
    if (drawimg) {
        drawimg->unreference();
    }
    drawimg = cr->pop_group();
}

void sseditor::draw_objects(
    Cairo::RefPtr<Cairo::Context> const& cr, int start, int end) {
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

            int ty = (i - get_scroll()) * SIMAGE_SIZE;
            int tx = angle_to_x(elem.first) - image->get_width() / 2;
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

struct RGB {
    double red;
    double green;
    double blue;
    constexpr RGB(uint8_t r, uint8_t g, uint8_t b)
        : red(r / 255.0), green(g / 255.0), blue(b / 255.0) {}
};

void sseditor::draw_balls(
    Cairo::RefPtr<Cairo::Context> const& cr, int ty) const {
    // TODO: Read palettes and use colors.
    static constexpr const std::array<RGB, 7> hilites{
        RGB{255, 172, 52}, RGB{255, 144, 0}, RGB{255, 172, 52},
        RGB{255, 172, 52}, RGB{0, 255, 87},  RGB{255, 172, 52},
        RGB{172, 172, 206}};
    static constexpr const std::array<RGB, 7> midtones{
        RGB{206, 144, 52}, RGB{206, 116, 0}, RGB{206, 144, 52},
        RGB{206, 144, 52}, RGB{0, 172, 52},  RGB{206, 144, 52},
        RGB{144, 144, 172}};
    static constexpr const std::array<RGB, 7> shadows{
        RGB{172, 116, 52}, RGB{172, 87, 0}, RGB{172, 116, 52},
        RGB{172, 116, 52}, RGB{0, 144, 0},  RGB{172, 116, 52},
        RGB{116, 116, 144}};
    const auto hilite  = hilites[currstage % hilites.size()];
    const auto midtone = midtones[currstage % midtones.size()];
    const auto shadow  = shadows[currstage % shadows.size()];
    for (int iangle = 0; iangle < 3; iangle++) {
        double angle  = (iangle * 64.0) / 3.0;
        int    mangle = static_cast<int>(angle);
        cr->set_source_rgb(shadow.red, shadow.green, shadow.blue);
        cr->arc(
            angle_to_x(mangle + 0x80) + HALF_IMAGE_SIZE, ty, HALF_IMAGE_SIZE,
            0.0, 2.0 * G_PI);
        cr->begin_new_sub_path();
        cr->arc(
            angle_to_x(0x00 - mangle) - HALF_IMAGE_SIZE, ty, HALF_IMAGE_SIZE,
            0.0, 2.0 * G_PI);
        cr->fill();
        cr->set_source_rgb(midtone.red, midtone.green, midtone.blue);
        cr->arc(
            angle_to_x(mangle + 0x80) + HALF_IMAGE_SIZE, ty - 1.5,
            HALF_IMAGE_SIZE - 2, 0.0, 2.0 * G_PI);
        cr->begin_new_sub_path();
        cr->arc(
            angle_to_x(0x00 - mangle) - HALF_IMAGE_SIZE, ty - 1.5,
            HALF_IMAGE_SIZE - 2, 0.0, 2.0 * G_PI);
        cr->fill();
        cr->set_source_rgb(hilite.red, hilite.green, hilite.blue);
        cr->arc(
            angle_to_x(mangle + 0x80) + HALF_IMAGE_SIZE, ty - 3.0,
            HALF_IMAGE_SIZE - 4, 0.0, 2.0 * G_PI);
        cr->begin_new_sub_path();
        cr->arc(
            angle_to_x(0x00 - mangle) - HALF_IMAGE_SIZE, ty - 3.0,
            HALF_IMAGE_SIZE - 4, 0.0, 2.0 * G_PI);
        cr->fill();
    }
}

bool sseditor::on_specialstageobjs_expose_event(
    const Cairo::RefPtr<Cairo::Context>& cr) {
    // TODO: Read palettes and use colors.
    static constexpr const std::array<RGB, 7> fgcolors{
        RGB{0, 172, 206},   RGB{206, 0, 144}, RGB{206, 87, 0},
        RGB{206, 206, 172}, RGB{255, 144, 0}, RGB{116, 172, 0},
        RGB{144, 144, 144}};
    static constexpr const std::array<RGB, 7> lanecolors{
        RGB{255, 172, 52}, RGB{255, 144, 0}, RGB{255, 172, 52},
        RGB{255, 172, 52}, RGB{0, 255, 87},  RGB{255, 172, 52},
        RGB{172, 172, 206}};
    // Base tube color
    const auto fgcolor = fgcolors[currstage % fgcolors.size()];
    cr->set_source_rgb(fgcolor.red, fgcolor.green, fgcolor.blue);
    cr->paint();

    if (!specialstages) {
        return true;
    }

    int start    = get_scroll();
    int end      = start + (draw_height + SIMAGE_SIZE - 1) / SIMAGE_SIZE;
    int last_seg = -1;

    // Draw area outside of the tube (left)
    // const auto bgcolor = bgcolors[currstage % bgcolors.size()];
    constexpr const auto bgcolor = RGB(0, 87, 116);
    cr->set_source_rgb(bgcolor.red, bgcolor.green, bgcolor.blue);
    cr->rectangle(0.0, 0.0, angle_to_x(0x00), draw_height);
    cr->fill();

    // Draw area outside of the tube (right)
    cr->rectangle(
        angle_to_x(0x80), 0.0, draw_width - angle_to_x(0x80), draw_height);
    cr->fill();

    cr->set_line_width(8.0);
    const auto lanecolor = lanecolors[currstage % lanecolors.size()];
    cr->set_source_rgb(lanecolor.red, lanecolor.green, lanecolor.blue);
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

        int ty = (ii - get_scroll()) * SIMAGE_SIZE;
        if ((ii + 1) % 4 == 0) {
            cr->set_line_width(HALF_IMAGE_SIZE);
            cr->set_source_rgb(lanecolor.red, lanecolor.green, lanecolor.blue);
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
            int my = (segpos[seg] - get_scroll()) * SIMAGE_SIZE;
            cr->move_to(0, my);
            cr->line_to(draw_width, my);
            cr->stroke();
        }
    }

    if (mode == eSelectMode) {
        cr->set_source_rgb(0.0, 0.0, 0.0);
        cr->set_line_width(2.0);
        draw_outlines(selection, hotstack, cr);
        draw_outlines(hotstack, selection, cr);
        if (hotspot.valid()) {
            cr->set_source_rgb(1.0, 1.0, 0.0);
            cr->set_line_width(2.0);
            int tx = angle_to_x(hotspot.get_angle()) - HALF_IMAGE_SIZE;
            int ty = (get_obj_pos<int>(hotspot) - get_scroll()) * SIMAGE_SIZE;
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
            int  tx = angle_to_x(hotspot.get_angle()) - HALF_IMAGE_SIZE;
            auto ty = (get_obj_pos<int>(hotspot) - get_scroll()) * SIMAGE_SIZE;
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

    draw_objects(cr, start, end);
    return true;
}

void sseditor::draw_box(Cairo::RefPtr<Cairo::Context> const& cr) {
    if (mode == eDeleteMode) {
        cr->set_line_width(2.0);
        cr->set_source_rgb(1.0, 0.0, 0.0);
        draw_x(hotstack, cr);
    }
    int tx0 = angle_to_x(lastclick.get_angle());
    int ty0 = get_obj_pos<int>(lastclick);
    int tx1 = angle_to_x(boxcorner.get_angle());
    int ty1 = get_obj_pos<int>(boxcorner);
    int dx  = tx1 - tx0;
    int dy  = ty1 - ty0;
    if (dx < 0) {
        swap(tx0, tx1);
    }
    if (dy < 0) {
        swap(ty0, ty1);
    }
    ty0 -= get_scroll();
    ty1 -= get_scroll();
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
    int start = get_scroll();
    int end   = start + (draw_height + SIMAGE_SIZE - 1) / SIMAGE_SIZE;

    for (int i = start; i <= end && !hotspot.valid(); i++) {
        int         seg     = find_segment(i);
        sssegments* currseg = get_segment(seg);
        if (currseg == nullptr) {
            break;
        }

        auto const& row = currseg->get_row(i - segpos[seg]);
        for (auto const& elem : row) {
            int ty = (i - get_scroll()) * SIMAGE_SIZE;
            int tx =
                angle_to_x(static_cast<int8_t>(elem.first)) - HALF_IMAGE_SIZE;

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
    Glib::RefPtr<Gdk::Window> window = pspecialstageobjs->get_window();
    if (!window) {
        return;
    }

    if (!specialstages) {
        return;
    }

    select_hotspot();
    render();
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
        show();
        break;
    }

    case GDK_KEY_Page_Down:
    case GDK_KEY_KP_Page_Down: {
        double delta = get_scroll_delta(event->state);
        pvscrollbar->set_value(pvscrollbar->get_value() + delta);
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

    if (event->button != GDK_BUTTON_PRIMARY) {
        return true;
    }
    int pos = static_cast<int>(event->y) / SIMAGE_SIZE + get_scroll();
    int seg = find_segment(pos);
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
