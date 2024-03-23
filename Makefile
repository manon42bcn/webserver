# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/03/03 13:03:40 by vaguilar          #+#    #+#              #
#    Updated: 2024/03/23 20:38:44 by vaguilar         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

DEL_LINE		=	\033[2K
ITALIC			=	\033[3m
BOLD			=	\033[1m
DEF_COLOR		=	\033[0;39m
GRAY			=	\033[0;90m
RED				=	\033[0;91m
GREEN			=	\033[0;92m
YELLOW			=	\033[0;93m
BLUE			=	\033[0;94m
MAGENTA			=	\033[0;95m
CYAN			=	\033[0;96m
WHITE			=	\033[0;97m
BLACK			=	\033[0;99m
ORANGE			=	\033[38;5;209m
BROWN			=	\033[38;2;184;143;29m
DARK_GRAY		=	\033[38;5;234m
MID_GRAY		=	\033[38;5;245m
DARK_GREEN		=	\033[38;2;75;179;82
DARK_YELLOW		=	\033[38;5;143m

NAME		=	webserv

SRCS_DIR	=	src
CONFIG_DIR  =   $(SRCS_DIR)/configFile
INCS_DIR	=	inc
INCS_DIR	+=	libs/Libft/inc
OBJS_DIR	=	.objs
DEPS_DIR	=	.deps

LIB_FT		=	libs/Libft
LIBS_LIBS	=	libs/Libft/libft.a
LIB_LINKS	=	-L ./$(LIB_FT)

SRCS		=	src/webserver.cpp
SRCS		+=	src/configuration/Config.cpp
SRCS		+=	src/configuration/parse.cpp
OBJS		=	$(patsubst $(SRCS_DIR)/%, $(OBJS_DIR)/%, $(SRCS:.cpp=.o))
DEPS		=	$(patsubst $(SRCS_DIR)/%, $(DEPS_DIR)/%, $(SRCS:.cpp=.d))

CPPFLAGS	+=	-Wall -Werror -Wextra -std=c++98 $(addprefix -I , $(INCS_DIR))
CPPFLAGS	+=	-MMD -MP -MF $(DEPS_DIR)/$*.d
# If you wanna be more strict in cpp standard uncomment next line
#CPPFLAGS	+=  -pedantic-errors

CXX			=	c++
RM			=	rm -rf
MKDIR		=	mkdir -p
MAKE		=	make --no-print-directory


$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@$(MKDIR) $(dir $@)
	@$(MKDIR) $(dir $(DEPS_DIR)/$*)
	@$(CXX) $(CPPFLAGS) -c $< -o $@ -MF $(DEPS_DIR)/$*.d
	@echo "$(GREEN)$(patsubst $(SRCS_DIR)/%,%, $<)" | awk '{printf "%-50s\tcompiled ✓$(DEF_COLOR)\n", $$0;}'

	
#$(OBJS_DIR)/%.o		:	$(SRCS_DIR)/%.cpp
#	$(CXX) $(CPPFLAGS) -c $< -o $@
#	@echo "$(GREEN)$(patsubst $(SRCS_DIR)/%,%, $<)" | awk '{printf "%-50s\tcompiled ✓$(DEF_COLOR)\n", $$0;}'

all		:	directories $(NAME)

$(NAME)	:	$(OBJS)
	@$(MAKE) -C $(LIB_FT)
	@$(CXX) $(LIB_LINKS) $^ -o $@ -lft
	@echo "$(MAGENTA)Executable $@ compiled$(DEF_COLOR)"

directories:
	@$(MKDIR)	$(OBJS_DIR)
	@$(MKDIR)	$(DEPS_DIR)

clean	:
	@$(RM)	$(OBJS_DIR)
	@$(RM)	$(DEPS_DIR)
	@$(RM) $(OBJS)
	@echo "$(RED)All temporary objects removed successfully${DEF_COLOR}"

fclean	:	clean
	@$(RM) $(NAME)
	@echo "$(RED)Executable have been fully cleaned${DEF_COLOR}"


re		:	fclean all

.PHONY: all clean fclean re

-include $(DEPS)
