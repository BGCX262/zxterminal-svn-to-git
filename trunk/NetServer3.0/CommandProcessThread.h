#pragma once

#include "controlled_module_ex.hpp"
#include "boost/any.hpp"
#include "msg.pb.h"

class CCommandProcessThread :
	public controlled_module_ex
{
public:
	CCommandProcessThread(void);
	~CCommandProcessThread(void);

protected:
	virtual void message(const _command & cmd);

};

