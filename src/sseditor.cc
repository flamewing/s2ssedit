/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * S2-SSEdit
 * Copyright (C) Flamewing 2011 <flamewing.sonic@gmail.com>
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
#include <cstdio>
#include <vector>
#include <fstream>
#include <gtkmm.h>
#include <gtkmm.h>
#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

#include "sseditor.h"

#include "ssobjfile.h"
#include "bigendian_io.h"

//#define DEBUG 1
#ifdef WIN32
#	define RINGFILE "./ring.png"
#	define BOMBFILE "./bomb.png"
#else
#	ifdef DEBUG
#		define RINGFILE "src/ring.png"
#		define BOMBFILE "src/bomb.png"
#	else
#		define RINGFILE PACKAGE_DATA_DIR"/s2ssedit/ui/ring.png"
#		define BOMBFILE PACKAGE_DATA_DIR"/s2ssedit/ui/bomb.png"
#	endif
#endif

#define IMAGE_SIZE 16
#define SEGMENT_SIZE 64
#define SEGMENT_PIXELS (IMAGE_SIZE * SEGMENT_SIZE)



sseditor *sseditor::instance = 0;

sseditor::sseditor(int argc, char *argv[], char const *uifile)
: update_in_progress(false), dragging(false), drop_enabled(false), can_drag(false),
	  specialstages(0), currstage(0), currsegment(0), mode(eSelectMode),
	  mouse_x(0), mouse_y(0),
	  selseg(-1), selx(0), sely(0), seltype(0),
	  hotseg(-1), hotx(0), hoty(0), hottype(0),
	  main_win(0)
{
	kit = new Gtk::Main(argc, argv);

	ringimg = Gdk::Pixbuf::create_from_file(RINGFILE);
	bombimg = Gdk::Pixbuf::create_from_file(BOMBFILE);
	
	//Load the Glade file and instiate its widgets:
	builder = Gtk::Builder::create_from_file(uifile);

	builder->get_widget("main_window", main_win);

	// Blessed be sed...
	builder->get_widget("specialstageobjs", pspecialstageobjs);
	builder->get_widget("hruler", phruler);
	pfilefilter = Glib::RefPtr<Gtk::FileFilter>::cast_dynamic(builder->get_object("filefilter"));
	// Labels
	builder->get_widget("labelcurrentstage", plabelcurrentstage);
	builder->get_widget("labeltotalstages", plabeltotalstages);
	builder->get_widget("labelcurrentsegment", plabelcurrentsegment);
	builder->get_widget("labeltotalsegments", plabeltotalsegments);
	// Scrollbar
	builder->get_widget("vscrollbar", pvscrollbar);
	// Main toolbar
	builder->get_widget("openfilebutton", popenfilebutton);
	builder->get_widget("savefilebutton", psavefilebutton);
	builder->get_widget("revertfilebutton", prevertfilebutton);
	builder->get_widget("selectmodebutton", pselectmodebutton);
	builder->get_widget("insertringbutton", pinsertringbutton);
	builder->get_widget("insertbombbutton", pinsertbombbutton);
	builder->get_widget("deletemodebutton", pdeletemodebutton);
	builder->get_widget("aboutbutton", paboutbutton);
	builder->get_widget("quitbutton", pquitbutton);
	// Special stage toolbar
	builder->get_widget("stage_toolbar", pstage_toolbar);
	builder->get_widget("first_stage_button", pfirst_stage_button);
	builder->get_widget("previous_stage_button", pprevious_stage_button);
	builder->get_widget("next_stage_button", pnext_stage_button);
	builder->get_widget("last_stage_button", plast_stage_button);
	builder->get_widget("insert_stage_before_button", pinsert_stage_before_button);
	builder->get_widget("append_stage_button", pappend_stage_button);
	builder->get_widget("delete_stage_button", pdelete_stage_button);
	builder->get_widget("swap_stage_prev_button", pswap_stage_prev_button);
	builder->get_widget("swap_stage_next_button", pswap_stage_next_button);
	// Segment toolbar
	builder->get_widget("segment_toolbar", psegment_toolbar);
	builder->get_widget("first_segment_button", pfirst_segment_button);
	builder->get_widget("previous_segment_button", pprevious_segment_button);
	builder->get_widget("next_segment_button", pnext_segment_button);
	builder->get_widget("last_segment_button", plast_segment_button);
	builder->get_widget("insert_segment_before_button", pinsert_segment_before_button);
	builder->get_widget("append_segment_button", pappend_segment_button);
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

	pfilefilter->add_pattern(SS_OBJECT_FILE);
	pfilefilter->add_pattern(SS_LAYOUT_FILE);


	pspecialstageobjs->signal_configure_event().connect(
		sigc::mem_fun(this, &sseditor::on_specialstageobjs_configure_event));
	pspecialstageobjs->signal_expose_event().connect(
		sigc::mem_fun(this, &sseditor::on_specialstageobjs_expose_event));
	pspecialstageobjs->signal_key_press_event().connect(
		sigc::mem_fun(this, &sseditor::on_specialstageobjs_key_press_event));
	pspecialstageobjs->signal_button_press_event().connect(
		sigc::mem_fun(this, &sseditor::on_specialstageobjs_button_press_event));
	pspecialstageobjs->signal_button_release_event().connect(
		sigc::mem_fun(this, &sseditor::on_specialstageobjs_button_release_event));
	pspecialstageobjs->signal_scroll_event().connect(
		sigc::mem_fun(this, &sseditor::on_specialstageobjs_scroll_event));
	pspecialstageobjs->signal_drag_begin().connect(
		sigc::mem_fun(this, &sseditor::on_specialstageobjs_drag_begin));
	pspecialstageobjs->signal_motion_notify_event().connect(
		sigc::mem_fun(this, &sseditor::on_specialstageobjs_motion_notify_event));
	pspecialstageobjs->signal_drag_data_get().connect(
		sigc::mem_fun(this, &sseditor::on_specialstageobjs_drag_data_get));
	pspecialstageobjs->signal_selection_clear_event().connect(
		sigc::mem_fun(this, &sseditor::on_specialstageobjs_selection_clear_event));
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
	pselectmodebutton->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_selectmodebutton_toggled));
	pinsertringbutton->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_insertringbutton_toggled));
	pinsertbombbutton->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_insertbombbutton_toggled));
	pdeletemodebutton->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_deletemodebutton_toggled));
	paboutbutton->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_aboutbutton_clicked));
	pquitbutton->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_quitbutton_clicked));
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
	pinsert_segment_before_button->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_insert_segment_before_button_clicked));
	pappend_segment_button->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_append_segment_button_clicked));
	pdelete_segment_button->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_delete_segment_button_clicked));
	pswap_segment_prev_button->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_swap_segment_prev_button_clicked));
	pswap_segment_next_button->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_swap_segment_next_button_clicked));
	// Segment flags
	pnormal_segment->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_normal_segment_toggled));
	pring_message->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_ring_message_toggled));
	pcheckpoint->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_checkpoint_toggled));
	pchaos_emerald->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_chaos_emerald_toggled));
	psegment_turnthenrise->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_segment_turnthenrise_toggled));
	psegment_turnthendrop->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_segment_turnthendrop_toggled));
	psegment_turnthenstraight->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_segment_turnthenstraight_toggled));
	psegment_straight->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_segment_straight_toggled));
	psegment_straightthenturn->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_segment_straightthenturn_toggled));
	psegment_right->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_segment_right_toggled));
	psegment_left->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_segment_left_toggled));
	// Object flags
	pmoveup->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_moveup_clicked));
	pmovedown->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_movedown_clicked));
	pmoveleft->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_moveleft_clicked));
	pmoveright->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_moveright_clicked));
	pringtype->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_ringtype_toggled));
	pbombtype->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_bombtype_toggled));

	update();
}

