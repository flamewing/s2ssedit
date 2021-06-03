/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Copyright (C) Flamewing 2011-2015 <flamewing.sonic@gmail.com>
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

#ifndef ABSTRACTACTION_H
#define ABSTRACTACTION_H

#include "s2ssedit/ignore_unused_variable_warning.hh"
#include "s2ssedit/object.hh"
#include "s2ssedit/sslevelobjs.hh"
#include "s2ssedit/ssobjfile.hh"
#include "s2ssedit/sssegmentobjs.hh"

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <utility>

class ssobj_file;

using ssobj_file_shared = std::shared_ptr<ssobj_file>;
using object_set        = std::set<object>;

class abstract_action {
public:
    enum MergeResult { eNoMerge = 0, eMergedActions = 1, eDeleteAction = -1 };
    // Boilerplate
    abstract_action() noexcept                  = default;
    abstract_action(abstract_action const&)     = default;
    abstract_action(abstract_action&&) noexcept = default;
    abstract_action& operator=(abstract_action const&) = default;
    abstract_action& operator=(abstract_action&&) noexcept = default;
    // End boilerplate
    virtual ~abstract_action() noexcept                        = default;
    virtual void apply(ssobj_file_shared ss, object_set* sel)  = 0;
    virtual void revert(ssobj_file_shared ss, object_set* sel) = 0;

    virtual MergeResult merge(std::shared_ptr<abstract_action> const& other) {
        ignore_unused_variable_warning(other);
        return eNoMerge;
    }
};

class alter_selection_action final : public abstract_action {
private:
    object_set              objlist;
    int                     stage;
    sssegments::ObjectTypes type;

public:
    alter_selection_action(int s, sssegments::ObjectTypes t, object_set sel)
            : objlist(std::move(sel)), stage(s), type(t) {}
    void apply(ssobj_file_shared ss, object_set* sel) override {
        sslevels* currlvl = ss->get_stage(stage);
        if (sel != nullptr) {
            sel->clear();
        }
        for (auto const& elem : objlist) {
            sssegments* currseg = currlvl->get_segment(elem.get_segment());
            currseg->update(elem.get_pos(), elem.get_angle(), type, false);
            if (sel != nullptr) {
                sel->insert(
                        object(elem.get_segment(), elem.get_angle(),
                               elem.get_pos(), type));
            }
        }
    }
    void revert(ssobj_file_shared ss, object_set* sel) override {
        sslevels* currlvl = ss->get_stage(stage);
        for (auto const& elem : objlist) {
            sssegments* currseg = currlvl->get_segment(elem.get_segment());
            currseg->update(
                    elem.get_pos(), elem.get_angle(), elem.get_type(), false);
        }
        if (sel != nullptr) {
            *sel = objlist;
        }
    }
    MergeResult merge(std::shared_ptr<abstract_action> const& other) override {
        std::shared_ptr<alter_selection_action> act
                = std::dynamic_pointer_cast<alter_selection_action>(other);
        if (!act) {
            return eNoMerge;
        }

        if (objlist.size() != act->objlist.size()) {
            return eNoMerge;
        }

        if (!std::equal(objlist.begin(), objlist.end(), act->objlist.begin())) {
            return eNoMerge;
        }

        for (auto const& elem : objlist) {
            if (elem.get_type() != act->type) {
                type = act->type;
                return eMergedActions;
            }
        }
        return eDeleteAction;
    }
};

class delete_selection_action : public abstract_action {
private:
    object_set objlist;
    int        stage;

public:
    friend class move_objects_action;
    delete_selection_action(int s, object_set sel)
            : objlist(std::move(sel)), stage(s) {}
    void apply(ssobj_file_shared ss, object_set* sel) override {
        if (sel != nullptr) {
            sel->clear();
        }
        sslevels* currlvl     = ss->get_stage(stage);
        int       numsegments = currlvl->num_segments();

        for (auto const& elem : objlist) {
            if (elem.get_segment() >= numsegments) {
                continue;
            }

            sssegments* currseg = currlvl->get_segment(elem.get_segment());
            currseg->remove(elem.get_pos(), elem.get_angle());
        }
    }
    void revert(ssobj_file_shared ss, object_set* sel) override {
        sslevels* currlvl     = ss->get_stage(stage);
        int       numsegments = currlvl->num_segments();

        for (auto const& elem : objlist) {
            if (elem.get_segment() >= numsegments) {
                continue;
            }

            sssegments* currseg = currlvl->get_segment(elem.get_segment());
            currseg->update(
                    elem.get_pos(), elem.get_angle(), elem.get_type(), true);
        }
        if (sel != nullptr) {
            *sel = objlist;
        }
    }
};

