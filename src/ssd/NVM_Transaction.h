#ifndef NVM_TRANSACTION_H
#define NVM_TRANSACTION_H

#include <list>
#include "../sim/Sim_Defs.h"
#include "../sim/Engine.h"
#include "User_Request.h"

namespace SSD_Components
{
	class User_Request;
	//事务类型
	enum class Transaction_Type { READ, WRITE, ERASE, UNKOWN };
	enum class Transaction_Source_Type { 	USERIO, // 用户I/O请求
											CACHE,  // 缓存请求
											GC_WL, // 垃圾回收写入请求
											MAPPING // 地址映射请求
	};

	class NVM_Transaction
	{
	public:
		NVM_Transaction(stream_id_type stream_id, Transaction_Source_Type source, Transaction_Type type, User_Request* user_request, IO_Flow_Priority_Class::Priority priority_class) :
			Stream_id(stream_id), Source(source), Type(type), UserIORequest(user_request), Priority_class(priority_class), Issue_time(Simulator->Time()), STAT_execution_time(INVALID_TIME), STAT_transfer_time(INVALID_TIME) {}
		stream_id_type Stream_id;
		Transaction_Source_Type Source;
		Transaction_Type Type;
		User_Request* UserIORequest;
		IO_Flow_Priority_Class::Priority Priority_class;
		//std::list<NVM_Transaction*>::iterator RelatedNodeInQueue;//Just used for high performance linkedlist insertion/deletion

		sim_time_type Issue_time;
		/* Used to calculate service time and transfer time for a normal read/program operation used to respond to the host IORequests.
		In other words, these variables are not important if FlashTransactions is used for garbage collection.*/
		sim_time_type STAT_execution_time, STAT_transfer_time;
	};
}

#endif //!NVM_TRANSACTION_H
