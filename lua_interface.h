#ifndef __LUA_INTERFACE_H_
#define __LUA_INTERFACE_H_

//通用的C++调用lua方法，适合固定参数和返回值的调用
extern int
call_lua(const char *func, const char *sig, ...);

#endif

