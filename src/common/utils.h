#pragma once
#ifndef COMMON_UTILS_H__
#define COMMON_UTILS_H__

#ifndef IMPLEMENT_STD_HASH_FOR_ENUM_CLASS
// Utility routine to add hashing support to enum classes so they can be inserted into maps and sets.
#define IMPLEMENT_STD_HASH_FOR_ENUM_CLASS(T) namespace std { template<> struct hash<T> { size_t operator()(T value) const { return hash<int>()((int)value); } }; }
#endif // IMPLEMENT_STD_HASH_FOR_ENUM_CLASS

#endif // COMMON_UTILS_H__
