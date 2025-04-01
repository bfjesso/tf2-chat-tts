#include "speach.h"
#include <sapi.h>
#include <sphelper.h>

unsigned char speak(wchar_t* str)
{
    ISpVoice* pVoice = NULL;
    ISpObjectToken* cpAudioOutToken;
    IEnumSpObjectTokens* cpEnum;
    ULONG ulCount = 0;

    if (FAILED(::CoInitialize(NULL)))
        return FALSE;

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    if (SUCCEEDED(hr))
    {
        // Enumerate the available audio output devices.
        hr = SpEnumTokens(SPCAT_AUDIOOUT, NULL, NULL, &cpEnum);
    }

    if (SUCCEEDED(hr) && cpEnum)
    {
        // Get the number of audio output devices.
        hr = cpEnum->GetCount(&ulCount);
    }

    // getting the last one
    while (SUCCEEDED(hr) && ulCount--)
    {
        if (SUCCEEDED(hr))
        {
            hr = cpEnum->Next(1, &cpAudioOutToken, NULL);
        }

        if (SUCCEEDED(hr) && cpAudioOutToken)
        {
            hr = pVoice->SetOutput(cpAudioOutToken, TRUE);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pVoice->Speak(str, 0, NULL);
        pVoice->Release();
        pVoice = NULL;
    }

    ::CoUninitialize();
    return TRUE;
}