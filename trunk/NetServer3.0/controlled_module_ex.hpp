// controlled_module_ex.hpp : controlled_module类的扩展
// 增强线程之间消息通讯
// 增加线程安全启动和安全关闭功能
// 增加定时器功能

#pragma once
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include "controlled_module.hpp"

struct _command
{
	typedef boost::shared_ptr<_command> CCmdPtr;
	unsigned int nCmd;
	boost::any anyParam;
};
struct _wait_command
{		
	boost::any par;
	unsigned int command;
	void * event;
	boost::shared_ptr<boost::any> resp;
};
class controlled_module_ex;
struct _notify
{
	controlled_module_ex * sender;
	int id;
	boost::any par;
};
#define BM_RESERVE 1000
#define BM_RING_START BM_RESERVE+1
#define BM_RING_STOP BM_RESERVE+2
#define BM_RING_SETTIME	 BM_RESERVE+3
#define BM_RING_SETPARENT BM_RESERVE+4
#define BM_RING_CYCLE BM_RESERVE+5
#define BM_RING_PROCESS BM_RESERVE+6
#define BM_RING_PROCESSEND BM_RESERVE+7
#define BM_RING_PROCESSFAIL BM_RESERVE+8
#define BM_TIMER	BM_RESERVE+9
#define BM_COMMAND	BM_RESERVE+10
#define BM_NOTIFY	BM_RESERVE+11

