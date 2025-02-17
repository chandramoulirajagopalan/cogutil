#ifndef _OPENCOG_ALGORITHM_H
#define _OPENCOG_ALGORITHM_H

#include <algorithm>
#include <set>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include <algorithm>

#include <opencog/util/numeric.h>
#include <opencog/util/exceptions.h>

namespace opencog
{
/** \addtogroup grp_cogutil
 *  @{
 */

/// @todo these needs to be changed for non-gcc
// using std::copy_n;
// using std::lexicographical_compare_3way;
// using std::random_sample_n;
// using std::random_sample;
// using std::is_heap;
using std::is_sorted;

//! binary for_each
template<typename It1, typename It2, typename F>
F for_each(It1 from1, It1 to1, It2 from2, F f)
{
    for (;from1 != to1;++from1, ++from2)
        f(*from1, *from2);
    return f;
}
//! ternary for_each
template<typename It1, typename It2, typename It3, typename F>
F for_each(It1 from1, It1 to1, It2 from2, It3 from3, F f)
{
    for (;from1 != to1;++from1, ++from2, ++from3)
        f(*from1, *from2, *from3);
    return f;
}
//! quaternary for_each
template<typename It1, typename It2, typename It3, typename It4, typename F>
F for_each(It1 from1, It1 to1, It2 from2, It3 from3, It4 from4, F f)
{
    for (;from1 != to1;++from1, ++from2, ++from3, ++from4)
        f(*from1, *from2, *from3, *from4);
    return f;
}

//! accumulate over a range of containers
template <class iter, class T>
T accumulate2d(iter first, iter last, T init)
{
    for ( ;first != last;++first)
        init = std::accumulate(first->begin(), first->end(), init);
    return init;
}

//! appends b at the end of a, that is a = a@b
template<class T>
void append(T& a, const T& b) {
    a.insert(a.end(), b.begin(), b.end());
}

//! erase the intersection of sorted ranges [from1,to1) and [from2,to2) from c,
//! leaving the difference
template<typename Erase, typename It1, typename It2, typename Comp>
void erase_set_intersection(Erase erase, It1 from1, It1 to1,
                            It2 from2, It2 to2, Comp comp)
{
    OC_ASSERT(std::is_sorted(from1, to1, comp),
              "algorithm - from1 -> to1 aren't sorted (erase_set_intersection).");
    OC_ASSERT(std::is_sorted(from2, to2, comp),
              "algorithm - from2 -> to2 aren't sorted (erase_set_intersection).");

    while (from1 != to1 && from2 != to2)
        if (comp(*from1, *from2)) {
            ++from1;
        } else if (comp(*from2, *from1)) {
            ++from2;
        } else {
            erase(from1++);
            ++from2;
        }
}

//! erase the difference of sorted ranges [from1,to1) and [from2,to2) from c,
//! leaving the intersection
template<typename Erase, typename It1, typename It2, typename Comp>
void erase_set_difference(Erase erase, It1 from1, It1 to1,
                          It2 from2, It2 to2, Comp comp)
{
    OC_ASSERT(std::is_sorted(from1, to1, comp),
              "algorithm - from1 -> to1 aren't sorted (erase_set_difference).");
    OC_ASSERT(std::is_sorted(from2, to2, comp),
              "algorithm - from2 -> to2 aren't sorted (erase_set_difference).");

    while (from1 != to1 && from2 != to2)
        if (comp(*from1, *from2)) {
            erase(from1++);
        } else if (comp(*from2, *from1)) {
            ++from2;
        } else {
            ++from1;
            ++from2;
        }
    while (from1 != to1)
        erase(from1++);
}

//! insert [from2,to2)-[from1,to1) with inserter
//! i.e., if insert inserts into the container holding [from1,to1),
//! it will now hold the union
template<typename Insert, typename It1, typename It2, typename Comp>
void insert_set_complement(Insert insert, It1 from1, It1 to1,
                           It2 from2, It2 to2, Comp comp)
{
    cassert(TRACE_INFO, std::is_sorted(from1, to1, comp),
            "algorithm - from1 -> to1 aren't sorted (insert_set_complement).");
    cassert(TRACE_INFO, std::is_sorted(from2, to2, comp),
            "algorithm - from2 -> to2 aren't sorted (insert_set_complement).");

    while (from1 != to1 && from2 != to2)
        if (comp(*from1, *from2)) {
            ++from1;
        } else if (comp(*from2, *from1)) {
            insert(from1, *from2);
            ++from2;
        } else {
            ++from1;
            ++from2;
        }
    while (from2 != to2) {
        insert(from1, *from2);
        ++from2;
    }
}

//! determine if the intersection of two sets is the empty set
template<typename It1, typename It2, typename Comp>
bool has_empty_intersection(It1 from1, It1 to1,
                            It2 from2, It2 to2, Comp comp)
{
    cassert(TRACE_INFO, std::is_sorted(from1, to1, comp),
            "algorithm - from1 -> to1 aren't sorted (has_empty_intersection).");
    cassert(TRACE_INFO, std::is_sorted(from2, to2, comp),
            "algorithm - from2 -> to2 aren't sorted (has_empty_intersection).");

    while (from1 != to1 && from2 != to2)
        if (comp(*from1, *from2))
            ++from1;
        else if (comp(*from2, *from1))
            ++from2;
        else
            return false;
    return true;
}

//! determine if the intersection of two sets is the empty set
template<typename Set>
bool has_empty_intersection(const Set& ls, const Set& rs) {
    return has_empty_intersection(ls.begin(), ls.end(),
                                  rs.begin(), rs.end(),
                                  ls.key_comp());
}

/**
 * Like has_empty_intersection but more efficient. Code by Graphics
 * Noob, taken from stackoverflow. WARNING: it doesn't check whether
 * the 2 containers are sorted.
 */
template<class Set1, class Set2>
bool is_disjoint(const Set1 &set1, const Set2 &set2)
{
    if (set1.empty() || set2.empty()) return true;

    typename Set1::const_iterator
        from1 = set1.begin(),
        to1 = set1.end();
    typename Set2::const_iterator
        from2 = set2.begin(),
        to2 = set2.end();

    if (*set2.rbegin() < *from1 || *set1.rbegin() < *from2) return true;

    while (from1 != to1 && from2 != to2)
    {
        if (*from1 == *from2)
            return false;
        if (*from1 < *from2)
            ++from1;
        else
            ++from2;
    }

    return true;
}

/**
 * \return a singleton set with the input given in it
 */
template<typename Set>
Set make_singleton_set(const typename Set::value_type& v) {
    Set ret;
    ret.insert(v);
    return ret;
}

/**
 * Modify s1 to that it contains s1 union s2, s1 and s2 being std::set
 * or similar concept.
 */
template<typename Set>
void set_union_modify(Set& s1, const Set& s2) {
    s1.insert(s2.begin(), s2.end());
}

/**
 * \return s1 union s2
 * s1 and s2 being std::set or similar concept
 */
template<typename Set>
Set set_union(const Set& s1, const Set& s2) {
    Set res(s1);
    set_union_modify(res, s2);
    return res;
}

/**
 * \return s1 inter s2
 * s1 and s2 must be sorted
 */
template<typename Set>
Set set_intersection(const Set& s1, const Set& s2) {
    Set res;
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                          std::inserter(res, res.end()));
    return res;
}

