/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Copyright (C) Flamewing 2011-2013 <flamewing.sonic@gmail.com>
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

#ifndef _ABSTRACTACTION_H_
#define _ABSTRACTACTION_H_

#include <tr1/memory>
#include <string>
#include <set>
#include <algorithm>
#include "ssobjfile.h"
#include "sslevelobjs.h"
#include "sssegmentobjs.h"
#include "object.h"

class ssobj_file;

class abstract_action {
public:
	enum MergeResult {
		eNoMerge = 0,
		eMergedActions = 1,
		eDeleteAction = -1
	};
	virtual ~abstract_action() {  }
	virtual std::string const display_string() const = 0;
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) = 0;
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) = 0;
	virtual MergeResult merge(std::tr1::shared_ptr<abstract_action> other) {
		return eNoMerge;
	}
};

class alter_selection_action : public abstract_action {
protected:
	std::set<object> objlist;
	int stage;
	sssegments::ObjectTypes type;
public:
	alter_selection_action(int s, sssegments::ObjectTypes t, std::set<object> const &sel)
		: objlist(sel), stage(s), type(t) {     }
	virtual ~alter_selection_action() {  }
	virtual std::string const display_string() const {
		return std::string("Change selection");
	}
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		sslevels *currlvl = ss->get_stage(stage);
		if (sel)
			sel->clear();
		for (std::set<object>::iterator it = objlist.begin();
		        it != objlist.end(); ++it) {
			sssegments *currseg = currlvl->get_segment(it->get_segment());
			currseg->update(it->get_pos(), it->get_angle(), type, false);
			if (sel)
				sel->insert(object(it->get_segment(), it->get_angle(),
				                   it->get_pos(), type));
		}
	}
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		sslevels *currlvl = ss->get_stage(stage);
		for (std::set<object>::iterator it = objlist.begin();
		        it != objlist.end(); ++it) {
			sssegments *currseg = currlvl->get_segment(it->get_segment());
			currseg->update(it->get_pos(), it->get_angle(), it->get_type(), false);
		}
		if (sel)
			*sel = objlist;
	}
	virtual MergeResult merge(std::tr1::shared_ptr<abstract_action> other) {
		std::tr1::shared_ptr<alter_selection_action> act =
		    std::tr1::dynamic_pointer_cast<alter_selection_action>(other);
		if (!act)
			return eNoMerge;

		if (objlist.size() != act->objlist.size())
			return eNoMerge;

		if (!std::equal(objlist.begin(), objlist.end(), act->objlist.begin()))
			return eNoMerge;

		for (std::set<object>::iterator it = objlist.begin();
		        it != objlist.end(); ++it)
			if (it->get_type() != act->type) {
				type = act->type;
				return eMergedActions;
			}
		return eDeleteAction;
	}
};

class delete_selection_action : public abstract_action {
protected:
	std::set<object> objlist;
	int stage;
public:
	friend class move_objects_action;
	delete_selection_action(int s, std::set<object> const &sel)
		: objlist(sel), stage(s) {      }
	virtual ~delete_selection_action() {  }
	virtual std::string const display_string() const {
		return std::string("Delete selection");
	}
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		if (sel)
			sel->clear();
		sslevels *currlvl = ss->get_stage(stage);
		size_t numsegments = currlvl->num_segments();

		for (std::set<object>::iterator it = objlist.begin();
		        it != objlist.end(); ++it) {
			if (it->get_segment() >= numsegments)
				continue;

			sssegments *currseg = currlvl->get_segment(it->get_segment());
			currseg->remove(it->get_pos(), it->get_angle());
		}
	}
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		sslevels *currlvl = ss->get_stage(stage);
		size_t numsegments = currlvl->num_segments();

		for (std::set<object>::iterator it = objlist.begin();
		        it != objlist.end(); ++it) {
			if (it->get_segment() >= numsegments)
				continue;

			sssegments *currseg = currlvl->get_segment(it->get_segment());
			currseg->update(it->get_pos(), it->get_angle(),
			                it->get_type(), true);
		}
		if (sel)
			*sel = objlist;
	}
};

class cut_selection_action : public delete_selection_action {
public:
	cut_selection_action(int s, std::set<object> const &sel)
		: delete_selection_action(s, sel) {     }
	virtual ~cut_selection_action() {  }
	virtual std::string const display_string() const {
		return std::string("Cut selection");
	}
};

class insert_objects_action : public delete_selection_action {
public:
	insert_objects_action(int s, std::set<object> const &sel)
		: delete_selection_action(s, sel) {     }
	virtual ~insert_objects_action() {  }
	virtual std::string const display_string() const {
		return std::string("Insert objects");
	}
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		delete_selection_action::revert(ss, sel);
	}
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		delete_selection_action::apply(ss, sel);
	}
};

