#include "HTTPmessage.hpp"

/* Returns location index if it exist, it not, returns -1 */
int	getLocationIndex(std::vector<LocationConfig> &locations, std::string path)
{
	size_t		max_len = 0;
	size_t		location_pos = -1;

	for (size_t i = 0; i < locations.size(); i++)
	{
		size_t	len = locations[i].name.length();
		
		if (!path.find(locations[i].name))
		{
			if (len > max_len)
			{
				location_pos = i;
				max_len = len;
			}
		}
	}

	if (!max_len)
		return -1;
	else
		return location_pos;
}

std::string	getMIMEtype(std::string ext)
{
	stringmap::iterator it = mime_types.find(ext);

	if (it == mime_types.end())
		return "application/octet-stream";

	return it->second;
}

int	pipePipes(int pipe_in[2], int pipe_out[2])
{
	if (pipe(pipe_out) < 0)
		return -1;

	if (pipe(pipe_in) < 0)
	{
		close(pipe_out[0]);
		close(pipe_out[1]);
		return -1;
	}

	return 0;
}

void	closePipes(int pipe_in[2], int pipe_out[2])
{
	close(pipe_in[0]);
	close(pipe_in[1]);
	close(pipe_out[0]);
	close(pipe_out[1]);
}