/**
 * \return s1 - s2
 * s1 and s2 must be sorted
 */
template<typename Set>
Set set_difference(const Set& s1, const Set& s2) {
    Set res;
    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(res, res.end()));
    return res;
}

/**
 * \return (s1 - s2) union (s2 - s1)
 * s1 and s2 must be sorted
 */
template<typename Set>
Set set_symmetric_difference(const Set& s1, const Set& s2) {
    Set res;
    std::set_symmetric_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                                  std::inserter(res, res.end()));
    return res;
}

//! Predicate maps to the range [0, n)
//! n-1 values (the pivots) are copied to out
template<typename It, typename Pred, typename Out>
Out n_way_partition(It begin, It end, const Pred p, int n, Out out)
{
    // could be made more efficient if needed
    for (int i = 0;i < n - 1;++i)
        *out++ = begin = std::partition(begin, end, boost::bind(p, _1) == i);
    return out;
}

/**
 * \return the power set ps of s such that all elements of ps are
 * subsets of size n or below.
 *
 * @param s       the input set
 * @param n       the size of the largest subset of s. If n is larger than
 *                |s| then the size of the largest subset of s will be |s|
 * @param exact   if true then do not include subsets of size below n
 * @return        the power set of s with subsets up to size n
 */
template<typename Set> std::set<Set> powerset(const Set& s, size_t n,
                                              bool exact=false)
{
    std::set<Set> res;
    if (n > 0) {
        std::set<Set> ps = powerset(s, n-1, exact);
        for (const Set& ss : ps)
            for (const auto& el : s) {
                Set subset(ss);
                if (subset.find(el) == subset.end()) {
                    subset.insert(el);
                    res.insert(subset);
                }
            }
        if (!exact)
            res.insert(ps.begin(), ps.end());
    } else
        res.insert(Set());
    return res;
}

