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

//#define DEBUG 1
#ifdef WIN32
#    define RINGFILE "./ring.png"
#    define BOMBFILE "./bomb.png"
#else
#    ifdef DEBUG
#        define RINGFILE "src/ring.png"
#        define BOMBFILE "src/bomb.png"
#    else
#        define RINGFILE PACKAGE_DATA_DIR "/s2ssedit/ui/ring.png"
#        define BOMBFILE PACKAGE_DATA_DIR "/s2ssedit/ui/bomb.png"
#    endif
#endif

using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::make_shared;
using std::set;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::swap;
using std::to_string;
using std::tie;
using std::tuple;
using std::vector;

sseditor::sseditor(int argc, char* argv[], char const* uifile)
    : update_in_progress(false), dragging(false), drop_enabled(false),
      currstage(0), currsegment(0), draw_width(0), draw_height(0), mouse_x(0),
      mouse_y(0), state(0), mode(eSelectMode), ringmode(eSingle),
      bombmode(eSingle), copypos(0), drawbox(false), snaptogrid(true),
      endpos(0), main_win(nullptr), helpdlg(nullptr), aboutdlg(nullptr),
      filedlg(nullptr), pspecialstageobjs(nullptr), pmodenotebook(nullptr),
      plabelcurrentstage(nullptr), plabeltotalstages(nullptr),
      plabelcurrentsegment(nullptr), plabeltotalsegments(nullptr),
      plabelcurrsegrings(nullptr), plabelcurrsegbombs(nullptr),
      plabelcurrsegshadows(nullptr), plabelcurrsegtotal(nullptr),
      pimagecurrsegwarn(nullptr), pvscrollbar(nullptr),
      popenfilebutton(nullptr), psavefilebutton(nullptr),
      prevertfilebutton(nullptr), pundobutton(nullptr), predobutton(nullptr),
      phelpbutton(nullptr), paboutbutton(nullptr),
      pquitbutton(nullptr), pmodebuttons{}, psnapgridbutton(nullptr),
      pcutbutton(nullptr), pcopybutton(nullptr), ppastebutton(nullptr),
      pdeletebutton(nullptr), pringmodebuttons{}, pbombmodebuttons{},
      pstage_toolbar(nullptr), pfirst_stage_button(nullptr),
      pprevious_stage_button(nullptr), pnext_stage_button(nullptr),
      plast_stage_button(nullptr), pinsert_stage_before_button(nullptr),
      pappend_stage_button(nullptr), pcut_stage_button(nullptr),
      pcopy_stage_button(nullptr), ppaste_stage_button(nullptr),
      pdelete_stage_button(nullptr), pswap_stage_prev_button(nullptr),
      pswap_stage_next_button(nullptr), psegment_toolbar(nullptr),
      pfirst_segment_button(nullptr), pprevious_segment_button(nullptr),
      pnext_segment_button(nullptr), plast_segment_button(nullptr),
      pinsert_segment_before_button(nullptr), pappend_segment_button(nullptr),
      pcut_segment_button(nullptr), pcopy_segment_button(nullptr),
      ppaste_segment_button(nullptr), pdelete_segment_button(nullptr),
      pswap_segment_prev_button(nullptr), pswap_segment_next_button(nullptr),
      psegment_expander(nullptr), pnormal_segment(nullptr),
      pring_message(nullptr), pcheckpoint(nullptr), pchaos_emerald(nullptr),
      psegment_turnthenrise(nullptr), psegment_turnthendrop(nullptr),
      psegment_turnthenstraight(nullptr), psegment_straight(nullptr),
      psegment_straightthenturn(nullptr), psegment_right(nullptr),
      psegment_left(nullptr), pobject_expander(nullptr), pmoveup(nullptr),
      pmovedown(nullptr), pmoveleft(nullptr), pmoveright(nullptr),
      pringtype(nullptr), pbombtype(nullptr) {
    kit = make_shared<Gtk::Main>(argc, argv);

    ringimg = Gdk::Pixbuf::create_from_file(RINGFILE);
    bombimg = Gdk::Pixbuf::create_from_file(BOMBFILE);

    // Load the Glade file and instiate its widgets:
    builder = Gtk::Builder::create_from_file(uifile);

    builder->get_widget("main_window", main_win);

    // All hail sed...
    builder->get_widget("specialstageobjs", pspecialstageobjs);
    pfilefilter = Glib::RefPtr<Gtk::FileFilter>::cast_dynamic(
        builder->get_object("filefilter"));
    builder->get_widget("modenotebook", pmodenotebook);
    // Labels
    builder->get_widget("labelcurrentstage", plabelcurrentstage);
    builder->get_widget("labeltotalstages", plabeltotalstages);
    builder->get_widget("labelcurrentsegment", plabelcurrentsegment);
    builder->get_widget("labeltotalsegments", plabeltotalsegments);
    builder->get_widget("labelcurrsegrings", plabelcurrsegrings);
    builder->get_widget("labelcurrsegbombs", plabelcurrsegbombs);
    builder->get_widget("labelcurrsegshadows", plabelcurrsegshadows);
    builder->get_widget("labelcurrsegtotal", plabelcurrsegtotal);
    // Images
    builder->get_widget("imagecurrsegwarn", pimagecurrsegwarn);
    // Scrollbar
    builder->get_widget("vscrollbar", pvscrollbar);
    // Main toolbar
    builder->get_widget("openfilebutton", popenfilebutton);
    builder->get_widget("savefilebutton", psavefilebutton);
    builder->get_widget("revertfilebutton", prevertfilebutton);
    builder->get_widget("undobutton", pundobutton);
    builder->get_widget("redobutton", predobutton);
    builder->get_widget("selectmodebutton", pmodebuttons[eSelectMode]);
    builder->get_widget("insertringbutton", pmodebuttons[eInsertRingMode]);
    builder->get_widget("insertbombbutton", pmodebuttons[eInsertBombMode]);
    builder->get_widget("deletemodebutton", pmodebuttons[eDeleteMode]);
    builder->get_widget("snapgridbutton", psnapgridbutton);
    builder->get_widget("helpbutton", phelpbutton);
    builder->get_widget("aboutbutton", paboutbutton);
    builder->get_widget("quitbutton", pquitbutton);
    // Selection toolbar
    builder->get_widget("cutbutton", pcutbutton);
    builder->get_widget("copybutton", pcopybutton);
    builder->get_widget("pastebutton", ppastebutton);
    builder->get_widget("deletebutton", pdeletebutton);
    // Insert ring toolbar
    builder->get_widget("ringsinglebutton", pringmodebuttons[eSingle]);
    builder->get_widget("ringlinebutton", pringmodebuttons[eLine]);
    builder->get_widget("ringloopbutton", pringmodebuttons[eLoop]);
    builder->get_widget("ringzigbutton", pringmodebuttons[eZigzag]);
    builder->get_widget("ringdiamondbutton", pringmodebuttons[eDiamond]);
    builder->get_widget("ringlozengebutton", pringmodebuttons[eLozenge]);
    builder->get_widget("ringstarbutton", pringmodebuttons[eStar]);
    builder->get_widget("ringtrianglebutton", pringmodebuttons[eTriangle]);
    // Insert bomb toolbar
    builder->get_widget("bombsinglebutton", pbombmodebuttons[eSingle]);
    builder->get_widget("bomblinebutton", pbombmodebuttons[eLine]);
    builder->get_widget("bombloopbutton", pbombmodebuttons[eLoop]);
    builder->get_widget("bombzigbutton", pbombmodebuttons[eZigzag]);
    builder->get_widget("bombdiamondbutton", pbombmodebuttons[eDiamond]);
    builder->get_widget("bomblozengebutton", pbombmodebuttons[eLozenge]);
    builder->get_widget("bombstarbutton", pbombmodebuttons[eStar]);
    builder->get_widget("bombtrianglebutton", pbombmodebuttons[eTriangle]);
    // Special stage toolbar
    builder->get_widget("stage_toolbar", pstage_toolbar);
    builder->get_widget("first_stage_button", pfirst_stage_button);
    builder->get_widget("previous_stage_button", pprevious_stage_button);
    builder->get_widget("next_stage_button", pnext_stage_button);
    builder->get_widget("last_stage_button", plast_stage_button);
    builder->get_widget(
        "insert_stage_before_button", pinsert_stage_before_button);
    builder->get_widget("append_stage_button", pappend_stage_button);
    builder->get_widget("cut_stage_button", pcut_stage_button);
    builder->get_widget("copy_stage_button", pcopy_stage_button);
    builder->get_widget("paste_stage_button", ppaste_stage_button);
    builder->get_widget("delete_stage_button", pdelete_stage_button);
    builder->get_widget("swap_stage_prev_button", pswap_stage_prev_button);
    builder->get_widget("swap_stage_next_button", pswap_stage_next_button);
    // Segment toolbar
    builder->get_widget("segment_toolbar", psegment_toolbar);
    builder->get_widget("first_segment_button", pfirst_segment_button);
    builder->get_widget("previous_segment_button", pprevious_segment_button);
    builder->get_widget("next_segment_button", pnext_segment_button);
    builder->get_widget("last_segment_button", plast_segment_button);
    builder->get_widget(
        "insert_segment_before_button", pinsert_segment_before_button);
    builder->get_widget("append_segment_button", pappend_segment_button);
    builder->get_widget("cut_segment_button", pcut_segment_button);
    builder->get_widget("copy_segment_button", pcopy_segment_button);
    builder->get_widget("paste_segment_button", ppaste_segment_button);
    builder->get_widget("delete_segment_button", pdelete_segment_button);
    builder->get_widget("swap_segment_prev_button", pswap_segment_prev_button);
    builder->get_widget("swap_segment_next_button", pswap_segment_next_button);
    // Segment flags
    builder->get_widget("segment_expander", psegment_expander);
    builder->get_widget("normal_segment", pnormal_segment);
    builder->get_widget("ring_message", pring_message);
    builder->get_widget("checkpoint", pcheckpoint);
    builder->get_widget("chaos_emerald", pchaos_emerald);
    builder->get_widget("segment_turnthenrise", psegment_turnthenrise);
    builder->get_widget("segment_turnthendrop", psegment_turnthendrop);
    builder->get_widget("segment_turnthenstraight", psegment_turnthenstraight);
    builder->get_widget("segment_straight", psegment_straight);
    builder->get_widget("segment_straightthenturn", psegment_straightthenturn);
    builder->get_widget("segment_right", psegment_right);
    builder->get_widget("segment_left", psegment_left);
    // Object flags
    builder->get_widget("object_expander", pobject_expander);
    builder->get_widget("moveup", pmoveup);
    builder->get_widget("movedown", pmovedown);
    builder->get_widget("moveleft", pmoveleft);
    builder->get_widget("moveright", pmoveright);
    builder->get_widget("ringtype", pringtype);
    builder->get_widget("bombtype", pbombtype);

    pfilefilter->add_pattern("*");

    pspecialstageobjs->signal_configure_event().connect(
        sigc::mem_fun(this, &sseditor::on_specialstageobjs_configure_event));
    pspecialstageobjs->signal_expose_event().connect(
        sigc::mem_fun(this, &sseditor::on_specialstageobjs_expose_event));
    pspecialstageobjs->signal_key_press_event().connect(
        sigc::mem_fun(this, &sseditor::on_specialstageobjs_key_press_event));
    pspecialstageobjs->signal_button_press_event().connect(
        sigc::mem_fun(this, &sseditor::on_specialstageobjs_button_press_event));
    pspecialstageobjs->signal_button_release_event().connect(sigc::mem_fun(
        this, &sseditor::on_specialstageobjs_button_release_event));
    pspecialstageobjs->signal_scroll_event().connect(
        sigc::mem_fun(this, &sseditor::on_specialstageobjs_scroll_event));
    pspecialstageobjs->signal_drag_begin().connect(
        sigc::mem_fun(this, &sseditor::on_specialstageobjs_drag_begin));
    pspecialstageobjs->signal_motion_notify_event().connect(sigc::mem_fun(
        this, &sseditor::on_specialstageobjs_motion_notify_event));
    pspecialstageobjs->signal_drag_data_get().connect(
        sigc::mem_fun(this, &sseditor::on_specialstageobjs_drag_data_get));
    pspecialstageobjs->signal_drag_end().connect(
        sigc::mem_fun(this, &sseditor::on_specialstageobjs_drag_end));
    // Scrollbar
    pvscrollbar->signal_value_changed().connect(
        sigc::mem_fun(this, &sseditor::on_vscrollbar_value_changed));
    // Main toolbar
    popenfilebutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_openfilebutton_clicked));
    psavefilebutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_savefilebutton_clicked));
    prevertfilebutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_revertfilebutton_clicked));
    pundobutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_undobutton_clicked));
    predobutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_redobutton_clicked));
    pmodebuttons[eSelectMode]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_modebutton_toggled<eSelectMode>));
    pmodebuttons[eInsertRingMode]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_modebutton_toggled<eInsertRingMode>));
    pmodebuttons[eInsertBombMode]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_modebutton_toggled<eInsertBombMode>));
    pmodebuttons[eDeleteMode]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_modebutton_toggled<eDeleteMode>));
    psnapgridbutton->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_snapgridbutton_toggled), true);
    phelpbutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_helpbutton_clicked));
    paboutbutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_aboutbutton_clicked));
    pquitbutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_quitbutton_clicked));
    // Selection toolbar
    pcutbutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_cutbutton_clicked));
    pcopybutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_copybutton_clicked));
    ppastebutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_pastebutton_clicked));
    pdeletebutton->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_deletebutton_clicked));
    // Insert ring toolbar
    pringmodebuttons[eSingle]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_ringmode_toggled<eSingle>));
    pringmodebuttons[eLine]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_ringmode_toggled<eLine>));
    pringmodebuttons[eLoop]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_ringmode_toggled<eLoop>));
    pringmodebuttons[eZigzag]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_ringmode_toggled<eZigzag>));
    pringmodebuttons[eDiamond]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_ringmode_toggled<eDiamond>));
    pringmodebuttons[eLozenge]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_ringmode_toggled<eLozenge>));
    pringmodebuttons[eStar]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_ringmode_toggled<eStar>));
    pringmodebuttons[eTriangle]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_ringmode_toggled<eTriangle>));
    // Insert bomb toolbar
    pbombmodebuttons[eSingle]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_bombmode_toggled<eSingle>));
    pbombmodebuttons[eLine]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_bombmode_toggled<eLine>));
    pbombmodebuttons[eLoop]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_bombmode_toggled<eLoop>));
    pbombmodebuttons[eZigzag]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_bombmode_toggled<eZigzag>));
    pbombmodebuttons[eDiamond]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_bombmode_toggled<eDiamond>));
    pbombmodebuttons[eLozenge]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_bombmode_toggled<eLozenge>));
    pbombmodebuttons[eStar]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_bombmode_toggled<eStar>));
    pbombmodebuttons[eTriangle]->signal_toggled().connect(
        sigc::mem_fun(this, &sseditor::on_bombmode_toggled<eTriangle>));
    // Special stage toolbar
    pfirst_stage_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_first_stage_button_clicked));
    pprevious_stage_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_previous_stage_button_clicked));
    pnext_stage_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_next_stage_button_clicked));
    plast_stage_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_last_stage_button_clicked));
    pinsert_stage_before_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_insert_stage_before_button_clicked));
    pappend_stage_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_append_stage_button_clicked));
    pcut_stage_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_cut_stage_button_clicked));
    pcopy_stage_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_copy_stage_button_clicked));
    ppaste_stage_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_paste_stage_button_clicked));
    pdelete_stage_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_delete_stage_button_clicked));
    pswap_stage_prev_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_swap_stage_prev_button_clicked));
    pswap_stage_next_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_swap_stage_next_button_clicked));
    // Segment toolbar
    pfirst_segment_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_first_segment_button_clicked));
    pprevious_segment_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_previous_segment_button_clicked));
    pnext_segment_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_next_segment_button_clicked));
    plast_segment_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_last_segment_button_clicked));
    pinsert_segment_before_button->signal_clicked().connect(sigc::mem_fun(
        this, &sseditor::on_insert_segment_before_button_clicked));
    pappend_segment_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_append_segment_button_clicked));
    pcut_segment_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_cut_segment_button_clicked));
    pcopy_segment_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_copy_segment_button_clicked));
    ppaste_segment_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_paste_segment_button_clicked));
    pdelete_segment_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_delete_segment_button_clicked));
    pswap_segment_prev_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_swap_segment_prev_button_clicked));
    pswap_segment_next_button->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_swap_segment_next_button_clicked));
    // Segment flags
    pnormal_segment->signal_toggled().connect(sigc::mem_fun(
        this, &sseditor::on_segmenttype_toggled<
                  sssegments::eNormalSegment, &sseditor::pnormal_segment>));
    pring_message->signal_toggled().connect(sigc::mem_fun(
        this, &sseditor::on_segmenttype_toggled<
                  sssegments::eRingsMessage, &sseditor::pring_message>));
    pcheckpoint->signal_toggled().connect(sigc::mem_fun(
        this, &sseditor::on_segmenttype_toggled<
                  sssegments::eCheckpoint, &sseditor::pcheckpoint>));
    pchaos_emerald->signal_toggled().connect(sigc::mem_fun(
        this, &sseditor::on_segmenttype_toggled<
                  sssegments::eChaosEmerald, &sseditor::pchaos_emerald>));
    psegment_turnthenrise->signal_toggled().connect(sigc::mem_fun(
        this,
        &sseditor::on_segment_segmentgeometry_toggled<
            sssegments::eTurnThenRise, &sseditor::psegment_turnthenrise>));
    psegment_turnthendrop->signal_toggled().connect(sigc::mem_fun(
        this,
        &sseditor::on_segment_segmentgeometry_toggled<
            sssegments::eTurnThenDrop, &sseditor::psegment_turnthendrop>));
    psegment_turnthenstraight->signal_toggled().connect(sigc::mem_fun(
        this, &sseditor::on_segment_segmentgeometry_toggled<
                  sssegments::eTurnThenStraight,
                  &sseditor::psegment_turnthenstraight>));
    psegment_straight->signal_toggled().connect(sigc::mem_fun(
        this, &sseditor::on_segment_segmentgeometry_toggled<
                  sssegments::eStraight, &sseditor::psegment_straight>));
    psegment_straightthenturn->signal_toggled().connect(sigc::mem_fun(
        this, &sseditor::on_segment_segmentgeometry_toggled<
                  sssegments::eStraightThenTurn,
                  &sseditor::psegment_straightthenturn>));
    psegment_right->signal_toggled().connect(sigc::mem_fun(
        this, &sseditor::on_segmentdirection_toggled<
                  false, &sseditor::psegment_right>));
    psegment_left->signal_toggled().connect(sigc::mem_fun(
        this, &sseditor::on_segmentdirection_toggled<
                  true, &sseditor::psegment_left>));
    // Object flags
    pmoveup->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_movebtn_clicked<0, -1>));
    pmovedown->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_movebtn_clicked<0, 1>));
    pmoveleft->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_movebtn_clicked<-1, 0>));
    pmoveright->signal_clicked().connect(
        sigc::mem_fun(this, &sseditor::on_movebtn_clicked<1, 0>));
    pringtype->signal_toggled().connect(sigc::mem_fun(
        this, &sseditor::on_objecttype_toggled<
                  sssegments::eRing, &sseditor::pringtype>));
    pbombtype->signal_toggled().connect(sigc::mem_fun(
        this, &sseditor::on_objecttype_toggled<
                  sssegments::eBomb, &sseditor::pbombtype>));

    update();
}

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

