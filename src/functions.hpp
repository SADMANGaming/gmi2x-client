#pragma once

#include <windows.h>
#define MAX_MSGLEN 16384


#define CVAR_ARCHIVE        1   // set to cause it to be saved to vars.rc
#define CVAR_USERINFO       2   // sent to server on connect or change
#define CVAR_SERVERINFO     4   // sent in response to front end requests
#define CVAR_SYSTEMINFO     8   // these cvars will be duplicated on all clients
#define CVAR_INIT           16  // don't allow change from console at all,
#define CVAR_LATCH          32  // will only change when C code next does

#define CVAR_ROM            64  // display only, cannot be set by user at all
#define CVAR_USER_CREATED   128 // created by a set command
#define CVAR_TEMP           256 // can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT          512 // can not be changed if cheats are disabled
#define CVAR_NORESTART      1024    // do not clear when a cvar_restart is issued
#define CVAR_WOLFINFO       2048    // DHM - NERVE :: Like userinfo, but for wolf multiplayer info

#define CVAR_UNSAFE         4096    // ydnar: unsafe system cvars (renderer, sound settings, anything that might cause a crash)
#define CVAR_SERVERINFO_NOUPDATE        8192    // gordon: WONT automatically send this to clients, but server browsers will see it


#define MAX_STRING_CHARS    1024 // max length of a string passed to Cmd_TokenizeString
#define MAX_STRING_TOKENS   256 // max tokens resulting from Cmd_TokenizeString
#define MAX_RELIABLE_COMMANDS 64
#define MAX_INFO_STRING     1024
#define MAX_INFO_KEY        1024
#define MAX_INFO_VALUE      1024

#define BIG_INFO_STRING     8192 // used for system info key only
#define BIG_INFO_KEY        8192
#define BIG_INFO_VALUE      8192

#define Q_IsColorString( p )  ( p && *( p ) == Q_COLOR_ESCAPE && *( ( p ) + 1 ) && *( ( p ) + 1 ) != Q_COLOR_ESCAPE )

#define Q_COLOR_ESCAPE  '^'
#define Q_IsColorString( p )  ( p && *( p ) == Q_COLOR_ESCAPE && *( ( p ) + 1 ) && *( ( p ) + 1 ) != Q_COLOR_ESCAPE )

#define DotProduct( x,y )         ( ( x )[0] * ( y )[0] + ( x )[1] * ( y )[1] + ( x )[2] * ( y )[2] )
#define VectorCopy( a,b )         ( ( b )[0] = ( a )[0],( b )[1] = ( a )[1],( b )[2] = ( a )[2] )

#define cls_realtime ((int*)0x4AD3E80) // 0x155F3E0)
#define cls_state ((int*)0x4AD3D60)
#define clc_stringData ((PCHAR)0x495E6F4)
#define clc_stringOffsets ((PINT)0x495C6F4)
#define clc_demoplaying ((PINT)0x4DE960C)
#define cls_numglobalservers ((int*)0x4AD8AA8)
#define cls_pingUpdateSource ((int*)0x4DC39B4)

#define cs0 (clc_stringData + clc_stringOffsets[0])
#define cs1 (clc_stringData + clc_stringOffsets[1])



#define MAX_QPATH 64
#define MAX_OSPATH 256
typedef int fileHandle_t;

typedef enum
{
	qfalse,
	qtrue
} qboolean;

typedef enum {
	ET_GENERAL,
	ET_PLAYER,
	ET_CORPSE,
	ET_ITEM,
	ET_MISSILE,
	ET_MOVER,
	ET_PORTAL,
	ET_INVISIBLE,
	ET_SCRIPTMOVER,
	ET_TURRET //???
} entityTypes;

typedef struct {
	int unknown; //0
	byte* data; //4
	int maxsize; //8 (most likely maxsize value 16384)
	int cursize; //12 //value 129
	int readcount; //16 //value 16
	int bit; //20 //value 0
} msg_t; //size is 0x18 (24)

typedef enum {
	STEREO_CENTER,
	STEREO_LEFT,
	STEREO_RIGHT
} stereoFrame_t;

typedef enum {
	CA_UNINITIALIZED,
	CA_DISCONNECTED = 0,
	CA_CONNECTING,
	CA_CHALLENGING,
	CA_CONNECTED,
} connstate_t;

#if 0
typedef enum {
	CA_UNINITIALIZED,
	CA_DISCONNECTED,    // not talking to a server
	CA_AUTHORIZING,     // not used any more, was checking cd key
	CA_CONNECTING,      // sending request packets to the server
	CA_CHALLENGING,     // sending challenge packets to the server
	CA_CONNECTED,       // netchan_t established, getting gamestate
	CA_LOADING,         // only during cgame initialization, never during main loop
	CA_PRIMED,          // got gamestate, waiting for first frame
	CA_ACTIVE,          // game views should be displayed
	CA_CINEMATIC        // playing a cinematic or a static pic, not connected to a server
} connstate_t;
#endif


typedef enum {
	NA_BOT,
	NA_BAD,                 // an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX
} netadrtype_t;

typedef enum {
	NS_CLIENT,
	NS_SERVER
} netsrc_t;


typedef struct
{
	netadrtype_t type;
	union
	{
		int _ip;
		unsigned char ip[4];
	};
	unsigned char ipx[10];
	unsigned short port;
} netadr_t;

typedef enum
{
	ERR_FATAL,				// exit the entire game with a popup window
	//ERR_VID_FATAL,		// exit the entire game with a popup window and doesn't delete profile.pid
	ERR_DROP,				// print to console and disconnect from game
	ERR_SERVERDISCONNECT,	// don't kill server
	ERR_DISCONNECT,			// client disconnected from the server
	ERR_NEED_CD,			// pop up the need-cd dialog
	ERR_AUTOUPDATE
} errorParm_t;


