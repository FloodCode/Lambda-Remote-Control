#pragma once
#include "lrcdatatypes.hpp"
#include <vector>

namespace Services
{
	namespace WebSocket
	{
		void Run();
		void Stop();
		bool IsRunning();

		bool Send(std::vector<byte> data);
	}
}