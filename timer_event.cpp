#include "timer_event.h"
#include "lua_interface.h"
#include "log.h"

void handle_timeout(void* ptr)
{
	TimerEvent* sh = (TimerEvent*)ptr;
	sh->OnTimer(sh->m_ev.time_id);
}

unsigned long  TimerEvent::m_increase_id = 0;
timer_list_t timer_list;

TimerEvent::TimerEvent()
{
	m_guid = ++m_increase_id;
}

TimerEvent::~TimerEvent(void)
{
}

void TimerEvent::StartTimer(int sec, int usec)
{
	m_timeout = sec;
	start_timer(sec, usec, &m_ev);
}

void TimerEvent::StopTimer()
{
	stop_timer(&m_ev);
}

void TimerEvent::ResetTimer()
{
	StopTimer();
	StartTimer(m_timeout);
}

//void TimerEvent::SetTimerId(int timer_id)
void TimerEvent::SetTimerId(unsigned long timer_id)
{
	m_ev.time_id = timer_id;
	m_ev.callback = handle_timeout;
	m_ev.ptr = this;
}

//void TimerEvent::OnTimer(int timer_id)
void TimerEvent::OnTimer(unsigned long timer_id)

{
	int ret;
	
	log_debug("TimerEvent::OnTimer, timer_id = %lu \n", m_ev.time_id);
	
	call_lua("handle_timeout", "d>d", m_guid, &ret);
}

int
TimerEvent::GetRemain()
{
	return remain_timer(&m_ev);
}


