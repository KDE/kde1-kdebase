#!/usr/bin/perl

# =====================================================================
# Client
# =====================================================================

# ---------------------------------------------------------------------
# printClientBegin
#
# ---------------------------------------------------------------------

sub printClientBegin
{
    my( $mod, @inherits ) = @_;
    my( $i );

# --------------------------------------
# Write C++ file
# CLIENT2_CPP
# --------------------------------------

    print CLIENT2_CPP <<EOT;
// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis\@stud.uni-frankfurt.de
#include "@ARGV[2].h"

EOT

# --------------------------------------
# Write C++ file
# CLIENT_CPP
# --------------------------------------

    print CLIENT_CPP <<EOT;
// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis\@stud.uni-frankfurt.de

#include "ipc.h"
#include "@ARGV[2].h"

$mod\::$mod( int _port )
{
    bHeader = TRUE;
    cHeader = 0;
    pBody = 0L;

    port = _port;
    sock = new KSocket( "localhost", port );
    connect( sock, SIGNAL( readEvent(KSocket*) ), this, SLOT( readEvent(KSocket*) ) );
    connect( sock, SIGNAL( closeEvent(KSocket*) ), this, SLOT( closeEvent(KSocket*) ) );
    sock->enableRead( TRUE );
    connected = TRUE;
}

$mod\::~$mod()
{
    delete sock;
}

bool $mod\::isConnected()
{
    return connected;
}

void $mod\::closeEvent( KSocket * _sock )
{
    connected = FALSE;
}

void $mod\::readEvent( KSocket *_sock )
{
    if ( bHeader )
    {
	int n;
	n = read( sock->socket(), headerBuffer + cHeader, 1 );
	if ( headerBuffer[ cHeader ] == ' ' )
	{
	    bHeader = FALSE;
	    cHeader = 0;
	    bodyLen = atoi( headerBuffer );
	    cBody = 0;
	    if ( bodyLen <= 0 )
	    {
		printf("ERROR: Invalid header\\n");
		delete this;
		return;
	    }
	    if ( pBody != 0L )
		free( pBody );
	    pBody = (char*)malloc( bodyLen + 1 );
	}
	else if ( cHeader + n == 10 )
	{
	    printf("ERROR: Too long header\\n");
	    delete this;
	    return;
	}
	else
	{
	    if ( !isdigit( headerBuffer[ cHeader ] ) )
	    {
		printf("ERROR: Header must be an int\\n");
		delete this;
		return;
	    }

	    cHeader += n;
	    return;
	}
    }
	
    int n;
    n = read( sock->socket(), pBody + cBody, bodyLen - cBody );
    if ( n + cBody == bodyLen )
    {
	pBody[bodyLen] = 0;
	printf(">>'%s'\\n",pBody);
	bHeader = TRUE;
	parse( pBody, bodyLen );
	return;
    }
    cBody += n;
}

void $mod\::parse( char *_data, int _len )
{
    int pos = 0;
    char *name = read_string( _data, pos, _len );
    if ( name == 0L )
	return;
    _data += pos;
    _len -= pos;

EOT

# --------------------------------------
# Write start of header file
# --------------------------------------

    $mod_h = $mod."_h";
    print CLIENT_H <<EOT;
// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis\@stud.uni-frankfurt.de

#ifndef $mod_h
#define $mod_h

#include <ctype.h>
#include <ksock.h>
#include <qobject.h>
#include "ipc.h"

class $mod : public QObject
{
    Q_OBJECT
public:
    $mod( int _port );
    ~$mod();

    bool isConnected();

EOT

    # if ( $#inherits >= 0 ) { print CLIENT_H ' : public '; }
    # for $i (0..$#inherits)
    # {
    #	print CLIENT_H @inherits[$i];
    #	if ( $i < $#inherits ) { print CLIENT_H ", "; }
    # }
}

# ---------------------------------------------------------------------
# printClientEnd
#
# End of C++ class header
# ---------------------------------------------------------------------

sub printClientEnd
{
# --------------------------------------
# Write end of cpp file
# CLIENT_CPP
# --------------------------------------

    print CLIENT_CPP <<EOT;
    { printf("Unknown command '%s'\\n",name); }
}

EOT

# --------------------------------------
# Write end of header file
# --------------------------------------

    print CLIENT_H  <<EOT;
public slots:
    void readEvent( KSocket * );
    void closeEvent( KSocket * );
private:
    void parse( char *_data, int _len );

    int port;
    KSocket *sock;
    bool connected;
    char headerBuffer[11];
    int cHeader;
    bool bHeader;
    char *pBody;
    int cBody;
    int bodyLen;
};

#endif
EOT

    print CLIENT_CPP "\n#include \"@ARGV[2].moc\"\n";
}

