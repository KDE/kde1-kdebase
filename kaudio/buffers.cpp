

ABuffer::ABuffer(uint32 samples, uint32 frags, uint8 bps)
{
  this->samples	= samples;
  this->bps	= bps;
  this->frags	= frags;
  // Set Read and write positions to 0
  WFrag		=0;
  RFrag		=0;

  Free		= frags;

  buffer	= new char[length*bps];
  if (buffer == NULL)
    cerr << "ABuffer: Buffer alloc failed\n";
}


bool ABuffer::write(unsigned char* smpl, int num)
{
  if (Free<FragSize)
    return false;

  if (FragSize+WPos > length)
    {
      // Have to wrap around at array end
      int first_num = length-WPos;
      memcpy(buffer+(WPos*bps), smpl, first_num*bps);

      if (validBytes >= FragSize)
	// Do a filter callback
	if (FilterCallback(buffer+(WPos*bps)))
	  {
	  }
	    

      WPos = 0;
      num -= first_num;
      smpl+= first_num*bps; // just for the memcpy() below
    }
  // Write the "rest" of the samples (after wrapping round)
  memcpy(buffer+(WPos*bps),smpl,num*bps);
  WPos += num;	// Set new Write position
  Free -= num;	// Reduce number of free samples slots.
  validBytes+=num;



  return true;
}



char* ABuffer16::read()
{
  if (RPos==-1)
    return -1;

  value = (buffer[RPos++]<<8)+buffer[RPos++];
  if (RPos>length)
    RPos=0;
  return value;
}

unsigned int ABuffer8::read()
{
  if (RPos==-1)
    return -1;

  value = buffer[RPos++];
  if (RPos>length)
    RPos=0;
  return value;
}

bool ABuffer::FilterCallback(void *)
{
  // Daufault callback does not do any sample processing, so return false
  return false;
}
