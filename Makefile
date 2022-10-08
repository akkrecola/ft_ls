# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: elehtora <elehtora@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/09/29 13:01:10 by elehtora          #+#    #+#              #
#    Updated: 2022/10/06 02:46:14 by elehtora         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	:= ft_ls

SRCDIR	:= sources
SRCS	:= main.c \
		   parser.c \
		   reader.c \
		   sort.c \
		   list.c \
		   format.c

OBJDIR	:= objects
OBJS	:= $(SRCS:.c=.o)
OBJS	:= $(addprefix $(OBJDIR)/,$(OBJS))

LIBDIR	:= lib
LIBNAME	:= libftprintf.a
LIB		:= -L$(LIBDIR) -lftprintf

INCL	:= -Iincludes -I$(LIBDIR)/includes -I$(LIBDIR)/libft

CC		:= gcc
CFLAGS	:= -Wall -Werror -Wextra # Not specified by subject, but ?

RM		:= /bin/rm -rf

# Rules
all : $(NAME)

$(NAME) : $(OBJDIR) $(LIBNAME) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIB) -o $(NAME)

$(OBJDIR) :
	-mkdir -p $(OBJDIR)

$(LIBNAME) :
	$(MAKE) -C $(LIBDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCL) -c $< -o $@

clean :
	$(RM) $(OBJS) $(OBJDIR)
	$(MAKE) -C $(LIBDIR) clean

fclean : clean
	$(RM) $(NAME)
	$(MAKE) -C $(LIBDIR) fclean

re : fclean all
