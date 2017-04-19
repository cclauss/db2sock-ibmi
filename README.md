#------IMPORTANT WARNING -------
This project is under construction. APIs are changing daily, therefore, you should not use for ANY production purpose. 
When this warning disappears, APIs will be considered stable.


#db2sock

Welcome to db2sock project. Goal is PASE DB2 CLI asynchronous API driver and more (libdb400.a).

A vast number of features have been added to new libdb400.a, async CLI, ILE direct CLI APIs, 
Unicode CLI "Unix" API (UTF-8), Unicode CLI "wide" APIs (UTF-16), and more.
These additions should make language extension writing easier.
CCSID topic describes new libdb400.a mode settings via SQLOverrideCCSID400(ccsid).

Run time, libdb400.a should fit seamless under any existing scripting language db2 extension. 
That is to say, exports everything old PASE libdb400.a, while providing advanced functions.
You do NOT have to recompile your language extension, simply set PASE LIBPATH for new libdb400.a.

At this time, this additional libdb400.a driver is designed to augment current PASE
libdb400.a. Therefore both must be on the machine. However, eventually
this libdb400.a driver may replace PASE version entirely.

This project originated because of a need to create async DB2 requests for Node.js on IBM i, 
but it isn't just for Node.js and can instead be applied to all PASE langs (PHP, Ruby, Python, etc).

#Future
Many more features are planned, such as, tracing CLI APIs, debug message to joblog, socket based db2,
web based db2, json based db2, etc. Ok, no promise, but, you get the idea.

Author two cents, when stable, start using this driver, 
you will grow function by leaps with very little effort.

#Run
Place new libdb400.a in some directory (mytest). 
Do not replace PASE /usr/lib/libdb400.a (someday).
```
$ export PATH=/mytest:$PATH
$ export LIBPATH=.:/mytest/lib:/usr/lib
$ run-my-scripts-or-whatever
```

This project gcc compiles are NOT using gcc runtime, aka,
not using perzl.org runtime elements, therefore 
do not put /opt/freeware front of LIBPATH (bad /opt/freeware).
```
bash-4.3$ echo $LIBPATH
.:/opt/freeware/lib:/usr/lib
bash-4.3$ test0003_async_callback_connect_64
Could not load program test0003_async_callback_connect_64:
Could not load module /home/monoroot/libdb400/tests/./libdb400.a(shr_64.o).
        Dependent module /opt/freeware/lib/libiconv.a(shr4_64.o) could not be loaded.
```

#Contributors
- Tony Cairns, IBM
- Aaron Bartell, Krengel Technology, Inc.

