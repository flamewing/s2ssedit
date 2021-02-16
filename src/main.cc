/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Copyright (C) Flamewing 2011-2019 <flamewing.sonic@gmail.com>
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

#include <array>
#include <fstream>
#include <iostream>

#include <s2ssedit/sseditor.hh>

/* For testing propose use the local (not installed) ui file */
//#define DEBUG 1
#ifdef WIN32
#    define UI_FILE "./s2ssedit.ui"
#else
#    ifdef DEBUG
#        define UI_FILE "src/s2ssedit.ui"
#    else
#        define UI_FILE PACKAGE_DATA_DIR "/s2ssedit/ui/s2ssedit.ui"
#    endif
#endif

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

using std::array;
using std::cerr;
using std::endl;
using std::make_shared;
using std::tie;
using std::to_string;

int main(int argc, char* argv[]) {
    try {
        sseditor Editor(make_shared<Gtk::Main>(argc, argv), UI_FILE);
        Editor.run();
    } catch (const Glib::FileError& ex) {
        cerr << ex.what() << endl;
        return 1;
    }
    return 0;
}

sseditor::sseditor(std::shared_ptr<Gtk::Main>&& application, char const* uifile)
    : update_in_progress(false), dragging(false), drop_enabled(false),
      currstage(0), currsegment(0), draw_width(0), draw_height(0), mouse_x(0),
      mouse_y(0), state(0), mode(eSelectMode), ringmode(eSingle),
      bombmode(eSingle), copypos(0), drawbox(false), snaptogrid(true),
      endpos(0), main_win(nullptr), kit(std::move(application)),
      helpdlg(nullptr), aboutdlg(nullptr), filedlg(nullptr),
      pspecialstageobjs(nullptr), pmodenotebook(nullptr),
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

template <size_t N>
void update_array(array<Gtk::RadioToolButton*, N>& buttons, bool state) {
    for (auto& elem : buttons) {
        elem->set_sensitive(state);
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

        update_array(pmodebuttons, false);

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

        update_array(pmodebuttons, true);

        pmodenotebook->set_sensitive(true);
        pstage_toolbar->set_sensitive(true);

        unsigned numstages = specialstages->num_stages();
        fix_stage(numstages);

        bool have_next_stage = numstages > 0 && currstage < numstages - 1;
        pnext_stage_button->set_sensitive(have_next_stage);
        plast_stage_button->set_sensitive(have_next_stage);
        pswap_stage_next_button->set_sensitive(have_next_stage);

        bool have_prev_stage = currstage > 0;
        pfirst_stage_button->set_sensitive(have_prev_stage);
        pprevious_stage_button->set_sensitive(have_prev_stage);
        pswap_stage_prev_button->set_sensitive(have_prev_stage);

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
            fix_segment(numsegments);

            bool have_next_segment =
                numsegments > 0 && currsegment < numsegments - 1;
            pnext_segment_button->set_sensitive(have_next_segment);
            plast_segment_button->set_sensitive(have_next_segment);
            pswap_segment_next_button->set_sensitive(have_next_segment);

            bool have_prev_segment = currsegment > 0;
            pfirst_segment_button->set_sensitive(have_prev_segment);
            pprevious_segment_button->set_sensitive(have_prev_segment);
            pswap_segment_prev_button->set_sensitive(have_prev_segment);

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

                segment_type_button(currseg->get_type())->set_active(true);
                geometry_button(currseg->get_geometry())->set_active(true);
                direction_button(currseg->get_direction())->set_active(true);

                size_t nrings;
                size_t nbombs;
                tie(nrings, nbombs) = count_objects(selection);
                bool empty          = nrings == 0 && nbombs == 0;
                bool inconsistent   = empty || (nrings > 0 && nbombs > 0);
                pobject_expander->set_sensitive(!empty);
                if (!inconsistent) {
                    if (nbombs > 0) {
                        pbombtype->set_active(true);
                    } else if (nrings > 0) {
                        pringtype->set_active(true);
                    }
                }
                pringtype->set_inconsistent(inconsistent);
                pbombtype->set_inconsistent(inconsistent);
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
