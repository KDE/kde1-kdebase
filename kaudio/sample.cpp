#include <stdio.h>
#include <iostream>
#include <sys/stat.h>
#include "sample.h"
#include "maudio.h"
#include <string.h>
#include <stdint.h>

// Wavplay compatibility function
#ifdef DEBUG
void err(char *value)
#else
void err(char *)
#endif
{
#ifdef DEBUG
  std::cerr << value << '\n';
#endif
}

// findchunk() from wavplay
char*  findchunk (char* s1, char* s2, size_t n) ;
#define WW_BADOUTPUTFILE        1
#define WW_BADWRITEHEADER       2

#define WR_BADALLOC             3
#define WR_BADSEEK              4
#define WR_BADRIFF              5
#define WR_BADWAVE              6
#define WR_BADFORMAT            7
#define WR_BADFORMATSIZE        8

#define WR_NOTPCMFORMAT         9
#define WR_NODATACHUNK          10
#define WR_BADFORMATDATA        11

#define PCM_WAVE_FORMAT         1   // by definition of the Wav header specifications

typedef  struct
{	uint32     dwSize ;
	uint16    wFormatTag ;
	uint16    wChannels ;
	uint32     dwSamplesPerSec ;
	uint32     dwAvgBytesPerSec ;
	uint16    wBlockAlign ;
	uint16    wBitsPerSample ;
} WAVEFORMAT ;

extern int          BUFFSIZE;   // Crap! Must be out into AudioSample class

AudioSample::AudioSample()
{
  opened = false;
  clearBuffers();
}

void AudioSample::clearBuffers()
{
  setRBuf(0);
  setWBuf(0);
  buffersValid=0;
}

void AudioSample::setRBuf(int id)
{
  if (id>=NUM_BUF)
    id=0;
  RBufId = id;
  RBuffer= Buffers[id];
  //  std::cerr << "RBuf = " << id << "\n";
}

void AudioSample::setWBuf(int id)
{
  if (id>=NUM_BUF)
    id=0;
  WBufId = id;
  WBuffer= Buffers[id];
  //  std::cerr << "WBuf = " << id << "\n";
}

void AudioSample::nextWBuf()
{
  if(buffersValid>0) {
    setWBuf(WBufId+1);
    buffersValid--;
  }
  else
    std::cerr << "nextWBuf():  buffersValid==0\n";
}


int AudioSample::setFilename(char* fname)
{
  const int BUFFERSIZE = 1024;

  uint32	bytes_read;
  struct	stat statBuf;
  int		ret;
  char		head[BUFFERSIZE];
  //  WAVE_HEADER	*WavHead;

  clearBuffers(); // Be sure to clear buffers before next media is played


  if(opened) {
    fclose(audiofile);
    opened = false;
  }

  audiofile = fopen(fname, "r");
  if (!audiofile) {
    std::cerr << "maudio: Cannot open file.\n";
    return 1;
  }

  ret = stat(fname, &statBuf);
  if (ret==-1)
    {
      std::cerr << "maudio: Cannot stat file.\n";
      fclose(audiofile);
      return 1;
    }


  // Set an "approximation" of the header Length#
  // (headerLen is calculated precisely later
  headerLen = sizeof(WAVE_HEADER); // !!! Hardcoded WAV, TODO

  /* Read in audioheader */
  bytes_read = fread(head, 1, BUFFERSIZE, audiofile);

  // Probe the file
  // This is the WAV probe code
  if (bytes_read < sizeof(WAVE_HEADER))
    {
      std::cerr << "maudio: Incomplete audio header.\n";
      fclose(audiofile);
      return 1;
    }

  fseek(audiofile,headerLen,SEEK_SET);

  // Wavplay compatibility issue
  char *buffer = head;


  // Wavplay 1.0pl2 original code starts here
  static WAVEFORMAT  waveformat ;
  char*   ptr ;
  uint32  databytes ;

  if (findchunk (buffer, "RIFF", BUFFERSIZE) != buffer) {
    err("Bad format: Cannot find RIFF file marker");	/* wwg: Report error */
    return  WR_BADRIFF ;
  }

  if (! findchunk (buffer, "WAVE", BUFFERSIZE)) {
    err("Bad format: Cannot find WAVE file marker");	/* wwg: report error */
    return  WR_BADWAVE ;
  }

  ptr = findchunk (buffer, "fmt ", BUFFERSIZE) ;
  if (! ptr) {
    err("Bad format: Cannot find 'fmt' file marker");	/* wwg: report error */
    return  WR_BADFORMAT ;
  }

  ptr += 4 ;	/* Move past "fmt ".*/
  memcpy (&waveformat, ptr, sizeof (WAVEFORMAT)) ;

  if (waveformat.dwSize < (sizeof (WAVEFORMAT) - sizeof (uint32))) {
    err("Bad format: Bad fmt size");			/* wwg: report error */
    return  WR_BADFORMATSIZE ;
  }

  if (waveformat.wFormatTag != PCM_WAVE_FORMAT) {
    err("Only supports PCM wave format");			/* wwg: report error */
    return  WR_NOTPCMFORMAT ;
  }

  ptr = findchunk (buffer, "data", BUFFERSIZE) ;
  if (! ptr) {
    err("Bad format: unable to find 'data' file marker");	/* wwg: report error */
    return  WR_NODATACHUNK ;
  }

  ptr += 4 ;	/* Move past "data".*/
  memcpy (&databytes, ptr, sizeof (uint32)) ;

  /* Everything is now cool, so fill in output data.*/


  // ------------ OK, back from Wavplay code to sample.cpp again ----------------------
  MediaLength = databytes; //   databytes/waveformat.wBlockAlign;
  // OLD: statBuf.st_size - sizeof(headerLen); // !!! Must subtract crap at end */

  /* determine basic wave characteristics */
  frequency	= waveformat.dwSamplesPerSec ;
  bytes	        = waveformat.wBitsPerSample/8;
  bit_p_spl	= waveformat.wBitsPerSample ;
  channels	= waveformat.wChannels;
  stereo	= ( channels == 2 );

  bytes_per_s = (frequency * bit_p_spl)/8;
  if (stereo)
    bytes_per_s *= 2;
  headerLen     = ((intptr_t) (ptr + 4)) - ((intptr_t) (&(buffer[0]))) ;

#ifdef DEBUG
  std::cerr << fname << " is a ";
  if (stereo)
    std::cerr << "stereo";
  else
  std::cerr << "mono";
  std::cerr << " sample with " << bytes \
       << " byte(s)/sample at " << frequency << " Hz.\n";
  std::cerr << "Bit(s)/sample is " << bit_p_spl << "\n";
#endif

  if (waveformat.dwSamplesPerSec != waveformat.dwAvgBytesPerSec / waveformat.wBlockAlign) {
    err("Bad file format");			/* wwg: report error */
    return  WR_BADFORMATDATA ;
  }

  if (waveformat.dwSamplesPerSec != waveformat.dwAvgBytesPerSec / waveformat.wChannels / ((waveformat.wBitsPerSample == 16) ? 2 : 1)) {
    err("Bad file format");			/* wwg: report error */
    return  WR_BADFORMATDATA ;
  }

  // Make the buffer/fragment size small, so that latency is low.
  // But not too small that low-end equipment is able to transport
  // data to the buffer.
  if (bit_p_spl==8)
    BUFFSIZE = 256;
  else
    BUFFSIZE = 512;
  // Stereo samples need doubled buffer size
  if (stereo)
    BUFFSIZE *= 2;

  Duration    = MediaLength/bytes_per_s;

  opened = true;
  return 0;
}



