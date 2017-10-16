#include "main.h"
#include "AppDelegate.h"
#include "cocos2d.h"
#include <Windows.h>
#include <Mmsystem.h>
#include "wavpack\wavpack.h"
#include "SoundTouch\include\SoundTouch.h"
#include "NativeDefs.h"

USING_NS_CC;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // create the application instance
    AppDelegate app;
    return Application::getInstance()->run();
}


// RECORDING

HWAVEIN wavein = 0;
HWAVEOUT waveout = 0;
WAVEHDR wihdr;
short rec_data_buf[160000]; // 10 sec buffer * 16KHz * 16bit
int recorded = 0;
static bool stopping = false;

void CALLBACK _rec_callback(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	if (uMsg == WIM_DATA && !stopping) waveInStop(hwi);
}

int native_stopAudioRec() {
	if (!wavein) return 0;
	stopping = true;
	waveInStop(wavein);
	waveInUnprepareHeader(wavein, &wihdr, sizeof(wihdr));
	waveInClose(wavein);
	wavein = 0;
	stopping = false;
	return (recorded = wihdr.dwBytesRecorded / 2);
	//change_pitch((short*)rec_data_buf, wihdr.dwBytesRecorded / 2, 0, -7.0);
	//pack_buffer(rec_data_buf, wihdr.dwBytesRecorded / 2, "c:\\w\\output.wv");
}

bool native_startAudioRec()
{
	if (wavein) native_stopAudioRec();
	if (waveout) native_stopPlay();

	WAVEFORMATEX wf = { 0 };
	wf.cbSize = sizeof(wf);
	wf.nChannels = 1;
	wf.nSamplesPerSec = 16000;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.wBitsPerSample = 16;
	wf.nBlockAlign = 2;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;

	recorded = 0;
	if (waveInOpen(&wavein, WAVE_MAPPER, &wf, (DWORD_PTR)_rec_callback, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
		return false;

	wihdr.dwFlags = 0;
	wihdr.lpData = (LPSTR)rec_data_buf;
	wihdr.dwBufferLength = sizeof(rec_data_buf);
	waveInPrepareHeader(wavein, &wihdr, sizeof(wihdr));
	waveInAddBuffer(wavein, &wihdr, sizeof(wihdr));
	waveInStart(wavein);
	return true;
}

bool native_getRecBuffer(short *buf, int size)
{
	if (!recorded) return false;
	if (size > recorded) size = recorded;
	memcpy(buf, rec_data_buf, recorded * 2);
	return true;
}

void native_stopPlay()
{
	if (!waveout) return;
	if (stopping) return;
	stopping = true;
	waveOutReset(waveout);
	waveOutUnprepareHeader(waveout, &wihdr, sizeof(wihdr));
	waveOutClose(waveout);
	stopping = false;
	waveout = 0;
}

DWORD WINAPI stop_play_thread(LPVOID lpParameter)
{
	native_stopPlay();
	return 0;
}

void CALLBACK _play_callback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	if (uMsg == WOM_DONE && !stopping) {
		CreateThread(0, 0, stop_play_thread, 0, 0, 0);
	}
}

bool native_playBuffer(short *buf, int size)
{
	if (wavein) native_stopAudioRec();
	if (waveout) native_stopPlay();

	WAVEFORMATEX wf = { 0 };
	wf.cbSize = sizeof(wf);
	wf.nChannels = 1;
	wf.nSamplesPerSec = 16000;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.wBitsPerSample = 16;
	wf.nBlockAlign = 2;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;

	if(waveOutOpen(&waveout, WAVE_MAPPER, &wf, (DWORD_PTR)_play_callback, 0, CALLBACK_FUNCTION | WAVE_ALLOWSYNC) != MMSYSERR_NOERROR)
		return false;

	size *= 2;
	if (size > sizeof(rec_data_buf)) size = sizeof(rec_data_buf);
	if (buf)
		memcpy(rec_data_buf, buf, size);

	wihdr.dwFlags = 0;
	wihdr.lpData = (LPSTR)rec_data_buf;
	wihdr.dwBufferLength = size;

	waveOutPrepareHeader(waveout, &wihdr, sizeof(wihdr));
	waveOutWrite(waveout, &wihdr, sizeof(wihdr));
	return true;
}


static int numtries = 0;

void native_startDnsSD()
{
	numtries = 0;
}

int native_getDnsSDdiscoveryNum()
{
	//return ++numtries / 8;
	//return 0;   //Forces Demo mode
	return 1;
}

void native_getDnsSDdiscoveryItem(int i, char *buf, int bsize)
{
	//buf[0] = 0;
	if (i == 0) strcpy(buf, "192.168.0.107");
	//else if (i == 1) strcpy(buf, "192.168.4.1");
}


void native_stopDnsSD()
{

}

void native_startSmartConfig(const char *ssid, const char *pwd)
{
	// unimplemented for win32
}

void native_stopSmartConfig()
{
	// unimplemented for win32
}

void native_getSSID(char *buf, unsigned bsize)
{
	strncpy(buf, "IoTarium", bsize);
	buf[bsize - 1] = 0;
}
