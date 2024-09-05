#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include <memory>

#include "session.h"


TcpSession::TcpSession(TcpConnDriverPtr & drvPtr)
{
    m_drvPtr = drvPtr;
}

TcpSession::~TcpSession()
{
}

