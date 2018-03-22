/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
 *
 * This file is part of RAIM.
 *
 * RAIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RAIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/panic.h>
#include <raim/proc.h>
#include <raim/vmm.h>

void proctree_add_child(struct proc *child, struct proc *parent)
{
	child->parent = parent;
	child->next_sibling = parent->first_child;
	if (parent->first_child != NULL)
		parent->first_child->prev_sibling = child;
	parent->first_child = child;
}

void proctree_remove(struct proc *pos)
{
	if(pos->first_child != NULL)
		panic("non-empty proc_tree node.\n");
	if(pos->parent == NULL)
		panic("attempt to remove root node.\n");

	struct proc *father = pos->parent;
	if(pos == father->first_child)
		father->first_child = pos->next_sibling;
	if(pos->prev_sibling != NULL)
		pos->prev_sibling->next_sibling = pos->next_sibling;
	if(pos->next_sibling != NULL)
		pos->next_sibling->prev_sibling = pos->prev_sibling;
}

