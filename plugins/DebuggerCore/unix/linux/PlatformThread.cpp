/*
Copyright (C) 2015 - 2015 Evan Teran
                          evan.teran@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PlatformThread.h"
#include "PlatformCommon.h"
#include "IProcess.h"
#include "DebuggerCore.h"
#include <cassert>

namespace DebuggerCore {

//------------------------------------------------------------------------------
// Name: 
// Desc: 
//------------------------------------------------------------------------------
PlatformThread::PlatformThread(DebuggerCore *core, IProcess *process, edb::tid_t tid) : core_(core), process_(process), tid_(tid) {
	assert(process);
	assert(core);
}

//------------------------------------------------------------------------------
// Name: 
// Desc: 
//------------------------------------------------------------------------------
PlatformThread::~PlatformThread() {
}

//------------------------------------------------------------------------------
// Name: 
// Desc: 
//------------------------------------------------------------------------------
edb::tid_t PlatformThread::tid() const {
	return tid_;
}

//------------------------------------------------------------------------------
// Name: 
// Desc: 
//------------------------------------------------------------------------------
QString PlatformThread::name() const  {
	struct user_stat thread_stat;
	int n = get_user_stat(QString("/proc/%1/task/%2/stat").arg(process_->pid()).arg(tid_), &thread_stat);
	if(n >= 2) {
		return thread_stat.comm;
	}
	
	return QString();
}

//------------------------------------------------------------------------------
// Name: 
// Desc: 
//------------------------------------------------------------------------------
int PlatformThread::priority() const  {
	struct user_stat thread_stat;
	int n = get_user_stat(QString("/proc/%1/task/%2/stat").arg(process_->pid()).arg(tid_), &thread_stat);
	if(n >= 30) {
		return thread_stat.priority;
	}
	
	return 0;
}

//------------------------------------------------------------------------------
// Name: 
// Desc: 
//------------------------------------------------------------------------------
edb::address_t PlatformThread::instruction_pointer() const  {
	struct user_stat thread_stat;
	int n = get_user_stat(QString("/proc/%1/task/%2/stat").arg(process_->pid()).arg(tid_), &thread_stat);
	if(n >= 18) {
		return thread_stat.kstkeip;
	}
	
	return 0;
}

//------------------------------------------------------------------------------
// Name: 
// Desc: 
//------------------------------------------------------------------------------
QString PlatformThread::runState() const  {
	struct user_stat thread_stat;
	int n = get_user_stat(QString("/proc/%1/task/%2/stat").arg(process_->pid()).arg(tid_), &thread_stat);
	if(n >= 3) {
		switch(thread_stat.state) {           // 03
		case 'R':
			return tr("%1 (Running)").arg(thread_stat.state);
			break;
		case 'S':
			return tr("%1 (Sleeping)").arg(thread_stat.state);
			break;
		case 'D':
			return tr("%1 (Disk Sleep)").arg(thread_stat.state);
			break;		
		case 'T':
			return tr("%1 (Stopped)").arg(thread_stat.state);
			break;		
		case 't':
			return tr("%1 (Tracing Stop)").arg(thread_stat.state);
			break;		
		case 'Z':
			return tr("%1 (Zombie)").arg(thread_stat.state);
			break;		
		case 'X':
		case 'x':
			return tr("%1 (Dead)").arg(thread_stat.state);
			break;
		case 'W':
			return tr("%1 (Waking/Paging)").arg(thread_stat.state);
			break;			
		case 'K':
			return tr("%1 (Wakekill)").arg(thread_stat.state);
			break;		
		case 'P':
			return tr("%1 (Parked)").arg(thread_stat.state);
			break;		
		default:
			return tr("%1").arg(thread_stat.state);
			break;		
		} 
	}
	
	return tr("Unknown");
}

//------------------------------------------------------------------------------
// Name: step
// Desc: steps this thread one instruction, passing the signal that stopped it 
//       (unless the signal was SIGSTOP)
//------------------------------------------------------------------------------
void PlatformThread::step() {
	core_->ptrace_step(tid_, resume_code(status_));
}

//------------------------------------------------------------------------------
// Name: step
// Desc: steps this thread one instruction, passing the signal that stopped it 
//       (unless the signal was SIGSTOP, or the passed status != DEBUG_EXCEPTION_NOT_HANDLED)
//------------------------------------------------------------------------------
void PlatformThread::step(edb::EVENT_STATUS status) {
	const int code = (status == edb::DEBUG_EXCEPTION_NOT_HANDLED) ? resume_code(status_) : 0;
	core_->ptrace_step(tid_, code);
}

//------------------------------------------------------------------------------
// Name: resume
// Desc: resumes this thread, passing the signal that stopped it 
//       (unless the signal was SIGSTOP)
//------------------------------------------------------------------------------	
void PlatformThread::resume() {
	core_->ptrace_continue(tid_, resume_code(status_));
}

//------------------------------------------------------------------------------
// Name: resume
// Desc: resumes this thread , passing the signal that stopped it 
//       (unless the signal was SIGSTOP, or the passed status != DEBUG_EXCEPTION_NOT_HANDLED)
//------------------------------------------------------------------------------
void PlatformThread::resume(edb::EVENT_STATUS status) {
	const int code = (status == edb::DEBUG_EXCEPTION_NOT_HANDLED) ? resume_code(status_) : 0;
	core_->ptrace_continue(tid_, code);
}

}
