//     -*- mode: c++ ; c-file-style: "osse" ; -*-
//
// Copyright (c) 2003
// Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
//    View of Tasks.
//
#ifndef _BTT_VIEW_TASKS_HH_
#define _BTT_VIEW_TASKS_HH_


#include <string>
#include <btt/Tasks.hh>
#include <boost/noncopyable.hpp>
#include <xju/Ring.hh>
#include <xju/format.hh>
#include <xju/Observer.hh>
#include <boost/utility.hpp>
#include <xju/formatTime.hh>
#include <FL/Fl_Browser.H>
#include <btt/fltk1.3-view/Callback.hh>
#include <memory>

namespace btt
{
    namespace view
    {
	class Tasks : boost::noncopyable
	{
	public:
	    Tasks(Fl_Browser& widget,
		  const btt::Tasks& tasks) throw();

            // popup action menu for selectedTask_
	    //xju::Event popupMenu_;

            // edit selectedTask_
	    xju::Event editTask_;

	    //
	    // tasks.end() means none selected
	    //
	    xju::Notifying<btt::Tasks::const_iterator> selectedTask_;
	    
	private:
            Fl_Browser& widget_;
	    const btt::Tasks& tasks_;
	    class Task;
	    friend class Task;
	    
	    xju::Ring<Task*>  registry_;


	    class Task : boost::noncopyable
	    {
	    public:
                Tasks*const parent_;
		const btt::Tasks::const_iterator task_;
                Fl_Browser& browser_;
                
		//
		// Insert task before successor within specified parent list.
		//
		Task(Tasks* parent,
		     const btt::Tasks::const_iterator task,
		     xju::Ring<Task*>& successor,
                     Fl_Browser& browser) throw():
                    parent_(parent),
		    task_(task),
		    browser_(browser),
		    reg_(this),
		    startedChanged_(
			(reg_.splice(successor),*this),
			&Task::changed,
			(*task).started_.m_changed),
		    secondsSpentOnThisTaskChanged_(
			*this,
			&Task::changed,
			(*task).secondsSpentOnThisTask_.m_changed),
		    descriptionChanged_(
			*this,
			&Task::changed,
			(*task).description_.m_changed),
		    newPredecessor_(*this,
				    &Task::newPredecessor,
				    parent->tasks_.newPredecessor(task)),
		    removing_(*this,
			      &Task::removing,
			      parent->tasks_.removing(task))
		{
                    int const i(index());
                    browser.insert(i+1, "", this);
		    changed_(i);
                    browser.select(i+1, true);
                    browser.redraw();
                    parent_->selectedTask_ = task_;
		}

                void changed() throw()
                {
                    changed_(index());
                }
                
                void changed_(int i) throw()
                {
		    const std::string id((*task_).id_.readableRepr());

		    const std::string started(
			(*task_).started_.value() ?
			xju::formatTime(*(*task_).started_.value(), "%x") :
			std::string(""));

		    double seconds((*task_).secondsSpentOnThisTask_.value());
		    double hours(seconds/60.0/60.0);
                    std::string const spentHrs(
                        xju::format::float_(
                            hours,
                            std::ios_base::fixed,
                            1));
		    const std::string desc((*task_).description_.value());
                    
                    std::ostringstream s;
                    s << "@."
                      << id << "\t" 
                      << started << "\t" 
                      << spentHrs << "\t" 
                      << desc;
                    browser_.text(i+1,s.str().c_str());
		}

	    private:
		xju::Ring<Task*> reg_;

		xju::Observer<Task> startedChanged_;
		xju::Observer<Task> secondsSpentOnThisTaskChanged_;
		xju::Observer<Task> descriptionChanged_;

		xju::Observer<Task> removing_;
		xju::Observer<Task> newPredecessor_;

                // zero-based index of this Task w.r.t. first task.
                unsigned int index() const throw()
                {
                    unsigned int result(0);
                    for(xju::Ring<Task*> const* x = &reg_; 
                        x->prev().m_value;
                        x=&x->prev())
                    {
                        ++result;
                    }
                    return result;
                }
                
		void removing() throw()
		{
                    int const i=index();
                    if (browser_.value()==i+1) {
                        parent_->selectedTask_ = parent_->tasks_.end();
                        browser_.value(0);
                    }
                    browser_.remove(i+1);
		    delete this;
		}
		
		void newPredecessor() throw()
		{
		    new Task(parent_, boost::prior(task_), reg_, browser_);
		}

	    };

	    void taskAddedAtEnd() throw()
	    {
		new Task(this,
                         boost::prior(tasks_.end()), 
                         registry_,
                         widget_);
	    }

            // void tasksCallback() throw()
            // {
            //     if (widget_.item())
            //     {
            //         widget_.item()->do_callback();
            //     }
            //     else
            //     {
            //         selectedTask_ = tasks_.end();
            //     }
            // }
            void tasksCallback() throw()
            {
                if (widget_.value()) {
                    int const index(widget_.value()-1);
                    Task* t = (Task*)widget_.data(index+1);
                    selectedTask_ = t->task_;
                }
                else
                {
                    selectedTask_ = tasks_.end();
                }
            }
            
	    xju::Observer<Tasks> taskAddedAtEnd_;

            btt::view::Callback<Tasks> tasksCallback_;
        
	};
    }
}


#endif