class paste_objects_action : public insert_objects_action {
public:
	paste_objects_action(int s, std::set<object> const &sel)
		: insert_objects_action(s, sel) {       }
	virtual ~paste_objects_action() {  }
	virtual std::string const display_string() const {
		return std::string("Paste objects");
	}
};

class move_objects_action : public abstract_action {
protected:
	std::tr1::shared_ptr<delete_selection_action> from;
	std::tr1::shared_ptr<paste_objects_action> to;
public:
	move_objects_action(int s, std::set<object> const &del, std::set<object> const &add)
		: from(new delete_selection_action(s, del)), to(new paste_objects_action(s, add)) {
	}
	virtual ~move_objects_action() {  }
	virtual std::string const display_string() const {
		return std::string("Move objects");
	}
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		from->apply(ss, sel);
		to->apply(ss, sel);
	}
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		to->revert(ss, sel);
		from->revert(ss, sel);
	}
	virtual MergeResult merge(std::tr1::shared_ptr<abstract_action> other) {
		std::tr1::shared_ptr<move_objects_action> act =
		    std::tr1::dynamic_pointer_cast<move_objects_action>(other);
		if (!act)
			return eNoMerge;

		std::set<object> &list1 = to->objlist, list2 = act->from->objlist;
		if (list1.size() != list2.size())
			return eNoMerge;

		if (!std::equal(list1.begin(), list1.end(), list2.begin(),
		                ObjectMatchFunctor()))
			return eNoMerge;

		if (std::equal(from->objlist.begin(), from->objlist.end(),
		               act->to->objlist.begin(), ObjectMatchFunctor()))
			return eDeleteAction;

		list1 = act->to->objlist;
		return eMergedActions;
	}
};

class insert_objects_ex_action : public move_objects_action {
public:
	insert_objects_ex_action(int s, std::set<object> const &del, std::set<object> const &add)
		: move_objects_action(s, del, add) {
	}
	virtual std::string const display_string() const {
		return std::string("Insert objects");
	}
	virtual MergeResult merge(std::tr1::shared_ptr<abstract_action> other) {
		return eNoMerge;
	}
};

class alter_segment_action : public abstract_action {
protected:
	int stage, seg;
	bool newflip, oldflip;
	sssegments::SegmentTypes    newterminator, oldterminator;
	sssegments::SegmentGeometry newgeometry  , oldgeometry;
public:
	alter_segment_action(int s, int sg, sssegments const &sgm, bool tf,
	                     sssegments::SegmentTypes newterm,
	                     sssegments::SegmentGeometry newgeom)
		: stage(s), seg(sg) {
		newflip = tf;
		oldflip = sgm.get_direction();
		newterminator = newterm;
		oldterminator = sgm.get_type();
		newgeometry = newgeom;
		oldgeometry = sgm.get_geometry();
	}
	virtual ~alter_segment_action() {  }
	virtual std::string const display_string() const {
		return std::string("Change selection");
	}
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		sslevels *currlvl = ss->get_stage(stage);
		if (!currlvl)
			return;

		sssegments *currseg = currlvl->get_segment(seg);
		if (!currseg)
			return;

		currseg->set_direction(newflip);
		currseg->set_type(newterminator);
		currseg->set_geometry(newgeometry);
		if (sel)
			sel->clear();
	}
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		sslevels *currlvl = ss->get_stage(stage);
		if (!currlvl)
			return;

		sssegments *currseg = currlvl->get_segment(seg);
		if (!currseg)
			return;

		currseg->set_direction(oldflip);
		currseg->set_type(oldterminator);
		currseg->set_geometry(oldgeometry);
		if (sel)
			sel->clear();
	}
	virtual MergeResult merge(std::tr1::shared_ptr<abstract_action> other) {
		std::tr1::shared_ptr<alter_segment_action> act =
		    std::tr1::dynamic_pointer_cast<alter_segment_action>(other);
		if (!act)
			return eNoMerge;

		if (stage != act->stage || seg != act->seg)
			return eNoMerge;

		if (newflip == act->newflip && newterminator == act->newterminator
		        && newgeometry == act->newgeometry)
			return eDeleteAction;

		newflip = act->newflip;
		newterminator = act->newterminator;
		newgeometry = act->newgeometry;
		return eMergedActions;
	}
};

class delete_segment_action : public abstract_action {
protected:
	sssegments segment;
	int stage, seg;
public:
	delete_segment_action(int s, int sg, sssegments const &sgm)
		: segment(sgm), stage(s), seg(sg) {     }
	virtual ~delete_segment_action() {  }
	virtual std::string const display_string() const {
		return std::string("Delete segment");
	}
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		sslevels *currlvl = ss->get_stage(stage);
		if (seg >= currlvl->num_segments())
			return;