#define BM_USER	9000
class controlled_timer;
class controlled_module_ex: public controlled_module
{
public:
	controlled_module_ex()
	{
		m_safe = false;
	}
	~controlled_module_ex()
	{
		safestop();
	}
public:
	template<typename T>
	bool postmessage(unsigned int nCmd, const boost::shared_ptr<T>& p)
	{
		if(this==0||!m_safe)return false;
		boost::mutex::scoped_lock lock(m_mutex_command);
		_command::CCmdPtr cmd(new _command);
		cmd->nCmd = nCmd;
		cmd->anyParam = p;
		m_list_command.push_back(cmd);
		return true;
	}
	boost::any execute(unsigned int command,boost::any par,int timeout=-1)
	{
		boost::shared_ptr<_wait_command> shared(new _wait_command);
		_wait_command & cmd = *shared;
		cmd.command = command;
		cmd.event = (void *)CreateEvent(0,FALSE,FALSE,0);
		cmd.par = par;
		cmd.resp = boost::shared_ptr<boost::any>(new boost::any);
		if(this->postmessage(BM_COMMAND,shared))
		{
			DWORD dw = WaitForSingleObject(cmd.event,timeout);
			CloseHandle(cmd.event);
			if(dw!=WAIT_OBJECT_0)
				return boost::any();
			else
				return *cmd.resp;
		}
		else
		{
			CloseHandle(cmd.event);
			return boost::any();
		}
	}
	void notify(_notify p)
	{
		this->postmessage(BM_NOTIFY,p);
	}
	bool postmessage(unsigned int nCmd,boost::any p)
	{
		if(this==0||!m_safe)
			return false;
		boost::mutex::scoped_lock lock(m_mutex_command);
		_command::CCmdPtr cmd(new _command);
		cmd->nCmd = nCmd;
		cmd->anyParam = p;
		m_list_command.push_back(cmd);
		return true;
	}
	bool postmessage(unsigned int nCmd)
	{
		if(this==0||!m_safe)
			return false;
		boost::mutex::scoped_lock lock(m_mutex_command);
		_command::CCmdPtr cmd(new _command);
		cmd->nCmd = nCmd;
		cmd->anyParam = 0;
		m_list_command.push_back(cmd);
		return true;
	}
	virtual bool work()
	{
		if(!getmessage())
			return false;
		else
		{
			Sleep(this->m_sleeptime);
			return true;
		}
	}
	virtual void message(const _command & cmd)
	{
		if(cmd.nCmd==BM_RING_START)
		{
			this->on_safestart();
		}
		else if(cmd.nCmd==BM_RING_STOP)
		{
			this->on_safestop();
		}
		else if(cmd.nCmd==BM_TIMER)
		{
			this->on_timer(boost::any_cast<controlled_timer*>(cmd.anyParam));
		}
		else if(cmd.nCmd==BM_COMMAND)
		{
			boost::shared_ptr<_wait_command> shared = boost::any_cast< boost::shared_ptr<_wait_command> >(cmd.anyParam);
			_wait_command & cmd = *shared;
			*cmd.resp = this->on_command(cmd.command,cmd.par);
			SetEvent((HANDLE)cmd.event);
		}
		else if(cmd.nCmd==BM_NOTIFY)
		{
			try
			{
				_notify par = boost::any_cast<_notify>(cmd.anyParam);
				this->on_notify(par);
			}
			catch(boost::bad_any_cast)
			{
			}
		}
	}
	virtual void release()
	{
		boost::mutex::scoped_lock lock(m_mutex_command);
		m_list_command.clear();
	}
	void safestart()
	{
		if(!islive())
			start();
		m_safe = true;
		m_safestart_event = (void*)CreateEvent(NULL,FALSE,FALSE,0);
		postmessage(BM_RING_START);
		::WaitForSingleObject((HANDLE)m_safestart_event,INFINITE);
		CloseHandle(m_safestart_event);
	}
	void safestop()
	{
	if(this->islive())
	{
		m_safe = false;
		m_safestop_event = (void*)CreateEvent(NULL,FALSE,FALSE,0);
		{
			boost::mutex::scoped_lock lock(m_mutex_command);
			_command::CCmdPtr cmd(new _command);
			cmd->nCmd = BM_RING_STOP;
			cmd->anyParam = 0;
			m_list_command.push_back(cmd);
		}
		DWORD dw = ::WaitForSingleObject((HANDLE)m_safestop_event,3*1000);
		if(WAIT_OBJECT_0!=dw)
		{
		}
		CloseHandle(m_safestop_event);
		stop();
	}
	}
	virtual void on_timer(const controlled_timer * p){}
	virtual void on_safestart()
	{
		SetEvent(m_safestart_event);
	}
	virtual void on_safestop()
	{
		SetEvent(m_safestop_event);
	}
	virtual void on_notify(const _notify & p)
	{
	}
protected:
	virtual boost::any on_command(const unsigned int command,const boost::any par)
	{
		return boost::any();
	}
	bool getmessage()
	{
		std::list<_command::CCmdPtr> cache;
		{
			boost::mutex::scoped_lock lock(m_mutex_command);
			while(!m_list_command.empty())
			{
				_command::CCmdPtr p = m_list_command.front();
				m_list_command.pop_front();
				cache.push_back(p);
			}
		}
		_command::CCmdPtr stop_command;
		std::list<_command::CCmdPtr>::iterator item;
		for(item = cache.begin();item!=cache.end();item++)
		{
			if((*(*item)).nCmd==BM_RING_STOP)
			{
				stop_command = *item;				
				break;
			}
		}
		if(stop_command.get()==0)
		{
			while(!cache.empty())
			{
				_command::CCmdPtr p = cache.front();
				cache.pop_front();
				try
				{
					if((*p).nCmd!=BM_RING_START)
					{
						if(!this->m_safe)
							continue;
					}
					this->message(*p);
				}
				catch(boost::bad_any_cast &)
				{
				}
			}
			return true;
		}
		else
		{
			cache.clear();
			this->message(*stop_command);
			return false;
		}
	}
private:
	void*	m_safestart_event;
	void* m_safestop_event;
	bool m_safe;//在多线程，尤其牵涉到线程之间有类似socket级别关联时，当父线程safestop以后有可能会收到其他线程的postmessage，这时会引起线程死锁，这个m_safe就是解决这个问题的，当safestop以后不再接收新消息处理
    boost::mutex m_mutex_command;
	std::list<_command::CCmdPtr> m_list_command;
};
class controlled_timer: public controlled_module_ex
{
public:
  controlled_timer()
  {
	this->m_time = 0;
	this->m_parent = 0;
	this->m_step = 0;
  }
  ~controlled_timer(){
  }
protected:
  controlled_module_ex* m_parent;
  int m_time;
  int m_step;
public:
  void starttimer(int time,controlled_module_ex* parent)
  {
	this->safestart();
	this->postmessage(BM_RING_SETPARENT,parent);
	this->postmessage(BM_RING_SETTIME,time);
  }
  void stoptimer()
  {
	this->safestop();
  }
public:
  virtual void on_safestop()
  {
	m_time = 0;
	controlled_module_ex::on_safestop();
  }
  virtual void message(const _command & cmd)
  {
	controlled_module_ex::message(cmd);
	if(cmd.nCmd==BM_RING_SETTIME)
	{
		int time = boost::any_cast<int>(cmd.anyParam);
		this->m_time = time/this->m_sleeptime;
		this->postmessage(BM_RING_CYCLE);
	}
	else if(cmd.nCmd==BM_RING_SETPARENT)
	{
		this->m_parent  = boost::any_cast<controlled_module_ex*>(cmd.anyParam);
	}	
	else if(cmd.nCmd==BM_RING_CYCLE)
	{
		if(m_time>0)
		{
			if(m_step>m_time)
			{
				m_parent->postmessage(BM_TIMER,this);
				m_step=0;
			}
			m_step++;
		}
		this->postmessage(BM_RING_CYCLE);
	} 
  }
};