void sseditor::update() {
    if (update_in_progress) {
        return;
    }

    update_in_progress = true;

    if (!specialstages) {
        psavefilebutton->set_sensitive(false);
        prevertfilebutton->set_sensitive(false);

        for (auto& elem : pmodebuttons) {
            elem->set_sensitive(false);
        }

        pmodenotebook->set_sensitive(false);
        pstage_toolbar->set_sensitive(false);
        psegment_toolbar->set_sensitive(false);

        psegment_expander->set_sensitive(false);
        pobject_expander->set_sensitive(false);
        pringtype->set_inconsistent(true);
        pbombtype->set_inconsistent(true);
        plabelcurrsegrings->set_label("0");
        plabelcurrsegbombs->set_label("0");
        plabelcurrsegshadows->set_label("0");
        plabelcurrsegtotal->set_label("0/100");
        pimagecurrsegwarn->set_visible(false);
        disable_scroll();
    } else {
        psavefilebutton->set_sensitive(true);
        prevertfilebutton->set_sensitive(true);
        pundobutton->set_sensitive(!undostack.empty());
        predobutton->set_sensitive(!redostack.empty());

        for (auto& elem : pmodebuttons) {
            elem->set_sensitive(true);
        }

        pmodenotebook->set_sensitive(true);
        pstage_toolbar->set_sensitive(true);

        unsigned numstages = specialstages->num_stages();

        if (numstages == 0 || currstage == numstages - 1 ||
            numstages <= currstage) {
            if (numstages == 0) {
                currstage = 0;
            } else {
                currstage = numstages - 1;
            }
            pnext_stage_button->set_sensitive(false);
            plast_stage_button->set_sensitive(false);
            pswap_stage_next_button->set_sensitive(false);
        } else {
            pnext_stage_button->set_sensitive(true);
            plast_stage_button->set_sensitive(true);
            pswap_stage_next_button->set_sensitive(true);
        }

        if (currstage == 0 || numstages == 0) {
            pfirst_stage_button->set_sensitive(false);
            pprevious_stage_button->set_sensitive(false);
            pswap_stage_prev_button->set_sensitive(false);
        } else {
            pfirst_stage_button->set_sensitive(true);
            pprevious_stage_button->set_sensitive(true);
            pswap_stage_prev_button->set_sensitive(true);
        }

        if (numstages == 0) {
            pcutbutton->set_sensitive(false);
            pcopybutton->set_sensitive(false);
            ppastebutton->set_sensitive(false);
            pdeletebutton->set_sensitive(false);
            psegment_toolbar->set_sensitive(false);
            psegment_expander->set_sensitive(false);
            pobject_expander->set_sensitive(false);
            pringtype->set_inconsistent(true);
            pbombtype->set_inconsistent(true);
            pinsert_stage_before_button->set_sensitive(false);
            pcut_stage_button->set_sensitive(false);
            pcopy_stage_button->set_sensitive(false);
            ppaste_stage_button->set_sensitive(false);
            pdelete_stage_button->set_sensitive(false);
            plabeltotalstages->set_label("0");
            plabelcurrentstage->set_label("0");
            plabeltotalsegments->set_label("0");
            plabelcurrentsegment->set_label("0");
            plabelcurrsegrings->set_label("0");
            plabelcurrsegbombs->set_label("0");
            plabelcurrsegshadows->set_label("0");
            plabelcurrsegtotal->set_label("0/100");
            pimagecurrsegwarn->set_visible(false);
        } else {
            psegment_toolbar->set_sensitive(true);
            pinsert_stage_before_button->set_sensitive(true);
            pcut_stage_button->set_sensitive(true);
            pcopy_stage_button->set_sensitive(true);
            ppaste_stage_button->set_sensitive(copylevel != nullptr);
            pdelete_stage_button->set_sensitive(true);
            plabeltotalstages->set_label(to_string(numstages));
            plabelcurrentstage->set_label(to_string(currstage + 1));

            sslevels* currlvl     = specialstages->get_stage(currstage);
            unsigned  numsegments = segpos.size();

            if (numsegments == 0 || currsegment == numsegments - 1 ||
                numsegments <= currsegment) {
                if (numsegments == 0) {
                    currsegment = 0;
                } else {
                    currsegment = numsegments - 1;
                }
                pnext_segment_button->set_sensitive(false);
                plast_segment_button->set_sensitive(false);
                pswap_segment_next_button->set_sensitive(false);
            } else {
                pnext_segment_button->set_sensitive(true);
                plast_segment_button->set_sensitive(true);
                pswap_segment_next_button->set_sensitive(true);
            }

            if (currsegment == 0 || numsegments == 0) {
                pfirst_segment_button->set_sensitive(false);
                pprevious_segment_button->set_sensitive(false);
                pswap_segment_prev_button->set_sensitive(false);
            } else {
                pfirst_segment_button->set_sensitive(true);
                pprevious_segment_button->set_sensitive(true);
                pswap_segment_prev_button->set_sensitive(true);
            }

            if (numsegments == 0) {
                pcutbutton->set_sensitive(false);
                pcopybutton->set_sensitive(false);
                ppastebutton->set_sensitive(false);
                pdeletebutton->set_sensitive(false);
                psegment_expander->set_sensitive(false);
                pobject_expander->set_sensitive(false);
                pringtype->set_inconsistent(true);
                pbombtype->set_inconsistent(true);
                pinsert_segment_before_button->set_sensitive(false);
                pcut_segment_button->set_sensitive(false);
                pcopy_segment_button->set_sensitive(false);
                ppaste_segment_button->set_sensitive(false);
                pdelete_segment_button->set_sensitive(false);
                plabeltotalsegments->set_label("0");
                plabelcurrentsegment->set_label("0");
                plabelcurrsegrings->set_label("0");
                plabelcurrsegbombs->set_label("0");
                plabelcurrsegshadows->set_label("0");
                plabelcurrsegtotal->set_label("0/100");
                pimagecurrsegwarn->set_visible(false);
            } else {
                pcutbutton->set_sensitive(!selection.empty());
                pcopybutton->set_sensitive(!selection.empty());
                ppastebutton->set_sensitive(!copystack.empty());
                pdeletebutton->set_sensitive(!selection.empty());

                pinsert_segment_before_button->set_sensitive(true);
                pcut_segment_button->set_sensitive(true);
                pcopy_segment_button->set_sensitive(true);
                ppaste_segment_button->set_sensitive(copyseg != nullptr);
                pdelete_segment_button->set_sensitive(true);

                psegment_expander->set_sensitive(true);
                plabeltotalsegments->set_label(to_string(numsegments));
                plabelcurrentsegment->set_label(to_string(currsegment + 1));

                sssegments* currseg = currlvl->get_segment(currsegment);
                plabelcurrsegrings->set_label(
                    to_string(currseg->get_numrings()));
                plabelcurrsegbombs->set_label(
                    to_string(currseg->get_numbombs()));
                plabelcurrsegshadows->set_label(
                    to_string(currseg->get_numshadows()));
                plabelcurrsegtotal->set_label(
                    to_string(currseg->get_totalobjs()) + "/100");
                constexpr const uint16_t max_safe_num_objs = 100;
                pimagecurrsegwarn->set_visible(
                    currseg->get_totalobjs() > max_safe_num_objs);

                switch (currseg->get_type()) {
                case sssegments::eRingsMessage:
                    pring_message->set_active(true);
                    break;
                case sssegments::eCheckpoint:
                    pcheckpoint->set_active(true);
                    break;
                case sssegments::eChaosEmerald:
                    pchaos_emerald->set_active(true);
                    break;
                default:
                    pnormal_segment->set_active(true);
                    break;
                }
                switch (currseg->get_geometry()) {
                case sssegments::eTurnThenDrop:
                    psegment_turnthendrop->set_active(true);
                    break;
                case sssegments::eTurnThenStraight:
                    psegment_turnthenstraight->set_active(true);
                    break;
                case sssegments::eStraight:
                    psegment_straight->set_active(true);
                    break;
                case sssegments::eStraightThenTurn:
                    psegment_straightthenturn->set_active(true);
                    break;
                default:
                    psegment_turnthenrise->set_active(true);
                    break;
                }
                if (currseg->get_direction()) {
                    psegment_left->set_active(true);
                } else {
                    psegment_right->set_active(true);
                }

                if (selection.empty()) {
                    pringtype->set_inconsistent(true);
                    pbombtype->set_inconsistent(true);
                    pobject_expander->set_sensitive(false);
                } else {
                    size_t nrings = 0;
                    size_t nbombs = 0;
                    for (auto const& elem : selection) {
                        if (elem.get_type() == sssegments::eBomb) {
                            ++nbombs;
                        } else {
                            ++nrings;
                        }
                    }
                    pobject_expander->set_sensitive(true);
                    if ((nbombs > 0 && nrings > 0) ||
                        (nbombs == 0 && nrings == 0)) {
                        pringtype->set_inconsistent(true);
                        pbombtype->set_inconsistent(true);
                    } else if (nbombs > 0) {
                        pringtype->set_inconsistent(false);
                        pbombtype->set_inconsistent(false);
                        pbombtype->set_active(true);
                    } else {
                        pringtype->set_inconsistent(false);
                        pbombtype->set_inconsistent(false);
                        pringtype->set_active(true);
                    }
                }
            }
        }
    }

    show();

    update_in_progress = false;
}