typedef struct {
	netsrc_t	sock;

	int			dropped;			// between last packet and previous

	netadr_t	remoteAddress;
	int			qport;				// qport value to write when transmitting

									// sequencing variables
	int			incomingSequence;
	int			outgoingSequence;

	// incoming fragment assembly buffer
	int			fragmentSequence;
	int			fragmentLength;
	byte		fragmentBuffer[MAX_MSGLEN];

	// outgoing fragment buffer
	// we need to space out the sending of large fragmented messages
	qboolean	unsentFragments;
	int			unsentFragmentStart;
	int			unsentLength;
	byte		unsentBuffer[MAX_MSGLEN];
} netchan_t;

typedef struct {

	int			clientNum;
	int			lastPacketSentTime;			// for retransmits during connection
	int			lastPacketTime;				// for timeouts

	netadr_t	serverAddress;
	int			connectTime;				// for connection retransmits
	int			connectPacketCount;			// for display on connection dialog
	char		serverMessage[MAX_STRING_TOKENS];	// for display on connection dialog

	int			challenge;					// from the server to use for connecting
	int			checksumFeed;				// from the server for checksum calculations

											// these are our reliable messages that go to the server
	int			reliableSequence;
	int			reliableAcknowledge;		// the last one the server has executed
	char		reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];

	// server message (unreliable) and command (reliable) sequence
	// numbers are NOT cleared at level changes, but continue to
	// increase as long as the connection is valid

	// message sequence is used by both the network layer and the
	// delta compression layer
	int			serverMessageSequence;

	// reliable messages received from server
	int			serverCommandSequence;
	int			lastExecutedServerCommand;		// last server command grabbed or executed with CL_GetServerCommand
	char		serverCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];

	// file transfer from server
	fileHandle_t download;
	char		downloadTempName[MAX_OSPATH];
	char		downloadName[MAX_OSPATH];
	int			downloadNumber;
	int			downloadBlock;	// block we are waiting for
	int			downloadCount;	// how many bytes we got
	int			downloadSize;	// how many bytes we got
	char		downloadList[MAX_INFO_STRING]; // list of paks we need to download
	qboolean	downloadRestart;	// if true, we need to do another FS_Restart because we downloaded a pak

									// demo information
	char		demoName[MAX_QPATH];
	qboolean	spDemoRecording;
	qboolean	demorecording;
	qboolean	demoplaying;
	qboolean	demowaiting;	// don't record until a non-delta message is received
	qboolean	firstDemoFrameSkipped;
	fileHandle_t	demofile;

	int			timeDemoFrames;		// counter of rendered frames
	int			timeDemoStart;		// cls.realtime before first frame
	int			timeDemoBaseTime;	// each frame will be at this time + frameNum * 50

									// big stuff at end of structure so most offsets are 15 bits or less
	netchan_t	netchan;
} clientConnection_t;

typedef struct cvar_s
{
	char* name;
	char* string;
	char* resetString; // cvar_restart will reset to this value
	char* latchedString; // for CVAR_LATCH vars
	int flags;
	qboolean modified; // set each time the cvar is changed
	int modificationCount; // incremented each time the cvar is changed
	float value; // atof( string )
	int integer; // atoi( string )
	struct cvar_s* next;
	struct cvar_s* hashNext;
} cvar_t;

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];

typedef void(*Cvar_Set_t)(const char*, const char*);
typedef cvar_t* (*Cvar_Get_t)(const char*, const char*, int);

typedef cvar_t* (*Cvar_FindVar_t)(const char*);

extern Cvar_Set_t Cvar_Set;
extern Cvar_Get_t Cvar_Get;

extern Cvar_FindVar_t Cvar_FindVar;



char* Cvar_VariableString(const char*);
int Cvar_VariableIntegerValue(const char* var_name);
float Cvar_VariableValue(const char *var_name);
void __cdecl Com_sprintf(char* dest, int size, const char* fmt, ...);
void Info_RemoveKey(char* s, const char* key);
void Info_RemoveKey_Big(char* s, const char* key);
void Info_SetValueForKey(char *s, const char *key, const char *value);
const char* Info_ValueForKey(const char* s, const char* key); //FIXME: overflow check?
int Q_stricmpn(const char* s1, const char* s2, int n);
int Q_stricmp(const char* s1, const char* s2);

extern DWORD uo_cgame_mp;
extern DWORD uo_game_mp;

#define GAME_OFF(x) (uo_game_mp + (x - 0x20000000))
#define CGAME_OFF(x) (uo_cgame_mp + (x - 0x30000000))


typedef void(*Com_Printf_t)(const char*, ...);
extern Com_Printf_t Com_Printf;
typedef void(*Com_DPrintf_t)(const char*, ...);
extern Com_DPrintf_t Com_DPrintf;


typedef void(*Com_Error_t)(int, const char*, ...);
extern Com_Error_t Com_Error;

typedef void(*xfunc)(void);

typedef void(*Cmd_AddCommand_t)(const char*, xfunc);
extern Cmd_AddCommand_t Cmd_AddCommand;

char* va(const char* format, ...);

void Q_strncpyz(char *dest, const char *src, int destsize);
char* Q_CleanStr(char* string, bool colors = false);
char* Com_CleanHostname(char* hostname, bool colors);
char* Com_CleanMapname(char* mapname);
const char* Com_GametypeName(char* gt, bool colors = false);