#ifndef SPEECHMANAGER_H
#define SPEECHMANAGER_H

#include <wx/wx.h>

namespace SpeechMan {
	bool Initialize();
	bool DeInitialize();
	bool WasBuiltIn();
	bool IsInitialized();

	void Speak(wxString what);
	void SetVoice(size_t i);
	int GetVoice();
	wxArrayString EnumVoices();
	void SetVolume(int volume);
	int GetVolume();

};

#endif