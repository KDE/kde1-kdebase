//-*-C++-*-

/** The ABuffer class is a represantation for an audio buffer. An audio buffer holds a specific
  number of samples in a selectable number of fragments. A fragment must always be filled
  completly (or they will be padded with 0 samples). ATTENTION!!! This is a base class, one
  is not supposed to use it directly. Please use derivations of it, so you can make up your
  own FilterCallback(). This filter 
*/
class ABuffer
{
public:
  /** Create a buffer of overall size samples*frags*bps. Which effectively means, "frags"
      buffers are created, each capable of holding "samples" samples. Each sample takes
      up "bps" bytes of memory. */
  ABuffer(uint32 samples, uint32 frags, char bps);

  /** Writes "num" Samples into next free fragment of sample buffer. "num" must not be bigger
      than a fragments size (which effectively is the value "samples" from creation). On failure
      (num>samples or no free fragment) false is returned. If num<samples, the buffer is padded
      with 0 samples. */
  virtual bool write(unsigned char* smpl, int num) = 0;
  /// Returns the number of free fragments
  inline int getFree() { return Free };
  /** Returns a pointer to the next unread fragment. Will return NULL if there is no sample available */
  virtual void* read() = 0;

private:
  /// Buffer
  char *buffer;
  /// Samples per fragment.
  uint32 samples;
  /// Bytes per sample
  unit8 bps;
  /// Number of fragments. See (e.g.) OSS documentation for some documentation on frags.
  uint32 frags;

  virtual bool FilterCallback(void *);

  /// How many "unwritten"/free frags are there
  int Free;
};

class ABuffer16 : ABuffer
{
};

class ABuffer8 : ABuffer
{
};
