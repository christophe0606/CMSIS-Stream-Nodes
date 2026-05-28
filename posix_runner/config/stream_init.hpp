#pragma once

#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "stream_runtime_init.hpp"

#define NB_APPS 1

extern stream_execution_context_t contexts[NB_APPS];
extern int currentNetwork;

int stream_configure_and_start();
void stream_free_all(bool callerIsRuntimeThread = false);
