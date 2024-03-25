/***********************************************************************
 * Header:
 *    LIST
 * Summary:
 *    Our custom implementation of std::list
 *      __      __     _______        __
 *     /  |    /  |   |  _____|   _  / /
 *     `| |    `| |   | |____    (_)/ /
 *      | |     | |   '_.____''.   / / _
 *     _| |_   _| |_  | \____) |  / / (_)
 *    |_____| |_____|  \______.' /_/
 *
 *    This will contain the class definition of:
 *        List         : A class that represents a List
 *        ListIterator : An iterator through List
 * Author
 *       Marco Varela & Andre Regino (02/07/2024)
 ************************************************************************/

#pragma once
#include <cassert>     // for ASSERT
#include <iostream>    // for nullptr
#include <new>         // std::bad_alloc
#include <memory>      // for std::allocator

class TestList; // forward declaration for unit tests
class TestHash; // forward declaration for hash used later

namespace custom
{

/**************************************************
 * LIST
 * Just like std::list
 **************************************************/
template <typename T, typename A = std::allocator<T>>
class list
{
   friend class ::TestList; // give unit tests access to the privates
   friend class ::TestHash;
   friend void swap(list& lhs, list& rhs);
public:
   
   //
   // Construct
   //
   list(const A& a = A())
   {
      numElements = 0;
      pHead = pTail = nullptr;
   }
   
   list(list <T, A> & rhs, const A& a = A())
   {
      numElements = 0;
      pHead = pTail = nullptr;
      if (rhs.size() > 0 )
         // Add every element from the given list
         for (auto it = rhs.begin(); it != rhs.end(); it++)
            push_back(*it);
   }
   list(list <T, A>&& rhs, const A& a = A());
   list(size_t num, const T & t, const A& a = A());
   list(size_t num, const A& a = A());
   list(const std::initializer_list<T>& il, const A& a = A())
   {
      numElements = 0;
      pHead = pTail = nullptr;
      
      // Add every element from the initializer list
      for (auto& elem : il)
         push_back(elem);
   }
   template <class Iterator>
   list(Iterator first, Iterator last, const A& a = A())
   {
      numElements = 0;
      pHead = pTail = nullptr;
      for (auto it = first; it != last; it++)
         push_back(*it);
   }
   ~list()
   {
      while (pHead != nullptr)
      {
         // Erase every element in the list
         Node* temp = pHead;
         pHead = pHead->pNext;
         delete temp;
      }
      pTail = nullptr;
   }
   
   //
   // Assign
   //
   
   list <T, A> & operator = (list <T, A> &  rhs);
   list <T, A> & operator = (list <T, A> && rhs);
   list <T, A> & operator = (const std::initializer_list<T>& il);
   void swap(list <T, A>& rhs)
   {
      std::swap(pHead, rhs.pHead);
      std::swap(pTail, rhs.pTail);
      std::swap(numElements, rhs.numElements);
   }

   //
   // Iterator
   //
   
   class iterator;
   iterator begin()  { return iterator (pHead);   }
   iterator rbegin() { return iterator (pTail);   }
   iterator end()    { return iterator (nullptr); }
   
   //
   // Access
   //
   
   T & front();
   T & back();

   //
   // Insert
   //
   
   void push_front(const T &  data);
   void push_front(      T && data);
   void push_back (const T &  data);
   void push_back (      T && data);
   iterator insert(iterator it, const T &  data);
   iterator insert(iterator it,       T && data);

   //
   // Remove
   //
   
   void pop_back();
   void pop_front();
   void clear();
   iterator erase(const iterator & it);

   //
   // Status
   //
   
   bool empty()  const { return size() == 0; }
   size_t size() const { return numElements;   }

private:
   // nested linked list class
   class Node;

