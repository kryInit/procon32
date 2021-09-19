#pragma once
#include <vec2.hpp>
#include <direction.hpp>

constexpr int MIN_DIV_NUM_Y = 2;
constexpr int MIN_DIV_NUM_X = 2;
constexpr int MIN_DIV_NUM   = MIN_DIV_NUM_Y * MIN_DIV_NUM_X;
constexpr int MAX_DIV_NUM_Y = 16;
constexpr int MAX_DIV_NUM_X = 16;
constexpr int MAX_DIV_NUM   = MAX_DIV_NUM_Y * MAX_DIV_NUM_X;

constexpr int MIN_SELECTABLE_TIMES = 2;
constexpr int MAX_SELECTABLE_TIMES = 128;

constexpr int MIN_SWAP_COST   = 1;
constexpr int MIN_SELECT_COST = 1;
constexpr int MAX_SWAP_COST   = 100;
constexpr int MAX_SELECT_COST = 500;

/* * * * * * * * * * * * * * * *
 * row   , UR: RUULDRDLUURDLDRU *
 * row   , DR: RDDLURULDDRULURD *
 * row   , DL: LDDRULURDDLURULD *
 * row   , UL: LUURDLDRUULDRDLU *
 * column, UR: URRDLULDRRULDLUR *
 * column, DR: DRRULDLURRDLULDR *
 * column, DL: DLLURDRULLDRURDL *
 * column, UL: ULLDRURDLLURDRUL *
  * * * * * * * * * * * * * * * */

inline const Path row_UR = Path::from_string("RUULDRDLUURDLDRU");
inline const Path row_DR = Path::from_string("RDDLURULDDRULURD");
inline const Path row_DL = Path::from_string("LDDRULURDDLURULD");
inline const Path row_UL = Path::from_string("LUURDLDRUULDRDLU");
inline const Path column_UR = Path::from_string("URRDLULDRRULDLUR");
inline const Path column_DR = Path::from_string("DRRULDLURRDLULDR");
inline const Path column_DL = Path::from_string("DLLURDRULLDRURDL");
inline const Path column_UL = Path::from_string("ULLDRURDLLURDRUL");

inline Vec2<int> div_num;
inline int swap_cost, select_cost, selectable_times;

