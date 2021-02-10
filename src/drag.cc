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

#include <set>
#include <sstream>
#include <vector>

#include <s2ssedit/sseditor.hh>

#include <mdcomp/bigendian_io.hh>

#include <s2ssedit/ignore_unused_variable_warning.hh>

using std::ios;
using std::max;
using std::min;
using std::set;
using std::stringstream;
using std::tie;
using std::tuple;
using std::vector;

bool sseditor::on_specialstageobjs_configure_event(GdkEventConfigure* event) {
    draw_width      = event->width;
    draw_height     = event->height;
    int half_height = draw_height / SIMAGE_SIZE;
    pvscrollbar->set_range(0.0, endpos + 9 - half_height);
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

tuple<int, int, int> sseditor::get_mouseup_loc(GdkEventButton* event) {
    int angle;
    int pos;
    int seg;
    if (!hotspot.valid()) {
        angle = x_to_angle(event->x, want_snap_to_grid(state), 4U);
        pos =
            static_cast<int>(event->y / IMAGE_SIZE + pvscrollbar->get_value());
        seg = find_segment(pos);
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

    int angle;
    int pos;
    int seg;
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
    case eNumModes:
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
    int seg         = find_segment(y);
    int pos         = y - segpos[seg];
    if (seg < numsegments) {
        insertstack.emplace(seg, angle_normal(angle), pos, type);
    }
    int delta;
    int i;
    int last;
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
            int const cnt    = (2 * delta) / HALF_IMAGE_SIZE;
            int const middle = static_cast<int>((cnt % 2) == 0);
            int const dj     = 8 * delta;
            int const min    = middle != 0 ? cnt * SIMAGE_SIZE : 0;
            if (middle != 0) {
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
        int seg = find_segment(i);
        int pos = i - segpos[seg];

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
        int seg = find_segment(pos0);
        int pos = pos0 - segpos[seg];
        pos0 += delta;
        if (seg < numsegments) {
            insertstack.emplace(
                seg, angle_normal(static_cast<int8_t>(angle0)), pos, type);
            angle0 = static_cast<int8_t>(angle0 + angledelta);
        }
    } while (pos0 != pos1 + delta);
}

static inline int get_angle_delta(bool grid) { return grid ? 4 : 1; }

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
        int seg = find_segment(pos0);
        int pos = pos0 - segpos[seg];
        if (seg < numsegments) {
            insertstack.emplace(
                seg, angle_normal(static_cast<int8_t>(angle0)), pos, type);
            angle0 += static_cast<int>(delta);
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
        int seg = find_segment(pos0);
        int pos = pos0 - segpos[seg];
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
    int seg    = find_segment(pos0);
    int pos    = pos0 - segpos[seg];
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

void sseditor::motion_update_star_lozenge(
    int dpos, int pos0, int pos1, int angle0, ObjectTypes type, int angledelta,
    bool fill) {
    int off0 = static_cast<int>(dpos >= 0);
    int off1 = static_cast<int>(dpos < 0);
    object_triangle(
        angle0, pos0, angledelta, sigplus(dpos), (dpos + off0) / 2, type, fill,
        insertstack);
    object_triangle(
        angle0, pos1, angledelta, -sigplus(dpos), (-dpos + off1) / 2, type,
        fill, insertstack);
}

int sseditor::motion_compute_angledelta(
    int dpos, InsertModes submode, bool grid, int angledelta) {
    if (submode == eLine) {
        angledelta /= abs(dpos);
    }
    if (grid) {
        angledelta += 3 * sigplus(angledelta);
        angledelta /= 4;
        angledelta *= 4;
    }
    return angledelta;
}

void sseditor::motion_update_insertion(
    int dangle, int dpos, int pos0, int pos1, int angle0, int angle1, bool grid,
    bool lbutton_pressed) {
    insertstack.clear();
    ObjectTypes type;
    InsertModes submode;
    tie(type, submode) = get_obj_type();

    if (submode == eSingle || (submode != eLoop && dpos == 0) ||
        !lbutton_pressed) {
        int seg1 = find_segment(pos1);
        insertstack.emplace(
            seg1, angle_normal(angle1), pos1 - segpos[seg1], type);
        return;
    }

    const int angledelta =
        motion_compute_angledelta(dpos, submode, grid, dangle);

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
        motion_update_star_lozenge(
            dpos, pos0, pos1, angle0, type,
            clamp(abs(angledelta), QUARTER_IMAGE_SIZE, HALF_IMAGE_SIZE),
            submode == eLozenge);
        break;
    }
    case eTriangle:
        object_triangle(
            angle0, pos1,
            clamp(abs(angledelta), QUARTER_IMAGE_SIZE, HALF_IMAGE_SIZE),
            -sigplus(dpos), -dpos, type, true, insertstack);
        break;
    case eSingle:
    case eNumInsertModes:
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
        int angle1;
        int pos1;
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

        case eNumModes:
            __builtin_unreachable();
        }
    }
}

bool sseditor::on_specialstageobjs_motion_notify_event(GdkEventMotion* event) {
    if (!specialstages) {
        return true;
    }

    bool lbutton_pressed = (event->state & GDK_BUTTON1_MASK) != 0;
    bool isdragging =
        hotspot.valid() && selection.find(hotspot) != selection.end();

    state   = event->state;
    mouse_x = event->x;
    mouse_y = event->y;
    drawbox = drawbox && lbutton_pressed;

    update();

    bool no_dragdrop =
        (mode != eSelectMode || drawbox || !lbutton_pressed || !isdragging);
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
        dpos   = y / SIMAGE_SIZE + get_scroll();
    }
    dangle = static_cast<int8_t>(dangle - lastclick.get_angle());
    dpos -= get_obj_pos<int>(lastclick);

    insertstack.clear();

    if (dangle == 0 && dpos == 0) {
        insertstack = sourcestack;
    } else {
        int maxpos = endpos - 1;
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