# ---------------------------------------------------------------------
# printClientSlot
# 
# Write all the stuff neccessary to implement the given slot
# on client side.
# ---------------------------------------------------------------------

sub printClientSlot
{
   my( $retType, $retArray, $name, $param ) = @_;

# --------------------------------------
# Write to client header file
# --------------------------------------

   print CLIENT_H "public slots:\n\t";
   cppPrintMethod( CLIENT_H, $retType, $retArray, $name, $param );
   print CLIENT_H ";\n";

# --------------------------------------
# Write to client cpp file
# --------------------------------------

   printClientSlotCPP( CLIENT2_CPP, $retType, $retArray, $name, $param );

}

# ---------------------------------------------------------------------
# printClientSlotCPP
#
# Write the code that writes the parameters to the IO.
# ---------------------------------------------------------------------

sub printClientSlotCPP
{
    my( $OUT, $retType, $retArray, $name, $param ) = @_;
    my $i;

    cppPrintMethod( $OUT, $retType, $retArray, "$mainModule\::"."$name", $param );
    print $OUT "\n{\n\tint len = 0;\n";

    # Parse every paramter
    @param = split( /,/, $param );
    for $i (0..$#param)
    {
	$_ = @param[$i];
	/(\w*)( |@)(\w*)/ || die "Syntax error in parameter '$_'\n";
	( testType( $1 ) == 1 ) || die "Unknown type '$1'\n";
	if ( $2 eq "@" )
	{
	    print $OUT "\tlen += len_$1"."List( $3 );\n";
	}
	else
	{
	    print $OUT "\tlen += len_$1( $3 );\n";
	}
    }

    print $OUT "\tlen += len_string(\"$name\");\n";
    print $OUT "\twrite_int( sock->socket(), len );\n";
    print $OUT "\twrite_string( sock->socket(), \"$name\" );\n";

    # Parse every paramter
    @param = split( /,/, $param );
    for $i (0..$#param)
    {
	$_ = @param[$i];
	/(\w*)( |@)(\w*)/ || die "Syntax error in parameter '$_'\n";
	( testType( $1 ) == 1 ) || die "Unknown type '$1'\n";
	if ( $2 eq "@" )
	{
	    print $OUT "\twrite_$1"."List( sock->socket(), $3 );\n";
	}
	else
	{
	    print $OUT "\twrite_$1( sock->socket(), $3 );\n";
	}
    }

    if ( $retType ne "void" )
    {
	die "Only void allowed as return type\n";
    }

    print $OUT "}\n\n";
}

# ---------------------------------------------------------------------
# printClientSignal
#
# Write the stuff neccessary to parse an incoming signal from
# the server.
# ---------------------------------------------------------------------

sub printClientSignal
{
   my( $retType, $retArray, $name, $param ) = @_;
   print CLIENT_H "signals:\n\t";
   cppPrintMethod( CLIENT_H, $retType, $retArray, $name, $param );
   print CLIENT_H ";\nprivate:\n";
   print CLIENT_H "\tvoid parse_$name( char *_data, int _len );\n";

   printClientMainParser( CLIENT_CPP, $name );

   printClientParser( CLIENT2_CPP, $retType, $retArray, $name, $param );
}

# ---------------------------------------------------------------------
# printClientMainParser
#
# Write in the cpp file that checks the name of the incoming signal.
# If this fits '$name', calls the function 'parse_$name'.
# ( CLIENT_CPP )
# ---------------------------------------------------------------------

sub printClientMainParser
{
    my( $OUT, $name ) = @_;

    print $OUT "\tif ( strcmp( name, \"$name\" ) == 0 ) { parse_$name( _data, _len ); } else\n";
}

# ---------------------------------------------------------------------
# printClientParser
#
# Write the parser that is neccessary to parse all arguments of a signal.
# ( CLIENT2_CPP )
# ---------------------------------------------------------------------

sub printClientParser
{
    my( $OUT, $retType, $retArray, $name, $param ) = @_;
    my $i;

    print $OUT "void $mainModule\::parse_$name( char *_data, int _len )\n{\n\tint pos = 0;\n\n";

    # Parse every paramter and provide a variable for
    # all parameters and then fill the variables with the
    # arguments to the function.

    @param = split( /,/, $param );
    for $i (0..$#param)
    {
	$_ = @param[$i];
	/(\w*)( |@)(\w*)/ || die "Syntax error in parameter '$_'\n";
	( testType( $1 ) == 1 ) || die "Unknown type '$1'\n";
	print $OUT "\t// Parsing $_\n\t";
	cppPrintVariable( $OUT, $1, $2, $3, "Value" );
	print $OUT ";\n";
	cppParse( $OUT, $1, $2, $3 );
    }

    # Call the function and take the return value

    print $OUT "\t// Calling function\n";
    print $OUT "\temit $name( ";

    # Pass all arguments to the function
    for $i (0..$#param)
    {
	$_ = @param[$i];
	/(\w*)( |@*)(\w*)/;
	print $OUT "$3";
	if ( $i < $#param ) { print $OUT ", "; }
    }
    print $OUT " );\n";

    print $OUT "}\n\n";
}

# =====================================================================
# Server
# =====================================================================

# ---------------------------------------------------------------------
# printServerBegin
#
# Generating C++ class header
# ---------------------------------------------------------------------

sub printServerBegin
{
    my( $mod, @inherits ) = @_;
    my( $i );
    
# --------------------------------------
# Write start of header file
# --------------------------------------

    $mod_h = $mod."_h";
    print SERVER_H <<EOT;
// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis\@stud.uni-frankfurt.de

#ifndef $mod_h
#define $mod_h
#include <qobject.h>
#include <ksock.h>
#include <ctype.h>
#include "ipc.h"

class $mod : public QObject
{
    Q_OBJECT
public:
    $mod( KSocket * );
    ~$mod();

    void parse( char *_data, int _len );

EOT

# --------------------------------------
# Write start of cpp file for Parsing
# --------------------------------------

    print SERVER2_CPP <<EOT;
// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis\@stud.uni-frankfurt.de

#include "$ARGV[1].h"

EOT


# --------------------------------------
# Write start of cpp file for IO
# --------------------------------------

    $ipc = $mod."Server";
    print SERVER_CPP <<EOT;
// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis\@stud.uni-frankfurt.de

#include "@ARGV[1].h"

$ipc\::$ipc()
{
    serv_sock = new KServerSocket( 0 ); /* 0: choose free port */
    if ( serv_sock->socket() < 0 )
    {
	printf("ERROR: Could not establish server\\n");
	exit(1);
    }
    printf( "SOCK=%i\\n",serv_sock->getPort());
    connect( serv_sock, SIGNAL( accepted(KSocket*) ), 
	    this, SLOT( slotAccept(KSocket*) ) );
}

void $ipc\::slotAccept( KSocket *_sock )
{
    $mod * p = new $mod( _sock );
    emit newClient( p );
}

$ipc\::~$ipc()
{
    delete serv_sock;
}

int $ipc\::getPort()
{
    return serv_sock->getPort();
}

$mod\::$mod( KSocket *_sock )
{
    bHeader = TRUE;
    cHeader = 0;
    pBody = 0L;

    connect( _sock, SIGNAL( readEvent(KSocket*) ), this, SLOT( readEvent(KSocket*) ) );
    connect( _sock, SIGNAL( closeEvent(KSocket*) ), this, SLOT( closeEvent(KSocket*) ) );
    _sock->enableRead( TRUE );
    data_sock = _sock;
}

$mod\::~$mod()
{
    data_sock->enableRead( FALSE );
    delete data_sock;
    if ( pBody != 0L )
	free( pBody );
}

void $mod\::closeEvent( KSocket *_sock )
{
    delete this;
    return;
}

void $mod\::readEvent( KSocket *_sock )
{
    if ( bHeader )
    {
	int n;
	n = read( data_sock->socket(), headerBuffer + cHeader, 1 );
	if ( headerBuffer[ cHeader ] == ' ' )
	{
	    bHeader = FALSE;
	    cHeader = 0;
	    bodyLen = atoi( headerBuffer );
	    cBody = 0;
	    if ( bodyLen <= 0 )
	    {
		printf("ERROR: Invalid header\\n");
		delete this;
		return;
	    }
	    if ( pBody != 0L )
		free( pBody );
	    pBody = (char*)malloc( bodyLen + 1 );
	}
	else if ( cHeader + n == 10 )
	{
	    printf("ERROR: Too long header\\n");
	    delete this;
	    return;
	}
	else
	{
	    if ( !isdigit( headerBuffer[ cHeader ] ) )
	    {
		printf("ERROR: Header must be an int\\n");
		delete this;
		return;
	    }

	    cHeader += n;
	    return;
	}
    }
	
    int n;
    n = read( data_sock->socket(), pBody + cBody, bodyLen - cBody );
    if ( n + cBody == bodyLen )
    {
	pBody[bodyLen] = 0;
	printf(">>'%s'\\n",pBody);
	bHeader = TRUE;
	parse( pBody, bodyLen );
	return;
    }
    cBody += n;
}

void $mod\::parse( char *_data, int _len )
{
    int pos = 0;
    char *name = read_string( _data, pos, _len );
    if ( name == 0L )
	return;
    _data += pos;
    _len -= pos;
EOT
}

# ---------------------------------------------------------------------
# printServerEnd
#
# Write end of server files
# ---------------------------------------------------------------------

sub printServerEnd
{
# --------------------------------------
# Write end of header file
# --------------------------------------

    $ipc = $mainModule."Server";
    print SERVER_H <<EOT;

public slots:
   void closeEvent( KSocket* );
   void readEvent( KSocket* );
protected:
   KSocket* data_sock;
   char headerBuffer[11];
   int cHeader;
   bool bHeader;
   char *pBody;
   int cBody;
   int bodyLen;
};

class $ipc : public QObject
{
    Q_OBJECT
public:
    $ipc();
    ~$ipc();

    int getPort();

signals:
   void newClient( $mainModule * );

public slots:
   virtual void slotAccept( KSocket* );

protected:
   KServerSocket* serv_sock;
};

#endif
EOT

   print SERVER_CPP <<EOT;
\t\treturn;
\tfree_string( name );
}

#include "@ARGV[1].moc"
EOT

}

# ---------------------------------------------------------------------
# printServerSignal
# 
# Write all the stuff neccessary to implement the given signal
# on server side.
# ---------------------------------------------------------------------

sub printServerSignal
{
   my( $retType, $retArray, $name, $param ) = @_;

# --------------------------------------
# Write to server header file
# --------------------------------------

   print SERVER_H "public slots:\n\t";
   cppPrintMethod( SERVER_H, $retType, $retArray, $name, $param );
   print SERVER_H ";\n";

# --------------------------------------
# Write to client cpp file
# --------------------------------------

   printServerSignalCPP( SERVER2_CPP, $retType, $retArray, $name, $param );

}

# ---------------------------------------------------------------------
# printServerSignalCPP
#
# Write the code that writes the parameters to the IO.
# ---------------------------------------------------------------------

sub printServerSignalCPP
{
    my( $OUT, $retType, $retArray, $name, $param ) = @_;
    my $i;

    cppPrintMethod( $OUT, $retType, $retArray, "$mainModule\::"."$name", $param );
    print $OUT "\n{\n\tint len = 0;\n";

    # Parse every paramter
    @param = split( /,/, $param );
    for $i (0..$#param)
    {
	$_ = @param[$i];
	/(\w*)( |@)(\w*)/ || die "Syntax error in parameter '$_'\n";
	( testType( $1 ) == 1 ) || die "Unknown type '$1'\n";
	if ( $2 eq "@" )
	{
	    print $OUT "\tlen += len_$1"."List( $3 );\n";
	}
	else
	{
	    print $OUT "\tlen += len_$1( $3 );\n";
	}
    }

    print $OUT "\tlen += len_string(\"$name\");\n";
    print $OUT "\twrite_int( data_sock->socket(), len );\n";
    print $OUT "\twrite_string( data_sock->socket(), \"$name\" );\n";

    # Parse every paramter
    @param = split( /,/, $param );
    for $i (0..$#param)
    {
	$_ = @param[$i];
	/(\w*)( |@)(\w*)/ || die "Syntax error in parameter '$_'\n";
	( testType( $1 ) == 1 ) || die "Unknown type '$1'\n";
	if ( $2 eq "@" )
	{
	    print $OUT "\twrite_$1"."List( data_sock->socket(), $3 );\n";
	}
	else
	{
	    print $OUT "\twrite_$1( data_sock->socket(), $3 );\n";
	}
    }

    if ( $retType ne "void" )
    {
	die "Only void allowed as return type\n";
    }

    print $OUT "}\n\n";
}

# ---------------------------------------------------------------------
# printServerSlot
#
# Write the stuff neccessary to parse an incoming call.
# ---------------------------------------------------------------------

sub printServerSlot
{
   my( $retType, $retArray, $name, $param ) = @_;
   print SERVER_H "signals:\n\t";
   cppPrintMethod( SERVER_H, $retType, $retArray, $name, $param );
   print SERVER_H ";\n";
   print SERVER_H "private:\n\tvoid parse_$name( char *_data, int _len );\n";

   printServerMainParser( SERVER_CPP, $name );

   printServerParser( SERVER2_CPP, $retType, $retArray, $name, $param );
}

# ---------------------------------------------------------------------
# printServerMainParser
#
# Write in the cpp file that checks the name of the incoming function
# call. If this fits '$name', calls the function 'parse_$name'.
# ( SERVER_CPP )
# ---------------------------------------------------------------------

sub printServerMainParser
{
    my( $OUT, $name ) = @_;

    print $OUT "\tif ( strcmp( name, \"$name\" ) == 0 ) { parse_$name( _data, _len ); } else\n";
}

# ---------------------------------------------------------------------
# printServerParser
#
# Write the parser that is neccessary to parse all arguments to a function.
# ( SERVER2_CPP )
# ---------------------------------------------------------------------

sub printServerParser
{
    my( $OUT, $retType, $retArray, $name, $param ) = @_;
    my $i;

    print $OUT "void $mainModule\::parse_$name( char *_data, int _len )\n{\n\tint pos = 0;\n\n";

    # Parse every paramter and provide a variable for
    # all parameters and then fill the variables with the
    # arguments to the function.

    @param = split( /,/, $param );
    for $i (0..$#param)
    {
	$_ = @param[$i];
	/(\w*)( |@)(\w*)/ || die "Syntax error in parameter '$_'\n";
	( testType( $1 ) == 1 ) || die "Unknown type '$1'\n";
	print $OUT "\t// Parsing $_\n\t";
	cppPrintVariable( $OUT, $1, $2, $3, "Value" );
	print $OUT ";\n";
	cppParse( $OUT, $1, $2, $3 );
    }

    # Call the function and take the return value

    print $OUT "\t// Calling function\n";
    print $OUT "\temit $name( ";

    # Pass all arguments to the function
    for $i (0..$#param)
    {
	$_ = @param[$i];
	/(\w*)( |@*)(\w*)/;
	print $OUT "$3";
	if ( $i < $#param ) { print $OUT ", "; }
    }
    print $OUT " );\n";

    print $OUT "}\n\n";
}

# =====================================================================
# C++ Stuff
#
# General stuff needed by server and client
# =====================================================================

# ---------------------------------------------------------------------
# cppParse
#
# Read from '_data', at position 'pos', where data has the maximum
# length of '_len' a value of type '$type' in variable called '$name'.
# The variable is an array of the type, if array is not equal to " ".
# ---------------------------------------------------------------------

sub cppParse
{
    my( $OUT, $type, $array, $name ) = @_;
    my $i;

    if ( $array eq " " )
    {
	print $OUT "\t$name = read_$type","( _data, pos, _len );\n";
    }
    else
    {
	print $OUT "\tread_$type","List( _data, pos, _len, &","$name );\n";
    }
}

# ---------------------------------------------------------------------
# cppPrintMethod
#
# print C++ class method ( like: void setText( char* _text); )
# ---------------------------------------------------------------------

sub cppPrintMethod
{
    my( $OUT, $retType, $retArray, $name, $param ) = @_;
    my( $i );

    ( testType( $retType ) == 1 ) || die "Unknown type '$retType'\n";
 
    cppPrintType( $OUT, $retType, $retArray, "Pointer" );
    print $OUT " $name(";

    @param = split( /,/, $param );
    for $i (0..$#param)
    {
	$_ = @param[$i];
	/(\w*)( |@*)(\w*)/;
	( testType( $1 ) == 1 ) || die "Unknown type '$1'\n";
	cppPrintVariable( $OUT, $1, $2, $3, "Reference" );
	if ( $i < $#param ) { print $OUT ", "; }
    }

    print $OUT ")";
}

# ---------------------------------------------------------------------
# cppPrintType
#
# print C++ variabble type ( like: char* )
# '$array' may be " " or "@". The last one denotes an array.
# ---------------------------------------------------------------------

sub cppPrintType
{
    # $ref says wether we want complex type as "Pointer", "Reference" or "Value"
    my( $OUT, $type, $array, $ref ) = @_;

    if ( $array ne " " )
    {
	print $OUT "struct $type", "List";
    }
    elsif ( $type eq "void" || $type eq "int" || $type eq "char" || $type eq "double" || $type eq "bool" )
    {
	print $OUT "$type";
    }
    elsif ( $type eq "string" )
    {
	print $OUT "const char*";
    }
    elsif ( $type eq "hash" )
    {
	print $OUT "hash*";
    }

    if ( $array ne " " )
    {
	if ( $ref eq "Pointer" ) { print $OUT "*"; }
	elsif ( $ref eq "Reference" ) { print $OUT "&"; }
    }
}

# ---------------------------------------------------------------------
# cppPrintVariable
#
# print named C++ variable ( like: char *_x )
# ---------------------------------------------------------------------

sub cppPrintVariable
{
    # $ref says wether we want complex type as "Pointer", "Reference" or "Value"
    my( $OUT, $type, $array, $name, $ref ) = @_;
    
    cppPrintType( $OUT, $type, $array, $ref );
    print $OUT " $name";
}


# =====================================================================
# IPCL Stuff
# =====================================================================

# Test wether the parameter is a valid ipcl type

sub testType
{
    my $p = shift;
    $_ = $p;
    if ( /^void$|^string$|^hash$|^int$|^double$|^bool$/ ) { 1; } else { 0; }
}

# The Main Parser
    
sub readIPC
{
    my $INPUT = shift;
    my $code = "FALSE";
    my $module = "";
    my $mode = "";

    while (<$INPUT>)
    {
	$tmp = $_;

	if ( /^extends / )
	{
	    $code eq "FALSE" || die "EXTENDS between definitions not allowed";

	    ( $module ne "" ) || die "No module name defined\n";

	    if ( /(.*) (.*)\;/ )
	    {
		open( IN2, "$2.ipc" ) || die "Could not open $2.ipc\n";
		readIPC( IN2 );
	    }
	}
	elsif ( /^\#/ || /^\s*\n$/ )
	{
	}
	elsif ( /^\s*slots:/ )
	{
	    $mode = "slots";
	}
	elsif ( /^\s*signals:/ )
	{
	    $mode = "signals";
	}
	elsif ( /^\s*server:/ )
	{
	    $mode = "server";
	    print SERVER_H "public:\n"
	}
	elsif ( /^\s*client:/ )
	{
	    $mode = "client";
	    print CLIENT_H "public:\n"
	}
	elsif ( /^module / )
	{
	    $code eq "FALSE" || die "EXTENDS between definitions not allowed";

	    /(.*) (.*)\;/ ? $module = $2 : die "Error in line: $_";

	    if ( !defined( $mainModule ) )
	    { $mainModule = $module; }
	    else
	    { push @extends, $module; }
	}
	elsif ( /\w/ )
	{
	    ( $module ne "" ) || die "No module name defined\n";

	    if ( defined( $header ) )
	    {
		printServerBegin( $mainModule, @extends );
		printClientBegin( $mainModule, @extends );
		undef( $header );
	    }

	    $code = "TRUE";

	    if ( $mode eq "server" )
	    {
		print SERVER_H $_;
	    }
	    elsif ( $mode eq "client" )
	    {
		print CLIENT_H $_;
	    }
	    else
	    {
		while ( /  / ) { $_ =~ s/  / /g; }
		$_ =~ s/(\w)  ( *)(\w)/$1 $3/g;
		$_ =~ s/(\W) ( *)(\w|\W)/$1$3/g;
		$_ =~ s/(\w) ( *)(\W)/$1$3/g;
		$_ =~ s/\[\]/@/g;
		/(\w*)( |@)(\w*)\((.*)\)\;/ || die "Syntax error in '$tmp'";
		
		$ret = $1;
		$retArray = $2;
		$name = $3;
		$param = $4;
		
		if ( $mode eq "slots" )
		{
		    printServerSlot( $ret, $retArray, $name, $param );
		    printClientSlot( $ret, $retArray, $name, $param );
		}
		else
		{
		    printServerSignal( $ret, $retArray, $name, $param );
		    printClientSignal( $ret, $retArray, $name, $param );
		}
	    }
	}
    }
}

# =====================================================================
# Main Program
# =====================================================================

( $#ARGV == 2 ) || die "Syntax:\nipcc.pl foo.ipc server client\n";
@extends = ();
$header = "TRUE";

open( CLIENT_CPP, ">".@ARGV[2].".cpp" ) || die "Could not open @ARGV[2]".".cpp for output\n";
open( CLIENT2_CPP, ">".@ARGV[2]."2.cpp" ) || die "Could not open @ARGV[2]"."2.cpp for output\n";
open( CLIENT_H, ">".@ARGV[2].".h" ) || die "Could not open @ARGV[2].h for output\n";
open( SERVER_H, ">".@ARGV[1].".h" ) || die "Could not open @ARGV[1].h for output\n";
open( SERVER2_CPP, ">".@ARGV[1]."2.cpp" ) || die "Could not open @ARGV[1]"."2.cpp for output\n";
open( SERVER_CPP, ">".@ARGV[1].".cpp" ) || die "Could not open @ARGV[1]".".cpp for output\n";
open( IN, @ARGV[0] ) || die "Could not open @ARGV[0] for input\n";

print "Parsing...\n";
readIPC( IN );
printServerEnd();
printClientEnd();

close CLIENT_CPP;
close CLIENT2_CPP;
close CLIENT_H;
close SERVER_H;
close SERVER_CPP;
close SERVER2_CPP;

system "moc -o @ARGV[1].moc @ARGV[1].h";
system "moc -o @ARGV[2].moc @ARGV[2].h";

print "Done\n";
