//
// debugging new
//
// (C) by Martin Jones and Daniel Bell 1995
//

#ifdef DEBUGNEW

#include <stdio.h>
#include "dbnew.h"

#undef new
#undef delete

// ============================================================================

struct sMem
{
	char *Ptr;
	const char *Filename;
	int Line;
	char Flags;
	size_t Size;
	sMem *Next;
	sMem *Prev;
};


// ============================================================================

void *GetMem(size_t Size, const char *Filename, int Line, char Flag = 0);
void ReleaseMem(void *Ptr, char Flag = 0);
void CheckMem(sMem *Mem, char Flag = -1);
void Append(sMem *Mem);
void Insert(sMem *Mem);
void Remove();
void PrintMem(sMem *Mem);
void PrintWhere();

// ============================================================================

static FILE *Out = stderr;
static int enable = 0;
int DBNew_Verbose = 0;
const char *DBFilename;
int DBLine;

// ============================================================================

class cReportWriter
{
public:
	cReportWriter() {	enable = 1; }
	~cReportWriter();
};

static cReportWriter ReportWriter;


// ============================================================================

// Standard new
//
void *operator new(size_t Size)
{
	void *Mem = GetMem(Size, NULL, 0);

	if (DBNew_Verbose)
	      fprintf(Out, "Allocated: File unknown, Size: %d, Addr: 0x%x\n",
			int(Size), int(Mem));

	return Mem;
}

// new operator with file name and line info
//
void *operator new(size_t Size, const char *Filename, int Line)
{
	void *Mem = GetMem(Size, Filename, Line);

	if (DBNew_Verbose)
		fprintf(Out, "Allocated: %s: %d, Size: %d, Addr: 0x%x\n",
			Filename, Line, int(Size), int(Mem));

	return Mem;
}

#ifdef NEWARRAY
// new operator with file name and line info
//
void *operator new[](size_t Size, const char *Filename, int Line)
{
	void *Mem = GetMem(Size, Filename, Line, DBN_ARRAY);

	if (DBNew_Verbose)
		fprintf(Out, "Allocated: %s: %d, Size: %d, Addr: 0x%x\n",
			Filename, Line, int(Size), int(Mem));

	return Mem;
}
#endif

// delete operator
//
void operator delete(void *Mem)
{
	if (DBNew_Verbose)
	{
		fprintf(Out, "Deleting: ");
		PrintWhere();
		fprintf(Out, ", Addr: 0x%x\n", int(Mem));
	}

	ReleaseMem(Mem);
}


#ifdef NEWARRAY
// delete operator
//
void operator delete[](void *Mem)
{
	if (DBNew_Verbose)
	{
		fprintf(Out, "Deleting: ");
		PrintWhere();
		fprintf(Out, ", Addr: 0x%x\n", int(Mem));
	}

	ReleaseMem(Mem, DBN_ARRAY);
}
#endif

// ============================================================================

static sMem *Head = NULL, *Tail = NULL, *Curr = NULL;
static int NumNews = 0, NumDeletes = 0, BadDeletes = 0;
static int TotalMem = 0, UsedMem = 0, PeakMem = 0;

#ifdef NEWARRAY
static int NumArrayNews = 0, NumArrayDeletes = 0, IncorrectDeletes = 0;
#endif


// ============================================================================
// Cleanup - print report, undeleted memory
//
cReportWriter::~cReportWriter()
{
	Curr = Head;
	DBFilename = NULL;
	DBLine = 0;

	fprintf(Out, "=========== DBNew Report ===========\n");
	fprintf(Out, "Number of news       : %d\n", NumNews);
	fprintf(Out, "Number of deletes    : %d\n", NumDeletes);
#ifdef NEWARRAY
	fprintf(Out, "Number of new[]'s    : %d\n", NumArrayNews);
	fprintf(Out, "Number of delete[]'s : %d\n", NumArrayDeletes);
	fprintf(Out, "Incorrect deletes    : %d\n", IncorrectDeletes);
#endif
	fprintf(Out, "Bad Deletes          : %d\n", BadDeletes);
	fprintf(Out, "\nTotal memory used    : %d bytes\n", TotalMem);
	fprintf(Out, "Peak memory Usage    : %d bytes\n", PeakMem);

	if (Curr)
	{
		fprintf(Out, "------------------------------------\n");
		fprintf(Out, "Undeleted memory :\n");
	}

	while (Curr)
	{
		PrintMem(Curr);
		fprintf(Out, "\n");
		CheckMem(Curr);
		Curr = Curr->Next;
	}

	enable = 0;
}


// Set the position in a file
//
void SetDBFileLinePos(const char *TheFilename, int TheLine)
{
	DBFilename = TheFilename;
	DBLine = TheLine;
}


