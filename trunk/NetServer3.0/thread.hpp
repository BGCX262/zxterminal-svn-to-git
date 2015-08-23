// thread.hpp : controlled_module_ex类的扩展
// 增强线程事务处理能力
#pragma once
#include "controlled_module_ex.hpp"
  class thread: public controlled_module_ex
  {
	protected:
		static const int NONE = -1;
		static const int WAITING =-2;
		static const int DONE =-3;
		static const int FAILED =-4;
	protected:
		struct process
		{
			int level;
			int status;
			int sequence;
			int trycount;
			int tryindex;
			std::string lasterror;
			double timeout;
			bool bTimeout;
		};
		process m_process;
		controlled_timer m_timer_process;
		int m_process_begin,m_process_end;
		double m_timeout_default;
	public:
		void startprocess(int process_begin,int process_end,double timeout_default=1.0,int cycle=1000)
		{
			m_process_begin = process_begin;
			m_process_end = process_end;
			m_timeout_default = timeout_default;
			m_process.level = m_process_begin;
			m_process.tryindex = 0;
			this->postmessage(BM_RING_PROCESS);
			m_timer_process.starttimer(cycle,this);
		}
		void tryagain()
		{
			if(this->m_process.level==thread::NONE)
				return;
			this->m_process.tryindex++;
			if(this->m_process.trycount>0 && this->m_process.tryindex>=this->m_process.trycount)
			{
				this->fail();
			}
			else
				this->postmessage(BM_RING_PROCESS);
		}
		void next()
		{
			if(this->m_process.level==thread::NONE)
				return;
			if(this->m_process.level>=this->m_process_end)
			{
				this->m_timer_process.stoptimer();
				this->postmessage(BM_RING_PROCESSEND);
			}
			else
			{
				this->m_process.tryindex = 0;
				this->m_process.level++;
				this->m_process.bTimeout = false;
				this->postmessage(BM_RING_PROCESS);
			}
		}
		void fail()
		{
			m_process.level = thread::NONE;
			this->m_timer_process.stoptimer();
			this->postmessage(BM_RING_PROCESSFAIL);
		}
		virtual void on_safestart()
		{
			m_process.level = thread::NONE;
			m_process.status = thread::NONE;
			m_process_begin = m_process_end = thread::NONE;
			controlled_module_ex::on_safestart();
		}
		virtual void on_safestop()
		{
			m_timer_process.stoptimer();
			controlled_module_ex::on_safestop();
		}
		virtual void message(const _command & cmd)
		{
			controlled_module_ex::message(cmd);
			if(cmd.nCmd==BM_RING_PROCESS)
			{
				this->on_process();
			}
			if(cmd.nCmd==BM_RING_PROCESSEND)
			{
				this->m_process.level = thread::NONE;
				this->on_process_end();
			}
			if(cmd.nCmd==BM_RING_PROCESSFAIL)
			{
				this->m_process.level = thread::NONE;
				this->on_process_fail();
			}
		}
		virtual void on_timer(const controlled_timer * p)
		{
			if(p==&this->m_timer_process)
			{
				if(this->m_process.level!=thread::NONE)
				{
					if(this->m_process.level>=this->m_process_begin && this->m_process.level<=this->m_process_end)
					{
						if(this->m_process.status==thread::NONE)
						{
							this->m_process.level = this->m_process_begin;
							m_process.tryindex = 0;
							on_process();
						}
						else if(this->m_process.status==thread::WAITING)
						{
							if(this->m_process.timeout>0)
							{
								time_t cur;
								time(&cur);
								if(difftime(cur,(time_t)this->m_process.sequence)>this->m_process.timeout)
								{
									this->m_process.bTimeout = true;
									this->tryagain();
								}
							}
						}
						else if(this->m_process.status==thread::FAILED)
						{
							this->tryagain();
						}
						else if(this->m_process.status==thread::DONE)
						{
							this->m_process.level++;
							m_process.tryindex = 0;
							this->on_process();
						}
					}
				}
			}
		}
		virtual void on_process()
		{
			time((time_t*)&m_process.sequence);
			m_process.timeout = m_timeout_default;
			m_process.status = thread::WAITING;
			m_process.trycount = -1;
		}
		virtual void on_process_end(){}
		virtual void on_process_fail(){}
		int get_sequence(){return m_process.sequence;}
		void put_timeout(double v){m_process.timeout = v;}
		void put_trycount(int v){m_process.trycount = v;}
		int get_level(){return m_process.level;}
		void put_level(int v){m_process.level=v;}
		std::string get_lasterror(){return m_process.lasterror;}
		void put_lasterror(std::string v){m_process.lasterror=v;}
		__declspec(property(put=put_trycount)) int trycount;
		__declspec(property(put=put_timeout)) double timeout;
		__declspec(property(get=get_level,put=put_level)) int level;
		__declspec(property(get=get_sequence)) int sequence;
		__declspec(property(get=get_lasterror,put=put_lasterror)) std::string lasterror;
  };
