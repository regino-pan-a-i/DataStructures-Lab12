/***********************************************************************
 * Header:
 *    HASH
 * Summary:
 *    Our custom implementation of std::unordered_Set
 *      __      __     _______        __
 *     /  |    /  |   |  _____|   _  / /
 *     `| |    `| |   | |____    (_)/ /
 *      | |     | |   '_.____''.   / / _
 *     _| |_   _| |_  | \____) |  / / (_)
 *    |_____| |_____|  \______.' /_/
 *
 *    This will contain the class definition of:
 *        unordered_set           : A class that represents a hash
 *        unordered_set::iterator : An interator through hash
 * Author
 *       Marco Varela & Andre Regino
 ************************************************************************/

#pragma once

#include "list.h"     // because this->buckets[0] is a list
#include "vector.h"   // because this->buckets is a vector
#include <memory>     // for std::allocator
#include <functional> // for std::hash
#include <cmath>      // for std::ceil
   

class TestHash;             // forward declaration for Hash unit tests

namespace custom
{
/************************************************
 * UNORDERED SET
 * A set implemented as a hash
 ************************************************/
template <typename T,
          typename Hash = std::hash<T>,
          typename EqPred = std::equal_to<T>,
          typename A = std::allocator<T> >
class unordered_set
{
   friend class ::TestHash;   // give unit tests access to the privates
public:
   //
   // Construct
   //
   unordered_set() : buckets(8), numElements(0), maxLoadFactor(1)
   {
   }
   unordered_set(size_t numBuckets) : buckets(numBuckets), numElements(0), maxLoadFactor(1)
   {
   }
   unordered_set(const unordered_set&  rhs) 
   {
      *this = rhs;
   }
   unordered_set(unordered_set&& rhs) noexcept
   {
      *this = std::move(rhs);
   }
   template <class Iterator>
   unordered_set(Iterator first, Iterator last)
   {
      while (first != last)
      {
         insert(*first++);
      }
   }

   //
   // Assign
   //
   unordered_set& operator=(const unordered_set& rhs)
   {
      buckets = rhs.buckets;
      numElements = (int)rhs.size();
      maxLoadFactor = rhs.max_load_factor();
      
      
      return *this;
   }
   unordered_set& operator=(unordered_set&& rhs) noexcept
   {
      if (!rhs.buckets.empty())
         buckets = rhs.buckets;
      else
         buckets = std::move(rhs.buckets);
      
      numElements = (int)rhs.size();
      maxLoadFactor = rhs.max_load_factor();
      
      rhs.numElements = 0;
      rhs.maxLoadFactor = 1.0;
      
      return *this;
   }
   unordered_set& operator=(const std::initializer_list<T>& il)
   {
      return *this;
   }
   void swap(unordered_set& rhs)
   {
      std::swap(*this, rhs);
   }

   // 
   // Iterator
   //
   class iterator;
   class local_iterator;
   iterator begin()
   {
      for (auto itBucket = buckets.begin(); itBucket != buckets.end(); itBucket++)
      {
         if (!(*itBucket).empty())
            return iterator(buckets.end(), itBucket, (*itBucket).begin());
      }
      return end();
   }
   iterator end()
   {
      return iterator(buckets.end(), buckets.end(), buckets[0].end());
   }
   local_iterator begin(size_t iBucket)
   {
      return local_iterator(buckets[iBucket].begin());
   }
   local_iterator end(size_t iBucket)
   {
      return local_iterator(buckets[iBucket].end());
   }

   //
   // Access
   //
   size_t bucket(const T& t)
   {
      Hash hashFunction;
      return hashFunction(t) % bucket_count();
   }
   iterator find(const T& t);

   //   
   // Insert
   //
   custom::pair<iterator, bool> insert(const T& t);
   void insert(const std::initializer_list<T> & il);
   void rehash(size_t numBuckets);
   void reserve(size_t num)
   {
      // Calculate the desired load factor based on the maximum load factor
      float possibleLoadFactor = static_cast<float>(num) / buckets.size();

      // Adjust the load factor if it exceeds the maximum load factor
      if (possibleLoadFactor > maxLoadFactor)
      {
         // Calculate the new load factor that is less than or equal to the maximum load factor
         float newLoadFactor = std::min(possibleLoadFactor, maxLoadFactor);

         // Calculate the number of buckets required based on the new load factor
         size_t newNumBuckets = static_cast<size_t>(num / newLoadFactor);

         // Rehash with the new number of buckets
         rehash(newNumBuckets);
      }
   }

   // 
   // Remove
   //
   void clear() noexcept
   {
      for (auto &bucket : buckets)
         bucket.clear();
      numElements = 0;
   }
   iterator erase(const T& t);

