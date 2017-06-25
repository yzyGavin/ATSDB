/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * JobManager.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: sk
 */

#include <qtimer.h>
#include <QThreadPool>

#include "jobmanager.h"
#include "job.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

JobManager::JobManager()
    : Configurable ("JobManager", "JobManager0", 0, "conf/threads.xml")
{
    logdbg  << "JobManager: constructor";
    boost::mutex::scoped_lock l(mutex_);

    registerParameter ("num_workers", &num_workers_, 3);
    registerParameter ("update_time", &update_time_, 10);

    logdbg  << "JobManager: constructor: end";
}

JobManager::~JobManager()
{
    logdbg  << "JobManager: destructor";
}

void JobManager::shutdown ()
{
    logdbg  << "JobManager: shutdown";
    boost::mutex::scoped_lock l(mutex_);

    stop_requested_ = true;

    logdbg  << "JobManager: shutdown: done";
}

void JobManager::addJob (std::shared_ptr<Job> job)
{
    boost::mutex::scoped_lock l(mutex_);

    QThreadPool::globalInstance()->start(job.get());

    todos_signal_.push_back(job);
}

void JobManager::cancelJob (std::shared_ptr<Job> job)
{
    boost::mutex::scoped_lock l(mutex_);

    QThreadPool::globalInstance()->cancel(job.get());

    todos_signal_.erase(find(todos_signal_.begin(), todos_signal_.end(), job));
}

void JobManager::flushFinishedJobs ()
{
    boost::mutex::scoped_lock l(mutex_);
    while (todos_signal_.size() > 0)
    {
        std::shared_ptr<Job> current = todos_signal_.front();

        if( !current->obsolete() && !current->done() )
            break;

        todos_signal_.pop_front();
        logdbg << "JobManager: flushFinishedJobs: flushed job";
        if(current->obsolete())
        {
            logdbg << "JobManager: flushFinishedJobs: flushing obsolete job";
            current->emitObsolete();
            continue;
        }

        loginf << "JobManager: flushFinishedJobs: flushing done job";
        current->emitDone();
    }
}

bool JobManager::noJobs ()
{
    boost::mutex::scoped_lock l(mutex_);
    //logdbg << "JobManager: noJobs: todos " << todos_signal_.size();
    return todos_signal_.size() == 0;
}

/**
 * Creates thread if possible.
 *
 * \exception std::runtime_error if thread already existed
 */
void JobManager::run()
{
    logdbg  << "JobManager: run: start";

    while (1)
    {
        if (todos_signal_.size() > 0)
            flushFinishedJobs ();

        if (stop_requested_)
            break;

        msleep(update_time_);
    }
}

unsigned int JobManager::numJobs ()
{
    boost::mutex::scoped_lock l(mutex_);
    return todos_signal_.size();
}
