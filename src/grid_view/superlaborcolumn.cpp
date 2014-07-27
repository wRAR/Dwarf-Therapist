/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "superlaborcolumn.h"
#include "skillcolumn.h"
#include "columntypes.h"
#include "viewcolumnset.h"
#include "gamedatareader.h"
#include "dwarfmodel.h"
#include "truncatingfilelogger.h"
#include "superlabor.h"
#include "dwarftherapist.h"
#include "dwarf.h"
#include "labor.h"
#include "viewmanager.h"

SuperLaborColumn::SuperLaborColumn(const QString &title, QString id, ViewColumnSet *set, QObject *parent)
    : SkillColumn(title,-1, set, parent,CT_SUPER_LABOR)
    , m_id(id)
{
    m_current_sort = ViewManager::get_default_col_sort(CT_SUPER_LABOR);
}

SuperLaborColumn::SuperLaborColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : SkillColumn(s,set,parent)
    , m_id(s.value("id").toString())
{
    m_type = CT_SUPER_LABOR;
    m_current_sort = ViewManager::get_default_col_sort(m_type);
}

SuperLaborColumn::SuperLaborColumn(const SuperLaborColumn &to_copy)
    : SkillColumn(to_copy)
    , m_id(to_copy.m_id)
{
    m_type = CT_SUPER_LABOR;
    m_current_sort = ViewManager::get_default_col_sort(m_type);
}

QStandardItem *SuperLaborColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    LaborListBase *ml = DT->get_super_labor(m_id);
    item->setData(CT_SUPER_LABOR, DwarfModel::DR_COL_TYPE);

    if(!ml){
        item->setData(-1, DwarfModel::DR_RATING);
        item->setData(-1, DwarfModel::DR_DISPLAY_RATING);
        item->setData(-1,DwarfModel::DR_LABORS);
        item->setToolTip(tr("Unknown super labor."));
        return item;
    }else{
        float rating = ml->get_skill_rating(d->id());
        item->setData(rating, DwarfModel::DR_RATING);
        item->setData(rating, DwarfModel::DR_DISPLAY_RATING);
        item->setData(ml->get_converted_labors(),DwarfModel::DR_LABORS);
    }

    item->setToolTip(build_tooltip(d,ml));
    return item;
}

QString SuperLaborColumn::build_tooltip(Dwarf *d, LaborListBase *ml){
    float role_rating = ml->get_role_rating(d->id());
    float skill_rating = ml->get_skill_rating(d->id());

    refresh_sort(d, m_current_sort);

    QStringList modified_desc;
    QHash<int,QString> labors = ml->get_labor_desc();
    foreach(int id, labors.uniqueKeys()){
        if(d->labor_enabled(id))
            modified_desc.append(QString("<font color=%1>%2</font>").arg(ml->active_labor_color().name()).arg(labors.value(id)));
        else
            modified_desc.append(labors.value(id));
    }

    QString labors_desc = "";
    labors_desc = tr("<br/><br/><b>Labors:</b> %1").arg(labors.count() <= 0 ? tr("None") : modified_desc.join(", "));

//    QString prof_desc = "";
//    QString prof_name = ml->get_custom_prof_name();
//    if(!prof_name.isEmpty()){
//        prof_desc = tr("<br/><b>Custom Profession:</b> ");
//        if(d->profession() == prof_name)
//            prof_desc.append(QString("<font color=%1>%2</font>").arg(ml->active_labor_color().name()).arg(prof_name));
//        else
//            prof_desc.append(prof_name);
//    }

    QString skill_msg = "";
    skill_msg = tr("<h4 style=\"margin:0;\">Average Skill Level: %1</h4>").arg(QString::number(skill_rating,'f',2));

    QString role_msg = ml->get_role_name();
    if(role_msg.isEmpty()){
        role_msg = tr("<h4 style=\"margin:0;\">Average Role Rating: %1%</h4>").arg(QString::number(role_rating,'f',2));
    }else{
        role_msg = tr("<h4 style=\"margin:0;\">%1 Rating: %2%</h4>").arg(role_msg).arg(QString::number(role_rating,'f',2));
    }

    QString tooltip = QString("<center><h3>%1</h3><b>%2</b></center>%3%4%5%6")
            .arg(m_title)
            .arg(m_id)
            .arg(skill_msg)
            .arg(role_msg)
//            .arg(prof_desc)
            .arg(labors_desc)
            .arg(tooltip_name_footer(d));

    return tooltip;
}

float SuperLaborColumn::get_rating(int id, LaborListBase::LLB_RATING_TYPE type){
    float m_sort_val = 0.0;
    //SuperLabor *sl = DT->get_super_labor(m_id);
    LaborListBase *ml = DT->get_super_labor(m_id);
    if(ml)
        m_sort_val = ml->get_rating(id,type);
    return m_sort_val;
}

float SuperLaborColumn::get_base_sort(Dwarf *d){
    return get_rating(d->id(), LaborListBase::LLB_ACTIVE);
}

float SuperLaborColumn::get_role_rating(Dwarf *d){
    return get_rating(d->id(), LaborListBase::LLB_ROLE);
}

float SuperLaborColumn::get_skill_rating(int id, Dwarf *d){
    Q_UNUSED(id);
    return get_rating(d->id(), LaborListBase::LLB_SKILL);
}

float SuperLaborColumn::get_skill_rate_rating(int id, Dwarf *d){
    Q_UNUSED(id);
    return get_rating(d->id(), LaborListBase::LLB_SKILL_RATE);
}

QStandardItem *SuperLaborColumn::build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves){
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}

void SuperLaborColumn::write_to_ini(QSettings &s){
    ViewColumn::write_to_ini(s);
    s.setValue("id",m_id);
}