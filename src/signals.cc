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

#include <cassert>
#include <fstream>

#include <s2ssedit/sseditor.hh>

using std::ifstream;
using std::ios;
using std::make_shared;
using std::shared_ptr;
using std::string;

void sseditor::on_vscrollbar_value_changed() {
    if (update_in_progress) {
        return;
    }
    currsegment = get_current_segment();
    render();
    update();
}

void sseditor::on_filedialog_response(int response_id) {
    if (response_id == Gtk::RESPONSE_OK) {
        string   dirname    = filedlg->get_filename() + '/';
        string   layoutfile = dirname + SS_LAYOUT_FILE;
        string   objectfile = dirname + SS_OBJECT_FILE;
        ifstream fobj(objectfile.c_str(), ios::in | ios::binary);
        ifstream flay(layoutfile.c_str(), ios::in | ios::binary);
        if (!fobj.good() || !flay.good()) {
            return;
        }

        fobj.close();
        flay.close();
        specialstages = make_shared<ssobj_file>(dirname);
        undostack.clear();
        redostack.clear();
        selection.clear();
        hotstack.clear();
        insertstack.clear();
        sourcestack.clear();
        currstage = currsegment = 0;
        update_segment_positions(true);
        render();
        update();
    }
    filedlg->hide();
}

void sseditor::on_helpdialog_response(int response_id) {
    ignore_unused_variable_warning(response_id);
    helpdlg->hide();
}

void sseditor::on_aboutdialog_response(int response_id) {
    ignore_unused_variable_warning(response_id);
    aboutdlg->hide();
}

void sseditor::on_openfilebutton_clicked() {
    if (filedlg == nullptr) {
        builder->get_widget("filechooserdialog", filedlg);
        filedlg->signal_response().connect(
            sigc::mem_fun(this, &sseditor::on_filedialog_response));
    }

    if (filedlg != nullptr) {
        filedlg->run();
    }
}

void sseditor::on_savefilebutton_clicked() {
    specialstages->write();
    update();
}

void sseditor::on_revertfilebutton_clicked() {
    undostack.clear();
    redostack.clear();
    selection.clear();
    hotstack.clear();
    insertstack.clear();
    sourcestack.clear();
    specialstages->read();
    currstage = currsegment = 0;
    update_segment_positions(true);
    render();
    update();
}

void sseditor::on_undobutton_clicked() {
    shared_ptr<abstract_action> act = undostack.front();
    undostack.pop_front();
    redostack.push_front(act);
    act->revert(specialstages, &selection);
    if (mode != eSelectMode) {
        selection.clear();
    }

    if (currstage >= specialstages->num_stages()) {
        currstage = specialstages->num_stages() - 1;
    }
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }

    render();
    update();
}

void sseditor::on_redobutton_clicked() {
    shared_ptr<abstract_action> act = redostack.front();
    redostack.pop_front();
    undostack.push_front(act);
    act->apply(specialstages, &selection);
    if (mode != eSelectMode) {
        selection.clear();
    }

    if (currstage >= specialstages->num_stages()) {
        currstage = specialstages->num_stages() - 1;
    }
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }

    render();
    update();
}

void sseditor::on_helpbutton_clicked() {
    if (helpdlg == nullptr) {
        builder->get_widget("helpdialog", helpdlg);
        helpdlg->signal_response().connect(
            sigc::mem_fun(this, &sseditor::on_helpdialog_response));
    }
    if (helpdlg != nullptr) {
        helpdlg->run();
    }
}

void sseditor::on_aboutbutton_clicked() {
    if (aboutdlg == nullptr) {
        builder->get_widget("aboutdialog", aboutdlg);
        aboutdlg->signal_response().connect(
            sigc::mem_fun(this, &sseditor::on_aboutdialog_response));
    }
    if (aboutdlg != nullptr) {
        aboutdlg->run();
    }
}

void sseditor::on_quitbutton_clicked() { kit->quit(); }

void sseditor::on_cutbutton_clicked() {
    copystack = selection;
    copypos   = static_cast<int>(pvscrollbar->get_value());
    do_action<cut_selection_action>(currstage, copystack);
    selection.clear();
    render();
    update();
}

void sseditor::on_copybutton_clicked() {
    copystack = selection;
    copypos   = static_cast<int>(pvscrollbar->get_value());
    update();
}

void sseditor::on_pastebutton_clicked() {
    int maxpos = static_cast<int>(endpos) - 1;
    int delta  = static_cast<int>(pvscrollbar->get_value()) - copypos;

    selection.clear();
    for (auto const& elem : copystack) {
        int newpos = delta + get_obj_pos<int>(elem);
        if (newpos < 0) {
            newpos = 0;
        } else if (newpos > maxpos) {
            newpos = maxpos;
        }

        int newseg = static_cast<int>(find_segment(newpos));
        int newy   = newpos - static_cast<int>(segpos[newseg]);
        selection.emplace(newseg, elem.get_angle(), newy, elem.get_type());
    }

    do_action<paste_objects_action>(currstage, selection);
    render();
    update();
}