void sseditor::update()
{
	if (update_in_progress)
		return;

	update_in_progress = true;
	
	if (!specialstages)
	{
		psavefilebutton->set_sensitive(false);
		prevertfilebutton->set_sensitive(false);

		pselectmodebutton->set_sensitive(false);
		pinsertringbutton->set_sensitive(false);
		pinsertbombbutton->set_sensitive(false);
		pdeletemodebutton->set_sensitive(false);

		pstage_toolbar->set_sensitive(false);
		psegment_toolbar->set_sensitive(false);

		psegment_expander->set_sensitive(false);
		pobject_expander->set_sensitive(false);
		pvscrollbar->set_value(0.0);
		pvscrollbar->set_range(0.0, 1.0);
	}
	else
	{
		psavefilebutton->set_sensitive(true);
		prevertfilebutton->set_sensitive(true);

		pselectmodebutton->set_sensitive(true);
		pinsertringbutton->set_sensitive(true);
		pinsertbombbutton->set_sensitive(true);
		pdeletemodebutton->set_sensitive(true);

		pstage_toolbar->set_sensitive(true);

		size_t numstages = specialstages->num_stages();

		if (currstage == numstages - 1 || numstages <= currstage || numstages == 0)
		{
			currstage = numstages - 1;
			pnext_stage_button->set_sensitive(false);
			plast_stage_button->set_sensitive(false);
			pswap_stage_next_button->set_sensitive(false);
		}
		else
		{
			pnext_stage_button->set_sensitive(true);
			plast_stage_button->set_sensitive(true);
			pswap_stage_next_button->set_sensitive(true);
		}

		if (currstage == 0 || numstages == 0)
		{
			pfirst_stage_button->set_sensitive(false);
			pprevious_stage_button->set_sensitive(false);
			pswap_stage_prev_button->set_sensitive(false);
		}
		else
		{
			pfirst_stage_button->set_sensitive(true);
			pprevious_stage_button->set_sensitive(true);
			pswap_stage_prev_button->set_sensitive(true);
		}

		if (numstages == 0)
		{
			psegment_toolbar->set_sensitive(false);
			psegment_expander->set_sensitive(false);
			pobject_expander->set_sensitive(false);
			plabeltotalstages->set_label("0");
			plabelcurrentstage->set_label("0");
			plabeltotalsegments->set_label("0");
			plabelcurrentsegment->set_label("0");
			pvscrollbar->set_value(0.0);
			pvscrollbar->set_range(0.0, 1.0);
		}
		else
		{
			psegment_toolbar->set_sensitive(true);
			char buf[20];
			sprintf(buf, "%d", (int)numstages);
			plabeltotalstages->set_label(buf);

			sprintf(buf, "%d", currstage + 1);
			plabelcurrentstage->set_label(buf);

			sslevels *currlvl = specialstages->get_stage(currstage);
			size_t numsegments = currlvl->num_segments();

			if (currsegment == numsegments - 1 || numsegments <= currsegment || numsegments == 0)
			{
				currsegment = numsegments - 1;
				pnext_segment_button->set_sensitive(false);
				plast_segment_button->set_sensitive(false);
				pswap_segment_next_button->set_sensitive(false);
			}
			else
			{
				pnext_segment_button->set_sensitive(true);
				plast_segment_button->set_sensitive(true);
				pswap_segment_next_button->set_sensitive(true);
			}

			if (currsegment == 0 || numsegments == 0)
			{
				pfirst_segment_button->set_sensitive(false);
				pprevious_segment_button->set_sensitive(false);
				pswap_segment_prev_button->set_sensitive(false);
			}
			else
			{
				pfirst_segment_button->set_sensitive(true);
				pprevious_segment_button->set_sensitive(true);
				pswap_segment_prev_button->set_sensitive(true);
			}

			if (numsegments == 0)
			{
				psegment_expander->set_sensitive(false);
				pobject_expander->set_sensitive(false);
				plabeltotalsegments->set_label("0");
				plabelcurrentsegment->set_label("0");
				pvscrollbar->set_value(0.0);
				pvscrollbar->set_range(0.0, 1.0);
			}
			else
			{
				pvscrollbar->set_range(0.0, SEGMENT_PIXELS * numsegments);
				pvscrollbar->set_increments(IMAGE_SIZE * 4, IMAGE_SIZE * 2 * numsegments);

				psegment_expander->set_sensitive(true);
				sprintf(buf, "%d", (int)numsegments);
				plabeltotalsegments->set_label(buf);

				sprintf(buf, "%d", currsegment + 1);
				plabelcurrentsegment->set_label(buf);

				sssegments *currseg = currlvl->get_segment(currsegment);
				switch (currseg->get_type())
				{
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
				switch (currseg->get_geometry())
				{
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
				if (currseg->get_direction())
					psegment_left->set_active(true);
				else
					psegment_right->set_active(true);

				if (selseg < 0)
				{
					pringtype->set_active(true);
					pobject_expander->set_sensitive(false);
				}
				else
				{
					pobject_expander->set_sensitive(true);
					if (seltype == 0x40)
					{
						if (!pbombtype->get_active())
							pbombtype->set_active(true);
					}
					else
					{
						if (!pringtype->get_active())
							pringtype->set_active(true);
					}
				}
			}
		}
	}

	Glib::RefPtr<Gdk::Window> window = pspecialstageobjs->get_window();
	if (window)
	{
		Gdk::Rectangle r(0, 0, draw_width, draw_height);
		window->invalidate_rect(r, false);
	}

	update_in_progress = false;
}

void sseditor::run()
{
	if (main_win)
		kit->run(*main_win);
}

void sseditor::on_vscrollbar_value_changed()
{
	currsegment = ((int)pvscrollbar->get_value()) / SEGMENT_PIXELS;
	update();
}

void sseditor::on_filedialog_response(int response_id)
{
	switch (response_id)
	{
		case Gtk::RESPONSE_OK:
		{
			std::string dirname = filedlg->get_filename() + '/';
			std::string layoutfile = dirname + SS_LAYOUT_FILE,
					    objectfile = dirname + SS_OBJECT_FILE;
			std::ifstream fobj(objectfile.c_str(), std::ios::in|std::ios::binary),
					      flay(layoutfile.c_str(), std::ios::in|std::ios::binary);
			if (!fobj.good() || !flay.good())
				return;

			delete specialstages;
			specialstages = new ssobj_file(dirname);
			update();
			break;
		}
		default:
			break;
	}
	filedlg->hide();
}

void sseditor::on_openfilebutton_clicked()
{
	if (!filedlg)
	{
		builder->get_widget("filechooserdialog", filedlg);
		filedlg->signal_response().connect(
			sigc::mem_fun(this, &sseditor::on_filedialog_response));
	}

	if (filedlg)
		filedlg->run();
}

void sseditor::on_savefilebutton_clicked()
{
	specialstages->write();
	update();
}

void sseditor::on_revertfilebutton_clicked()
{
	specialstages->read();
	update();
}

void sseditor::on_selectmodebutton_toggled()
{
	mode = eSelectMode;
	selseg = -1;
	selx = 0;
	sely = 0;
	seltype = 0;
	update();
}

void sseditor::on_insertringbutton_toggled()
{
	mode = eInsertRingMode;
	selseg = -1;
	selx = 0;
	sely = 0;
	seltype = 0;
	update();
}

void sseditor::on_insertbombbutton_toggled()
{
	mode = eInsertBombMode;
	selseg = -1;
	selx = 0;
	sely = 0;
	seltype = 0;
	update();
}

void sseditor::on_deletemodebutton_toggled()
{
	mode = eDeleteMode;
	selseg = -1;
	selx = 0;
	sely = 0;
	seltype = 0;
	update();
}

void sseditor::on_aboutdialog_response(int response_id)
{
	aboutdlg->hide();
}

void sseditor::on_aboutbutton_clicked()
{
	if (!aboutdlg)
	{
		builder->get_widget("aboutdialog", aboutdlg);
		aboutdlg->signal_response().connect(
			sigc::mem_fun(this, &sseditor::on_aboutdialog_response));
	}
	if (aboutdlg)
		aboutdlg->run();
}

void sseditor::on_quitbutton_clicked()
{
	kit->quit();
}

void sseditor::on_first_stage_button_clicked()
{
	currstage = 0;
	update();
}

void sseditor::on_previous_stage_button_clicked()
{
	if (currstage > 0)
	{
		currstage--;
		update();
	}
}

void sseditor::on_next_stage_button_clicked()
{
	if (currstage + 1 < specialstages->num_stages())
	{
		currstage++;
		update();
	}
}

void sseditor::on_last_stage_button_clicked()
{
	currstage = specialstages->num_stages() - 1;
	update();
}

void sseditor::on_insert_stage_before_button_clicked()
{
	specialstages->insert(sslevels(), currstage);
	update();
}

void sseditor::on_append_stage_button_clicked()
{
	specialstages->append(sslevels());
	update();
}

void sseditor::on_delete_stage_button_clicked()
{
	specialstages->remove(currstage);
	update();
}

void sseditor::on_swap_stage_prev_button_clicked()
{
	specialstages->move_left(currstage);
	currstage--;
	update();
}

void sseditor::on_swap_stage_next_button_clicked()
{
	specialstages->move_right(currstage);
	currstage++;
	update();
}

void sseditor::on_first_segment_button_clicked()
{
	currsegment = 0;
	pvscrollbar->set_value(SEGMENT_PIXELS * currsegment);
	update();
}

void sseditor::on_previous_segment_button_clicked()
{
	if (currsegment > 0)
	{
		currsegment--;
		pvscrollbar->set_value(SEGMENT_PIXELS * currsegment);
		update();
	}
}

void sseditor::on_next_segment_button_clicked()
{
	if (currsegment + 1 < specialstages->get_stage(currstage)->num_segments())
	{
		currsegment++;
		pvscrollbar->set_value(SEGMENT_PIXELS * currsegment);
		update();
	}
}

void sseditor::on_last_segment_button_clicked()
{
	currsegment = specialstages->get_stage(currstage)->num_segments() - 1;
	pvscrollbar->set_value(SEGMENT_PIXELS * currsegment);
	update();
}

void sseditor::on_insert_segment_before_button_clicked()
{
	specialstages->get_stage(currstage)->insert(sssegments(), currsegment);
	pvscrollbar->set_value(SEGMENT_PIXELS * currsegment);
	update();
}

void sseditor::on_append_segment_button_clicked()
{
	specialstages->get_stage(currstage)->append(sssegments());
	pvscrollbar->set_value(SEGMENT_PIXELS * currsegment);
	update();
}

void sseditor::on_delete_segment_button_clicked()
{
	specialstages->get_stage(currstage)->remove(currsegment);
	pvscrollbar->set_value(SEGMENT_PIXELS * currsegment);
	update();
}

void sseditor::on_swap_segment_prev_button_clicked()
{
	specialstages->get_stage(currstage)->move_left(currsegment);
	currsegment--;
	pvscrollbar->set_value(SEGMENT_PIXELS * currsegment);
	update();
}

void sseditor::on_swap_segment_next_button_clicked()
{
	specialstages->get_stage(currstage)->move_right(currsegment);
	currsegment++;
	pvscrollbar->set_value(SEGMENT_PIXELS * currsegment);
	update();
}

void sseditor::on_normal_segment_toggled()
{
	if (!specialstages)
		return;
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	sssegments *currseg = currlvl->get_segment(currsegment);
	currseg->set_type(sssegments::eNormalSegment);
}

void sseditor::on_ring_message_toggled()
{
	if (!specialstages)
		return;
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	sssegments *currseg = currlvl->get_segment(currsegment);
	currseg->set_type(sssegments::eRingsMessage);
}

void sseditor::on_checkpoint_toggled()
{
	if (!specialstages)
		return;
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	sssegments *currseg = currlvl->get_segment(currsegment);
	currseg->set_type(sssegments::eCheckpoint);
}

void sseditor::on_chaos_emerald_toggled()
{
	if (!specialstages)
		return;
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	sssegments *currseg = currlvl->get_segment(currsegment);
	currseg->set_type(sssegments::eChaosEmerald);
}

void sseditor::on_segment_turnthenrise_toggled()
{
	if (!specialstages)
		return;
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	sssegments *currseg = currlvl->get_segment(currsegment);
	currseg->set_geometry(sssegments::eTurnThenRise);
}

void sseditor::on_segment_turnthendrop_toggled()
{
	if (!specialstages)
		return;
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	sssegments *currseg = currlvl->get_segment(currsegment);
	currseg->set_geometry(sssegments::eTurnThenDrop);
}

void sseditor::on_segment_turnthenstraight_toggled()
{
	if (!specialstages)
		return;
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	sssegments *currseg = currlvl->get_segment(currsegment);
	currseg->set_geometry(sssegments::eTurnThenStraight);
}

void sseditor::on_segment_straight_toggled()
{
	if (!specialstages)
		return;
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	sssegments *currseg = currlvl->get_segment(currsegment);
	currseg->set_geometry(sssegments::eStraight);
}

void sseditor::on_segment_straightthenturn_toggled()
{
	if (!specialstages)
		return;
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	sssegments *currseg = currlvl->get_segment(currsegment);
	currseg->set_geometry(sssegments::eStraightThenTurn);
}

void sseditor::on_segment_right_toggled()
{
	if (!specialstages)
		return;
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	sssegments *currseg = currlvl->get_segment(currsegment);
	currseg->set_direction(false);
}

void sseditor::on_segment_left_toggled()
{
	if (!specialstages)
		return;
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	sssegments *currseg = currlvl->get_segment(currsegment);
	currseg->set_direction(true);
}

bool sseditor::move_object(int dx, int dy)
{
	if (!specialstages)
		return true;

	if (selseg != -1)
	{
		sslevels *currlvl = specialstages->get_stage(currstage);
		size_t numsegments = currlvl->num_segments();

		if (selseg >= numsegments)
		{
			selseg = -1;
			selx = sely = seltype = 0;
			update();
			return true;
		}

		sssegments *currseg = currlvl->get_segment(selseg);
		size_t type;
		if (!currseg->exists(sely, selx, type))
		{
			selseg = -1;
			selx = sely = seltype = 0;
			update();
			return true;
		}

		int newx = (selx + dx) & 0xff, newy = (sely + dy) & 0x3f;

		int cnt = 0x40;
		while (cnt > 0 && currseg->exists(newy, newx, type))
		{
			newx += dx;
			newx &= 0xff;
			newy += dy;
			newy &= 0x3f;
			cnt--;
		}

		currseg->remove(sely, selx);
		currseg->update(newy, newx, seltype, true);

		int pos = (newy * IMAGE_SIZE), loc = pvscrollbar->get_value();
		if (pos < loc || pos >= loc + draw_height)
			pvscrollbar->set_value(pos);

		selx = newx;
		sely = newy;
		update();
		return true;
	}
	return true;
}

void sseditor::on_moveup_clicked()
{
	move_object(0, -1);
}

void sseditor::on_movedown_clicked()
{
	move_object(0, 1);
}

void sseditor::on_moveleft_clicked()
{
	move_object(-1, 0);
}

void sseditor::on_moveright_clicked()
{
	move_object(1, 0);
}

void sseditor::on_ringtype_toggled()
{
	if (!specialstages || selseg == -1)
		return;

	sslevels *currlvl = specialstages->get_stage(currstage);
	sssegments *currseg = currlvl->get_segment(selseg);
	currseg->update(sely, selx, seltype = 0x00, false);
	update();
}

void sseditor::on_bombtype_toggled()
{
	if (!specialstages || selseg == -1)
		return;

	sslevels *currlvl = specialstages->get_stage(currstage);
	sssegments *currseg = currlvl->get_segment(selseg);
	currseg->update(sely, selx, seltype = 0x40, false);
	update();
}

bool sseditor::on_specialstageobjs_configure_event(GdkEventConfigure *event)
{
	draw_width = event->width;
	draw_height = event->height;
	if (!drop_enabled)
	{
		drop_enabled = true;
		std::vector<Gtk::TargetEntry> vec;
		vec.push_back(Gtk::TargetEntry("SpecialStageObjects", Gtk::TargetFlags(0), 1));
		pspecialstageobjs->drag_dest_set(vec, Gtk::DEST_DEFAULT_ALL,
		                                 Gdk::ACTION_COPY|Gdk::ACTION_MOVE);
		pspecialstageobjs->signal_drag_data_received().connect(
			sigc::mem_fun(this, &sseditor::on_specialstageobjs_drag_data_received));
	}
	update();
	return true;
}

void sseditor::on_specialstageobjs_drag_data_received(Glib::RefPtr<Gdk::DragContext> const& context,
                                                      int x, int y,
                                                      Gtk::SelectionData const& selection_data,
                                                      guint info, guint time)
{
	if (selection_data.get_data_type() == "SpecialStageObjects" &&
	    selection_data.get_format() == 32 && selection_data.get_length() > 0)
	{
		char const *data = (char const *)selection_data.get_data();
		char const *ptr = data;
		size_t oldseg = BigEndian::Read4(ptr),
		       oldx = BigEndian::Read4(ptr),
		       oldy = BigEndian::Read4(ptr),
		       oldtype = BigEndian::Read4(ptr);

		size_t angle, pos, seg;
		angle = x - 4 + IMAGE_SIZE / 2;
		angle /= 2;
		angle = (angle + 0xc0) & 0xff;

		pos = y + pvscrollbar->get_value();
		pos /= IMAGE_SIZE;
		seg = pos / SEGMENT_SIZE;
		pos %= SEGMENT_SIZE;
	
		sslevels *currlvl = specialstages->get_stage(currstage);
		size_t numsegments = currlvl->num_segments();

		if (seg >= numsegments)
			return;

		sssegments *currseg = currlvl->get_segment(seg),
		           *srcseg = currlvl->get_segment(oldseg);
		srcseg->remove(oldy, oldx);
		currseg->update(pos, angle, oldtype, true);
		selseg = seg;
		selx = angle;
		sely = pos;
		seltype = oldtype;
		update();
	}
}

bool sseditor::on_specialstageobjs_expose_event(GdkEventExpose *event)
{
	Glib::RefPtr<Gdk::Window> window = pspecialstageobjs->get_window();

	if (!window)
		return true;
	
	Glib::RefPtr<Gdk::GC> gc = pspecialstageobjs->get_style()->get_black_gc();
	Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
    cr->set_source_rgba(0.337, 0.612, 0.117, 0.9);   // green
    cr->paint();
	
	if (!specialstages)
		return true;

	int start = pvscrollbar->get_value(), end = start + draw_height;
	start /= IMAGE_SIZE;
	end = (end + IMAGE_SIZE - 1) / IMAGE_SIZE;

	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	int last_seg = -1;
	hotseg = -1;
	hotx = hoty = hottype = 0;

	for (int i = start; i <= end; i++)
	{
		size_t seg = i / SEGMENT_SIZE;
		if (seg >= numsegments)
			break;

		if (last_seg != seg)
		{
			last_seg = seg;
			cr->set_line_width(4.0);
			cr->set_source_rgb(1.0, 1.0, 1.0);
			int ty = SEGMENT_PIXELS * seg - pvscrollbar->get_value();
			cr->move_to(0, ty);
			cr->line_to(draw_width, ty);
			cr->stroke();
		}
		
		sssegments *currseg = currlvl->get_segment(seg);
		std::map<size_t,size_t> const& row = currseg->get_row(i % SEGMENT_SIZE);
		for (std::map<size_t,size_t>::const_iterator it = row.begin();
		     it != row.end(); ++it)
		{
			size_t pos = (i * IMAGE_SIZE) - pvscrollbar->get_value(),
			       angle = ((it->first + 0x40) & 0xff) * 2 + 4;

			Glib::RefPtr<Gdk::Pixbuf> image = (it->second == 0x40) ? bombimg : ringimg;

			int tx = angle - image->get_width()/2,
			    ty = pos;

			bool drawx = false;
			if (mode == eSelectMode)
			{
				bool drawbox = false;
				cr->set_line_width(2.0);
				if (hotseg == -1 &&
				    mouse_x >= tx && mouse_y >= ty &&
				    mouse_x <= tx + image->get_width() &&
				    mouse_y <= ty + image->get_height())
				{
					hotseg = seg;
					hotx = it->first;
					hoty = (i % SEGMENT_SIZE);
					hottype = it->second;
					cr->set_source_rgb(1.0, 1.0, 0.0);
					drawbox = true;
				}
				if (selseg == seg && selx == it->first && sely == (i % SEGMENT_SIZE))
				{
					cr->set_source_rgb(0.0, 0.0, 0.0);
					drawbox = true;
				}
				if (drawbox)
				{
					cr->move_to(tx, ty);
					cr->line_to(tx + image->get_width(), ty);
					cr->line_to(tx + image->get_width(), ty + image->get_height());
					cr->line_to(tx, ty + image->get_height());
					cr->line_to(tx, ty);
					cr->stroke();
				}
			}
			else if (hotseg == -1 && mode == eDeleteMode &&
			         mouse_x >= tx && mouse_y >= ty &&
			         mouse_x <= tx + image->get_width() &&
			         mouse_y <= ty + image->get_height())
			{
				hotseg = seg;
				hotx = it->first;
				hoty = (i % SEGMENT_SIZE);
				hottype = it->second;
				cr->set_line_width(2.0);
				cr->set_source_rgb(1.0, 0.0, 0.0);
				cr->move_to(tx, ty);
				cr->line_to(tx + image->get_width(), ty);
				cr->line_to(tx + image->get_width(), ty + image->get_height());
				cr->line_to(tx, ty + image->get_height());
				cr->line_to(tx, ty);
				cr->stroke();
				drawx = true;
			}

			image->render_to_drawable(window, gc, 0, 0, tx, ty,
			                            image->get_width(),
			                            image->get_height(),
			                            Gdk::RGB_DITHER_NONE, 0, 0);

			if (drawx)
			{
				cr->set_line_width(2.0);
				cr->set_source_rgb(1.0, 0.0, 0.0);
				cr->move_to(tx, ty);
				cr->line_to(tx + image->get_width(), ty + image->get_height());
				cr->move_to(tx + image->get_width(), ty);
				cr->line_to(tx, ty + image->get_height());
				cr->stroke();
			}
		}
	}

	if (mode == eInsertRingMode || mode == eInsertBombMode)
	{
		Glib::RefPtr<Gdk::Pixbuf> image = (mode == eInsertBombMode) ? bombimg : ringimg;
		int tx = mouse_x + 4 - image->get_width()/2,
			ty = mouse_y + pvscrollbar->get_value();
		ty = ty - (ty % IMAGE_SIZE);
		ty = ty - pvscrollbar->get_value();
		cr->set_line_width(1.0);
		cr->set_source_rgb(1.0, 1.0, 0.0);
		cr->move_to(tx, ty);
		cr->line_to(tx + image->get_width(), ty);
		cr->line_to(tx + image->get_width(), ty + image->get_height());
		cr->line_to(tx, ty + image->get_height());
		cr->line_to(tx, ty);
		cr->stroke();
		image->render_to_drawable(window, gc, 0, 0, tx, ty,
		                          image->get_width(), image->get_height(),
		                          Gdk::RGB_DITHER_NONE, 0, 0);
	}

	return true;
}

bool sseditor::on_specialstageobjs_key_press_event(GdkEventKey *event)
{
	if (!specialstages)
		return true;
	
	switch (event->keyval)
	{
		case GDK_KEY_Up:
		case GDK_KEY_KP_Up:
			return move_object(0, -1);
		
		case GDK_KEY_Down:
		case GDK_KEY_KP_Down:
			return move_object(0, 1);

		case GDK_KEY_Left:
		case GDK_KEY_KP_Left:
		{
			bool ctrl = (event->state & GDK_CONTROL_MASK) != 0;
			return move_object(ctrl ? -4 : -1, 0);
		}

		case GDK_KEY_Right:
		case GDK_KEY_KP_Right:
		{
			bool ctrl = (event->state & GDK_CONTROL_MASK) != 0;
			return move_object(ctrl ? 4 : 1, 0);
		}

		case GDK_KEY_Delete:
		case GDK_KEY_KP_Delete:
			if (selseg != -1)
			{
				sslevels *currlvl = specialstages->get_stage(currstage);
				size_t numsegments = currlvl->num_segments();

				if (selseg >= numsegments)
					return true;

				sssegments *currseg = currlvl->get_segment(selseg);
				currseg->remove(sely, selx);
				selseg = -1;
				selx = sely = seltype = 0;
				update();
			}

		case GDK_KEY_Page_Up:
		case GDK_KEY_KP_Page_Up:
		{
			sslevels *currlvl = specialstages->get_stage(currstage);
			size_t numsegments = currlvl->num_segments();
			bool ctrl = (event->state & GDK_CONTROL_MASK) != 0;
			double delta = ctrl ? IMAGE_SIZE * 2 * numsegments : IMAGE_SIZE * 4;
			pvscrollbar->set_value(pvscrollbar->get_value() - delta);
			break;
		}

		case GDK_KEY_Page_Down:
		case GDK_KEY_KP_Page_Down:
		{
			sslevels *currlvl = specialstages->get_stage(currstage);
			size_t numsegments = currlvl->num_segments();
			bool ctrl = (event->state & GDK_CONTROL_MASK) != 0;
			double delta = ctrl ? IMAGE_SIZE * 2 * numsegments : IMAGE_SIZE * 4;
			pvscrollbar->set_value(pvscrollbar->get_value() + delta);
			break;
		}

		case GDK_KEY_Home:
		case GDK_KEY_KP_Home:
			pvscrollbar->set_value(0);
			break;
			
		case GDK_KEY_End:
		case GDK_KEY_KP_End:
		{
			sslevels *currlvl = specialstages->get_stage(currstage);
			size_t numsegments = currlvl->num_segments();
			pvscrollbar->set_value(SEGMENT_PIXELS * numsegments);
		}
			
	}
	return true;
}

bool sseditor::on_specialstageobjs_button_press_event(GdkEventButton *event)
{
	if (!specialstages)
		return true;

	if (event->button != 1)
		return true;

	if (mode == eSelectMode)
	{
		selseg = hotseg;
		selx = hotx;
		sely = hoty;
		seltype = hottype;
		can_drag = true;
		update();
		return true;
	}

	can_drag = false;
	selseg = -1;
	selx = sely = seltype = 0;
	update();
	return true;
}

bool sseditor::on_specialstageobjs_button_release_event(GdkEventButton *event)
{
	if (!specialstages)
		return true;

	if (event->button == 1 && dragging)
		dragging = false;

	if (event->button == 1)
		can_drag = false;
	
	size_t angle, pos, seg;
	if (hotseg == -1)
	{
		angle = event->x - 4 + IMAGE_SIZE / 2;
		angle /= 2;
		angle = (angle + 0xc0) & 0xff;

		pos = event->y + pvscrollbar->get_value();
		pos /= IMAGE_SIZE;
		seg = pos / SEGMENT_SIZE;
		pos %= SEGMENT_SIZE;
	}
	else
	{
		angle = hotx;
		pos = hoty;
		seg = hotseg;
	}
	
	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();

	if (seg >= numsegments)
		return true;

	sssegments *currseg = currlvl->get_segment(seg);
	
	switch (mode)
	{
		case eSelectMode:
			if (event->button == 3)
			{
				size_t type;
				if (currseg->exists(pos, angle, type))
				{
					if (type == 0x40)
					{
						currseg->remove(pos, angle);
						if (selseg == seg && sely == pos && selx == angle)
						{
							selseg = -1;
							selx = sely = seltype = 0;
						}
					}
					else
					{
						currseg->update(pos, angle, 0x40, false);
						selseg = seg;
						selx = angle;
						sely = pos;
						seltype = 0x40;
					}
				}
				else
				{
					currseg->update(pos, angle, 0x00, true);
					selseg = seg;
					selx = angle;
					sely = pos;
					seltype = 0x00;
				}
			}
			break;

		case eDeleteMode:
			if (event->button != 1)
				break;
			currseg->remove(pos, angle);
			break;

		case eInsertBombMode:
			if (event->button == 1)
				currseg->update(pos, angle, 0x40, true);
			else if (event->button == 3)
				currseg->remove(pos, angle);
			break;
			
		case eInsertRingMode:
			if (event->button == 1)
				currseg->update(pos, angle, 0x00, true);
			else if (event->button == 3)
				currseg->remove(pos, angle);
			break;
	}
	update();
	return true;
}

bool sseditor::on_specialstageobjs_scroll_event(GdkEventScroll *event)
{
	if (!specialstages)
		return false;

	sslevels *currlvl = specialstages->get_stage(currstage);
	size_t numsegments = currlvl->num_segments();
	bool ctrl = (event->state & GDK_CONTROL_MASK) != 0;
	double delta = ctrl ? IMAGE_SIZE * 2 * numsegments : IMAGE_SIZE * 4;

	switch (event->direction)
	{
		case GDK_SCROLL_UP:
			pvscrollbar->set_value(pvscrollbar->get_value() - delta);
			break;
			
		case GDK_SCROLL_DOWN:
			pvscrollbar->set_value(pvscrollbar->get_value() + delta);
			break;

		case GDK_SCROLL_LEFT:
			switch (mode)
			{
				case eSelectMode:
					pdeletemodebutton->set_active(true);
					break;

				case eInsertRingMode:
					pselectmodebutton->set_active(true);
					break;

				case eInsertBombMode:
					pinsertringbutton->set_active(true);
					break;

				case eDeleteMode:
					pinsertbombbutton->set_active(true);
					break;
			}
			break;

		case GDK_SCROLL_RIGHT:
			switch (mode)
			{
				case eSelectMode:
					pinsertringbutton->set_active(true);
					break;

				case eInsertRingMode:
					pinsertbombbutton->set_active(true);
					break;

				case eInsertBombMode:
					pdeletemodebutton->set_active(true);
					break;

				case eDeleteMode:
					pselectmodebutton->set_active(true);
					break;
			}
			break;
	}

	update();
	return true;
}

void sseditor::on_specialstageobjs_drag_begin(Glib::RefPtr<Gdk::DragContext> const& targets)
{
	if (selseg == -1)
		return;

	Glib::RefPtr<Gdk::Pixbuf> image = seltype == 0x40 ? bombimg : ringimg;
	targets->set_icon(image, 0, 0);
}

bool sseditor::on_specialstageobjs_motion_notify_event(GdkEventMotion *event)
{
	double min = phruler->get_range_lower(), max = phruler->get_range_upper();
	double scale = (max - min)/(double)draw_width;
	phruler->property_position() = event->x * scale + phruler->get_range_lower();
	phruler->draw_pos();

	mouse_x = event->x;
	mouse_y = event->y;
	update();

	if (dragging || selseg == -1 || !can_drag)
		return true;
	dragging = true;
	std::vector<Gtk::TargetEntry> vec;
	vec.push_back(Gtk::TargetEntry("SpecialStageObjects", Gtk::TargetFlags(0), 1));
	Glib::RefPtr<Gtk::TargetList> lst = Gtk::TargetList::create(vec);
	pspecialstageobjs->drag_begin(lst, Gdk::ACTION_COPY|Gdk::ACTION_MOVE,
	                              1, (GdkEvent *)event);
	return true;
}

void sseditor::on_specialstageobjs_drag_data_get
	(
	 Glib::RefPtr<Gdk::DragContext> const& targets,
	 Gtk::SelectionData& selection_data,
	 guint info, guint time
	)
{
	if (selseg == -1)
		return;
	char buf[30];
	char *ptr = buf;
	BigEndian::Write4(ptr, size_t(selseg));
	BigEndian::Write4(ptr, size_t(selx));
	BigEndian::Write4(ptr, size_t(sely));
	BigEndian::Write4(ptr, size_t(seltype));
	selection_data.set("SpecialStageObjects", 32, (guint8 *)buf, ptr - buf);
}

bool sseditor::on_specialstageobjs_selection_clear_event(GdkEventSelection *event)
{
	return true;
}

