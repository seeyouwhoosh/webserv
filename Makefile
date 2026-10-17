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
CFLAGS		= -Wall -Werror -Wextra -std=c++98 -g3
MAKE		= make -sC
RM			= rm -rf

# Includes
INCLUDE_DIR	= includes
INCLUDES	= -I$(INCLUDE_DIR)

# Sources
SRC_DIR		= srcs
SOURCES		= main.cpp \
				HTTPmessage.cpp \
				WebServer.cpp \
				utils.cpp \
				ConfigParser/ConfigParser.cpp ConfigParser/HTTPContext.cpp ConfigParser/ServerConfig.cpp ConfigParser/LocationConfig.cpp \
				Client.cpp \
				CGIHandler.cpp \
				RequestParser.cpp

# Objects
OBJ_DIR		= objs
OBJ			= $(SOURCES:.cpp=.o)
OBJECTS		= $(addprefix $(OBJ_DIR)/, $(SOURCES:.cpp=.o))

# Bonus sources
BSRC_DIR	= srcs
BSOURCES	= 

# Bonus objects
BOBJ_DIR	= objs_bonus
BOBJECTS	= $(addprefix $(BOBJ_DIR)/, $(BSOURCES:.cpp=.o))

all: $(NAME)


# Main program
$(NAME): $(OBJECTS)
	@echo "$(BBLUE)$(NAME)$(NC): Creating $(GREEN)$(NAME)$(NC)..."
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LINKER)
	@echo "$(BBLUE)$(NAME)$(NC): $(GREEN)$(NAME)$(NC) created"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR) $(OBJ_DIR)/HTTPmessage $(OBJ_DIR)/WebServer $(OBJ_DIR)/ConfigParser $(OBJ_DIR)/utils $(OBJ_DIR)/RequestParser
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ 

# Bonus program
bonus: $(BOBJECTS)
	@echo "$(BBLUE)$(NAME)$(NC): Creating $(GREEN)$(NAME) ($@)$(NC)..."
	$(CC) $(CFLAGS) -o $(NAME) $(BOBJECTS) $(LINKER)
	@echo "$(BBLUE)$(NAME)$(NC): $(GREEN)$(NAME) ($@)$(NC) created"

$(BOBJ_DIR)/%.o: $(BOBJ_DIR)/%.cpp
	@mkdir -p $(BOBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Cleaning targets
clean:
	$(RM) $(OBJ_DIR)
	$(RM) $(BOBJ_DIR)
	@echo "$(BBLUE)$(NAME)$(NC): Objects cleaned"

fclean: clean
	$(RM) $(NAME)
	@echo "$(BBLUE)$(NAME)$(NC): Fully cleaned"

re: fclean all
