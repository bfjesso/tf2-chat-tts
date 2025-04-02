#include "speach.h"
#include <sapi.h>
#include <sphelper.h>

unsigned char speak(wchar_t* str, unsigned char isMale, int pitch)
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
        ISpObjectToken* cpToken(NULL);

        if (isMale) { SpFindBestToken(SPCAT_VOICES, L"gender=male", L"", &cpToken); }
        else { SpFindBestToken(SPCAT_VOICES, L"gender=female", L"", &cpToken); }
        
        pVoice->SetVoice(cpToken);
        cpToken->Release();

        char pitchModifier[50];
        sprintf(pitchModifier, "<pitch middle = '%d'/>", pitch);
        
        wchar_t wc[255];
        mbstowcs(wc, pitchModifier, 50);
        memcpy(wc + wcslen(wc), str, wcslen(str) * sizeof(wchar_t));

        hr = pVoice->Speak(wc, 0, NULL);
        pVoice->Release();
        pVoice = NULL;
    }

    ::CoUninitialize();
    return TRUE;
}