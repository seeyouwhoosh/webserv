#include "ConfigParser.hpp"
#include "utils.hpp"

bool	incompleteSemicolons(std::string &s)
{
	std::stringstream	ss(s);
	std::string			buff;

	while (std::getline(ss, buff))
	{
		if (buff.find_first_not_of(" \t\r\n") == std::string::npos)
			continue;

		if (buff.find('{') != std::string::npos || buff.find('}') != std::string::npos)
			continue;

		size_t last_char = buff.find_last_not_of(" \t\r\n");
		if (last_char == std::string::npos || buff[last_char] != ';')
			return true;
	}
	return false;
}

bool	unmatchedBracket(std::string &s)
{
	int		par = 0;
	bool	in_comment = false;

	for (size_t i = 0; i < s.size(); i++)
	{
		if (s[i] == '#')
		{
			in_comment = true;
			continue;
		}
		if (in_comment)
		{
			if (s[i] == '\n')
				in_comment = false;
			continue;
		}
		if (s[i] == '{')
			par++;
		else if (s[i] == '}')
		{
			par--;
			if (par < 0)
				return true;
		}
	}

	if (par)
		return true;
	return false;
}
