#include "Client.hpp"

Client()
{
    ;
}

Client(Client &other)
{
    ;
}

Client	&operator=(Client &other)
{
    ;
}

~Client()
{
    ;
}


time_t		getLastActivity()
{
    ;
}

int			getKeepAlive()
{
    ;
}

int			getMaxRequest()
{
    ;
}

int			getNumberRequests()
{
    ;
}

HTTPmessage	&getHTTPmessage()
{
    ;
}

Parser		&getParser()
{
    ;
}

void		setLastActivity(time_t last)
{
    ;
}

void		setKeepAlive(int keepalive)
{
    ;
}

void		setMaxRequest(int max_request)
{
    ;
}

void		setNumberRequests(int n)
{
    ;
}

void		setHTTPmessage(HTTPmessage &message)
{
    ;
}

void		setParser(Parser &parser)
{
    ;
}
