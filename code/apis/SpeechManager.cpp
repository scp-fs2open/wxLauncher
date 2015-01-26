/*
Copyright (C) 2009-2010 wxLauncher Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <wx/wx.h>
#include "generated/configure_launcher.h"
#if USE_SPEECH
#include <sapi.h>
#endif

#include <vector>

#include "apis/SpeechManager.h"

#include "global/MemoryDebugging.h"

using namespace SpeechMan;

#if USE_SPEECH
// wxLauncher objects
bool isInitialized = false;

class VoiceData {
public:
	VoiceData(const wxString& voiceName, ISpObjectToken* voice);
	VoiceData::VoiceData(const VoiceData& vd);
	VoiceData& VoiceData::operator=(const VoiceData& vd);
	const wxString& GetVoiceName() const { return this->voiceName; }
	ISpObjectToken* GetVoice() const { return this->voice; }
private:
	VoiceData();
	wxString voiceName;
	ISpObjectToken* voice;
};

VoiceData::VoiceData(const wxString& voiceName, ISpObjectToken* voice)
: voiceName(voiceName), voice(voice) {
}

VoiceData::VoiceData(const VoiceData& vd)
: voiceName(vd.voiceName), voice(vd.voice) {
}

VoiceData& VoiceData::operator=(const VoiceData& vd) {
	if (this == &vd) {
		return *this;
	}

	this->voiceName = vd.voiceName;
	this->voice = vd.voice;
	return *this;
}

std::vector<VoiceData> voices;

// COM objects
ISpVoice * comVoice = NULL;

void enumerateObjectToken(ISpObjectToken * token) {
	HRESULT valuesres = S_OK;
	HRESULT datares = S_OK;
	wxLogDebug(wxT_2("Enumerating token:"));
	ULONG valueIndex = 0;
	LPWSTR value = NULL;
	LPWSTR valuedata = NULL;
	do {
		valuesres = token->EnumValues(valueIndex, &value);
		if ( valuesres == S_OK ) {
			datares = token->GetStringValue(value, &valuedata);
			if ( datares == S_OK ) {
				wxLogDebug(wxT_2(" %s=%s"), wxString(value, wxMBConvUTF16()).c_str(),
					wxString(valuedata, wxMBConvUTF16()).c_str());
				CoTaskMemFree(valuedata);
				valuedata = NULL;
			} else {
				wxLogDebug(wxT_2(" %s= (NONE)"), wxString(value, wxMBConvUTF16()).c_str());
			}
			CoTaskMemFree(value);
			value = NULL;
		}
		valueIndex++;
	} while( SUCCEEDED(valuesres) );
}

bool EnumerateVoices() {
	HRESULT comResult = S_OK;
	ISpObjectTokenCategory * comTokenCategory = NULL;
	IEnumSpObjectTokens * comVoices = NULL;
	ULONG comVoicesCount = 0;
	
	// Init speech api
	comResult = ::CoCreateInstance(
		CLSID_SpVoice, NULL, CLSCTX_INPROC_SERVER, IID_ISpVoice, (LPVOID*)&comVoice);
	wxCHECK_MSG( SUCCEEDED(comResult), false, _T("Unable to instantiate speech API"));

	// Generate enumeration of voices
	comResult = ::CoCreateInstance(CLSID_SpObjectTokenCategory, NULL,
		CLSCTX_INPROC_SERVER, IID_ISpObjectTokenCategory, (LPVOID*)&comTokenCategory);
	wxCHECK_MSG( SUCCEEDED(comResult), false, _T("Unable to instantiate a TokenCategory"));

	comResult = comTokenCategory->SetId(SPCAT_VOICES, false);
	wxCHECK_MSG( SUCCEEDED(comResult), false,
		_T("Unable to to set location to find the installed voices.")
		_T("Likely the key that I am looking for does not exist and thus ")
		_T("there are no installed voices on this system."));

	comResult = comTokenCategory->EnumTokens(NULL, NULL, &comVoices);
	wxCHECK_MSG( SUCCEEDED(comResult), false,
		_T("Unable to enumerate the installed voices.  Check that this system")
		_T(" has voices installed."));

	comResult = comVoices->GetCount(&comVoicesCount);
	wxCHECK_MSG( SUCCEEDED(comResult), false,
		_T("Unable get a count of the installed voices."));

	while( comVoicesCount > 0 ) {
		ISpObjectToken * comAVoice = NULL;
		comVoices->Next(1, &comAVoice, NULL); // retrieve just one
		LPWSTR id = NULL;
		comAVoice->GetStringValue(NULL, &id);
		size_t idlength = wcslen(id);
		wxLogDebug(_T("  Got string of length %d:"), idlength);
		for(size_t i = 0; i < idlength; i++) {
			wxLogDebug(_T("   %04X"), id[i]);
		}
		voices.push_back(VoiceData(wxString(id, wxMBConvUTF16(), wcslen(id)), comAVoice));

#ifdef __WXDEBUG__
		enumerateObjectToken(comAVoice);
#endif
		comAVoice->Release();
		comVoicesCount--;
	}

	comTokenCategory->Release();

	return true;
}

#endif // USE_SPEECH

bool SpeechMan::Initialize() {
#if USE_SPEECH
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	wxCHECK_MSG(EnumerateVoices(), false, _T("Cannot enumerate voices"));

	isInitialized = true;
	return true;
#else
	return false;
#endif
}

bool SpeechMan::DeInitialize() {
#if USE_SPEECH
	isInitialized = false;
	::CoUninitialize();
	return true;
#else
	return false;
#endif
}
/** Returns true if Speech Support was compiled in.  Returns false if speech 
support was not compiled in. */
bool SpeechMan::WasBuiltIn() {
#if USE_SPEECH
	return true;
#else
	return false;
#endif
}

