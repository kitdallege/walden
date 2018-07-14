#ifndef APP_H
#define APP_H

#include "reload/reload.h"

extern const struct AppApi app_api;
/*
 *
 * app
 *  * watcher
 *  * handler
 * 
 * ATM:
 *  watcher.step get events call handler
 *  handler.step process events. (might need to add watch) 
 *
 * app.step
 *   watcher.step: needs to figure out if it needs to add watches itself
 *   handler.step: just processes the path all dumb like.
 *
 * conf should get AppConfig passed in all the way down.
 * if you need to configure something, it gets pulled up into the conf file.
 *
 *
 * All these 'er' structs are really just *State structs.
 * The functions are the 'object/system's vtable API.
 *
 * you could have a Object struct which holds the vtable and state
 * and 'go there with it'. personally this is about as oo as i get,
 * I think of objects as 'systems' how one would code various sub-systems
 * in a game/simulation. This architecture allows you to do things like
 * hot-reload, custom memory management, multi-threading, a lot easy than
 * an organically structured application which attempts to 'bolt those' 
 * features on.
 * 
 *
 * Systems oriented programming:
 *  decouple state from functions that operate on it.
 *  provide a means to allocate, initialize, de-initialize, free state memory
 *  provide a means to perform an action.
 *   Type *type_aloc();
 *   type_conf(Type *self, void *user);
 *   type_zero(Type *self);
 *   type_free(Type *self);
 *
 *   // in simulations you typically have a single method 
 *   type_step(Type *self, void *user);
 * 
 *   // but could have many api methods in the form of 
 *   // where *self is the first arg and the rest are used defined.
 *	 type_*(Type *self, other args....);
 *
 *  the main 'part' is the state life-cycle funtions and that we've decoupled
 *  state from the functions that operate on it.
 *
 *  Systems can be nested (where their lifecycle is owned by the parent)
 *  or injected as an outside dependency.
 *
 *  The latter typically makes for easy to understand/modify systems.
 *
 * Reload:
 *  // maybe something like....
 *  Type *current = var, *new = type_aloc();
 *  if (new == 0x00 || type_conf(new, data)) {
 *     self = current;
 *     free(new); 
 *  } else {
 *     self = new;
 *     free(current);
 *  }
 * // the idea being that we allow config to fail.  and so can aloc
 *
 *
 *
 * TODO: alloc might grow a alloc_using 'friend' which would allow for
 * me to hook my own version of malloc/realloc/free on a per-instance
 * basis.. other ways of doing this are the sdl style where you have a static
 * which you can set, which is the 'allocator struct' to use.
 *
 * Whilst that is handy when you want to just say 'pool it all', there are 
 * times when i want to allocate objects in different memory areans.
 *
 * defines whilst removing the indirection, are the same issue.
 * 
 * I don't mind that i can't poll allocate system 'types'
 * as they handle their own allocation, typically there will be 'few'
 * systems which are all long lived.
 *
 * The container types used in the various systems, I could see wanting 
 * to specialize to pre-allocated pools.
 *  [something to think about]....
 */
#endif
