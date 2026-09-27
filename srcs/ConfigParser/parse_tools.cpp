#include "ConfigParser.hpp"
#include "utils.hpp"

int	ConfigParser::sendParseError(int error)
{
	std::string	msg;

	switch (error)
	{
	case GENERIC_ERROR:
		msg = "an error has ocurred while parsing the config file";
		break;
	case UNCLOSED_QUOTE:
		msg = "quote opened but never closed";
		break;
	case UNMATCHED_BRACKET:
		msg = "bracket unmatched";
		break;
	case MISSING_SEMICOLON:
		msg = "missing semicolon";
		break;
	case EMPTY_DIRECTIVE:
		msg = "empty directive";
		break;
	case UNKNOWN_DIRECTIVE:
		msg = "unknown directive \"" + error_par + "\"";
		break;
	case DUPLICATED_DIRECTIVE:
		msg = "directive \"" + error_par + "\" doesn't allow duplicates";
		break;
	case DUPLICATED_VALUE:
		msg = "directive \"" + error_par + "\" doesn't allow duplicated values";
		break;
	case INVALID_VALUE:
		msg = "invalid value in directive \"" + error_par + "\"";
		break;
	case MULTIPLE_VALUES:
		msg = "directive \"" + error_par + "\" doesn't allow multiple values";
		break;
	case EMPTY_VALUE:
		msg = "empty value in directive \"" + error_par + "\"";
		break;
	default:
		break;
	}
	return error_msg(1, "parse", msg.c_str());
}

int ConfigParser::handleQuote(std::stringstream &ss, std::string starter, char quote_char)
{
	size_t		remainder_index = starter.find(quote_char);
	std::string	remainder = starter.substr(0, remainder_index);
	tokens.push_back(remainder);

	size_t		end_quote = starter.find(quote_char, remainder_index + 1);
	std::string quote;

	if (end_quote != std::string::npos)
	{
		quote = starter.substr(remainder_index + 1, end_quote - remainder_index - 1);
		tokens.push_back(quote);
		
		remainder = starter.substr(end_quote + 1);
		tokens.push_back(remainder);
		return 0;
	}

	quote = starter.substr(remainder_index + 1);
	std::string	str;
	if (std::getline(ss, str, quote_char))
	{
		quote += str;
		tokens.push_back(quote);
		return 0;
	}
	else
	{
		if (ss.eof())
			return UNCLOSED_QUOTE;
		return GENERIC_ERROR;
	}
}

void	ConfigParser::push_separately(std::string &s, size_t index)
{
	std::string	remainder = s.substr(0, index);
	tokens.push_back(remainder);
	tokens.push_back(";");
	if ((index + 1) < s.length())
		tokens.push_back(s.substr(index + 1));
}

int	ConfigParser::getTokens()
{
	std::stringstream	ss(raw);
	std::string			temp;
	int					status;

	while (ss >> temp)
	{
		size_t	index;
		if ((index = temp.find(';')) != std::string::npos)
			push_separately(temp, index);
		if (temp.find('{'))
			push_separately(temp, index);
		if (temp.find('}'))
			push_separately(temp, index);
		size_t	quote_pos = temp.find('\'') == std::string::npos ? temp.find('\'') : temp.find('\"');
		if (quote_pos != std::string::npos && (status = handleQuote(ss, temp, temp[quote_pos])))
			return status;
	}
	return 0;
}


