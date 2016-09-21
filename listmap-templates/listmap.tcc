// $Id: listmap.tcc,v 1.7 2015-04-28 19:22:02-07 - - $

#include "listmap.h"
#include "trace.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::node.
/////////////////////////////////////////////////////////////////
//

//
// listmap::node::node (link*, link*, const value_type&)
//
template <typename Key, typename Value, class Less>
listmap<Key, Value, Less>::node::node (node* next, node* prev,
                                       const value_type& value):
   link (next, prev), value (value) {
}

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename Key, typename Value, class Less>
listmap<Key, Value, Less>::~listmap() {
   TRACE ('l', (void*) this);
}


//
// iterator listmap::insert (const value_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator
listmap<Key, Value, Less>::insert (const value_type& pair) {
   TRACE ('l', &pair << "->" << pair);
   for (auto itor = begin(); itor != end(); ++itor) {
      //if the key already exist, replace the value
      if (!(less(pair.first, (*itor).first)) and !(less((*itor).first, pair.first))) {
         (*itor).second = pair.second;
         return itor;
      } else if (less(pair.first, (*itor).first)) {
         //if the key is new, insert it using insertion sort
         node* Node = new node(itor.return_node(), itor.return_node()->prev, pair);
         itor.return_node()->prev->next = Node;
         itor.return_node()->prev = Node;
         return itor;
      }
   }
   //at this point, new key is the current largest key in the map.
   auto itor = end();
   node* Node = new node(itor.return_node(), itor.return_node()->prev, pair);
   itor.return_node()->prev->next = Node;
   itor.return_node()->prev = Node;
   return itor;
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator
listmap<Key, Value, Less>::find (const key_type& that) {
   TRACE ('l', that);
   for (auto itor = begin(); itor != end(); ++itor) {
      if (itor->first == that) return itor;
   }
   return end(); //if no value is found, return end()

}

//
// iterator listmap::erase (iterator position)
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator
listmap<Key, Value, Less>::erase (iterator position) {
   TRACE ('l', &*position);
   auto Node = position.return_node();
   Node->next->prev = Node->prev;
   Node->prev->next = Node->next;
   delete Node;
   return ++position;
}


//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

//
// listmap::value_type& listmap::iterator::operator*()
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::value_type&
listmap<Key, Value, Less>::iterator::operator*() {
   TRACE ('l', where);
   return where->value;
}

//
// listmap::value_type* listmap::iterator::operator->()
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::value_type*
listmap<Key, Value, Less>::iterator::operator->() {
   TRACE ('l', where);
   return &(where->value);
}

//
// listmap::iterator& listmap::iterator::operator++()
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator&
listmap<Key, Value, Less>::iterator::operator++() {
   TRACE ('l', where);
   where = where->next;
   return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template <typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator&
listmap<Key, Value, Less>::iterator::operator--() {
   TRACE ('l', where);
   where = where->prev;
   return *this;
}


//
// bool listmap::iterator::operator== (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key, Value, Less>::iterator::operator==
(const iterator& that) const {
   return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key, Value, Less>::iterator::operator!=
(const iterator& that) const {
   return this->where != that.where;
}

template <typename Key, typename Value, class Less>
typename  listmap<Key, Value, Less>::node*
listmap<Key, Value, Less>::iterator::return_node() {
   return where; //return node* for the iterator.
}

