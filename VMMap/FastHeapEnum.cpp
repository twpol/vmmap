//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "FastHeapEnum.h"


BOOL GetFirstHeapBlock( PDEBUG_HEAP_INFORMATION curHeapNode, HeapBlock *hb)
{
  ULONG_PTR *block;

  hb->reserved = 0;
  hb->dwAddress = 0;
  hb->dwFlags = 0;

  block = (ULONG_PTR*) curHeapNode->Blocks;

  while( ( *(block+1) & 2) == 2)
  {
    hb->reserved++;
    hb->dwAddress = (void *) ( *(block+3) + curHeapNode->Granularity );
    block = block+4;
    hb->dwSize = *block;
  }

  DWORD flags = *(block+1);

  if( ( flags & 0xF1 ) != 0 || ( flags & 0x0200 ) != 0 )
    hb->dwFlags = LF32_FIXED;
  else if( (flags & 0x20) != 0 )
    hb->dwFlags = LF32_MOVEABLE;
  else if( (flags & 0x0100) != 0 )
    hb->dwFlags = LF32_FREE;

   return TRUE;
}



BOOL GetNextHeapBlock( PDEBUG_HEAP_INFORMATION curHeapNode, HeapBlock *hb)
{
  ULONG_PTR *block;

  hb->reserved++;

  // If all the blocks have been enumerated....exit
  if (hb->reserved > curHeapNode->BlockCount)
	  return FALSE;

  block = (ULONG_PTR*) curHeapNode->Blocks;

  block = block + hb->reserved * 4;

  if( ( *(block+1) & 2) == 2)
  {
    do{
      hb->dwAddress = (void *) ( *(block+3) + curHeapNode->Granularity );

	  hb->reserved++;

      if( hb->reserved > curHeapNode->BlockCount)
        return FALSE;

      block = block + 4; 
      hb->dwSize = *block;
     }while( ( *(block+1) & 2) == 2);
  }
  else
  {
    hb->dwAddress = (void*) ( (ULONG_PTR)hb->dwAddress + hb->dwSize );
    hb->dwSize = *block;
  }

  DWORD flags = *( block+1);

  if( ( flags & 0xF1 ) != 0 || ( flags & 0x0200 ) != 0 )
	  hb->dwFlags = LF32_FIXED;
  else if( (flags & 0x20) != 0 )
	  hb->dwFlags = LF32_MOVEABLE;
  else if( (flags & 0x0100) != 0 )
	  hb->dwFlags = LF32_FREE;

  return TRUE;
}