using cut_selection_action = delete_selection_action;

class insert_objects_action final : public delete_selection_action {
public:
    insert_objects_action(int s, object_set const& sel)
            : delete_selection_action(s, sel) {}
    void apply(ssobj_file_shared ss, object_set* sel) override {
        delete_selection_action::revert(ss, sel);
    }
    void revert(ssobj_file_shared ss, object_set* sel) override {
        delete_selection_action::apply(ss, sel);
    }
};

using paste_objects_action = insert_objects_action;

class move_objects_action : public abstract_action {
private:
    std::shared_ptr<delete_selection_action> from;
    std::shared_ptr<paste_objects_action>    to;

public:
    move_objects_action(int s, object_set const& del, object_set const& add)
            : from(std::make_shared<delete_selection_action>(s, del)),
              to(std::make_shared<paste_objects_action>(s, add)) {}
    void apply(ssobj_file_shared ss, object_set* sel) final {
        from->apply(ss, sel);
        to->apply(ss, sel);
    }
    void revert(ssobj_file_shared ss, object_set* sel) override {
        to->revert(ss, sel);
        from->revert(ss, sel);
    }
    MergeResult merge(std::shared_ptr<abstract_action> const& other) override {
        std::shared_ptr<move_objects_action> act
                = std::dynamic_pointer_cast<move_objects_action>(other);
        if (!act) {
            return eNoMerge;
        }

        object_set& list1 = to->objlist;
        object_set& list2 = act->from->objlist;
        if (list1.size() != list2.size()) {
            return eNoMerge;
        }

        if (!std::equal(
                    list1.begin(), list1.end(), list2.begin(),
                    ObjectMatchFunctor())) {
            return eNoMerge;
        }

        if (std::equal(
                    from->objlist.begin(), from->objlist.end(),
                    act->to->objlist.begin(), ObjectMatchFunctor())) {
            return eDeleteAction;
        }

        list1 = act->to->objlist;
        return eMergedActions;
    }
};

class insert_objects_ex_action final : public move_objects_action {
public:
    insert_objects_ex_action(
            int s, object_set const& del, object_set const& add)
            : move_objects_action(s, del, add) {}
    MergeResult merge(std::shared_ptr<abstract_action> const& other) override {
        ignore_unused_variable_warning(other);
        return eNoMerge;
    }
};

class alter_segment_action final : public abstract_action {
private:
    int  stage, seg;
    bool newflip, oldflip;

    sssegments::SegmentTypes    newterminator, oldterminator;
    sssegments::SegmentGeometry newgeometry, oldgeometry;

public:
    alter_segment_action(
            int s, int sg, sssegments const& sgm, bool tf,
            sssegments::SegmentTypes    newterm,
            sssegments::SegmentGeometry newgeom)
            : stage(s), seg(sg) {
        newflip       = tf;
        oldflip       = sgm.get_direction();
        newterminator = newterm;
        oldterminator = sgm.get_type();
        newgeometry   = newgeom;
        oldgeometry   = sgm.get_geometry();
    }
    void apply(ssobj_file_shared ss, object_set* sel) override {
        sslevels* currlvl = ss->get_stage(stage);
        if (currlvl == nullptr) {
            return;
        }

        sssegments* currseg = currlvl->get_segment(seg);
        if (currseg == nullptr) {
            return;
        }

        currseg->set_direction(newflip);
        currseg->set_type(newterminator);
        currseg->set_geometry(newgeometry);
        if (sel != nullptr) {
            sel->clear();
        }
    }
    void revert(ssobj_file_shared ss, object_set* sel) override {
        sslevels* currlvl = ss->get_stage(stage);
        if (currlvl == nullptr) {
            return;
        }

        sssegments* currseg = currlvl->get_segment(seg);
        if (currseg == nullptr) {
            return;
        }

        currseg->set_direction(oldflip);
        currseg->set_type(oldterminator);
        currseg->set_geometry(oldgeometry);
        if (sel != nullptr) {
            sel->clear();
        }
    }
    MergeResult merge(std::shared_ptr<abstract_action> const& other) override {
        std::shared_ptr<alter_segment_action> act
                = std::dynamic_pointer_cast<alter_segment_action>(other);
        if (!act) {
            return eNoMerge;
        }

        if (stage != act->stage || seg != act->seg) {
            return eNoMerge;
        }

        if (newflip == act->newflip && newterminator == act->newterminator
            && newgeometry == act->newgeometry) {
            return eDeleteAction;
        }

        newflip       = act->newflip;
        newterminator = act->newterminator;
        newgeometry   = act->newgeometry;
        return eMergedActions;
    }
};

