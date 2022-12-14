/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   flist_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elehtora <elehtora@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/13 20:42:54 by elehtora          #+#    #+#             */
/*   Updated: 2022/10/18 05:57:16 by elehtora         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ls.h"

size_t	len_flist(t_flist *flist)
{
	size_t	len;

	len = 0;
	while (flist)
	{
		len++;
		flist = flist->next;
	}
	return (len);
}

static void	add_stat(t_flist *fnode, const char *path, t_options *op)
{
	t_stat	stat;

	if (lstat(path, &stat) == -1)
	{
		ls_read_error("", fnode->filename, op, E_MINOR);
		fnode->stat = NULL;
	}
	else
	{
		fnode->stat = (t_stat *)malloc(sizeof(stat));
		if (!fnode->stat && errno == ENOMEM)
			ls_critical_error("stat allocation failed");
		ft_memcpy(fnode->stat, &stat, sizeof(stat));
	}
}

t_flist	*get_fnode(const char *path, t_options *op)
{
	t_flist	*fnode;

	fnode = init_fnode();
	if (!fnode)
		ls_critical_error("initializing file node failed");
	fnode->filename = ft_basename(path);
	fnode->path = ft_strdup(path);
	if (!fnode->filename || !fnode->path)
		ls_critical_error("filename or path allocation failed");
	add_stat(fnode, path, op);
	return (fnode);
}

t_flist	*collect_flist(DIR *dirp, const char *dirname, t_options *op)
{
	t_flist		*flist;
	t_dirent	*dirent;
	char		*path;

	flist = NULL;
	path = NULL;
	while (1)
	{
		dirent = readdir(dirp);
		if (errno == EBADF)
			ls_critical_error("Reading directory stream failed");
		if (!dirent)
			return (flist);
		if (dirent->d_name[0] == '.' && !(op->options & O_ALL))
			continue ;
		path = ft_join_path((char *)dirname, dirent->d_name);
		if (!path)
			ls_critical_error("allocating path failed");
		append_fnode(&flist, get_fnode(path, op));
		free(path);
	}
	return (flist);
}
