#ifndef HOST_IO_REQUEST_H
#define HOST_IO_REQUEST_H

#include "../ssd/SSD_Defs.h"

namespace Host_Components
{
	enum class Host_IO_Request_Type { READ, WRITE };
	// request请求模拟，只有两种类型，读和写，再加上起始地址和逻辑块数量共同构成一次请求
	class Host_IO_Request
	{
	public:
		sim_time_type Arrival_time;//The time that the request has been generated
		sim_time_type Enqueue_time;//The time that the request enqueued into the I/O queue
		LHA_type Start_LBA;
		unsigned int LBA_count;
		Host_IO_Request_Type Type;
		uint16_t IO_queue_info;
		uint16_t Source_flow_id;//Only used in SATA host interface
	};
}

#endif // !HOST_IO_REQUEST_H
