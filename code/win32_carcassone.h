#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_replay_buffer
{
	HANDLE FileHandle;
	HANDLE MemoryMap;
	char FileName[WIN32_STATE_FILE_NAME_COUNT];
	void *MemoryBlock;
};

struct win32_state
{
	uint64 TotalSize;
	void *GameMemoryBlock;
	win32_replay_buffer ReplayBuffers[4]; 


	HANDLE RecordingHandle;
	int InputRecordingIndex;

	HANDLE PlayBackHandle;
	int InputPlayingIndex;


	char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
	char *OnePastLastEXEFileNameSlash;
};