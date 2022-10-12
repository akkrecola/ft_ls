/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reader.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elehtora <elehtora@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/01 11:18:09 by elehtora          #+#    #+#             */
/*   Updated: 2022/10/12 07:47:27 by elehtora         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ls.h"

static void	add_stat(t_flist *fnode, const char *dir)
{
	t_stat	stat;
	char	*path;

	path = ft_strdjoin(dir, "/", fnode->filename);
	if (!path)
		ls_error("Path allocation failed");
	if (lstat(path, &stat) == -1)
	{
		ls_read_error("", fnode->filename);
		fnode->stat = NULL;
		free(path);
		return ;
	}
	fnode->stat = (t_stat *)malloc(sizeof(stat));
	if (!fnode->stat && errno == ENOMEM)
		ls_error("Stat allocation failed");
	ft_memcpy(fnode->stat, &stat, sizeof(stat));
	free(path);
}

static t_flist	*get_fnode(t_options *op, char *filename, const char *path)
{
	t_flist	*fnode;

	if (!filename)
		ls_error("Invalid filename in get_fnode()");
	fnode = init_fnode();
	if (!fnode)
		ls_error("Initializing file node failed");
	fnode->filename = ft_strdup(filename);
	if (!fnode->filename)
		ls_error("Allocating memory to file name failed");
	if (errno == ENOMEM)
		ls_error("File name allocation failed");
	if (op->options & (O_LONG | MASK_TIME | O_REC))
		add_stat(fnode, path);
	return (fnode);
}

static t_flist	*collect_flist(t_flist **head, DIR *dirp, const char *path, t_options *op)
{
	t_flist		*fnode;
	t_flist		*last;
	t_dirent	*dirent;

	last = NULL;
	while (1)
	{
		dirent = readdir(dirp);
		if (errno == EBADF)
			ls_error("Reading directory stream failed");
		if (!dirent)
			return (*head);
		if (dirent->d_name[0] == '.' && !(op->options & O_ALL))
			continue ;
		fnode = get_fnode(op, dirent->d_name, path);
		if (*head == NULL)
			*head = fnode;
		last = append_flist(&last, fnode);
	}
	return (*head);
}

static void	recurse_directories(t_options *op, char *path, t_flist *flist)
{
	char	*dirpath;

	dirpath = NULL;
	while (flist)
	{
		if (flist->stat != NULL
				&& (flist->stat->st_mode & S_IFMT) == S_IFDIR // TODO Add permission mode checks
				&& !(ft_strequ(flist->filename, ".")
					|| ft_strequ(flist->filename, "..")))
		{
			dirpath = ft_strdjoin(path, "/", flist->filename);
			if (!dirpath)
				ls_error("Path name allocation failed");
			ft_printf("\n%s:\n", dirpath);
			list(op, dirpath);
		}
		flist = flist->next;
	}
}

void	list_dir(t_options *op, char *path)
{
	DIR			*dirp;
	t_flist		*flist;

	flist = NULL;
	dirp = opendir(path);
	if (!dirp)
		return (ls_read_error("", path));
	if (!collect_flist(&flist, dirp, path, op))
		ls_error("File list initialization failed");
	if (op->options & (O_LONG | O_MTIME))
		get_unique_forms(flist);
	sort(op, &flist);
	if (op->options & O_REV)
		flist = reverse_flist(flist, flist);
	format(op, flist, (const char *)path);
	if (op->options & O_REC)
		recurse_directories(op, path, flist);
	delete_flist(&flist);
	if (closedir(dirp) == -1)
		ls_error("Closing directory stream failed");
}

static void	list_file(t_options *op, char *path)
{
	t_flist	*fnode;
	char	*cond_filename = ft_strrchr(path, '/');

	if (cond_filename != NULL)
	{
		// Separate the path to filename and base by effectively splitting them.
		// 'path' still points to the allocated section, so it can be freed.
		*cond_filename = '\0';
		cond_filename += 1;
		fnode = get_fnode(op, cond_filename, path);
	}
	else
	{
		fnode = get_fnode(op, path, ".");
	}
	format(op, fnode, (const char *)path);
	delete_flist(&fnode);
}

void	list(t_options *op, char *path)
{
	t_stat		stat;

	if (lstat(path, &stat) == -1)
		ls_error("stat error");
	if ((stat.st_mode & S_IFMT) == S_IFDIR)
	{
		if (stat.st_mode & S_IXUSR)
			list_dir(op, path);
	}
	else
	{
		list_file(op, path);
	}
	free(path);
}

/* Iterate through file arguments
*/
void	list_args(t_options *op, char **argv, int argc)
{
	char	*path;

	// No file/directory arguments
	if (argc == 0)
		list(op, ft_strdup("."));
	while (argc--)
	{
		path = ft_strdup(*argv);
		list(op, path);
		argv++;
	}
}
