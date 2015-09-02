// bsTree - binary search tree
/* $Id: bstree.h,v 1.1.1.1 2001/10/05 22:12:18 bitman Exp $ */
/* Copyright (C) 2001 Ryan Phillips <bitman@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */ 

#ifndef ___BSTREE_H
#define ___BSTREE_H

#include <iostream>
//#include <openssl/sha.h>
#include <string.h>

using namespace std;

template <class keytype, class thing>
struct bsTreeNode {
	keytype key;
	thing* data;

	bsTreeNode* less, * more;

	bsTreeNode(const keytype& _key, thing* _data)
		: key(_key)
		{ data = _data; less = more = 0; }
	~bsTreeNode(void) { if (less) delete less; if (more) delete more; }

	bsTreeNode* removeMe(void);
	thing* remove(const keytype& searchKey);
};

template <class keytype, class thing>
class bsTree {
protected:
//	bsTreeNode<keytype, thing>* root;
	bool myMemoryCleanup;
	
	void print_data(thing* data, ostream& out);
	void print_node(bsTreeNode<keytype, thing>* node, ostream& out);

	void destroyChildren(bsTreeNode<keytype, thing>* node);
	thing* get_node(bsTreeNode<keytype, thing>* node, const keytype& key);

public:
	bsTreeNode<keytype, thing>* root;
	bsTree(bool memoryCleanup = false)
		: root(0), myMemoryCleanup(memoryCleanup) {}
	~bsTree(void)
		{ if (root) { if (myMemoryCleanup) destroy(); else delete root; } }

	/* Node insertion functions */
	thing* insert(const keytype& key, thing* data);
	thing* insert(bsTreeNode<keytype, thing>* node, const keytype& key, thing* data);

	thing* remove(const keytype& key);

	thing* get(const keytype& key) { if (root) return get_node(root, key); else return 0; }

	/* Functions which call delete on things */
	void replace(const keytype& key, thing* data);
	void destroy(const keytype& key);
	void destroy(void) { if (root) { destroyChildren(root); delete root; root = 0; } }

	void print(ostream& out) { if (root) print_node(root, out); }
};

template <class keytype, class thing>
bsTreeNode<keytype, thing>* 
bsTreeNode<keytype, thing>::removeMe(void)
{
	bsTreeNode* replacement = NULL;
	bsTreeNode* dropPoint = NULL;

	if (more != NULL) {
		replacement = more;
		for (dropPoint = more; dropPoint->less != NULL; dropPoint = dropPoint->less)
			;
		dropPoint->less = less;
		less = more = NULL;
		return replacement;
	} else if (less != NULL) {
		replacement = less;
		less = more = NULL;
		return replacement;
	}
	return NULL;
}

template <class keytype, class thing>
thing* 
bsTreeNode<keytype, thing>::remove(const keytype& searchKey)
{
	bsTreeNode* oldNode = 0;
	thing* oldData = 0;

	if (less != NULL && searchKey < key) {
		if (less->key == searchKey) {
			oldNode = less;
			oldData = less->data;
			less = less->removeMe();
			delete oldNode;
			return oldData;
		} else
			return less->remove(searchKey);
	}
	else if (more != NULL && searchKey > key) {
		if (more->key == searchKey) {
			oldNode = more;
			oldData = more->data;
			more = more->removeMe();
			delete oldNode;
			return oldData;
		} else
			return more->remove(searchKey);
	}
	return 0;
}


/*

template <class keytype, class thing>
bsTreeNode<keytype, thing>::
*/

/******** bsTree IMPLEMENTATION ****************/

template <class keytype, class thing>
thing*
bsTree<keytype, thing>::insert(const keytype& key, thing* data)
{
	if (root) {
		return insert(root, key, data);
	} else {
		root = new bsTreeNode<keytype, thing>(key, data);
		return 0;
	}
}

template <class keytype, class thing>
thing*
bsTree<keytype, thing>::insert(bsTreeNode<keytype, thing>* node, const keytype& key, thing* data)
{
	if (key < node->key) {
		/* new node goes on lesser branch */
		if (node->less)
			return insert(node->less, key, data);
		else {
			node->less = new bsTreeNode<keytype, thing>(key, data);
			return 0;
		}
	} else if (key > node->key) {
		/* new node goes on greater branch */
		if (node->more)
			return insert(node->more, key, data);
		else {
			node->more = new bsTreeNode<keytype, thing>(key, data);
			return 0;
		}
	} else {
		/* new node goes replaces this node */
		thing* oldData = node->data;
		node->key = key;
		node->data = data;
		return oldData;
	}
}

template <class keytype, class thing>
void
bsTree<keytype, thing>::destroyChildren(bsTreeNode<keytype, thing>* node)
{
	if (node->less) {
		destroyChildren(node->less);
		delete node->less;
		node->less = 0;
	}
	if (node->more) {
		destroyChildren(node->more);
		delete node->more;
		node->more = 0;
	}

	delete node->data;
}


template <class keytype, class thing>
void 
bsTree<keytype, thing>::replace(const keytype& key, thing* data)
{
	thing* oldData = insert(key, data);
	if (oldData)
		delete oldData;
}

template <class keytype, class thing>
thing*
bsTree<keytype, thing>::remove(const keytype& searchKey)
{
	if (!root)
		return 0;

	if (root->key == searchKey) {
		bsTreeNode<keytype, thing>* oldNode = root;
		thing* oldData = root->data;
		root = root->removeMe();
		delete oldNode;
		return oldData;
	} else {
		return root->remove(searchKey);
	}
}

template <class keytype, class thing>
void 
bsTree<keytype, thing>::destroy(const keytype& key)
{
	thing* oldData = remove(key);
	if (oldData)
		delete oldData;
}


template <class keytype, class thing>
thing* 
bsTree<keytype, thing>::get_node(bsTreeNode<keytype, thing>* node, const keytype& key)
{
	if (!node) return 0;

	if (node->key == key)
		return node->data;
	else if (key < node->key)
		return get_node(node->less, key);
	else if (key > node->key)
		return get_node(node->more, key);
	else
		return 0;
}


template <class keytype, class thing>
void 
bsTree<keytype, thing>::print_node(bsTreeNode<keytype, thing>* node, ostream& out)
{
	if (node) {
		print_node(node->less, out);
		out << node->key;
		if (node->data) {
			out << " = ";
			print_data(node->data, out);
		}
		out << endl;
		print_node(node->more, out);
	}
}


template <class keytype, class thing>
void 
bsTree<keytype, thing>::print_data(thing* data, ostream& out)
{
	out << *data;
}

#endif
