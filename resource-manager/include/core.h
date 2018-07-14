#ifndef CORE_H
#define CORE_H

typedef struct Watcher Watcher;				// defined in watcher.h
typedef struct Configurator Configurator;	// defined in config.h

struct AppState
{
	// system handles
	Configurator	*configurator;
	Watcher			*watcher;
};

#endif

