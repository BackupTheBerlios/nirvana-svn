/*
 * Copyright (C) 2003 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef QVALUELIST_H_
#define QVALUELIST_H_

#include "KWQDef.h"
#include "KWQValueListImpl.h"

#ifdef _KWQ_IOSTREAM_
#include <ostream.h>
#endif

template <class T> class QValueList;
template <class T> class QValueListPtr;
template <class T> class QValueListConstIterator;

template<class T> class QValueListNode : private KWQValueListNodeImpl {
public:
    QValueListNode(const T &val) : value(val) { }
    T value;
    friend class QValueList<T>;
};

template<class T> class QValueListNodePtr : private KWQValueListNodeImpl {
public:
    QValueListNodePtr(T *val) { value = val; }
    T* value;
    friend class QValueListPtr<T>;
};

template<class T> class QValueListIterator {
public: 
    QValueListIterator() { }
    T& operator*() const { return ((QValueListNode<T> *)impl.node())->value; } 
    T* value() { return ((QValueListNodePtr<T> *)impl.node())->value;} // LEMON
    QValueListIterator &operator++() { ++impl; return *this; }    
    QValueListIterator &operator--() { --impl; return *this; }
    QValueListIterator operator++(int) { return impl++; }
    bool operator==(const QValueListIterator &other) { return impl == other.impl; }
    bool operator!=(const QValueListIterator &other) { return impl != other.impl; }

private:
    QValueListIterator(const KWQValueListIteratorImpl &pImp) : impl(pImp) { }
    KWQValueListIteratorImpl impl;
    friend class QValueList<T>;
    friend class QValueListPtr<T>;
    friend class QValueListConstIterator<T>;
};

template<class T> class QValueListConstIterator {
public:
    QValueListConstIterator() { }
    QValueListConstIterator(const QValueListIterator<T> &it) : impl(it.impl) { }
    const T& operator*() const { return ((const QValueListNode<T> *)impl.node())->value; }
    const T* value() const { return ((QValueListNodePtr<T> *)impl.node())->value;} // LEMON used in signal
    QValueListConstIterator &operator++() { ++impl; return *this; }
    QValueListConstIterator &operator--() { --impl; return *this; }
    QValueListConstIterator operator++(int) { return impl++; }
    bool operator==(const QValueListConstIterator &other) { return impl == other.impl; }
    bool operator!=(const QValueListConstIterator &other) { return impl != other.impl; }

private:
    QValueListConstIterator(const KWQValueListIteratorImpl &pImp) : impl(pImp) { }
    KWQValueListIteratorImpl impl;
    friend class QValueList<T>;
    friend class QValueListPtr<T>;
};

template<class T> bool operator==(const QValueList<T> &a, const QValueList<T> &b);
template<class T> bool operator==(const QValueListPtr<T> &a, const QValueListPtr<T> &b);

template <class T> class QValueListPtr {
public:
    typedef QValueListIterator<T> Iterator;
    typedef QValueListConstIterator<T> ConstIterator; 
    QValueListPtr() : impl(deleteNodePtr, copyNodePtr) { }  
    void clear() { impl.clear(); }
    uint count() { return impl.count(); }
    bool isEmpty() { return impl.isEmpty(); } 
    Iterator append(T* val) { return impl.appendNode(new QValueListNodePtr<T>(val)); }
    Iterator prepend(T* val) { return impl.prependNode(new QValueListNodePtr<T>(val)); }
    void remove(T* val) { QValueListNodePtr<T> nodePtr(val); impl.removeEqualNodes(&nodePtr, nodesEqualPtr); }
    uint contains(T* val) const { QValueListNodePtr<T> nodePtr(val); return impl.containsEqualNodes(&nodePtr, nodesEqualPtr); }
    Iterator insert(Iterator iter, T* val) { return impl.insert(iter.impl, new QValueListNodePtr<T>(val)); }
    Iterator remove(Iterator iter) { return impl.removeIterator(iter.impl); }
    Iterator fromLast() { return impl.fromLast(); }
    T* first() { return ((QValueListNodePtr<T> *)impl.firstNode())->value; }
    T* last() { return ((QValueListNodePtr<T> *)impl.lastNode())->value; }
    Iterator begin() { return impl.begin(); }
    Iterator end() { return impl.end(); }
    ConstIterator begin() const { return impl.begin(); }
    ConstIterator end() const { return impl.end(); }
    ConstIterator fromLast() const { return impl.fromLast(); }
    T* operator[] (uint index) { return ((QValueListNodePtr<T> *)impl.nodeAt(index))->value; } 
    QValueListPtr &operator+=(T *value) { impl.appendNode(new QValueListNodePtr<T>(value)); return *this; }
    QValueListPtr &operator<<(T *value) { impl.appendNode(new QValueListNodePtr<T>(value)); return *this; }
    static bool equals(const QValueListPtr<T> &, const QValueListPtr<T> &);
private:
    KWQValueListImpl impl;
    static void deleteNodePtr(KWQValueListNodeImpl *node) { delete (QValueListNodePtr<T> *)node; }
    static bool nodesEqualPtr(const KWQValueListNodeImpl *a, const KWQValueListNodeImpl *b) // LEMON
        { return ((QValueListNodePtr<T> *)a)->value == ((QValueListNodePtr<T> *)b)->value; }
    static KWQValueListNodeImpl *copyNodePtr(KWQValueListNodeImpl *node)
        { return new QValueListNodePtr<T>(((QValueListNodePtr<T> *)node)->value); }
};

template <class T> class QValueList {
public:
    typedef QValueListIterator<T> Iterator;
    typedef QValueListConstIterator<T> ConstIterator;
    QValueList() : impl(deleteNode, copyNode) { }
    void clear() { impl.clear(); }
    uint count() const { return impl.count(); }
    bool isEmpty() const { return impl.isEmpty(); }
    Iterator append(const T &val) { return impl.appendNode(new QValueListNode<T>(val)); } 
    Iterator prepend(const T &val) { return impl.prependNode(new QValueListNode<T>(val)); } 
    void remove(const T &val) { QValueListNode<T> node(val); impl.removeEqualNodes(&node, nodesEqual); }
    uint contains(const T &val) const { QValueListNode<T> node(val); return impl.containsEqualNodes(&node, nodesEqual); }
    Iterator insert(Iterator iter, const T& val) { return impl.insert(iter.impl, new QValueListNode<T>(val)); }
    Iterator remove(Iterator iter) { return impl.removeIterator(iter.impl); }
    Iterator fromLast() { return impl.fromLast(); }
    const T& first() const { return ((QValueListNode<T> *)impl.firstNode())->value; }
    const T& last() const { return ((QValueListNode<T> *)impl.lastNode())->value; }
    Iterator begin() { return impl.begin(); }
    Iterator end() { return impl.end(); }
    ConstIterator begin() const { return impl.begin(); }
    ConstIterator end() const { return impl.end(); }
    ConstIterator fromLast() const { return impl.fromLast(); }
    T& operator[] (uint index) { return ((QValueListNode<T> *)impl.nodeAt(index))->value; }
    const T& operator[] (uint index) const { return ((const QValueListNode<T> *)impl.nodeAt(index))->value; }
    QValueList &operator+=(const T &value) { impl.appendNode(new QValueListNode<T>(value)); return *this; }
    QValueList &operator<<(const T &value) { impl.appendNode(new QValueListNode<T>(value)); return *this; }
    static bool equals(const QValueList<T> &, const QValueList<T> &);
    
private:
    KWQValueListImpl impl;
    static void deleteNode(KWQValueListNodeImpl *node) { delete (QValueListNode<T> *)node; }
    static bool nodesEqual(const KWQValueListNodeImpl *a, const KWQValueListNodeImpl *b)
        { return ((QValueListNode<T> *)a)->value == ((QValueListNode<T> *)b)->value; }	
    static KWQValueListNodeImpl *copyNode(KWQValueListNodeImpl *node)
        { return new QValueListNode<T>(((QValueListNode<T> *)node)->value); }
};

template<class T>
inline bool operator==(const QValueList<T> &a, const QValueList<T> &b) {
    return a.impl.isEqual(b.impl, QValueList<T>::nodesEqual);
}

template<class T>
inline bool equals(const QValueList<T> &a, const QValueList<T> &b) {
    return a.impl.isEqual(b.impl, QValueList<T>::nodesEqual);
}

template<class T>
inline bool operator==(const QValueListPtr<T> &a, const QValueListPtr<T> &b) {
    return a.impl.isEqual(b.impl, QValueListPtr<T>::nodesEqual);
}

template<class T>
inline bool equals(const QValueListPtr<T> &a, const QValueListPtr<T> &b) {
    return a.impl.isEqual(b.impl, QValueListPtr<T>::nodesEqual);
}

#ifdef _KWQ_IOSTREAM_

template<class T>
inline std::ostream &operator<<(std::ostream &o, const QValueList<T>&p)
{
    o <<
        "QValueList: [size: " <<
        (Q_UINT32)p.count() <<
        "; items: ";
        QValueListConstIterator<T> it = p.begin();
        while (it != p.end()) {
            o << *it;
            if (++it != p.end()) {
                o << ", ";
            }
        }
        o << "]";
    return o;
}

#endif

#endif