		ss->get_stage(stage)->remove(seg);
		if (sel)
			sel->clear();
	}
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		sslevels *currlvl = ss->get_stage(stage);
		if (seg == currlvl->num_segments())
			ss->get_stage(stage)->append(segment);
		else if (seg < currlvl->num_segments())
			ss->get_stage(stage)->insert(segment, seg);
		if (sel)
			sel->clear();
	}
};

class cut_segment_action : public delete_segment_action {
public:
	cut_segment_action(int s, int sg, sssegments const &sgm)
		: delete_segment_action(s, sg, sgm) {       }
	virtual ~cut_segment_action() {  }
	virtual std::string const display_string() const {
		return std::string("Cut segment");
	}
};

class insert_segment_action : public delete_segment_action {
public:
	insert_segment_action(int s, int sg, sssegments const &sgm)
		: delete_segment_action(s, sg, sgm) {       }
	virtual ~insert_segment_action() {  }
	virtual std::string const display_string() const {
		return std::string("Insert segment");
	}
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		delete_segment_action::revert(ss, sel);
	}
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		delete_segment_action::apply(ss, sel);
	}
};

class paste_segment_action : public insert_segment_action {
public:
	paste_segment_action(int s, int sg, sssegments const &sgm)
		: insert_segment_action(s, sg, sgm) {       }
	virtual ~paste_segment_action() {  }
	virtual std::string const display_string() const {
		return std::string("Paste segment");
	}
};

class move_segment_action : public abstract_action {
protected:
	int stage, seg, dir;
public:
	move_segment_action(int s, int sg, int d)
		: stage(s), seg(sg), dir(d) {
	}
	virtual ~move_segment_action() {  }
	virtual std::string const display_string() const {
		return std::string("Move segment");
	}
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		sslevels *currlvl = ss->get_stage(stage);
		if (dir > 0)
			ss->get_stage(stage)->move_right(seg);
		else if (dir < 0)
			ss->get_stage(stage)->move_left(seg);
		if (sel)
			sel->clear();
	}
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		sslevels *currlvl = ss->get_stage(stage);
		if (dir < 0)
			ss->get_stage(stage)->move_right(seg - 1);
		else if (dir > 0)
			ss->get_stage(stage)->move_left(seg + 1);
		if (sel)
			sel->clear();
	}
};

class delete_stage_action : public abstract_action {
protected:
	sslevels level;
	int stage;
public:
	friend class move_stage_action;
	delete_stage_action(int s, sslevels const &l)
		: level(l), stage(s) {      }
	virtual ~delete_stage_action() {  }
	virtual std::string const display_string() const {
		return std::string("Delete stage");
	}
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		if (stage >= ss->num_stages())
			return;

		ss->remove(stage);
		if (sel)
			sel->clear();
	}
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		if (stage == ss->num_stages())
			ss->append(level);
		else if (stage < ss->num_stages())
			ss->insert(level, stage);
		if (sel)
			sel->clear();
	}
};

class cut_stage_action : public delete_stage_action {
public:
	cut_stage_action(int s, sslevels const &l)
		: delete_stage_action(s, l) {       }
	virtual ~cut_stage_action() {  }
	virtual std::string const display_string() const {
		return std::string("Cut stage");
	}
};

class insert_stage_action : public delete_stage_action {
public:
	insert_stage_action(int s, sslevels const &l)
		: delete_stage_action(s, l) {       }
	virtual ~insert_stage_action() {  }
	virtual std::string const display_string() const {
		return std::string("Insert stage");
	}
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		delete_stage_action::revert(ss, sel);
	}
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		delete_stage_action::apply(ss, sel);
	}
};

class paste_stage_action : public insert_stage_action {
public:
	paste_stage_action(int s, sslevels const &l)
		: insert_stage_action(s, l) {       }
	virtual ~paste_stage_action() {  }
	virtual std::string const display_string() const {
		return std::string("Paste stage");
	}
};

class move_stage_action : public abstract_action {
protected:
	int stage, dir;
public:
	move_stage_action(int s, int d)
		: stage(s), dir(d) {
	}
	virtual ~move_stage_action() {  }
	virtual std::string const display_string() const {
		return std::string("Move stage");
	}
	virtual void apply(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		if (dir > 0)
			ss->move_right(stage);
		else if (dir < 0)
			ss->move_left(stage);
		if (sel)
			sel->clear();
	}
	virtual void revert(std::tr1::shared_ptr<ssobj_file> ss, std::set<object> *sel) {
		if (dir < 0)
			ss->move_right(stage - 1);
		else if (dir > 0)
			ss->move_left(stage + 1);
		if (sel)
			sel->clear();
	}
};


#endif // _ABSTRACTACTION_H_
