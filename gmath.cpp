#include "gmath.h"
using namespace std;
using namespace GMath;

template<typename T,int n> const GVect<T,n> GVect<T,n>::zero;
template<typename T,int n> const GVect<T,n> GVect<T,n>::one;
template<typename T,int n> const GVect<T,n> GVect<T,n>::up;

template<> const vec2  vec2::zero      = {0,0};
template<> const vec2  vec2::one       = {1,1};
template<> const vec3  vec3::zero      = {0,0,0};
template<> const vec3  vec3::one       = {1,1,1};
template<> const vec3  vec3::up        = {0,1,0};
template<> const vec4  vec4::zero      = {0,0,0,0};
template<> const vec4  vec4::one       = {1,1,1,1};
template<> const vec4  vec4::up        = {0,1,0,1};

template<> const vec2f vec2f::one      = {1.0f,1.0f};
template<> const vec2f vec2f::zero     = {0.0f,0.0f};
template<> const vec3f vec3f::one      = {1.0f,1.0f,1.0f};
template<> const vec3f vec3f::zero     = {0.0f,0.0f,0.0f};
template<> const vec3f vec3f::right    = {1.0f,0.0f,0.0f};
template<> const vec3f vec3f::up       = {0.0f,1.0f,0.0f};
template<> const vec3f vec3f::forward  = {0.0f,0.0f,1.0f};
template<> const vec4f vec4f::right    = {1.0f,0.0f,0.0f,1.0f};
template<> const vec4f vec4f::up       = {0.0f,1.0f,0.0f,1.0f};
template<> const vec4f vec4f::forward  = {0.0f,0.0f,1.0f,1.0f};


template<> const vec2i vec2i::one = {1,1};
template<> const vec2i vec2i::zero = {0,0};

template<typename T> const GInterval<T> GInterval<T>::empty;
template<typename T> const GInterval<T> GInterval<T>::universe;
template<typename T> const GInterval<T> GInterval<T>::init;

template<> const interval interval::empty = {+numeric_limits<double>::infinity(), -numeric_limits<double>::infinity()};
template<> const interval interval::universe = {-numeric_limits<double>::infinity(), +numeric_limits<double>::infinity()};
template<> const interval interval::init = {0.001, +numeric_limits<double>::infinity()};
