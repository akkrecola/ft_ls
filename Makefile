# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: elehtora <elehtora@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/09/29 13:01:10 by elehtora          #+#    #+#              #
#    Updated: 2022/10/18 05:08:08 by elehtora         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	:= ft_ls

SRCDIR	:= sources
SRCS	:= args.c \
		   main.c \
		   comparisons.c \
		   error.c \
		   flist.c \
		   flist_utils.c \
		   format.c \
		   longform.c \
		   longform_common.c \
		   longform_unique.c \
		   parser.c \
		   permissions.c \
		   reader.c \
		   sort.c \
		   time.c

OBJDIR	:= objects
OBJS	:= $(SRCS:.c=.o)
OBJS	:= $(addprefix $(OBJDIR)/,$(OBJS))

LIBDIR	:= lib
LIBNAME	:= libftprintf.a
LIB		:= -L$(LIBDIR) -lftprintf

INCL	:= -Iincludes -I$(LIBDIR)/includes -I$(LIBDIR)/libft

CC		:= gcc
CFLAGS	:= -Wall -Werror -Wextra -O3

RM		:= /bin/rm -rf

# Rules
all : build

$(NAME) : $(OBJS)
	@echo "Linking executable:\t\033[1;32m$(NAME)\033[0m"
	@$(CC) $(CFLAGS) $(OBJS) $(LIB) -o $(NAME)

build : $(OBJDIR)
	@$(MAKE) -C $(LIBDIR)
	@$(MAKE) $(NAME)

$(OBJDIR) :
	@-mkdir -p $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@echo "Creating object file:\t\033[1;32m$(notdir $(<:.c=.o))\033[0m"
	@$(CC) $(CFLAGS) $(INCL) -c $< -o $@

clean :
	@echo "Cleaned object files of \033[1;32m$(NAME)\033[0m"
	@$(RM) $(OBJS) $(OBJDIR)
	@$(MAKE) -C $(LIBDIR) clean

fclean : clean
	@echo "Cleaned build executable of \033[1;32m$(NAME)\033[0m"
	@$(RM) $(NAME)
	@$(MAKE) -C $(LIBDIR) fclean

re : fclean all