uint32 AudioSample::duration()
{
  if (!opened)
    return 0;
  return Duration;
}



uint32 AudioSample::playpos()
{
  uint32 secsCurrent, cur_read_pos;

  if (!opened)
    return 0;

  /* Calculate play position (in seconds) */
  if (audiofile != NULL)
    cur_read_pos = ftell(audiofile);
  else
    cur_read_pos = 0;

  secsCurrent = cur_read_pos-headerLen;
  secsCurrent /= bytes_per_s;
  return secsCurrent;
}

void AudioSample::seek(uint32 secs, uint32 msecs)
{
  uint32 bytepos;
  if (opened) {
    bytepos = secs * bytes_per_s;
    if (msecs != 0)
      bytepos = (bytepos * 1000) / msecs; // !!! falsch, TODO
    fseek( audiofile, bytepos+headerLen, SEEK_SET);
  }
}

// This is a wrapper around readDataI() , made for implementing a simple buffering mechanism
int AudioSample::readData()
{
  if ( buffersValid >= NUM_BUF) {
    // We definitely read enough. Return a key to indicate this
    std::cerr << "maudio: Read too many buffers (OUCH!) \n";
    return 0;
  }
  int num = readDataI();
  if (num != 0) {
    // set read buffer to next buffer
    setRBuf(RBufId+1);
    buffersValid++;
  }
  return num;
}

int AudioSample::readDataI()
{
  int len, len_toRead;
  uint32 tmpReadPos, cur_read_pos;

  /* Calculate play position (in seconds) */
  if (audiofile != NULL)
    cur_read_pos = ftell(audiofile);
  else
    cur_read_pos = 0;

  tmpReadPos = cur_read_pos-headerLen;
  if (tmpReadPos+BUFFSIZE > MediaLength)
    len_toRead = MediaLength-tmpReadPos;	// End of media is here. Ignore trailing garbage
  else
    len_toRead = BUFFSIZE;			// A full buffer can be read without harm

#ifdef DEBUG
  //  std::cerr << "Trying to read " << len_toRead << " bytes from " << cur_read_pos << ". Read: ";
#endif

  if (len_toRead == 0) // -<- This ends the media
    return 0;
  len = fread(RBuffer, 1, len_toRead, audiofile);
  if (len==0)
    return 0; // -<- Just paranoia

  // Always pad with ZeroData!!!
  if (bit_p_spl == 8) {
    for (int i=len; i<BUFFSIZE; i++) {
      RBuffer[i]=0x80; // !!! Wrong padding !!!
    }
  }
  else {
    for (int i=len; i<BUFFSIZE; i++) {
      RBuffer[i]=0x00;
    }
  }

#ifdef DEBUG
  //  std::cerr << len << '\n';
#endif

  return(BUFFSIZE);
}

AudioSample::~AudioSample()
{
  if (opened)
    fclose(audiofile);
}


char* findchunk  (char* pstart, char* fourcc, size_t n)
{	char	*pend ;
	int		k, test ;

	pend = pstart + n ;

	while (pstart < pend)
	{ 	if (*pstart == *fourcc)       /* found match for first char*/
		{	test = true ;
			for (k = 1 ; fourcc [k] != 0 ; k++)
				test = (test ? ( pstart [k] == fourcc [k] ) : false) ;
			if (test)
				return  pstart ;
			} ; /* if*/
		pstart ++ ;
		} ; /* while lpstart*/

	return  NULL ;
} ; /* findchuck*/