class delete_segment_action : public abstract_action {
private:
    sssegments segment;
    unsigned   stage, seg;

public:
    delete_segment_action(int s, int sg, sssegments sgm)
            : segment(std::move(sgm)), stage(s), seg(sg) {}
    void apply(ssobj_file_shared ss, object_set* sel) override {
        sslevels* currlvl = ss->get_stage(stage);
        if (seg >= currlvl->num_segments()) {
            return;
        }

        ss->get_stage(stage)->remove(seg);
        if (sel != nullptr) {
            sel->clear();
        }
    }
    void revert(ssobj_file_shared ss, object_set* sel) override {
        sslevels* currlvl = ss->get_stage(stage);
        if (seg == currlvl->num_segments()) {
            ss->get_stage(stage)->append(segment);
        } else if (seg < currlvl->num_segments()) {
            ss->get_stage(stage)->insert(segment, seg);
        }
        if (sel != nullptr) {
            sel->clear();
        }
    }
};

class cut_segment_action final : public delete_segment_action {
public:
    cut_segment_action(int s, int sg, sssegments const& sgm)
            : delete_segment_action(s, sg, sgm) {}
};

class insert_segment_action final : public delete_segment_action {
public:
    insert_segment_action(int s, int sg, sssegments const& sgm)
            : delete_segment_action(s, sg, sgm) {}
    void apply(ssobj_file_shared ss, object_set* sel) override {
        delete_segment_action::revert(ss, sel);
    }
    void revert(ssobj_file_shared ss, object_set* sel) override {
        delete_segment_action::apply(ss, sel);
    }
};

using paste_segment_action = insert_segment_action;

class move_segment_action final : public abstract_action {
private:
    int stage, seg, dir;

public:
    move_segment_action(int s, int sg, int d) : stage(s), seg(sg), dir(d) {}
    void apply(ssobj_file_shared ss, object_set* sel) override {
        if (dir > 0) {
            ss->get_stage(stage)->move_right(seg);
        } else if (dir < 0) {
            ss->get_stage(stage)->move_left(seg);
        }
        if (sel != nullptr) {
            sel->clear();
        }
    }
    void revert(ssobj_file_shared ss, object_set* sel) override {
        if (dir < 0) {
            ss->get_stage(stage)->move_right(seg - 1);
        } else if (dir > 0) {
            ss->get_stage(stage)->move_left(seg + 1);
        }
        if (sel != nullptr) {
            sel->clear();
        }
    }
};

class delete_stage_action : public abstract_action {
private:
    sslevels level;
    unsigned stage;

public:
    friend class move_stage_action;
    delete_stage_action(int s, sslevels l) : level(std::move(l)), stage(s) {}
    void apply(ssobj_file_shared ss, object_set* sel) override {
        if (stage >= ss->num_stages()) {
            return;
        }

        ss->remove(stage);
        if (sel != nullptr) {
            sel->clear();
        }
    }
    void revert(ssobj_file_shared ss, object_set* sel) override {
        if (stage == ss->num_stages()) {
            ss->append(level);
        } else if (stage < ss->num_stages()) {
            ss->insert(level, stage);
        }
        if (sel != nullptr) {
            sel->clear();
        }
    }
};

using cut_stage_action = delete_stage_action;

class insert_stage_action final : public delete_stage_action {
public:
    insert_stage_action(int s, sslevels const& l) : delete_stage_action(s, l) {}
    void apply(ssobj_file_shared ss, object_set* sel) override {
        delete_stage_action::revert(ss, sel);
    }
    void revert(ssobj_file_shared ss, object_set* sel) override {
        delete_stage_action::apply(ss, sel);
    }
};

using paste_stage_action = insert_stage_action;

class move_stage_action final : public abstract_action {
private:
    int stage, dir;

public:
    move_stage_action(int s, int d) : stage(s), dir(d) {}
    void apply(ssobj_file_shared ss, object_set* sel) override {
        if (dir > 0) {
            ss->move_right(stage);
        } else if (dir < 0) {
            ss->move_left(stage);
        }
        if (sel != nullptr) {
            sel->clear();
        }
    }
    void revert(ssobj_file_shared ss, object_set* sel) override {
        if (dir < 0) {
            ss->move_right(stage - 1);
        } else if (dir > 0) {
            ss->move_left(stage + 1);
        }
        if (sel != nullptr) {
            sel->clear();
        }
    }
};

#endif    // ABSTRACTACTION_H
