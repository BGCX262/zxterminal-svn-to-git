// controlled_module.hpp : 可主动关闭的boost线程类
//
#pragma once

#include   <boost/utility.hpp>   
#include   <boost/thread/condition.hpp>   
#include   <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include "janitor.hpp"

class   controlled_module_implement   :   boost::noncopyable   {   
  public:   
  controlled_module_implement()   :active_(false),command_exit_(false){}   
  boost::condition   module_is_exit;   
  boost::mutex   monitor;   
  void   active(bool   ac)   
  {boost::mutex::scoped_lock   lk(monitor);   if(!(active_=ac))module_is_exit.notify_all();else   command_exit_=false;}   
  bool   command_exit(){boost::mutex::scoped_lock   lk(monitor);   return   command_exit_;}   
  bool   active_,command_exit_;   
    
  };
  class   controlled_module   :   boost::noncopyable   {   
  public:   
  virtual   void   run()
  {
	  ObjJanitor   janitor(*impl_,&controlled_module_implement::active,false);   
	impl_->active(true);   
	{   
		ObjJanitor   janitor(*this,&controlled_module::release);   
		if(this->initialize())   
		{   
			m_live = true;
			SetEvent(m_event_init);
			while(!impl_->command_exit()   &&  this->islive() &&  this->work())
			{
			}
		}
		else
		{
			m_live = false;
			SetEvent(m_event_init);
		}
	}  
  }
  bool   exit(unsigned   long   sec=0)
  {
  boost::mutex::scoped_lock   lk(impl_->monitor);   
  impl_->command_exit_   =   true;   
  while(impl_->active_)   
  {   
  if(sec)   
  {   
  boost::xtime   xt;   
  boost::xtime_get(&xt,   boost::TIME_UTC_);   
  xt.sec   +=   sec;   
  if(!impl_->module_is_exit.timed_wait(lk,xt))   
  return   false;   
  }   
  else   
  impl_->module_is_exit.wait(lk);   
  }   
  return   true;   
  }
  protected:   
  controlled_module()
  :impl_(new   controlled_module_implement)   
  ,m_live(false)
  ,m_event_init(0)
  ,m_sleeptime(10)
  {   	
  }   
  virtual   ~controlled_module()
  {
	  if(m_live)
		  stop();
	  delete   impl_;   
  }
  private:    
	  virtual   bool   initialize(){return true;} 
	  virtual   void   release(){}
  protected:
	  virtual   bool   work()
	  {
		Sleep(this->m_sleeptime);
		return true;
	  }
	  int m_sleeptime;
  private:   
	bool m_live;
 	void * m_event_init;
 controlled_module_implement*   impl_;   
  public:
	bool start()
	{
		m_event_init = CreateEvent(NULL,FALSE,FALSE,"");
		boost::thread thd(boost::bind(&controlled_module::run,this));
		::WaitForSingleObject(m_event_init,INFINITE);
		CloseHandle(m_event_init);
		m_event_init = 0;
		return m_live;
	}
	void stop()
	{
		m_live = false;
		exit(1);
	}
	bool islive(){return m_live;}
	void die()
	{
	  m_live = false;
		SetEvent(m_event_init);
	}
	void setsleeptime(int n)
	{
	  m_sleeptime = n;
	}
  };
    