   //
   // Status
   //
   size_t size() const 
   { 
      return numElements;
   }
   bool empty() const 
   { 
      return size() == 0;
   }
   size_t bucket_count() const 
   { 
      return buckets.size();
   }
   size_t bucket_size(size_t i) const
   {
      return buckets[i].size();
   }
   float load_factor() const noexcept 
   { 
      return (float)(size()/bucket_count());
   }
   float max_load_factor() const noexcept 
   { 
      return maxLoadFactor; 
   }
   void  max_load_factor(float m)
   {
      maxLoadFactor = m;
   }

private:

   size_t min_buckets_required(size_t num) const
   {
      return (size_t)99;
   }

   custom::vector<custom::list<T,A>> buckets;  // each bucket in the hash
   int numElements;                            // number of elements in the Hash
   float maxLoadFactor;                        // the ratio of elements to buckets signifying a rehash
};


/************************************************
 * UNORDERED SET ITERATOR
 * Iterator for an unordered set
 ************************************************/
template <typename T, typename H, typename E, typename A>
class unordered_set <T, H, E, A> ::iterator
{
   friend class ::TestHash;   // give unit tests access to the privates
   template <typename TT, typename HH, typename EE, typename AA>
   friend class custom::unordered_set;
public:
   // 
   // Construct
   iterator() 
   {
   }
   iterator(const typename custom::vector<custom::list<T> >::iterator& itVectorEnd,
            const typename custom::vector<custom::list<T> >::iterator& itVector,
            const typename custom::list<T>::iterator &itList)
   {
      this->itVectorEnd = itVectorEnd;
      this->itVector = itVector;
      this->itList = itList;
   }
   iterator(const iterator& rhs) 
   { 
      itVectorEnd = rhs.itVectorEnd;
      itVector = rhs.itVector;
      itList = rhs.itList;
   }

   //
   // Assign
   //
   iterator& operator = (const iterator& rhs)
   {
      itVectorEnd = rhs.itVectorEnd;
      itVector = rhs.itVector;
      itList = rhs.itList;
      return *this;
   }

   //
   // Compare
   //
   bool operator != (const iterator& rhs) const 
   { 
      if (itList == rhs.itList && itVector == rhs.itVector && itVectorEnd == rhs.itVectorEnd)
         return false;
      else
         return true;
   }
   bool operator == (const iterator& rhs) const 
   { 
      if (itList == rhs.itList && itVector == rhs.itVector && itVectorEnd == rhs.itVectorEnd)
         return true;
      else
         return false;
   }

   // 
   // Access
   //
   T& operator * ()
   {
      return *(itList);
   }

   //
   // Arithmetic
   //
   iterator& operator ++ ();
   iterator operator ++ (int postfix)
   {
      iterator temp(*this);
      ++(*this);
      return temp;
   }

private:
   typename vector<list<T>>::iterator itVectorEnd;
   typename list<T>::iterator itList;
   typename vector<list<T>>::iterator itVector;
};


/************************************************
 * UNORDERED SET LOCAL ITERATOR
 * Iterator for a single bucket in an unordered set
 ************************************************/
template <typename T, typename H, typename E, typename A>
class unordered_set <T, H, E, A> ::local_iterator
{
   friend class ::TestHash;   // give unit tests access to the privates

   template <typename TT, typename HH, typename EE, typename AA>
   friend class custom::unordered_set;
public:
   // 
   // Construct
   //
   local_iterator()  
   {
   }
   local_iterator(const typename custom::list<T>::iterator& itList) 
   {
      this->itList = itList;
   }
   local_iterator(const local_iterator& rhs) 
   { 
      itList = rhs.itList;
   }

   //
   // Assign
   //
   local_iterator& operator = (const local_iterator& rhs)
   {
      itList = rhs.itList;
      return *this;
   }

   // 
   // Compare
   //
   bool operator != (const local_iterator& rhs) const
   {
      if (itList == rhs.itList)
         return false;
      else
         return true;
   }
   bool operator == (const local_iterator& rhs) const
   {
      if (itList == rhs.itList)
         return true;
      else
         return false;
   }

   // 
   // Access
   //
   T& operator * ()
   {
      return *(itList);
   }

