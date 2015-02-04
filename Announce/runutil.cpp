char * Base64Encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

void Util_Base64Encode( char *cpInput, char *cpOutput )
{
int nIdx[ 4 ];  
while ( '\0' != *cpInput )
{
  nIdx[0] = ((*cpInput) & 0xFC)>>2;
  nIdx[1] = ((*cpInput) & 0x03)<<4;
  cpInput++;
  if ( '\0' != *cpInput )
  {
    nIdx[1] |= ((*cpInput) & 0xF0)>>4;
    nIdx[2]  = ((*cpInput) & 0x0F)<<2;
    cpInput++;
    if ( '\0' != (*cpInput) )
    {
      nIdx[2] |= ((*cpInput) & 0xC0) >> 6;
      nIdx[3]  = (*cpInput) & 0x3F;
      cpInput++;
    }
    else
      nIdx[3] = 64;
  }
  else
  {
    nIdx[2] = 64;
    nIdx[3] = 64;
  }

  *(cpOutput+0) = *(Base64Encoding + nIdx[0]);
  *(cpOutput+1) = *(Base64Encoding + nIdx[1]);
  *(cpOutput+2) = *(Base64Encoding + nIdx[2]);
  *(cpOutput+3) = *(Base64Encoding + nIdx[3]);
  cpOutput += 4;
}

*cpOutput = '\0';

return;
}

//----------------------------------------------------

void K_EncodeString(char *szInput, char *szOutput)
{
	char b;

	while(*szInput) {
		b = *szInput;
		_asm mov bl, b
		_asm ror bl, 3
		_asm mov b, bl
		*szOutput = b;
		szInput++;
		szOutput++;
	}
	*szOutput = 0;
}

//----------------------------------------------------

char * K_DecodeString(char *szInput)
{
	char b;
	char *st = szInput;
    
	while(*szInput) {
		b = *szInput;
		_asm mov bl, b
		_asm rol bl, 3
		_asm mov b, bl
		*szInput = b;
		szInput++;
	}

	return st;
}

//----------------------------------------------------

void FilterInvalidNickChars(char * szString)
{
	while(*szString) {
		if(*szString < ' ' || *szString > 'z') {
			*szString = '_';
		}
		szString++;
	}
}

//----------------------------------------------------

int CanFileBeOpenedForReading(char * filename)
{
	FILE *f;
	if(f=fopen(filename,"r")) {
		fclose(f);
		return 1;
	}
	return 0;
}

//----------------------------------------------------