void sseditor::run() {
    if (main_win != nullptr) {
        kit->run(*main_win);
    }
}

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

void sseditor::on_helpdialog_response(int response_id) {
    ignore_unused_variable_warning(response_id);
    helpdlg->hide();
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

void sseditor::on_aboutdialog_response(int response_id) {
    ignore_unused_variable_warning(response_id);
    aboutdlg->hide();
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
        if (drawimg) {
            drawimg->unreference();
        }
        drawimg = cr->pop_group();
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

    for (int i = start; i <= end; i++) {
        int         seg     = find_segment(i);
        sssegments* currseg = get_segment(seg);
        if (currseg == nullptr) {
            break;
        }

        if ((i + 1) % 4 == 0) {
            cr->set_line_width(HALF_IMAGE_SIZE);
            cr->set_source_rgb(1.0, 180.0 / 256.0, 36.0 / 256.0);
            // Horizontal beams.
            int ty = (i - pvscrollbar->get_value()) * IMAGE_SIZE;
            cr->move_to(angle_to_x(0x00), ty);
            cr->line_to(angle_to_x(0x80), ty);
            cr->stroke();
            // Horizontal balls.
            for (int iangle = 0; iangle < 3; iangle++) {
                double angle = (iangle * 64.0) / 3.0;
                cr->set_source_rgb(180.0 / 256.0, 108.0 / 256.0, 36.0 / 256.0);
                cr->arc(
                    angle_to_x(angle + 0x80) + HALF_IMAGE_SIZE, ty,
                    HALF_IMAGE_SIZE, 0.0, 2.0 * G_PI);
                cr->begin_new_sub_path();
                cr->arc(
                    angle_to_x(0x00 - angle) - HALF_IMAGE_SIZE, ty,
                    HALF_IMAGE_SIZE, 0.0, 2.0 * G_PI);
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

        if ((seg == 2 &&
             (i - segpos[seg] == (currseg->get_length() - 1) / 2)) ||
            ((currseg->get_type() == sssegments::eCheckpoint ||
              currseg->get_type() == sssegments::eChaosEmerald) &&
             (i - segpos[seg]) == currseg->get_length() - 1)) {
            cr->save();
            vector<double> dash;
            dash.push_back(8.0);
            int ty = (i - pvscrollbar->get_value()) * IMAGE_SIZE;
            cr->set_dash(dash, 0.0);
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

    for (int i = start; i <= end; i++) {
        int         seg     = find_segment(i);
        sssegments* currseg = get_segment(seg);
        if (currseg == nullptr) {
            break;
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

    if (drawimg) {
        drawimg->unreference();
    }
    drawimg = cr->pop_group();
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
            cr->set_source_rgba(0.0, 1.0, 0.0, 0.25);
            cr->rectangle(tx0, ty0, tx1 - tx0, ty1 - ty0);
            cr->fill_preserve();
            cr->set_source_rgb(0.0, 1.0, 0.0);
            cr->set_line_width(1.0);
            cr->stroke();
        }
    } else if (mode == eDeleteMode) {
        if (drawbox) {
            cr->set_line_width(2.0);
            cr->set_source_rgb(1.0, 0.0, 0.0);
            draw_x(hotstack, cr);
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
            cr->set_source_rgba(1.0, 0.0, 0.0, 0.25);
            cr->rectangle(tx0, ty0, tx1 - tx0, ty1 - ty0);
            cr->fill_preserve();
            cr->set_source_rgb(1.0, 0.0, 0.0);
            cr->set_line_width(1.0);
            cr->stroke();
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
        return move_object(want_snap_to_grid(event->state) ? -4 : -1, 0);
    }

    case GDK_KEY_Right:
    case GDK_KEY_KP_Right: {
        return move_object(want_snap_to_grid(event->state) ? 4 : 1, 0);
    }

    case GDK_KEY_Page_Up:
    case GDK_KEY_KP_Page_Up: {
        bool   shift = (event->state & GDK_SHIFT_MASK) != 0;
        double delta = shift ? 32 : 4;
        pvscrollbar->set_value(pvscrollbar->get_value() - delta);
        render();
        show();
        break;
    }

    case GDK_KEY_Page_Down:
    case GDK_KEY_KP_Page_Down: {
        bool   shift = (event->state & GDK_SHIFT_MASK) != 0;
        double delta = shift ? 32 : 4;
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

tuple<int,int,int> sseditor::get_mouseup_loc(GdkEventButton* event) {
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
    return {angle, pos, seg};
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
            cycle_object_type(seg, angle, pos);
        }
        break;

    case eDeleteMode: {
        if (!hotstack.empty()) {
            delete_set(hotstack);
        } else {
            delete_existing_object(seg, angle, pos);
        }
        break;
    }
    case eInsertBombMode:
    case eInsertRingMode: {
        if (event->button == GDK_BUTTON_LEFT) {
            insert_set();
        } else if (event->button == GDK_BUTTON_RIGHT) {
            delete_existing_object(seg, angle, pos);
        }
        break;
    }
    default:
        assert(false);
    }
    render();
    update();
    return true;
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
        if (ctrl) {
            if (mode == eInsertRingMode) {
                ringmode = InsertModes(
                    (int(ringmode) + int(eNumInsertModes) - 1) %
                    int(eNumInsertModes));
                pringmodebuttons[ringmode]->set_active(true);
            } else if (mode == eInsertBombMode) {
                bombmode = InsertModes(
                    (int(bombmode) + int(eNumInsertModes) - 1) %
                    int(eNumInsertModes));
                pbombmodebuttons[bombmode]->set_active(true);
            }
        } else {
            mode = EditModes((int(mode) + int(eNumModes) - 1) % int(eNumModes));
            pmodebuttons[mode]->set_active(true);
        }
        break;

    case GDK_SCROLL_RIGHT:
        if (ctrl) {
            if (mode == eInsertRingMode) {
                ringmode =
                    InsertModes((int(ringmode) + 1) % int(eNumInsertModes));
                pringmodebuttons[ringmode]->set_active(true);
            } else if (mode == eInsertBombMode) {
                bombmode =
                    InsertModes((int(bombmode) + 1) % int(eNumInsertModes));
                pbombmodebuttons[bombmode]->set_active(true);
            }
        } else {
            mode = EditModes((int(mode) + 1) % int(eNumModes));
            pmodebuttons[mode]->set_active(true);
        }
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

bool sseditor::on_specialstageobjs_motion_notify_event(GdkEventMotion* event) {
    if (!specialstages) {
        return true;
    }

    state = event->state;

    mouse_x = event->x;
    mouse_y = event->y;

    if ((event->state & GDK_BUTTON1_MASK) == 0) {
        drawbox = false;
    }
    if (drawbox || mode == eInsertRingMode || mode == eInsertBombMode ||
        ((event->state & GDK_BUTTON1_MASK) != 0 &&
         (!hotspot.valid() || selection.find(hotspot) == selection.end()))) {
        if ((event->state & GDK_BUTTON1_MASK) != 0) {
            if (event->y < 5) {
                pvscrollbar->set_value(pvscrollbar->get_value() - 4.0);
            } else if (draw_height - event->y < 5) {
                pvscrollbar->set_value(pvscrollbar->get_value() + 4.0);
            }
        }
        int angle1, pos1;
        if (!hotspot.valid()) {
            angle1 = x_to_angle(event->x, want_snap_to_grid(state), 4U);
            pos1   = event->y / IMAGE_SIZE + pvscrollbar->get_value();
        } else {
            angle1 = hotspot.get_angle();
            pos1   = get_obj_pos<int>(hotspot);
        }
        int angle0 = lastclick.get_angle(), pos0 = get_obj_pos<int>(lastclick);
        angle0     = angle_simple(angle0);
        angle1     = angle_simple(angle1);
        int dangle = angle1 - angle0, dpos = pos1 - pos0;
        int numsegments = segpos.size();
        if (mode == eSelectMode || mode == eDeleteMode) {
            hotstack.clear();
        } else {
            insertstack.clear();
        }

        switch (mode) {
        case eSelectMode:
        case eDeleteMode:
            if (dangle || dpos) {
                int seg1 = find_segment(pos1);
                boxcorner.set(
                    seg1, static_cast<int8_t>(angle1 + 0xc0),
                    pos1 - segpos[seg1], sssegments::eRing);
                if (dpos < 0) {
                    swap(pos0, pos1);
                }
                if (dangle < 0) {
                    swap(angle0, angle1);
                }
                drawbox = true;
                for (int i = pos0; i <= pos1; i++) {
                    int         seg = find_segment(i), pos = i - segpos[seg];
                    sssegments* currseg = get_segment(seg);
                    if (currseg == nullptr) {
                        continue;
                    }
                    for (int j = angle0; j <= angle1; j++) {
                        int         angle = static_cast<int8_t>(j + 0xc0);
                        ObjectTypes type;
                        if (currseg->exists(pos, angle, type)) {
                            hotstack.emplace(seg, angle, pos, type);
                        }
                    }
                }
            }
            break;

        case eInsertRingMode:
        case eInsertBombMode: {
            ObjectTypes type;
            InsertModes submode;
            if (mode == eInsertBombMode) {
                type    = sssegments::eBomb;
                submode = bombmode;
            } else {
                type    = sssegments::eRing;
                submode = ringmode;
            }

            if (submode == eSingle || (submode != eLoop && !dpos) ||
                (event->state & GDK_BUTTON1_MASK) == 0) {
                int seg1 = find_segment(pos1);
                insertstack.emplace(
                    seg1, angle_normal(angle1), pos1 - segpos[seg1], type);
                break;
            }

            int angledelta = dangle;
            if (submode == eLine) {
                angledelta /= (dpos > 0 ? dpos : -dpos);
            }
            if (want_snap_to_grid(event->state)) {
                angledelta += (angledelta >= 0 ? 3 : -3);
                angledelta /= 4;
                angledelta *= 4;
            }

            switch (submode) {
            case eLine: {
                int const delta = dpos >= 0 ? 1 : -1;
                int       angle = angle0;
                int       i     = pos0;
                do {
                    int seg = find_segment(i), pos = i - segpos[seg];
                    i += delta;
                    if (seg >= numsegments) {
                        continue;
                    }

                    insertstack.emplace(seg, angle_normal(angle), pos, type);
                    angle = static_cast<int8_t>(angle + angledelta);
                } while (i != pos1 + delta);
                break;
            }
            case eLoop: {
                int    dy = dpos == 0 ? 0 : (dpos > 0 ? 1 : -1);
                int    nobj;
                double delta;
                if (dy != 0) {
                    if (want_snap_to_grid(event->state)) {
                        if (angledelta >= 0 && angledelta < 4) {
                            angledelta = 4;
                        } else if (angledelta < 0 && angledelta > -4) {
                            angledelta = -4;
                        }
                    } else if (!angledelta) {
                        angledelta = 1;
                    }
                    int sign = angledelta >= 0 ? 1 : -1;
                    int dx   = sign * angledelta;
                    nobj     = 0x100 / dx;
                    delta    = sign * (double(0x100) / nobj);
                } else {
                    if (angledelta < 0) {
                        angledelta = -angledelta;
                    }
                    if (angledelta < HALF_IMAGE_SIZE) {
                        angledelta = HALF_IMAGE_SIZE;
                    }
                    nobj  = (0x100 - angledelta) / angledelta;
                    delta = double(0x100 - angledelta) / nobj;
                }

                double angle = angle0;
                for (int i = 0; i <= nobj; i++, pos0 += dy) {
                    int seg = find_segment(pos0), pos = pos0 - segpos[seg];
                    if (seg >= numsegments) {
                        continue;
                    }
                    insertstack.emplace(
                        seg, angle_normal(static_cast<int8_t>(angle)), pos,
                        type);
                    angle += delta;
                }
                break;
            }
            case eZigzag: {
                int const delta = dpos >= 0 ? 1 : -1;
                if (angledelta > HALF_IMAGE_SIZE) {
                    angledelta = HALF_IMAGE_SIZE;
                } else if (angledelta < -HALF_IMAGE_SIZE) {
                    angledelta = -HALF_IMAGE_SIZE;
                }
                int angle = angle0;
                int i     = pos0;
                do {
                    int seg = find_segment(i), pos = i - segpos[seg];
                    i += delta;
                    if (seg >= numsegments) {
                        continue;
                    }

                    insertstack.emplace(seg, angle_normal(angle), pos, type);
                    angle      = static_cast<int8_t>(angle + angledelta);
                    angledelta = -angledelta;
                } while (i != pos1 + delta);
                break;
            }
            case eDiamond: {
                int const delta = dpos >= 0 ? 1 : -1;
                if (angledelta < 0) {
                    angledelta = -angledelta;
                }
                if (angledelta > HALF_IMAGE_SIZE) {
                    angledelta = HALF_IMAGE_SIZE;
                } else if (angledelta < QUARTER_IMAGE_SIZE) {
                    angledelta = QUARTER_IMAGE_SIZE;
                }
                int angle = angle0;
                int i     = pos0 + delta;
                int seg = find_segment(pos0), pos = pos0 - segpos[seg];
                if (seg < numsegments) {
                    insertstack.emplace(seg, angle_normal(angle), pos, type);
                }
                seg = find_segment(pos1);
                pos = pos1 - segpos[seg];
                if (seg < numsegments) {
                    insertstack.emplace(seg, angle_normal(angle), pos, type);
                }
                while (i != pos1) {
                    seg = find_segment(i);
                    pos = i - segpos[seg];
                    i += delta;
                    if (seg >= numsegments) {
                        continue;
                    }

                    int tx =
                        angle_normal(static_cast<int8_t>(angle - angledelta));
                    insertstack.emplace(seg, tx, pos, type);
                    tx = angle_normal(static_cast<int8_t>(angle + angledelta));
                    insertstack.emplace(seg, tx, pos, type);
                }
                break;
            }
            case eLozenge:
            case eStar: {
                bool      fill  = submode == eLozenge;
                int const delta = dpos >= 0 ? 1 : -1;
                if (angledelta < 0) {
                    angledelta = -angledelta;
                }
                if (angledelta > HALF_IMAGE_SIZE) {
                    angledelta = HALF_IMAGE_SIZE;
                } else if (angledelta < QUARTER_IMAGE_SIZE) {
                    angledelta = QUARTER_IMAGE_SIZE;
                }
                if (dpos >= 0) {
                    object_triangle(
                        angle0, pos0, angledelta, delta, (dpos + 1) / 2, type,
                        fill, insertstack);
                    object_triangle(
                        angle0, pos1, angledelta, -delta, -(dpos / 2), type,
                        fill, insertstack);
                } else {
                    object_triangle(
                        angle0, pos0, angledelta, delta, dpos / 2, type, fill,
                        insertstack);
                    object_triangle(
                        angle0, pos1, angledelta, -delta, (-dpos + 1) / 2, type,
                        fill, insertstack);
                }
                break;
            }
            case eTriangle: {
                int const delta = dpos >= 0 ? 1 : -1;
                if (angledelta < 0) {
                    angledelta = -angledelta;
                }
                if (angledelta > HALF_IMAGE_SIZE) {
                    angledelta = HALF_IMAGE_SIZE;
                } else if (angledelta < QUARTER_IMAGE_SIZE) {
                    angledelta = QUARTER_IMAGE_SIZE;
                }
                object_triangle(
                    angle0, pos1, angledelta, -delta, -dpos, type, true,
                    insertstack);
                break;
            }
            default:
                break;
            }
            break;
        }

        default:
            break;
        }
    }

    update();

    if (mode != eSelectMode || drawbox ||
        (event->state & GDK_BUTTON1_MASK) == 0 || !hotspot.valid() ||
        selection.find(hotspot) == selection.end()) {
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
