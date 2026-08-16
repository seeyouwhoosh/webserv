#include "HTTPmessage.hpp"

HTTPmessage::HTTPmessage() {}

HTTPmessage::~HTTPmessage() {}

HTTPmessage::HTTPmessage(HTTPmessage &other)
{
	*this = other;
}

HTTPmessage	&HTTPmessage::operator=(HTTPmessage &other)
{
	(void)other;
}