#Follow Along
We will be discussing things in the [Issues](http://bit.ly/db2sock-issues) section of this repo.  

#License
MIT

#------ DRIVER CLI TRACE -------
The driver will trace CLI calls with env var TRACE (latest only).
```
export TRACE=on (file)
export TRACE=off
export TRACE=ws (console)
```

```
bash-4.3$ export TRACE=on
bash-4.3$ ls /tmp
bash-4.3$ ./test0054_async_callback_query_fetch_array_64                    
main_environ (thread 1): starting
main_environ (thread 1): leaving
SQL400EnvironmentCallback (thread 258): starting
SQL400EnvironmentCallback (thread 258): complete: sqlrc=0, *ohnd=1 options=180001538 callback=180000d38
main_connect (thread 258): starting
:
bash-4.3$ ls /tmp/
libdb400_trace_129505
bash-4.3$ cat /tmp/libdb400_trace_129505 
SQLAllocHandle.129505.1492615392.1.tbeg +++success+++
SQLAllocHandle.129505.1492615392.1.walk printf_stack (ffffffffffff7a0)
SQLAllocHandle.129505.1492615392.1.walk dump_SQLAllocHandle (fffff950 10001a04e4298 a5f1bb7c 9001000a5f1bb7c)
SQLAllocHandle.129505.1492615392.1.walk SQLAllocHandle (104b8e0ddf00d a045a9a0 9001000a5f1bb7c)
SQLAllocHandle.129505.1492615392.1.walk custom_SQLOverrideCCSID400 (4b8a0449ab0)
:
```

#------ DRIVER BUILDER SECTION -------
For driver builders of scripting extensions (php ibm_db2, ruby ibm_db, etc.).
The idea with this 'super driver' is many tasks repeated over and over in each
language driver could be moved into the lowest CLI driver (libdb400.a).
Also adding features to each language, such as asynchronous calls, CLI tracing, etc,
will be much easier.

## build
All make files have been converted to gcc compiles.
```
optional (git already completed) ...
> python gen.py

gcc compiles ...
> ./make_libdb400.sh
```
Note: The gcc compiles will not run unless you take force gcc align quadword 
action in notes for PASE /usr/include/as400_types.h.

I am using a chroot with following packages from [ibmichroot](https://bitbucket.org/litmis/ibmichroot). 
```
> pkg_setup.sh pkg_perzl_gcc-4.8.3.lst
```


Alternative pre-compiled Yips binary

* http://yips.idevcloud.com/wiki/index.php/Databases/SuperDriver 
* (test only, not production)


##gen.py creates: 
- PaseCliAsync.h         -- header asynchronous extensions (php, node, ...)
- PaseCliAsync_gen.c     -- asynchronous driver APIs
- PaseCliILE_gen.c       -- direct ILE call APIs (exported)
- PaseCliLibDB400_gen.c  -- PASE libdb400.a override dlsyms
- libdb400.exp           -- all CLI export APIs

##human coding:
- PaseCliCustom.c        -- 'big function' APIs (experimental)
- PaseCliInit.c          -- db2 resource table manager (read)
- PaseCliInit.h          -- db2 resource table header

##examples
```
build
> cd tests
> python genmake.py
> ./xlcmakeall

optional set user profile
> export SQL_DB400="*LOCAL"
> export SQL_UID400="UID"
> export SQL_PWD400="PWD"

run 32-bit or 64-bit
> testnnnn_32
> testnnnn_64
```

##set CCSID first
You should call SQLOverrideCCSID400(ccsid), before any other SQL activity (see tests).
Environment setting SQLOverrideCCSID400 defines how this libdb400.a operates.
```
=== UTF-8 ccsid, normal Unix mode (direct ILE call) ===
SQLOverrideCCSID400(1208)
-->SQLExecDirect(Async)-->ILE_SQLExecDirect-->DB2

=== UTF-16 ccsid, 'W'ide interfaces (direct ILE call) ===
SQLOverrideCCSID400(1200)
-->SQLExecDirectW(Async)-->ILE_SQLExecDirectW-->DB2

=== PASE ccsid, original mode (call original libdb400.a) ===
SQLOverrideCCSID400(0) -- job ccsid, best guess
SQLOverrideCCSID400(pase_ccsid)
-->SQLExecDirect(Async)-->PASE libdb400.a(SQLExecDirect)-->DB2 (*)
```
(*) Calling old libdb400.a less desirable, so may change over time.


##Usage CLI APIs

In general, use CLI APIs, which, enable correct locking for async and non-async db2 operations.
However, feel free to use new direct call ILE DB2 APIs (ILE_SQLxxx). 
```
=== CLI APIs UTF-8 or APIWs UTF-16  ===
=== choose async and/or normal wait === 
SQLRETURN SQLExecDirect(..);
SQLRETURN SQLExecDirectW(..);

== callback or reap/join with async ===
pthread_t SQLExecDirectAsync(..);
pthread_t SQLExecDirectWAsync(..);
void SQLExecDirectCallback(SQLExecDirectStruct* );
SQLExecDirectStruct * SQLExecDirectJoin (pthread_t tid, SQLINTEGER flag);
void SQLExecDirectWCallback(SQLExecDirectWStruct* );
SQLExecDirectWStruct * SQLExecDirectWJoin (pthread_t tid, SQLINTEGER flag);

=== bypass all, call PASE libdb400.a directly  (not recommended) ===
SQLRETURN libdb400_SQLExecDirect(..);
SQLRETURN libdb400_SQLExecDirectW(..); (*)

=== bypass all, call ILE directly (not recommended) ===
SQLRETURN ILE_SQLExecDirect(..);
SQLRETURN ILE_SQLExecDirectW(..);

```

## Advanced features
Experimental advanced features, large, complex operations in one async call.

```
TBD - re-work 400 custom APIs in progress
```

##Note:
All make files have been converted to gcc compiles. 

In switching to gcc, you have to edit PASE header to force gcc align quadword.
If you miss this edit header step, all will compile, but NOTHING will run.

```
/usr/include/as400_types.h:
/*
 * Quadword (aligned) area for a tagged ILE pointer
 */
typedef union _ILEpointer {
#if !(defined(_AIX) || defined(KERNEL))
#pragma pack(1,16,_ILEpointer)	/* Force sCC quadword alignment */
#endif
/* CAUTION: Some compilers only provide 64-bits for long double */
#if defined( __GNUC__ )
    long double	align __attribute__((aligned(16))); /* force gcc align quadword */
#else
    long double align;	/* Force xlc quadword alignment (with -qldbl128 -qalign=natural) */
#endif
#ifndef _AIX
    void		*openPtr; /* MI open pointer (tagged quadword) */
#endif
    struct {
	char		filler[16-sizeof(uint64)];
	address64_t	addr;	/* (PASE) memory address */
    } s;
} ILEpointer;
```

gcc options in Makefile ...
```
### gmake
### gmake TGT64=64
### gcc options
### -v            - verbose compile
### -Wl,-bnoquiet - verbose linker
### -shared       - shared object
### -maix64       - 64bit
### -isystem      - compile PASE system headers
### -nostdlib     - remove libgcc_s.a and crtcxa_s.o

gcc code tip unsigned long long ...

gcc bug cast to unsigned long long not work (bad sign extend), 
therefore we also need cast ulong to match size of pointer 32/64 
   arglist->ohnd.s.addr = (ulong) ohnd; /* silly gcc compiler */
```

xlc no longer supported ...
```
When using xlc, use options -qldbl128 -qalign=natural. 
Missing these options will result in ILE DB2 call failures.
See /usr/include/as400_types.h, type ILEpointer (quadword align compiler issues).
```