bool SpeechMan::IsInitialized() {
#if USE_SPEECH
	return isInitialized;
#else
	return false;
#endif
}

wxArrayString SpeechMan::EnumVoices() {
	wxArrayString arr;
#if USE_SPEECH
	for (std::vector<VoiceData>::const_iterator it = voices.begin();
		it != voices.end(); ++it) {
			arr.Add(it->GetVoiceName());
	}
#endif
	return arr;  // reference counted copy
}

void SpeechMan::Speak(wxString what) {
#if USE_SPEECH
	wxCHECK_RET( isInitialized, _T("Speak called but SpeechMan not initialized."));
	wxCHECK_RET( comVoice != NULL, _T("Speak called but comVoice is null"));
	wxWritableWCharBuffer buffer(what.wchar_str());
	LPCWSTR str = buffer.operator const wchar_t *();

	HRESULT result = comVoice->Speak(str, SPF_IS_NOT_XML, NULL);
	wxCHECK_RET( SUCCEEDED(result), _T("Failed to speak"));
#else
	wxFAIL_MSG(_T("Speak called but USE_SPEECH not defined"));
#endif
}

#if USE_SPEECH
void SpeechMan::SetVoice(size_t i) {
	wxCHECK_RET( isInitialized, _T("SetVoice called but SpeechMan not initialized."));
	wxCHECK_RET( comVoice != NULL, _T("SetVoice called but comVoice is null"));
	wxCHECK_RET( i < voices.size(), _T("i is larger than the number of voices"));

	wxLogDebug(_T("Selected voice: %s"), voices[i].GetVoiceName().c_str());

	HRESULT result = comVoice->SetVoice(voices[i].GetVoice());
	wxCHECK_RET( SUCCEEDED(result), _T("Error in setting voice"));
#else
void SpeechMan::SetVoice(size_t) {
	wxFAIL_MSG(_T("SetVoice called but USE_SPEECH not defined"));
#endif
}

/** Returns the currently set voice.  Useful mostly get get the system default
voice if there is no value for the voice set by the profile, or the FS2_open
registry entries. */
int SpeechMan::GetVoice() {
#if USE_SPEECH
	wxCHECK_MSG( isInitialized, -1, _T("GetVoice called but SpeechMan not initialized."));
	wxCHECK_MSG( comVoice != NULL, -1, _T("GetVoice called but comVoice is null"));
	ISpObjectToken * voice = NULL;

	HRESULT result = comVoice->GetVoice(&voice);
	wxCHECK_MSG( SUCCEEDED(result), -1, _T("Error in getting voice"));

	LPWSTR name = NULL;
	result = voice->GetStringValue(NULL, &name);
	wxCHECK_MSG( SUCCEEDED(result), -1, _T("Error in getting voice name"));

	wxString voiceName(name, wxMBConvUTF16());
	int index = EnumVoices().Index(voiceName.c_str());
	wxCHECK_MSG( index != wxNOT_FOUND, -1, 
		wxString::Format(_T("Could not find voice (%s) in database"), voiceName));

	return index;
#else
	wxFAIL_MSG(_T("GetVoice called but USE_SPEECH not defined"));
	return -1;
#endif
}

int SpeechMan::GetVolume() {
#if USE_SPEECH
	wxCHECK_MSG( isInitialized, -1, _T("GetVolume called but SpeechMan not initialized."));
	wxCHECK_MSG( comVoice != NULL, -1, _T("GetVolume called but comVoice is null"));

	USHORT volume = 100;
	HRESULT result = comVoice->GetVolume(&volume);
	wxCHECK_MSG( SUCCEEDED(result), -1, _T("Unable to retrieve volume"));

	return volume;
#else
	wxFAIL_MSG(_T("GetVolume called but USE_SPEECH not defined"));
	return 0;
#endif
}

#if USE_SPEECH
void SpeechMan::SetVolume(int volume) {
	wxCHECK_RET( isInitialized, _T("SetVolume called but SpeechMan not initialized."));
	wxCHECK_RET( comVoice != NULL, _T("SetVolume called but comVoice is null"));
	wxCHECK_RET( volume >= 0, _T("Volume is less than zero"));
	wxCHECK_RET( volume <= 100, _T("Volume is greater than 100"));

	/* volume must be within 0-100 which is well within USHORT's range.*/
	USHORT v = static_cast<USHORT>(volume); 
	HRESULT result = comVoice->SetVolume(v);
	wxCHECK_RET( SUCCEEDED(result), _T("Unable to set volume"));
#else
void SpeechMan::SetVolume(int) {
	wxFAIL_MSG(_T("SetVolume called but USE_SPEECH not defined"));
#endif
}


