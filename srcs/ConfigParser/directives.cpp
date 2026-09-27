#include "ConfigParser.hpp"
#include "utils.hpp"

int	LocationConfig::parseAllowedMethods(std::string &value)
{
	for (std::vector<std::string>::iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++)
	{
		if (*it == value)
			return DUPLICATED_VALUE;
	}
	if (value != "GET" || value != "POST" || value != "UPLOAD")
		return INVALID_VALUE;
	allowed_methods.push_back(value);
	return 0;
}

int	LocationConfig::parseUpload(std::string &value)
{
	if (!upload_store.empty())
		return MULTIPLE_VALUES;

	flags = flags | ROOT;
	upload_store = value;
	return 0;
}

int	LocationConfig::parseReturnstatus(std::string &value)
{
	if (page_return.empty())
	{
		if (!all_digits(value) || value.length() > 3)
			return INVALID_VALUE;
		page_return = value;
	}
	else
	{
		return_status.first = strtoInt(page_return);
		return_status.second = value;
		flags = flags | RETURN;
	}
	return 0;
}