/**
 * \return the power set of s.
 */
template<typename Set> std::set<Set> powerset(const Set& s)
{
    return powerset(s, s.size());
}

/**
 * Return the n-fold Cartesian product of a given container with
 * itself. For instance if
 *
 * c = {1, 2, 3}, nfold = 2
 *
 * the result is
 *
 * { [1, 1], [1, 2], [1, 3],
     [2, 1], [2, 2], [2, 3],
     [3, 1], [3, 2], [3, 3] }
 */
template<typename C>
std::set<std::vector<typename C::value_type>> cartesian_product(const C& c,
                                                                size_t nfold=2)
{
	typedef typename C::value_type T;
	// Recursive case
	if (nfold > 0) {
		std::set<std::vector<T>> res;
		std::set<std::vector<T>> cp = cartesian_product(c, nfold - 1);
		for (const std::vector<T>& t : cp) {
			for (const auto& el : c) {
				std::vector<T> tel(t);
				tel.push_back(el);
				res.insert(tel);
			}
		}
		return res;
	}

	// Base case
	return {{}};
}

/**
 * Given a sequence of indexes, and a sequence of elements, return a
 * sequence of all elements corresponding to the indexes (in the order
 * of the indexes).
 *
 * Maybe boost offers something like that already but I can't find it.
 */
template<typename Indices, typename Seq>
Seq seq_filtered(const Seq& seq, const Indices& indices)
{
    Seq res;
    for (const auto& idx : indices)
        res.push_back(seq[idx]);
    return res;
}

template<typename T>
bool is_in(const typename std::set<T>::value_type& el,
           const typename std::set<T>& set)
{
	return set.find(el) != set.end();
}

/**
 * Return true if el is in s.
 */
template<typename Container>
bool is_in(const typename Container::value_type& el, const Container& c)
{
	return std::find(c.begin(), c.end(), el) != c.end();
}

/**
 * Clear by swap. STL container clear method doesn't necessary
 * deallocate the memory. This method does.
 */
template<typename C>
void clear_by_swap(C& c)
{
	C empty;
	c.swap(empty);
}

// Crappy range versions of std::any_of, std::all_of and std::none_of,
// till it makes into the boost library
//
// TODO: remove those as they are implemented in boost
// https://www.boost.org/doc/libs/1_69_0/libs/algorithm/doc/html/algorithm/CXX11.html
// will soon be part of the standard.
template<typename C, typename P>
bool any_of(const C& c, const P& p)
{
	return std::any_of(c.begin(), c.end(), p);
}

template<typename C, typename P>
bool all_of(const C& c, const P& p)
{
	return std::all_of(c.begin(), c.end(), p);
}

template<typename C, typename P>
bool none_of(const C& c, const P& p)
{
	return std::none_of(c.begin(), c.end(), p);
}

/** @}*/
} //~namespace opencog

#endif
