#pragma once
#include <tuple>

typedef std::tuple<float, float, float, float> rgba;
typedef std::tuple<float, float, float> rgb;

inline rgba MakeRGBA(float r, float g, float b, float a) { return std::make_tuple(r, g, b, a); }
inline rgb MakeRGB(float r, float g, float b) { return std::make_tuple(r, g, b); }
inline rgba AddAlpha(rgb c, float alpha) { return std::make_tuple(std::get<0>(c), std::get<1>(c), std::get<2>(c), alpha); }
inline rgba ChangeAlpha(rgba c, float alphaDelta) { return std::make_tuple(std::get<0>(c), std::get<1>(c), std::get<2>(c), std::get<3>(c)+alphaDelta); }
inline rgb RemoveAlpha(rgba c) { return std::make_tuple(std::get<0>(c), std::get<1>(c), std::get<2>(c)); }