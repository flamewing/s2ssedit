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
#include <gtkmm.h>
#include <gtkmm.h>
#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

#include "sseditor.h"

#include <string>
#include <sstream>
#include <fstream>
#include <mdcomp/kosinski.h>
#include <mdcomp/nemesis.h>
#include "ssobjfile.h"
//#define DEBUG_DECODER 1
//#define DEBUG_ENCODER 1

#define SS_OBJECT_FILE "Special stage object location lists (Kosinski compression).bin"
#define SS_LAYOUT_FILE "Special stage level layouts (Nemesis compression).bin"

sseditor *sseditor::instance = 0;

sseditor::sseditor(int argc, char *argv[], char const *uifile)
	: main_win(0)
{
	// TODO: Add implementation here
	kit = new Gtk::Main(argc, argv);
	
	//Load the Glade file and instiate its widgets:
	builder = Gtk::Builder::create_from_file(uifile);

	builder->get_widget("main_window", main_win);

	// Blessed be sed...
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
	builder->get_widget("insertmodebutton", pinsertmodebutton);
	builder->get_widget("deletemodebutton", pdeletemodebutton);
	builder->get_widget("aboutbutton", paboutbutton);
	builder->get_widget("quitbutton", pquitbutton);
	// Special stage toolbar
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
	builder->get_widget("normal_segment", pnormal_segment);
	builder->get_widget("ring_message", pring_message);
	builder->get_widget("checkpoint", pcheckpoint);
	builder->get_widget("chaos_emerald", pchaos_emerald);
	builder->get_widget("segment_turnthenrise", psegment_turnthenrise);
	builder->get_widget("segment_turnthendrop", psegment_turnthendrop);
	builder->get_widget("segment_turnthenstraight", psegment_turnthenstraight);
	builder->get_widget("segment_straight", psegment_straight);
	builder->get_widget("segment_straightthenturn", psegment_straightthenturn);

	pfilefilter->add_pattern(SS_OBJECT_FILE);
	pfilefilter->add_pattern(SS_LAYOUT_FILE);

	// Scrollbar
	pvscrollbar->signal_value_changed().connect(
		sigc::mem_fun(this, &sseditor::on_vscrollbar1_value_changed));
	// Main toolbar
	popenfilebutton->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_openfilebutton_clicked));
	psavefilebutton->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_savefilebutton_clicked));
	prevertfilebutton->signal_clicked().connect(
		sigc::mem_fun(this, &sseditor::on_revertfilebutton_clicked));
	pselectmodebutton->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_selectmodebutton_toggled));
	pinsertmodebutton->signal_toggled().connect(
		sigc::mem_fun(this, &sseditor::on_insertmodebutton_toggled));
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
}

void sseditor::run()
{
	if (main_win)
	{
		kit->run(*main_win);
	}
}

void sseditor::on_vscrollbar1_value_changed()
{

}

void sseditor::on_filedialog_response(int response_id)
{
	switch (response_id)
	{
		case Gtk::RESPONSE_OK:
		{
			std::string dirname = filedlg->get_filename();
			dirname += '/';
			std::string layoutfile = dirname + SS_LAYOUT_FILE,
			            objectfile = dirname + SS_OBJECT_FILE;
			std::ifstream fobj(objectfile.c_str(), std::ios::in|std::ios::binary),
			              flay(layoutfile.c_str(), std::ios::in|std::ios::binary);
			if (!fobj.good() || !flay.good())
				return;
#ifdef DEBUG_DECODER
			std::fstream objfile((objectfile + ".dec").c_str(),
			                     std::ios::in|std::ios::out|std::ios::binary|std::ios::trunc),
			             layfile((layoutfile + ".dec").c_str(),
			                     std::ios::in|std::ios::out|std::ios::binary|std::ios::trunc);			
#else
			std::stringstream objfile(std::ios::in|std::ios::out|std::ios::binary),
			                  layfile(std::ios::in|std::ios::out|std::ios::binary);
#endif
			kosinski::decode(fobj, objfile);
			fobj.close();
			objfile.seekg(0);

			nemesis::decode(flay, layfile);
			flay.close();
			layfile.seekg(0);

#if defined(DEBUG_ENCODER)
			std::fstream kosfile((objectfile + ".kos").c_str(),
			                     std::ios::in|std::ios::out|std::ios::binary|std::ios::trunc);			
			std::fstream nemfile((layoutfile + ".nem").c_str(),
			                     std::ios::in|std::ios::out|std::ios::binary|std::ios::trunc);			
			kosinski::encode(objfile, kosfile);
			objfile.seekg(0);

			nemesis::encode(layfile, nemfile);
			layfile.seekg(0);
#endif
			ssobj_file ssfile;
			ssfile.read(objfile, layfile);
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
	{
		filedlg->run();
	}
}

void sseditor::on_savefilebutton_clicked()
{

}

void sseditor::on_revertfilebutton_clicked()
{

}

void sseditor::on_selectmodebutton_toggled()
{

}

void sseditor::on_insertmodebutton_toggled()
{

}

void sseditor::on_deletemodebutton_toggled()
{

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
	{
		aboutdlg->run();
	}
}

void sseditor::on_quitbutton_clicked()
{
	kit->quit();
}

void sseditor::on_first_stage_button_clicked()
{

}

void sseditor::on_previous_stage_button_clicked()
{

}

void sseditor::on_next_stage_button_clicked()
{

}

void sseditor::on_last_stage_button_clicked()
{

}

void sseditor::on_insert_stage_before_button_clicked()
{

}

void sseditor::on_append_stage_button_clicked()
{

}

void sseditor::on_delete_stage_button_clicked()
{

}

void sseditor::on_swap_stage_prev_button_clicked()
{

}

void sseditor::on_swap_stage_next_button_clicked()
{

}

void sseditor::on_first_segment_button_clicked()
{

}

void sseditor::on_previous_segment_button_clicked()
{

}

void sseditor::on_next_segment_button_clicked()
{

}

void sseditor::on_last_segment_button_clicked()
{

}

void sseditor::on_insert_segment_before_button_clicked()
{

}

void sseditor::on_append_segment_button_clicked()
{

}

void sseditor::on_delete_segment_button_clicked()
{

}

void sseditor::on_swap_segment_prev_button_clicked()
{

}

void sseditor::on_swap_segment_next_button_clicked()
{

}

void sseditor::on_normal_segment_toggled()
{

}

void sseditor::on_ring_message_toggled()
{

}

void sseditor::on_checkpoint_toggled()
{

}

void sseditor::on_chaos_emerald_toggled()
{

}

void on_segment_turnthenrise_toggled()
{

}

void on_segment_turnthendrop_toggled()
{

}

void on_segment_turnthenstraight_toggled()
{

}

void on_segment_straight_toggled()
{

}

void on_segment_straightthenturn_toggled()
{

}

void on_segment_right_toggled()
{

}

void on_segment_left_toggled()
{

}
