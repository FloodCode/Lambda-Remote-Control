#pragma once
#include "lrcdatatypes.hpp"

namespace Services
{
	namespace LRCDataHandler
	{
		void Run();
		void Stop();
		bool IsRunning();

		void Process(LRCData::ByteVector data);
	}
}