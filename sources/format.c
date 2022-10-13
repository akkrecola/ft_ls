/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   format.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elehtora <elehtora@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/06 02:04:25 by elehtora          #+#    #+#             */
/*   Updated: 2022/10/13 19:28:05 by elehtora         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ls.h"

static void	init_fwidths(t_fwidths *fwidths)
{
	fwidths->links_len = 0;
	fwidths->author_len = 0;
	fwidths->group_len = 0;
	fwidths->size_len = 0;
	fwidths->total_blocks = 0;
	fwidths->major = 0;
	fwidths->minor = 0;
}

/* Gets the common attributes for the long format, so that the field widths
 * can be aligned correctly.
 */
static void	get_common_widths(t_fwidths *fwidths, t_flist *flist)
{
	init_fwidths(fwidths);
	while (flist)
	{
		if (flist->stat != NULL)
		{
			if (ft_count_digs(flist->stat->st_nlink) > fwidths->links_len)
				fwidths->links_len = ft_count_digs(flist->stat->st_nlink);
			if (ft_strlen(flist->lform->author) > fwidths->author_len)
				fwidths->author_len = ft_strlen(flist->lform->author);
			if (ft_strlen(flist->lform->group) > fwidths->group_len)
				fwidths->group_len = ft_strlen(flist->lform->group);
			if (ft_count_digs(flist->stat->st_size) > fwidths->size_len)
				fwidths->size_len = ft_count_digs(flist->stat->st_size);
			if (ft_count_digs(flist->lform->major) > fwidths->major)
				fwidths->major = ft_count_digs(flist->lform->major);
			if (ft_count_digs(flist->lform->minor) > fwidths->minor)
				fwidths->minor = ft_count_digs(flist->lform->minor);
			fwidths->total_blocks += flist->stat->st_blocks;
		}
		flist = flist->next;
	}
	if (fwidths->minor > 3)
		fwidths->minor = 3;
}

static t_longform	*init_lform(void)
{
	t_longform	*lform;

	lform = (t_longform *)malloc(sizeof(t_longform));
	if (!lform)
		ls_critical_error("Long format allocation failed");
	lform->hardlinks = 0;
	lform->size = 0;
	lform->author = NULL;
	lform->group = NULL;
	lform->date = NULL;
	lform->major = 0;
	lform->minor = 0;
	return (lform);
}

#define MAJOR_MASK 0x7FF00000
#define MINOR_MASK 0xFFFFF
static void	get_device_id(t_flist *fnode, t_longform *lform)
{
	lform->major = (fnode->stat->st_rdev & (MAJOR_MASK)) >> 24;
	lform->minor = (fnode->stat->st_rdev & (MINOR_MASK));
}

void	get_unique_forms(t_flist *fnode)
{
	t_longform	*lform;
	t_passwd	*passwd;
	t_group		*group;

	while (fnode)
	{
		if (fnode->stat != NULL)
		{
			lform = init_lform();
			passwd = getpwuid(fnode->stat->st_uid);
			if (!passwd) // No match, display UID
				lform->author = ft_itoa(fnode->stat->st_uid);
			else
				lform->author = ft_strdup(passwd->pw_name);
			group = getgrgid(fnode->stat->st_gid);
			if (!group)
				lform->group = ft_itoa(fnode->stat->st_gid);
			else
				lform->group = ft_strdup(group->gr_name);
			if (!lform->author || !lform->group)
				ls_critical_error("Allocation of author or group strings failed");
			if ((fnode->stat->st_mode & S_IFMT) == S_IFCHR || \
				(fnode->stat->st_mode & S_IFMT) == S_IFBLK)
				get_device_id(fnode, lform);
			fnode->lform = lform;
		}
		fnode = fnode->next;
	}
}

static time_t	get_time(t_flist *fnode, t_options *op)
{
	if ((op->options & MASK_TIME) == O_MTIME)
		return (fnode->stat->st_mtime);
	/*else if ((op->options & MASK_TIME) == O_ATIME)*/
	/*return (fnode->stat->st_atime);*/
	/*else if ((op->options & MASK_TIME) == O_CTIME)*/
	/*return (fnode->stat->st_ctime);*/
	return (fnode->stat->st_mtime); // default
}

/* Format the date to a desired string format. The one received
 * from ctime(3) is not readily usable.
 *
 * Required format: <Mmm> <DD> <HH:MM>
 * Exception: if the time is 6 mo in the past or future, display
 * year instead of HH:MM
 */
#define SECS_IN_6_MONTHS 15778463
static void	output_date(time_t format_time)
{
	const char	*unformatted_date = ctime(&format_time);
	char		datebuf[DATE_FW + 1];

	ft_bzero(datebuf, DATE_FW);
	ft_memcpy(datebuf, unformatted_date + 4, 6); // Month + Day
	ft_memcpy(datebuf + 6, " ", 1);
	if (ft_labs(format_time - time(NULL)) > SECS_IN_6_MONTHS)
		ft_strncat(datebuf, unformatted_date + 19, 5); // Changed from + 20, 4 to accommodate preceding space
	else
		ft_strncat(datebuf, unformatted_date + 11, 5);
	ft_printf("%s", datebuf);
}

#define READLINK_BUFSIZE 1024
static void	resolve_link(t_flist *fnode, const char *base, t_options *op)
{
	char	*path;
	char	buf[READLINK_BUFSIZE];

	path = ft_strdjoin(base, "/", fnode->filename);
	if (!path)
		ls_critical_error("Path allocation failed");
	ft_bzero(buf, READLINK_BUFSIZE);
	if (readlink(path, buf, READLINK_BUFSIZE) == -1)
	{
		ls_read_error("", fnode->filename, op, E_MINOR);
		return (free(path));
	}
	free(path);
	ft_printf(" -> %s", buf);
}

static void	print_sizeblock(t_flist *fnode, t_fwidths *fwidths)
{
	if ((fnode->stat->st_mode & S_IFMT) == S_IFCHR \
			|| (fnode->stat->st_mode & S_IFMT) == S_IFBLK)
	{
		if (fnode->lform->minor > 512)
			ft_printf(" %*u, %#*x ", fwidths->major, fnode->lform->major, \
					fwidths->minor, fnode->lform->minor);
		else
			ft_printf(" %*u, %*u ", fwidths->major, fnode->lform->major, \
					fwidths->minor, fnode->lform->minor);
	}
	else
		ft_printf("%*u ", fwidths->size_len, fnode->stat->st_size);

}

static void	print_longform(t_flist *flist, t_options *op, const char *path)
{
	t_fwidths	fwidths;

	get_common_widths(&fwidths, flist);
	ft_printf("total %lu\n", fwidths.total_blocks);
	while (flist)
	{
		if (flist->stat != NULL)
		{
			print_permissions(flist);
			ft_printf("%*u ", fwidths.links_len, flist->stat->st_nlink);
			ft_printf("%-*s  ", fwidths.author_len, flist->lform->author);
			ft_printf("%-*s  ", fwidths.group_len, flist->lform->group);
			print_sizeblock(flist, &fwidths);
			output_date(get_time(flist, op));
			ft_printf(" %s", flist->filename);
			if ((flist->stat->st_mode & S_IFMT) == S_IFLNK)
				resolve_link(flist, path, op);
			ft_printf("\n");
		}
		flist = flist->next;
	}
}

void	format(t_options *op, t_flist *flist, const char *path)
{
	if (op->options & O_LONG)
	{
		print_longform(flist, op, path);
	}
	else
	{
		while (flist)
		{
			ft_printf("%s\n", flist->filename);
			flist = flist->next;
		}
	}
}