// Get some memory
//
void *GetMem(size_t Size, const char *Filename, int Line, char Flag)
{
	if (!enable)
		return malloc(Size);
	
	sMem *Mem = (sMem *) malloc(sizeof(sMem));

	if (!Mem)
	{
	        fprintf(Out, "Out of Memory!!!\n");
		exit(1);
	}

	Mem->Ptr = (char *) malloc(Size+2);
	if (!Mem->Ptr)
	{
		fprintf(Out, "Out of Memory!!!\n");
		fprintf(Out, " Cannot allocate: %s: %d, Size: %d\n", Filename,
				Line, Size);
		exit(1);
	}
	Mem->Filename = Filename;
	Mem->Line = Line;
	Mem->Size = Size;
	Mem->Flags = Flag;

	Mem->Ptr[Size] = DBN_MAGIC1;
	Mem->Ptr[Size+1] = DBN_MAGIC2;

	Insert(Mem);
#ifdef NEWARRAY
	if (Flag & DBN_ARRAY)
		NumArrayNews++;
	else
#endif
	NumNews++;
	TotalMem += Size;
	UsedMem += Size;
	if (UsedMem > PeakMem)
		PeakMem = UsedMem;

	return (void *)(Mem->Ptr);
}

// Release mem and check for any errors
//
void ReleaseMem(void *Ptr, char Flag)
{
	if (!enable)
	{
		free(Ptr);
		return;
	}

	char *CPtr;

	if (Ptr == NULL)
	{
		fprintf(Out, "Attempt to delete NULL ptr : ");
		PrintWhere();
		fprintf(Out, "\n");
		BadDeletes++;
		return;
	}

	CPtr = (char *)Ptr;
	Curr = Head;

	while (Curr)
	{
		if (Curr->Ptr == CPtr)
		{
			CheckMem(Curr, Flag);
			UsedMem -= Curr->Size;
			free(Curr->Ptr);
			Remove();
#ifdef NEWARRAY
			if (Flag & DBN_ARRAY)
				NumArrayDeletes++;
			else
#endif
			NumDeletes++;

			DBFilename = NULL;
			DBLine = 0;

			return;
		}

		Curr = Curr->Next;
	}

	fprintf(Out, "Attempt to delete illegal ptr : 0x%x ", (int)Ptr);
	PrintWhere();
	fprintf(Out, "\n");
	BadDeletes++;
	DBFilename = NULL;
	DBLine = 0;
}

void CheckMem(sMem *Mem, char Flag)
{
	if ((Mem->Ptr[Mem->Size] != DBN_MAGIC1) ||
		(Mem->Ptr[Mem->Size+1] != DBN_MAGIC2))
	{
		fprintf(Out, " While deleting : ");
		PrintWhere();
		fprintf(Out, "\n  Upper bounds overwritten\n   Allocated at ");
		PrintMem(Mem);
		fprintf(Out, "\n");
	}

#ifdef NEWARRAY
	if (Flag < 0) return;

	if ((Mem->Flags & DBN_ARRAY) && !(Flag & DBN_ARRAY))
	{
		fprintf(Out, " While deleting : ");
		PrintWhere();
		fprintf(Out, "\n  Attempted deleting array with `delete'\n");
		fprintf(Out, "   Allocated at ");
		PrintMem(Mem);
		fprintf(Out, "\n");
		IncorrectDeletes++;
	}

	else if (!(Mem->Flags & DBN_ARRAY) && (Flag & DBN_ARRAY))
	{
		fprintf(Out, " While deleting : ");
		PrintWhere();
		fprintf(Out, "\n  Attempted deleting memory with `delete[]'\n");
		fprintf(Out, "   Allocated at ");
		PrintMem(Mem);
		fprintf(Out, "\n");
		IncorrectDeletes++;
	}
#endif
}


// Append to tail of list
//
void Append(sMem *Mem)
{
	Mem->Next = NULL;
	Mem->Prev = Tail;

	if (!Head)
		Head = Mem;
	if (Tail)
		Tail->Next = Mem;

	Tail = Mem;
	Curr = Mem;
}

// Insert at head of list
//
void Insert(sMem *Mem)
{
	Mem->Prev = NULL;
	Mem->Next = Head;

	if (!Tail)
		Tail = Mem;
	if (Head)
		Head->Prev = Mem;

	Head = Mem;
	Curr = Mem;
}

// Remove from list
//
void Remove()
{
	if (Curr)
	{
		if (Curr == Head)
			Head = Curr->Next;
		else
			Curr->Prev->Next = Curr->Next;

		if (Curr == Tail)
			Tail = Curr->Prev;
		else
			Curr->Next->Prev = Curr->Prev;

		free(Curr);
		Curr = Head;
	}
}


// ============================================================================

void PrintMem(sMem *Mem)
{
	char buffer[11];

	strncpy( buffer, Mem->Ptr, 10 );
	buffer[10] = '\0';

	if (Mem->Filename)
		fprintf(Out, "%s: %d, ptr: 0x%x, Size: %d, Contents: %s",
			Mem->Filename, Mem->Line, int(Mem->Ptr), int(Mem->Size),
			buffer);
	else
		fprintf(Out, "File unknown: ptr: 0x%x, Size: %d, Contents: %s",
			int(Mem->Ptr), int(Mem->Size), buffer);
}


void PrintWhere()
{
	if (DBFilename)
		fprintf(Out, "%s: %d", DBFilename, DBLine);
	else
		fprintf(Out, "File unknown");
}

#endif		// DEBUGNEW
