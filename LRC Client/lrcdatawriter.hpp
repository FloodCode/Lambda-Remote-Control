#ifndef LRCDATA_LRCDATAWRITER
#define LRCDATA_LRCDATAWRITER
#include "binarywriter.hpp"
#include "lrcdatatypes.hpp"
#include <string>
#include <list>

namespace LRCData
{
	class LRCDataWriter
	{
	public:
		LRCDataWriter(std::string id);
		bool WriteData(std::string filename, std::vector<Keyboard> data);
		bool WriteData(std::string filename, std::vector<Clipboard> data);
		ByteVector GetBytes(std::vector<Keyboard> data);
		ByteVector GetBytes(std::vector<Clipboard> data);
	private:
		Header header;
		ByteVector getHeaderBytes();
		void initHeader();
	};
}

#endif // !LRCDATA_LRCDATAWRITER
