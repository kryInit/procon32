#pragma once

#define JOIN(x, y) x ## y
#define likely(x) (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))
#define UNREACHABLE Utility::exit_with_message(Utility::concat("[",__func__,": ",__LINE__,"] unreachable"));
#define UNIMPLEMENTED Utility::exit_with_message(Utility::concat("[",__func__,": ",__LINE__,"] unimplemented"));
#define EXIT_DEBUG(...) Utility::exit_with_message(Utility::concat("[",__func__,": ",__LINE__,"] ") + Utility::concat_with_space(__VA_ARGS__))
#define GET_VAL_NAME(...) #__VA_ARGS__
#define DUMP_TO(os, ...) Utility::dump_to(os, __VA_ARGS__)
#define DUMP(...) Utility::dump(__VA_ARGS__)
#define EDUMP(...) Utility::edump(__VA_ARGS__)
#define PRINT_TO(os, ...) os << GET_VAL_NAME(__VA_ARGS__) << ": ", Utility::print_to(os, __VA_ARGS__)
#define PRINT(...) cout << GET_VAL_NAME(__VA_ARGS__) << ": ", Utility::print(__VA_ARGS__)
#define EPRINT(...) cerr << GET_VAL_NAME(__VA_ARGS__) << ": ", Utility::eprint(__VA_ARGS__)
#define rep(i,n) for(int i=0, JOIN($,i)=static_cast<int>(n); i<JOIN($,i); ++i)
#define repr(i,l,r) for(int i=static_cast<int>(l), JOIN($,i)=static_cast<int>(r); i<JOIN($,i); ++i)
#define revrep(i,n) for(int i=static_cast<int>(n)-1; (i)>=0; --(i))
#define all(v) (v).begin(), (v).end()
#define rall(v) (v).rbegin(), (v).rend()