   // member variables
   A    alloc;         // use alloacator for memory allocation
   size_t numElements; // though we could count, it is faster to keep a variable
   Node * pHead;       // pointer to the beginning of the list
   Node * pTail;       // pointer to the ending of the list
};

/*************************************************
 * NODE
 * the node class.  Since we do not validate any
 * of the setters, there is no point in making them
 * private.  This is the case because only the
 * List class can make validation decisions
 *************************************************/
template <typename T, typename A>
class list <T, A> :: Node
{
public:
   //
   // Construct
   //
   Node() : pNext(nullptr), pPrev(nullptr) {}

   Node(const T& data) : data(data), pNext(nullptr), pPrev(nullptr) {}


   Node(T&& data) : data(std::move(data)), pNext(nullptr), pPrev(nullptr) {}


   //
   // Member Variables
   //

   T data;             // user data
   Node * pNext;       // pointer to next node
   Node * pPrev;       // pointer to previous node
};


/***********************************************
 * NODE::COPY
 * Copy the list from the pSource and return
 * the new list
 *   INPUT  : the list to be copied
 *   OUTPUT : return the new list
 *   COST   : O(n)
 **********************************************/
template <typename T, typename A>
typename list<T, A>::Node * copy(const typename list<T, A>::Node * pSource)
{
   if (pSource == nullptr)
      return nullptr;
   
   typename list<T, A>::Node * pDestination = new typename list<T, A>::Node(pSource->data);
   const typename list<T, A>::Node * pSrc = pSource;
   typename list<T, A>::Node * pDes = pDestination;

   for (pSrc = pSrc->pNext; pSrc; pSrc = pSrc->pNext)
      pDes = insert(pDes,pSrc->data, true);
   
   return pDestination;
}

/***********************************************
 * NODE:: ASSIGN
 * Copy the values from pSource into pDestination
 * reusing the nodes already created in pDestination if possible.
 *   INPUT  : the list to be copied
 *   OUTPUT : return the new list
 *   COST   : O(n)
 **********************************************/
template <typename T, typename A>
inline void assign(typename list<T, A>::Node * & pDestination, const typename list<T, A>::Node * pSource)
{
   // Check if both lists are empty.
   if (pSource == nullptr && pDestination == nullptr)
      return;
   
   typename list<T, A>::Node * pDesPrevious = pDestination;
   typename list<T, A>::Node * pDesHead = nullptr;
   
   // Remember the head only if pDestination is not null
   if (pDestination != nullptr)
   {
      pDesHead = pDestination;
   }
   
   
   // Copy nodes from pSource to pDestination.
   while (pSource != nullptr && pDestination != nullptr)
   {
      pDestination->data = pSource->data;
      pDesPrevious = pDestination;
      pDestination = pDestination->pNext;
      pSource = pSource->pNext;
   }
   
   // Check if source is longer than destination.
   if (pSource != nullptr)
   {
      typename list<T, A>::Node * pDes = pDesPrevious;
      
      // Insert all nodes needed.
      while (pSource != nullptr)
      {
         pDes = insert(pDes, pSource->data, true);
         
         if (pDestination == nullptr)
            pDestination = pDes;
         
         pSource = pSource->pNext;
      }
      
      if (pDesHead != nullptr)
      {
         // Set pDestination to the original head.
         pDestination = pDesHead;
      }
   }
   
   // Check if destination is larger than the source.
   else if (pSource == nullptr && pDestination != nullptr)
   {
      bool setToNull = false;
      
      // Disconect last needed node to the rest.
      if (pDestination->pPrev != nullptr)
      {
         pDestination->pPrev->pNext = nullptr;
         
         // Delete every node that is left.
         while (pDestination != nullptr)
         {
            typename list<T, A>::Node * temp = pDestination;
            pDestination = pDestination->pNext;
            delete temp;
         }
         
         // Set pDestination to the original head.
         pDestination = pDesHead;
      }
      else
      {
         setToNull = true;

         // Delete all data from list
         clear(pDestination);
      }
      

      if (setToNull)
         pDestination = nullptr;
   
   }
}

/***********************************************
 * NODE:: SWAP
 * Swap the list from LHS to RHS
 *   COST   : O(1)
 **********************************************/
template <typename T, typename A>
inline void swap(typename list<T, A>::Node* &pLHS, typename list<T, A>::Node* &pRHS)
{
   std :: swap(pLHS, pRHS);
}

/***********************************************
 * NODE:: REMOVE
 * Remove the node pSource in the linked list
 *   INPUT  : the node to be removed
 *   OUTPUT : the pointer to the parent node
 *   COST   : O(1)
 **********************************************/
template <typename T, typename A>
inline typename list<T, A>::Node * remove(const typename list<T, A>::Node * pRemove)
{
   typename list<T, A>::Node* pReturn = nullptr;
   
   // Check if there is nothing to remove
   if (pRemove == nullptr)
   {
      return nullptr;
   }
      
   
   // Link current previous and current next together
   if (pRemove->pNext)
   {
      pRemove->pNext->pPrev = pRemove->pPrev;
   }
   if (pRemove->pPrev)
   {
      pRemove->pPrev->pNext = pRemove->pNext;
   }
   
   // Return a pointer to the current previous if exists
   if (pRemove->pPrev)
   {
      pReturn = pRemove->pPrev;
   }
   
   // Return a pointer to the current next
   else
   {
      pReturn = pRemove->pNext;
   }
   
   // Delete the node we were given
   delete pRemove;
   
   
   return pReturn;
}


/**********************************************
 * NODE:: INSERT
 * Insert a new node the the value in "t" into a linked
 * list immediately before the current position.
 *   INPUT   : t - the value to be used for the new node
 *             pCurrent - a pointer to the node before which
 *                we will be inserting the new node
 *             after - whether we will be inserting after
 *   OUTPUT  : return the newly inserted item
 *   COST    : O(1)
 **********************************************/
template <typename T, typename A>
inline typename list<T, A>::Node* insert(typename list<T, A>::Node* pCurrent, const T& t, bool after = false)
{
   // Create a node with the given value
   typename list<T, A>::Node* newNode = new typename list<T, A>::Node(t);
   
   // Confirm if we receive a null pointer
   if (!pCurrent)
   {
      newNode->pNext = nullptr;
      newNode->pPrev = nullptr;
      return newNode;
   }

   // Check if we insert to the left
   if (!after)
   {
      // Set the new node's prev and next from current
      newNode->pNext = pCurrent;
      newNode->pPrev = pCurrent->pPrev;
      
      // Link current's prev node to the new node
      if (pCurrent->pPrev)
         pCurrent->pPrev->pNext = newNode;
      
      // Link the current node to the new node
      pCurrent->pPrev = newNode;
   }
   
   // Insert to the right
   else
   {
      // Set the new node's prev and next from current
      newNode->pNext = pCurrent->pNext;
      newNode->pPrev = pCurrent;
      
      // Link current's next node to the new node
      if (pCurrent->pNext)
         pCurrent->pNext->pPrev = newNode;
      
      // Link the current node to the new node
      pCurrent->pNext = newNode;
   }

   return newNode;
}

/******************************************************
 * NODE:: SIZE
 * Find the size an unsorted linked list.
 *  INPUT   : a pointer to the head of the linked list
 *            the value to be found
 *  OUTPUT  : number of nodes
 *  COST    : O(n)
 ********************************************************/
template <typename T, typename A>
inline size_t size(const typename list<T, A>::Node * pHead)
{
   size_t count = 0;
   while (pHead != nullptr)
   {
      count += 1;
      pHead = pHead->pNext;
 
   }
   return count;
}

/***********************************************
 * NODE:: DISPLAY
 * Display all the items in the linked list from here on back
 *    INPUT  : the output stream
 *             pointer to the linked list
 *    OUTPUT : the data from the linked list on the screen
 *    COST   : O(n)
 **********************************************/
template <typename T, typename A>
inline std::ostream & operator << (std::ostream & out, const typename list<T, A>::Node * pHead)
{
   return out;
}

/*****************************************************
 * NODE:: FREE DATA
 * Free all the data currently in the linked list
 *   INPUT   : pointer to the head of the linked list
 *   OUTPUT  : pHead set to NULL
 *   COST    : O(n)
 ****************************************************/
template <typename T, typename A>
inline void clear(typename list<T, A>::Node * & pHead)
{
   typename list<T, A>::Node* deleteNode = nullptr;
   while (pHead != nullptr)
   {
      deleteNode = pHead;
      pHead = pHead->pNext;
      delete deleteNode;
  
   }
}





/*************************************************
 * LIST ITERATOR
 * Iterate through a List, non-constant version
 ************************************************/
template <typename T, typename A>
class list <T, A> :: iterator
{
   friend class ::TestList; // give unit tests access to the privates
   friend class ::TestHash;
   template <typename TT, typename AA>
   friend class custom::list;
   
public:
   // constructors, destructors, and assignment operator
   iterator()
   {
      p = nullptr;
   }
   iterator(Node * pRHS)
   {
      p = pRHS;
   }
   iterator(const iterator  & rhs)
   {
      if (rhs.p == nullptr)
         p = nullptr;
      else
         p = rhs.p;
   }
   iterator & operator = (const iterator & rhs)
   {
      p = rhs.p;
      return *this;
   }
   
