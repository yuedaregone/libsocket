#ifndef __NETCONF_H__
#define __NETCONF_H__

#if defined(LIB_NET_DLL)	/* { */

#if defined(LIB_NET_LIB)	/* { */
#define NET_API __declspec(dllexport)
#else						/* }{ */
#define NET_API __declspec(dllimport)
#endif						/* } */

#else				/* }{ */

#define NET_API		extern

#endif		

#endif