   // 
   // Arithmetic
   //
   local_iterator& operator ++ ()
   {
      ++itList;
      return *this;
   }
   local_iterator operator ++ (int postfix)
   {
      auto temp = this;
      ++this;
      return temp;
   }

private:
   typename list<T>::iterator itList;
};


/*****************************************
 * UNORDERED SET :: ERASE
 * Remove one element from the unordered set
 ****************************************/
template <typename T, typename Hash, typename E, typename A>
typename unordered_set <T, Hash, E, A> ::iterator unordered_set<T,Hash,E,A>::erase(const T& t)
{
   // Find element to be erased. Return end() if the element is not present.
   iterator itErase = find(t);
   if (itErase == end())
      return itErase;
   
   // Determine the return value.
   iterator itReturn = itErase;
   itReturn++;
   
   // Erase the element from the bucket.
   buckets[bucket(t)].erase(itErase.itList);

   
   numElements--;
   
   // Return iterator to the next element.
   return itReturn;

}

/*****************************************
 * UNORDERED SET :: INSERT
 * Insert one element into the hash
 ****************************************/
template <typename T, typename H, typename E, typename A>
custom::pair<typename custom::unordered_set<T, H, E, A>::iterator, bool> unordered_set<T, H, E, A>::insert(const T& t)
{
   // Find the bucket where the new element is to reside.
   size_t iBucket = bucket(t);

   // See if the element is already there. If so, then return out.
   for (typename custom::list<T>::iterator it = buckets[iBucket].begin(); it != buckets[iBucket].end(); ++it)
       if (*it == t)
           return custom::pair<custom::unordered_set<T, H, E, A>::iterator, bool>(find(t), false);

   // Reserve more space if we are already at the limit.
   if (min_buckets_required(numElements + 1) > bucket_count())
   {
       reserve(numElements * 2);
       iBucket = bucket(t);
   }

   // Actually insert the new element on the back of the bucket.
   buckets[iBucket].push_back(t);
   ++numElements; // Increment the count of elements

   iterator itInserted = find(t);

   // Return the results.
   return custom::pair<custom::unordered_set<T, H, E, A>::iterator, bool>(itInserted, true);
}
template <typename T, typename H, typename E, typename A>
void unordered_set<T, H, E, A>::insert(const std::initializer_list<T> & il)
{
}

/*****************************************
 * UNORDERED SET :: REHASH
 * Re-Hash the unordered set by numBuckets
 ****************************************/
template <typename T, typename Hash, typename E, typename A>
void unordered_set<T, Hash, E, A>::rehash(size_t numBuckets)
{
   Hash hash;
   
   
   // If the current bucket count is sufficient, then do nothing.
   if (numBuckets <= bucket_count())
      return;
   
   // Create a new hash bucket.
   custom::vector<custom::list<T,A>> bucketNew(numBuckets);
   
   // Insert the elements into the new hash table, one at a time.
   for (auto& bucket : buckets) 
   {
      for (auto& element : bucket)
      {
         bucketNew[hash(element) % numBuckets].push_back(std::move(element));
      }
   }
   
   for (auto &bucket : buckets)
      bucket.clear();
   
   //Swap the old bucket for the new.
   std::swap(buckets, bucketNew);
   
   
}


/*****************************************
 * UNORDERED SET :: FIND
 * Find an element in an unordered set
 ****************************************/
template <typename T, typename H, typename E, typename A>
typename unordered_set <T, H, E, A> ::iterator unordered_set<T, H, E, A>::find(const T& t)
{
   // Get the index of the bucket where 't' will be in
   auto iBucket = bucket(t);
   
   // Get an iterator until we find the right bucket
   for (auto itBucket = buckets.begin(); itBucket != buckets.end(); ++itBucket)
   {
      if ((*itBucket).begin() == buckets[iBucket].begin())
      {
         // Get a list iterator to the element iterating through it
         for (auto itList = buckets[iBucket].begin(); itList != buckets[iBucket].end(); ++itList)
         {
            if (*itList == t)
            {
               return iterator(buckets.end(),itBucket, itList);
            }
         }
      }
   }
   return end();
}

/*****************************************
 * UNORDERED SET :: ITERATOR :: INCREMENT
 * Advance by one element in an unordered set
 ****************************************/
template <typename T, typename H, typename E, typename A>
typename unordered_set <T, H, E, A> ::iterator & unordered_set<T, H, E, A>::iterator::operator ++ ()
{
   
   // Only advance if we are not already at the end
   if (itVector == itVectorEnd)
      return *this;
   
   // Advance the list iterator. If we are not at the end, then we are done.
   
   ++itList;
   if (itList != (*itVector).end())
      return *this;
   
   // We are at the end of the list. Find the next bucket.
   ++itVector;
   while (itVector != itVectorEnd && (*itVector).empty())
      ++itVector;
   if (itVector != itVectorEnd)
      itList = (*itVector).begin();
   return *this;
}

/*****************************************
 * SWAP
 * Stand-alone unordered set swap
 ****************************************/
template <typename T, typename H, typename E, typename A>
void swap(unordered_set<T,H,E,A>& lhs, unordered_set<T,H,E,A>& rhs)
{
   std::swap(lhs, rhs);
}

}