   // equals, not equals operator
   bool operator == (const iterator & rhs) const { return p == rhs.p; }
   bool operator != (const iterator & rhs) const { return p != rhs.p; }

   // dereference operator, fetch a node
   T & operator * ()
   {
      return p->data;
   }

   // postfix increment
   iterator operator ++ (int postfix)
   {
      iterator temp(*this);
      p = p->pNext;
      return temp;
   }

   // prefix increment
   iterator & operator ++ ()
   {
      p = p->pNext;
      return *this;
   }
   
   // postfix decrement
   iterator operator -- (int postfix)
   {
      iterator temp(*this);
      p = p->pPrev;
      return temp;
   }

   // prefix decrement
   iterator & operator -- ()
   {
      p = p->pPrev;
      return *this;
   }

   // two friends who need to access p directly
   friend iterator list <T, A> :: insert(iterator it, const T &  data);
   friend iterator list <T, A> :: insert(iterator it,       T && data);
   friend iterator list <T, A> :: erase(const iterator & it);

private:

   typename list <T, A> :: Node * p;
};

/*****************************************
 * LIST :: NON-DEFAULT constructors
 * Create a list initialized to a value
 ****************************************/
template <typename T, typename A>
list <T, A> ::list(size_t num, const T & t, const A& a)
{
   numElements = num;
   pHead = nullptr;
   pTail = nullptr;
   if (num > 0)
   {
      // Create a new node with the provided data
      pHead = new Node (t);
      pHead->pPrev = nullptr;

      Node *pPrevious = pHead;
      for (size_t i = 1; i < num; ++i)
      {
         // Create a new node with the provided data
         Node *pNew = new Node(t);
         
         //Insert a new node into the list
         pNew->pPrev = pPrevious;
         pPrevious->pNext = pNew;
         pPrevious = pNew;
      }

      pTail = pPrevious;
      pTail->pNext = nullptr;
   }
}

/*****************************************
 * LIST :: NON-DEFAULT constructors
 * Create a list initialized to a value
 ****************************************/
template <typename T, typename A>
list <T, A> ::list(size_t num, const A& a)
{
   numElements = num;
   pHead = nullptr;
   pTail = nullptr;
   if (num > 0)
   {
      //Create a new node
      pHead = new Node;
      pHead->pPrev = nullptr;

      Node *pPrevious = pHead;
      for (size_t i = 1; i < num; ++i)
      {
         //Insert a new node into the list
         Node *pNew = new Node;
         pNew->pPrev = pPrevious;
         pPrevious->pNext = pNew;
         pPrevious = pNew;
      }

      pTail = pPrevious;
      pTail->pNext = nullptr;
   }
}

/*****************************************
 * LIST :: MOVE constructors
 * Steal the values from the RHS
 ****************************************/
template <typename T, typename A>
list <T, A> ::list(list <T, A>&& rhs, const A& a) :
   numElements(rhs.numElements), pHead(rhs.pHead), pTail(rhs.pTail), alloc(a)
{
   rhs.pHead = rhs.pTail = nullptr;
   rhs.numElements = 0;
}

/**********************************************
 * LIST :: assignment operator - MOVE
 * Copy one list onto another
 *     INPUT  : a list to be moved
 *     OUTPUT :
 *     COST   : O(n) with respect to the size of the LHS
 *********************************************/
template <typename T, typename A>
list <T, A>& list <T, A> :: operator = (list <T, A> && rhs)
{
   clear();
   swap(rhs);
   
   return *this;
}
/**********************************************
 * LIST :: assignment operator
 * Copy one list onto another
 *     INPUT  : a list to be copied
 *     OUTPUT :
 *     COST   : O(n) with respect to the number of nodes
 *********************************************/
template <typename T, typename A>
list <T, A> & list <T, A> :: operator = (list <T, A> & rhs)
{
   iterator itRHS = rhs.begin();
   iterator itLHS = begin();
   
   // Iterate through given list and this
   while( itRHS != rhs.end() && itLHS != end())
   {
      // Copy element into current node
      *itLHS = *itRHS;
      ++itRHS;
      ++itLHS;
   }
   
   // Check if rhs still has nodes
   if (itRHS != rhs.end())
   {
      // Create an push new nodes
      while (itRHS != rhs.end())
      {
         push_back(*itRHS);
         ++itRHS;
      }
   }
   
   // Check if rhs is empty
   else if (rhs.empty())
      
      // Empty current list
      clear();
   
   // Check if our list still has nodes
   else if (itLHS != end())
   {
      Node * p = itLHS.p;
      pTail = p->pPrev;
      Node *pNext = p->pNext;
      
      // Delete every extra nodes
      while (p != nullptr)
      {
         pNext = p->pNext;
         delete p;
         p = pNext;
         numElements--;
         pTail->pNext = nullptr;
      }
   }
   
   return *this;
}

/**********************************************
 * LIST :: assignment operator
 * Copy one list onto another
 *     INPUT  : a list to be copied
 *     OUTPUT :
 *     COST   : O(n) with respect to the number of nodes
 *********************************************/
template <typename T, typename A>
list <T, A>& list <T, A> :: operator = (const std::initializer_list<T>& rhs)
{
   
   auto itRHS = rhs.begin();
   iterator itLHS = begin();
   
   // Iterate through given list and this
   while( itRHS != rhs.end() && itLHS != end())
   {
      // Copy element into current node
      *itLHS = *itRHS;
      ++itRHS;
      ++itLHS;
   }
   
   // Check if rhs still has nodes
   if (itRHS != rhs.end())
   {
      // Create an push new nodes
      while (itRHS != rhs.end())
      {
         push_back(*itRHS);
         ++itRHS;
      }
   }
   
   // Check if rhs is empty
   else if (rhs.size() == 0)
      
      // Empty current list
      clear();
   
   // Check if our list still has nodes
   else if (itLHS != end())
   {
      Node * p = itLHS.p;
      pTail = p->pPrev;
      Node *pNext = p->pNext;
      
      // Delete every extra nodes
      while (p != nullptr)
      {
         pNext = p->pNext;
         delete p;
         p = pNext;
         numElements--;
         pTail->pNext = nullptr;
      }
   }
   
   return *this;
}

/**********************************************
 * LIST :: CLEAR
 * Remove all the items currently in the linked list
 *     INPUT  :
 *     OUTPUT :
 *     COST   : O(n) with respect to the number of nodes
 *********************************************/
template <typename T, typename A>
void list <T, A> :: clear()
{
   Node * p = pHead;
   
   // Delete every item in the list
   while (p != nullptr)
   {
      Node * pNext = p->pNext;
      delete p;
      p = pNext;
   }
   pHead = pTail = nullptr;
   numElements = 0;
}

/*********************************************
 * LIST :: PUSH BACK
 * add an item to the end of the list
 *    INPUT  : data to be added to the list
 *    OUTPUT :
 *    COST   : O(1)
 *********************************************/
template <typename T, typename A>
void list <T, A> :: push_back(const T & data)
{
   // Create a new node with the given data
   Node* pNew = new Node(data);
   pNew->pPrev = pTail;
   
   //Check if the list is not empty
   if (pTail)
      
      // Add to the end
      pTail->pNext = pNew;
   else
      
      // Add to the front
      pHead = pNew;
   
   pTail = pNew;
   numElements++;
}

template <typename T, typename A>
void list <T, A> ::push_back(T && data)
{
   // Create a new node with the given data
   Node* pNew = new Node(std::forward<T>(data));
   pNew->pPrev = pTail;
   
   //Check if the list is not empty
   if (pTail)
      
      // Add to the end
     pTail->pNext = pNew;
   else
      
     // Add to the front
     pHead = pNew;
   
   pTail = pNew;
   numElements++;
}

/*********************************************
 * LIST :: PUSH FRONT
 * add an item to the head of the list
 *     INPUT  : data to be added to the list
 *     OUTPUT :
 *     COST   : O(1)
 *********************************************/
template <typename T, typename A>
void list <T, A> :: push_front(const T & data)
{
   // Create a new node with the given data
   Node* pNew = new Node(data);
   pNew->pNext = pHead;
   
   //Check if the list is not empty
   if (pHead)
      
      // Add to the front
      pHead->pPrev = pNew;
   else
      
      // Add to the end
      pTail = pNew;
   pHead = pNew;
   numElements++;
}

template <typename T, typename A>
void list <T, A> ::push_front(T && data)
{
   // Create a new node with the given data
   Node* pNew = new Node(std::forward<T>(data));
   pNew->pNext = pHead;
   
   //Check if the list is not empty
   if (pHead)
      
      // Add to the front
      pHead->pPrev = pNew;
   else
      
      // Add to the end
      pTail = pNew;
   pHead = pNew;
   numElements++;
}


/*********************************************
 * LIST :: POP BACK
 * remove an item from the end of the list
 *    INPUT  :
 *    OUTPUT :
 *    COST   : O(1)
 *********************************************/
template <typename T, typename A>
void list <T, A> ::pop_back()
{
   if (numElements == 0) {
      return;
   }
   else if (numElements == 1) {
      delete pTail;
      pHead = pTail = nullptr;
   }
   else {
      Node* temp = pTail;
      pTail = pTail->pPrev;
      pTail->pNext = nullptr;
      delete temp;
   }
   --numElements;
}

/*********************************************
 * LIST :: POP FRONT
 * remove an item from the front of the list
 *    INPUT  :
 *    OUTPUT :
 *    COST   : O(1)
 *********************************************/
template <typename T, typename A>
void list <T, A> ::pop_front()
{
   if (numElements == 0) {
      return;
   }
   else if (numElements == 1) {
      delete pHead;
      pHead = pTail = nullptr;
   }
   else {
      Node* temp = pHead;
      pHead = pHead->pNext;
      pHead->pPrev = nullptr;
      delete temp;
   }
   --numElements;
}

/*********************************************
 * LIST :: FRONT
 * retrieves the first element in the list
 *     INPUT  :
 *     OUTPUT : data to be displayed
 *     COST   : O(1)
 *********************************************/
template <typename T, typename A>
T & list <T, A> :: front()
{
   if (pHead == nullptr)
      throw ("ERROR: unable to access data from an empty list");
      
   return pHead->data;
}

/*********************************************
 * LIST :: BACK
 * retrieves the last element in the list
 *     INPUT  :
 *     OUTPUT : data to be displayed
 *     COST   : O(1)
 *********************************************/
template <typename T, typename A>
T & list <T, A> :: back()
{
   if (pTail == nullptr)
      throw("ERROR: unable to access data from an empty list");
      
   return pTail->data;
}

/******************************************
 * LIST :: REMOVE
 * remove an item from the middle of the list
 *     INPUT  : an iterator to the item being removed
 *     OUTPUT : iterator to the new location
 *     COST   : O(1)
 ******************************************/
template <typename T, typename A>
typename list <T, A> :: iterator  list <T, A> :: erase(const list <T, A> :: iterator & it)
{
   if (it == nullptr)
      return nullptr;
      
      
   iterator itNext = end();
   
   // Check if the next node exists
   if (it.p->pNext)
   {
      // Adjust pointers of adjacent nodes
      it.p->pNext->pPrev = it.p->pPrev;
      itNext = it.p->pNext;
   }
   else
      
      // Update tail if no next node
      pTail = pTail->pPrev;

   // Adjust pointers of previous nodes
   if (it.p->pPrev)
      it.p->pPrev->pNext = it.p->pNext;
   else
      
      // Update head if no previous node
      pHead = pHead->pNext;

   // Delete the current node and decrement element count
   delete it.p;
   numElements--;
   
   return itNext;
}

/******************************************
 * LIST :: INSERT
 * add an item to the middle of the list
 *     INPUT  : data to be added to the list
 *              an iterator to the location where it is to be inserted
 *     OUTPUT : iterator to the new item
 *     COST   : O(1)
 ******************************************/
template <typename T, typename A>
typename list <T, A> :: iterator list <T, A> :: insert(list <T, A> :: iterator it,
                                                 const T & data)
{
   Node* newNode = new Node(data);

   if (it.p == nullptr)
   {
      // Inserting at the end if the iterator is null
      if (pTail == nullptr)
         
         // Update both head and tail if list is empty
         pHead = pTail = newNode;
      else
      {
         // Otherwise, adjust pointers for tail and new node
         pTail->pNext = newNode;
         newNode->pPrev = pTail;
         pTail = newNode;
      }
   }
   else
   {
      // Inserting in the middle
      Node* current = it.p;
      Node* prev = current->pPrev;
      newNode->pNext = current;
      newNode->pPrev = prev;
      if (prev != nullptr)
         
         // Adjust previous node's next pointer
         prev->pNext = newNode;
      else
         
         // Update head if there's no previous node
         pHead = newNode;

      // Adjust current node's previous pointer
      current->pPrev = newNode;
   }

   // Increment element count and return iterator
   ++numElements;
   return iterator(newNode);
}


/******************************************
 * LIST :: INSERT
 * add several items into the middle of the list
 *     INPUT  : data to be added to the list
 *              an iterator to the location where it is to be inserted
 *     OUTPUT : iterator to the new item
 *     COST   : O(1)
 ******************************************/
template <typename T, typename A>
typename list <T, A> ::iterator list <T, A> ::insert(list <T, A> ::iterator it,
   T && data)
{
   Node* newNode = new Node(std::forward<T>(data));
   if (it.p == nullptr)
   {
      if (pTail == nullptr)
         pHead = pTail = newNode;
      else
      {
         pTail->pNext = newNode;
         newNode->pPrev = pTail;
         pTail = newNode;
      }
   }
   else
   {
      Node* current = it.p;
      Node* prev = current->pPrev;
      newNode->pNext = current;
      newNode->pPrev = prev;
      if (prev != nullptr)
         prev->pNext = newNode;
      else
         pHead = newNode;

      current->pPrev = newNode;
   }
   
   ++numElements;
   return iterator(newNode);
}

/**********************************************
 * LIST :: assignment operator - MOVE
 * Copy one list onto another
 *     INPUT  : a list to be moved
 *     OUTPUT :
 *     COST   : O(n) with respect to the size of the LHS
 *********************************************/
template <typename T, typename A>
void swap(list<T, A>& lhs, list<T, A>& rhs)
{
   std::swap(lhs.pHead, rhs.pHead);
   std::swap(lhs.pTail, rhs.pTail);
   std::swap(lhs.numElements, rhs.numElements);
}

}; // namespace custom


