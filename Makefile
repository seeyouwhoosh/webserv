# Special targets
.POSIX:
.PHONY: clean re fclean all

# Color
BBLUE		= \033[1;34m
GREEN		= \033[0;32m
NC			= \033[0m

# Compiler
NAME		= webserv
CC			= c++
CFLAGS		= -Wall -Werror -Wextra -std=c++98
RM			= rm -rf

# Sources
SOURCES		= 

# Objects
OBJECTS		= $(SOURCES:.cpp=.o)


# Main program
all: $(OBJECTS) $(NAME)

$(NAME): $(OBJECTS)
	@echo "$(BBLUE)$(NAME)$(NC): Creating $(GREEN)$@$(NC)..."
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) 
	@echo "$(BBLUE)$(NAME)$(NC): $(GREEN)$(NAME)$(NC) created"

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	@$(RM) $(OBJECTS)
	@echo "$(BBLUE)$(NAME)$(NC): Objects cleaned"

fclean: clean
	@$(RM) $(NAME)
	@echo "$(BBLUE)$(NAME)$(NC): Fully cleaned"

re: fclean all