void sseditor::on_deletebutton_clicked() {
    delete_set(selection);
    render();
    update();
}

void sseditor::on_first_stage_button_clicked() {
    currstage = 0;
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_previous_stage_button_clicked() {
    if (currstage > 0) {
        currstage--;
        update_segment_positions(false);
        if (currsegment >= segpos.size()) {
            goto_segment(segpos.size() - 1);
        }
        render();
        update();
    }
}

void sseditor::on_next_stage_button_clicked() {
    if (currstage + 1 < specialstages->num_stages()) {
        currstage++;
        update_segment_positions(false);
        if (currsegment >= segpos.size()) {
            goto_segment(segpos.size() - 1);
        }
        render();
        update();
    }
}

void sseditor::on_last_stage_button_clicked() {
    currstage = specialstages->num_stages() - 1;
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_insert_stage_before_button_clicked() {
    do_action<insert_stage_action>(currstage, sslevels());
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_append_stage_button_clicked() {
    currstage = specialstages->num_stages();
    do_action<insert_stage_action>(currstage, sslevels());
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_cut_stage_button_clicked() {
    sslevels* lev = specialstages->get_stage(currstage);
    copylevel     = make_shared<sslevels>(*lev);
    do_action<delete_stage_action>(currstage, *copylevel);
    if (currstage >= specialstages->num_stages()) {
        currstage = specialstages->num_stages() - 1;
    }
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_copy_stage_button_clicked() {
    sslevels* lev = specialstages->get_stage(currstage);
    copylevel     = make_shared<sslevels>(*lev);

    update();
}

void sseditor::on_paste_stage_button_clicked() {
    do_action<paste_stage_action>(++currstage, *copylevel);
    if (currstage >= specialstages->num_stages()) {
        currstage = specialstages->num_stages() - 1;
    }
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_delete_stage_button_clicked() {
    sslevels* lev = specialstages->get_stage(currstage);
    do_action<delete_stage_action>(currstage, *lev);
    if (currstage >= specialstages->num_stages()) {
        currstage = specialstages->num_stages() - 1;
    }
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_swap_stage_prev_button_clicked() {
    do_action<move_stage_action>(currstage, -1);
    // Sanity check.
    assert(currstage != 0);
    currstage--;
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_swap_stage_next_button_clicked() {
    do_action<move_stage_action>(currstage, +1);
    // Sanity check.
    assert(currstage + 1 < specialstages->num_stages());
    currstage++;
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_first_segment_button_clicked() {
    goto_segment(0);
    render();
    update();
}

void sseditor::on_previous_segment_button_clicked() {
    if (currsegment > 0) {
        goto_segment(currsegment - 1);
        render();
        update();
    }
}

void sseditor::on_next_segment_button_clicked() {
    if (currsegment + 1 < segpos.size()) {
        goto_segment(currsegment + 1);
        render();
        update();
    }
}

void sseditor::on_last_segment_button_clicked() {
    goto_segment(segpos.size() - 1);
    render();
    update();
}

void sseditor::on_insert_segment_before_button_clicked() {
    do_action<insert_segment_action>(currstage, currsegment, sssegments());
    update_segment_positions(true);
    render();
    update();
}

void sseditor::on_append_segment_button_clicked() {
    int cs = specialstages->get_stage(currstage)->num_segments();
    do_action<insert_segment_action>(currstage, cs, sssegments());
    update_segment_positions(false);
    goto_segment(segpos.size() - 1);
    render();
    update();
}

void sseditor::on_cut_segment_button_clicked() {
    sssegments* seg =
        specialstages->get_stage(currstage)->get_segment(currsegment);
    copyseg = make_shared<sssegments>(*seg);
    do_action<cut_segment_action>(currstage, currsegment, *copyseg);
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_copy_segment_button_clicked() {
    sssegments* seg =
        specialstages->get_stage(currstage)->get_segment(currsegment);
    copyseg = make_shared<sssegments>(*seg);

    update();
}

void sseditor::on_paste_segment_button_clicked() {
    do_action<paste_segment_action>(currstage, currsegment, *copyseg);
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_delete_segment_button_clicked() {
    sssegments const& seg =
        *specialstages->get_stage(currstage)->get_segment(currsegment);
    do_action<delete_segment_action>(currstage, currsegment, seg);
    update_segment_positions(false);
    if (currsegment >= segpos.size()) {
        goto_segment(segpos.size() - 1);
    }
    render();
    update();
}

void sseditor::on_swap_segment_prev_button_clicked() {
    do_action<move_segment_action>(currstage, currsegment, -1);
    // Sanity check.
    assert(currsegment != 0);
    currsegment--;
    update_segment_positions(true);
    render();
    update();
}

void sseditor::on_swap_segment_next_button_clicked() {
    do_action<move_segment_action>(currstage, currsegment, +1);
    // Sanity check.
    assert(currsegment + 1 < segpos.size());
    currsegment++;
    update_segment_positions(true);
    render();
    update();
}
